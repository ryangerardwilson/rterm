#include "terminal.h"
#include "radial_menu_controller.h"
#include <stdio.h>

void init_app_data(AppData *app_data, GtkApplication *app, GtkWidget *window) {
  app_data->app = app;
  app_data->window = gtk_overlay_new();
  gtk_window_set_child(GTK_WINDOW(window), app_data->window);

  // Set CSS classes (still useful for styling, but no need for transparency
  // hacks)
  gtk_widget_set_css_classes(window, (const char *[]){"window", NULL});
  gtk_widget_set_css_classes(app_data->window,
                             (const char *[]){"overlay", NULL});

  // Simplified CSS: just basic styling, transparency handled by
  // gtk_widget_set_opacity
  GtkCssProvider *provider = gtk_css_provider_new();
  const char *css = "overlay { background-color: rgba(0, 0, 0, 0.5); }\n"
                    "terminal { background-color: transparent; }";
  gtk_css_provider_load_from_string(provider, css);
  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);

  // Debug: Keep display info for troubleshooting
  GdkDisplay *display = gdk_display_get_default();
  if (display) {
    printf("Display: %s, Compositor active: %s\n",
           gdk_display_get_name(display),
           gdk_display_is_composited(display) ? "yes" : "no");
  } else {
    printf("No display detected!\n");
  }

  app_data->tabs = NULL;
  app_data->current = NULL;
  app_data->radial_menu = NULL;
  app_data->active_tab_index = -1;

  radial_menu_controller_init(app_data);
  register_key_handlers(app_data, window);

  // Set opacity directly (e.g., 0.7 for 70% opaque)
  gtk_widget_set_opacity(window, 0.7); // Changed to window instead of overlay
}

TerminalTab *create_terminal_tab(AppData *app_data) {
  TerminalTab *tab = g_new0(TerminalTab, 1);
  tab->terminal = vte_terminal_new();
  tab->label = g_strdup_printf("Tab %d", g_list_length(app_data->tabs) + 1);
  tab->user_data = NULL;

  // Add CSS class to VTE terminal
  gtk_widget_set_css_classes(tab->terminal, (const char *[]){"terminal", NULL});

  printf("Creating new tab: %s\n", tab->label);

  vte_terminal_set_font_scale(VTE_TERMINAL(tab->terminal), 1.2);
  vte_terminal_set_colors(
      VTE_TERMINAL(tab->terminal),
      &(GdkRGBA){
          .red = 0.9, .green = 0.9, .blue = 0.9, .alpha = 1.0}, // Foreground
      NULL, // No background color
      NULL, 0);

  gtk_widget_set_margin_start(tab->terminal, 0);
  gtk_widget_set_margin_end(tab->terminal, 0);
  gtk_widget_set_margin_top(tab->terminal, 0);
  gtk_widget_set_margin_bottom(tab->terminal, 0);

  char *argv[] = {"/bin/bash", NULL};
  vte_terminal_spawn_async(VTE_TERMINAL(tab->terminal), VTE_PTY_DEFAULT, NULL,
                           argv, NULL, G_SPAWN_DEFAULT, NULL, NULL, NULL, -1,
                           NULL, NULL, NULL);

  return tab;
}

gboolean handle_app_key_press(GtkEventControllerKey *controller, guint keyval,
                              guint keycode, GdkModifierType state,
                              gpointer data) {
  AppData *app_data = (AppData *)data;

  // Log key events with more detail
  const char *key_name = gdk_keyval_name(keyval);
  printf("Key pressed: keyval=%u (%s), keycode=%u, modifiers=%d\n", keyval,
         key_name ? key_name : "unknown", keycode, state);

  // Check for Ctrl+Q
  if (state & GDK_CONTROL_MASK) {
    if (keyval == GDK_KEY_q || keyval == GDK_KEY_Q) {
      printf("Ctrl+Q detected, quitting...\n");
      g_application_quit(G_APPLICATION(app_data->app));
      return TRUE;
    }
  }

  return FALSE; // Let other handlers process if not Ctrl+Q
}

void register_key_handlers(AppData *app_data, GtkWidget *window) {
  // Register app-level key handler on the top-level window in capture phase
  GtkEventController *app_key_controller = gtk_event_controller_key_new();
  gtk_event_controller_set_propagation_phase(
      app_key_controller, GTK_PHASE_CAPTURE); // Capture before children
  gtk_widget_add_controller(window, app_key_controller);
  g_signal_connect(app_key_controller, "key-pressed",
                   G_CALLBACK(handle_app_key_press), app_data);

  // Register tab-level key handlers
  GList *iter;
  for (iter = app_data->tabs; iter; iter = iter->next) {
    TerminalTab *tab = iter->data;
    GtkEventController *key_controller = gtk_event_controller_key_new();
    gtk_widget_add_controller(tab->terminal, key_controller);
    g_signal_connect(key_controller, "key-pressed",
                     G_CALLBACK(radial_menu_controller_handle_key), app_data);
  }
}
