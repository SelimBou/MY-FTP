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

static char *clean_path(char *path)
{
    char *end;

    while (*path == ' ')
        path++;
    end = strchr(path, '\r');
    if (end)
        *end = '\0';
    end = strchr(path, '\n');
    if (end)
        *end = '\0';
    return path;
}

void del_handling(command_t *cmd)
{
    char *args = cmd->buffer + 4;
    char *cleaned_path = clean_path(args);

    if (*cleaned_path == '\0') {
        write(cmd->fds[cmd->i].fd, "501 Syntax error in parameters.\r\n", 33);
        return;
    }
    if (access(cleaned_path, F_OK) != 0) {
        write(cmd->fds[cmd->i].fd, "550 File not found.\r\n", 21);
        return;
    }
    if (access(cleaned_path, W_OK) != 0) {
        write(cmd->fds[cmd->i].fd, "550 Access denied.\r\n", 20);
        return;
    }
    if (remove(cleaned_path) == 0) {
        write(cmd->fds[cmd->i].fd, "250 file action okay, completed.\r\n", 34);
    } else {
        write(cmd->fds[cmd->i].fd, "550 Could not delete file.\r\n", 28);
    }
}

static int accept_data_connection(command_t *cmd)
{
    int data_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if (!cmd->clients[cmd->i].pasv_conn.is_active &&
        !cmd->clients[cmd->i].port_conn.is_active) {
        write(cmd->fds[cmd->i].fd, "425 Use PASV or PORT first.\r\n", 28);
        return -1;
    }
    if (cmd->clients[cmd->i].pasv_conn.is_active) {
        data_socket = accept(cmd->clients[cmd->i].pasv_conn.data_socket,
            (struct sockaddr*)&client_addr, &addr_len);
        if (check_data_connection(cmd, data_socket) < 0)
            return -1;
    } else {
        data_socket = cmd->clients[cmd->i].port_conn.data_socket;
    }
    return data_socket;
}

static void send_directory_listing(command_t *cmd, int data_socket)
{
    struct dirent *entry;
    char file_info[256];
    DIR *dir = opendir(cmd->clients[cmd->i].cwd);

    if (!dir) {
        write(cmd->fds[cmd->i].fd, "550 Failed to open directory.\r\n", 32);
        return;
    }
    entry = readdir(dir);
    while (entry != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;
        snprintf(file_info, sizeof(file_info), "%s\r\n", entry->d_name);
        if (write(data_socket, file_info, strlen(file_info)) < 0)
            break;
        entry = readdir(dir);
    }
    closedir(dir);
}

void list_handling(command_t *cmd)
{
    int data_socket;

    data_socket = accept_data_connection(cmd);
    if (data_socket < 0) {
        return;
    }
    write(cmd->fds[cmd->i].fd, "150 Here comes the directory listing.\r\n",
        38);
    send_directory_listing(cmd, data_socket);
    close(data_socket);
    write(cmd->fds[cmd->i].fd, "226 Directory send OK.\r\n", 24);
    if (cmd->clients[cmd->i].pasv_conn.is_active) {
        close(cmd->clients[cmd->i].pasv_conn.data_socket);
        cmd->clients[cmd->i].pasv_conn.is_active = false;
    } else {
        close(cmd->clients[cmd->i].port_conn.data_socket);
        cmd->clients[cmd->i].port_conn.is_active = false;
    }
}

static int validate_retr_arguments(command_t *cmd, char **args)
{
    *args = cmd->buffer + 4;
    if (!cmd->clients[cmd->i].pasv_conn.is_active &&
        !cmd->clients[cmd->i].port_conn.is_active) {
        write(cmd->fds[cmd->i].fd, "425 Use PASV or PORT first.\r\n", 28);
        return -1;
    }
    while (**args == ' ')
        (*args)++;
    if (**args == '\0' || **args == '\r' || **args == '\n') {
        write(cmd->fds[cmd->i].fd, "501 Syntax error in parameters.\r\n", 33);
        return -1;
    }
    if (cmd->clients[cmd->i].pasv_conn.is_active) {
        return cmd->clients[cmd->i].pasv_conn.data_socket;
    } else {
        return cmd->clients[cmd->i].port_conn.data_socket;
    }
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
    int data_socket;

    data_socket = validate_retr_arguments(cmd, &args);
    if (data_socket < 0) {
        return;
    }
    send_file_content(cmd, data_socket, args);
    if (cmd->clients[cmd->i].pasv_conn.is_active) {
        close(cmd->clients[cmd->i].pasv_conn.data_socket);
        cmd->clients[cmd->i].pasv_conn.is_active = false;
    } else {
        close(cmd->clients[cmd->i].port_conn.data_socket);
        cmd->clients[cmd->i].port_conn.is_active = false;
    }
}
