#ifndef PROTOS_TP2E_SOCKS_NIO_H
#define PROTOS_TP2E_SOCKS_NIO_H

#include "./stm.h"

#define BUFFER_SIZE 64

void socks_passive_accept(struct selector_key *key);

#endif //PROTOS_TP2E_SOCKS_NIO_H
