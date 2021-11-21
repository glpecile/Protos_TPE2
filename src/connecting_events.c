#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>


#include "../include/connecting_events.h"
#include "../include/socks_nio.h"

unsigned
connecting(struct selector_key *key) {
    fprintf(stdout, "CONNECTING");
    int error;
    socklen_t len = sizeof(error);
    struct sock *d = ATTACHMENT(key);
    char *error_msg = "-ERR Connection refused.\r\n";

    d->origin_fd = key->fd;

    if (getsockopt(key->fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        send(d->client_fd, error_msg, strlen(error_msg), 0);
        fprintf(stderr, "Connection failed\n");
        selector_set_interest_key(key, OP_NOOP);
        return ERROR;
    } else {
        if (error == 0) {
            d->origin_fd = key->fd;
        } else {
            send(d->client_fd, error_msg, strlen(error_msg), 0);
            fprintf(stderr, "Connection failed\n");
            selector_set_interest_key(key, OP_NOOP);
            return ERROR;
        }
    }

     selector_status ss = SELECTOR_SUCCESS;
    ss |= selector_set_interest_key(key, OP_WRITE);
    ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_NOOP);

    return SELECTOR_SUCCESS == ss ? COPYING : ERROR;

}
