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

static void noop_handling(command_t *cmd)
{
    write(cmd->fds[cmd->i].fd, "200 NOOP command okay.\r\n", 24);
}

static void help_handling(command_t *cmd)
{
    const char *help_msg =
        "214 All commands: USER, PASS, CWD, CDUP, QUIT, PWD, NOOP, HELP\r\n";

    write(cmd->fds[cmd->i].fd, help_msg, strlen(help_msg));
}

void check_command_2(char *buffer, command_t *cmd, char *path)
{
    if (is_valid_command(buffer, "CWD", 3)) {
        cwd_handling(buffer, cmd, path);
        return;
    }
    if (is_valid_command(buffer, "CDUP", 4)) {
        cdup_handling(cmd, path);
        return;
    }
    if (is_valid_command(buffer, "NOOP", 4)) {
        noop_handling(cmd);
        return;
    }
    if (is_valid_command(buffer, "HELP", 4)) {
        help_handling(cmd);
        return;
    }
    write(cmd->fds[cmd->i].fd, "500 Commande non reconnue\r\n", 27);
}
