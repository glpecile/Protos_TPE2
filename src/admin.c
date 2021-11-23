#include "../include/admin.h"

#define BUFF_SIZE 400

//current_command current;
//
//char *get_response() {
//    if (current->command == START) {
//        return "start\n";
//    } else if (current->command == GET_STATS) {
//        return "get stats\n";
//    } else if (current->command == GET_CURRENT_CON) {
//        return "get current con\n";
//    } else if (current->command == SET_AUTH) {
//        return "set auth\n";
//    } else if (current->command == SET_MEM_SPACE) {
//        return "set mem space\n";
//    } else if (current->command == SET_TIMEOUT) {
//        return "set timeout\n";
//    } else if (current->command == HELP) {
//        return "help\n";
//    }
//    return "Invalid command. Try typing 'HELP' to get more information\n"
//           "Keep in mind commands are case sensitive\n";
//
//}


//void parse_udp(char *buffer, int length) {
//    if (strcmp(buffer, "START\n") == 0) {
//        current->command = START;
//    } else if (strcmp(buffer, "GET STATS\n") == 0) {
//        current->command = GET_STATS;
//    } else if (strcmp(buffer, "GET CURRENT_CON\n") == 0) {
//        current->command = GET_CURRENT_CON;
//    } else if (strcmp(buffer, "SET AUTH\n") == 0) {
//        current->command = SET_AUTH;
//    } else if (strcmp(buffer, "SET MEM_SPACE\n") == 0) {
//        current->command = SET_MEM_SPACE;
//    } else if (strcmp(buffer, "SET TIMEOUT\n") == 0) {
//        current->command = SET_TIMEOUT;
//    } else if (strcmp(buffer, "HELP\n") == 0) {
//        current->command = HELP;
//    }
//}
//admin *admincommands;
void udp_read(struct selector_key *key) {
    struct admin *admin_commands = ATTACHMENT_ADMIN(key);
//    admin *admin_commands = calloc(1,sizeof(admin));
//    if (admin_commands == NULL) {
//        return;
//    }

//    key->data = admin_commands;
    struct sockaddr_storage clntAddr;            // Client address
    socklen_t clntAddrLen = sizeof(clntAddr);
//    char buffer[BUFF_SIZE] = {0};
    size_t size_can_write;
    uint8_t *read = buffer_write_ptr(&admin_commands->read_buffer, &size_can_write);
    errno = 0;
    ssize_t valread = recvfrom(key->fd, (char *) read, BUFF_SIZE, 0, (struct sockaddr *) &clntAddr, &clntAddrLen);
    if (valread <= 0) {
        fprintf(stderr, "recvfrom() failed: %s\n", strerror(errno));
        exit(1);
    } else {
        buffer_write_adv(&admin_commands->read_buffer, valread);
//        parse_udp(buffer, (int) valread);
        printf("Parsear y hacer algo\n");
        selector_set_interest_key(key, OP_WRITE);
    }
}

void udp_write(struct selector_key *key) {
    struct admin *admin_commands = ATTACHMENT_ADMIN(key);
    struct sockaddr_storage clntAddr;            // Client address
    socklen_t clntAddrLen = sizeof(clntAddr);
//    strcpy(answer, get_response());
    size_t size_can_read;
    uint8_t *read = buffer_read_ptr(&admin_commands->read_buffer, &size_can_read);
    int bytes_to_send = (int) size_can_read;
    ssize_t numBytesSent = sendto(key->fd, (char *) read, bytes_to_send, 0, (struct sockaddr *) &clntAddr, clntAddrLen);
    //reset parser
    if (numBytesSent < 0) {
        fprintf(stderr, "sendto() failed.\n");
        exit(1);
    } else if (numBytesSent != bytes_to_send) {
        fprintf(stderr, "sendto() sent unexpected number of bytes.\n");
        exit(1);
    }
    buffer_read_adv(&admin_commands->read_buffer, numBytesSent);
    selector_set_interest_key(key, OP_READ);
}
