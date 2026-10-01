/* curlit.c: Simple HTTP client*/

#include "socket.h"

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>


#include <netdb.h>

/* Constants */

#define HOST_DELIMITER  "://"
#define PATH_DELIMITER  '/'
#define PORT_DELIMITER  ':'
#define BILLION         (1000000000.0)
#define MEGABYTES       (1<<20)

/* Macros */

#define streq(a, b) (strcmp(a, b) == 0)

/* Structures */

typedef struct {
    char host[NI_MAXHOST];
    char port[NI_MAXSERV];
    char path[PATH_MAX];
} URL;

/* Functions */

/**
 * Display usage message and exit.
 * @param   status      Exit status.
 **/
void    usage(int status) {
    fprintf(stderr, "Usage: curlit [-h] URL\n");
    exit(status);
}

/**
 * Parse URL string into URL structure.
 * @param   s       URL string
 * @param   url     Pointer to URL structure
 **/
void    parse_url(const char *s, URL *url) {
    // TODO: Copy data to local buffer
    char buffer[BUFSIZ];
    strcpy(buffer, s);

    // TODO: Skip scheme to host
    char *host_port_path = strstr(buffer, HOST_DELIMITER);
    if (host_port_path == NULL) {
        strcpy(url->host, buffer); 
    } else {
        // printf("in here\n");
        host_port_path += strlen(HOST_DELIMITER); 
        // printf("%s\n", host_port_path); 
        strcpy(url->host, host_port_path);
    }

    // TODO: Split host:port from path
    char *port_path_delimiter = strchr(url->host, PATH_DELIMITER);
    if (port_path_delimiter != NULL) {
        // printf("%s\n", port_path_delimiter); 
        strcpy(url->path, port_path_delimiter);
        *port_path_delimiter = '\0'; 
        // printf("%s\n", url->path); 
    }

    // TODO: Split host and port
    char *port_delimiter = strchr(url->host, PORT_DELIMITER);

    // TODO: Copy components to URL
    if (port_delimiter != NULL) {
        *port_delimiter = '\0'; 
        strcpy(url->port, port_delimiter + 1); 
    } else {
        strcpy(url->port, "80"); 
    }
}

/**
 * Fetch contents of URL and print to standard out.
 *
 * Print elapsed time and bandwidth to standard error.
 * @param   s       URL string
 * @param   url     Pointer to URL structure
 * @return  true if client is able to read all of the content (or if the
 * content length is unset), otherwise false
 **/
bool    fetch_url(URL *url) {
    // TODO: Grab start time
    struct timespec start_time; 
    bool result = true; 

    int start_return = clock_gettime(CLOCK_MONOTONIC, &start_time);
    if(start_return < 0) {
        fprintf(stderr, "%s\n", strerror(errno)); 
        return false; 
    }

    // TODO: Connect to remote host and port
    FILE *socket = socket_dial(url->host, url->port);
    if (socket == NULL) {
        fprintf(stderr, "Error: Failed to connect to %s:%s\n", url->host, url->port);
        return false;
    }

    // TODO: Send request to server
    fprintf(socket, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", url->path, url->host);

    // TODO: Read status response from server
    char response[BUFSIZ];
    if (fgets(response, sizeof(response), socket) == NULL) {
        fprintf(stderr, "Error: Failed to read response status\n");
        // fclose(socket);
        result = false;
    }

    // TODO: Read response headers from server
    // printf("response is \n%s\n%s\n%d\n", strstr(response, " "), " 200 OK\r\n", streq(strstr(response, " "), " 200 OK\r\n")); 
    if (!streq(strstr(response, " "), " 200 OK\r\n")) {
        fprintf(stderr, "Error: Server response is not 200 OK\n");
        // fclose(socket);
        result = false;
    }
    
    char header[BUFSIZ];
    size_t content_length = 0;
    while (fgets(header, sizeof(header), socket) != NULL && strcmp(header, "\r\n") != 0) {
        if (streq(strtok(header, ":"), "Content-Length")) {
            sscanf(strtok(NULL, ":"), " %zu", &content_length);
        }
    }

    // TODO: Read response body from server
    size_t total_bytes_read = 0, nread = 0; 
    char buffer[BUFSIZ];
    while ((nread = fread(buffer, 1, BUFSIZ, socket)) > 0) {
        fwrite(buffer, 1, nread, stdout);
        // printf("nread is %ld\n", nread); 
        total_bytes_read += nread; 

    }


    // TODO: Grab end time
    struct timespec end_time; 
    int end_return = clock_gettime(CLOCK_MONOTONIC, &end_time); 
    if(end_return < 0) {
        fprintf(stderr, "%s\n", strerror(errno)); 
        fclose(socket);
        return false; 
    }

    // TODO: Output metrics
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / BILLION;

    // printf("%.2ld\n", total_bytes_read); 
    double bandwidth = total_bytes_read / elapsed_time / MEGABYTES;

    fprintf(stderr, "Elapsed Time: %.2f s\n", elapsed_time);
    fprintf(stderr, "Bandwidth:    %.2f MB/s\n", bandwidth);

    if (content_length != 0 && total_bytes_read != content_length) {
        // printf("%.2ld %.2ld %d\n", total_bytes_read, content_length, result); 

        fprintf(stderr, "Error: Incomplete content read\n");
        // fclose(socket);
        result = false;
        // printf("%d\n", result); 
    }

    fclose(socket);
    // printf("%d\n", result); 
    return result;
}

/* Main Execution */

int     main(int argc, char *argv[]) {
    // TODO: Parse command line options
    int argind = 1;

    while(argind < argc && strlen(argv[argind])>1 && argv[argind][0] == '-') {
        char *arg = argv[argind++]; 
        if (argc < 1) 
            usage(1);
        
        switch (arg[1]) {
                case 'h': usage(0); break; 
                default: usage(1); break;
        }
    }
    
    if (argind >= argc) {
        usage(1);
    }
    
    // TODO: Parse URL
    URL url = {0};
    parse_url(argv[argind], &url);
    // printf("Parsed URL:\n");
    // printf("Host: %s\n", url.host);
    // printf("Port: %s\n", url.port);
    // printf("Path: %s\n", url.path);

    // TODO: Fetch URL
    // printf("result is %d\n", !fetch_url(&url));
    if (!fetch_url(&url)) {
        // printf("failed\n"); 
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim: set sts=4 sw=4 ts=8 expandtab ft=c: */
