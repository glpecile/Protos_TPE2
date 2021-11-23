#include "../include/admin.h"

#define BUFF_SIZE 400

int validate_password(char *to_parse, struct admin *admin_commands);

int validate_options(char *to_parse);

void validate_sendto(ssize_t sent, ssize_t to_send);

int validate_get_parameters(char *to_parse);

void udp_read(struct selector_key *key) {
    struct admin *admin_commands = ATTACHMENT_ADMIN(key);
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);
    size_t size_can_write;
    uint8_t *read = buffer_write_ptr(&admin_commands->read_buffer, &size_can_write);
    errno = 0;
    ssize_t valread = recvfrom(key->fd, (char *) read, BUFF_SIZE, 0, (struct sockaddr *) &clntAddr, &clntAddrLen);
    if (valread <= 0) {
        fprintf(stderr, "recvfrom() failed: %s\n", strerror(errno));
        exit(1);
    } else {
        buffer_write_adv(&admin_commands->read_buffer, valread);
        printf("Parsear y hacer algo\n");
        selector_set_interest_key(key, OP_WRITE);
    }
}

void udp_write(struct selector_key *key) {
    struct admin *admin_commands = ATTACHMENT_ADMIN(key);
    struct sockaddr_storage clntAddr;            // Client address
    socklen_t clntAddrLen = sizeof(clntAddr);
    size_t size_can_read;
    char *to_parse = (char *) buffer_read_ptr(&admin_commands->read_buffer, &size_can_read);
    int bytes_to_send = (int) size_can_read;
    ssize_t num_bytes_sent = 0;
    if (validate_password(to_parse, admin_commands) != 0) {
        char *to_print = "200 UNAUTHORIZED (INVALID AUTH_ID)\n";
        bytes_to_send = (int) strlen(to_print);
        num_bytes_sent = sendto(key->fd, to_print, bytes_to_send, 0, (struct sockaddr *) &clntAddr, clntAddrLen);
        validate_sendto(num_bytes_sent, bytes_to_send);
    } else {
        switch (validate_options(to_parse)) {
            case HELP: {
                char *to_print = "~~ ADMIN HELP ~~\n"
                                 "GET STATS \n"
                                 "GET CURRENT_CON \n"
                                 "SET PASS  \n"
                                 "SET MEM_SPACE \n"
                                 "SET TIMEOUT\n"
                                 "HELP \n";
                bytes_to_send = (int) strlen(to_print);
                num_bytes_sent = sendto(key->fd, to_print, bytes_to_send, 0, (struct sockaddr *) &clntAddr,
                                        clntAddrLen);
                validate_sendto(num_bytes_sent, bytes_to_send);
            }
            break;
            case GET_BUFF_SIZE: {
                int a = 1;
                printf("%d\n", a);
            }
            break;
            case GET_STATS: {
                int a = 1;
                printf("%d\n", a);
            }
            break;
            case GET_CURRENT_CON: {
                int a = 1;
                printf("%d\n", a);
            }
            break;
            case SET_TIMEOUT: {
                int a = 1;
                printf("%d\n", a);
            }
            break;
            case SET_MEM_SPACE: {
                int a = 1;
                printf("%d\n", a);
            }
            break;
            case SET_AUTH: {
                int a = 1;
                printf("%d\n", a);
            }
            break;
            case NONE: {
                char *to_print = "300 UNKNOWN/UNSUPPORTED COMMAND\n";
                bytes_to_send = (int) strlen(to_print);
                num_bytes_sent = sendto(key->fd, to_print, bytes_to_send, 0, (struct sockaddr *) &clntAddr,
                                        clntAddrLen);
                validate_sendto(num_bytes_sent, bytes_to_send);
            }
            break;
            default:
                break;
        }


    }
//    buffer_read_adv(&admin_commands->read_buffer, num_bytes_sent);
    buffer_reset(&admin_commands->read_buffer);
    selector_set_interest_key(key, OP_READ);
}

void set_admin_password(struct admin *admin, char new_password[6]) {
    strcpy(admin->password, new_password);
}

int validate_password(char *to_parse, struct admin *admin_commands) {
    int i = 0;
    while (to_parse[i] != ' ') {
        if (admin_commands->password[i] != to_parse[i] || i >= PASS_LEN || to_parse[i] == '\0' || to_parse[i] == '\n') {
            return -1;
        }
        i++;
    }
    if (i != PASS_LEN) return -1;
    return 0;
}

int parse_options(char *to_parse, char *to_save, int offset) {
    int i = offset, j = 0;
    while (to_parse[i] != ' ' && to_parse[i] != '\0' && to_parse[i] != '\n') {
        to_save[j++] = to_parse[i++];
    }
    to_save[j] = '\0';
    return j;
}

enum options validate_options(char *to_parse) {
    int offset = PASS_LEN + 1;
    char option[30] = {0};
    //int parsed = ;
    parse_options(to_parse, option, offset);
    if (strcmp("HELP", option) == 0) {
        return HELP;
    } else if (strcmp("GET_BUFF_SIZE", option) == 0) {
        return GET_BUFF_SIZE;
    } else if (strcmp("GET_STATS", option) == 0) {
        return GET_STATS;
    } else if (strcmp("GET_CURRENT_CON", option) == 0) {
        return GET_CURRENT_CON;
    } else if (strcmp("SET_AUTH", option) == 0) {
        return SET_AUTH;
    } else if (strcmp("SET_MEM_SPACE", option) == 0) {
        return SET_MEM_SPACE;
    } else if (strcmp("SET_TIMEOUT", option) == 0) {
        return SET_TIMEOUT;
    }
    return NONE;
}


void validate_sendto(ssize_t sent, ssize_t to_send) {
    if (sent < 0) {
        fprintf(stderr, "sendto() failed.\n");
        exit(1);
    } else if (sent != to_send) {
        fprintf(stderr, "sendto() sent unexpected number of bytes.\n");
        exit(1);
    }
}
