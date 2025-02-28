#include "radial_menu_ui.h"
#include "radial_menu_controller.h"
#include <math.h>
#include <stdio.h>

static void draw_radial_menu(GtkDrawingArea *area, cairo_t *cr, int width,
                             int height, gpointer data) {
  AppData *app_data = (AppData *)data;
  int center_x = width / 2;
  int center_y = height / 2;
  int num_tabs = g_list_length(app_data->tabs);

  // Draw solid black background for the radial menu
  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Solid black
  cairo_arc(cr, center_x, center_y, RADIUS + BUTTON_SIZE / 2 + 10, 0, 2 * M_PI);
  cairo_fill(cr);

  // Draw green border around the radial menu
  cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); // Green
  cairo_set_line_width(cr, 2.0);           // 2px border
  cairo_arc(cr, center_x, center_y, RADIUS + BUTTON_SIZE / 2 + 10, 0, 2 * M_PI);
  cairo_stroke(cr);

  GList *iter;
  int i = 0;
  for (iter = app_data->tabs; iter; iter = iter->next, i++) {
    TerminalTab *tab = iter->data;
    double angle = 2 * M_PI * i / num_tabs - M_PI / 2;
    int x = center_x + RADIUS * cos(angle) - BUTTON_SIZE / 2;
    int y = center_y + RADIUS * sin(angle) - BUTTON_SIZE / 2;

    // Draw empty green circle for each tab, ensuring no connecting lines
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); // Green
    cairo_set_line_width(cr, 1.5);
    cairo_new_sub_path(cr); // Start a new path to avoid connecting lines
    cairo_arc(cr, x + BUTTON_SIZE / 2, y + BUTTON_SIZE / 2, BUTTON_SIZE / 2, 0,
              2 * M_PI);
    cairo_stroke(cr);

    // Fill selected tab with a green dot
    if (i == app_data->active_tab_index) {
      cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); // Green
      cairo_arc(cr, x + BUTTON_SIZE / 2, y + BUTTON_SIZE / 2, BUTTON_SIZE / 4,
                0,
                2 * M_PI); // Smaller radius for the dot
      cairo_fill(cr);
    }
  }

  // Draw the active tab's name in the center of the radial menu
  if (app_data->active_tab_index >= 0 && app_data->tabs) {
    TerminalTab *active_tab =
        g_list_nth_data(app_data->tabs, app_data->active_tab_index);
    if (active_tab) {
      cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White text
      cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                             CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size(cr, 14);
      cairo_text_extents_t extents;
      cairo_text_extents(cr, active_tab->label, &extents);
      cairo_move_to(cr, center_x - extents.width / 2,
                    center_y + extents.height / 2);
      cairo_show_text(cr, active_tab->label);
    }
  }
}

static void radial_menu_clicked(GtkGesture *gesture, int n_press, gdouble x,
                                gdouble y, gpointer data);

void radial_menu_ui_setup(AppData *app_data) {
  app_data->radial_menu = gtk_drawing_area_new();
  gtk_widget_set_size_request(app_data->radial_menu,
                              RADIUS * 2 + BUTTON_SIZE + 20,
                              RADIUS * 2 + BUTTON_SIZE + 20);
  gtk_widget_set_halign(app_data->radial_menu, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(app_data->radial_menu, GTK_ALIGN_CENTER);
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(app_data->radial_menu),
                                 draw_radial_menu, app_data, NULL);

  gtk_overlay_add_overlay(GTK_OVERLAY(app_data->window), app_data->radial_menu);
  gtk_widget_set_visible(app_data->radial_menu, FALSE);

  GtkGesture *click_gesture = gtk_gesture_click_new();
  g_signal_connect(click_gesture, "pressed", G_CALLBACK(radial_menu_clicked),
                   app_data);
  gtk_widget_add_controller(app_data->radial_menu,
                            GTK_EVENT_CONTROLLER(click_gesture));

  printf("Radial menu initialized with %d tabs...\n",
         g_list_length(app_data->tabs));
}

void radial_menu_ui_toggle(AppData *app_data) {
  if (!app_data->radial_menu) {
    radial_menu_ui_setup(app_data);
    return;
  }
  gboolean visible = gtk_widget_get_visible(app_data->radial_menu);
  gtk_widget_set_visible(app_data->radial_menu, !visible);
  printf("Ctrl+T detected, %s radial menu...\n",
         visible ? "hiding" : "showing");
  if (!visible) {
    printf("Showing radial menu with %d tabs...\n",
           g_list_length(app_data->tabs));
    gtk_widget_queue_draw(app_data->radial_menu);
  }
}

void radial_menu_ui_update(AppData *app_data) {
  if (app_data->radial_menu) {
    gtk_widget_queue_draw(app_data->radial_menu);
  }
}

static void radial_menu_clicked(GtkGesture *gesture, int n_press, gdouble x,
                                gdouble y, gpointer data) {
  AppData *app_data = (AppData *)data;
  int width = gtk_widget_get_width(app_data->radial_menu);
  int height = gtk_widget_get_height(app_data->radial_menu);
  int center_x = width / 2;
  int center_y = height / 2;
  int num_tabs = g_list_length(app_data->tabs);

  double click_x = x - center_x;
  double click_y = y - center_y;
  double distance = sqrt(click_x * click_x + click_y * click_y);

  if (distance > RADIUS - BUTTON_SIZE / 2 &&
      distance < RADIUS + BUTTON_SIZE / 2) {
    double angle = atan2(click_y, click_x) + M_PI / 2;
    if (angle < 0)
      angle += 2 * M_PI;
    int tab_index = (int)(angle * num_tabs / (2 * M_PI)) % num_tabs;
    radial_menu_controller_switch_to_tab(app_data, tab_index);
  }
}
