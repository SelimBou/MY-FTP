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
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <fcntl.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <string.h>
    #include <poll.h>
    #include <unistd.h>
    #include <limits.h>
    #include <dirent.h>


typedef struct {
    int data_socket;
    int port;
    bool is_active;
} pasv_connection_t;

typedef struct client {
    int fd;
    int is_authenticated;
    char cwd[BUFFER_SIZE];
    char username[BUFFER_SIZE];
    pasv_connection_t pasv_conn;
    DIR *dir;
} client_t;

typedef struct command_s {
    struct pollfd *fds;
    client_t *clients;
    int *nfds;
    int i;
    char buffer[BUFFER_SIZE];
    char path[PATH_MAX];
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
void stor_handling(command_t *cmd);

#endif
