#include <gtk/gtk.h>
#include <glib.h> // pour GThread
#include "client_socket.h"
#include "../ui/window.h"
#include "../config/config.h"

static gpointer client_thread_func(gpointer data) {
    start_client();
    return NULL;
}

int main(int argc, char *argv[]) {
    // char *conninfo = load_conninfo_from_file("..\db_config.txt");
    // if (!conninfo) {
    //     g_printerr("❌ config.txt manquant ou illisible\n");
    //     return 1;
    // }


    g_thread_new("client_thread", (GThreadFunc)start_client, NULL);

    // 🚀 Lancer l'application GTK
    GtkApplication *app = gtk_application_new("com.jinx.myDiscord", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}

