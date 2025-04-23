#ifndef CLIENT_CRUD_H
#define CLIENT_CRUD_H

#include <libpq-fe.h>

// Struct to represent a Client
typedef struct {
    int id;
    char name[50];
    char last_name[50];
    char pseudo[50];
    char password[150];
    char email[50];
} Client;

// Create a new client account
int create_client_account(PGconn* conn, const Client* client);

// Authenticate a client
int authenticate_client(PGconn* conn, const char* pseudo, const char* password);

#endif // CLIENT_CRUD_H
