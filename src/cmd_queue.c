#include <stdlib.h>

#include "../include/cmd_queue.h"

typedef struct t_node {
    elem_type elem;
    struct t_node *next;
} t_node;

typedef struct queue_cdt {
    size_t size;
    t_node *first;
    t_node *last;
    t_node *iter;
} queue_cdt;


static void free_queue_rec(t_node *first);

queue_adt
new_queue() {
    return calloc(1, sizeof(queue_cdt));
}

void
free_queue(queue_adt q) {
    free_queue_rec(q->first);
    free(q);
}

static void
free_queue_rec(t_node *first) {
    if (first == NULL) {
        return;
    }
    free_queue_rec(first->next);
    free(first);
}

int
queue(queue_adt q, elem_type elem) {
    t_node *aux = calloc(1, sizeof(t_node));

    if (aux == NULL) {
        return 0; //Falla la asignacion
    }

    aux->elem = elem;

    if (q->size == 0) {
        q->first = aux;
        q->last = aux;
    } else {
        q->last->next = aux;
        q->last = aux;
    }

    (q->size)++;

    return 1;
}

int
dequeue(queue_adt q, elem_type *elem) {
    if (q->first == NULL) {
        return 0; //No puede sacar elementos
    }

    *elem = q->first->elem;

    t_node *aux = q->first;
    if (q->first == q->last) //o tambien si q->size == 1
    {
        q->first = NULL;
        q->last = NULL;
    } else {
        q->first = q->first->next;
    }

    (q->size)--;
    free(aux);
    return 1;
}

int
is_empty(queue_adt q) {
    return q->size == 0;
}

void
to_begin(queue_adt q) {
    q->iter = q->first;
}

int
has_next(queue_adt q) {
    return q->iter != NULL;
}

elem_type
next(queue_adt q) {
    elem_type aux = q->iter->elem;
    q->iter = q->iter->next;
    return aux;
}

