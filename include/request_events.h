//
// Created by Roberto J Catalan on 23/11/2021.
//

#ifndef PROTOS_TP2E_REQUEST_EVENTS_H
#define PROTOS_TP2E_REQUEST_EVENTS_H

#include "buffer.h"
#include "cmd_queue.h"
#include "selector.h"

struct request{
    buffer * req;
    bool cr;
    bool lf;
    bool request_finished;
    queue_adt cmd_queue;
    //TODO agregar los parsers.
};

void request_init(const unsigned state, struct selector_key *key);

unsigned request_read(struct selector_key *key);

unsigned request_send(struct selector_key *key);

#endif //PROTOS_TP2E_REQUEST_EVENTS_H
