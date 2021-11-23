#ifndef PROTOS_TP2E_GREETINGS_EVENTS_H
#define PROTOS_TP2E_GREETINGS_EVENTS_H

#include <stdbool.h>

#include "buffer.h"
#include "selector.h"

struct greetings{
    buffer * buffer;
    bool cr_received;
    bool greet_finished;
};

void greetings_init(const unsigned state, struct selector_key *key);

unsigned greetings_read(struct selector_key *key);

unsigned greetings_write(struct selector_key *key);

#endif //PROTOS_TP2E_GREETINGS_EVENTS_H
