#ifndef PROTOS_TP2E_ADMIN_CLIENT_H
#define PROTOS_TP2E_ADMIN_CLIENT_H
#define BUFF_SIZE 1000
#define PORT 9090

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>  // socket
#include <errno.h>

struct admin_client {
    char buffer[BUFF_SIZE];
    int fd;
};

struct read {
    char buffer[BUFF_SIZE];
    int amount;
};

#endif //PROTOS_TP2E_ADMIN_CLIENT_H
