#ifndef PROTOS_TP2E_CMD_QUEUE_H
#define PROTOS_TP2E_CMD_QUEUE_H

#include <stdbool.h>

#define MAX_CMD_SIZE 47 //incluye el \r\n


struct cmd {
    char cmd[MAX_CMD_SIZE];
    unsigned long cmd_size;
    bool multiline;
};

typedef struct queue_cdt *queue_adt;

typedef struct cmd elem_type;

queue_adt new_queue();

void free_queue(queue_adt q);

int queue(queue_adt q, elem_type elem);

int dequeue(queue_adt q, elem_type *elem);

int is_empty(queue_adt q);

void to_begin(queue_adt q);

int has_next(queue_adt q);

elem_type next(queue_adt q);

#endif //PROTOS_TP2E_CMD_QUEUE_H
