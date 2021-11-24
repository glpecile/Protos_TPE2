
#include "../include/response_events.h"
#include "../include/socks_nio.h"

#include <stdio.h>

static void check_char_multiline(struct response *g, char write) {
    switch (write) {
        case CR:
            if (!g->first_cr) {
                g->first_cr = true;
            } else {
                g->second_cr = g->dot;
            }
            break;
        case LF:
            if (!g->first_lf) {
                g->first_lf = g->first_cr;
            } else {
                g->response_finished = g->second_cr;
            }
            break;
        case DOT:
            g->dot = g->first_lf;
            break;
        default:
            g->first_cr = g->first_lf = g->dot = g->second_cr = false;
            break;
    }
}

static void check_char_singleline(struct response *g, char write) {
    switch (write) {
        case CR:
            g->first_cr = true;
            break;
        case LF:
            g->response_finished = g->first_cr;
            break;
        default:
            g->first_cr = false;
            break;

    }
}

static void check_char(struct response *g, char write) {
    if (g->next_cmd_multiline) {
        check_char_multiline(g, write);
    } else {
        check_char_singleline(g, write);
    }
}

void response_init(const unsigned state, struct selector_key *key) {
    struct response *response = &ATTACHMENT(key)->orig.response;
    response->first_lf = false;
    response->first_cr = false;
    response->dot = false;
    response->second_cr = false;
    response->response_finished = false;
    response->next_cmd_multiline = false;
    response->res = &ATTACHMENT(key)->write_buffer;
    response->command_id = -1;
}

unsigned response_read(struct selector_key *key) {
    struct response *response = &ATTACHMENT(key)->orig.response;
    unsigned ret = RESPONSE_ST; //En principio me quedo en este estado hasta que termine de enviar toda la respuesta recibida.

    ssize_t bytes_read;
    size_t size_can_write;
    if (!buffer_can_write(response->res)) {
        //Flushing de lo que tengo.
        selector_status ss = SELECTOR_SUCCESS;
        ss |= selector_set_interest_key(key, OP_NOOP);
        ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_WRITE);
        ret = SELECTOR_SUCCESS == ss ? ret : ERROR_ST;
        return ret;
    }
    uint8_t *write = buffer_write_ptr(response->res, &size_can_write);

    bytes_read = recv(ATTACHMENT(key)->origin_fd, (char *) write, size_can_write, 0);
    if (bytes_read <= 0) {
        ret = ERROR_ST;
    } else {
        ssize_t count = 0;
        struct cmd cmd;
        int dq = dequeue(ATTACHMENT(key)->client.request.cmd_queue, &cmd);
        while (count < bytes_read && dq) {
            response->next_cmd_multiline = cmd.multiline;
            response->command_id = cmd.cmd_id;
            check_char(response, (char) *write);
            count++;
            write++;
            if (response->response_finished) {
                buffer_write_adv(response->res, count);
                //cambio de interes en el selector para ir a escribirle al cliente
                selector_status ss = SELECTOR_SUCCESS;
                ss |= selector_set_interest_key(key, OP_NOOP);
                ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_WRITE);
                ret = SELECTOR_SUCCESS == ss ? ret : ERROR_ST;
                dq = dequeue(ATTACHMENT(key)->client.request.cmd_queue, &cmd);
            }
        }
    }
    return ret;
}

unsigned response_send(struct selector_key *key) {

    struct response *response = &ATTACHMENT(key)->orig.response;
    unsigned ret = RESPONSE_ST;//En principio si sale bien vamos al copying

    ssize_t bytes_write;
    size_t size_to_read;
    if (!buffer_can_read(response->res)) {
        //vuelvo al response read para ver si quedan cosas por leer.
        ret = ERROR_ST;
        log(ERROR, "%s", "No se puede escribir");
    } else {
        uint8_t *read = buffer_read_ptr(response->res, &size_to_read);
        bytes_write = send(ATTACHMENT(key)->client_fd, read, size_to_read, 0);
        if (bytes_write <= 0) {
            ret = ERROR_ST;
            log(ERROR, "%s", "Error when trying to send bytes to client\n")
        } else {
            buffer_read_adv(response->res, bytes_write);
            //cambio de intereses para volver al request

            //cambio de interes en el selector
            selector_status ss = SELECTOR_SUCCESS;
            //dejo el origin en read por el response state
            ss |= selector_set_interest_key(key, OP_NOOP);
            int fd = response->response_finished ? ATTACHMENT(key)->client_fd : ATTACHMENT(key)->origin_fd;
            ret = response->response_finished ? REQUEST_ST : RESPONSE_ST;
            ss |= selector_set_interest(key->s, fd, OP_READ);
            ret = SELECTOR_SUCCESS == ss ? ret : ERROR_ST;
        }
    }
    return ret;
}
