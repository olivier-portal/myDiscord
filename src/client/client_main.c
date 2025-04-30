#include <gtk/gtk.h>
#include <glib.h> // pour GThread
#include "client_socket.h"
#include "../ui/window.h"
#include "../ui/login_window.h"
#include "../config/config.h"

static gpointer client_thread_func(gpointer data) {
    start_client();
    return NULL;
}

static void on_activate(GtkApplication *app, gpointer user_data) {
    login_window(app);
};

int main(int argc, char *argv[]) {
    char *conninfo = load_conninfo_from_file("../db_config.txt");
    if (!conninfo) {
        g_printerr("db_config.txt missing or unreadable\n");
        return 1;
    }


    g_thread_new("client_thread", (GThreadFunc)start_client, NULL);

    // Launch gtk
    GtkApplication *app = gtk_application_new("com.jinx.myDiscord", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}

