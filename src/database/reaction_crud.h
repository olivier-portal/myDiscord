#ifndef REACTION_CRUD_H
#define REACTION_CRUD_H

#include <libpq-fe.h>

// Struct to represent a Reaction
typedef struct {
    int id;
    char emoji[20];
    int client_id;
    int message_id;
} Reaction;

// Add a reaction to a message
int add_reaction(PGconn* conn, const Reaction* reaction);

#endif // REACTION_CRUD_H
