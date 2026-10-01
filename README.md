# Network Systems & Utilities

A collection of C networking utilities built using TCP/IP sockets, HTTP/1.1, and Unix network programming APIs. The project includes a reusable TCP socket interface, a command-line HTTP client, and a multi-threaded network port scanner.

## Features

### TCP Socket Library

Implemented a reusable socket interface for establishing TCP connections to remote hosts.

The socket layer:

- Resolves hostnames and service ports using `getaddrinfo()`
- Supports IPv4 and IPv6 through `AF_UNSPEC`
- Creates TCP sockets with `SOCK_STREAM`
- Attempts connections across available address results
- Wraps connected socket file descriptors as standard C `FILE` streams
- Handles connection and resource cleanup on failure

### HTTP Client

Built a simple HTTP/1.1 client (`curlit`) on top of the TCP socket interface.

The client establishes a TCP connection to a remote server and uses HTTP requests to retrieve content over the network.

### Network Port Scanner

Built a multi-threaded network port scanner (`nmapit`) that attempts TCP connections to identify accessible network services.

The scanner uses concurrent host dialing to scan multiple ports efficiently and includes signal handling for interrupting and managing the scanning process.
