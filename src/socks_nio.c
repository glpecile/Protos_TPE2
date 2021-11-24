#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/buffer.h"
#include "../include/dns_resolution_events.h"
#include "../include/socks_nio.h"
#include "../include/capa_events.h"
#include "../include/response_events.h"

#define MAX_POOL 89 // numero primo y pertenece a la secuencia de fibonacci.

/*
    Pool de estructuras de socks inutilizadas (se aloco memoria pero no tienen uso)
    La idea consiste en que cuando se termina de utilizar de forma logica la estructura,
    es guardada en una pila para que luego otro evento pueda usar ese espacio.
    Al momento de crear un socks_state nuevo hay que preguntar si existe espacio inutilizado.
    Al finalizar un evento debe agregarselo a la pila.
*/
static unsigned pool_size = 0;
static struct sock *pool = NULL;

static const struct state_definition *get_client_states();

static struct sock *socks_new(int client_fd) {
    struct sock *to_return;

    if (pool == NULL) {
        //espacio de memoria nuevo
        to_return = malloc(sizeof(struct sock));
    } else {
        //reutilizacion de espacio
        to_return = pool;
        pool = pool->next;
        to_return->next = NULL;
        pool_size--;
    }

    if (to_return == NULL) {
        goto finally;
    }

    // Setear en cero, ya sea por un malloc, o por reutilizar uno existente.
    memset(to_return, 0x00, sizeof(struct sock));

    // Inicializacion de la estructura.
    to_return->client_fd = client_fd;
    // Se seteara en el estado de connecting con el origen.
    to_return->origin_fd = -1;

    // Seteo de la maquina de estados
    to_return->stm.initial = DNS_RESOLUTION_ST;
    to_return->stm.max_state = ERROR_ST;
    to_return->stm.states = get_client_states();

    // Inicialización de la máquina de estados
    stm_init(&to_return->stm);

    // Inicialización de los buffers
    buffer_init(&to_return->read_buffer, N(to_return->read_buffer_space), to_return->read_buffer_space);
    buffer_init(&to_return->write_buffer, N(to_return->write_buffer_space), to_return->write_buffer_space);

    to_return->references = 1;

    finally:
    return to_return;

}

/** realmente destruye */
static void
socks_destroy_(struct sock *s) {
    if (s->origin_resolution != NULL) {
        freeaddrinfo(s->origin_resolution);
        s->origin_resolution = 0;
    }
    if(s->client.request.cmd_queue != NULL)
        free_queue(s->client.request.cmd_queue);
    free(s);
}

/**
 * destruye un  `struct socks5', tiene en cuenta las referencias
 * y el pool de objetos.
 */
void
socks_destroy(struct sock *s) {
    if (s == NULL) {
        // nada para hacer
    } else if (s->references == 1) {
        if (s != NULL) {
            if (pool_size < MAX_POOL) {
                s->next = pool;
                pool = s;
                pool_size++;
            } else {
                socks_destroy_(s);
            }
        }
    } else {
        s->references -= 1;
    }
}

void
socks_pool_destroy(void) {
    struct sock *next, *s;
    for (s = pool; s != NULL; s = next) {
        next = s->next;
        socks_destroy_(s);
    }
}

void
socks_passive_accept(struct selector_key *key) {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct sock *state = NULL;

    // aceptamos el pedido pendiente que se encuentre en la cola, el socket es no bloqueante. debemos cachear el error.
    const int client = accept(key->fd, (struct sockaddr *) &client_addr, &client_addr_len);
    // pedidos de conexion --> accept --> si hay lo acepta y crea un socket nuevo (fd nuevo)
    //                                --> no hay es non block el codigo debe continuar.
    // no se encontro pedidos de conexion pendientes o hubo un error.
    if (client == -1) {
        goto fail;
    }
    // convierte en  non blocking el socket nuevo.
    if (selector_fd_set_nio(client) == -1) {
        goto fail;
    }
    // tenemos el nuevo socket non blocking
    state = socks_new(client);

    if (state == NULL) {
        goto fail;
    }
    // Como se creo bien el estado y la conexion se pudo efectuar, le paso los datos del cliente al estado.
    memcpy(&state->client_addr, &client_addr, client_addr_len);

    if (selector_register(key->s, client, &handler, OP_WRITE, state) != SELECTOR_SUCCESS) {
        goto fail;
    }
    stats_new_connection();
    fprintf(stdout, "New connection with client %d established", state->client_fd);
    return;

    fail:
    if (client != -1) {
        close(client);
    }

    socks_destroy(state);
}

/**
 * Definición de handlers para cada estado.
*/
//static const struct state_definition client_states[] = {
//        {
//                .state            = HELLO_READ,
//                .on_arrival       = hello_read_init,
//                .on_read_ready    = hello_read,
//        },
//        {
//                .state            = HELLO_WRITE,
//                .on_write_ready   = hello_write,
//        },
//        {
//                .state = DONE_ST,
//        },
//        {
//                .state = ERROR_ST,
//        }
//};
static const struct state_definition client_states[] = {
        {
                .state = DNS_RESOLUTION_ST,
                .on_write_ready = dns_resolution,
                .on_block_ready = dns_resolution_done
        },
        {
                .state = CONNECTING_ST,
                .on_write_ready = connecting
        },
        {
                .state = GREETINGS_ST,
                .on_arrival = greetings_init,
                .on_read_ready = greetings_read,
                .on_write_ready = greetings_write,
        },
        {
                .state = CAPA_ST,
                .on_arrival = capa_init,
                .on_read_ready = capa_read,//Segundo
                .on_write_ready = capa_send,//Primero
        },
        {
                .state = REQUEST_ST,
                .on_arrival = request_init,
                .on_read_ready = request_read,//Primero leo del cliente
                .on_write_ready = request_send,//Segundo escribo al origen
        },
        {
                .state = RESPONSE_ST,
                .on_arrival = response_init,
                .on_read_ready = response_read, //Primero se recibe la rta del origin
                .on_write_ready = response_send,//Ya se obtiene la rta que hay que entregarle al cliente
        },
        {
                .state = COPYING_ST,
                .on_arrival = copy_init,
                .on_read_ready = copy_r,
                .on_write_ready = copy_w,
        },
        {
                .state = DONE_ST,
        },
        {
                .state = ERROR_ST,
        }
};

static const struct state_definition *get_client_states() {
    return client_states;
}

