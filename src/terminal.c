#include "terminal.h"
#include "radial_menu_controller.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

void init_app_data(AppData *app_data, GtkApplication *app, GtkWidget *window) {
    app_data->app = app;
    app_data->window = gtk_overlay_new();
    gtk_window_set_child(GTK_WINDOW(window), app_data->window);
    gtk_widget_set_css_classes(window, (const char *[]){"window", NULL});
    gtk_widget_set_css_classes(app_data->window, (const char *[]){"overlay", NULL});

    utils_init_app_data_apply_css(window, app_data->window);
    utils_init_app_data_log_display_info();

    app_data->tabs = NULL;
    app_data->current = NULL;
    app_data->radial_menu = NULL;
    app_data->active_tab_index = -1;

    // Create a container for terminals that expands
    GtkWidget *terminal_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(terminal_container, TRUE); // Expand vertically to fill overlay
    gtk_widget_set_hexpand(terminal_container, TRUE); // Expand horizontally to fill overlay
    gtk_overlay_set_child(GTK_OVERLAY(app_data->window), terminal_container);
    app_data->terminal_container = terminal_container;

    radial_menu_controller_init(app_data);
    printf("After radial init, app_data->tabs length: %d\n", g_list_length(app_data->tabs));

    GList *iter;
    int i = 0;
    for (iter = app_data->tabs; iter; iter = iter->next, i++) {
        TerminalTab *tab = (TerminalTab *)iter->data;
        printf("Tab %d in app_data->tabs: PID %d, label %s\n", i + 1, tab->pid, tab->label);
    }

    radial_menu_ui_setup(app_data);
    register_key_handlers(app_data, window);
    gtk_widget_set_opacity(window, 0.7);
}

gboolean handle_terminal_key_press(GtkEventControllerKey *controller, guint keyval,
                                   guint keycode, GdkModifierType state, gpointer user_data) {
    AppData *app_data = (AppData *)user_data;
    TerminalTab *tab = app_data->current;
    if (!tab) {
        printf("No current tab in key press handler\n");
        return FALSE;
    }

    if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) {
        printf("Enter pressed on tab with PID %d\n", tab->pid);
        tab->user_data = app_data;
        g_idle_add(utils_update_label_after_idle, tab);
    }
    return FALSE;
}

TerminalTab *create_terminal_tab(AppData *app_data) {
    TerminalTab *tab = g_new0(TerminalTab, 1);
    tab->terminal = vte_terminal_new();
    
    char *shell_path = g_strdup("/bin/bash");
    tab->label = g_strdup(shell_path);
    
    gtk_widget_set_css_classes(tab->terminal, (const char *[]){"terminal", NULL});
    printf("Creating new tab: %s\n", tab->label);
    
    vte_terminal_set_font_scale(VTE_TERMINAL(tab->terminal), 1.2);
    vte_terminal_set_colors(VTE_TERMINAL(tab->terminal),
                            &(GdkRGBA){.red = 0.9, .green = 0.9, .blue = 0.9, .alpha = 1.0},
                            NULL, NULL, 0);

    gtk_widget_set_vexpand(tab->terminal, TRUE); // Terminal expands to fill box height
    gtk_widget_set_hexpand(tab->terminal, TRUE); // Terminal expands to fill box width
    gtk_widget_set_margin_start(tab->terminal, 0);
    gtk_widget_set_margin_end(tab->terminal, 0);
    gtk_widget_set_margin_top(tab->terminal, 0);
    gtk_widget_set_margin_bottom(tab->terminal, 0);

    GtkEventController *key_controller = gtk_event_controller_key_new();
    gtk_widget_add_controller(tab->terminal, key_controller);
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(handle_terminal_key_press), app_data);

    SpawnData *spawn_data = g_new0(SpawnData, 1);
    spawn_data->tab = tab;
    spawn_data->default_shell = shell_path;
    spawn_data->app_data = app_data;

    GError *error = NULL;
    tab->pty = vte_pty_new_sync(VTE_PTY_DEFAULT, NULL, &error);
    if (!tab->pty || error) {
        printf("Failed to create PTY: %s\n", error ? error->message : "Unknown error");
        g_clear_error(&error);
        g_free(tab);
        g_free(spawn_data);
        g_free(shell_path);
        return NULL;
    }

    vte_terminal_set_pty(VTE_TERMINAL(tab->terminal), tab->pty);
    char *argv[] = {"/bin/bash", NULL};
    vte_terminal_spawn_async(VTE_TERMINAL(tab->terminal),
                             VTE_PTY_DEFAULT,
                             NULL,
                             argv,
                             NULL,
                             G_SPAWN_DEFAULT,
                             NULL,
                             NULL,
                             NULL,
                             -1,
                             NULL,
                             utils_spawn_callback,
                             spawn_data);

    app_data->tabs = g_list_append(app_data->tabs, tab);
    gtk_box_append(GTK_BOX(app_data->terminal_container), tab->terminal);
    if (g_list_length(app_data->tabs) == 1) {
        app_data->current = tab;
        app_data->active_tab_index = 0;
        gtk_widget_set_visible(tab->terminal, TRUE);
    } else {
        gtk_widget_set_visible(tab->terminal, FALSE);
    }
    printf("Tab added, total tabs: %d, active index: %d\n", g_list_length(app_data->tabs), app_data->active_tab_index);

    return tab;
}

gboolean handle_app_key_press(GtkEventControllerKey *controller, guint keyval,
                              guint keycode, GdkModifierType state, gpointer data) {
    AppData *app_data = (AppData *)data;
    utils_handle_app_key_press_log_key_event(keyval, keycode, state);

    if (state & GDK_CONTROL_MASK && (keyval == GDK_KEY_q || keyval == GDK_KEY_Q)) {
        printf("Ctrl+Q detected, quitting...\n");
        g_application_quit(G_APPLICATION(app_data->app));
        return TRUE;
    }
    return FALSE;
}

void register_key_handlers(AppData *app_data, GtkWidget *window) {
    GtkEventController *app_key_controller = gtk_event_controller_key_new();
    gtk_event_controller_set_propagation_phase(app_key_controller, GTK_PHASE_CAPTURE);
    gtk_widget_add_controller(window, app_key_controller);
    g_signal_connect(app_key_controller, "key-pressed", G_CALLBACK(radial_menu_controller_handle_key), app_data);
    g_signal_connect(app_key_controller, "key-pressed", G_CALLBACK(handle_app_key_press), app_data);
}
