/* nmapit.c: Simple network port scanner */

#include "socket.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include <signal.h>
#include <errno.h>

#ifndef GNU_SOURCE
typedef void (*sighandler_t)(int);
#endif

/* Functions */

/**
 * Display usage message and exit.
 * @param   status      Exit status
 **/
void    usage(int status) {
    fprintf(stderr, "Usage: nmapit [-p START-END] HOST\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -p START-END    Specifies the range of port numbers to scan\n");
    exit(status);
}

/**
 * Handle alarm signal.
 * @param   signum      Signal number
 **/
void sigalrm_handler(int signum) {
    // TODO: Cancel current alarm
    alarm(signum); 
}

/**
 * Parse port range string into start and end port integers.
 * @param   range       Port range string (ie. START-END)
 * @param   start       Pointer to starting port integer
 * @param   end         Pointer to ending port integer
 * @return  true if parsing both start and end were successful, otherwise false
 **/
bool parse_ports(char *range, int *start, int *end) {
    // TODO: Parse starting port
    char *token = strtok(range, "-");
    if (token == NULL) return false;
    *start = atoi(token);
    
    // TODO: Parse ending port
    token = strtok(NULL, "-");
    if (token == NULL) return false;
    *end = atoi(token);

    return true;
}

/**
 * Scan ports at specified host from starting and ending port numbers
 * (inclusive).
 * @param   host        Host to scan
 * @param   start       Starting port number
 * @param   end         Ending port number
 * @return  true if any port is found, otherwise false
 **/
bool scan_ports(const char* host, int start, int end) {
    // TODO: Register signal handler for alarm
    struct sigaction sa;
    sa.sa_handler = sigalrm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    int status = sigaction(SIGALRM, &sa, NULL); 
    if (status == -1) {
        fprintf(stderr, "sigaction: %s\n", gai_strerror(status)); 
        return false; 
    }

    // TODO: For each port, set alarm, attempt to dial host and port
    bool found = false;
    for (int port = start; port <= end; port++) {
        char port_str[BUFSIZ]; 
        snprintf(port_str, BUFSIZ, "%d", port);

        sigalrm_handler(1);
        FILE *socket = socket_dial(host, port_str); 
        if (socket != NULL) {
            printf("%d\n", port);
            found = true;
            fclose(socket); 
        }
        sigalrm_handler(0);
    }
    return found;
}

/* Main Execution */

int main(int argc, char *argv[]) {
    // TODO: Parse command-line arguments
    int argind = 1;
    int start_port = 1; 
    int end_port = 1023;
    const char *host = NULL;

    while(argind < argc && strlen(argv[argind])>1 && argv[argind][0] == '-') {
        char *arg = argv[argind++]; 
        if (argc < 1) 
            usage(1);
        
        switch (arg[1]) {
                case 'h': usage(0); break; 
                case 'p': {
                    if (argind >= argc) usage(1);
                    char *range = argv[argind++];
                    if (!parse_ports(range, &start_port, &end_port)) usage(1);
                    break;
                }
                default: usage(1); break;
        }
    }
    // TODO: Scan ports
    if (argind < argc) {
        host = argv[argind++];
    }

    if (!host) {
        return EXIT_FAILURE;
    }

    if (!scan_ports(host, start_port, end_port)) {
        return EXIT_FAILURE; 
    }

    return EXIT_SUCCESS;
}

/* vim: set sts=4 sw=4 ts=8 expandtab ft=c: */
