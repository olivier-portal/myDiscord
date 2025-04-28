// Pour compiler ce code : gcc server.c -o server -lws2_32

#include <winsock2.h> // Pour les sockets Windows
#include <ws2tcpip.h> // Pour les focntionnalités de sockets TCP/IP
#include <stdio.h>
#include <string.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib") // Lien avec la bibliothèque Winsock

#define MAX_CLIENTS 5 // Nombre maximum de clients pouvant se connecter en même temps

DWORD WINAPI comm_client(LPVOID socket_desc); // Prototypage de la fonction qui gère l'interaction d'un client avec le serveur

DWORD WINAPI accept_connections(LPVOID param); // Prototypage de la fonction qui gère l'acceptation des connexions des clients

DWORD WINAPI stop_server(LPVOID param);

// structure pour permettre plusieurs paramètres à la fonction accept_connections
typedef struct {
    SOCKET listen_fd;
    HANDLE threads[MAX_CLIENTS]; // Tableau pour stocker les handles des threads (pour gérer plusieurs clients en même temps)
    int thread_count; // Compteur de threads
    HANDLE thread_count_mutex; // Mutex pour protéger l'accès au compteur de threads
    int stop_flag; // indicateur pour indiquer au thread d'acceptation de se terminer
    HANDLE stop_flag_mutex; // Mutex pour protéger l'accès à stop_flag
}AcceptConnectionsParamas;

int main() {
    WSADATA wsaData; // Pour intialiser Winsock
    SOCKET listen_fd; // Descripteurs de socket (fichiers seront utiliser pour écouter les connexions entrantes et pour communiquer avec le client)
    struct sockaddr_in servaddr; // Structure pour l'adresse du serveur

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
    if(listen(listen_fd, MAX_CLIENTS) == SOCKET_ERROR) { // 5 est le nombre maximum d'écouteurs en attente
        printf("Erreur lors de l'écoute : %d\n", WSAGetLastError());
        closesocket(listen_fd); 
        WSACleanup();
        return 1;
    }

    // création de la structure pour les paramètres de accpet_connections
    AcceptConnectionsParamas params;
    params.listen_fd = listen_fd;
    params.thread_count = 0; // Initialise le compteur de threads à 0
    memset(params.threads, 0, sizeof(params.threads)); // Initialise le tableau de threads à 0
    params.thread_count_mutex = CreateMutex(NULL, FALSE, NULL); // Crée un mutex pour protéger l'accès au compteur de threads
    params.stop_flag = 0; // Initialise le drapeau d'arrêt à 0 (ne pas arrêter)
    params.stop_flag_mutex = CreateMutex(NULL, FALSE, NULL); // Crée un mutex pour protéger l'accès au drapeau d'arrêt

    // créer un thread pour accepter les connexions
    HANDLE accept_thread = CreateThread(NULL, 0, accept_connections, (LPVOID)&params, 0, NULL);
    if (accept_thread == NULL) {
        printf("Erreur de création du thread d'acceptation : %d\n", GetLastError());
        closesocket(listen_fd);
        WSACleanup();
        return 1;
    }

    // créer un thread pour surveiller la condition d'arrêt du serveur
    HANDLE stop_thread = CreateThread(NULL, 0, stop_server, (LPVOID)&params, 0, NULL);
    if (stop_thread == NULL) {
        printf("Erreur de création du thread d'arrêt : %d\n", GetLastError());
        WaitForSingleObject(params.stop_flag_mutex, INFINITE); 
        params.stop_flag = 1;
        ReleaseMutex(params.stop_flag_mutex);
    }


    // attendre que le thread d'arrêt se termine 
    WaitForSingleObject(stop_thread, INFINITE);
    CloseHandle(stop_thread);

    // attendre que le thread d'acceptation se termine
    WaitForSingleObject(accept_thread, INFINITE);
    CloseHandle(accept_thread);

    // attendre que les threads se terminent
    if(params.thread_count > 0) {
        WaitForMultipleObjects(params.thread_count, params.threads, TRUE, INFINITE);
    }
    for(int i = 0; i<params.thread_count; i++) {
        CloseHandle(params.threads[i]); // Ferme les handles des threads de communication
    }

    // libérer le mutex
    CloseHandle(params.thread_count_mutex);
    CloseHandle(params.stop_flag_mutex);


    // Fermeture des sockets
    closesocket(listen_fd); // Ferme le socket d'écoute
    WSACleanup();
    printf("Serveur fermé.\n");

    return 0;
}


// Fonction pour gérer la communication avec un client
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

// Fonction pour accepter les connexions des clients
DWORD WINAPI accept_connections(LPVOID param) {
    AcceptConnectionsParamas*params = (AcceptConnectionsParamas*)param;
    SOCKET comm_fd; // Descripteur de socket pour la communication avec le client

    // Boucle infinie pour accepter les connexions des clients
    while (1) {
        // vérifier si le drapeau d'arrêt est activé
        WaitForSingleObject(params->stop_flag_mutex, INFINITE);
        if(params->stop_flag) {
            ReleaseMutex(params->stop_flag_mutex);
            break; // Sort de la boucle si le drapeau d'arrêt est activé
        }
        ReleaseMutex(params->stop_flag_mutex);

        // acceptation d'une connexion
        comm_fd = accept(params->listen_fd, (struct sockaddr*)NULL, NULL); // NULL car on ne se soucie pas de l'adresse du client ici
        if(comm_fd == INVALID_SOCKET) {
            printf("Erreur d'acceptation de connexion : %d\n", WSAGetLastError());
            continue; // Continue à accepter d'autres connexions
        }

        WaitForSingleObject(params->thread_count_mutex, INFINITE); // Acquiert le mutex pour protéger l'accès au compteur de threads

        // Nettoyer les threads terminés
        for (int i = 0; i < params->thread_count; i++) {
            DWORD exit_code;
            if (GetExitCodeThread(params->threads[i], &exit_code) && exit_code != STILL_ACTIVE) {
                // Si le thread est terminé, on le ferme et on le retire du tableau
                CloseHandle(params->threads[i]);

                // réorganisation du tableau pour combler le trou
                for (int j = i; j < params->thread_count -1; j++) {
                    params->threads[j] = params->threads[j+1];
                }
                params->thread_count--;
                i--; // Décrémente i pour vérifier le prochain thread
            }
        }

        // vérifier si un nouveau thread peut être crée
        if(params->thread_count < MAX_CLIENTS) {
            // crée un thread pour gérer la communication avec le client
            HANDLE new_thread = CreateThread(NULL, 0, comm_client, (LPVOID)comm_fd, 0, NULL);
            if(new_thread != NULL) {
                params->threads[params->thread_count] = new_thread; // Stocke le handle du thread dans le tableau
                params->thread_count++; // Incrémente le compteur de threads
            } else {
                printf("Erreur de création du thread client : %d\n", WSAGetLastError());
                closesocket(comm_fd);
            }
        } else {
            printf("Nombre maximum de clients atteint. Connexion refusée.\n");
            closesocket(comm_fd);
        }
        ReleaseMutex(params->thread_count_mutex);
    }

    return 0;
}

// fonction pour arrêter le serveur
DWORD WINAPI stop_server(LPVOID param) {
    AcceptConnectionsParamas*params = (AcceptConnectionsParamas*)params;

    // attendre que l'utilisateur appuie sur une touche pour arrêter le serveur
    printf("Appuyer sur une touche pour arrêter le serveur ...\n");
    getchar(); // Attendre que l'utilisateur appuie sur une touche

    // signaler au thread d'acceptation de se terminer 
    WaitForSingleObject(params->stop_flag_mutex, INFINITE);
    params->stop_flag = 1; // Définit le drapeau d'arrêt à 1 (arrêter)
    ReleaseMutex(params->stop_flag_mutex);

    printf("Arrêt du serveur demandé.\n");
    return 0;
}
