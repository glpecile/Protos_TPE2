#include <string.h>
#include <stdio.h>

#include "../include/capa_events.h"
#include "../include/socks_nio.h"

#define PIPELINING "\r\nPIPELINING\r\n"

static void check_char(struct capa *g, char write) {
    switch (write) {
        case CR:
            if(!g->first_cr){
                g->first_cr = true;
            }else{
                g->second_cr = g->dot;
            }
            break;
        case LF:
            if(!g->first_lf){
                g->first_lf = g->first_cr;
            }else{
                g->capa_finished = g->second_cr;
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
void capa_init(const unsigned tate, struct selector_key *key) {
    struct capa *d = &ATTACHMENT(key)->orig.capa;

    d->pipelining = false;
    d->res = &ATTACHMENT(key)->write_buffer;//TODO chequear si corresponde
    d->first_cr = false;
    d->second_cr = false;
    d->dot = false;
    d->capa_finished = false;

}

/**
 * Se lee la respuesta al comando CAPA provista por el origin server.
 */
unsigned capa_read(struct selector_key *key) {
    printf("Entramos al capa_read\n");
    printf("Soy el fd->%i\n", key->fd);
    struct capa *d = &ATTACHMENT(key)->orig.capa;
    unsigned ret = COPYING_ST;
    //Variables necesarias para los buffers
    uint8_t *write; //puntero al buffer
    size_t size_can_write = 1; // se va a leer de a 1 byte
    ssize_t bytes_read;

    write = buffer_write_ptr(d->res,&size_can_write);
    bytes_read = recv(ATTACHMENT(key)->origin_fd,(char *) write, 1,0);

    if(bytes_read <= 0){
        ret = ERROR_ST;
    }else{
        check_char(d,(char) *write);
        if(d->capa_finished){
            //Hacemos la busqueda del string de pipelining y eso.
            //si encuentra el pipelining se setea el flag
            if(strstr((char*) write,PIPELINING) != NULL){
                printf("Pipelining encontrado\n");
                d->pipelining = true;
            }
            buffer_reset(d->res);
            selector_status ss = SELECTOR_SUCCESS;
            ss |= selector_set_interest_key(key, OP_NOOP);
            ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd,OP_READ);
            ret = SELECTOR_SUCCESS == ss ? ret : ERROR_ST;
            printf("Probando\n");
        }else{
            printf("Nunca entramos al capa_finished\n");
            ret = CAPA_ST;
        }
    }

    return ret;
}

/**
 * Se envia el comando CAPA al origen para determinar si acepta pipelining.
*/
unsigned capa_send(struct selector_key *key) {
    printf("Entramos al capa_send\n");
    printf("Soy el fd->%i\n", key->fd);
    const char *to_send = "CAPA\r\n";
    unsigned ret = CAPA_ST;

    if (send(ATTACHMENT(key)->origin_fd, to_send, strlen(to_send), 0) <= 0) {
        ret = ERROR_ST;
    } else {
        //seteamos el interes de este estado en read para leer la respuesta del origin
        selector_status ss = SELECTOR_SUCCESS;
        ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd,OP_READ);
        ret = SELECTOR_SUCCESS == ss ? ret : ERROR_ST;
        printf("Cambiamos de interes a %i\n", ret);
    }
    return ret;
}
