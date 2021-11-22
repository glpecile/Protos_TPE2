#ifndef PROTOS_TP2E_DNS_RESOLUTION_EVENTS_H
#define PROTOS_TP2E_DNS_RESOLUTION_EVENTS_H

#include "selector.h"

unsigned dns_resolution(struct selector_key *key);

unsigned dns_resolution_done(struct selector_key *key);

#endif //PROTOS_TP2E_DNS_RESOLUTION_EVENTS_H
