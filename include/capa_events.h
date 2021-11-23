#ifndef PROTOS_TP2E_CAPA_EVENTS_H
#define PROTOS_TP2E_CAPA_EVENTS_H

#include "selector.h"
#include "buffer.h"

/** Estructura utilizada para el envio y response del comando CAPA al origin **/
struct capa {
    /** Buffer para la respuesta del origen */
    buffer *res;
    /**Flag para indicar si el origin server acepta pipelining**/
    bool pipelining;

    bool first_cr;
    bool first_lf;
    bool dot;
    bool second_cr;
    bool capa_finished;
};

void capa_init(const unsigned state, struct selector_key *key);

unsigned capa_read(struct selector_key *key);

unsigned capa_send(struct selector_key *key);

#endif //PROTOS_TP2E_CAPA_EVENTS_H
