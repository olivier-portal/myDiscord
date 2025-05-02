#include "client_crud.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_client_account(PGconn* conn, const Client* client) {
    const char* query = "INSERT INTO Client (first_name, last_name, pseudo, password, email) VALUES ($1, $2, $3, $4, $5)";
    PGresult* res = PQexecParams(conn, query, 5, NULL, (const char*[]){client->first_name, client->last_name, client->pseudo, client->password, client->email}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de la création du compte client: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Compte client créé avec succès.\n");
    PQclear(res);
    return 1;
}

int read_client_by_login(PGconn* conn, const char* pseudo, const char* password) {
    const char* query = "SELECT COUNT(*) FROM Client WHERE pseudo = $1 AND password = $2";
    PGresult* res = PQexecParams(conn, query, 2, NULL, (const char*[]){pseudo, password}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "❌ Erreur lors de l'authentification: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    int authenticated = atoi(PQgetvalue(res, 0, 0)) > 0;
    PQclear(res);

    if (authenticated) {
        printf("✅ Authentification réussie.\n");
    } else {
        printf("❌ Authentification échouée.\n");
    }

    return authenticated;
}

int update_client_name(PGconn* conn, int client_id, const char* name, const char* last_name) {
    const char* query = "UPDATE Client SET name = $1, last_name = $2 WHERE Id_client = $3";
    char client_id_str[12];
    snprintf(client_id_str, sizeof(client_id_str), "%d", client_id);

    PGresult* res = PQexecParams(conn, query, 3, NULL, 
        (const char*[]){name, last_name, client_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de la modification du nom du client: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Nom du client modifié avec succès.\n");
    PQclear(res);
    return 1;
}

int update_client_pseudo(PGconn* conn, int client_id, const char* pseudo) {
    const char* query = "UPDATE Client SET pseudo = $1 WHERE Id_client = $2";
    char client_id_str[12];
    snprintf(client_id_str, sizeof(client_id_str), "%d", client_id);

    PGresult* res = PQexecParams(conn, query, 2, NULL, 
        (const char*[]){pseudo, client_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de la modification du pseudo du client: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Pseudo du client modifié avec succès.\n");
    PQclear(res);
    return 1;
}

int update_client_email(PGconn* conn, int client_id, const char* email) {
    const char* query = "UPDATE Client SET email = $1 WHERE Id_client = $2";
    char client_id_str[12];
    snprintf(client_id_str, sizeof(client_id_str), "%d", client_id);

    PGresult* res = PQexecParams(conn, query, 2, NULL, 
        (const char*[]){email, client_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de la modification de l'email du client: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Email du client modifié avec succès.\n");
    PQclear(res);
    return 1;
}

int update_client_password(PGconn* conn, int client_id, const char* password) {
    const char* query = "UPDATE Client SET password = $1 WHERE Id_client = $2";
    char client_id_str[12];
    snprintf(client_id_str, sizeof(client_id_str), "%d", client_id);

    PGresult* res = PQexecParams(conn, query, 2, NULL, 
        (const char*[]){password, client_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de la modification du mot de passe du client: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Mot de passe du client modifié avec succès.\n");
    PQclear(res);
    return 1;
}

int delete_client(PGconn* conn, int client_id) {
    const char* query = "DELETE FROM Client WHERE Id_client = $1";
    char client_id_str[12];
    snprintf(client_id_str, sizeof(client_id_str), "%d", client_id);

    PGresult* res = PQexecParams(conn, query, 1, NULL, (const char*[]){client_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de la suppression du compte client: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Compte client supprimé avec succès.\n");
    PQclear(res);
    return 1;
}

char* read_client_pseudo_by_id(PGconn* conn, int client_id) {
    const char* query = "SELECT pseudo FROM Client WHERE Id_client = $1";
    char client_id_str[12];
    snprintf(client_id_str, sizeof(client_id_str), "%d", client_id);

    PGresult* res = PQexecParams(conn, query, 1, NULL, (const char*[]){client_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "❌ Erreur lors de la récupération du pseudo du client: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return NULL;
    }

    if (PQntuples(res) == 0) {
        fprintf(stderr, "❌ Aucun client trouvé avec l'ID spécifié.\n");
        PQclear(res);
        return NULL;
    }

    // Dynamically allocate memory for the pseudo
    char* pseudo = strdup(PQgetvalue(res, 0, 0));
    PQclear(res);

    if (!pseudo) {
        fprintf(stderr, "❌ Erreur d'allocation mémoire pour le pseudo.\n");
        return NULL;
    }

    printf("✅ Pseudo du client récupéré avec succès: %s\n", pseudo);
    return pseudo;
}