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
int list_public_channels(PGconn* conn);

#endif // CHANNEL_CRUD_H
