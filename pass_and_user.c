/*
** EPITECH PROJECT, 2024
** B-NWP-400-MAR-4-1-myftp-selim.bouasker
** File description:
** pass_&_user.c
*/

#include "ftp.h"

void handle_anonymous_login(command_t *cmd, const char *password)
{
    if (*password != '\0') {
        write(cmd->fds[cmd->i].fd, "530 Invalid password for Anonymous.\r\n",
            37);
        return;
    }
    cmd->clients[cmd->i].is_authenticated = 1;
    write(cmd->fds[cmd->i].fd, "230 User logged in, proceed.\r\n", 30);
}

void handle_standard_login(command_t *cmd, const char *password)
{
    if (*password == '\0') {
        write(cmd->fds[cmd->i].fd, "530 Usage: PASS <password>\r\n", 29);
        return;
    }
    cmd->clients[cmd->i].is_authenticated = 1;
    write(cmd->fds[cmd->i].fd, "230 User logged in, proceed.\r\n", 30);
}

void user_handling(const char *buffer, command_t *cmd)
{
    char username[BUFFER_SIZE] = {0};
    const char *args = buffer + 4;
    char *space;

    while (*args == ' ')
        args++;
    if (*args == '\0') {
        write(cmd->fds[cmd->i].fd, "530 Usage: USER <username>\r\n", 29);
        return;
    }
    strncpy(username, args, BUFFER_SIZE - 1);
    username[BUFFER_SIZE - 1] = '\0';
    space = strchr(username, ' ');
    if (space)
        *space = '\0';
    strncpy(cmd->clients[cmd->i].username, username, BUFFER_SIZE - 1);
    cmd->clients[cmd->i].username[BUFFER_SIZE - 1] = '\0';
    write(cmd->fds[cmd->i].fd, "331 User name okay, need password.\r\n", 36);
}
