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
    getcwd(clients[*nfds].cwd, BUFFER_SIZE);
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

static void trim_newline(char *buffer)
{
    char *pos = strpbrk(buffer, "\r\n");

    if (pos) {
        *pos = '\0';
    }
}

void handle_quit(command_t *cmd)
{
    write(cmd->fds[cmd->i].fd, "221 Goodbye.\r\n", 14);
    remove_client(cmd->fds, cmd->clients, cmd->nfds, cmd->i);
}

void init_commands(command_entry_t *commands)
{
    commands[0] = (command_entry_t){"USER", user_handling};
    commands[1] = (command_entry_t){"PASS", pass_handling};
    commands[2] = (command_entry_t){"QUIT", handle_quit};
    commands[3] = (command_entry_t){"CWD", cwd_handling};
    commands[4] = (command_entry_t){"CDUP", cdup_handling};
    commands[5] = (command_entry_t){"NOOP", noop_handling};
    commands[6] = (command_entry_t){"HELP", help_handling};
    commands[7] = (command_entry_t){"PWD", pwd_handling};
    commands[8] = (command_entry_t){"DELE", del_handling};
    commands[9] = (command_entry_t){"RETR", retr_handling};
    commands[10] = (command_entry_t){NULL, NULL};
}

static bool is_valid_command(const char *buffer, const char *cmd, int len)
{
    return (strncasecmp(buffer, cmd, len) == 0 &&
            (buffer[len] == ' ' || buffer[len] == '\0'));
}

static void check_command(command_t *cmd)
{
    command_entry_t commands[11];

    trim_newline(cmd->buffer);
    init_commands(commands);
    if (!cmd->clients[cmd->i].is_authenticated &&
        strncasecmp(cmd->buffer, "USER", 4) != 0 &&
        strncasecmp(cmd->buffer, "PASS", 4) != 0 &&
        strncasecmp(cmd->buffer, "QUIT", 4) != 0) {
        write(cmd->fds[cmd->i].fd,
            "530 Please login with USER and PASS.\r\n", 38);
        return;
    }
    for (int i = 0; commands[i].command != NULL; i++) {
        if (is_valid_command(cmd->buffer, commands[i].command,
            strlen(commands[i].command))) {
            commands[i].handler(cmd);
            return;
        }
    }
    write(cmd->fds[cmd->i].fd, "500 Commande non reconnue\r\n", 27);
}

void process_client_message(command_t *cmd)
{
    int bytes_received;

    memset(cmd->buffer, 0, BUFFER_SIZE);
    bytes_received = read(cmd->fds[cmd->i].fd, cmd->buffer, BUFFER_SIZE - 1);
    if (bytes_received <= 0) {
        remove_client(cmd->fds, cmd->clients, cmd->nfds, cmd->i);
        return;
    }
    printf("Message reçu : %s", cmd->buffer);
    check_command(cmd);
}
