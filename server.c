// Pour compiler ce code : gcc server.c -o server -lws2_32

#include <winsock2.h> // Pour les sockets Windows
#include <ws2tcpip.h> // Pour les focntionnalités de sockets TCP/IP
#include <stdio.h>
#include <string.h>

int main() {
    WSADATA wsaData; // Pour intialiser Winsock

    char str[500]; // Réception de la chaîne de caractères
    char sendline[500]; // Pour les messages envoyés
    memset(sendline, 0, 100); // Initialise sendline à 0 pour éviter la lecture de données indésirables

    int listen_fd, comm_fd; // Descripteurs de socket (fichiers seront utiliser pour écouter les connexions entrantes et pour communiquer avec le client)
    struct sockaddr_in servaddr; // Structure pour l'adresse du serveur

    // Initialisation de Winsock avec la version 2.2, et gestion de l'echec d'initialisation
    if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0) {
        printf("Erreur d'initialisation de Winsock.\n");
        return 1;
    }

    // connexion base de donnée

    // Création de la socket d'écoute (TCP)
    listen_fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET pour IPv4, SOCK_STREAM pour TCP
    if(listen_fd == INVALID_SOCKET) {
        printf("Erreur de création du socket.\n");
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
        printf("Erreur de liaison du socket.\n");
        closesocket(listen_fd); // Ferme le socket d'écoute
        WSACleanup();
        return 1;
    }

    // Ecoute des connexions 
    if(listen(listen_fd, 5) == SOCKET_ERROR) { // 5 est le nombre maximum d'écouteurs en attente
        printf("Erreur lors de l'écoute.\n");
        closesocket(listen_fd); 
        WSACleanup();
        return 1;
    }

    // Acceptation d'une connexion 
    // accept(): Attend qu'un client se connecte. Si une connexion est acceptée, elle retourne un nouveau descripteur de socket (comm_fd) pour la communication avec ce client.
    comm_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL); // NULL car on ne se soucie pas de l'adresse du client ici
    if(comm_fd == INVALID_SOCKET) {
        printf("Erreur d'acceptation de connexion.\n");
        closesocket(listen_fd);
        WSACleanup();
        return 1;
    }

    // Communication avec le client 
    // Booucle infinie pour recevoir et envoyer des messages
    while(1) {
        memset(str, 0, sizeof(str)); // Initialise str à 0 pour éviter la lecture de données indésirables
        memset(sendline, 0, sizeof(sendline)); // Initialise sendline à 0 pour éviter la lecture de données indésirables

        // Réception d'un message du client
        int str_received = recv(comm_fd, str, sizeof(str) -1, 0); // -1 pour laisser de la place pour le caractère nul
        if(str_received <= 0) { // Si la réception échoue ou si le client se déconnecte
            printf("Erreur de réception ou client déconnecté.\n");
            break;
        }

        str[str_received] = '\0'; // Ajoute un caractère nul à la fin de la chaîne recue pour la rendre une chaine C valide
        printf("Client: %s", str); 

        // Vérifie si le client a envoyé "exit" pour fermer la connexion
        if(strcmp(str, "exit") == 0) {
            printf("\nLe client a quitté la conversation.\n");
            break;
        }

        printf("\nVotre message : ");
        fgets(sendline, sizeof(sendline), stdin); //  Lit la ligne saisie au clavier et la stocke dans sendline 
        sendline[strcspn(sendline, "\n")] = '\0'; // Supprime le saut de ligne à la fin de la chaîne pour en faire une chaine de caractères C valide
        // D'ailleurs attention : la fonction fgets() ajoute un saut de ligne à la fin de la chaîne, donc il faut le supprimer manuellement.

        if(send(comm_fd, sendline, strlen(sendline), 0) == -1) {
            printf("Erreur d'envoi du message au client.\n");
            break;
        } // Envoie le message au client
    }

    // Fermeture des sockets
    closesocket(comm_fd); // Ferme le socket de communication
    closesocket(listen_fd); // Ferme le socket d'écoute
    WSACleanup();

    return 0;
}

