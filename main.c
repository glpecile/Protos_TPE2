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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>

#include <unistd.h>
#include <sys/types.h>   // socket
#include <sys/socket.h>  // socket
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "./include/selector.h"
#include "./include/args.h"

//static bool done = false;

//static void
//sigterm_handler(const int signal) {
//    printf("Signal %d, cleaning up and exiting\n", signal);
//    done = true;
//}

int
main(const int argc,  char **argv) {
    struct socks5args * sock_args = malloc(sizeof(struct socks5args));
    parse_args(argc, argv, sock_args);



    return 0;
}
