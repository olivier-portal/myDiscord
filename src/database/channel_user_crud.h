#ifndef CHANNEL_USER_CRUD_H
#define CHANNEL_USER_CRUD_H

#include <libpq-fe.h>

// Add a user to a channel with a specific role
int add_user_to_channel(PGconn* conn, int client_id, int channel_id, const char* role);

// Remove a user from a channel
int remove_user_from_channel(PGconn* conn, int client_id, int channel_id);

#endif // CHANNEL_USER_CRUD_H
