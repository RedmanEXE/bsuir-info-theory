//
// Created by REXE on 11.02.26.
//

#include "string_matrix.h"

G_DEFINE_TYPE(MatrixRow, matrix_row, G_TYPE_OBJECT)

static void matrix_row_finalize(GObject *object) {
    MatrixRow *self = APP_MATRIX_ROW(object);
    for (int i = 0; i < self->n_columns; i++) {
        g_free(self->columns[i]);
    }
    g_free(self->columns);
    G_OBJECT_CLASS(matrix_row_parent_class)->finalize(object);
}

static void matrix_row_class_init(MatrixRowClass *klass) {
    G_OBJECT_CLASS(klass)->finalize = matrix_row_finalize;
}

static void matrix_row_init(MatrixRow *self) {
    self->columns = NULL;
    self->n_columns = 0;
}

static MatrixRow *matrix_row_new(int n_columns) {
    MatrixRow *self = g_object_new(TYPE_MATRIX_ROW, NULL);
    self->n_columns = n_columns;
    self->columns = g_new0(char *, n_columns);
    for(int i = 0; i < n_columns; i++) {
        self->columns[i] = g_strdup("");
    }
    return self;
}

static void on_setup_label(GtkSignalListItemFactory *factory, GtkListItem *item, gpointer user_data) {
    GtkLabel *label = GTK_LABEL(gtk_label_new(NULL));
    gtk_label_set_xalign(label, 0.0f);
    gtk_list_item_set_child(item, GTK_WIDGET(label));
}

static void on_bind_label(GtkSignalListItemFactory *factory, GtkListItem *item, gpointer user_data) {
    GtkLabel *label = GTK_LABEL(gtk_list_item_get_child(item));
    MatrixRow *row = APP_MATRIX_ROW(gtk_list_item_get_item(item));
    int col_index = GPOINTER_TO_INT(user_data);

    if (row && col_index < row->n_columns) {
        gtk_label_set_text(label, row->columns[col_index]);
    }
}

StringMatrix* string_matrix_new(int initial_cols, int initial_rows) {
    StringMatrix *mat = g_new0(StringMatrix, 1);
    mat->cols = initial_cols;
    mat->store = g_list_store_new(TYPE_MATRIX_ROW);
    mat->selection = gtk_single_selection_new(G_LIST_MODEL(mat->store));
    mat->column_view = GTK_COLUMN_VIEW(gtk_column_view_new(GTK_SELECTION_MODEL(mat->selection)));
    mat->scrolled_window = gtk_scrolled_window_new();

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(mat->scrolled_window), GTK_WIDGET(mat->column_view));
    gtk_widget_set_vexpand(mat->scrolled_window, TRUE);
    gtk_column_view_set_show_row_separators(mat->column_view, TRUE);
    gtk_column_view_set_show_column_separators(mat->column_view, TRUE);
    gtk_column_view_set_reorderable(mat->column_view, FALSE);
    gtk_widget_set_visible(gtk_widget_get_first_child(GTK_WIDGET(mat->column_view)), FALSE);

    for (int i = 0; i < initial_cols; i++) {
        GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
        g_signal_connect(factory, "setup", G_CALLBACK(on_setup_label), NULL);
        g_signal_connect(factory, "bind", G_CALLBACK(on_bind_label), GINT_TO_POINTER(i));

        GtkColumnViewColumn *column = gtk_column_view_column_new("", factory);
        gtk_column_view_column_set_expand(column, TRUE);
        gtk_column_view_append_column(mat->column_view, column);
    }

    string_matrix_resize_rows(mat, initial_rows);

    return mat;
}

GtkWidget* string_matrix_get_widget(StringMatrix *mat) {
    return mat->scrolled_window;
}

void string_matrix_resize_rows(StringMatrix *mat, int new_row_count) {
    int current_rows = (int)g_list_model_get_n_items(G_LIST_MODEL(mat->store));

    if (new_row_count > current_rows) {
        for (int i = current_rows; i < new_row_count; i++) {
            MatrixRow *row = matrix_row_new(mat->cols);
            g_list_store_append(mat->store, row);
            g_object_unref(row);
        }
    } else if (new_row_count < current_rows) {
        int to_remove = current_rows - new_row_count;
        for (int i = 0; i < to_remove; i++) {
            g_list_store_remove(mat->store, new_row_count);
        }
    }
}

void string_matrix_set_text(StringMatrix *mat, int row_idx, int col_idx, const char *text) {
    MatrixRow *row = APP_MATRIX_ROW(g_list_model_get_item(G_LIST_MODEL(mat->store), row_idx));
    if (row) {
        if (col_idx >= 0 && col_idx < row->n_columns) {
            g_free(row->columns[col_idx]);
            row->columns[col_idx] = g_strdup(text ? text : "");

            g_list_store_remove(mat->store, row_idx);
            g_list_store_insert(mat->store, row_idx, row);
        }
        g_object_unref(row);
    }
}

void string_matrix_free(StringMatrix *mat) {
    if (!mat) return;
    g_object_unref(mat->store);
    g_object_unref(mat->selection);
    g_free(mat);
}
