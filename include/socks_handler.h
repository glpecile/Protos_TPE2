#ifndef PROTOS_TP2E_SOCKS_HANDLER_H
#define PROTOS_TP2E_SOCKS_HANDLER_H

#include "selector.h"

void socks_done(struct selector_key *key);

void socks_close(struct selector_key *key);

void socks_block(struct selector_key *key);

void socks_write(struct selector_key *key);

void socks_read(struct selector_key *key);

#endif //PROTOS_TP2E_SOCKS_HANDLER_H
