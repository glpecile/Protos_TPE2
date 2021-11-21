#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

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
    to_return->stm.initial = PRECONNECTING;
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
    fprintf(stdout, "New connection with client %d established", state->client_fd);
    return;

    fail:
    if (client != -1) {
        close(client);
    }

    socks_destroy(state);
}

static unsigned
preconnecting(struct selector_key *key) {
    //TODO set domain after name resolution
    /**Socket dedicado al origin server. Por cada cliente hay un socket dedicado al origin**/
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    unsigned ret = CONNECTING;
    //Cacheamos el error de socket()
    if(sock < 0){
        perror("socket creation failed.");
        return ERROR;
    }
    //Para que no sea bloqueante.
    if(selector_fd_set_nio(sock) == -1){
        goto error;
    }

    //TODO esto luego se debe obtener de la resolucioon de nombres.-
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockaddr.sin_port = htons(8080);

    if(connect(sock,(const struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1){
        if(errno == EINPROGRESS){
            //Al ser no bloqueante si todavia no establecio la conexion... OP_NOOP->sin un interes en particular
            selector_status st = selector_set_interest_key(key, OP_NOOP);
            if(st != SELECTOR_SUCCESS){
                goto error;
            }
            //registramos la conexion en el selector como WRITE (como indican las buenas practicas)
            st = selector_register(key->s,sock,&handler,OP_WRITE, key->data);
            if(st != SELECTOR_SUCCESS){
                goto error;
            }
            //indicamos que ahora hay otra conexion que depende de este estado.
//            ATTACHMENT(key)->origin_fd = sock;
            ATTACHMENT(key)->references += 1;

        }else{
            goto error;
        }
    }else{
        abort();
    }

    return ret;

    error:
    ret = ERROR;
    fprintf(stdout,"Connecting to origin server failed.\n");

    if(sock != -1) {
        close(sock);
    }

    return ret;
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
//                .state = DONE,
//        },
//        {
//                .state = ERROR,
//        }
//};
static const struct state_definition client_states[] = {
        {
                .state = PRECONNECTING,
                .on_write_ready = preconnecting
        },
        {
                .state = CONNECTING,
                .on_write_ready = connecting
        },
        {
                .state = COPYING,
                .on_arrival = copy_init,
                .on_read_ready = copy_r,
                .on_write_ready = copy_w
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

