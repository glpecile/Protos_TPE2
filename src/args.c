#include "../include/args.h"

//static unsigned short port(const char *s) {
//     char *end     = 0;
//     const long sl = strtol(s, &end, 10);
//
//     if (end == s|| '\0' != *end
//        || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
//        || sl < 0 || sl > USHRT_MAX) {
//         fprintf(stderr, "port should in in the range of 1-65536: %s\n", s);
//         exit(1);
//         return 1;
//     }
//     return (unsigned short)sl;
//    return 0;
//}

//static void
//user(char *s, struct users *user) {
//    char *p = strchr(s, ':');
//    if(p == NULL) {
//        fprintf(stderr, "password not found\n");
//        exit(1);
//    } else {
//        *p = 0;
//        p++;
//        user->name = s;
//        user->pass = p;
//    }
//
//}

static void version(void) {
    fprintf(stderr, "POP3 Proxy version -1.0\n"
                    "ITBA Protocolos de Comunicación 2021/2Q -- Group 2\n"
    );
}

static void usage() {
    fprintf(stderr,
            "Usage: pop3filter [ POSIX style options ] <origin_server>\n"
            "\n"
            "   -e               Specifies the file where stderr is sent after the execution of the filters. By default the file is /dev/null.\n"
            "   -h               Prints help and exits.\n"
            "   -l               Establishes the direction where the proxy will be served.  By default it listens every interfaces.\n"
            "   -L               Establishes the direction where the management service is sent. By default it listens only in loopback.\n"
            "   -o               Port where management server is found.   By default it is set to 9090.\n"
            "   -p               TCP port where it is listening for incoming POP3 connections.  By default it is set to 1110.\n"
            "   -P               TCP port where the POP3 server in the origin server.  By default it is set to 110.\n"
            "   -t               Command used for external transformations.  Compatible with system(3).  The FILTERS section gives information regarding the interaction between pop3filter and the filter command.  By default no transformations are applied.\n"
            "   -v               Prints information regarding the version and exits.\n"
            "\n");
//    exit(1);
}

static void help() {
    fprintf(stderr, "\n -------------- Help -------------- \n");
    usage();
}

int is_valid_port(char *parameter) {
    if (strlen(parameter) == 4) {
        for (int i = 0; i < 4; i++) {
            if (!isdigit(parameter[i])) {
                return -1;
            }
        }
        return 0;
    }
    return -1;
}

int is_valid_error_file(const char *parameter) {
    FILE *fb = fopen(parameter, "r");
    if (fb == NULL)
        return -1;
    else
        return 0;
}

int validate_parameter(const char *option, char *param) {
    switch (option[1]) { //option[0] must be '-'.
        case 'e':
            if (is_valid_error_file(param) != 0) {
                fprintf(stderr, "Invalid error file\n");
                return -1;
            }
            break;
        case 'o':
            if (is_valid_port(param) != 0) {
                fprintf(stderr, "Invalid management port\n");
                return -1;
            }
            break;
        case 'p':
            if (is_valid_port(param) != 0) {
                fprintf(stderr, "Invalid port\n");
                return -1;
            }
            break;
        case 'P':
            if (is_valid_port(param) != 0) {
                fprintf(stderr, "Invalid origin port\n");
                return -1;
            }
            break;
    }
    return 0;
}

void free_argv_parameters(char **argv_parameter, int size) {
    for (int i = 0; i < size; i++) {
        free(argv_parameter[i]);
    }
    free(argv_parameter);
}

int validate_parameters(const int argc, char **argv) {
    int size = argc - 1;
    char **argv_params = (char **) malloc((size) * sizeof(char *));
    for (int i = 0; i < size; i++) {
        argv_params[i] = malloc(sizeof(char) * strlen(argv[i]) + 1);
        strcpy(argv_params[i], argv[i]);
    }
    opterr = 0;
    char *next_param = NULL;
    for (int i = 1; i < size; i++) {
        if (optind < size) {
            next_param = argv_params[optind];

            if (next_param[0] != '-') {
                fprintf(stderr, "Options must begin with '-'. Run ./pop3filter -h for help.\n");
                free_argv_parameters(argv_params, size);
                return -1;
            }
            int option = getopt(size, argv_params, "e:l:L:o:p:P:t:");
            if (optarg != NULL)
                if (optarg[0] == '-') {
                    fprintf(stderr, "Parameters cannot begin with '-'.\n");
                    free_argv_parameters(argv_params, size);
                    return -1;
                }
            switch (option) {
                case ':':
                    fprintf(stderr, "Missing parameter for option %s.\n", next_param);
                    break;
                case '?':
                    fprintf(stderr, "Invalid option: %s.\n", next_param);
                    break;
                default:
                    fprintf(stderr, "Argument of option '%s' is %s.\n", next_param, optarg);
                    validate_parameter(next_param, optarg);
                    break;
            }
        }
    }
    fprintf(stderr, "No errors on input\n");
    free_argv_parameters(argv_params, size);
    return 0;
}

int parse_parameters(const int argc, char **argv) {
    if (argc < 2) {
        fprintf(stdout, "POP3 Proxy execution requires at least one argument.\n");
        usage();
        return -1;
    }
    switch (argv[1][1]) { //argv[1][0] = '-'
        case 'h':
            if (argc >= 3) {
                usage();
                return -1;
            }
            help();
            exit(0);
        case 'v':
            if (argc >= 3) {
                usage();
                return -1;
            }
            version();
            exit(0);
    }
    if (validate_parameters(argc, argv) != 0) {
        usage();
        return -1;
    }
    return 0;
}

params parameters;

void initialize_pop3_parameters_options() {
    parameters = malloc(sizeof(*parameters));
    parameters->port = 1110;
    parameters->error_file = "/dev/null";
    parameters->management_address = "127.0.0.1";
    parameters->management_port = 9090;
    parameters->listen_address = "0.0.0.0";
    parameters->origin_port = 110;
//    parameters->filter_command                  = malloc(sizeof(*e_transformation));
//    parameters->filter_command->switch_program  = false; //tiene que estar seteado en false
//    parameters->filter_command->program_name    = (unsigned char *) "cat"; //tiene que estar seteado en cat
}

params assign_param_values(const int argc, char **argv) {
    int c;

    parameters->origin_server = argv[argc - 1];
    optind = 1;

    while ((c = getopt(argc, argv, "e:l:L:o:p:P:t:")) != -1) { // -v and -h were considered before, in parse_parameters

        switch (c) {
            case 'e':
                parameters->error_file = optarg;
                break;
            case 'l':
                parameters->listen_address = optarg;
                break;
            case 'L':
                parameters->management_address = optarg;
                break;
            case 'o':
                parameters->management_port = (uint16_t) atoi(optarg);
                break;
            case 'p':
                parameters->port = (uint16_t) atoi(optarg);
                break;
            case 'P':
                parameters->origin_port = (uint16_t) atoi(optarg);
                break;
            case 't':
                // TODO
                break;
            default:
                //should not enter here if validations worked correctly
                usage();
                exit(1);
                break;
        }

    }
    return parameters;
}
