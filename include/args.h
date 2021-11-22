#ifndef ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8
#define ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>
#include <ctype.h>

#define MAX_USERS 10

struct users {
    char *name;
    char *pass;
};

struct doh {
    char           *host;
    char           *ip;
    unsigned short  port;
    char           *path;
    char           *query;
};

struct socks5args {
    char           *socks_addr;
    unsigned short  socks_port;

    char *          mng_addr;
    unsigned short  mng_port;

    bool            disectors_enabled;

    struct doh      doh;
    struct users    users[MAX_USERS];
};

struct params {
    uint16_t port;              // -p
    char *error_file;           // -e
    char *listen_address;       // -l
    char *management_address;   // -L
    uint16_t management_port;   // -o
    char *origin_server;        // This is the argument origin_server, it's not an option
    uint16_t origin_port;       // -P
//    transformations          filter_command;       // -t TODO
};

typedef struct params * params;

extern params parameters;

/**
 * Interpreta la linea de comandos (argc, argv) llenando
 * args con defaults o la seleccion humana. Puede cortar
 * la ejecución.
 */
int parse_parameters(const int argc, char **argv);

void initialize_pop3_parameters_options();

params assign_param_values(const int argc, char **argv);

#endif

