/*
** EPITECH PROJECT, 2024
** B-NWP-400-MAR-4-1-myftp-selim.bouasker
** File description:
** main.c
*/

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

void print_help(void)
{
    printf("USAGE: ./myftp port path\n");
    printf("  port is the port number on which the server socket listens\n");
    printf("  path is the path to the home directory for the Anonymous user");
    print("\n");
}

void init_server(int *my_socket, struct sockaddr_in *server_addr, char *str)
{
    *my_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (*my_socket == -1) {
        perror("socket");
        exit(1);
    }
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(atoi(str));
    if (bind(*my_socket, (struct sockaddr*)server_addr,
        sizeof(*server_addr)) == -1 ||
        listen(*my_socket, 5) == -1) {
        perror("bind/listen");
        exit(1);
    }
}

void handle_clients(int my_socket)
{
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int client_socket;
    char message[] = "I lost the game :3\n";

    while (1) {
        client_socket = accept(my_socket, (struct sockaddr*)&client_addr,
            &addr_size);
        if (client_socket == -1)
            continue;
        if (!fork()) {
            close(my_socket);
            sleep(5);
            write(client_socket, message, strlen(message));
            close(client_socket);
            exit(0);
        }
        close(client_socket);
    }
}

void my_ftp(char *str)
{
    int my_socket;
    struct sockaddr_in server_addr;

    init_server(&my_socket, &server_addr, str);
    printf("Serveur en attente de connexions sur le port %s...\n", str);
    handle_clients(my_socket);
    close(my_socket);
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
