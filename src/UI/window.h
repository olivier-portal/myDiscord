#ifndef WINDOW_H
#define WINDOW_H

void on_activate(GtkApplication *app, gpointer user_data);
void send_message(GtkWidget *widget, gpointer data);
void apply_custom_theme();
void on_allocation_changed(GtkWidget *widget, GParamSpec *pspec, gpointer user_data);

#endif
