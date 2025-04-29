#ifndef CHANNEL_CRUD_H
#define CHANNEL_CRUD_H

#include <libpq-fe.h>

// Struct to represent a Channel
typedef struct {
    int id;
    char name[50];
    int is_private;
} Channel;

// List all public channels
int list_public_channels(PGconn *conn);

// Create a new channel
int create_channel(PGconn *conn, const char *name, int is_private);

// Edit a channel's name and privacy status
int edit_channel(PGconn *conn, int channel_id, int user_id, const char *new_name, int is_private);

// Select all channels a user is part of
int select_user_channels(PGconn *conn, int user_id);

// Delete a channel (only if the user is an admin)
int delete_channel(PGconn *conn, int channel_id, int user_id);

#endif // CHANNEL_CRUD_H
