#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include "client_socket.h"

void run_client() {
    WSADATA wsaData;
    int sockfd;
    char sendline[500];
    char recvline[500];
    struct sockaddr_in servaddr;

    if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0) {
        printf("Erreur d'initialisation de Winsock.\n");
        return;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == INVALID_SOCKET) {
        printf("Erreur de création du socket.\n");
        WSACleanup();
        return;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        printf("Erreur de connexion au serveur.\n");
        closesocket(sockfd);
        WSACleanup();
        return;
    }

    while(1) {
        printf("\nVotre message (ou tapez 'exit' pour quitter): ");
        fgets(sendline, sizeof(sendline), stdin);
        sendline[strcspn(sendline, "\n")] = 0;

        if(send(sockfd, sendline, strlen(sendline), 0) == SOCKET_ERROR) {
            printf("Erreur d'envoi du message.\n");
            break;
        }

        if(strcmp(sendline, "exit") == 0) {
            printf("Fermeture de la connexion...\n");
            break;
        }

        memset(recvline, 0, sizeof(recvline));
        int recv_len = recv(sockfd, recvline, sizeof(recvline)-1, 0);
        if(recv_len <= 0) {
            printf("Erreur de réception du message.\n");
            break;
        }

        recvline[sizeof(recvline)-1] = '\0';
        printf("Message du serveur : %s", recvline);
        memset(sendline, 0, sizeof(sendline));
    }

    closesocket(sockfd);
    WSACleanup();
}

void start_client() {
    printf("🟢 Client intégré lancé\n");
    run_client();
}

#ifdef TEST_CLIENT_MAIN
int main() {
    run_client();
    return 0;
}
#endif