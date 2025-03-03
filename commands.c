/*
** EPITECH PROJECT, 2024
** B-NWP-400-MAR-4-1-myftp-selim.bouasker
** File description:
** commands.c
*/

#include "ftp.h"

static void handle_anonymous_login(command_t *cmd)
{
    if (*(cmd->buffer) != '\0') {
        write(cmd->fds[cmd->i].fd,
            "530 Invalid password for Anonymous.\r\n", 37);
        return;
    }
    cmd->clients[cmd->i].is_authenticated = 1;
    write(cmd->fds[cmd->i].fd, "230 User logged in, proceed.\r\n", 30);
}

static void handle_standard_login(command_t *cmd)
{
    if (*(cmd->buffer) == '\0') {
        write(cmd->fds[cmd->i].fd, "530 Usage: PASS <password>\r\n", 29);
        return;
    }
    cmd->clients[cmd->i].is_authenticated = 1;
    write(cmd->fds[cmd->i].fd, "230 User logged in, proceed.\r\n", 30);
}

void pass_handling(command_t *cmd)
{
    char *buffer_ptr = cmd->buffer;

    if (cmd->clients[cmd->i].username[0] == '\0') {
        write(cmd->fds[cmd->i].fd, "503 Login with USER first.\r\n", 28);
        return;
    }
    if (cmd->clients[cmd->i].is_authenticated) {
        write(cmd->fds[cmd->i].fd, "530 User already connected.\r\n", 30);
        return;
    }
    while (*buffer_ptr == ' ')
        buffer_ptr++;
    if (strcmp(cmd->clients[cmd->i].username, "Anonymous") == 0) {
        handle_anonymous_login(cmd);
        return;
    }
    handle_standard_login(cmd);
}

void user_handling(command_t *cmd)
{
    char *args = cmd->buffer;
    char *space;

    while (*args == ' ')
        args++;
    if (*args == '\0') {
        write(cmd->fds[cmd->i].fd, "530 Usage: USER <username>\r\n", 29);
        return;
    }
    strncpy(cmd->clients[cmd->i].username, args, BUFFER_SIZE - 1);
    cmd->clients[cmd->i].username[BUFFER_SIZE - 1] = '\0';
    space = strchr(cmd->clients[cmd->i].username, ' ');
    if (space)
        *space = '\0';
    write(cmd->fds[cmd->i].fd, "331 User name okay, need password.\r\n", 36);
}

void cdup_handling(command_t *cmd)
{
    char current_path[PATH_MAX];

    if (!getcwd(current_path, PATH_MAX)) {
        write(cmd->fds[cmd->i].fd, "550 Retrieving directory failed.\r\n", 35);
        return;
    }
    if (strcmp(current_path, "/") == 0) {
        write(cmd->fds[cmd->i].fd, "550 Already at root directory.\r\n", 33);
        return;
    }
    if (chdir("..") == 0) {
        write(cmd->fds[cmd->i].fd, "200 Command okay.\r\n", 19);
    } else {
        write(cmd->fds[cmd->i].fd, "550 Failed to change directory.\r\n", 34);
    }
}

static void send_error_response(int fd, const char *message)
{
    write(fd, message, strlen(message));
}

void cwd_handling(command_t *cmd)
{
    char current_path[PATH_MAX];
    char *buffer_ptr = cmd->buffer;

    while (*buffer_ptr == ' ')
        buffer_ptr++;
    if (*buffer_ptr == '\0')
        return send_error_response(cmd->fds[cmd->i].fd,
            "501 Syntax error in parameters.\r\n");
    if (!getcwd(current_path, PATH_MAX))
        return send_error_response(cmd->fds[cmd->i].fd,
            "550 Retrieving directory failed.\r\n");
    if (strcmp(current_path, buffer_ptr) == 0)
        return send_error_response(cmd->fds[cmd->i].fd,
            "250 Already in the directory.\r\n");
    if (chdir(buffer_ptr) == 0) {
        write(cmd->fds[cmd->i].fd, "250 Requested action completed.\r\n", 34);
    } else {
        return send_error_response(cmd->fds[cmd->i].fd,
            "550 Failed to change directory.\r\n");
    }
}

void pwd_handling(command_t *cmd)
{
    char current_path[PATH_MAX];

    if (!getcwd(current_path, sizeof(current_path))) {
        write(cmd->fds[cmd->i].fd,
            "550 Failed to retrieve current directory.\r\n", 44);
        return;
    }
    write(cmd->fds[cmd->i].fd, "257 \"", 5);
    write(cmd->fds[cmd->i].fd, current_path, strlen(current_path));
    write(cmd->fds[cmd->i].fd, "\" is the current directory.\r\n", 29);
}
