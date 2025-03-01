#include "utils.h"
#include "radial_menu_controller.h"
#include "radial_menu_ui.h"  // Added to access RADIUS and BUTTON_SIZE
#include <math.h>
#include <stdio.h>

void utils_main_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_maximize(GTK_WINDOW(window));
    gtk_widget_set_opacity(window, 0.7);

    AppData *app_data = g_new0(AppData, 1);
    init_app_data(app_data, app, window);
    gtk_window_present(GTK_WINDOW(window));
}

void utils_init_app_data_apply_css(GtkWidget *window, GtkWidget *overlay) {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = "overlay { background-color: rgba(0, 0, 0, 0.5); }\n"
                      "terminal { background-color: transparent; }";
    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

void utils_init_app_data_log_display_info(void) {
    GdkDisplay *display = gdk_display_get_default();
    if (display) {
        printf("Display: %s, Compositor active: %s\n",
               gdk_display_get_name(display),
               gdk_display_is_composited(display) ? "yes" : "no");
    } else {
        printf("No display detected!\n");
    }
}

void utils_handle_app_key_press_log_key_event(guint keyval, guint keycode, GdkModifierType state) {
    const char *key_name = gdk_keyval_name(keyval);
    printf("Key pressed: keyval=%u (%s), keycode=%u, modifiers=%d\n", keyval,
           key_name ? key_name : "unknown", keycode, state);
}

void utils_radial_menu_ui_setup_draw_radial_menu(GtkDrawingArea *area, cairo_t *cr, int width,
                                                 int height, gpointer data) {
    AppData *app_data = (AppData *)data;
    int center_x = width / 2;
    int center_y = height / 2;
    int num_tabs = g_list_length(app_data->tabs);

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_arc(cr, center_x, center_y, RADIUS + BUTTON_SIZE / 2 + 10, 0, 2 * M_PI);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_set_line_width(cr, 2.0);
    cairo_arc(cr, center_x, center_y, RADIUS + BUTTON_SIZE / 2 + 10, 0, 2 * M_PI);
    cairo_stroke(cr);

    GList *iter;
    int i = 0;
    for (iter = app_data->tabs; iter; iter = iter->next, i++) {
        TerminalTab *tab = iter->data;
        double angle = 2 * M_PI * i / num_tabs - M_PI / 2;
        int x = center_x + RADIUS * cos(angle) - BUTTON_SIZE / 2;
        int y = center_y + RADIUS * sin(angle) - BUTTON_SIZE / 2;

        cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
        cairo_set_line_width(cr, 1.5);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x + BUTTON_SIZE / 2, y + BUTTON_SIZE / 2, BUTTON_SIZE / 2, 0, 2 * M_PI);
        cairo_stroke(cr);

        if (i == app_data->active_tab_index) {
            cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
            cairo_arc(cr, x + BUTTON_SIZE / 2, y + BUTTON_SIZE / 2, BUTTON_SIZE / 4, 0, 2 * M_PI);
            cairo_fill(cr);
        }
    }

    if (app_data->active_tab_index >= 0 && app_data->tabs) {
        TerminalTab *active_tab = g_list_nth_data(app_data->tabs, app_data->active_tab_index);
        if (active_tab) {
            cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
            cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
            cairo_set_font_size(cr, 14);
            cairo_text_extents_t extents;
            cairo_text_extents(cr, active_tab->label, &extents);
            cairo_move_to(cr, center_x - extents.width / 2, center_y + extents.height / 2);
            cairo_show_text(cr, active_tab->label);
        }
    }
}




void utils_radial_menu_ui_setup_radial_menu_clicked(GtkGesture *gesture, int n_press, gdouble x,
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

    if (distance > RADIUS - BUTTON_SIZE / 2 && distance < RADIUS + BUTTON_SIZE / 2) {
        double angle = atan2(click_y, click_x) + M_PI / 2;
        if (angle < 0) angle += 2 * M_PI;
        int tab_index = (int)(angle * num_tabs / (2 * M_PI)) % num_tabs;
        radial_menu_controller_switch_to_tab(app_data, tab_index);
    }
}
