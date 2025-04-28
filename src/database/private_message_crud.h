#ifndef PRIVATE_MESSAGE_CRUD_H
#define PRIVATE_MESSAGE_CRUD_H

#include <libpq-fe.h>

// Struct to represent a private message
typedef struct {
    int id;
    char content[500];
    char date[50];
    int sender_id;
    int target_id;
} PrivateMessage;

// Send a private message
int send_private_message(PGconn* conn, const PrivateMessage* message);

// Fetch private messages for a user
// Returns an array of PrivateMessage structs and sets message_count to the number of messages fetched
// The caller is responsible for freeing the returned array {free(messages)}
PrivateMessage* fetch_private_messages(PGconn* conn, int user_id, int* message_count);

#endif // PRIVATE_MESSAGE_CRUD_H
