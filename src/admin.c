#include "../include/admin.h"

#define BUFF_SIZE 400

current_command current;

char *get_response() {
    if (current->command == START) {
        return "start\n";
    } else if (current->command == GET_STATS) {
        return "get stats\n";
    } else if (current->command == GET_CURRENT_CON) {
        return "get current con\n";
    } else if (current->command == SET_AUTH) {
        return "set auth\n";
    } else if (current->command == SET_MEM_SPACE) {
        return "set mem space\n";
    } else if (current->command == SET_TIMEOUT) {
        return "set timeout\n";
    } else if (current->command == HELP) {
        return "help\n";
    }
    return "Invalid command. Try typing 'HELP' to get more information\n"
           "Keep in mind commands are case sensitive\n";

}


void parse_udp(char *buffer, int length) {
    if (strcmp(buffer, "START\n") == 0) {
        current->command = START;
    } else if (strcmp(buffer, "GET STATS\n") == 0) {
        current->command = GET_STATS;
    } else if (strcmp(buffer, "GET CURRENT_CON\n") == 0) {
        current->command = GET_CURRENT_CON;
    } else if (strcmp(buffer, "SET AUTH\n") == 0) {
        current->command = SET_AUTH;
    } else if (strcmp(buffer, "SET MEM_SPACE\n") == 0) {
        current->command = SET_MEM_SPACE;
    } else if (strcmp(buffer, "SET TIMEOUT\n") == 0) {
        current->command = SET_TIMEOUT;
    } else if (strcmp(buffer, "HELP\n") == 0) {
        current->command = HELP;
    }
}

void udp_read(struct selector_key *key) {
    current = malloc(sizeof(*current));
    struct sockaddr_storage clntAddr;            // Client address
    socklen_t clntAddrLen = sizeof(clntAddr);
    char buffer[BUFF_SIZE] = {0};
    errno = 0;
    ssize_t valread = recvfrom(key->fd, buffer, BUFF_SIZE, 0, (struct sockaddr *) &clntAddr, &clntAddrLen);
    if (valread <= 0) {
        fprintf(stderr, "recvfrom() failed: %s\n", strerror(errno));
        exit(1);
    } else {
        parse_udp(buffer, (int) valread);
        printf("Parsear y hacer algo\n");
        selector_set_interest_key(key, OP_WRITE);
    }
}

void udp_write(struct selector_key *key) {
    struct sockaddr_storage clntAddr;            // Client address
    socklen_t clntAddrLen = sizeof(clntAddr);
    char answer[BUFF_SIZE];
    strcpy(answer,get_response());
    int bytes_to_send = (int) strlen(answer);
    ssize_t numBytesSent = sendto(key->fd, answer, bytes_to_send, 0, (struct sockaddr *) &clntAddr, clntAddrLen);
    //reset parser
    if (numBytesSent < 0) {
        fprintf(stderr, "sendto() failed.\n");
        exit(1);
    } else if (numBytesSent != bytes_to_send) {
        fprintf(stderr, "sendto() sent unexpected number of bytes.\n");
        exit(1);
    }
    selector_set_interest_key(key, OP_READ);
    free(current);
}
