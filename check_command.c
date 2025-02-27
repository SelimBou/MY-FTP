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

void check_command_2(char *buffer, command_t *cmd)
{
    if (is_valid_command(buffer, "CDUP", 4)) {
        cdup_handling(cmd);
        return;
    }
    write(cmd->fds[cmd->i].fd, "500 Commande non reconnue\r\n", 27);
}
