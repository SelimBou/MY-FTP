/*
** EPITECH PROJECT, 2024
** B-NWP-400-MAR-4-1-myftp-selim.bouasker
** File description:
** commands.c
*/

#include "ftp.h"

static void handle_anonymous_login(command_t *cmd, char *buffer_ptr)
{
    if (*buffer_ptr != '\0') {
        write(cmd->fds[cmd->i].fd,
            "530 Invalid password for Anonymous.\r\n", 37);
        return;
    }
    cmd->clients[cmd->i].is_authenticated = 1;
    write(cmd->fds[cmd->i].fd, "230 User logged in, proceed.\r\n", 30);
}

static void handle_standard_login(command_t *cmd, char *buffer_ptr)
{
    if (*buffer_ptr == '\0') {
        write(cmd->fds[cmd->i].fd, "530 Usage: PASS <password>\r\n", 28);
        return;
    }
    write(cmd->fds[cmd->i].fd, "530 Incorrect pwd.\r\n", 20);
}

void pass_handling(command_t *cmd)
{
    char *buffer_ptr = cmd->buffer;

    while (*buffer_ptr && !isspace(*buffer_ptr))
        buffer_ptr++;
    while (*buffer_ptr && isspace(*buffer_ptr))
        buffer_ptr++;
    if (cmd->clients[cmd->i].username[0] == '\0') {
        write(cmd->fds[cmd->i].fd, "503 Login with USER first.\r\n", 28);
        return;
    }
    if (cmd->clients[cmd->i].is_authenticated) {
        write(cmd->fds[cmd->i].fd, "530 User already connected.\r\n", 29);
        return;
    }
    if (strcmp(cmd->clients[cmd->i].username, "Anonymous") == 0) {
        handle_anonymous_login(cmd, buffer_ptr);
        return;
    }
    handle_standard_login(cmd, buffer_ptr);
}

void user_handling(command_t *cmd)
{
    char *args = cmd->buffer + 4;
    char *space;

    while (*args == ' ')
        args++;
    if (*args == '\0') {
        write(cmd->fds[cmd->i].fd, "530 Usage: USER <username>\r\n", 28);
        return;
    }
    strncpy(cmd->clients[cmd->i].username, args, BUFFER_SIZE - 1);
    cmd->clients[cmd->i].username[BUFFER_SIZE - 1] = '\0';
    space = strchr(cmd->clients[cmd->i].username, '\n');
    if (space)
        *space = '\0';
    space = strchr(cmd->clients[cmd->i].username, ' ');
    if (space)
        *space = '\0';
    write(cmd->fds[cmd->i].fd, "331 User name okay, need password.\r\n", 36);
}

void cdup_handling(command_t *cmd)
{
    char current_path[PATH_MAX];

    if (!getcwd(current_path, PATH_MAX)) {
        write(cmd->fds[cmd->i].fd, "550 Retrieving directory failed.\r\n", 34);
        return;
    }
    if (strcmp(current_path, "/") == 0) {
        write(cmd->fds[cmd->i].fd, "550 Already at root directory.\r\n", 32);
        return;
    }
    if (chdir("..") == 0) {
        write(cmd->fds[cmd->i].fd, "200 Command okay.\r\n", 19);
    } else {
        write(cmd->fds[cmd->i].fd, "550 Failed to change directory.\r\n", 33);
    }
}

static void send_error_response(int fd, const char *message)
{
    write(fd, message, strlen(message));
}

static void resolve_requested_path(command_t *cmd,
    char *buffer_ptr, char *requested_path)
{
    while (*buffer_ptr == ' ')
        buffer_ptr++;
    if (*buffer_ptr == '\0') {
        send_error_response(cmd->fds[cmd->i].fd,
            "501 Syntax error in parameters.\r\n");
        return;
    }
    if (buffer_ptr[0] == '/') {
        snprintf(requested_path, PATH_MAX, "%s", buffer_ptr);
    } else {
        snprintf(requested_path, PATH_MAX, "%s/%s",
            cmd->clients[cmd->i].cwd, buffer_ptr);
    }
}

static void change_directory(command_t *cmd, char *requested_path)
{
    char new_path[PATH_MAX];

    if (access(requested_path, F_OK) != 0) {
        send_error_response(cmd->fds[cmd->i].fd,
            "550 Directory not found.\r\n");
        return;
    }
    if (chdir(requested_path) != 0) {
        send_error_response(cmd->fds[cmd->i].fd,
            "550 Failed to change directory.\r\n");
        return;
    }
    if (!getcwd(new_path, sizeof(new_path))) {
        send_error_response(cmd->fds[cmd->i].fd,
            "550 Retrieving directory failed.\r\n");
        return;
    }
    snprintf(cmd->clients[cmd->i].cwd, PATH_MAX, "%s", new_path);
    send_error_response(cmd->fds[cmd->i].fd,
        "250 Directory successfully changed.\r\n");
}

void cwd_handling(command_t *cmd)
{
    char requested_path[PATH_MAX];

    resolve_requested_path(cmd, cmd->buffer + 4, requested_path);
    change_directory(cmd, requested_path);
}

void pwd_handling(command_t *cmd)
{
    char current_path[PATH_MAX];

    if (!getcwd(current_path, sizeof(current_path))) {
        write(cmd->fds[cmd->i].fd,
            "550 Failed to retrieve current directory.\r\n", 43);
        return;
    }
    write(cmd->fds[cmd->i].fd, "257 \"", 5);
    write(cmd->fds[cmd->i].fd, current_path, strlen(current_path));
    write(cmd->fds[cmd->i].fd, "\" is the current directory.\r\n", 29);
}
