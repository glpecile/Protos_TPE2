#include "../include/admin.h"

#define BUFF_SIZE 400

int validate_password(char *to_parse, struct admin *admin_commands);

int validate_options(char *to_parse);

void validate_sendto(ssize_t sent, ssize_t to_send);

int parse_get_set_arguments(char *to_parse, char *to_fill);

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
        char *to_print = "-ERR\tUNAUTHORIZED (INVALID AUTH_ID)\n";
        bytes_to_send = (int) strlen(to_print);
        num_bytes_sent = sendto(key->fd, to_print, bytes_to_send, 0, (struct sockaddr *) &clntAddr, clntAddrLen);
        validate_sendto(num_bytes_sent, bytes_to_send);
    } else {
        switch (validate_options(to_parse)) {
            case HELP: {
                char *to_print = "+OK\n"
                                 "~~~~ ADMIN HELP ~~~~\n"
                                 " - GET_BUFF_SIZE\n"
                                 " - GET_STATS\n"
                                 " - GET_CURRENT_CON\n"
                                 " - SET_AUTH\n"
                                 " - SET_MEM_SPACE\n"
                                 " - SET_TIMEOUT\n"
                                 " - HELP\n";
                bytes_to_send = (int) strlen(to_print);
                num_bytes_sent = sendto(key->fd, to_print, bytes_to_send, 0, (struct sockaddr *) &clntAddr,
                                        clntAddrLen);
                validate_sendto(num_bytes_sent, bytes_to_send);
            }
                break;
            case GET_BUFF_SIZE: {
                int buff_size = BUFF_SIZE;
                char *to_print = calloc(1, 20 * sizeof(char));
                sprintf(to_print, "+OK\t %d\n", buff_size);
                bytes_to_send = (int) strlen(to_print);
                num_bytes_sent = sendto(key->fd, to_print, bytes_to_send, 0, (struct sockaddr *) &clntAddr,
                                        clntAddrLen);
                validate_sendto(num_bytes_sent, bytes_to_send);
                free(to_print);
            }
                break;
            case GET_STATS: {
                if (stats == NULL) { //no deberia llegar aca pues en main.c se verifica esto
                    char *to_print = "-ERR\tUNKNOWN ERROR\n";
                    bytes_to_send = (int) strlen(to_print);
                    num_bytes_sent = sendto(key->fd, to_print, bytes_to_send, 0, (struct sockaddr *) &clntAddr,
                                            clntAddrLen);
                    validate_sendto(num_bytes_sent, bytes_to_send);
                } else {
                    char *to_print = calloc(1, 100 * sizeof(char));
                    sprintf(to_print, "+OK\t%d\t%d\t%d\n", stats->historic_connections,
                            stats->curent_connections, stats->bytes_transfered);
                    bytes_to_send = (int) strlen(to_print);
                    num_bytes_sent = sendto(key->fd, to_print, bytes_to_send, 0, (struct sockaddr *) &clntAddr,
                                            clntAddrLen);
                    validate_sendto(num_bytes_sent, bytes_to_send);
                    free(to_print);
                }
            }
                break;
            case SET_AUTH: {
                char *parameter = calloc(1, sizeof(char) * 30);
                int length = parse_get_set_arguments(to_parse, parameter);
                if (length != PASS_LEN) {
                    char *to_print = "-ERR\tINCORRECT ARGUMENT FOR COMMAND\n";
                    bytes_to_send = (int) strlen(to_print);
                    num_bytes_sent = sendto(key->fd, to_print, bytes_to_send, 0, (struct sockaddr *) &clntAddr,
                                            clntAddrLen);
                    validate_sendto(num_bytes_sent, bytes_to_send);
                } else {
                    set_admin_password(admin_commands, parameter);
                    char *to_print = "-OK\n";
                    bytes_to_send = (int) strlen(to_print);
                    num_bytes_sent = sendto(key->fd, to_print, bytes_to_send, 0, (struct sockaddr *) &clntAddr,
                                            clntAddrLen);
                    validate_sendto(num_bytes_sent, bytes_to_send);
                }
                free(parameter);
            }
                break;
            default: {
                char *to_print = "-ERR\tUNKNOWN/UNSUPPORTED COMMAND\n";
                bytes_to_send = (int) strlen(to_print);
                num_bytes_sent = sendto(key->fd, to_print, bytes_to_send, 0, (struct sockaddr *) &clntAddr,
                                        clntAddrLen);
                validate_sendto(num_bytes_sent, bytes_to_send);
            }
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
    } else if (strcmp("SET_AUTH", option) == 0) {
        return SET_AUTH;
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

int parse_get_set_arguments(char *to_parse, char *to_fill) {
    int offset = (int) (strrchr(to_parse, ' ') - to_parse);
    if (offset <= 0) return -1;
    offset++; //pues esta en ' '
    int i = 0;
    while (to_parse[offset] != '\n' && to_parse[offset] != '\0') {
        to_fill[i++] = to_parse[offset++];
    }
    to_fill[i] = '\0';
    return i;
}
