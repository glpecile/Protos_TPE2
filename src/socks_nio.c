#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#include "../include/buffer.h"
#include "../include/hello.h"
#include "../include/selector.h"
#include "../include/socks_nio.h"
#include "../include/stm.h"

#define MAX_POOL 89; //numero primo y pertenece a la secuencia de fibonacci.

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
 * Definición de variables para cada estado
 */

/** usado por HELLO_READ, HELLO_WRITE */
struct hello_st {
    /** buffer utilizado para I/O */
    buffer *rb, *wb;
    struct hello_parser parser;
    /** el método de autenticación seleccionado */
    uint8_t method;
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

/*
    Pool de estructuras de socks inutilizadas (se aloco memoria pero no tienen uso)
    La idea consiste en que cuando se termina de utilizar de forma logica la estructura,
    es guardada en una pila para que luego otro evento pueda usar ese espacio.
    Al momento de crear un socks_state nuevo hay que preguntar si existe espacio inutilizado.
    Al finalizar un evento debe agregarselo a la pila.
*/
static unsigned pool_size = 0;
static struct sock *pool = NULL;

static const struct state_definition * get_client_states()

static struct sock *socks_new(int client_fd) {
    struct sock *to_return;

    if (pool == NULL) {
        //espacio de memoria nuevo
        to_return = malloc(sizeof(struct sock *));
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


finally:
    return to_return;

}

void
socks_passive_accept(struct selector_key *key) {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct sock *state = NULL;

    // aceptamos el pedido pendiente que se encuentre en la cola, el socket es no bloqueante. debemos cachear el error.
    const int client = accept(key->fd, (struct sockaddr *) &client_addr, &client_addr_len);
    // pedidos de conexion --> accept --> si hay lo acepta y crea un socket nuevo (fd nuevo)
    //                                -->no hay es non block el codigo debe continuar.
    // no se encontro pedidos de conexion pendientes o hubo un error.
    if (client == -1) {
        goto fail;
    }
    // convierte en non blocking el socket nuevo.
    if (selector_fd_set_nio(client) == -1) {
        goto fail;
    }
    // tenemos el nuevo socket non blocking

    fail:
    if (client != -1) {
        close(client);
    }
}

/**
 * ---------------------------------------------
 * HELLO
 * ---------------------------------------------
 */

/**
 * callback del parser utilizado en `read_hello'
 */
static void
on_hello_method(struct hello_parser *p, const uint8_t method) {
    uint8_t *selected = p->data;

    if (SOCKS_HELLO_NOAUTHENTICATION_REQUIRED == method) {
        *selected = method;
    }
}

/**
 * inicializa las variables de los estados HELLO_…
 */
static void
hello_read_init(const unsigned state, struct selector_key *key) {
    struct hello_st *d = &ATTACHMENT(key)->client.hello;

    d->rb = &(ATTACHMENT(key)->read_buffer);
    d->wb = &(ATTACHMENT(key)->write_buffer);
    d->parser.data = &d->method;
    d->parser.on_authentication_method = on_hello_method, hello_parser_init(
            &d->parser);
}

static unsigned
hello_process(const struct hello_st *d);

/**
 * lee todos los bytes del mensaje de tipo `hello' y inicia su proceso
 */
static unsigned
hello_read(struct selector_key *key) {
    struct hello_st *d = &ATTACHMENT(key)->client.hello;
    unsigned ret = HELLO_READ;
    bool error = false;
    uint8_t *ptr;
    size_t count;
    ssize_t n;

    ptr = buffer_write_ptr(d->rb, &count);
    n = recv(key->fd, ptr, count, 0);
    if (n > 0) {
        buffer_write_adv(d->rb, n);
        const enum hello_state st = hello_consume(d->rb, &d->parser, &error);
        if (hello_is_done(st, 0)) {
            if (SELECTOR_SUCCESS == selector_set_interest_key(key, OP_WRITE)) {
                ret = hello_process(d);
            } else {
                ret = ERROR;
            }
        }
    } else {
        ret = ERROR;
    }

    return error ? ERROR : ret;
}

/**
 * procesamiento del mensaje `hello'
 */
static unsigned
hello_process(const struct hello_st *d) {
    unsigned ret = HELLO_WRITE;

    uint8_t m = d->method;
    const uint8_t r = (m == SOCKS_HELLO_NO_ACCEPTABLE_METHODS) ? 0xFF : 0x00;
    if (-1 == hello_marshall(d->wb, r)) {
        ret = ERROR;
    }
    if (SOCKS_HELLO_NO_ACCEPTABLE_METHODS == m) {
        ret = ERROR;
    }
    return ret;
}


/**
 * Definición de handlers para cada estado.
*/
static const struct state_definition client_states[] = {
        {
                .state            = HELLO_READ,
                .on_arrival       = hello_read_init,
                .on_read_ready    = hello_read,
        },{
                .state            = HELLO_WRITE,
                .on_write_ready   = hello_process,
        }
};

static const struct state_definition * get_client_states() {
    return client_states;
}

