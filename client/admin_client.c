#include "include/admin_client.h"

int main() {
    struct sockaddr_in client_address;
    struct admin_client *client = calloc(1, sizeof(struct admin_client));
    struct read *read_message = calloc(1, sizeof(struct read));

    int upd_socket_type = SOCK_DGRAM;
    client_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    client_address.sin_port = htons(PORT);
    client_address.sin_family = AF_INET;

    if ((client->fd = socket(AF_INET, upd_socket_type, 0)) < 0) {
        printf("Error creating client socket");
        exit(0);
    }
    if (connect(client->fd, (struct sockaddr *) &client_address, sizeof(client_address)) < 0) {
        printf("Error connecting client socket");
        exit(0);
    }
    printf("Connection with server successfully established.\n");
    printf("Insert 'quit' to close the connection or <password> HELP to get available commands.\n");
    while(1) {
        if ((read_message->amount = (int) read(STDIN_FILENO, read_message->buffer, BUFF_SIZE)) < 0) {
            printf("Nothing to read");
            close(client->fd);
        }
        if(strcmp("quit\n", read_message-> buffer) == 0 || strcmp("QUIT\n", read_message-> buffer) == 0){
            break;
        }
        ssize_t sent;
        sent = sendto(client->fd, read_message->buffer, read_message->amount, 0, (struct sockaddr *) NULL,
                      sizeof(client_address));
        if (sent < 0) {
            fprintf(stderr, "sendto() failed.\n");
        } else if (sent != read_message->amount) {
            fprintf(stderr, "sendto() sent unexpected number of bytes.\n");
        }
        errno = 0;
        ssize_t valread = recvfrom(client->fd, client->buffer, sizeof(client->buffer), 0, (struct sockaddr *) NULL,
                                   NULL);
        if (valread <= 0) {
            fprintf(stderr, "recvfrom() failed: %s\n", strerror(errno));
            exit(1);
        } else {
            printf("%s", client->buffer);
            memset(client->buffer, 0, valread);
            memset(read_message->buffer, 0, sent);
        }
    }
    free(read_message);
    free(client);
    return 0;
}
