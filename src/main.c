#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <direct.h> // Use this for _chdir on Windows
#include "database/db_connection.h"

// 🖼️ Fonction appelée à l'ouverture de la fenêtre GTK
static void on_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "myDiscord - GTK4 + PostgreSQL");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
    gtk_widget_set_visible(window, TRUE); // Replaces gtk_widget_show
}

int main(int argc, char *argv[]) {
    printf("Changement du répertoire de travail...\n");
    if (_chdir("d:/_Thibault/Documents/_Plateforme/!Dev_log_1/MyDiscord/myDiscord") != 0) { // Use _chdir for Windows
        perror("❌ Impossible de changer le répertoire de travail");
        system("pause");
        return 1;
    }

    printf("Lecture de la configuration de la base de données...\n");
    DBConfig config;
    if (!load_db_config("db_config.txt", &config)) { // Load the database configuration
        fprintf(stderr, "❌ Impossible de charger la configuration de la base de données.\n");
        system("pause");
        return 1;
    }

    printf("Connexion à PostgreSQL...\n");
    PGconn *conn = connect_to_database(&config); // Connect to the database
    if (!conn) {
        system("pause");
        return 1;
    }

    printf("✅ Connexion PostgreSQL réussie.\n");
    disconnect_from_database(conn); // Disconnect from the database

    printf("Lancement de GTK...\n");
    GtkApplication *app = gtk_application_new("com.jinx.myDiscord", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    printf("Fin du programme (GTK fermée).\n");
    return status;
}
