#include <gtk/gtk.h>
#include <adwaita.h>
#include "pages/pages.h"
#include "strings/ru_strings.h"
#include "../Build Utils/platform_build.h"

#define MATRIX_LENGTH 4
#define gtk_widget_set_margin(widget, margin)\
    gtk_widget_set_margin_top(widget, margin);\
    gtk_widget_set_margin_start(widget, margin);\
    gtk_widget_set_margin_end(widget, margin);\
    gtk_widget_set_margin_bottom(widget, margin);

static void on_open_file_open_dialog(AppPage *page);
static void on_open_file_save_dialog(AppPage *page);
void (*open_file_open_dialog)(AppPage *page) = on_open_file_open_dialog;
void (*open_file_save_dialog)(AppPage *page) = on_open_file_save_dialog;

static GtkFileDialog *file_dialog;
static GtkWindow *window;
static void on_open_file_dialog_result(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    AppPage *page = (AppPage *)user_data;
    page->on_file_dialog_open_result(page, source_object, res);
}

static void on_save_file_dialog_result(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    AppPage *page = (AppPage *)user_data;
    page->on_file_dialog_save_result(page, source_object, res);
}

static void on_open_file_open_dialog(AppPage *page)
{
    gtk_file_dialog_set_title(file_dialog, APP_STR_FILE_OPEN_DIALOG_TITLE);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, APP_STR_FILE_OPEN_DIALOG_FILTER_TXT);
    gtk_file_filter_add_pattern(filter, "*.txt");

    GListStore *filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
    g_list_store_append(filters, filter);

    gtk_file_dialog_set_filters(file_dialog, G_LIST_MODEL(filters));
    g_object_unref(filter);
    g_object_unref(filters);

    gtk_file_dialog_open(file_dialog, window, NULL, on_open_file_dialog_result, page);
}

static void on_open_file_save_dialog(AppPage *page)
{
    gtk_file_dialog_set_title(file_dialog, APP_STR_FILE_SAVE_DIALOG_TITLE);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, APP_STR_FILE_OPEN_DIALOG_FILTER_TXT);
    gtk_file_filter_add_pattern(filter, "*.txt");

    GListStore *filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
    g_list_store_append(filters, filter);

    gtk_file_dialog_set_filters(file_dialog, G_LIST_MODEL(filters));
    g_object_unref(filter);
    g_object_unref(filters);

    gtk_file_dialog_save(file_dialog, window, NULL, on_save_file_dialog_result, page);
}

static void on_activate(GtkApplication *app, gpointer user_data)
{
    window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_window_set_title(window, APP_STR_TITLE);
    gtk_window_set_default_size(window, 500, 200);

    // TabView
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_top(GTK_WIDGET(vbox), 10);
    GtkWidget *stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(stack), 300);

    GtkWidget *switcher = gtk_stack_switcher_new();
    gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(switcher), GTK_STACK(stack));
    gtk_widget_set_halign(switcher, GTK_ALIGN_CENTER);

    // Pages: rotation matrix
    rotation_matrix_page.on_create(&rotation_matrix_page, GTK_WIDGET(window));
    gtk_stack_add_titled(GTK_STACK(stack), rotation_matrix_page.page, "rotation_matrix_page", RTMP_STR_TITLE);
    // Pages: Vigenere algorithm
    vigenere_algorithm_page.on_create(&vigenere_algorithm_page, GTK_WIDGET(window));
    gtk_stack_add_titled(GTK_STACK(stack), vigenere_algorithm_page.page, "vigenere_algorithm_page", VALP_STR_TITLE);

    gtk_box_append(GTK_BOX(vbox), switcher);
    gtk_box_append(GTK_BOX(vbox), stack);

    gtk_window_set_child(window, GTK_WIDGET(vbox));
    gtk_window_present(window);
}

int main(int argc, char **argv)
{
    setup_env();

    AdwApplication *app = adw_application_new("dev.rexe.infoth.Task1", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    file_dialog = gtk_file_dialog_new();

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}