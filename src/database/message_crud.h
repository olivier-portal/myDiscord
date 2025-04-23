#ifndef MESSAGE_CRUD_H
#define MESSAGE_CRUD_H

#include <libpq-fe.h>

// Struct to represent a Message
typedef struct {
    int id;
    char content[500];
    char date[50];
    int channel_id;
    int sender_id;
} Message;

// Write a new message in a public channel
int write_message(PGconn* conn, const Message* message);

#endif // MESSAGE_CRUD_H
