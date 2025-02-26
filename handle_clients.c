/*
** EPITECH PROJECT, 2024
** B-NWP-400-MAR-4-1-myftp-selim.bouasker
** File description:
** handle_clients.c
*/

#include "ftp.h"

void add_server_socket(struct pollfd *fds, int server_socket)
{
    fds[0].fd = server_socket;
    fds[0].events = POLLIN;
    printf("Serveur en attente de connexions...\n");
}

static void setup_client(client_t *clients, struct pollfd *fds, int *nfds,
    int client_socket)
{
    fds[*nfds].fd = client_socket;
    fds[*nfds].events = POLLIN;
    clients[*nfds].fd = client_socket;
    clients[*nfds].is_authenticated = 0;
    (*nfds)++;
    write(client_socket, "220 FTP Server Ready\r\n", 22);
}

int accept_new_client(int server_socket, client_t *clients,
    struct pollfd *fds, int *nfds)
{
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int client_socket = accept(server_socket,
        (struct sockaddr*)&client_addr, &addr_size);

    if (client_socket < 0) {
        perror("accept");
        return -1;
    }
    printf("Connexion de %s:%d\n", inet_ntoa(client_addr.sin_addr),
        ntohs(client_addr.sin_port));
    if (*nfds < MAX_CLIENTS + 1)
        setup_client(clients, fds, nfds, client_socket);
    else
        close(client_socket);
    return 0;
}

static void remove_client(struct pollfd *fds, client_t *clients,
    int *nfds, int i)
{
    printf("Client %d déconnecté\n", fds[i].fd);
    close(fds[i].fd);
    fds[i] = fds[*nfds - 1];
    clients[i] = clients[*nfds - 1];
    (*nfds)--;
}

void check_command(const char *buffer, command_t *cmd)
{
    if (strncmp(buffer, "USER Anonymous", 14) == 0) {
        write(cmd->fds[cmd->i].fd,
            "331 User name okay, need password.\r\n", 36);
        return;
    }
    if (strncmp(buffer, "PASS", 4) == 0) {
        if (strlen(buffer) == 6) {
            cmd->clients[cmd->i].is_authenticated = 1;
            write(cmd->fds[cmd->i].fd, "230 User logged in, proceed.\r\n", 30);
        } else {
            write(cmd->fds[cmd->i].fd, "530 Login incorrect.\r\n", 22);
        }
        return;
    }
    if (strncmp(buffer, "QUIT", 4) == 0) {
        write(cmd->fds[cmd->i].fd, "221 Goodbye.\r\n", 13);
        remove_client(cmd->fds, cmd->clients, cmd->nfds, cmd->i);
        return;
    }
    write(cmd->fds[cmd->i].fd, "500 Commande non reconnue\r\n", 27);
}

void process_client_message(struct pollfd *fds, client_t *clients,
    int *nfds, int i)
{
    int bytes_received;
    char buffer[BUFFER_SIZE];
    command_t cmd = {fds, clients, nfds, i};

    memset(buffer, 0, BUFFER_SIZE);
    bytes_received = read(fds[i].fd, buffer, BUFFER_SIZE - 1);
    if (bytes_received <= 0) {
        remove_client(fds, clients, nfds, i);
        return;
    }
    printf("Message reçu : %s", buffer);
    check_command(buffer, &cmd);
}
