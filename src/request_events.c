#include <ctype.h>
#include <string.h>
#include <strings.h>
#include "../include/request_events.h"
#include "../include/socks_nio.h"

static void check_char(struct request *r, char write) {
    switch (write) {
        case CR:
            r->cr = true;
            break;
        case LF:
            r->lf = true;
            break;
        default:
            r->cr = r->lf = false;
            break;
    }
    r->request_finished = r->cr && r->lf;
}

static void
parse_cmd(struct cmd *cmd) {
    cmd->cmd_id = COMMON;
    cmd->multiline = false;
    switch (toupper(cmd->cmd[0])) {
        case 'R':
            if (strncasecmp(cmd->cmd, "RETR", 4) == 0) {
                cmd->cmd_id = RETR;
                cmd->multiline = true;
            }
            break;
        case 'Q':
            if (strncasecmp(cmd->cmd, "QUIT", 4) == 0) {
                cmd->cmd_id = QUIT;
            }
            break;
        case 'L':
            if (cmd->cmd_size == 4 && strncasecmp(cmd->cmd, "LIST", 4) == 0) {
                cmd->multiline = true;
            }
            break;
        case 'C':
            if (strncasecmp(cmd->cmd, "CAPA", 4) == 0) {
                cmd->multiline = true;
            }
            break;
    }
}

void
request_init(const unsigned state, struct selector_key *key) {
    struct request *request = &ATTACHMENT(key)->client.request;
    request->cr = false;
    request->lf = false;
    request->request_finished = false;
    request->req = &ATTACHMENT(key)->read_buffer;
    buffer_reset(request->req);
    request->cmd_queue = new_queue();//TODO revisar tema memoria.
}

/**
 * Lee de a 1 byte los char que recibe del cliente. Separa los comandos por \n o por \r\n
 * Una vez que tenga el comando separado, lo guarda en la cola.
 * Termina la funcion al recibir el CRLF, ahi cambia los intereses CLIENTE->NOOP y ORIGIN->OP_WRITE
 *  Tomar en consideracion que a la queue se agrega unicamente el comando, sin el CRCLF O LF
 */
unsigned
request_read(struct selector_key *key) {
    struct request *request = &ATTACHMENT(key)->client.request;
    unsigned ret = REQUEST_ST; //En principio sigo en REQUEST_ST y con los mismos intereses.

    ssize_t bytes_read;
    size_t size_can_write;
    uint8_t * write = buffer_write_ptr(request->req, &size_can_write);
    bytes_read = recv(ATTACHMENT(key)->client_fd, (char *) write, size_can_write, 0);
    if (bytes_read <= 0) {
        ret = ERROR_ST;
    } else {
//        check_buffer(request,write,bytes_read);
        bool flag = false;
        size_t i;
        buffer_write_adv(request->req,bytes_read);
        uint8_t c;
        uint8_t * br_first = buffer_read_ptr(request->req,&i);
        int size = 0;
        while((c = buffer_read(request->req))){
            check_char(request, (char) c);
            flag = request->lf || request->request_finished;
            size ++;
            if(flag){
                struct cmd cmd;
                cmd.cmd_size = size + 1 - request->lf - request->cr;
                memcpy(cmd.cmd, br_first, cmd.cmd_size);
                parse_cmd(&cmd);
                queue(request->cmd_queue, cmd);
                if (request->request_finished || !buffer_can_write(request->req)) {
                    //cambio de interes en el selector
                    selector_status ss = SELECTOR_SUCCESS;
                    ss |= selector_set_interest_key(key, OP_NOOP);
                    ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_WRITE);
                    ret = SELECTOR_SUCCESS == ss ? ret : ERROR_ST;
                }
            }

        }
    }
    return ret;
}

unsigned
request_send(struct selector_key *key) {
    struct request *request = &ATTACHMENT(key)->client.request;
    unsigned ret = RESPONSE_ST; //En principio sigo en REQUEST_ST y con los mismos intereses.

    to_begin(request->cmd_queue);
    if (is_empty(request->cmd_queue)) {
        ret = ERROR_ST;
    }
    while (has_next(request->cmd_queue)) {
        struct cmd command = next(request->cmd_queue);
        if (send(ATTACHMENT(key)->origin_fd, command.cmd, command.cmd_size, 0) <= 0) {
            ret = ERROR_ST;
            return ret;
        }
        if (!ATTACHMENT(key)->orig.capa.pipelining || !has_next(request->cmd_queue)) {
            if (send(ATTACHMENT(key)->origin_fd, "\r", 1, 0) <= 0) {
                ret = ERROR_ST;
                return ret;
            }
        }
        if (send(ATTACHMENT(key)->origin_fd, "\n", 1, 0) <= 0) {
            ret = ERROR_ST;
            return ret;
        }
    }
    //cambio de interes en el selector
    selector_status ss = SELECTOR_SUCCESS;
    //dejo el origin en read por el response state
    ss |= selector_set_interest_key(key, OP_READ);
    ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_NOOP);
    ret = SELECTOR_SUCCESS == ss ? ret : ERROR_ST;
    return ret;
}
