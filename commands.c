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

static int check_cwd_errors(command_t *cmd, char *path, char *current_path)
{
    while (*path == ' ') {
        path++;
    }
    if (*path == '\0') {
        write(cmd->fds[cmd->i].fd, "501 Syntax error in parameters.\r\n", 33);
        return -1;
    }
    if (!getcwd(current_path, PATH_MAX)) {
        write(cmd->fds[cmd->i].fd, "550 Retrieving directory failed.\r\n", 42);
        return -1;
    }
    if (strcmp(current_path, path) == 0) {
        write(cmd->fds[cmd->i].fd, "250 Already in the directory.\r\n", 42);
        return -1;
    }
    if (strcmp(current_path, "/") == 0 && strcmp(path, "..") == 0) {
        write(cmd->fds[cmd->i].fd, "550 Already at root directory.\r\n", 34);
        return -1;
    }
    return 0;
}

void cwd_handling(const char *buffer, command_t *cmd, char *path)
{
    char current_path[PATH_MAX];

    if (!cmd || !cmd->fds || !cmd->clients || !cmd->nfds || !path) {
        write(2, "Erreur: paramètres invalides dans cwd_handling.\n", 50);
        return -1;
    }
    if (check_cwd_errors(cmd, path, current_path) == -1)
        return;
    if (chdir(path) == 0) {
        write(cmd->fds[cmd->i].fd,
            "250 Requested file action okay, completed.\r\n", 44);
    } else {
        write(cmd->fds[cmd->i].fd, "550 Failed to change directory.\r\n", 34);
    }
}

static int check_cdup_errors(command_t *cmd, char *path, char *current_path)
{
    if (!cmd || !cmd->fds || !cmd->clients || !cmd->nfds) {
        write(2, "Erreur: paramètres invalides dans cdup_handling.\n", 50);
        return -1;
    }
    if (!getcwd(current_path, PATH_MAX)) {
        write(cmd->fds[cmd->i].fd, "550 Retrieving directory failed.\r\n", 42);
        return -1;
    }
    if (strcmp(current_path, "/") == 0) {
        write(cmd->fds[cmd->i].fd, "550 Already at root directory.\r\n", 34);
        return -1;
    }
    if (path && strcmp(current_path, path) == 0) {
        write(cmd->fds[cmd->i].fd,
            "550 Cannot go above the initial root directory.\r\n", 48);
        return -1;
    }
    return 0;
}

void cdup_handling(command_t *cmd, char *path)
{
    char current_path[PATH_MAX];

    if (check_cdup_errors(cmd, path, current_path) == -1)
        return;
    if (chdir("..") == 0) {
        write(cmd->fds[cmd->i].fd, "200 Command okay.\r\n", 19);
    } else {
        write(cmd->fds[cmd->i].fd, "550 Failed to change directory.\r\n", 34);
    }
}
