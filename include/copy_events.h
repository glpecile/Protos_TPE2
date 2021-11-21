#ifndef PROTOS_TP2E_COPY_EVENTS_H
#define PROTOS_TP2E_COPY_EVENTS_H

#include "buffer.h"
#include "selector.h"

/** Estructura utilizada para la copia de datos del cliente<->origin **/
struct copy {
    /** El otro file descriptor */
    int *fd;

    /** Bbuffer utilizado para I/O **/
    buffer *rb, *wb;

    /** ¿Cerramos ya la escritura o la lectura? */
    fd_interest duplex;

    struct copy *other;
};

/**
 *
 */
void copy_init(unsigned state, struct selector_key *key);

/**
 * Lee bytes de un socket y los encola para ser escritos en otro socket
 */
unsigned copy_r(struct selector_key *key);

/**
 * Escribe bytes encolados
 */
unsigned copy_w(struct selector_key *key);

#endif //PROTOS_TP2E_COPY_EVENTS_H
