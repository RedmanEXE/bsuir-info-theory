//
// Created by REXE on 13.02.26.
//

#include "pages.h"
#include "../crypto/rotation_matrix.h"

#define MATRIX_LENGTH 4

void rotation_matrix_page_create(struct AppPage *page, GtkWidget *window);
void rotation_matrix_page_open_response(AppPage *page, GObject *original_object, GAsyncResult *res);
void rotation_matrix_page_free(struct AppPage *page);

AppPage rotation_matrix_page = {
    .page = NULL,
    .on_create = rotation_matrix_page_create,
    .on_file_dialog_result = rotation_matrix_page_open_response,
    .on_free = rotation_matrix_page_free
};

struct RotationMatrixData
{
    Crypto_RotationMatrix *rotation_matrix;

    StringMatrix *matrix;
    GtkWidget *text_edit;
    GtkWidget *summary_edit;
};

static int delete_all_non_english_chars(char* str)
{
    const int len = (int)strlen(str);
    int shift = 0, final_len = len;
    for (int i = 0; i < len; i++)
    {
        if (('A' > str[i] || 'Z' < str[i]) && ('a' > str[i] || 'z' < str[i]))
        {
            shift++;
            final_len--;
            continue;
        }

        if (!shift)
            continue;

        str[i - shift] = str[i];
    }

    str[final_len] = '\0';
    return final_len;
}

static void on_file_open_pressed(GtkWidget *widget, gpointer user_data)
{
    open_file_dialog(user_data);
}

static void on_encode_pressed(GtkWidget *widget, gpointer user_data)
{
    struct RotationMatrixData *data = ((AppPage *)user_data)->data;

    const char *input = gtk_editable_get_text(GTK_EDITABLE(data->text_edit));
    char *prop_input = strdup(input);
    const int input_len = (int)delete_all_non_english_chars(prop_input);
    char output[20];

    Crypto_RotationMatrix_Encode(data->rotation_matrix, input_len, prop_input, output);
    gtk_editable_set_text(GTK_EDITABLE(data->summary_edit), output);

    for (int i = 0; i < MATRIX_LENGTH; i++)
        for (int j = 0; j < MATRIX_LENGTH; j++)
        {
            char text[10];
            sprintf(text, "%c", data->rotation_matrix->process_matrix[i][j]);
            string_matrix_set_text(data->matrix, i, j, text);
        }
}

static void on_decode_pressed(GtkWidget *widget, gpointer user_data)
{
    struct RotationMatrixData *data = ((AppPage *)user_data)->data;

    const char *input = gtk_editable_get_text(GTK_EDITABLE(data->text_edit));
    char *prop_input = strdup(input);
    const int input_len = (int)delete_all_non_english_chars(prop_input);
    char output[20];

    Crypto_RotationMatrix_Decode(data->rotation_matrix, input_len, prop_input, output);
    gtk_editable_set_text(GTK_EDITABLE(data->summary_edit), output);

    for (int i = 0; i < MATRIX_LENGTH; i++)
        for (int j = 0; j < MATRIX_LENGTH; j++)
        {
            char text[10];
            sprintf(text, "%c", data->rotation_matrix->process_matrix[i][j]);
            string_matrix_set_text(data->matrix, i, j, text);
        }
}

static void on_clear_pressed(GtkWidget *widget, gpointer user_data)
{
    struct RotationMatrixData *data = ((AppPage *)user_data)->data;

    gtk_editable_set_text(GTK_EDITABLE(data->text_edit), "");
    gtk_editable_set_text(GTK_EDITABLE(data->summary_edit), "");

    for (int r = 0; r < data->rotation_matrix->matrix_length; r++)
        memset(data->rotation_matrix->process_matrix[r], ' ', data->rotation_matrix->matrix_length);
    for (int i = 0; i < MATRIX_LENGTH; i++)
        for (int j = 0; j < MATRIX_LENGTH; j++)
            string_matrix_set_text(data->matrix, i, j, "");
}

void rotation_matrix_page_create(struct AppPage *page, GtkWidget *window)
{
    struct RotationMatrixData *data = calloc(1, sizeof(struct RotationMatrixData));
    page->data = data;

    data->rotation_matrix = Crypto_RotationMatrix_Create(MATRIX_LENGTH);

    GtkWidget *container = gtk_grid_new();
    page->page = container;
    gtk_grid_set_column_spacing(GTK_GRID(container), 10);
    gtk_grid_set_row_spacing(GTK_GRID(container), 10);
    gtk_widget_set_margin(GTK_WIDGET(container), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(container), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(container), TRUE);

    // Top
    GtkWidget *label = gtk_label_new("Поворачивающаяся решётка");
    gtk_label_set_yalign(GTK_LABEL(label), 0.5f);
    {
        PangoAttrList *label_attrlist = pango_attr_list_new();
        PangoFontDescription *label_attrlist_font_desc = pango_font_description_new();
        pango_font_description_set_size(label_attrlist_font_desc, 20 * PANGO_SCALE);
        pango_font_description_set_weight(label_attrlist_font_desc, PANGO_WEIGHT_HEAVY);
        PangoAttribute *label_attrlist_attr = pango_attr_font_desc_new(label_attrlist_font_desc);
        pango_attr_list_insert(label_attrlist, label_attrlist_attr);
        gtk_label_set_attributes(GTK_LABEL(label), label_attrlist);
    }
    gtk_grid_attach(GTK_GRID(container), label, 0, 0, 24, 1);

    GtkWidget *text_edit_label = gtk_label_new("Исходный текст:");
    gtk_label_set_xalign(GTK_LABEL(text_edit_label), 0);
    {
        PangoAttrList *text_edit_label_attrlist = pango_attr_list_new();
        PangoFontDescription *text_edit_label_attrlist_font_desc = pango_font_description_new();
        pango_font_description_set_size(text_edit_label_attrlist_font_desc, 14 * PANGO_SCALE);
        pango_font_description_set_weight(text_edit_label_attrlist_font_desc, PANGO_WEIGHT_BOLD);
        PangoAttribute *text_edit_label_attrlist_attr = pango_attr_font_desc_new(text_edit_label_attrlist_font_desc);
        pango_attr_list_insert(text_edit_label_attrlist, text_edit_label_attrlist_attr);
        gtk_label_set_attributes(GTK_LABEL(text_edit_label), text_edit_label_attrlist);
    }
    gtk_grid_attach(GTK_GRID(container), text_edit_label, 0, 1, 24, 1);
    data->text_edit = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(data->text_edit), 16);
    gtk_grid_attach(GTK_GRID(container), data->text_edit, 0, 2, 22, 1);
    GtkWidget *text_selection_btn = gtk_button_new_with_label("Файл...");
    gtk_grid_attach(GTK_GRID(container), text_selection_btn, 22, 2, 2, 1);
    g_signal_connect(G_OBJECT(text_selection_btn), "clicked", G_CALLBACK(on_file_open_pressed), page);

    GtkWidget *summary_edit_label = gtk_label_new("Итоговый текст:");
    gtk_label_set_xalign(GTK_LABEL(summary_edit_label), 0);
    {
        PangoAttrList *summary_edit_label_attrlist = pango_attr_list_new();
        PangoFontDescription *summary_edit_label_attrlist_font_desc = pango_font_description_new();
        pango_font_description_set_size(summary_edit_label_attrlist_font_desc, 14 * PANGO_SCALE);
        pango_font_description_set_weight(summary_edit_label_attrlist_font_desc, PANGO_WEIGHT_BOLD);
        PangoAttribute *summary_edit_label_attrlist_attr = pango_attr_font_desc_new(summary_edit_label_attrlist_font_desc);
        pango_attr_list_insert(summary_edit_label_attrlist, summary_edit_label_attrlist_attr);
        gtk_label_set_attributes(GTK_LABEL(summary_edit_label), summary_edit_label_attrlist);
    }
    gtk_grid_attach(GTK_GRID(container), summary_edit_label, 0, 3, 24, 1);
    data->summary_edit = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(data->summary_edit), FALSE);
    gtk_grid_attach(GTK_GRID(container), data->summary_edit, 0, 4, 24, 1);

    // Center
    GtkWidget *string_matrix_label = gtk_label_new("Матрица, полученная в ходе операции:");
    gtk_label_set_xalign(GTK_LABEL(string_matrix_label), 0);
    {
        PangoAttrList *string_matrix_label_attrlist = pango_attr_list_new();
        PangoFontDescription *string_matrix_label_attrlist_font_desc = pango_font_description_new();
        pango_font_description_set_size(string_matrix_label_attrlist_font_desc, 14 * PANGO_SCALE);
        pango_font_description_set_weight(string_matrix_label_attrlist_font_desc, PANGO_WEIGHT_BOLD);
        PangoAttribute *string_matrix_label_attrlist_attr = pango_attr_font_desc_new(string_matrix_label_attrlist_font_desc);
        pango_attr_list_insert(string_matrix_label_attrlist, string_matrix_label_attrlist_attr);
        gtk_label_set_attributes(GTK_LABEL(string_matrix_label), string_matrix_label_attrlist);
    }
    gtk_grid_attach(GTK_GRID(container), string_matrix_label, 0, 5, 24, 1);
    data->matrix = string_matrix_new(MATRIX_LENGTH, MATRIX_LENGTH);
    gtk_grid_attach(GTK_GRID(container), GTK_WIDGET(string_matrix_get_widget(data->matrix)), 0, 6, 24, 5);

    // Bottom
    GtkWidget *encode_btn = gtk_button_new_with_label("Шифровать");
    gtk_grid_attach(GTK_GRID(container), encode_btn, 0, 11, 10, 1);
    g_signal_connect(G_OBJECT(encode_btn), "clicked", G_CALLBACK(on_encode_pressed), page);

    GtkWidget *decode_btn = gtk_button_new_with_label("Дешифровать");
    gtk_grid_attach(GTK_GRID(container), decode_btn, 10, 11, 10, 1);
    g_signal_connect(decode_btn, "clicked", G_CALLBACK(on_decode_pressed), page);

    GtkWidget *clear_btn = gtk_button_new_with_label("Очистить");
    gtk_grid_attach(GTK_GRID(container), clear_btn, 20, 11, 4, 1);
    g_signal_connect(clear_btn, "clicked", G_CALLBACK(on_clear_pressed), page);
}

void rotation_matrix_page_open_response(AppPage *page, GObject *original_object, GAsyncResult *res)
{
    struct RotationMatrixData *data = page->data;
    GtkFileDialog *dialog = GTK_FILE_DIALOG(original_object);
    GError *error = NULL;
    GFile *file = gtk_file_dialog_open_finish(dialog, res, &error);

    if (file != NULL) {
        char *path = g_file_get_path(file);

        FILE *f = fopen(path, "r");
        char text[18];
        memset(text, 0, sizeof(text));
        fgets(text, 17, f);
        gtk_editable_set_text(GTK_EDITABLE(data->text_edit), text);

        fclose(f);
        g_free(path);
        g_object_unref(file);
    } else {
        g_clear_error(&error);
    }
}

void rotation_matrix_page_free(struct AppPage *page)
{

}
