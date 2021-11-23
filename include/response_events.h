
#ifndef PROTOS_TP2E_RESPONSE_EVENTS_H
#define PROTOS_TP2E_RESPONSE_EVENTS_H

#include "buffer.h"
#include "selector.h"
struct response{
    buffer * res;
    bool first_cr;
    bool first_lf;
    bool dot;
    bool second_cr;
    bool response_finished;
    bool next_cmd_multiline;

};

void response_init(const unsigned state, struct selector_key *key);

unsigned response_read(struct selector_key *key);

unsigned response_send(struct selector_key *key);

#endif //PROTOS_TP2E_RESPONSE_EVENTS_H
