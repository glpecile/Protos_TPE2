#ifndef PROTOS_TP2E_ADMIN_UTILS_H
#define PROTOS_TP2E_ADMIN_UTILS_H

#include <stdlib.h>


struct udp_stats{
    int historic_connections;
    int curent_connections;
    long bytes_transfered;
};

typedef struct udp_stats * udp_stats;
extern udp_stats stats;

int initialize_stats();

/*
 * Increments the number of current and historic connections by 1.
 */
void stats_new_connection();

/*
 * Decrements the number of current connections by 1.
 */
void stats_closed_connection();

void add_transfered_bytes(long amount);

void close_stats();

#endif //PROTOS_TP2E_ADMIN_UTILS_H
