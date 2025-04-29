#ifndef CHANNEL_USER_CRUD_H
#define CHANNEL_USER_CRUD_H

#include <libpq-fe.h>

// Add a user to a channel with a specific role
int create_user_channel(PGconn *conn, int client_id, int channel_id, const char *role);

// Remove a user from a channel
int delete_user_channel(PGconn *conn, int client_id, int channel_id);

// Update a user's role in a channel
int update_user_channel(PGconn *conn, int client_id, int channel_id, const char *new_role);

#endif // CHANNEL_USER_CRUD_H
