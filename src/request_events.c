
#include <stdio.h>
#include <string.h>
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

void request_init(const unsigned state, struct selector_key *key){
    struct request * request = &ATTACHMENT(key)->client.request;
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
unsigned request_read(struct selector_key *key){
    printf("Entramos al request_read\n");
    printf("Soy el fd->%i\n", key->fd);

    struct request * request = &ATTACHMENT(key)->client.request;
    unsigned ret = REQUEST_ST; //En principio sigo en REQUEST_ST y con los mismos intereses.

    ssize_t bytes_read;

    if(!buffer_can_write(request->req)){
        //TODO administar buffer si se lleno esperando las request del cliente
        printf("no puedo escribir en el buffer\n");
    }

    uint8_t c;
    bytes_read = recv(ATTACHMENT(key)->client_fd, (char *) &c, 1, 0);
    if(bytes_read <= 0){
        ret = ERROR_ST;
    }else{
        check_char(request,(char) c);

        if(request->lf || request->request_finished){
            printf("request casi terminado\n");
            size_t size_can_read;
            uint8_t * read = buffer_read_ptr(request->req, &size_can_read);
            //quizas convenga avanzar el write en el buffer.
            *(read + size_can_read) = '\0';
            printf("la request es %s\n", read);
            struct cmd cmd;
            memcpy(cmd.cmd,read,size_can_read);
            queue(request->cmd_queue,cmd);
            if(request->request_finished){
                printf("request terminado\n");
                //cambio de interes en el selector
                selector_status ss = SELECTOR_SUCCESS;
                ss |= selector_set_interest_key(key,OP_NOOP);
                ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd,OP_WRITE);
                ret = SELECTOR_SUCCESS == ss ? ret : ERROR_ST;
            }
        }else{
            buffer_write(request->req,c);
        }
    }
    return ret;
}

unsigned request_send(struct selector_key *key){
    printf("Entramos al request_send\n");
    printf("Soy el fd->%i\n", key->fd);

    struct request * request = &ATTACHMENT(key)->client.request;
    unsigned ret = RESPONSE_ST; //En principio sigo en REQUEST_ST y con los mismos intereses.

    to_begin(request->cmd_queue);
    if(is_empty(request->cmd_queue)){
        ret = ERROR_ST;
    }
    while(has_next(request->cmd_queue)){
        struct cmd command = next(request->cmd_queue);
        if(send(ATTACHMENT(key)->origin_fd,command.cmd,MAX_CMD_SIZE,0)<=0){
            ret = ERROR_ST;
            return ret;
        }
    }
    //cambio de interes en el selector
    selector_status ss = SELECTOR_SUCCESS;
    //dejo el origin en read por el response state
    ss |= selector_set_interest_key(key,OP_READ);
    ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd,OP_NOOP);
    ret = SELECTOR_SUCCESS == ss ? ret : ERROR_ST;
    return ret;
}
