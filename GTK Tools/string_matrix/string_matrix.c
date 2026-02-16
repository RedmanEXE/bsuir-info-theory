//
// Created by REXE on 11.02.26.
//

#include "string_matrix.h"

G_DEFINE_TYPE(MatrixRow, matrix_row, G_TYPE_OBJECT)

static void matrix_row_finalize(GObject *object)
{
    MatrixRow *self = APP_MATRIX_ROW(object);
    for (int i = 0; i < self->n_columns; i++) {
        g_free(self->items[i].text);
    }
    g_free(self->items);
    G_OBJECT_CLASS(matrix_row_parent_class)->finalize(object);
}

static void matrix_row_class_init(MatrixRowClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = matrix_row_finalize;
}

static void matrix_row_init(MatrixRow *self)
{
    self->items = NULL;
    self->n_columns = 0;
}

static MatrixRow *matrix_row_new(unsigned int n_columns)
{
    MatrixRow *self = g_object_new(TYPE_MATRIX_ROW, NULL);
    self->n_columns = n_columns;
    self->items = g_new0(MatrixRowItem, n_columns);

    for (int i = 0; i < n_columns; i++)
    {
        self->items[i].text = g_strdup("");
        self->items[i].style = STRING_MATRIX_CELL_NORMAL;
    }
    return self;
}

static void on_setup_label(GtkSignalListItemFactory *factory, GtkListItem *item, gpointer user_data)
{
    GtkLabel *label = GTK_LABEL(gtk_label_new(NULL));
    gtk_label_set_xalign(label, 0.5f);
    gtk_list_item_set_child(item, GTK_WIDGET(label));
}

static void on_bind_label(GtkSignalListItemFactory *factory, GtkListItem *item, gpointer user_data)
{
    GObject *obj = gtk_list_item_get_item(item);
    if (!obj || !APP_IS_MATRIX_ROW(obj)) return;

    MatrixRow *row = APP_MATRIX_ROW(obj);
    GtkLabel *label = GTK_LABEL(gtk_list_item_get_child(item));
    int col_index = GPOINTER_TO_INT(user_data);

    if (col_index >= 0 && col_index < row->n_columns && row->items)
    {
        gtk_label_set_text(label, row->items[col_index].text);
        PangoAttrList *attrs = pango_attr_list_new();
        PangoFontDescription *font_desc = pango_font_description_new();
        pango_font_description_set_size(font_desc, 14 * PANGO_SCALE);
        if (row->items[col_index].style & STRING_MATRIX_CELL_BOLD)
            pango_font_description_set_weight(font_desc, PANGO_WEIGHT_BOLD);
        pango_attr_list_insert(attrs, pango_attr_font_desc_new(font_desc));
        gtk_label_set_attributes(label, attrs);
        pango_attr_list_unref(attrs);
        pango_font_description_free(font_desc);
    }
}

static void on_teardown_label(GtkSignalListItemFactory *factory, GtkListItem *item, gpointer user_data)
{
    GObject *obj = gtk_list_item_get_item(item);
    if (!obj || !APP_IS_MATRIX_ROW(obj)) return;

    g_object_unref(obj);
}

static void append_view_column(StringMatrix *mat, unsigned int col_index)
{
    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
    g_signal_connect(factory, "setup", G_CALLBACK(on_setup_label), NULL);
    g_signal_connect(factory, "bind", G_CALLBACK(on_bind_label), GINT_TO_POINTER(col_index));
    g_signal_connect(factory, "teardown", G_CALLBACK(on_teardown_label), GINT_TO_POINTER(col_index));
    GtkColumnViewColumn *column = gtk_column_view_column_new("", factory);
    gtk_column_view_column_set_expand(column, TRUE);
    gtk_column_view_append_column(mat->column_view, column);
}

StringMatrix* string_matrix_new(unsigned int initial_cols, unsigned int initial_rows)
{
    StringMatrix *mat = (StringMatrix *)calloc(1, sizeof(StringMatrix));

    mat->store = g_list_store_new(TYPE_MATRIX_ROW);
    mat->selection = gtk_single_selection_new(G_LIST_MODEL(mat->store));
    mat->column_view = GTK_COLUMN_VIEW(gtk_column_view_new(GTK_SELECTION_MODEL(mat->selection)));
    mat->scrolled_window = gtk_scrolled_window_new();

    gtk_widget_add_css_class(GTK_WIDGET(mat->column_view), "card");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(mat->scrolled_window), GTK_WIDGET(mat->column_view));
    gtk_widget_set_vexpand(mat->scrolled_window, TRUE);
    gtk_widget_set_hexpand(mat->scrolled_window, TRUE);
    gtk_column_view_set_show_row_separators(mat->column_view, TRUE);
    gtk_column_view_set_show_column_separators(mat->column_view, TRUE);
    gtk_column_view_set_reorderable(mat->column_view, FALSE);
    gtk_widget_set_visible(gtk_widget_get_first_child(GTK_WIDGET(mat->column_view)), FALSE);

    string_matrix_resize_columns(mat, initial_cols);
    string_matrix_resize_rows(mat, initial_rows);

    return mat;
}

GtkWidget* string_matrix_get_widget(StringMatrix *mat)
{
    return GTK_WIDGET(mat->scrolled_window);
}

void string_matrix_resize_rows(StringMatrix *mat, unsigned int new_row_count)
{
    GListModel *columns = gtk_column_view_get_columns(mat->column_view);
    const unsigned int columns_len = g_list_model_get_n_items(columns);
    unsigned int rows_len = g_list_model_get_n_items(G_LIST_MODEL(mat->store));

    if (rows_len == new_row_count)
        return;

    if (rows_len > new_row_count)
    {
        while (rows_len > new_row_count)
        {
            g_list_store_remove(mat->store, rows_len - 1);
            rows_len--;
        }
    } else if (rows_len < new_row_count)
    {
        while (rows_len < new_row_count)
        {
            MatrixRow *row = matrix_row_new(columns_len);
            g_list_store_append(mat->store, row);
            g_object_unref(row);
            rows_len++;
        }
    }
}

void string_matrix_resize_columns(StringMatrix *mat, unsigned int new_col_count)
{
    GListModel *columns = gtk_column_view_get_columns(mat->column_view);
    unsigned int columns_len = g_list_model_get_n_items(columns);
    unsigned int rows_len = g_list_model_get_n_items(G_LIST_MODEL(mat->store));

    if (columns_len == new_col_count)
        return;

    if (columns_len > new_col_count)
    {
        while (columns_len > new_col_count)
        {
            GtkColumnViewColumn *col = g_list_model_get_item(columns, columns_len - 1);
            gtk_column_view_remove_column(mat->column_view, col);
            g_object_unref(col);
            columns_len--;
        }
    } else if (columns_len < new_col_count)
    {
        while (columns_len < new_col_count)
        {
            append_view_column(mat, columns_len);
            columns_len++;
        }
    }

    mat->cols = new_col_count;
    g_list_model_items_changed(G_LIST_MODEL(mat->store), 0, rows_len, rows_len);
}

void string_matrix_set_cell(StringMatrix *mat, unsigned int row_idx, unsigned int col_idx, const char *text, MatrixCellFlags flags)
{
    MatrixRow *row = APP_MATRIX_ROW(g_list_model_get_item(G_LIST_MODEL(mat->store), row_idx));
    if (row)
    {
        if (col_idx < row->n_columns)
        {
            g_free(row->items[col_idx].text);
            row->items[col_idx].text = g_strdup(text ? text : "");
            row->items[col_idx].style = (unsigned int) flags;

            g_list_store_remove(mat->store, row_idx);
            g_list_store_insert(mat->store, row_idx, row);
        }
        g_object_unref(row);
    }
}

void string_matrix_free(StringMatrix *mat)
{
    if (!mat) return;

    g_object_unref(mat->store);
    g_object_unref(mat->selection);
    g_free(mat);
}