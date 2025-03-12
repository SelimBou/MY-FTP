/*
** EPITECH PROJECT, 2024
** B-NWP-400-MAR-4-1-myftp-selim.bouasker
** File description:
** ftp.h
*/

#ifndef FTP_H_
    #define FTP_H_
    #define MAX_CLIENTS 150
    #define BUFFER_SIZE 1024
    #define PATH_MAX 4096
    #include <netinet/in.h>
    #include <ctype.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #include <sys/stat.h>
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <fcntl.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <string.h>
    #include <poll.h>
    #include <limits.h>
    #include <dirent.h>

typedef struct {
    int data_socket;
    int port;
    bool is_active;
} pasv_connection_t;

typedef struct {
    int data_socket;
    bool is_active;
} port_connection_t;

typedef struct {
    struct sockaddr_in client_addr;
    bool success;
} port_arguments_t;

typedef struct client {
    int fd;
    int is_authenticated;
    char cwd[PATH_MAX];
    char username[BUFFER_SIZE];
    pasv_connection_t pasv_conn;
    port_connection_t port_conn;
} client_t;

typedef struct command_s {
    struct pollfd *fds;
    client_t *clients;
    int *nfds;
    int i;
    char buffer[BUFFER_SIZE];
    char path[PATH_MAX];
    char base_dir[PATH_MAX];
} command_t;

typedef void (*command_handler_t)(command_t *);

typedef struct {
    char *command;
    command_handler_t handler;
} command_entry_t;

void add_server_socket(struct pollfd *fds, int server_socket);
int accept_new_client(int server_socket, client_t *clients,
    struct pollfd *fds, int *nfds);
void process_client_message(command_t *cmd);

void user_handling(command_t *cmd);
void pass_handling(command_t *cmd);
void noop_handling(command_t *cmd);
void help_handling(command_t *cmd);
void cdup_handling(command_t *cmd);
void cwd_handling(command_t *cmd);
void pwd_handling(command_t *cmd);
void pasv_handling(command_t *cmd);
void del_handling(command_t *cmd);
void retr_handling(command_t *cmd);
void list_handling(command_t *cmd);
void port_handling(command_t *cmd);

int check_data_connection(command_t *cmd, int sockfd);

#endif