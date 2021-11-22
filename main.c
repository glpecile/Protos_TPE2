/**
 * main.c - servidor proxy socks concurrente
 *
 * Interpreta los argumentos de línea de comandos, y monta un socket
 * pasivo.
 *
 * Todas las conexiones entrantes se manejarán en éste hilo.
 *
 * Se descargará en otro hilos las operaciones bloqueantes (resolución de
 * DNS utilizando getaddrinfo), pero toda esa complejidad está oculta en
 * el selector.
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <signal.h>

#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>  // socket


#include "./include/selector.h"
#include "./include/args.h"
#include "./include/buffer.h"
#include "./include/socks_nio.h"
#include "./include/logger.h"

#define PENDING_CONNECTIONS 20
#define INITIAL_ELEMENTS 1024
#define TRUE 1
#define FALSE 0

static bool done = false;

static void
sigterm_handler(const int signal) {
    printf("Signal %d, cleaning up and exiting\n", signal);
    done = true;
}
//
//
//static int create_server_ipv6(int port, bool * error, char * err_msg) {
//    int server_ipv6 = -1;
//    struct sockaddr_in6 addr6;
//    memset(&addr6, 0, sizeof(addr6));
//    addr6.sin6_family = AF_INET6;
//    addr6.sin6_addr = in6addr_any;
//    addr6.sin6_port = htons(port);
//    *error = false;
//    err_msg = NULL;
//
//    server_ipv6 = socket(addr6.sin6_family, SOCK_STREAM, IPPROTO_TCP);
//    if (server_ipv6 < 0) {
//        strcpy(err_msg,"Unable to create socket ipv6");
//        *error = true;
//        return server_ipv6;
//    }
//
//
//    // man 7 ip. no importa reportar nada si falla.
//    setsockopt(server_ipv6, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int));
//    setsockopt(server_ipv6, SOL_IPV6, IPV6_V6ONLY, &(int) {1}, sizeof(int));
//
//    if (bind(server_ipv6, (struct sockaddr *) &addr6, sizeof(addr6)) < 0) {
//        strcpy(err_msg,"unable to bind socket ipv6");
//        *error = true;
//        return server_ipv6;
//    }
//
//    if (listen(server_ipv6, PENDING_CONNECTIONS) < 0) {
//        strcpy(err_msg,"unable to listen ipv6");
//        *error = true;
//        return server_ipv6;
//    }
//    if(err_msg == NULL)
//        fprintf(stdout, "Listening with IPV6 on TCP port %d\n", port);
//    return server_ipv6;
//}
//
//static int create_server_ipv4(int port, bool * error, char * err_msg) {
//    int server_ipv4 = -1;
//    struct sockaddr_in addr;
//    memset(&addr, 0, sizeof(addr));
//    addr.sin_family = AF_INET;
//    addr.sin_addr.s_addr = htonl(INADDR_ANY);
//    addr.sin_port = htons(port);
//    *error = false;
//
//    server_ipv4 = socket(addr.sin_family, SOCK_STREAM, IPPROTO_TCP);
//    if (server_ipv4 < 0) {
//        strcpy(err_msg,"Unable to create socket ipv4");
//        *error = true;
//        return server_ipv4;
//    }
//
//    // man 7 ip. no importa reportar nada si falla.
//    setsockopt(server_ipv4, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int));
//
//    if (bind(server_ipv4, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
//        strcpy(err_msg,"unable to bind socket ipv4");
//        *error = true;
//        return server_ipv4;
//    }
//
//    if (listen(server_ipv4, PENDING_CONNECTIONS) < 0) {
//        strcpy(err_msg,"Unable to listen ipv4");
//        *error = true;
//        return server_ipv4;
//    }
//
//    fprintf(stdout, "Listening with IPV4 on TCP port %d\n", port);
//    return server_ipv4;
//}

//static int initialize_selector(int port) {
////    const char *err_msg = NULL;
//    char err_msg[64]={0};
//    bool error = false;
//    selector_status ss = SELECTOR_SUCCESS;
//    fd_selector selector = NULL;
//
//    int server_ipv6 = create_server_ipv6(port, &error, err_msg);
//    if(error){
//        goto finally;
//    }
//    int server_ipv4 = create_server_ipv4(port, &error, err_msg);
//    if(error){
//        goto finally;
//    }
//
//    signal(SIGTERM, sigterm_handler);
//    signal(SIGINT, sigterm_handler);
//
//    if (selector_fd_set_nio(server_ipv6) == -1) {
//        error = true;
//        strcpy(err_msg,"getting server socket flags for server_ipv6");
//        goto finally;
//    }
//    if (selector_fd_set_nio(server_ipv4) == -1) {
//        error = true;
//        strcpy(err_msg,"getting server socket flags for server_ipv4");
//        goto finally;
//    }
//    //1-Iniciar la libreria
//    const struct selector_init conf = {
//            .signal = SIGALRM,
//            .select_timeout = {
//                    .tv_sec  = 10,
//                    .tv_nsec = 0,
//            },
//    };
//    if (0 != selector_init(&conf)) {
//        error = true;
//        strcpy(err_msg,"initializing selector");
//        goto finally;
//    }
//
//    selector = selector_new(INITIAL_ELEMENTS);
//    if (selector == NULL) {
//        error = true;
//        strcpy(err_msg,"unable to create selector");
//        goto finally;
//    }
//    const struct fd_handler socks_handler = {
//            .handle_read       = socks_passive_accept,
//            .handle_write      = NULL,
//            .handle_close      = NULL, // nada que liberar
//    };
//
//    ss = selector_register(selector, server_ipv6, &socks_handler, OP_READ,NULL);
//    if (ss != SELECTOR_SUCCESS) {
//        error = true;
//        strcpy(err_msg,"registering fd for server_ipv6");
//        goto finally;
//    }
//
//    ss = selector_register(selector, server_ipv4, &socks_handler, OP_READ, NULL);
//    if (ss != SELECTOR_SUCCESS) {
//        error = true;
//        strcpy(err_msg,"registering fd for server_ipv4");
//        goto finally;
//    }
//
//    for (; !done;) {
//        fprintf(stdout,"Waiting for incoming connection...\n");
//        error = false;
//        ss = selector_select(selector);
//        if (ss != SELECTOR_SUCCESS) {
//            error = true;
//            strcpy(err_msg,"serving");
//            goto finally;
//        }
//    }
//
//    if (!error) {
//        strcpy(err_msg,"closing");
//    }
//
//    int ret = 0;
//
//    finally:
//    if (ss != SELECTOR_SUCCESS) {
//        fprintf(stderr, "%s: %s\n", error ? "" : err_msg,
//                ss == SELECTOR_IO
//                ? strerror(errno)
//                : selector_error(ss));
//        ret = 2;
//    } else if (error) {
//        perror(err_msg);
//        ret = 1;
//    }
//    if (selector != NULL) {
//        selector_destroy(selector);
//    }
//    printf("about to close the selector\n");
//    selector_close();
//    printf("about to destroy the pool\n");
//    socks_pool_destroy();
//
//    if (server_ipv6 >= 0) {
//        printf("about to close the server_ipv6\n");
//        close(server_ipv6);
//    }
//    if (server_ipv4 >= 0) {
//        printf("about to close the server_ipv4\n");
//        close(server_ipv4);
//    }
//    free(sock_args);
//    printf("closing main safely...\n");
//    return ret;
//}

static int initialize_server(int port) {
    const char *err_msg = NULL;
    selector_status ss = SELECTOR_SUCCESS;
    fd_selector selector = NULL;

    int server_ipv6 = -1;
    int server_ipv4 = -1;

    /**
     * Creamos el socket para UDP
     */
    struct sockaddr_in udp_address;
    int upd_socket_type = SOCK_DGRAM;
    int udp_socket;
    int opt = TRUE;

    //socket upd created
    if ((udp_socket = socket(AF_INET, upd_socket_type, 0)) < 0) {
        err_msg = "socket failed";
        goto finally;
    }
    //set master socket to allow multiple connections.
    if (setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
        err_msg = "setsockopt";
        goto finally;
    }

    //type of socket address created
    udp_address.sin_family = AF_INET;
    udp_address.sin_addr.s_addr = INADDR_ANY;
    udp_address.sin_port = htons(port);

    //bind the master socket.
    if (bind(udp_socket, (struct sockaddr *) &udp_address, sizeof(udp_address)) < 0) {
        if (close(udp_socket) < 0) {
            err_msg = "close failed";
            goto finally;
        }
        err_msg = "bind failed";
        goto finally;
    }

    fprintf(stdout, "UDP Listener on port %d\n", port);

    /**
     * Creamos el socket para IPv6
     */
    struct sockaddr_in6 addr6;
    memset(&addr6, 0, sizeof(addr6));
    addr6.sin6_family = AF_INET6;
    addr6.sin6_addr = in6addr_any;
    addr6.sin6_port = htons(port);

    server_ipv6 = socket(addr6.sin6_family, SOCK_STREAM, IPPROTO_TCP);
    if (server_ipv6 < 0) {
        err_msg = "Unable to create socket ipv6";
        goto finally;
    }

    fprintf(stdout, "Listening on TCP port %d\n", port);

    // man 7 ip. no importa reportar nada si falla.
    setsockopt(server_ipv6, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int));
    setsockopt(server_ipv6, SOL_IPV6, IPV6_V6ONLY, &(int) {1}, sizeof(int));

    if (bind(server_ipv6, (struct sockaddr *) &addr6, sizeof(addr6)) < 0) {
        err_msg = "unable to bind socket ipv6";
        goto finally;
    }

    if (listen(server_ipv6, PENDING_CONNECTIONS) < 0) {
        err_msg = "unable to listen ipv6";
        goto finally;
    }

    /**
     * Creamos el socket para IPv4
     */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    server_ipv4 = socket(addr.sin_family, SOCK_STREAM, IPPROTO_TCP);
    if (server_ipv4 < 0) {
        err_msg = "Unable to create socket ipv4";
        goto finally;
    }

    fprintf(stdout, "Listening on TCP port %d\n", port);

    // man 7 ip. no importa reportar nada si falla.
    setsockopt(server_ipv4, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int));

    if (bind(server_ipv4, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        err_msg = "unable to bind socket ipv4";
        goto finally;
    }

    if (listen(server_ipv4, PENDING_CONNECTIONS) < 0) {
        err_msg = "unable to listen ipv4";
        goto finally;
    }

    /**
     * Registrar sigterm es útil para terminar el programa normalmente.
     * Esto ayuda mucho en herramientas como valgrind.
     */
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    if (selector_fd_set_nio(server_ipv6) == -1) {
        err_msg = "getting server socket flags";
        goto finally;
    }
    if (selector_fd_set_nio(server_ipv4) == -1) {
        err_msg = "getting server socket flags";
        goto finally;
    }
    //1-Iniciar la libreria
    const struct selector_init conf = {
            .signal = SIGALRM,
            .select_timeout = {
                    .tv_sec  = 10,
                    .tv_nsec = 0,
            },
    };
    if (0 != selector_init(&conf)) {
        err_msg = "initializing selector";
        goto finally;
    }

    selector = selector_new(INITIAL_ELEMENTS);
    if (selector == NULL) {
        err_msg = "unable to create selector";
        goto finally;
    }
    const struct fd_handler socks_handler = {
            .handle_read       = socks_passive_accept,
            .handle_write      = NULL,
            .handle_close      = NULL, // nada que liberar
    };
    ss = selector_register(selector, server_ipv6, &socks_handler, OP_READ, NULL);
    if (ss != SELECTOR_SUCCESS) {
        err_msg = "registering fd";
        goto finally;
    }
    ss = selector_register(selector, server_ipv4, &socks_handler, OP_READ, NULL);
    if (ss != SELECTOR_SUCCESS) {
        err_msg = "registering fd";
        goto finally;
    }
    for (; !done;) {
        fprintf(stdout, "Waiting for incoming connection...\n");
        err_msg = NULL;
        ss = selector_select(selector);
        if (ss != SELECTOR_SUCCESS) {
            err_msg = "serving";
            goto finally;
        }
    }
    if (err_msg == NULL) {
        err_msg = "closing";
    }

    int ret = 0;

    /**
     * Finally
     */
    finally:
    if (ss != SELECTOR_SUCCESS) {
        fprintf(stderr, "%s: %s\n", (err_msg == NULL) ? "" : err_msg,
                ss == SELECTOR_IO
                ? strerror(errno)
                : selector_error(ss));
        ret = 2;
    } else if (err_msg) {
        perror(err_msg);
        ret = 1;
    }
    if (selector != NULL) {
        selector_destroy(selector);
    }
    printf("about to close the selector\n");
    selector_close();
    printf("about to destroy the pool\n");
    socks_pool_destroy();

    if (server_ipv6 >= 0) {
        printf("about to close the server_ipv6\n");
        close(server_ipv6);
    }
    if (server_ipv4 >= 0) {
        printf("about to close the server_ipv4\n");
        close(server_ipv4);
    }
    if (udp_socket >= 0){
        printf("about to close the udp socket\n");
        close(udp_socket);
    }
    free(parameters);
    printf("closing main safely...\n");
    return ret;
}

int
main(const int argc, char **argv) {

    if (parse_parameters(argc, argv) < 0) {
        return -1;
    }

    initialize_pop3_parameters_options();
    assign_param_values(argc, argv);

    //No tenemos nada que leer de stdin.
    //Un file descriptor extra
    close(STDIN_FILENO);

    if (setvbuf(stdout, NULL, _IONBF, 0)) {
        perror("Unable to disable buffering");
        free(parameters);
        exit(-1);
    }
    if (setvbuf(stderr, NULL, _IONBF, 0)) {
        perror("Unable to disable buffering");
        free(parameters);
        exit(-1);
    }
    return initialize_server(parameters->port);
}
