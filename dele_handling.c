/*
** EPITECH PROJECT, 2024
** B-NWP-400-MAR-4-1-myftp-selim.bouasker
** File description:
** dele_handling.c
*/

#include "ftp.h"

static int can_delete_file(char *cleaned_path, int client_fd)
{
    struct stat path_stat;

    if (access(cleaned_path, F_OK) != 0) {
        write(client_fd, "550 File not found.\r\n", 21);
        return 0;
    }
    if (stat(cleaned_path, &path_stat) != 0) {
        write(client_fd, "550 Could not retrieve file info.\r\n", 32);
        return 0;
    }
    if (S_ISDIR(path_stat.st_mode)) {
        write(client_fd, "550 Cannot delete a directory.\r\n", 32);
        return 0;
    }
    if (access(cleaned_path, W_OK) != 0) {
        write(client_fd, "550 Access denied.\r\n", 20);
        return 0;
    }
    return 1;
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
    if (!can_delete_file(cleaned_path, cmd->fds[cmd->i].fd))
        return;
    printf("%s\n", cleaned_path);
    if (remove(cleaned_path) == 0) {
        write(cmd->fds[cmd->i].fd, "250 File action okay, completed.\r\n", 34);
    } else {
        perror("remove");
        write(cmd->fds[cmd->i].fd, "550 Could not delete file.\r\n", 28);
    }
}
