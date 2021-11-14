#include <socket.h>
#include <unistd.h>

#include "../include/socks_handler.h"
#include "../include/stm.h"
#include "../include/selector.h"

struct sock_state {
    struct state_machine stm;

    union{
        struct hello_st hello;
//        struct request_st request;
//        struct copy copy;
    } client;

//    union{
//        struct connecting conn;
//        strut copy copy;
//    } orig;
};

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

