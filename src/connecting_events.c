#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>


#include "../include/connecting_events.h"
#include "../include/socks_nio.h"

unsigned
connecting(struct selector_key *key) {
    printf("CONNECTING");
    //TODO set domain after name resolution
    /**Socket dedicado al origin server. Por cada cliente hay un socket dedicado al origin**/
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    unsigned ret = COPYING;
    //Cacheamos el error de socket()
    if(sock < 0){
        perror("socket creation failed.");
        return ERROR;
    }
    //Para que no sea bloqueante.
    if(selector_fd_set_nio(sock) == -1){
        goto error;
    }

    //TODO esto luego se debe obtener de la resolucioon de nombres.-
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockaddr.sin_port = 8080;

    if(connect(sock,(const struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1){
        if(errno == EINPROGRESS){
            //Al ser no bloqueante si todavia no establecio la conexion... OP_NOOP->sin un interes en particular
            selector_status st = selector_set_interest_key(key, OP_NOOP);
            if(st != SELECTOR_SUCCESS){
                goto error;
            }
            //registramos la conexion en el selector como WRITE (como indican las buenas practicas)
            st = selector_register(key->s,sock,&handler,OP_WRITE, key->data);
            if(st != SELECTOR_SUCCESS){
                goto error;
            }
            //indicamos que ahora hay otra conexion que depende de este estado.
            ATTACHMENT(key)->origin_fd = sock;
            ATTACHMENT(key)->references += 1;

        }else{
            goto error;
        }
    }else{
        abort();
    }

    return ret;

    error:
    ret = ERROR;
    fprintf(stdout,"Connecting to origin server failed.\n");

    if(sock != -1) {
        close(sock);
    }

    return ret;
}
