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

static void pass_handling(const char *buffer, command_t *cmd)
{
    const char *password = buffer + 4;

    if (cmd->clients[cmd->i].username[0] == '\0') {
        write(cmd->fds[cmd->i].fd, "530 Login with USER first.\r\n", 28);
        return;
    }
    if (cmd->clients[cmd->i].is_authenticated) {
        write(cmd->fds[cmd->i].fd, "530 User already connected.\r\n", 30);
        return;
    }
    while (*password == ' ')
        password++;
    if (strcmp(cmd->clients[cmd->i].username, "Anonymous") == 0) {
        handle_anonymous_login(cmd, password);
        return;
    }
    handle_standard_login(cmd, password);
}

static void trim_newline(char *buffer)
{
    char *pos = strpbrk(buffer, "\r\n");

    if (pos) {
        *pos = '\0';
    }
}

void check_command(char *buffer, command_t *cmd)
{
    trim_newline(buffer);
    if (strncasecmp(buffer, "USER", 4) == 0 &&
        (buffer[4] == ' ' || buffer[4] == '\0')) {
        user_handling(buffer, cmd);
        return;
    }
    if (strncasecmp(buffer, "PASS", 4) == 0 &&
        (buffer[4] == ' ' || buffer[4] == '\0')) {
        pass_handling(buffer, cmd);
        return;
    }
    if (strncasecmp(buffer, "QUIT", 4) == 0 &&
        (buffer[4] == ' ' || buffer[4] == '\0')) {
        write(cmd->fds[cmd->i].fd, "221 Goodbye.\r\n", 15);
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
