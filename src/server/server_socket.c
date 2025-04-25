#include "server_socket.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>

void start_server() {
    WSADATA wsaData;
    int listen_fd, comm_fd;
    struct sockaddr_in servaddr;
    char str[500];
    char sendline[500];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Error: Winsock did not init.\n");
        return;
    }

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == INVALID_SOCKET) {
        printf("Error: Socket creation failed.\n");
        WSACleanup();
        return;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(8080);

    if (bind(listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        printf("Error: Connexion to server failed.\n");
        closesocket(listen_fd);
        WSACleanup();
        return;
    }

    if (listen(listen_fd, 5) == SOCKET_ERROR) {
        printf("Error: listen to server failed.\n");
        closesocket(listen_fd);
        WSACleanup();
        return;
    }

    printf("🟢 Serveur lancé (intégré ou standalone)\n");

    comm_fd = accept(listen_fd, NULL, NULL);
    if (comm_fd == INVALID_SOCKET) {
        printf("Error: Connexion not accepted.\n");
        closesocket(listen_fd);
        WSACleanup();
        return;
    }

    while (1) {
        memset(str, 0, sizeof(str));
        memset(sendline, 0, sizeof(sendline));

        int str_received = recv(comm_fd, str, sizeof(str) - 1, 0);
        if (str_received <= 0) {
            printf("Error: Client deconnected or reception failed.\n");
            break;
        }

        str[str_received] = '\0';
        printf("Client: %s\n", str);

        if (strcmp(str, "exit") == 0) {
            printf("Le client a quitté la conversation.\n");
            break;
        }

        printf("Votre message : ");
        fgets(sendline, sizeof(sendline), stdin);
        sendline[strcspn(sendline, "\n")] = '\0';

        if (send(comm_fd, sendline, strlen(sendline), 0) == -1) {
            printf("Error: Message send failed.\n");
            break;
        }
    }

    closesocket(comm_fd);
    closesocket(listen_fd);
    WSACleanup();
}

// 🔹 Main standalone conditionnel
#ifdef TEST_SERVER_MAIN
int main() {
    start_server();
    return 0;
}
#endif
