/*
** EPITECH PROJECT, 2024
** B-NWP-400-MAR-4-1-myftp-selim.bouasker
** File description:
** ftp.h
*/

#ifndef FTP_H_
    #define FTP_H_
    #define MAX_CLIENTS 10
    #define BUFFER_SIZE 1024
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <poll.h>

typedef struct client {
    int fd;
    int is_authenticated;
} client_t;

typedef struct command_s {
    struct pollfd *fds;
    client_t *clients;
    int *nfds;
    int i;
} command_t;

void add_server_socket(struct pollfd *fds, int server_socket);
int accept_new_client(int server_socket, client_t *clients,
    struct pollfd *fds, int *nfds);
void process_client_message(struct pollfd *fds, client_t *clients,
    int *nfds, int i);


#endif
