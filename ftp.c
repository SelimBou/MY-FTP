/*
** EPITECH PROJECT, 2024
** B-NWP-400-MAR-4-1-myftp-selim.bouasker
** File description:
** main.c
*/

#include "ftp.h"

void print_help(void)
{
    printf("USAGE: ./myftp port path\n");
    printf("  port is the port number on which the server socket listens\n");
    printf("  path is the path to the home directory for the Anonymous user");
    printf("\n");
}

void init_server(int *server_socket, struct sockaddr_in *server_addr,
    char *port)
{
    int opt = 1;

    *server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_socket == -1) {
        perror("socket");
        exit(1);
    }
    setsockopt(*server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(atoi(port));
    if (bind(*server_socket, (struct sockaddr*)server_addr,
        sizeof(*server_addr)) == -1 ||
        listen(*server_socket, MAX_CLIENTS) == -1) {
        perror("bind/listen");
        exit(1);
    }
}

static void check_client_activity(struct pollfd *fds, client_t *clients,
    int *nfds)
{
    for (int i = 1; i < *nfds; i++) {
        if (fds[i].revents & POLLIN) {
            process_client_message(fds, clients, nfds, i);
        }
    }
}

void handle_clients(int server_socket)
{
    struct pollfd fds[MAX_CLIENTS + 1];
    client_t clients[MAX_CLIENTS + 1];
    int nfds = 1;

    add_server_socket(fds, server_socket);
    while (1) {
        poll(fds, nfds, -1);
        if (fds[0].revents & POLLIN) {
            accept_new_client(server_socket, clients, fds, &nfds);
        }
        check_client_activity(fds, clients, &nfds);
    }
}

void my_ftp(char *port)
{
    int server_socket;
    struct sockaddr_in server_addr;

    init_server(&server_socket, &server_addr, port);
    handle_clients(server_socket);
    close(server_socket);
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "-help") == 0) {
        print_help();
        return 0;
    }
    if (argc == 2) {
        my_ftp(argv[1]);
        return 0;
    } else {
        return 84;
    }
}
