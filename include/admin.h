#ifndef UDP_ADMIN
#define UDP_ADMIN

#include "selector.h"
#include <stdint.h>
#include <stddef.h>
#include "args.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "./buffer.h"

#define BUFFER_SIZE 64

//
//enum command_enum {
//    START, GET_STATS, GET_CURRENT_CON, SET_AUTH, SET_MEM_SPACE, SET_TIMEOUT, HELP
//};
//
//struct current_command {
//    enum command_enum command;
//};
//
//typedef struct current_command *current_command;
//
//extern current_command current;

struct admin{
    /** Buffers */
    buffer read_buffer;
    uint8_t read_buffer_space[BUFFER_SIZE];
    buffer write_buffer;
    uint8_t write_buffer_space[BUFFER_SIZE];
};

#define ATTACHMENT_ADMIN(key) ((struct admin *)(key)->data)
#define N(x) (sizeof(x)/sizeof((x)[0]))

void udp_read(struct selector_key *key);

void udp_write(struct selector_key *key);

#endif
