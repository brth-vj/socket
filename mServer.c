#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int serverSocket();
void handleConnection(int server_fd, int *client_sockets);
void handleMessage(int i, int *client_sockets, fd_set *readfds);

int main() {
    int server_fd, client_sockets[MAX_CLIENTS] = {0};
    fd_set readfds;
    int max_sd, activity, i,sd;

    server_fd = serverSocket();

    printf("Server started on port %d\n", PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            continue;
        }

        if (FD_ISSET(server_fd, &readfds))
            handleConnection(server_fd, client_sockets);

        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds))
                handleMessage(i, client_sockets, &readfds);
        }
    }

    return 0;
}

int serverSocket() {
    int server_fd;
    struct sockaddr_in address;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        exit(1);
    }

//    int opt = 1;
  //  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(1);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(1);
    }

    return server_fd;
}

void handleConnection(int server_fd, int *client_sockets) {
    int new_socket,i;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (new_socket < 0) {
        perror("accept failed");
        return;
    }

    printf("New client connected: socket %d, IP: %s, PORT: %d\n",
           new_socket,
           inet_ntoa(address.sin_addr),
           ntohs(address.sin_port));

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == 0) {
            client_sockets[i] = new_socket;
            break;
        }
    }
}

void handleMessage(int i, int *client_sockets, fd_set *readfds) {
    int sd = client_sockets[i],j,other_sd;
    char buffer[BUFFER_SIZE];
    int valread = read(sd, buffer, sizeof(buffer));

    if (valread <= 0) {
        printf("Client disconnected (socket %d)\n", sd);
        close(sd);
        client_sockets[i] = 0;
    } else {
        buffer[valread] = '\0';
        printf("Message from client %d: %s\n", sd, buffer);

        // Broadcast except the sender
        for (j = 0; j < MAX_CLIENTS; j++) {
            other_sd = client_sockets[j];
            if (other_sd != 0 && other_sd != sd) {
                send(other_sd, buffer, strlen(buffer), 0);
            }
        }
    }
}
