#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#include "../include/args.h"
#include "../include/dns_resolution_events.h"
#include "../include/logger.h"
#include "../include/socks_nio.h"

/**
 * Realiza la resolución de DNS bloqueante
 *
 * Una vez resuelta notifica al selector para que el evento
 * esté disponible en la próxima iteración.
 */
static void *
dns_resolution_blocking(void *data) {
    struct selector_key *key = (struct selector_key *) data;
    struct sock *s = ATTACHMENT(key);

    pthread_detach(pthread_self());
    s->origin_resolution = 0;
    struct addrinfo hints = {
            .ai_family = AF_UNSPEC, /* Allow IPv4 or IPv6 */
            .ai_socktype = SOCK_STREAM, /* Datagram socket */
            .ai_flags = AI_PASSIVE, /* For wildcard IP address */
            .ai_protocol = 0, /* Any protocol */
            .ai_canonname = NULL,
            .ai_addr = NULL,
            .ai_next = NULL,
    };

    char buff[7];
    snprintf(buff, sizeof(buff), "%d", parameters->origin_port);

    printf("%s\n", parameters->origin_server);

    if (getaddrinfo(parameters->origin_server, buff, &hints, &s->origin_resolution) != 0) {
//        log(ERROR, "DNS resolution error");
    }

    selector_notify_block(key->s, key->fd);

    free(data);

    return 0;
}

static unsigned
request_connect(struct selector_key *key) {
    struct sock *s = ATTACHMENT(key);
    unsigned ret = CONNECTING_ST;

    int sock = socket(s->origin_domain, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        perror("socket creation failed.");
        return ERROR_ST;
    }
    //Para que no sea bloqueante.
    if (selector_fd_set_nio(sock) == -1) {
        goto error;
    }


    if (connect(sock,(const struct sockaddr *) &s->origin_addr, s->origin_addr_len) == -1) {
        if (errno == EINPROGRESS) {
            //Al ser no bloqueante si todavia no establecio la conexion... OP_NOOP->sin un interes en particular
            selector_status st = selector_set_interest_key(key, OP_NOOP);
            if (st != SELECTOR_SUCCESS) {
                goto error;
            }
            //registramos la conexion en el selector como WRITE (como indican las buenas practicas)
            st = selector_register(key->s, sock, &handler, OP_WRITE, key->data);
            if (st != SELECTOR_SUCCESS) {
                goto error;
            }
            //indicamos que ahora hay otra conexion que depende de este estado.
//            ATTACHMENT(key)->origin_fd = sock;
            s->references += 1;

        } else {
            goto error;
        }
    } else {
        abort();
    }

    return ret;

    error:
    ret = ERROR_ST;
    fprintf(stdout, "Connecting to origin server failed.\n");

    if (sock != -1) {
        close(sock);
    }

    return ret;
}

/**
 * Creamos un thread que se va a encargar de la resolución dns.
 *
 */
unsigned
dns_resolution(struct selector_key *key) {
    pthread_t thread_id;
    struct selector_key *resolution_key = malloc(sizeof(*key));
    unsigned ret = DNS_RESOLUTION_ST;

    if (resolution_key == NULL) {
//        log(ERROR, "Error allocating resolution_key");
        ret = ERROR_ST;
    } else {
        memcpy(resolution_key, key, sizeof(*key));
        if (pthread_create(&thread_id, 0, dns_resolution_blocking, resolution_key) != 0) {
            ret = ERROR_ST;
        } else {
            selector_set_interest_key(key, OP_NOOP);
        }
    }

    return ret;
}

/**
 * Procesa el resultado de la resolución de nombres.
 */
unsigned
dns_resolution_done(struct selector_key *key) {
    struct sock *s = ATTACHMENT(key);

    if (s->origin_resolution == 0) {
//        log(ERROR, "Invalid origin resolution");
        return ERROR_ST;
    } else {
        s->origin_domain = s->origin_resolution->ai_family;
        s->origin_addr_len = s->origin_resolution->ai_addrlen;
        memcpy(&s->origin_addr, s->origin_resolution->ai_addr, s->origin_resolution->ai_addrlen);
        freeaddrinfo(s->origin_resolution);
        s->origin_resolution = 0;
    }

    return request_connect(key);
}




