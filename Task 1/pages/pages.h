//
// Created by REXE on 13.02.26.
//

#ifndef INFO_THEORY_PAGE_ROTATION_MATRIX_H
#define INFO_THEORY_PAGE_ROTATION_MATRIX_H

#include <gtk/gtk.h>
#include <adwaita.h>
#include "../../GTK Tools/string_matrix/string_matrix.h"
#include "../../GTK Tools/entry_deco/entry_deco.h"
#include "./../strings/ru_strings.h"

#define gtk_widget_set_margin(widget, margin)\
    gtk_widget_set_margin_top(widget, margin);\
    gtk_widget_set_margin_start(widget, margin);\
    gtk_widget_set_margin_end(widget, margin);\
    gtk_widget_set_margin_bottom(widget, margin);

#define gtk_button_new_with_icon_and_label(icon_name, text, spacing) ({ \
    GtkWidget *__btn = gtk_button_new();                                \
    GtkWidget *__box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing);\
    GtkWidget *__img = gtk_image_new_from_icon_name(icon_name);         \
    GtkWidget *__lbl = gtk_label_new(text);                             \
    \
    gtk_widget_set_halign(__box, GTK_ALIGN_CENTER);                     \
    gtk_box_append(GTK_BOX(__box), __img);                              \
    gtk_box_append(GTK_BOX(__box), __lbl);                              \
    \
    gtk_button_set_child(GTK_BUTTON(__btn), __box);                     \
    __btn;                                                              \
})

typedef struct AppPage
{
    GtkWidget *page;
    void *data;

    void (*on_create)(struct AppPage *page, GtkWidget *window);
    void (*on_file_dialog_result)(struct AppPage *page, GObject *original_object, GAsyncResult *res);
    void (*on_free)(struct AppPage *page);
} AppPage;

extern AppPage rotation_matrix_page;
extern AppPage vigenere_algorithm_page;
extern void (*open_file_dialog)(AppPage *page);

#endif //INFO_THEORY_PAGE_ROTATION_MATRIX_H
