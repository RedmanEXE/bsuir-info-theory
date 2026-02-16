//
// Created by REXE on 11.02.26.
//

#ifndef INFO_THEORY_SIMPLIFIED_COLUMNS_H
#define INFO_THEORY_SIMPLIFIED_COLUMNS_H

#include <gtk/gtk.h>

typedef enum
{
    STRING_MATRIX_CELL_NORMAL   = 0,
    STRING_MATRIX_CELL_BOLD     = 1 << 0
} MatrixCellFlags;

#define TYPE_MATRIX_ROW (matrix_row_get_type())
G_DECLARE_FINAL_TYPE(MatrixRow, matrix_row, APP, MATRIX_ROW, GObject)

typedef struct
{
    char *text;
    unsigned int style;
} MatrixRowItem;

struct _MatrixRow
{
    GObject parent_instance;
    MatrixRowItem* items;
    unsigned int n_columns;
};

typedef struct
{
    GtkWidget *scrolled_window;
    GtkColumnView *column_view;
    GListStore *store;
    GtkSingleSelection *selection;
    unsigned int cols;
} StringMatrix;

StringMatrix* string_matrix_new(unsigned int initial_cols, unsigned int initial_rows);
GtkWidget* string_matrix_get_widget(StringMatrix *mat);
void string_matrix_resize_rows(StringMatrix *mat, unsigned int new_row_count);
void string_matrix_resize_columns(StringMatrix *mat, unsigned int new_col_count);
void string_matrix_set_cell(StringMatrix *mat, unsigned int row_idx, unsigned int col_idx, const char *text, MatrixCellFlags flags);
void string_matrix_free(StringMatrix *mat);

#endif //INFO_THEORY_SIMPLIFIED_COLUMNS_H
