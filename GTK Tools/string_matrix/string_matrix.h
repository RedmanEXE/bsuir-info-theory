//
// Created by REXE on 11.02.26.
//

#ifndef INFO_THEORY_SIMPLIFIED_COLUMNS_H
#define INFO_THEORY_SIMPLIFIED_COLUMNS_H

#ifndef STRING_MATRIX_H
#define STRING_MATRIX_H

#include <gtk/gtk.h>

#define TYPE_MATRIX_ROW (matrix_row_get_type())
G_DECLARE_FINAL_TYPE(MatrixRow, matrix_row, APP, MATRIX_ROW, GObject)

struct _MatrixRow {
    GObject parent_instance;
    char **columns;
    int n_columns;
};

typedef struct {
    GtkWidget *scrolled_window;
    GtkColumnView *column_view;
    GListStore *store;
    GtkSingleSelection *selection;
    int cols;
} StringMatrix;

StringMatrix* string_matrix_new(int initial_cols, int initial_rows);
GtkWidget* string_matrix_get_widget(StringMatrix *mat);
void string_matrix_resize_rows(StringMatrix *mat, int new_row_count);
void string_matrix_set_text(StringMatrix *mat, int row_idx, int col_idx, const char *text);
void string_matrix_free(StringMatrix *mat);

#endif

#endif //INFO_THEORY_SIMPLIFIED_COLUMNS_H
