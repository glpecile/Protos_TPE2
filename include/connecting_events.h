#ifndef PROTOS_TP2E_CONNECTING_EVENTS_H
#define PROTOS_TP2E_CONNECTING_EVENTS_H

#include "buffer.h"
#include "selector.h"

/** Estructura utilizada para conectarse con el Origin Server **/
struct connecting {
    /** File descriptor */
    int *fd;
};

unsigned connecting(struct selector_key *key);

#endif //PROTOS_TP2E_CONNECTING_EVENTS_H

