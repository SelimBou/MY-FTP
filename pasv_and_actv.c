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

static int get_free_port(void)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    struct sockaddr_in addr = {0};
    struct sockaddr_in sin = {0};
    socklen_t len = sizeof(sin);

    if (check_socket_creation(sockfd) < 0)
        return -1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0 ||
        getsockname(sockfd, (struct sockaddr *)&sin, &len) < 0 ||
        listen(sockfd, 1) < 0) {
        perror("Error socket setup");
        close(sockfd);
        return -1;
    }
    return ntohs(sin.sin_port);
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

void pasv_handling(command_t *cmd)
{
    struct sockaddr_in serv_addr;
    socklen_t addr_len = sizeof(serv_addr);
    char ip_str[INET_ADDRSTRLEN] = {0};
    int port = get_free_port();
    int p1;
    int p2;

    if (port < 0) {
        write(cmd->fds[cmd->i].fd, "425 Cannot open data connection\r\n", 33);
        return;
    }
    getsockname(cmd->fds[cmd->i].fd, (struct sockaddr *)&serv_addr, &addr_len);
    inet_ntop(AF_INET, &serv_addr.sin_addr, ip_str, sizeof(ip_str));
    format_ip_address(ip_str);
    p1 = port / 256;
    p2 = port % 256;
    send_pasv_response(cmd->fds[cmd->i].fd, ip_str, p1, p2);
    cmd->clients[cmd->i].pasv_conn = (pasv_connection_t){port, port, true};
}
