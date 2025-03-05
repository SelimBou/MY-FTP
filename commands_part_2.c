/*
** EPITECH PROJECT, 2024
** B-NWP-400-MAR-4-1-myftp-selim.bouasker
** File description:
** check_command.c
*/

#include "ftp.h"

void noop_handling(command_t *cmd)
{
    write(cmd->fds[cmd->i].fd, "200 NOOP command okay.\r\n", 24);
}

void help_handling(command_t *cmd)
{
    const char *help_msg =
        "214 All commands: USER, PASS, CWD, CDUP, QUIT, PWD, NOOP, HELP\r\n";

    write(cmd->fds[cmd->i].fd, help_msg, strlen(help_msg));
}

void del_handling(command_t *cmd)
{
    char *args = cmd->buffer + 4;

    while (*args == ' ')
        args++;
    if (*args == '\0') {
        write(cmd->fds[cmd->i].fd,
            "501 Syntax error in parameters or arguments.\r\n", 46);
        return;
    }
    if (remove(args) == 0) {
        write(cmd->fds[cmd->i].fd,
            "250 Requested file action okay, completed.\r\n", 44);
    } else {
        write(cmd->fds[cmd->i].fd, "550 File not found or access denied.\r\n",
            38);
    }
}

static int accept_data_connection(command_t *cmd)
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int data_socket = accept(cmd->clients[cmd->i].pasv_conn.data_socket,
        (struct sockaddr*)&client_addr, &addr_len);

    if (!cmd->clients[cmd->i].pasv_conn.is_active) {
        write(cmd->fds[cmd->i].fd, "425 Use PASV first.\r\n", 21);
        return -1;
    }
    if (data_socket < 0) {
        write(cmd->fds[cmd->i].fd, "425 Can't open data connection.\r\n", 33);
        return -1;
    }
    return data_socket;
}

static void send_directory_listing(command_t *cmd, int data_socket)
{
    struct dirent *entry = readdir(opendir(cmd->clients[cmd->i].cwd));
    char file_info[1024];

    while (entry != NULL) {
        snprintf(file_info, sizeof(file_info), "%s\r\n", entry->d_name);
        write(data_socket, file_info, strlen(file_info));
        entry = readdir(cmd->clients[cmd->i].cwd);
    }
}

void list_handling(command_t *cmd)
{
    int data_socket = accept_data_connection(cmd);

    if (data_socket < 0) {
        return;
    }
    write(cmd->fds[cmd->i].fd, "150 Here comes the directory listing\r\n", 38);
    send_directory_listing(cmd, data_socket);
    close(data_socket);
    write(cmd->fds[cmd->i].fd, "226 Directory send OK.\r\n", 24);
    close(cmd->clients[cmd->i].pasv_conn.data_socket);
    cmd->clients[cmd->i].pasv_conn.is_active = false;
}

static int validate_retr_arguments(command_t *cmd, char **args)
{
    int data_socket;

    *args = cmd->buffer + 4;
    if (!cmd->clients[cmd->i].pasv_conn.is_active) {
        write(cmd->fds[cmd->i].fd, "425 Use PASV first.\r\n", 21);
        return -1;
    }
    while (**args == ' ')
        (*args)++;
    if (**args == '\0') {
        write(cmd->fds[cmd->i].fd, "501 Syntax error in parameters.\r\n", 33);
        return -1;
    }
    return data_socket;
}

static void send_file_content(command_t *cmd, int data_socket, char *args)
{
    int file_fd = open(args, O_RDONLY);
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    if (file_fd < 0) {
        write(cmd->fds[cmd->i].fd, "550 File not found.\r\n", 21);
        close(data_socket);
        return;
    }
    write(cmd->fds[cmd->i].fd, "150 Opening data connection.\r\n", 30);
    bytes_read = read(file_fd, buffer, BUFFER_SIZE);
    while (bytes_read > 0) {
        write(data_socket, buffer, bytes_read);
        bytes_read = read(file_fd, buffer, BUFFER_SIZE);
    }
    close(file_fd);
    close(data_socket);
    write(cmd->fds[cmd->i].fd, "226 Transfer complete.\r\n", 24);
}

void retr_handling(command_t *cmd)
{
    char *args;
    int data_socket = validate_retr_arguments(cmd, &args);

    if (data_socket < 0) {
        return;
    }
    send_file_content(cmd, data_socket, args);
    close(cmd->clients[cmd->i].pasv_conn.data_socket);
    cmd->clients[cmd->i].pasv_conn.is_active = false;
}
