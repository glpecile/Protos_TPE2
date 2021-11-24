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
    log(INFO, "%s", "CONNECTING_ST");
    int error;
    socklen_t len = sizeof(error);
    struct sock *d = ATTACHMENT(key);
    char *error_msg = "-ERR Connection refused.\r\n";

    d->origin_fd = key->fd;

    if (getsockopt(key->fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        send(d->client_fd, error_msg, strlen(error_msg), 0);
        log(ERROR, "%s", "Connection failed");
        selector_set_interest_key(key, OP_NOOP);
        return ERROR_ST;
    } else {
        if (error == 0) {
            d->origin_fd = key->fd;
        } else {
            send(d->client_fd, error_msg, strlen(error_msg), 0);
            log(ERROR, "%s", "Connection failed");
            selector_set_interest_key(key, OP_NOOP);
            return ERROR_ST;
        }
    }

    selector_status ss = SELECTOR_SUCCESS;
    ss |= selector_set_interest_key(key, OP_WRITE); //TODO esto tiene que cambiar a read
    ss |= selector_set_interest(key->s, d->client_fd, OP_NOOP);

    return SELECTOR_SUCCESS == ss ? GREETINGS_ST : ERROR_ST;
}
