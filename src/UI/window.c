#include <gtk/gtk.h>
#include "window.h"

void print_hello(GtkWidget *widget, gpointer data) {
    g_print("Hello World\n");
}

void apply_custom_theme() {
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();

#ifdef _WIN32
    gchar *theme_path = g_strdup("C:\\Users\\porta\\.themes\\MyBreeze-Dark-GTK\\gtk-4.0\\gtk.css");
#else
    gchar *theme_path = g_build_filename(g_get_home_dir(), ".themes", "MyBreeze-Dark-GTK", "gtk-4.0", "gtk.css", NULL);
#endif

// MyBreeze-Dark theme
GFile *theme_file = g_file_new_for_path("C:/Users/porta/.themes/MyBreeze-Dark-GTK/gtk-4.0/gtk.css");
gtk_css_provider_load_from_file(provider, theme_file);
g_object_unref(theme_file);


// Custom css
GtkCssProvider *custom = gtk_css_provider_new();
gchar *css_path = g_build_filename("..", "src", "ui", "custom.css", NULL);
GFile *custom_file = g_file_new_for_path(css_path);

gtk_css_provider_load_from_file(custom, custom_file);
g_object_unref(custom_file);

gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_THEME);
gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(custom), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

g_free(css_path);
g_free(theme_path);
}

void on_activate(GtkApplication *app, gpointer user_data) {
    apply_custom_theme();

    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *sidebar;
    GtkWidget *label1;
    GtkWidget *label2;
    GtkWidget *chat_area;

    // Create main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "myDiscord");
    gtk_window_set_default_size(GTK_WINDOW(window), 1280, 800);

    // Create grid container
    grid = gtk_grid_new();

    // Sidebar with channels
    sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(sidebar, "sidebar");
    gtk_widget_set_hexpand(sidebar, FALSE);
    gtk_widget_set_vexpand(sidebar, TRUE);    
    label1 = gtk_label_new("📺 #general");
    label2 = gtk_label_new("🎮 #gaming");
    gtk_box_append(GTK_BOX(sidebar), label1);
    gtk_box_append(GTK_BOX(sidebar), label2);

    // Main chat area
    chat_area = gtk_text_view_new();
    gtk_widget_set_name(chat_area, "chatview");
    gtk_widget_set_hexpand(chat_area, TRUE);
    gtk_widget_set_vexpand(chat_area, TRUE);

    // header
    GtkWidget *header = gtk_header_bar_new();
    gtk_widget_set_name(header, "main-header");
    gtk_window_set_titlebar(GTK_WINDOW(window), header);

    // Attach widgets to the grid
    gtk_grid_attach(GTK_GRID(grid), sidebar, 0, 0, 1, 1);   // Left column
    gtk_grid_attach(GTK_GRID(grid), chat_area, 1, 0, 1, 1); // Right column

    // Add the grid to the window
    gtk_window_set_child(GTK_WINDOW(window), grid);
    gtk_window_present(GTK_WINDOW(window));
}