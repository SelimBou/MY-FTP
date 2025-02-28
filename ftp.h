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
    #define PATH_MAX 4096
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <string.h>
    #include <poll.h>
    #include <unistd.h>
    #include <limits.h>
    #include <dirent.h>

typedef struct client {
    int fd;
    int is_authenticated;
    char cwd[BUFFER_SIZE];
    char username[BUFFER_SIZE];
    DIR *dir;
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
void process_client_message(command_t *cmd, char *path);

void user_handling(const char *buffer, command_t *cmd);
void handle_standard_login(command_t *cmd, const char *password);
void handle_anonymous_login(command_t *cmd, const char *password);

void cdup_handling(command_t *cmd, char *path);
void cwd_handling(const char *buffer, command_t *cmd, char *path);
void check_command(char *buffer, command_t *cmd);
bool is_valid_command(const char *buffer, const char *cmd, int len);
void check_command_2(char *buffer, command_t *cmd, char *path);
void handle_anonymous_login(command_t *cmd, const char *password);
void handle_standard_login(command_t *cmd, const char *password);
void user_handling(const char *buffer, command_t *cmd);
#endif
