/*
** EPITECH PROJECT, 2024
** B-NWP-400-MAR-4-1-myftp-selim.bouasker
** File description:
** check_command.c
*/

#include "ftp.h"

bool is_valid_command(const char *buffer, const char *cmd, int len)
{
    return (strncasecmp(buffer, cmd, len) == 0 &&
            (buffer[len] == ' ' || buffer[len] == '\0'));
}

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
            "501 Syntax error in parameters or arguments.\r\n", 45);
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

void list_handling(command_t *cmd)
{
    struct dirent *entry;

    if (!cmd->clients[cmd->i].dir) {
        cmd->clients[cmd->i].dir = opendir(cmd->clients[cmd->i].cwd);
        if (!cmd->clients[cmd->i].dir) {
            write(cmd->fds[cmd->i].fd, "550 Failed to open directory.\r\n", 31);
            return;
        }
    }
    write(cmd->fds[cmd->i].fd, "150 Here comes the directory listing.\r\n", 38);
    while ((entry = readdir(cmd->clients[cmd->i].dir)) != NULL) {
        write(cmd->fds[cmd->i].fd, entry->d_name, strlen(entry->d_name));
        write(cmd->fds[cmd->i].fd, "\n", 1);
    }
    write(cmd->fds[cmd->i].fd, "226 Directory send OK.\r\n", 24);
}
