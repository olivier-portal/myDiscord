#include <gtk/gtk.h>
#include "window.h"

void on_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *win = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(win), "Hello GTK4");
    gtk_window_set_default_size(GTK_WINDOW(win), 400, 300);
    gtk_widget_set_visible(win, TRUE);
}
