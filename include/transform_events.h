
#ifndef PROTOS_TP2E_TRANSFORM_EVENTS_H
#define PROTOS_TP2E_TRANSFORM_EVENTS_H

#include "buffer.h"
#include "socks_nio.h"

struct transform{
    /** File Descriptors de los pipes */
    int read_fd;
    int write_fd;

    /** Buffers */
    buffer read_buffer;
    uint8_t read_buffer_space[BUFFER_SIZE];
    buffer write_buffer;
    uint8_t write_buffer_space[BUFFER_SIZE];
};

void transform_init(const unsigned state, struct selector_key *key);

unsigned transform_read(struct selector_key *key);

unsigned transform_send(struct selector_key *key);
#endif //PROTOS_TP2E_TRANSFORM_EVENTS_H
