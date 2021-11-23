
#include <stdio.h>
#include "../include/transform_events.h"
#include "../include/args.h"

#define READ 0
#define WRITE 1

static unsigned transformation(){
    int in[2];
    int out[2];

    if(pipe(in) == -1 || pipe(out) == -1) {
        perror("creating pipes");
        return -1;
    }

    const pid_t cmdpid = fork();
    if(cmdpid == -1){
        perror("creating process for user command");
        return -1;
    }else if(cmdpid == 0){
        //hijo
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(in[WRITE]);//write
        close(out[READ]);//read
        in[WRITE] = out[READ] = -1;
        dup2(in[READ], STDIN_FILENO);
        dup2(out[WRITE], STDOUT_FILENO);
        if(execl("/bin/sh","sh", "-c", parameters->transform_cmd, (char *) 0) == -1){
            perror("executing transform");
            close(in[READ]);
            close(out[WRITE]);
            return 1;
        }
        return 0;
    }else{
        close(in[READ]);
        close(out[WRITE]);
        in[READ] = out[WRITE] = -1;

        int fds[] = {STDIN_FILENO, STDOUT_FILENO, in[WRITE], out[READ]};

    }
}

void transform_init(const unsigned state, struct selector_key *key){
    struct transform * transform = &ATTACHMENT(key)->transform;

    buffer_init(&transform->read_buffer, N(transform->read_buffer_space), transform->read_buffer_space);
    buffer_init(&transform->write_buffer, N(transform->write_buffer_space), transform->write_buffer_space);

    transform->read_fd = -1;
    transform->write_fd = -1;


}

unsigned transform_read(struct selector_key *key);

unsigned transform_send(struct selector_key *key);