/**
 * Handlers top level de la conexión pasiva.
 * son los que emiten los eventos a la maquina de estados.
 */

#include <stdlib.h>
#include <unistd.h>

#include "../include/socks_handler.h"
#include "../include/socks_nio.h"


void
socks_read(struct selector_key *key) {
    struct state_machine *stm = &ATTACHMENT(key)->stm;
    const enum socks_state st = stm_handler_read(stm, key);

    if (ERROR == st || DONE == st) {
        socks_done(key);
    }
}

void
socks_write(struct selector_key *key) {
    struct state_machine *stm = &ATTACHMENT(key)->stm;
    const enum socks_state st = stm_handler_write(stm, key);

    if (ERROR == st || DONE == st) {
        socks_done(key);
    }
}

void
socks_block(struct selector_key *key) {
    struct state_machine *stm = &ATTACHMENT(key)->stm;
    const enum socks_state st = stm_handler_block(stm, key);

    if (ERROR == st || DONE == st) {
        socks_done(key);
    }
}

void
socks_close(struct selector_key *key) {
    socks_destroy(ATTACHMENT(key));
}

void
socks_done(struct selector_key *key) {
    const int fds[] = {
            ATTACHMENT(key)->client_fd,
            ATTACHMENT(key)->origin_fd,
    };
    for (unsigned i = 0; i < N(fds); i++) {
        if (fds[i] != -1) {
            if (SELECTOR_SUCCESS != selector_unregister_fd(key->s, fds[i])) {
                abort();
            }
            close(fds[i]);
        }
    }
}

