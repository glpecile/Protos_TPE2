#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/buffer.h"
#include "../include/hello.h"
#include "../include/selector.h"
#include "../include/socks_nio.h"
#include "../include/stm.h"

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
    to_return->stm.initial = HELLO_READ;
    to_return->stm.max_state = DONE;
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
        free(s);
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
    // convierte en non blocking el socket nuevo.
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
static const struct state_definition client_states[] = {
        {
                .state            = HELLO_READ,
                .on_arrival       = hello_read_init,
                .on_read_ready    = hello_read,
        },
        {
                .state            = HELLO_WRITE,
                .on_write_ready   = hello_write,
        },
        {
                .state = DONE,
        },
        {
                .state = ERROR,
        }
};

static const struct state_definition *get_client_states() {
    return client_states;
}

