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

int check_data_connection(command_t *cmd, int sockfd)
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

static port_arguments_t parse_port_arguments(char *args)
{
    port_arguments_t result;
    int h1;
    int h2;
    int h3;
    int h4;
    int p1;
    int p2;

    if (sscanf(args, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2) != 6) {
        result.success = false;
        return result;
    }
    result.client_addr.sin_family = AF_INET;
    result.client_addr.sin_port = htons((p1 << 8) + p2);
    result.client_addr.sin_addr.s_addr = htonl((h1 << 24) | (h2 << 16)
        | (h3 << 8) | h4);
    result.success = true;
    return result;
}

void port_handling(command_t *cmd)
{
    char *args = cmd->buffer + 5;
    port_arguments_t parsed_args;
    int data_socket = socket(AF_INET, SOCK_STREAM, 0);

    parsed_args = parse_port_arguments(args);
    if (!parsed_args.success) {
        write(cmd->fds[cmd->i].fd, "501 Syntax error in parameters.\r\n", 33);
        return;
    }
    if (check_data_connection(cmd, data_socket) < 0)
        return;
    if (connect(data_socket, (struct sockaddr *)&parsed_args.client_addr,
        sizeof(parsed_args.client_addr)) < 0) {
        close(data_socket);
        write(cmd->fds[cmd->i].fd, "425 Can't open data connection.\r\n", 32);
        return;
    }
    cmd->clients[cmd->i].port_conn.data_socket = data_socket;
    cmd->clients[cmd->i].port_conn.is_active = true;
    write(cmd->fds[cmd->i].fd, "200 PORT command successful.\r\n", 30);
}
