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

// Insert a new message into the database
// Returns 1 on success, 0 on failure
// The message content should be less than 500 characters
int write_message(PGconn* conn, const Message* message);

// Edit an existing message in the database
// Returns 1 on success, 0 on failure
int edit_message(PGconn* conn, int message_id, int user_id, const char* new_content);

// Fetch messages from a specific channel
// Returns an array of Message structs and sets message_count to the number of messages fetched
// The caller is responsible for freeing the returned array {free(messages)}
// Returns NULL on failure
Message* fetch_channel_messages(PGconn* conn, int channel_id, int* message_count);

#endif // MESSAGE_CRUD_H
