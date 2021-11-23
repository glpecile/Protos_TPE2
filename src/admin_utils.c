#include "../include/admin_utils.h"

udp_stats stats;

int initialize_stats() {
    stats = calloc(1, sizeof(*stats));
    if(stats == NULL) return -1;
    stats->curent_connections = 0;
    stats->historic_connections = 0;
    stats->bytes_transfered = 0;
    return 0;
}

void stats_new_connection() {
    stats->curent_connections += 1;
    stats->historic_connections += 1;
}

void stats_closed_connection() {
    if(stats->curent_connections > 0)
    stats->curent_connections -= 1;
}

void add_transfered_bytes(int amount) {
    stats->bytes_transfered += amount;
}

void close_stats() {
    free(stats);
}
