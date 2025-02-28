#include "radial_menu_controller.h"
#include "radial_menu_ui.h"
#include "terminal.h"
#include <stdio.h>

void radial_menu_controller_init(AppData *app_data) {
  TerminalTab *tab = create_terminal_tab(app_data);
  app_data->tabs = g_list_append(app_data->tabs, tab);
  app_data->current = tab;
  app_data->active_tab_index = 0;
  gtk_overlay_set_child(GTK_OVERLAY(app_data->window), tab->terminal);
  gtk_widget_set_visible(tab->terminal, TRUE);
  radial_menu_ui_setup(app_data);
}

void radial_menu_controller_switch_to_tab(AppData *app_data, int index) {
  if (index < 0 || index >= g_list_length(app_data->tabs))
    return;

  TerminalTab *new_tab = g_list_nth_data(app_data->tabs, index);
  if (app_data->current) {
    printf("Hiding current tab...\n");
    gtk_widget_set_visible(app_data->current->terminal, FALSE);
  }
  app_data->current = new_tab;
  app_data->active_tab_index = index;

  printf("Switching to %s...\n", new_tab->label);
  gtk_overlay_set_child(GTK_OVERLAY(app_data->window), new_tab->terminal);
  gtk_widget_set_visible(new_tab->terminal, TRUE);
  gtk_widget_grab_focus(new_tab->terminal);

  radial_menu_ui_update(app_data);
}

void radial_menu_controller_add_new_tab(AppData *app_data) {
  if (g_list_length(app_data->tabs) >= MAX_TABS) {
    printf("Max tabs reached (%d), cannot add more.\n", MAX_TABS);
    return;
  }

  TerminalTab *tab = create_terminal_tab(app_data);
  app_data->tabs = g_list_append(app_data->tabs, tab);
  int new_index = g_list_length(app_data->tabs) - 1;
  printf("Tab added, total tabs: %d\n", g_list_length(app_data->tabs));
  radial_menu_controller_switch_to_tab(app_data, new_index);

  // Show radial menu when a new tab is added
  if (app_data->radial_menu) {
    gtk_widget_set_visible(app_data->radial_menu, TRUE);
    printf("Showing radial menu with %d tabs...\n",
           g_list_length(app_data->tabs));
  }
  radial_menu_ui_update(app_data);

  // Register key handlers for the new tab
  GtkEventController *key_controller = gtk_event_controller_key_new();
  gtk_widget_add_controller(tab->terminal, key_controller);
  g_signal_connect(key_controller, "key-pressed",
                   G_CALLBACK(radial_menu_controller_handle_key), app_data);
}

void radial_menu_controller_remove_current_tab(AppData *app_data) {
  int num_tabs = g_list_length(app_data->tabs);
  if (num_tabs == 0) {
    printf("No tabs to remove, num_tabs=0\n");
    return;
  }

  printf("Ctrl+Backspace detected, closing tab %d...\n",
         app_data->active_tab_index + 1);
  GList *link = g_list_nth(app_data->tabs, app_data->active_tab_index);
  TerminalTab *tab = link->data;

  gtk_widget_unparent(tab->terminal);
  app_data->current = NULL;
  app_data->tabs = g_list_remove_link(app_data->tabs, link);
  g_free(tab->label);
  g_free(tab);
  g_list_free_1(link);

  num_tabs--;
  if (num_tabs == 0) {
    printf("No tabs left, quitting...\n");
    g_application_quit(G_APPLICATION(app_data->app));
  } else {
    int new_index = app_data->active_tab_index >= num_tabs
                        ? num_tabs - 1
                        : app_data->active_tab_index;
    printf("Switching to tab %d\n", new_index + 1);
    radial_menu_controller_switch_to_tab(app_data, new_index);
  }
}

gboolean radial_menu_controller_handle_key(GtkEventControllerKey *controller,
                                           guint keyval, guint keycode,
                                           GdkModifierType state,
                                           gpointer data) {
  AppData *app_data = (AppData *)data;
  VteTerminal *terminal = VTE_TERMINAL(
      gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller)));

  if (!(state & GDK_CONTROL_MASK))
    return FALSE;

  switch (keyval) {
  case GDK_KEY_plus:
  case GDK_KEY_equal:
    printf("Ctrl++ detected, zooming in...\n");
    vte_terminal_set_font_scale(terminal,
                                vte_terminal_get_font_scale(terminal) * 1.2);
    return TRUE;

  case GDK_KEY_minus:
    printf("Ctrl+- detected, zooming out...\n");
    vte_terminal_set_font_scale(terminal,
                                vte_terminal_get_font_scale(terminal) / 1.2);
    return TRUE;

  case GDK_KEY_t:
  case GDK_KEY_T:
    radial_menu_ui_toggle(app_data);
    return TRUE;

  case GDK_KEY_n:
  case GDK_KEY_N:
    printf("Ctrl+N detected, adding new tab...\n");
    radial_menu_controller_add_new_tab(app_data);
    return TRUE;

  case GDK_KEY_BackSpace:
    radial_menu_controller_remove_current_tab(app_data);
    return TRUE;

  case GDK_KEY_Left:
    if (g_list_length(app_data->tabs) > 1) {
      int new_index =
          (app_data->active_tab_index - 1 + g_list_length(app_data->tabs)) %
          g_list_length(app_data->tabs);
      printf("Ctrl+Left detected, switching to tab %d...\n", new_index + 1);
      radial_menu_controller_switch_to_tab(app_data, new_index);
    }
    return TRUE;

  case GDK_KEY_Right:
    if (g_list_length(app_data->tabs) > 1) {
      int new_index =
          (app_data->active_tab_index + 1) % g_list_length(app_data->tabs);
      printf("Ctrl+Right detected, switching to tab %d...\n", new_index + 1);
      radial_menu_controller_switch_to_tab(app_data, new_index);
    }
    return TRUE;
  }
  return FALSE;
}
