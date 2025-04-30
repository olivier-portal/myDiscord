// Pour compiler ce code : gcc server.c -o server -lws2_32

#include <winsock2.h> // Pour les sockets Windows
#include <ws2tcpip.h> // Pour les focntionnalités de sockets TCP/IP
#include <stdio.h>
#include <string.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib") // Lien avec la bibliothèque Winsock

#define MAX_CLIENTS 10 // Nombre maximum de clients pouvant se connecter en même temps
#define BUFFER_SIZE 1024 // Taille du buffer pour la communication
#define PORT 8080 // Port d'écoute du serveur
#define SERVER_IP "127.0.0.1" // Adresse IP du serveur (localhost)

typedef struct ClientData ClientData; // Déclaration anticipée de la structure ClientData
typedef struct ServerState ServerState; // Déclaration anticipée de la structure ServerState

// structure pour permettre plusieurs paramètres à la fonction accept_connections
typedef struct ServerState {
    SOCKET listen_fd;
    ClientData *clients[MAX_CLIENTS]; // tableau pour stocker les clients (les instances de ClientData)
    int stop_flag; // indicateur pour indiquer au thread d'acceptation de se terminer
    HANDLE stop_flag_mutex; // Mutex pour protéger l'accès à stop_flag
    char buffer[BUFFER_SIZE]; // Buffer pour la communication
    HANDLE buffer_mutex; // Mutex pour protéger l'accès au buffer
}ServerState;

typedef struct ClientData {
    SOCKET socket; // Descripteur de socket pour la communication avec le client
    int client_id; // Identifiant du client
    HANDLE thread; // Handle du thread de communication associé au client
    ServerState *state;
}ClientData; // Structure pour stocker les informations de connexion d'un client


int count_connected_clients(ClientData *clients[], int max_clients); // Prototypage de la fonction qui compte le nombre de clients connectés
int find_free_index(ClientData *clients[], int max_clients); // Prototypage de la fonction qui trouve un index libre dans le tableau de clients
DWORD WINAPI comm_client(LPVOID param); // Prototypage de la fonction qui gère l'interaction d'un client avec le serveur
DWORD WINAPI accept_connections(LPVOID param); // Prototypage de la fonction qui gère l'acceptation des connexions des clients
DWORD WINAPI stop_server(LPVOID param); // Prototypage de la fonction qui gère l'arrêt du serveur
DWORD WINAPI send_message(LPVOID param); // Prototypage de la fonction qui envoie un message à tous les clients connectés



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
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP); // Accepte toutes les addresses IP
    servaddr.sin_port = htons(PORT); // Configure le port d'écoute sur 8080 

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
    if(listen(listen_fd, MAX_CLIENTS) == SOCKET_ERROR) { 
        printf("Erreur lors de l'écoute : %d\n", WSAGetLastError());
        closesocket(listen_fd); 
        WSACleanup();
        return 1;
    }

    // création d'une instance state la structure ServerState gérer les paramètre du serveur et les connexion des clients
    ServerState state;
    state.listen_fd = listen_fd;
    memset(state.clients, 0, sizeof(state.clients)); // Initialise le tableau de threads à 0
    state.stop_flag = 0; // Initialise le drapeau d'arrêt à 0 (ne pas arrêter)
    state.stop_flag_mutex = CreateMutex(NULL, FALSE, NULL); // Crée un mutex pour protéger l'accès au drapeau d'arrêt
    memset(state.buffer, 0, sizeof(state.buffer)); // Initialise le buffer à 0
    state.buffer_mutex = CreateMutex(NULL, FALSE, NULL); // Crée un mutex pour protéger l'accès au buffer


    // créer un thread pour accepter les connexions
    HANDLE accept_thread = CreateThread(NULL, 0, accept_connections, (LPVOID)&state, 0, NULL);
    if (accept_thread == NULL) {
        printf("Erreur de création du thread d'acceptation : %d\n", GetLastError());
        closesocket(listen_fd);
        WSACleanup();
        return 1;
    }

    // créer un thread pour surveiller la condition d'arrêt du serveur
    HANDLE stop_thread = CreateThread(NULL, 0, stop_server, (LPVOID)&state, 0, NULL);
    if (stop_thread == NULL) {
        printf("Erreur de création du thread d'arrêt : %d\n", GetLastError());
        WaitForSingleObject(state.stop_flag_mutex, INFINITE); 
        state.stop_flag = 1;
        ReleaseMutex(state.stop_flag_mutex);
    }


    // attendre que le thread d'arrêt se termine 
    WaitForSingleObject(stop_thread, INFINITE);
    CloseHandle(stop_thread);

    // attendre que le thread d'acceptation se termine
    WaitForSingleObject(accept_thread, INFINITE);
    CloseHandle(accept_thread);

    // attendre que les threads se terminent
    int connected_clients = count_connected_clients(state.clients, MAX_CLIENTS); // Compte le nombre de clients connectés
    if(connected_clients > 0) { 
        HANDLE threads[MAX_CLIENTS]; // tableau temporaire pour stocker les handles des threads
        int thread_count = 0; // Compteur de threads
        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(state.clients[i] != NULL) {
                threads[thread_count++] = state.clients[i]->thread; // Récupère le handle du thread de chaque client
            }
        }
        WaitForMultipleObjects(thread_count, threads, TRUE, INFINITE);
    }


    // fermer les handles des threads et libérer les instances de ClientData
    for(int i = 0; i < MAX_CLIENTS; i++) { 
        if(state.clients[i] != NULL) {
            CloseHandle(state.clients[i]->thread);
            free(state.clients[i]);
            state.clients[i] = NULL;
        }
    }

    // libérer le mutex
    CloseHandle(state.stop_flag_mutex);


    // Fermeture des sockets
    closesocket(listen_fd); // Ferme le socket d'écoute
    WSACleanup();
    printf("Serveur fermé.\n");

    return 0;
}




// Fonction pour gérer la communication avec un client
DWORD WINAPI comm_client(LPVOID param) {
    //// mettre une boucle pour arréter le programme si le socket se ferme! pour pouvoir libérer le thread par la suite

    ClientData *client_conn = (ClientData*)param; // Récupère le descripteur de socket du client
    SOCKET comm_fd = client_conn->socket; // Récupère le descripteur de socket du client
    ServerState *state = client_conn->state; // Récupère l'état du serveur
    int bytesReceived; // Nombre d'octets reçus

    // Boucle pour gérer la communication avec le client
    while (1) {
        WaitForSingleObject(state->buffer_mutex, INFINITE);

        // réception d'un message du client dans le buffer partagé
        bytesReceived = recv(comm_fd, state->buffer, sizeof(state->buffer) - 1, 0); 
        if (bytesReceived <=0) {
            printf("Erreur de réception du client #%d : %d\n", client_conn->client_id, WSAGetLastError());
            ReleaseMutex(state->buffer_mutex);
            closesocket(comm_fd);
            break; 
        }

        state->buffer[bytesReceived] = '\0'; // Ajoute un caractère nul à la fin du message reçu
        printf("Message reçu du client #%d : %s\n", client_conn->client_id, state->buffer);

        // vérifier si le client a envoyé "exit"  pour fermer la connexion
        if (strcmp(state->buffer, "exit") == 0) {
            printf("Le client #%d a quitter la converstaion.\n", client_conn->client_id);
            ReleaseMutex(state->buffer_mutex);
            closesocket(comm_fd);
            break;
        }

        ReleaseMutex(state->buffer_mutex); // Libère le mutex pour permettre à d'autres threads d'accéder au buffer

        send_message((LPVOID)state); // Appelle la fonction pour envoyer le message à tous les clients connectés
    }
    closesocket(comm_fd); // Ferme le socket de communication
    free(client_conn);
    return 0;
}



// Fonction pour accepter les connexions des clients
DWORD WINAPI accept_connections(LPVOID param) {
    ServerState*state = (ServerState*)param;
    SOCKET comm_fd; // Descripteur de socket pour la communication avec le client

    // Boucle infinie pour accepter les connexions des clients
    while (1) {
        // vérifier si le drapeau d'arrêt est activé
        WaitForSingleObject(state->stop_flag_mutex, INFINITE);
        if(state->stop_flag) {
            ReleaseMutex(state->stop_flag_mutex);
            break; // Sort de la boucle si le drapeau d'arrêt est activé
        }
        ReleaseMutex(state->stop_flag_mutex);

        // acceptation d'une connexion
        comm_fd = accept(state->listen_fd, (struct sockaddr*)NULL, NULL); // NULL car on ne se soucie pas de l'adresse du client ici
        if(comm_fd == INVALID_SOCKET) {
            printf("Erreur d'acceptation de connexion : %d\n", WSAGetLastError());
            continue; // Continue à accepter d'autres connexions
        }

        printf("Nouvelle connexion acceptée.\n");

        // vérification de l'identité du client/ connexion!

        // créer une instance de ClientData pour stocker les informations du client 
        int connected_clients = count_connected_clients(state->clients, MAX_CLIENTS);
        if(connected_clients < MAX_CLIENTS) {
            // Créer une instance de ClientData pour le client
            // et allouer de la mémoire pour le client
            ClientData *client_conn = malloc(sizeof(ClientData));
            if(client_conn == NULL) {
                printf("Erreur d'allocation de mémoire pour le client.\n");
                closesocket(comm_fd);
                continue;
            }

            int free_index = find_free_index(state->clients, MAX_CLIENTS); // Vérifier si un index libre pour le client

            // initialiser les champs de ClientData
            client_conn->socket = comm_fd;
            client_conn->client_id = (free_index != -1) ? free_index : connected_clients; // Assigne un ID au client
            client_conn->state = state; // Associe l'état du serveur au client
            client_conn->thread = NULL; // d'abord le thread, puis l'id grace à free_....

            if(free_index != -1) {
                state->clients[free_index] = client_conn; // ajoute à l'index libre
            } else {
                state->clients[connected_clients] = client_conn; // ajoute à la fin du tableau
            }

            // créer un thread pour gérer la communication avec le client
            HANDLE new_thread = CreateThread(NULL, 0, comm_client, (LPVOID)client_conn, 0, NULL);
            if(new_thread != NULL) {
                client_conn->thread = new_thread; // Stocke le handle du thread dans la structure ClientData
            } else {
                printf("Erreur de création du thread client : %d\n", GetLastError());
                closesocket(comm_fd);
                free(client_conn);
                if (free_index != -1) {
                    state->clients[free_index] = NULL; // Libère l'entrée si le thread n'a pas été créé
                } else {
                    state->clients[connected_clients] = NULL;
                }
            }
        } else {
            printf("Nombre maximum de clients atteint. Connexion refusée.\n");
            closesocket(comm_fd);
        }

        // Nettoyer les threads terminés
        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(state->clients[i] != NULL) {
                DWORD exit_code; // Code de sortie du thread
                char buffer[1]; // buffer temporaire pour tester le socket

                // tester si le socket est encore ouvert
                int result = recv(state->clients[i]->socket, buffer, sizeof(buffer), MSG_PEEK); // MSG_PEEK pour ne pas retirer les données de la file d'attente
                if(result == 0 || result == SOCKET_ERROR) {
                    printf("Le client #%d déconnecté.\n", state->clients[i]->client_id);
                    closesocket(state->clients[i]->socket);

                    // fermer le handle du thread
                    if(GetExitCodeThread(state->clients[i]->thread, &exit_code) && exit_code != STILL_ACTIVE){ // Vérifie si le thread est actif
                        CloseHandle(state->clients[i]->thread); // Ferme le handle du thread
                    }
                    free(state->clients[i]);
                    state->clients[i] = NULL;
                }
            } 
            }
        }
    return 0;
}

// fonction pour arrêter le serveur
DWORD WINAPI stop_server(LPVOID param) {
    ServerState*state = (ServerState*)param;

    // attendre que l'utilisateur appuie sur une touche pour arrêter le serveur
    printf("Appuyer sur une touche pour arrêter le serveur ...\n");
    getchar(); // Attendre que l'utilisateur appuie sur une touche

    // signaler au thread d'acceptation de se terminer 
    WaitForSingleObject(state->stop_flag_mutex, INFINITE);
    state->stop_flag = 1; // Définit le drapeau d'arrêt à 1 (arrêter)
    ReleaseMutex(state->stop_flag_mutex);

    printf("Arrêt du serveur demandé.\n");
    return 0;
}

DWORD WINAPI send_message(LPVOID param) {
    ServerState *state = (ServerState*)param;

    WaitForSingleObject(state->buffer_mutex, INFINITE);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (state->clients[i] != NULL) {
            // envoyer le message du buffer à chaque client connecté
            int result = send(state->clients[i]->socket, state->buffer, strlen(state->buffer), 0);
            if (result == SOCKET_ERROR) {
                printf("Erreur d'envoi au client #%d : %d\n", state->clients[i]->client_id, WSAGetLastError());
            } else {
                printf("Message envoyé au client #%d : %s\n", state->clients[i]->client_id, state->buffer);
            }
        }
    }

    ReleaseMutex(state->buffer_mutex);
    return 0;
}

int find_free_index(ClientData *clients[], int max_clients) {
    for (int i = 0; i < max_clients; i++) {
        if (clients[i] == NULL) {
            return i; // Retourne l'index libre
        }
    }
    return -1; // Aucun espace libre
}

int count_connected_clients(ClientData *clients[], int max_clients) {
    int count = 0;
    for (int i = 0; i < max_clients; i++) {
        if (clients[i] != NULL) {
            count++;
        }
    }
    return count;
}