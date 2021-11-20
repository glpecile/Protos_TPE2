#ifndef PROTOS_TP2E_HELLO_EVENTS_H
#define PROTOS_TP2E_HELLO_EVENTS_H

/**
 * Definición de variables para cada estado
 */

/** usado por HELLO_READ, HELLO_WRITE */
struct hello_st {
    /** buffer utilizado para I/O */
    buffer *rb, *wb;
    struct hello_parser parser;
    /** el método de autenticación seleccionado */
    uint8_t method;
};

void hello_read_init(const unsigned state, struct selector_key *key);

unsigned hello_read(struct selector_key *key)

unsigned hello_write(struct selector_key *key);



#endif //PROTOS_TP2E_HELLO_EVENTS_H
