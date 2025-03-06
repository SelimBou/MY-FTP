/*
** EPITECH PROJECT, 2024
** B-NWP-400-MAR-4-1-myftp-selim.bouasker
** File description:
** pasv_and_actv.c
*/

#include "ftp.h"

static int check_socket_creation(int sockfd)
{
    if (sockfd < 0) {
        perror("Error creating socket");
        return -1;
    }
    return 0;
}

static int get_pasv_socket(struct sockaddr_in *addr)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;

    if (check_socket_creation(sockfd) < 0)
        return -1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr("127.0.0.1");
    addr->sin_port = 0;
    if (bind(sockfd, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
        close(sockfd);
        return -1;
    }
    if (listen(sockfd, 1) < 0) {
        perror("listen");
        close(sockfd);
        return -1;
    }
    return sockfd;
}

static void format_ip_address(char *ip_str)
{
    for (int j = 0; ip_str[j]; j++) {
        if (ip_str[j] == '.') {
            ip_str[j] = ',';
        }
    }
}

static void send_pasv_response(int fd, const char *ip_str, int p1, int p2)
{
    char response[128] = {0};

    snprintf(response, sizeof(response),
        "227 Entering Passive Mode (%s,%d,%d).\r\n", ip_str, p1, p2);
    write(fd, response, strlen(response));
}

static int check_data_connection(command_t *cmd, int sockfd)
{
    if (sockfd < 0) {
        write(cmd->fds[cmd->i].fd, "425 Cannot open data connection\r\n", 33);
        return - 1;
    }
    return 0;
}

void pasv_handling(command_t *cmd)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    char ip_str[INET_ADDRSTRLEN] = {0};
    int sockfd = get_pasv_socket(&addr);
    int port;
    int p1;
    int p2;

    if (check_data_connection(cmd, sockfd) < 0)
        return;
    getsockname(sockfd, (struct sockaddr *)&addr, &addr_len);
    port = ntohs(addr.sin_port);
    inet_ntop(AF_INET, &addr.sin_addr, ip_str, sizeof(ip_str));
    format_ip_address(ip_str);
    p1 = port / 256;
    p2 = port % 256;
    send_pasv_response(cmd->fds[cmd->i].fd, ip_str, p1, p2);
    cmd->clients[cmd->i].pasv_conn.data_socket = sockfd;
    cmd->clients[cmd->i].pasv_conn.port = port;
    cmd->clients[cmd->i].pasv_conn.is_active = true;
}

int open_data_connection(command_t *cmd)
{
    int data_socket = accept(cmd->clients[cmd->i].pasv_conn.data_socket,
        NULL, NULL);

    if (data_socket < 0) {
        perror("STOR: Error accepting data connection");
        write(cmd->fds[cmd->i].fd, "425 Can't open data connection.\r\n", 33);
    }
    return data_socket;
}

void receive_file(command_t *cmd, int data_socket, int file_fd)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    write(cmd->fds[cmd->i].fd, "150 Opening data connection.\r\n", 30);
    bytes_read = read(data_socket, buffer, sizeof(buffer));
    while (bytes_read > 0) {
        write(file_fd, buffer, bytes_read);
        bytes_read = read(data_socket, buffer, sizeof(buffer));
    }
    close(data_socket);
    close(file_fd);
    write(cmd->fds[cmd->i].fd, "226 Transfer complete.\r\n", 24);
}

static int check_stor_errors(command_t *cmd, char *filename)
{
    if (!filename) {
        write(cmd->fds[cmd->i].fd, "501 Syntax error in parameters.\r\n", 32);
        return 1;
    }
    if (!cmd->clients[cmd->i].pasv_conn.is_active) {
        write(cmd->fds[cmd->i].fd, "425 Use PASV first.\r\n", 21);
        return 1;
    }
    return 0;
}

void stor_handling(command_t *cmd)
{
    char *filename = strtok(NULL, " \r\n");
    int file_fd;
    int data_socket;

    if (check_stor_errors(cmd, filename)) {
        return;
    }
    file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (file_fd < 0) {
        perror("STOR: Error opening file");
        write(cmd->fds[cmd->i].fd, "550 Cannot create file.\r\n", 25);
        return;
    }
    data_socket = open_data_connection(cmd);
    if (data_socket >= 0)
        receive_file(cmd, data_socket, file_fd);
    else
        close(file_fd);
}
