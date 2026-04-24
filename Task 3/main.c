#include <stdio.h>

#include <gtk/gtk.h>
#include <adwaita.h>

#include "../Build Utils/platform_build.h"
#include "pages/pages.h"
#include "strings/ru_strings.h"

static GtkFileDialog *file_dialog;
static AdwApplicationWindow *window;

static void on_open_file_open_dialog(AppPage *page);
static void on_open_file_save_dialog(AppPage *page);
void (*open_file_open_dialog)(AppPage *page) = on_open_file_open_dialog;
void (*open_file_save_dialog)(AppPage *page) = on_open_file_save_dialog;

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
    gtk_file_dialog_open(file_dialog, GTK_WINDOW(window), NULL, on_open_file_dialog_result, page);
}

static void on_open_file_save_dialog(AppPage *page)
{
    gtk_file_dialog_set_title(file_dialog, APP_STR_FILE_SAVE_DIALOG_TITLE);
    gtk_file_dialog_save(file_dialog, GTK_WINDOW(window), NULL, on_save_file_dialog_result, page);
}

static void on_activate(GtkApplication *app, gpointer user_data)
{
    platform_activate();

    window = ADW_APPLICATION_WINDOW(adw_application_window_new(app));
    gtk_window_set_title(GTK_WINDOW(window), APP_STR_TITLE);
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 200);

    GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = adw_header_bar_new();
    gtk_box_append(GTK_BOX(container), header);

    // Pages: Rabin
    rabin_page.on_create(&rabin_page, GTK_WIDGET(window));

    gtk_box_append(GTK_BOX(container), rabin_page.page);

    adw_application_window_set_content(ADW_APPLICATION_WINDOW(window), GTK_WIDGET(container));
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
    setup_env();

    AdwApplication *app = adw_application_new("dev.rexe.infoth.Task3", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    file_dialog = gtk_file_dialog_new();

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
