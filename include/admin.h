#ifndef UDP_ADMIN
#define UDP_ADMIN

#include "selector.h"
#include <stdint.h>
#include <stddef.h>

void udp_read(struct selector_key *key);

void udp_write(struct selector_key *key);

#endif
