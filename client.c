// L'adresse de boucle locale, également connue sous le nom de "localhost", est l'adresse IP qui permet à un ordinateur de se connecter à lui-même. Voici quelques détails :
// IPv4 : L'adresse IP de boucle locale pour IPv4 est 127.0.0.1.
// IPv6 : Pour IPv6, l'adresse de boucle locale est ::1.
// Nom d'hôte : En plus des adresses IP, vous pouvez également utiliser le nom d'hôte localhost, qui est résolu en 127.0.0.1.





#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>

int main() {
    WSADATA wsaData;
    int sockfd; // Descrispteur de fichier pour le socket utilisée pour la communication
    char sendline[100];
    char recvline[100];
    struct sockaddr_in servaddr;

    // Initialisation de Wonsock
    if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0) {
        printf("Erreur d'initialisation de Winsock.\n");
        return 1;
    }

    // Création du socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == INVALID_SOCKET) {
        printf("Erreur de création du socket.\n");
        WSACleanup();
        return 1;
    }

    // Réinitialisation des buffers et de la Structure d'adresse
    memset(sendline, 0, sizeof(sendline)); 
    memset(recvline, 0, sizeof(recvline));
    memset(&servaddr, 0, sizeof(servaddr));

    // Configuration de l'adresse du serveur
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080); 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Adresse locale

    // Connexion au serveur
    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        printf("Erreur de connexion au serveur.\n");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // Boucle de communication 
    while(1) {
        printf("Votre message : ");
        fgets(sendline, sizeof(sendline), stdin);
        send(sockfd, sendline, strlen(sendline), 0);
        recv(sockfd, recvline, sizeof(recvline)-1, 0);
        recvline[sizeof(recvline)-1] = '\0';
        printf("Message du serveur : %s", recvline); 
    }
    
    // Fermeture du socket
    closesocket(sockfd);
    WSACleanup();

    return 0;
}