// L'adresse de boucle locale, également connue sous le nom de "localhost", est l'adresse IP qui permet à un ordinateur de se connecter à lui-même. Voici quelques détails :
// IPv4 : L'adresse IP de boucle locale pour IPv4 est 127.0.0.1.
// IPv6 : Pour IPv6, l'adresse de boucle locale est ::1.
// Nom d'hôte : En plus des adresses IP, vous pouvez également utiliser le nom d'hôte localhost, qui est résolu en 127.0.0.1.

// Pour compiler ce code : gcc client.c -o client -lws2_32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <stdlib.h> 

DWORD WINAPI send_message(LPVOID socket_desc); // Prototypage de la fonction d'envoi de message
// prototypage de la fonction de réception du message? à vois plus tard

int main() {
    WSADATA wsaData;
    SOCKET sockfd; // Descrispteur de fichier pour le socket utilisée pour la communication
    char recvline[500];
    struct sockaddr_in servaddr;
    DWORD exit_code; // Code de sortie du thread d'envoi
    BOOL still_active; // Indique si le thread d'envoi est toujours actif

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

    // Création d'un thread pour envoyer des messages
    HANDLE send_thread = CreateThread(NULL, 0, send_message, (LPVOID)sockfd, 0, NULL);
    // attribut de CreateThread():
    // NULL : pas d'attribut de sécurité (par défault)
    // 0 : pas de taille de pile spécifique (par défault)
    // send_message : fonction à exécuter dans le thread
    // (LPVOID)sockfd : passe le descripteur de socket comme argument à la fonction
    // 0 : pas de paramètre de création spécifique (par défault)
    // NULL : pas d'identifiant de thread spécifique (par défault)

    // vérifie si le thread a été créé avec succés
    if(send_thread == NULL) {
        printf("Erreur de création du thread d'envoi.\n");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // Boucle de réception de messages
    while(1) {
        // Réception de la réponse du serveur
        memset(recvline, 0, sizeof(recvline));
        int recv_len = recv(sockfd, recvline, sizeof(recvline)-1, 0);
        if(recv_len <= 0) {
            printf("Erreur de réception du message.\n");
            break;
        }

        recvline[sizeof(recvline)-1] = '\0';
        printf("Message du serveur : %s", recvline); 

        // vérification de l'état du thread d'envoi
        still_active = GetExitCodeThread(send_thread, &exit_code);
        if(still_active && exit_code != STILL_ACTIVE) {
            printf("Le thread s'est terminé avec le code de sortie : %lu\n", exit_code);
            break;
        }
    }


    // attendre que le thread d'envoi se termine, 
    // WaitForSingleObject "vérifie" si le thread d'envoi (géré par la fonction send_messages) 
    // a terminé son exécution. Si le thread d'envoi est toujours 
    // actif (par exemple, si l'utilisateur n'a pas encore entré "exit"), le thread principal restera bloqué à cet appel.
    WaitForSingleObject(send_thread, INFINITE);
    // fermeture du thread d'envoi
    CloseHandle(send_thread);
    
    // Fermeture du socket
    closesocket(sockfd);
    WSACleanup();

    return 0;
}

// Fonction d'envoi de message
DWORD WINAPI send_message(LPVOID socket_desc) {
    SOCKET sockfd = (SOCKET)socket_desc; // Cast du socket_desc en SOCKET
    char sendline[500];

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

        memset(sendline, 0, sizeof(sendline));
    }

    return 0;
}