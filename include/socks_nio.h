#ifndef PROTOS_TP2E_SOCKS_NIO_H
#define PROTOS_TP2E_SOCKS_NIO_H

#include <sys/socket.h>
#include <stdint.h>

#include "./buffer.h"
#include "./hello.h"
#include "./hello_events.h"
#include "./socks_handler.h"
#include "./stm.h"

#define BUFFER_SIZE 64

#define ATTACHMENT(key) ((struct sock *)(key)->data)
#define N(x) (sizeof(x)/sizeof((x)[0]))

/**
 * ---------------------------------------------
 * Máquina de estados general
 * ---------------------------------------------
 */
enum socks_state {
    /**
     * recibe el mensaje `hello` del cliente, y lo procesa
     *
     * Intereses:
     *     - OP_READ sobre client_fd
     *
     * Transiciones:
     *   - HELLO_READ  mientras el mensaje no esté completo
     *   - HELLO_WRITE cuando está completo
     *   - ERROR       ante cualquier error (IO/parseo)
     */
    HELLO_READ,

    /**
     * envía la respuesta del `hello' al cliente.
     *
     * Intereses:
     *     - OP_WRITE sobre client_fd
     *
     * Transiciones:
     *   - HELLO_WRITE  mientras queden bytes por enviar
     *   - REQUEST_READ cuando se enviaron todos los bytes
     *   - ERROR        ante cualquier error (IO/parseo)
     */
    HELLO_WRITE,


    // estados terminales
    DONE,
    ERROR,
};

/**
 * Manejador de los diferentes eventos..
 */
static const struct fd_handler handler = {
        .handle_read = socks_read,
        .handle_write = socks_write,
        .handle_close = socks_close,
        .handle_block = socks_block,
};

/**
 * Si bien cada estado tiene su propio struct que le da un alcance
 * acotado, disponemos de la siguiente estructura para hacer una única
 * alocación cuando recibimos la conexión.
 *
 * Se utiliza un contador de referencias (references) para saber cuando debemos
 * liberarlo finalmente, y un pool para reusar alocaciones previas.
 */
struct sock {
    /** Información del cliente */
    int client_fd;
    struct sockaddr_storage client_addr;

    /** Información del origin */
    int origin_fd;
    struct addrinfo *origin_resolution;

    /** Maquinas de estados */
    struct state_machine stm;

    /** Estados para el client_fd */
    union {
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
    struct sock *next;
};

void socks_passive_accept(struct selector_key *key);

void socks_destroy(struct sock *s);

void socks_pool_destroy(void);

#endif //PROTOS_TP2E_SOCKS_NIO_H
