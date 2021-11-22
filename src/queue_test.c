//
// Created by jarce on 22 nov. 2021.
//

#include "../include/cmd_queue.h"
#include <stdio.h>

int main(){

    queue_adt queue_ = new_queue();

    struct cmd command = {"hola", false};
    queue(queue_, command);

    struct cmd command2 = {"chau", false};
    queue(queue_, command2);


    to_begin(queue_);
    while(has_next(queue_)) {
        struct cmd command = next(queue_);
        printf("%s\n", command.cmd);
    }

    struct cmd cmd;

    dequeue(queue_, &cmd);
    printf("Dequeue: %s\n", cmd.cmd);
    dequeue(queue_, &cmd);
    printf("Dequeue: %s\n", cmd.cmd);

    to_begin(queue_);
    while(has_next(queue_)) {
        struct cmd command = next(queue_);
        printf("%s\n", command.cmd);
    }

//    free_queue(queue_);
}