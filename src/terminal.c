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

    radial_menu_controller_init(app_data);
    register_key_handlers(app_data, window);
    gtk_widget_set_opacity(window, 0.7);
}

static void spawn_callback(VteTerminal *terminal, GPid pid, GError *error, gpointer user_data) {
    SpawnData *spawn_data = (SpawnData *)user_data;
    TerminalTab *tab = spawn_data->tab;
    char *shell_path = spawn_data->default_shell;
    AppData *app_data = spawn_data->app_data;

    if (error != NULL) {
        printf("Failed to spawn terminal: %s\n", error->message);
        g_error_free(error);
    } else if (pid > 0) {
        char *username = g_get_user_name();
        char *hostname = g_get_host_name();
        char *pwd = g_get_current_dir();
        if (username && hostname && pwd) {
            g_free(tab->label);
            tab->label = g_strdup_printf("%s@%s:%s", username, hostname, pwd);
            g_free(pwd);
        } else {
            char proc_path[64];
            snprintf(proc_path, sizeof(proc_path), "/proc/%d/exe", pid);
            char *real_path = g_file_read_link(proc_path, NULL);
            if (real_path) {
                g_free(shell_path);
                shell_path = real_path;
                g_free(tab->label);
                tab->label = g_strdup(shell_path);
                g_free(shell_path);
            }
        }
        printf("Terminal spawned with PID %d, updated label: %s\n", pid, tab->label);
        radial_menu_ui_update(app_data);
    }

    g_free(spawn_data);
}

static gboolean update_label_after_idle(gpointer user_data) {
    TerminalTab *tab = (TerminalTab *)user_data;
    AppData *app_data = (AppData *)tab->user_data;

    char *text = vte_terminal_get_text(VTE_TERMINAL(tab->terminal), NULL, NULL, NULL);
    if (text) {
        printf("Terminal text after idle: [%s]\n", text);

        // Find the last prompt-like string
        char *username = g_get_user_name();
        char *hostname = g_get_host_name();
        if (username && hostname) {
            char *search_pattern = g_strdup_printf("%s@%s:", username, hostname);
            char *last_prompt = text;
            char *pos = text;
            while ((pos = strstr(pos, search_pattern)) != NULL) {
                last_prompt = pos; // Keep the last occurrence
                pos += strlen(search_pattern);
            }

            if (last_prompt != text || strstr(last_prompt, search_pattern) == last_prompt) {
                char *path_start = last_prompt + strlen(search_pattern);
                char *path_end = path_start;
                while (*path_end && *path_end != ' ' && *path_end != '$' && *path_end != '\n') {
                    path_end++;
                }
                *path_end = '\0';

                if (strlen(path_start) > 0) {
                    g_free(tab->label);
                    tab->label = g_strdup_printf("%s@%s:%s", username, hostname, path_start);
                    printf("Updated label after command: %s\n", tab->label);
                    radial_menu_ui_update(app_data);
                } else {
                    printf("No valid path found after %s\n", search_pattern);
                }
            } else {
                printf("No prompt found with %s\n", search_pattern);
            }
            g_free(search_pattern);
        }
        g_free(text);
    } else {
        printf("No text retrieved from terminal after idle\n");
    }
    return G_SOURCE_REMOVE;
}

gboolean handle_terminal_key_press(GtkEventControllerKey *controller, guint keyval,
                                   guint keycode, GdkModifierType state, gpointer user_data) {
    AppData *app_data = (AppData *)user_data;
    TerminalTab *tab = app_data->current;
    if (!tab) return FALSE;

    if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) {
        tab->user_data = app_data; // Store app_data for the callback
        g_idle_add(update_label_after_idle, tab); // Run when idle
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

    gtk_widget_set_margin_start(tab->terminal, 0);
    gtk_widget_set_margin_end(tab->terminal, 0);
    gtk_widget_set_margin_top(tab->terminal, 0);
    gtk_widget_set_margin_bottom(tab->terminal, 0);

    // Add key press handler to update label after each command
    GtkEventController *key_controller = gtk_event_controller_key_new();
    gtk_widget_add_controller(tab->terminal, key_controller);
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(handle_terminal_key_press), app_data);

    SpawnData *spawn_data = g_new0(SpawnData, 1);
    spawn_data->tab = tab;
    spawn_data->default_shell = shell_path;
    spawn_data->app_data = app_data;

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
                             spawn_callback,
                             spawn_data);

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
    g_signal_connect(app_key_controller, "key-pressed", G_CALLBACK(handle_app_key_press), app_data);

    GList *iter;
    for (iter = app_data->tabs; iter; iter = iter->next) {
        TerminalTab *tab = iter->data;
        GtkEventController *key_controller = gtk_event_controller_key_new();
        gtk_widget_add_controller(tab->terminal, key_controller);
        g_signal_connect(key_controller, "key-pressed",
                         G_CALLBACK(radial_menu_controller_handle_key), app_data);
    }
}
