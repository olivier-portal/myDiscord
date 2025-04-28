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

void on_allocation_changed(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data) {
    int width = allocation->width;
    GtkWidget *userSidebar = GTK_WIDGET(user_data);

    if (width < 900) {
        gtk_widget_set_visible(userSidebar, FALSE);
    } else {
        gtk_widget_set_visible(userSidebar, TRUE);
    }
}

void on_activate(GtkApplication *app, gpointer user_data) {
    apply_custom_theme();

    GtkWidget *window;
    GtkWidget *main_box;
    GtkWidget *channelSidebar;
    GtkWidget *chat_area;
    GtkWidget *userSidebar;
    GtkWidget *paned;

    // Create main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "myDiscord");
    gtk_window_set_default_size(GTK_WINDOW(window), 1280, 800);

    // header
    GtkWidget *header = gtk_header_bar_new();
    gtk_widget_set_name(header, "main-header");
    gtk_window_set_titlebar(GTK_WINDOW(window), header);

    // Create main horizontal box
    main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // Sidebar with channels
    channelSidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(channelSidebar, "channelSidebar");
    gtk_widget_set_size_request(channelSidebar, 100, -1); // min width
    
    // Add channels
    GtkWidget *label1 = gtk_label_new("📺 #general");
    GtkWidget *label2 = gtk_label_new("🎮 #gaming");
    gtk_box_append(GTK_BOX(channelSidebar), label1);
    gtk_box_append(GTK_BOX(channelSidebar), label2);

    // Main chat area
    chat_area = gtk_text_view_new();
    gtk_widget_set_name(chat_area, "chatview");
    gtk_widget_set_hexpand(chat_area, TRUE);
    gtk_widget_set_vexpand(chat_area, TRUE);

    // Sidebar with users
    userSidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(userSidebar, "userSidebar");
    gtk_widget_set_hexpand(userSidebar, FALSE);
    gtk_widget_set_vexpand(userSidebar, TRUE);

    // Add users
    GtkWidget *user1 = gtk_label_new("👤 User1");
    GtkWidget *user2 = gtk_label_new("👤 User2");
    gtk_box_append(GTK_BOX(userSidebar), user1);
    gtk_box_append(GTK_BOX(userSidebar), user2);

    // Paned between chat and users
    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_start_child(GTK_PANED(paned), chat_area);
    gtk_paned_set_end_child(GTK_PANED(paned), userSidebar);

    // Add everything into main_box
    gtk_box_append(GTK_BOX(main_box), channelSidebar); // Left sidebar
    gtk_box_append(GTK_BOX(main_box), paned);           // Center + Right sidebar

    // init separation position to force chat area to take the biggest place
    gtk_paned_set_position(GTK_PANED(paned), 1020);

    // Connect window size change to auto-hide userSidebar
    g_signal_connect(window, "notify::allocation", G_CALLBACK(on_allocation_changed), userSidebar);

    // Add the grid to the window
    gtk_window_present(GTK_WINDOW(window));
}
