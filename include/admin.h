#ifndef UDP_ADMIN
#define UDP_ADMIN

#include "selector.h"
#include <stdint.h>
#include <stddef.h>
#include "args.h"
#include <sys/types.h>
#include <sys/socket.h>

enum command_enum {
    START, GET_STATS, GET_CURRENT_CON, SET_AUTH, SET_MEM_SPACE, SET_TIMEOUT, HELP
};

struct current_command {
    enum command_enum command;
};

typedef struct current_command *current_command;

extern current_command current;

void udp_read(struct selector_key *key);

void udp_write(struct selector_key *key);

#endif
