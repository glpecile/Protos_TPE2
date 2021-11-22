#include "../include/admin.h"

#define BUFF_SIZE 100

const char *execute(char *string) {
    return "bitte\n";
}

void udp_read(struct selector_key *key) {
    struct sockaddr_storage clntAddr;            // Client address
    socklen_t clntAddrLen = sizeof(clntAddr);
    char buffer[BUFF_SIZE] = {0};
    errno = 0;
    ssize_t valread = recvfrom(key->fd, buffer, BUFF_SIZE, 0, (struct sockaddr *) &clntAddr, &clntAddrLen);
    if (valread <= 0) {
        fprintf(stderr, "recvfrom() failed: %s\n", strerror(errno));
        exit(1);
    } else {
        printf("Parsear y hacer algo\n");
        selector_set_interest_key(key, OP_WRITE);
    }
}

void udp_write(struct selector_key *key) {
    struct sockaddr_storage clntAddr;            // Client address
    socklen_t clntAddrLen = sizeof(clntAddr);

    char buffer[BUFF_SIZE] = {0};
    const char *answer;
    answer = execute(buffer);
    int bytes_to_send = strlen(answer);
    ssize_t numBytesSent = sendto(key->fd, answer, bytes_to_send, 0, (struct sockaddr *) &clntAddr, clntAddrLen);
    //reset parser
    if (numBytesSent < 0) {
        fprintf(stderr, "sendto() failed.\n");
        exit(1);
    } else if (numBytesSent != bytes_to_send) {
        fprintf(stderr, "sendto() sent unexpected number of bytes.\n");
        exit(1);
    }
//    }
    selector_set_interest_key(key, OP_READ);
}
