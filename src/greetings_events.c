
#include "../include/greetings_events.h"
#include "../include/socks_nio.h"

static void check_char(struct greetings *g, char write) {
    switch (write) {
        case CR:
            g->cr_received = true;
            break;
        case LF:
            g->greet_finished = g->cr_received;
            break;
        default:
            g->cr_received = false;
            break;

    }
}

static unsigned set_to_interest(struct selector_key *key, unsigned ret, fd_interest interest) {
    selector_status ss = SELECTOR_SUCCESS;
    ss |= selector_set_interest_key(key, OP_NOOP);
    int fd;
    if (interest == OP_WRITE)
        fd = ATTACHMENT(key)->client_fd;
    else
        fd = ATTACHMENT(key)->origin_fd;
    ss |= selector_set_interest(key->s, fd, interest);
    return SELECTOR_SUCCESS == ss ? ret : ERROR_ST;
}

void
greetings_init(const unsigned state, struct selector_key *key) {
    struct greetings *greetings = &ATTACHMENT(key)->orig.greet;
    greetings->buffer = &ATTACHMENT(key)->read_buffer;
    greetings->cr_received = false;
}

/**
 * Leemos el mensaje greet del origen (luego de confirmada la conexion)
 * Guardamos el mensaje en el buffer, cuando se llegue a un \r\n se corta el guardado.
 */
unsigned
greetings_read(struct selector_key *key) {
    //Nos traemos la estructura necesaria para el saludo.
    struct greetings *greetings = &ATTACHMENT(key)->orig.greet;
    unsigned ret = GREETINGS_ST;
    //Estado actual dentro del server


    //Variables necesarias para los buffers
    uint8_t *write; //puntero al buffer
    size_t size_can_write = 1; // se va a leer de a 1 byte
    ssize_t bytes_read;


    if (!buffer_can_write(greetings->buffer)) {
        //pasar al estado de write para dumpear el buffer.
        ret = set_to_interest(key, ret, OP_WRITE);
    } else {
        //Le pedimos al buffer el puntero para escribir
        write = buffer_write_ptr((buffer *) greetings->buffer, &size_can_write);

        bytes_read = recv(key->fd, write, 1, 0);

        if (bytes_read <= 0) {
            ret = ERROR_ST;
        } else {
            check_char(greetings, (char) *write);
            if (greetings->greet_finished) {
                ret = set_to_interest(key, ret, OP_WRITE);
            }
            buffer_write_adv(greetings->buffer, bytes_read);
        }
    }

    return ret;

}

unsigned
greetings_write(struct selector_key *key) {
    //Nos traemos la estructura necesaria para el saludo.
    struct greetings *greetings = &ATTACHMENT(key)->orig.greet;
    unsigned ret = GREETINGS_ST;
    //Estado actual dentro del server

    //Variables necesarias para los buffers
    uint8_t *read; //puntero al buffer
    size_t size_can_read = 1;
    ssize_t bytes_write;

    if (!buffer_can_read(greetings->buffer)) {
        //pasar al estado de write para dumpear el buffer.
        ret = set_to_interest(key, ret, OP_READ);
    } else {
        //Le pedimos al buffer el puntero para escribir
        read = buffer_read_ptr((buffer *) greetings->buffer, &size_can_read);

        bytes_write = send(key->fd, read, size_can_read, 0);

        if (bytes_write <= 0) {
            ret = ERROR_ST;
        } else {
            if (greetings->greet_finished) {
                ret = COPYING_ST;
            }
            buffer_read_adv(greetings->buffer, bytes_write);
        }
    }

    return ret;
}



