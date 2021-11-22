#ifndef UDP_ADMIN
#define UDP_ADMIN

#include "selector.h"
#include <stdint.h>
#include <stddef.h>
#include "args.h"
#include <sys/types.h>
#include <sys/socket.h>

void udp_read(struct selector_key *key);

void udp_write(struct selector_key *key);

#endif
