#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

void client_server(char **argv)
{
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));
    connect(client_socket, (struct sockaddr*) &server_addr, sizeof(struct sockaddr));

    char buffer[1024] = {0};
    int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);

    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        write(STDOUT_FILENO, "Server said: ", 13);
        write(STDOUT_FILENO, buffer, bytes_read);
    } else {
        perror("Erreur lors de la lecture du message du serveur");
    }

    close(client_socket);
}

int main(int argc, char **argv)
{
    client_server(argv);
}
