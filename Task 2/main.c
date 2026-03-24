#include <stdio.h>

#include <gtk/gtk.h>
#include <adwaita.h>

#include "../Build Utils/platform_build.h"
#include "pages/pages.h"
#include "strings/ru_strings.h"

static GtkFileDialog *file_dialog;
static GtkWindow *window;

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
    gtk_file_dialog_open(file_dialog, window, NULL, on_open_file_dialog_result, page);
}

static void on_open_file_save_dialog(AppPage *page)
{
    gtk_file_dialog_set_title(file_dialog, APP_STR_FILE_SAVE_DIALOG_TITLE);
    gtk_file_dialog_save(file_dialog, window, NULL, on_save_file_dialog_result, page);
}

static void on_activate(GtkApplication *app, gpointer user_data)
{
    window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_window_set_title(window, APP_STR_TITLE);
    gtk_window_set_default_size(window, 500, 200);

    // Pages: LFSR
    lfsr_page.on_create(&lfsr_page, GTK_WIDGET(window));

    gtk_window_set_child(window, GTK_WIDGET(lfsr_page.page));
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
