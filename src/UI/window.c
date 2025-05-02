#include <gtk/gtk.h>
#include "window.h"
#include "theme.h"

GtkWidget *chat_entry_global = NULL;
GtkWidget *chat_area_global = NULL;

void send_message(GtkWidget *widget, gpointer data) {
    const gchar *text = gtk_editable_get_text(GTK_EDITABLE(chat_entry_global));

    if (text && *text != '\0') {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_area_global));
        
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);

        // Ajoute texte + saut de ligne
        gtk_text_buffer_insert(buffer, &end, text, -1);
        gtk_text_buffer_insert(buffer, &end, "\n", -1);

        // Efface l'entrée
        gtk_editable_set_text(GTK_EDITABLE(chat_entry_global), "");

        // Scroll tout en bas
        GtkTextMark *mark = gtk_text_buffer_get_insert(buffer);
        gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(chat_area_global), mark);
    }
}

void on_allocation_changed(GtkWidget *widget, GParamSpec *pspec, gpointer user_data) {
    GtkWidget *userSidebar = GTK_WIDGET(user_data);
    GtkWidget *paned = g_object_get_data(G_OBJECT(userSidebar), "paned");

    int width = gtk_widget_get_width(widget);
    gboolean is_visible = gtk_widget_get_visible(userSidebar);

    if (width < 900 && is_visible) {
        gtk_widget_set_visible(userSidebar, FALSE);
        gtk_paned_set_position(GTK_PANED(paned), width); // étire chat_area
    } 
    else if (width >= 900 && !is_visible) {
        gtk_widget_set_visible(userSidebar, TRUE);
        gtk_paned_set_position(GTK_PANED(paned), width - 260); // sidebar = 260px
    }
}

void mydiscord_main_window(GtkApplication *app, gpointer user_data) {
    apply_custom_theme();

    GtkWidget *window;
    GtkWidget *main_box;
    GtkWidget *channelSidebar;
    GtkWidget *chat_box;
    GtkWidget *chat_area;
    GtkWidget *chat_entry;
    GtkWidget *chat_entry_box;
    GtkWidget *chat_button;
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

    // Create the chat_box (vertical layout)
    chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_hexpand(chat_box, TRUE);
    gtk_widget_set_vexpand(chat_box, TRUE);

    // Create chat area (read only)
    chat_area = gtk_text_view_new();
    gtk_widget_set_name(chat_area, "chatview");
    gtk_text_view_set_editable(GTK_TEXT_VIEW(chat_area), FALSE); // read only
    gtk_widget_set_hexpand(chat_area, TRUE);
    gtk_widget_set_vexpand(chat_area, TRUE);

    gtk_box_append(GTK_BOX(chat_box), chat_area);

    // Create chat entry + send button (horizontal layout)
    chat_entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name(chat_entry_box, "chatentrybox");

    chat_entry = gtk_entry_new();
    gtk_widget_set_name(chat_entry, "chatentry");
    gtk_widget_set_hexpand(chat_entry, TRUE);

    chat_entry_global = chat_entry;
    chat_area_global = chat_area;    

    chat_button = gtk_button_new_with_label("Send");
    gtk_widget_set_name(chat_button, "chatbutton");
    

    gtk_box_append(GTK_BOX(chat_entry_box), chat_entry);
    gtk_box_append(GTK_BOX(chat_entry_box), chat_button);

    gtk_box_append(GTK_BOX(chat_box), chat_entry_box);

    // Connect entry key
    g_signal_connect(chat_entry, "activate", G_CALLBACK(send_message), NULL);

    // Connect click on send button
    g_signal_connect(chat_button, "clicked", G_CALLBACK(send_message), NULL);

    // Sidebar with users
    GtkWidget *userSidebarContent = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(userSidebarContent, "userSidebarContent");

    // Add users
    GtkWidget *user1 = gtk_label_new("👤 User1");
    GtkWidget *user2 = gtk_label_new("👤 User2");
    gtk_box_append(GTK_BOX(userSidebarContent), user1);
    gtk_box_append(GTK_BOX(userSidebarContent), user2);

    // Encapsulate in a scrolledwindow
    userSidebar = gtk_scrolled_window_new();
    gtk_widget_set_name(userSidebar, "userSidebar");
    gtk_widget_set_size_request(userSidebar, 100, -1);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(userSidebar), userSidebarContent);

    // Paned between chat and users
    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_start_child(GTK_PANED(paned), chat_box);
    gtk_paned_set_end_child(GTK_PANED(paned), userSidebar);
    g_object_set_data(G_OBJECT(userSidebar), "paned", paned);

    // Add everything into main_box
    gtk_box_append(GTK_BOX(main_box), channelSidebar); // Left sidebar
    gtk_box_append(GTK_BOX(main_box), paned);           // Center + Right sidebar

    // init separation position to force chat area to take the biggest place
    gtk_paned_set_position(GTK_PANED(paned), 1020);

    // Connect paned width for auto-hide userSidebar
    g_signal_connect(paned, "notify::width", G_CALLBACK(on_allocation_changed), userSidebar);

    // Add the grid to the window
    gtk_window_present(GTK_WINDOW(window));
}
