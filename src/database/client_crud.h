#ifndef CLIENT_CRUD_H
#define CLIENT_CRUD_H

#include <libpq-fe.h>

// Struct to represent a client
typedef struct {
    int id_client;
    char first_name[50];
    char last_name[50];
    char pseudo[50];
    char password[150];
    char email[50];
} Client;

// Create a new client account
int create_client_account(PGconn *conn, const Client *client);

// Authenticate a client by pseudo and password
int read_client_by_login(PGconn *conn, const char *pseudo, const char *password);

// Update client name and last name
int update_client_name(PGconn *conn, int client_id, const char *name, const char *last_name);

// Update client pseudo
int update_client_pseudo(PGconn *conn, int client_id, const char *pseudo);

// Update client email
int update_client_email(PGconn *conn, int client_id, const char *email);

// Update client password
int update_client_password(PGconn *conn, int client_id, const char *password);

// Delete a client account
int delete_client(PGconn *conn, int client_id);

/* 
* Retrieve a client's pseudo by their ID
* Usage:
* char* sender_pseudo = read_client_pseudo_by_id(conn, 1);
* if (sender_pseudo) {
*     printf("Pseudo: %s\n", pseudo);
*     free(pseudo); // Free the allocated memory
* } else {
*     printf("Failed to retrieve sender pseudo.\n");
* }
*/
char *read_client_pseudo_by_id(PGconn *conn, int client_id);

#endif // CLIENT_CRUD_H
