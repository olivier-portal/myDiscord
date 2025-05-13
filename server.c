// Pour compiler ce code : gcc server.c -o server -lws2_32

#include <winsock2.h> // Pour les sockets Windows
#include <ws2tcpip.h> // Pour les focntionnalités de sockets TCP/IP
#include <stdio.h>
#include <string.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib") // Lien avec la bibliothèque Winsock

#define MAX_CLIENTS 100 
#define BUFFER_SIZE 1024 
#define PORT 8080 
#define SERVER_IP "127.0.0.1" 

typedef struct ClientData ClientData; // Déclaration anticipée de la structure ClientData
typedef struct ServerState ServerState; // Déclaration anticipée de la structure ServerState


typedef struct ServerState {
    SOCKET listen_fd;
    SOCKET stop_socket; // socket pour l'arrêt du serveur
    ClientData *clients[MAX_CLIENTS]; 
    HANDLE clients_mutex; 
    char buffer[BUFFER_SIZE]; 
    HANDLE buffer_mutex; 
}ServerState;

typedef struct ClientData {
    SOCKET socket; 
    int client_id; 
    HANDLE thread; 
    ServerState *state;
}ClientData;

int release_client_resources(ServerState *state, ClientData *client_conn);
int count_connected_clients(ServerState *state, ClientData *clients[], int max_clients);
int find_free_index(ServerState *state, ClientData *clients[], int max_clients);
DWORD WINAPI comm_client(LPVOID param); 
DWORD WINAPI accept_connections(LPVOID param);
DWORD WINAPI stop_server(LPVOID param); 
DWORD WINAPI send_message(LPVOID param); 


int main() {
    WSADATA wsaData; 
    SOCKET listen_fd;
    SOCKET stop_socket; // socket pour l'arrêt du serveur
    struct sockaddr_in servaddr;
    struct sockaddr_in stop_addr;

    printf("[DEBUG] Initialisation du serveur...\n");

    if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0) {
        printf("[ERREUR] Erreur d'initialisation de Winsock : %d\n", WSAGetLastError());
        return 1;
    }
    printf("[DEBUG] Winsock initialisé.\n");

    // connexion base de donnée

    // Création de la socket d'écoute (TCP)
    listen_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if(listen_fd == INVALID_SOCKET) {
        printf("[ERREUR] Erreur de création du socket listen_fd : %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("[DEBUG] Socket d'écoute crée\n");

    // Configuration de l'adresse du serveur
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP); 
    servaddr.sin_port = htons(PORT); 

    // Liason du socket à l'adresse du serveur
    if(bind(listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        printf("[ERREUR] Erreur de liaison du socket : %d\n", WSAGetLastError());
        closesocket(listen_fd);
        WSACleanup();
        return 1;
    }
    printf("[DEBUG] Socket lié à l'adresse %s : %d\n", SERVER_IP, PORT);

    // Ecoute des connexions 
    if(listen(listen_fd, MAX_CLIENTS) == SOCKET_ERROR) { 
        printf("[ERREUR] Erreur lors de l'écoute : %d\n", WSAGetLastError());
        closesocket(listen_fd); 
        WSACleanup();
        return 1;
    }
    printf("[DEBUG] Serveur en écoute sur le port %d\n", PORT);

    // Création d'un socket d'arret
    stop_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(stop_socket == INVALID_SOCKET) {
        printf("[ERREUR] Erreur de création du socket stop_socket : %d\n", WSAGetLastError());
        closesocket(listen_fd);
        WSACleanup();
        return 1;
    }
    printf("[DEBUG] Socket d'arrêt créé\n");

    memset(&stop_addr, 0, sizeof(stop_addr));
    stop_addr.sin_family = AF_INET;
    stop_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    stop_addr.sin_port = htons(PORT + 1); // Port différent pour le socket d'arrêt

    if (bind(stop_socket, (struct sockaddr *)&stop_addr, sizeof(stop_addr)) == SOCKET_ERROR) {
        printf("[ERREUR] Erreur de liaison du socket d'arrêt : %d\n", WSAGetLastError());
        closesocket(listen_fd);
        closesocket(stop_socket);
        WSACleanup();
        return 1;
    }
    printf("[DEBUG] Socket d'arrêt lié à l'adresse locale.\n");
    

    // création d'une instance state la structure ServerState gérer les paramètre du serveur et les connexion des clients
    ServerState state;
    state.listen_fd = listen_fd;
    state.stop_socket = stop_socket; // socket pour l'arrêt du serveur
    memset(state.clients, 0, sizeof(state.clients)); 
    memset(state.buffer, 0, sizeof(state.buffer));

    state.clients_mutex = CreateMutex(NULL, FALSE, NULL);
    if (state.clients_mutex == NULL) {
        printf("[ERREUR] Erreur de création du mutex clients_mutex : %d\n", GetLastError());
        closesocket(listen_fd);
        WSACleanup();
        return 1;
    } else {
        printf("[DEBUG] Mutex clients_mutex créé avec succès.\n");
    }

    state.buffer_mutex = CreateMutex(NULL, FALSE, NULL);
    if (state.buffer_mutex == NULL) {
        printf("[ERREUR] Erreur de création du mutex buffer_mutex : %d\n", GetLastError());
        CloseHandle(state.clients_mutex);
        closesocket(listen_fd);
        WSACleanup();
        return 1;
    } else {
        printf("[DEBUG] Mutex buffer_mutex créé avec succès.\n");
    }

    // créer un thread pour accepter les connexions
    HANDLE accept_thread = CreateThread(NULL, 0, accept_connections, (LPVOID)&state, 0, NULL);
    if (accept_thread == NULL) {
        printf("[ERREUR] Erreur de création du thread d'acceptation : %d\n", GetLastError());
        CloseHandle(state.clients_mutex);
        CloseHandle(state.buffer_mutex);
        closesocket(listen_fd);
        WSACleanup();
        return 1;
    } else {
        printf("[DEBUG] Thread d'acceptation créé avec succès.\n");
    }

    // créer un thread pour surveiller la condition d'arrêt du serveur
    HANDLE stop_thread = CreateThread(NULL, 0, stop_server, (LPVOID)&state, 0, NULL);
    if (stop_thread == NULL) {
        printf("[ERREUR] Erreur de création du thread d'arrêt : %d\n", GetLastError());
        CloseHandle(state.clients_mutex);
        CloseHandle(state.buffer_mutex);
        closesocket(listen_fd);
        WSACleanup();
        return 1;
    } else {
        printf("[DEBUG] Thread d'arrêt créé avec succès.\n");
    }


    // attendre que le thread d'arrêt se termine 
    WaitForSingleObject(stop_thread, INFINITE);
    CloseHandle(stop_thread);
    printf("[DEBUG] Thread d'arrêt terminé.\n");

    // Libérer l'événement d'arrêt
    CloseHandle(stop_server);
    printf("[DEBUG] Socket d'arret fermé.\n");

    // attendre que le thread d'acceptation se termine
    WaitForSingleObject(accept_thread, INFINITE);
    CloseHandle(accept_thread);
    printf("[DEBUG] Thread d'acceptation terminé.\n");

    // attendre que les threads clients se terminent
    printf("[DEBUG] Attente de la terminaison des threads clients...\n");
    int connected_clients = count_connected_clients(state.clients_mutex, state.clients, MAX_CLIENTS); 
    printf("[DEBUG] Nombre de clients connectés : %d\n", connected_clients);
    if(connected_clients > 0) { 
        HANDLE threads[MAX_CLIENTS]; // tableau temporaire pour stocker les handles des threads
        int thread_count = 0;
        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(state.clients[i] != NULL) {
                threads[thread_count++] = state.clients[i]->thread;
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
    printf("[DEBUG] Tous les clients ont été déconnectés et leur ressources libérées.\n");

    // libérer le mutex
    CloseHandle(state.buffer_mutex);
    CloseHandle(state.clients_mutex);
    printf("[DEBUG] Mutex buffer et clients libérés.\n");


    // Fermeture des sockets
    closesocket(listen_fd); 
    WSACleanup();
    printf("[DEBUG] Serveur fermé.\n");

    return 0;
}




// Fonction pour gérer la communication avec un client
DWORD WINAPI comm_client(LPVOID param) {
    ClientData *client_conn = (ClientData*)param;
    SOCKET comm_fd = client_conn->socket;
    ServerState *state = client_conn->state;
    int bytesReceived;
    DWORD wait_result;

    printf("[DEBUG] Démarrage de la fonction comm_client pour le client #%d.\n", client_conn->client_id);

    // Envoyer un message de bienvenue au client
    const char *welcome_message = "Bienvenue sur le serveur MyDiscord !\n";
    if (send(comm_fd, welcome_message, strlen(welcome_message), 0) == SOCKET_ERROR) {
        printf("[ERREUR] Impossible d'envoyer le message de bienvenue au client #%d : %d\n", client_conn->client_id, WSAGetLastError());
        goto cleanup; // Aller à la section de nettoyage en cas d'erreur
    }
    printf("[DEBUG] Message de bienvenue envoyé au client #%d.\n", client_conn->client_id);

    // Boucle pour gérer la communication avec le client
    while (1) {   
        // réception d'un message du client
        bytesReceived = recv(comm_fd, state->buffer, sizeof(state->buffer) - 1, 0); 
        if (bytesReceived == 0) {
            // Le client a fermé la connexion ou une erreur s'est produite
            printf("[INFO] Le client #%d a fermé la connexion.\n", client_conn->client_id);
            break; 
        } else if (bytesReceived < 0) {
            // Une erreur s'est produite lors de la réception
            printf("[ERREUR] Erreur de réception du client #%d : %d\n", client_conn->client_id, WSAGetLastError());
            break;
        }

        // Si des données sont reçues, acquérir le mutex pour protéger le buffer
        wait_result = WaitForSingleObject(state->buffer_mutex, INFINITE);
        if (wait_result != WAIT_OBJECT_0) {
            printf("[ERREUR] Échec de WaitForSingleObject pour buffer_mutex : %d\n", GetLastError());
            break;
        }
        printf("[DEBUG] Mutex buffer_mutex acquis pour le client #%d.\n", client_conn->client_id);

        // Traiter les données reçues
        state->buffer[bytesReceived] = '\0';
        printf("[DEBUG] Message reçu du client #%d : %s\n", client_conn->client_id, state->buffer);

        // Vérifier si le client a envoyé "exit"  pour fermer la connexion
        if (strcmp(state->buffer, "exit") == 0) {
            printf("[INFO] Le client #%d a quitté la conversation.\n", client_conn->client_id);

            // Libérer le mutex buffer_mutex avant de quitter
            if (!ReleaseMutex(state->buffer_mutex)) {
                printf("[ERREUR] Échec de ReleaseMutex pour buffer_mutex : %d\n", GetLastError());
            } else {
                printf("[DEBUG] Mutex buffer_mutex libéré pour le client #%d.\n", client_conn->client_id);
            }
            break;
        }

        // Si le message n'est pas "exit", envoyer le message à tous les clients connectés
        if (!ReleaseMutex(state->buffer_mutex)) {
            printf("[ERREUR] Échec de ReleaseMutex pour buffer_mutex : %d\n", GetLastError());
            break;
        }
        printf("[DEBUG] Mutex buffer_mutex libéré pour le client #%d.\n", client_conn->client_id);

        printf("[DEBUG] Envoi du message du client #%d à tous les clients connectés.\n", client_conn->client_id);
        // Crée un tableau de paramètres pour la fonction send_message
        void *params[2];
        params[0] = (void *)state;
        params[1] = (void *)client_conn; 
        send_message((LPVOID)params);
    }

cleanup:
    printf("[DEBUG] Fin de la connexion et de la fonction comm_client pour le client #%d\n", client_conn->client_id);

    // Libérer les ressources du client
    if (release_client_resources(state, client_conn) != 0) {
        printf("[ERREUR] Échec de la libération des ressources pour le client #%d.\n", client_conn->client_id);
    }

    return 0;
}



// Fonction pour accepter les connexions des clients
DWORD WINAPI accept_connections(LPVOID param) {
    ServerState*state = (ServerState*)param;
    SOCKET comm_fd; 
    fd_set read_fds; // Ensemble de descripteurs de fichiers pour la sélection
    struct timeval timeout; // Structure pour le délai d'attente
    

    printf("[DEBEUG] Démarrage de la fonction accept_connections.\n");

    // Boucle infinie pour accepter les connexions des clients
    while (1) {
        // Vérification du nombre de clients connectés avant d'accepter une connexion
        printf("[DEBUG] Vérification du nombre de clients connectés avant accept.\n");
        int connected_clients = count_connected_clients(state, state->clients, MAX_CLIENTS);
        printf("[DEBUG] Nombre de clients connectés : %d\n", connected_clients);

        if (connected_clients >= MAX_CLIENTS) {
            printf("[DEBUG] Nombre maximum de clients atteint. Connexion refusée.\n");
            Sleep(1000); // Attendre un moment avant de réessayer
            continue;
        }

        // Initialiser l'ensemble des descripteurs
        FD_ZERO(&read_fds);
        FD_SET(state->listen_fd, &read_fds); // Ajouter le socket d'écoute à l'ensemble
        FD_SET(state->stop_socket, &read_fds); // Ajouter le socket d'arrêt à l'ensemble

        // configurer un délai d'attente pour eviter un blocage infini
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        // Appeler 'select' pour surveiller les connexions entrantes
        int activity = select(0, &read_fds, NULL, NULL, &timeout);
        if (activity == SOCKET_ERROR) {
            printf("[ERREUR] Erreur de select : %d\n", WSAGetLastError());
            break;
        }

        if (activity == 0) {
            // Timeout, vérifier si le serveur doit s'arrêter
            if (FD_ISSET(state->stop_socket, &read_fds)) {
                printf("[DEBUG] Événement d'arrêt déclenché. Arrêt du serveur.\n");
                break;
            }
            continue;
        }
        

        // Vérifier si une connexion entrante est détectée
        if (FD_ISSET(state->listen_fd, &read_fds)) {
            printf("[DEBUG] Connexion entrante détectée.\n");
            comm_fd = accept(state->listen_fd, NULL, NULL); 
            if (comm_fd == INVALID_SOCKET) {
                printf("[ERREUR] Erreur d'acceptation de la connexion : %d\n", WSAGetLastError());
                continue;
            }
            printf("[DEBUG] Connexion acceptée.\n");
        }
        

        // vérification de l'identité du client/ connexion!

        // créer une instance de ClientData pour stocker les informations du client 
        printf("[DEBUG] Création d'une instance de ClientData pour le nouveau client.\n");
        ClientData *client_conn = malloc(sizeof(ClientData));
        if(client_conn == NULL) {
            printf("[ERREUR] erreur d'allocation de mémoire pour le client.\n");
            closesocket(comm_fd);
            continue;
        }         

        // initialiser les champs de ClientData
        client_conn->socket = comm_fd;
        client_conn->state = state; 
        client_conn->thread = NULL;   

        printf("[DEBUG] Appel de find_free_index avec les paramètres suivants :\n");
        printf(" - state : %p\n", (void *)state);
        printf(" - clients : %p\n", (void *)state->clients);
        printf(" - max_clients : %d\n", MAX_CLIENTS);

        int free_index = find_free_index(state, state->clients, MAX_CLIENTS); // Trouve un index libre dans le tableau de clients

        DWORD wait_result = WaitForSingleObject(state->clients_mutex, INFINITE);
        if (wait_result != WAIT_OBJECT_0) {
            printf("[ERREUR] Échec de WaitForSingleObject pour clients_mutex : %d\n", GetLastError());
            free(client_conn);
            closesocket(comm_fd);
            continue;
        }
        printf("[DEBUG] Mutex clients_mutex acquis pour l'ajout du client.\n");

        if(free_index != -1) {
            client_conn->client_id = free_index; // Associe l'ID du client à l'index libre
            state->clients[free_index] = client_conn;
            printf("[DEBUG] Client ajouté à l'index libre : %d.\n", free_index);   
        } else {
            client_conn->client_id = connected_clients;
            state->clients[connected_clients] = client_conn;
            printf("[DEBUG] Client ajouté à la fin de la liste.\n");
        }

        if (!ReleaseMutex(state->clients_mutex)) {
            printf("[ERREUR] Échec de ReleaseMutex pour clients_mutex : %d\n", GetLastError());
            if(release_client_resources(state, client_conn) != 0) {
                printf("[ERREUR] Échec de la libération des ressources pour le client #%d.\n", client_conn->client_id);
            }
            continue;
        }
        printf("[DEBUG] Mutex clients_mutex libéré pour l'ajout du client.\n");
        

        // créer un thread pour gérer la communication avec le client
        printf("[DEBEUG] Création d'un thread pour gérer la communication avec le client #%d.\n", client_conn->client_id);

        HANDLE new_thread = CreateThread(NULL, 0, comm_client, (LPVOID)client_conn, 0, NULL);

        if(new_thread != NULL) {
            client_conn->thread = new_thread; 
            printf("[DEBEUG] thread créé avec succès pour le client #%d.\n", client_conn->client_id);
        } else {
            printf("[ERREUR] Erreur de création du thread client : %d\n", GetLastError());
            if(release_client_resources(state, client_conn) != 0) {
                printf("[ERREUR] Échec de la libération des ressources pour le client #%d.\n", client_conn->client_id);
            }
            continue; // Continue à accepter d'autres connexions
        }
    }

    printf("[DEBUG] Fin de la fonction accpet_connections.\n");
    return 0;
}



// fonction pour arrêter le serveur
DWORD WINAPI stop_server(LPVOID param) {
    ServerState*state = (ServerState*)param;

    printf("[DEBUG] Appuyez sur une touche pour arrêter le serveur...\n");
    fflush(stdin);
    getchar(); // Attendre que l'utilisateur appuie sur une touche
 
    char stop_signal = '1'; // Signal d'arrêt
    sendto(state->stop_socket, &stop_signal, sizeof(stop_signal), 0, NULL, 0); // Envoyer le signal d'arrêt
    printf("[DEBUG] Signal d'arrêt envoyé au serveur.\n");

    return 0;
}



// Fonction pour envoyer un message à tous les clients connectés
DWORD WINAPI send_message(LPVOID param) {
    void **params = (void**)param;
    ServerState *state = (ServerState *)params[0];
    ClientData *client_conn = (ClientData *)params[1];

    printf("[DEBUG] Démarrage de la fonction send_message.\n");

    // Tentative d'acquisition du mutex buffer_mutex
    printf("[DEBUG] Tentative d'acquisition du mutex buffer_mutex.\n");
    DWORD wait_result = WaitForSingleObject(state->buffer_mutex, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        printf("[ERREUR] Échec de WaitForSingleObject pour buffer_mutex : %d\n", GetLastError());
        return 1;
    }
    printf("[DEBUG] Mutex buffer_mutex acquis avec succès.\n");

    // Tentative d'acquisition du mutex clients_mutex
    printf("[DEBUG] Tentative d'acquisition du mutex clients_mutex.\n");
    wait_result = WaitForSingleObject(state->clients_mutex, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        printf("[ERREUR] Échec de WaitForSingleObject pour clients_mutex : %d\n", GetLastError());
        ReleaseMutex(state->buffer_mutex); // Libérer buffer_mutex en cas d'échec
        return 1;
    }
    printf("[DEBUG] Mutex clients_mutex acquis avec succès.\n");

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (state->clients[i] != NULL && state->clients[i]->client_id != client_conn->client_id) {
            // envoyer le message du buffer à chaque client connecté
            int result = send(state->clients[i]->socket, state->buffer, strlen(state->buffer), 0);
            if (result == SOCKET_ERROR) {
                printf("[ERREUR] Erreur d'envoi au client #%d : %d\n", state->clients[i]->client_id, WSAGetLastError());
            } else {
                printf("[DEBUG] Message envoyé au client #%d : %s\n", state->clients[i]->client_id, state->buffer);
            }
        }
    }

    // vider le buffer après l'envoi
    memset(state->buffer, 0, sizeof(state->buffer));
    printf("[DEBUG] Buffer vidé après l'envoi.\n");

    // Libérer le mutex buffer_mutex
    if (!ReleaseMutex(state->buffer_mutex)) {
        printf("[ERREUR] Échec de ReleaseMutex pour buffer_mutex : %d\n", GetLastError());
    } else {
        printf("[DEBUG] Mutex buffer_mutex libéré avec succès.\n");
    }

    // Libérer le mutex clients_mutex
    if (!ReleaseMutex(state->clients_mutex)) {
        printf("[ERREUR] Échec de ReleaseMutex pour clients_mutex : %d\n", GetLastError());
    } else {
        printf("[DEBUG] Mutex clients_mutex libéré avec succès.\n");
    }

    printf("[DEBUG] Fin de la fonction send_message.\n");
    return 0;
};



// Fonction pour trouver un index libre dans le tableau de clients
int find_free_index(ServerState *state, ClientData *clients[], int max_clients) {
    printf("[DEBUG] Démarrage de la fonction find_free_index.\n");

    printf(" - state : %p\n", (void *)state);
    printf(" - clients : %p\n", (void *)state->clients);
    printf(" - max_clients : %d\n", MAX_CLIENTS);

    // Vérification des paramètres
    if (state == NULL) {
        printf("[ERREUR] Paramètre 'state' est NULL.\n");
        return -1;
    }
    if (clients == NULL) {
        printf("[ERREUR] Paramètre 'clients' est NULL.\n");
        return -1;
    }

    // Vérification du mutex
    if (state->clients_mutex == NULL) {
        printf("[ERREUR] Mutex clients_mutex est NULL.\n");
        return -1;
    }

    DWORD wait_result = WaitForSingleObject(state->clients_mutex, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        printf("[ERREUR] Échec de WaitForSingleObject pour clients_mutex : %d\n", GetLastError());
        return -1;
    }
    printf("[DEBUG] Mutex clients_mutex acquis pour la recherche d'un index libre.\n");

    // Recherche d'un index libre
    for (int i = 0; i < max_clients; i++) {
        if (clients[i] == NULL) {
            // Libérer le mutex avant de retourner l'index
            if (!ReleaseMutex(state->clients_mutex)) {
                printf("[ERREUR] Échec de ReleaseMutex pour clients_mutex : %d\n", GetLastError());
            } else {
                printf("[DEBUG] Mutex clients_mutex libéré après la recherche d'un index libre.\n");
            }
            return i; // Retourne l'index libre
        }
    }

    // Aucun index libre trouvé
    if (!ReleaseMutex(state->clients_mutex)) {
        printf("[ERREUR] Échec de ReleaseMutex pour clients_mutex : %d\n", GetLastError());
    } else {
        printf("[DEBUG] Mutex clients_mutex libéré après la recherche d'un index libre.\n");
    }
    printf("[DEBUG] Aucun index libre trouvé.\n");
    return -1; // Aucun espace libre
}



// Fonction pour compter le nombre de clients connectés
int count_connected_clients(ServerState *state, ClientData *clients[], int max_clients) {
    printf("[DEBUG] Démarrage de la fonction count_connected_clients.\n");

    // Vérification des paramètres
    if (state == NULL || clients == NULL) {
        printf("[ERREUR] Paramètres invalides : état ou clients NULL.\n");
        return -1;
    }

    if (state->clients_mutex == NULL) {
        printf("[ERREUR] Mutex clients_mutex NULL.\n");
        return -1;
    }

    // Tentative d'acquisition du mutex clients_mutex
    printf("[DEBUG] Tentative d'acquisition du mutex clients_mutex pour le comptage des clients.\n");
    DWORD wait_result = WaitForSingleObject(state->clients_mutex, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        printf("[ERREUR] Échec de WaitForSingleObject pour clients_mutex : %d\n", GetLastError());
        return -1;
    }
    printf("[DEBUG] Mutex clients_mutex acquis pour le comptage des clients.\n");

    // Comptage des clients connectés
    int count = 0;
    for (int i = 0; i < max_clients; i++) {
        if (clients[i] != NULL) {
            count++;
        }
    }

    // Libération du mutex clients_mutex
    if (!ReleaseMutex(state->clients_mutex)) {
        printf("[ERREUR] Échec de ReleaseMutex pour clients_mutex : %d\n", GetLastError());
    } else {
        printf("[DEBUG] Mutex clients_mutex libéré après le comptage des clients.\n");
    }

    printf("[DEBUG] Nombre de clients connectés : %d\n", count);
    return count;
}



// Fonction pour libérer les ressources d'un client
int release_client_resources(ServerState *state, ClientData *client_conn) {
    if (state == NULL || client_conn == NULL) {
        printf("[ERREUR] Paramètres invalides pour release_client_resources.\n");
        return -1; // Échec
    }

    printf("[DEBUG] Libération des ressources pour le client #%d.\n", client_conn->client_id);

    // Fermer le socket du client
    closesocket(client_conn->socket);
    printf("[DEBUG] Socket fermé pour le client #%d.\n", client_conn->client_id);

    // Acquérir le mutex pour modifier le tableau des clients
    DWORD wait_result = WaitForSingleObject(state->clients_mutex, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        printf("[ERREUR] Échec de WaitForSingleObject pour clients_mutex : %d\n", GetLastError());
        return -1; // Échec
    }
    printf("[DEBUG] Mutex clients_mutex acquis pour la suppression du client #%d.\n", client_conn->client_id);

    // Supprimer le client du tableau
    if (state->clients[client_conn->client_id] != NULL) {
        state->clients[client_conn->client_id] = NULL;
        printf("[DEBUG] Client #%d retiré du tableau des clients.\n", client_conn->client_id);
    } else {
        printf("[DEBUG] Le client #%d a déjà été supprimé du tableau.\n", client_conn->client_id);
    }

    // Libérer le mutex
    if (!ReleaseMutex(state->clients_mutex)) {
        printf("[ERREUR] Échec de ReleaseMutex pour clients_mutex : %d\n", GetLastError());
        return -1; // Échec
    } else {
        printf("[DEBUG] Mutex clients_mutex libéré après la suppression du client #%d.\n", client_conn->client_id);
    }

    // Libérer la mémoire allouée pour le client
    free(client_conn);
    printf("[DEBUG] Mémoire libérée pour le client #%d.\n", client_conn->client_id);

    return 0; // Succès
}