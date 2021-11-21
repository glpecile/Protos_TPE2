#include <assert.h>
#include <stdlib.h>

#include "../include/copy_events.h"
#include "../include/socks_nio.h"

/**
 * Computa los intereses en base a la disponibilidad de los buffer.
 * La variable duplex nos permite saber si alguna vía ya fue cerrada.
 * Arranca OP_READ | OP_WRITE.
 */
static fd_interest
copy_compute_interests(fd_selector s, struct copy *d) {
    fd_interest ret = OP_NOOP;

    if ((d->duplex & OP_READ) && buffer_can_write(d->rb)) {
        ret |= OP_READ;
    }

    if ((d->duplex & OP_WRITE) && buffer_can_read(d->wb)) {
        ret |= OP_WRITE;
    }

    if (SELECTOR_SUCCESS != selector_set_interest(s, *d->fd, ret)) {
        abort();
    }

    return ret;
}

/**
 * Elige la estructura de copia correcta de cada fd (origin o client).
 */
static struct copy *
copy_ptr(struct selector_key *key) {
    struct copy *d = &ATTACHMENT(key)->client.copy;

    if (*d->fd == key->fd) {
        //OK

    } else {
        d = d->other;
    }

    return d;
}

/**
 *
 */
void
copy_init(const unsigned state, struct selector_key *key) {
    struct copy *d = &ATTACHMENT(key)->client.copy;

    d->fd = &ATTACHMENT(key)->client_fd;
    d->rb = &ATTACHMENT(key)->read_buffer;
    d->wb = &ATTACHMENT(key)->write_buffer;
    d->duplex = OP_READ | OP_WRITE;
    d->other = &ATTACHMENT(key)->orig.copy;

    d = &ATTACHMENT(key)->orig.copy;
    d->fd = &ATTACHMENT(key)->origin_fd;
    d->rb = &ATTACHMENT(key)->write_buffer;
    d->wb = &ATTACHMENT(key)->read_buffer;
    d->duplex = OP_READ | OP_WRITE;
    d->other = &ATTACHMENT(key)->client.copy;
}

/**
 * Lee bytes de un socket y los encola para ser escritos en otro socket
 */
unsigned
copy_r(struct selector_key *key) {
    struct copy *d = copy_ptr(key);

    assert(*d->fd == key->fd);

    size_t size;
    ssize_t n;
    buffer *b = d->rb;
    unsigned ret = COPYING;

    uint8_t *ptr = buffer_write_ptr(b, &size);
    n = recv(key->fd, ptr, size, 0);
    if (n <= 0) {
        shutdown(*d->fd, SHUT_RD);
        d->duplex &= -OP_READ;
        if (*d->other->fd != -1) {
            shutdown(*d->other->fd, SHUT_WR);
            d->other->duplex &= -OP_WRITE;
        }
    } else {
        buffer_write_adv(b, n);
    }

    copy_compute_interests(key->s, d);
    copy_compute_interests(key->s, d->other);

    if (d->duplex == OP_NOOP) {
        ret = DONE;
    }

    return ret;
}

/**
 * Escribe bytes encolados
 */
unsigned
copy_w(struct selector_key *key) {
    struct copy *d = copy_ptr(key);

    assert(*d->fd == key->fd);

    size_t size;
    ssize_t n;
    buffer *b = d->wb;
    unsigned ret = COPYING;

    uint8_t *ptr = buffer_read_ptr(b, &size);
    n = send(key->fd, ptr, size, MSG_NOSIGNAL);
    if (n == -1) {
        shutdown(*d->fd, SHUT_WR);
        d->duplex &= -OP_WRITE;
        if (*d->other->fd != -1) {
            shutdown(*d->other->fd, SHUT_RD);
            d->other->duplex &= -OP_READ;
        }
    } else {
        buffer_read_adv(b, n);
    }

    copy_compute_interests(key->s, d);
    copy_compute_interests(key->s, d->other);

    if (d->duplex == OP_NOOP) {
        ret = DONE;
    }

    return ret;
}
