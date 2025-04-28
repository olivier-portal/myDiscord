// Pour compiler ce code : gcc server.c -o server -lws2_32

#include <winsock2.h> // Pour les sockets Windows
#include <ws2tcpip.h> // Pour les focntionnalités de sockets TCP/IP
#include <stdio.h>
#include <string.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib") // Lien avec la bibliothèque Winsock

#define MAX_CLIENTS 5 // Nombre maximum de clients pouvant se connecter en même temps

DWORD WINAPI comm_client(LPVOID socket_desc); // Prototypage de la fonction qui gère l'interaction d'un client avec le serveur

int main() {
    WSADATA wsaData; // Pour intialiser Winsock
    SOCKET listen_fd, comm_fd; // Descripteurs de socket (fichiers seront utiliser pour écouter les connexions entrantes et pour communiquer avec le client)
    struct sockaddr_in servaddr; // Structure pour l'adresse du serveur
    HANDLE threads[MAX_CLIENTS]; // Tableau pour stocker les handles des threads (pour gérer plusieurs clients en même temps)
    int thread_count = 0; // Compteur de threads

    // Initialisation de Winsock avec la version 2.2, et gestion de l'echec d'initialisation
    if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0) {
        printf("Erreur d'initialisation de Winsock : %d\n", WSAGetLastError());
        return 1;
    }

    // connexion base de donnée

    // Création de la socket d'écoute (TCP)
    listen_fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET pour IPv4, SOCK_STREAM pour TCP
    if(listen_fd == INVALID_SOCKET) {
        printf("Erreur de création du socket : %d\n", WSAGetLastError());
        WSACleanup(); // Libération des ressources de Winsock
        return 1;
    }

    // Configuration de l'adresse du serveur
    memset(&servaddr, 0, sizeof(servaddr)); // Initialise servaddr à 0 pour éviter la lecture de données indésirables

    // Définition de l'adresse du serveur
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Accepte toutes les addresses IP
    servaddr.sin_port = htons(8080); // Configure le port d'écoute sur 8080 

    // Laison du socket à l'adresse du serveur
    // bind(): Associe le socket à l'adresse et au port définis dans servaddr. Cela permet au socket d'écouter les connexions entrantes à cette adresse et ce port.
    // SOCKET_ERROR: Vérifie si bind() a échoué, et si c'est le cas, il nettoie les ressources et quitte le programme.
    if(bind(listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        printf("Erreur de liaison du socket : %d\n", WSAGetLastError());
        closesocket(listen_fd); // Ferme le socket d'écoute
        WSACleanup();
        return 1;
    }

    // Ecoute des connexions 
    if(listen(listen_fd, 5) == SOCKET_ERROR) { // 5 est le nombre maximum d'écouteurs en attente
        printf("Erreur lors de l'écoute : %d\n", WSAGetLastError());
        closesocket(listen_fd); 
        WSACleanup();
        return 1;
    }

    // boucle d'acceptation d'une connexion 
    // accept(): Attend qu'un client se connecte. Si une connexion est acceptée, elle retourne un nouveau descripteur de socket (comm_fd) pour la communication avec ce client.
    while (1) {
        comm_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL); // NULL car on ne se soucie pas de l'adresse du client ici
        if(comm_fd == INVALID_SOCKET) {
            printf("Erreur d'aceptation de connexion: %d\n", WSAGetLastError());
            closesocket(listen_fd);
            WSACleanup();
            return 1;
        }

        // Vérifie si un nouveau thread peut être créé 
        if(thread_count < MAX_CLIENTS) {
            // Crée un thread pour gérer la communication avec le client
            threads[thread_count] = CreateThread(NULL, 0, comm_client, (LPVOID)comm_fd, 0, NULL);
            if(threads[thread_count] == NULL) {
                printf("Erreur de création du thread : %d\n", WSAGetLastError());
                closesocket(comm_fd);
            } else {
                CloseHandle(threads[thread_count]); // Ferme le handle du thread, mais pas le thread lui-même
                thread_count++; // incrémente le compteur de threads
            }
        } else {
            printf("Nombre maximum de clients atteint. Connexion refusée.\n");
            closesocket(comm_fd);
        }
    }

    // attendre que les threads se terminent
    WaitForMultipleObjects(thread_count, threads, TRUE, INFINITE);

    // Fermeture des sockets
    closesocket(listen_fd); // Ferme le socket d'écoute
    WSACleanup();
    printf("Serveur fermé.\n");

    return 0;
}


DWORD WINAPI comm_client(LPVOID socket_desc) {
    SOCKET comm_fd = (SOCKET)socket_desc;
    char rcvline[500]; // Réception de la chaîne de caractères
    int bytesReceived; // Nombre d'octets reçus


    // Communication avec le client 
    // Booucle infinie pour recevoir
    while(1) {
        memset(rcvline, 0, sizeof(rcvline)); // Initialise rcvline à 0 pour éviter la lecture de données indésirables

        // Réception d'un message du client
        bytesReceived = recv(comm_fd, rcvline, sizeof(rcvline) -1, 0); // -1 pour laisser de la place pour le caractère nul
        if(bytesReceived <= 0) { // Si la réception échoue ou si le client se déconnecte
            printf("Erreur de réception ou client déconnecté.\n");
            break;
        }

        rcvline[bytesReceived] = '\0'; // Ajoute un caractère nul à la fin de la chaîne recue pour la rendre une chaine C valide
        printf("\nClient: %s", rcvline); 

        // Vérifie si le client a envoyé "exit" pour fermer la connexion
        if(strcmp(rcvline, "exit") == 0) {
            printf("\nLe client a quitté la conversation.\n");
            break;
        }
    }

    closesocket(comm_fd); // Ferme le socket de communication
    return 0;
}
