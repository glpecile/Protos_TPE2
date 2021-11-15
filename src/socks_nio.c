#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#include "../include/socks_nio.h"
#include "../include/stm.h"
#include "../include/selector.h"
#include "../include/buffer.h"
#include "../include/hello.h"

#define MAX_POOL 89; //numero primo y pertenece a la secuencia de fibonacci.
/**
 * Si bien cada estado tiene su propio struct que le da un alcance
 * acotado, disponemos de la siguiente estructura para hacer una única
 * alocación cuando recibimos la conexión.
 *
 * Se utiliza un contador de referencias (references) para saber cuando debemos
 * liberarlo finalmente, y un pool para reusar alocaciones previas.
 */
struct sock_state {
    /** Información del cliente */
    int client_fd;

    /** Información del origin */
    int origin_fd;
    struct addrinfo * origin_resolution;
    
    /** Maquinas de estados */
    struct state_machine stm;

    /** Estados para el client_fd */
    union{
        struct hello_st hello;
//        struct request_st request;
//        struct copy copy;
    } client;

    /** Estados para el origin_fd */
//    union{
//        struct connecting conn;
//        strut copy copy;
//    } orig;

    /** Buffers */
    buffer read_buffer;
    uint8_t read_buffer_space[BUFFER_SIZE];
    buffer write_buffer;
    uint8_t write_buffer_space[BUFFER_SIZE];

    /** Contador de referencias (cliente o origen que utiliza este estado)*/
    unsigned int references;
    /** Siguiente estructura */
    struct sock_state * next;
};

/*
Pool de estructuras de socks inutilizadas (se aloco memoria pero no tienen uso)
La idea consiste en que cuando se termina de utilizar de forma logica la estructura, 
es guardada en una pila para que luego otro evento pueda usar ese espacio.
Al momento de crear un socks_state nuevo hay que preguntar si existe espacio inutilizado.
Al finalizar un evento debe agregarselo a la pila.
*/
static unsigned pool_size = 0;
static struct sock_state * pool = NULL;

static struct sock_state * socks_new(int client_fd){
    struct sock_state * to_return;
    
    if(pool == NULL) {
        //espacio de memoria nuevo
        to_return = malloc(sizeof(struct sock_state *));
    } else {
        //reutilizacion de espacio
        to_return = pool;
        pool = pool->next;
        to_return->next = NULL;
        pool_size--;
    }
    
    if(to_return == NULL) {
        goto finally;
    }

    //Setear en cero, ya sea por un malloc, o por reutilizar uno existente
    memset(to_return, 0x00, sizeof(struct sock_state *));
    
    //inicializacion de la estructura
    to_return->client_fd = client_fd;
    //se seteara en el estado de connecting con el origen
    to_return->origin_fd = -1;

    
    
    
    finally:
    return to_return;

}

void
socks_passive_accept(struct selector_key *key) {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct sock_state *state = NULL;

    //aceptamos el pedido pendiente que se encuentre en la cola, el socket es no bloqueante. debemos cachear el error.
    const int client = accept(key->fd, (struct sockaddr *) &client_addr, &client_addr_len);
    //pedidos de conexion --> accept --> si hay lo acepta y crea un socket nuevo (fd nuevo)
    //                                -->no hay es non block el codigo debe continuar.
    //no se encontro pedidos de conexion pendientes o hubo un error.
    if (client == -1) {
        goto fail;
    }
    //convierte en non blocking el socket nuevo.
    if(selector_fd_set_nio(client) == -1) {
        goto fail;
    }
    //tenemos el nuevo socket non blocking
    
    
    fail:
    if (client != -1) {
        close(client);
    }
}

