//
// Created by REXE on 13.02.26.
//

#include "pages.h"
#include "../crypto/rotation_matrix.h"

#define MATRIX_STEPS_COUNT 4

void rotation_matrix_page_create(struct AppPage *page, GtkWidget *window);
void rotation_matrix_page_open_response(AppPage *page, GObject *original_object, GAsyncResult *res);
void rotation_matrix_page_save_response(AppPage *page, GObject *original_object, GAsyncResult *res);
void rotation_matrix_page_free(struct AppPage *page);

AppPage rotation_matrix_page = {
    .on_create = rotation_matrix_page_create,
    .on_file_dialog_open_result = rotation_matrix_page_open_response,
    .on_file_dialog_save_result = rotation_matrix_page_save_response,
    .on_free = rotation_matrix_page_free
};

struct RotationMatrixData
{
    Crypto_RotationMatrix *rotation_matrix;

    StringMatrix *matrix;
    GtkWidget *text_edit;
    GtkWidget *summary_edit;
    GtkWidget *summary_save_btn;

    GtkWidget *string_matrix_prev_step_btn;
    GtkWidget *string_matrix_next_step_btn;
    GtkWidget *string_matrix_step_label;

    unsigned int curr_matrix_step;
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

static void update_steps_page(struct RotationMatrixData *data)
{
    gtk_widget_set_sensitive(data->string_matrix_prev_step_btn, data->curr_matrix_step != 0);
    gtk_widget_set_sensitive(data->string_matrix_next_step_btn, data->curr_matrix_step < MATRIX_STEPS_COUNT);

    string_matrix_resize_rows(data->matrix, data->rotation_matrix->matrix_length);
    string_matrix_resize_columns(data->matrix, data->rotation_matrix->matrix_length);

    char *text = strdup(RTMP_STR_MATRIX_STEP_TITLE);
    sprintf(text, RTMP_STR_MATRIX_STEP_TITLE, data->curr_matrix_step + 1, MATRIX_STEPS_COUNT);
    gtk_label_set_text(GTK_LABEL(data->string_matrix_step_label), text);
    free(text);

    for (int i = 0; i < data->rotation_matrix->matrix_length; i++)
        for (int j = 0; j < data->rotation_matrix->matrix_length; j++)
        {
            char text[10];
            sprintf(text, "%c", data->rotation_matrix->steps[data->curr_matrix_step][i][j].c);
            string_matrix_set_cell(
                data->matrix,
                i, j,
                text,
                data->rotation_matrix->steps[data->curr_matrix_step][i][j].is_new ? STRING_MATRIX_CELL_BOLD : STRING_MATRIX_CELL_NORMAL
                );
        }
}

static void process_input_values(struct RotationMatrixData *data, int decode)
{
    const char *raw_input = gtk_editable_get_text(GTK_EDITABLE(data->text_edit));
    const int raw_input_len = (int)strlen(raw_input);
    char *prop_input = strdup(raw_input);
    const int prop_input_len = (int)delete_all_non_english_chars(prop_input);
    char output[257];

    if (0 < prop_input_len && prop_input_len != raw_input_len)
    {
        EntryDeco_MarkEntryRowAsWarning(
            ADW_ENTRY_ROW(data->text_edit),
            RTMP_STR_NON_VALID_CHARS_IN_SOURCE_TEXT_WARNING
            );
    }

    if (0 != prop_input_len)
    {
        if (!decode)
            Crypto_RotationMatrix_Encode(data->rotation_matrix, prop_input_len, prop_input, output);
        else
            Crypto_RotationMatrix_Decode(data->rotation_matrix, prop_input_len, prop_input, output);
        gtk_editable_set_text(GTK_EDITABLE(data->summary_edit), output);

        update_steps_page(data);
    } else
    {
        EntryDeco_MarkEntryRowAsError(
            ADW_ENTRY_ROW(data->text_edit),
            (0 == raw_input_len) ? RTMP_STR_EMPTY_SOURCE_TEXT_ERROR : RTMP_STR_NO_VALID_CHARS_IN_SOURCE_TEXT_ERROR
            );
    }

    free(prop_input);
}

static void on_file_open_pressed(GtkWidget *widget, gpointer user_data)
{
    open_file_open_dialog(user_data);
}

static void on_file_save_pressed(GtkWidget *widget, gpointer user_data)
{
    open_file_save_dialog(user_data);
}

static void on_encode_pressed(GtkWidget *widget, gpointer user_data)
{
    struct RotationMatrixData *data = ((AppPage *)user_data)->data;
    process_input_values(data, 0);
}

static void on_decode_pressed(GtkWidget *widget, gpointer user_data)
{
    struct RotationMatrixData *data = ((AppPage *)user_data)->data;
    process_input_values(data, 1);
}

static void on_entry_changed(GtkWidget *widget, gpointer user_data)
{
    EntryDeco_ClearRowDecorations(ADW_ENTRY_ROW(widget));
}

static void on_summary_changed(GtkWidget *widget, gpointer user_data)
{
    struct RotationMatrixData *data = ((AppPage *)user_data)->data;

    const unsigned int summary_len = adw_entry_row_get_text_length(ADW_ENTRY_ROW(data->summary_edit));
    gtk_widget_set_sensitive(GTK_WIDGET(data->summary_save_btn), summary_len != 0);
}

static void on_clear_pressed(GtkWidget *widget, gpointer user_data)
{
    struct RotationMatrixData *data = ((AppPage *)user_data)->data;

    gtk_editable_set_text(GTK_EDITABLE(data->text_edit), "");
    gtk_editable_set_text(GTK_EDITABLE(data->summary_edit), "");

    for (int r = 0; r < data->rotation_matrix->matrix_length; r++)
    {
        memset(data->rotation_matrix->process_matrix[r], ' ', data->rotation_matrix->matrix_length);
        for (int step = 0; step < MATRIX_STEPS_COUNT; step++)
            memset(data->rotation_matrix->steps[step][r], ' ', data->rotation_matrix->matrix_length);
    }
    for (int i = 0; i < data->rotation_matrix->matrix_length; i++)
        for (int j = 0; j < data->rotation_matrix->matrix_length; j++)
            string_matrix_set_cell(data->matrix, i, j, "", STRING_MATRIX_CELL_NORMAL);
}

static void on_next_step_pressed(GtkWidget *widget, gpointer user_data)
{
    struct RotationMatrixData *data = ((AppPage *)user_data)->data;

    if ((MATRIX_STEPS_COUNT - 1) <= data->curr_matrix_step)
        return;

    data->curr_matrix_step++;
    update_steps_page(data);
}

static void on_prev_step_pressed(GtkWidget *widget, gpointer user_data)
{
    struct RotationMatrixData *data = ((AppPage *)user_data)->data;

    if (0 == data->curr_matrix_step)
        return;

    data->curr_matrix_step--;
    update_steps_page(data);
}

void rotation_matrix_page_create(struct AppPage *page, GtkWidget *window)
{
    struct RotationMatrixData *data = calloc(1, sizeof(struct RotationMatrixData));
    page->data = data;

    data->rotation_matrix = Crypto_RotationMatrix_Create(4);

    GtkWidget *container = gtk_grid_new();
    page->page = container;
    gtk_grid_set_column_spacing(GTK_GRID(container), 10);
    gtk_grid_set_row_spacing(GTK_GRID(container), 10);
    gtk_widget_set_margin(GTK_WIDGET(container), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(container), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(container), TRUE);

    // Top
    GtkWidget *label = gtk_label_new(RTMP_STR_TITLE);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5f);
    {
        PangoAttrList *label_attrlist = pango_attr_list_new();
        PangoFontDescription *label_attrlist_font_desc = pango_font_description_new();
        pango_font_description_set_size(label_attrlist_font_desc, 24 * PANGO_SCALE);
        pango_font_description_set_weight(label_attrlist_font_desc, PANGO_WEIGHT_HEAVY);
        PangoAttribute *label_attrlist_attr = pango_attr_font_desc_new(label_attrlist_font_desc);
        pango_attr_list_insert(label_attrlist, label_attrlist_attr);
        gtk_label_set_attributes(GTK_LABEL(label), label_attrlist);
    }
    gtk_grid_attach(GTK_GRID(container), label, 0, 0, 24, 1);

    data->text_edit = adw_entry_row_new();
    gtk_widget_add_css_class(data->text_edit, "card");
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->text_edit), RTMP_STR_ENTRY_SOURCE_TEXT_TITLE);
    GtkWidget *text_edit_icon = gtk_image_new_from_icon_name("font-x-generic-symbolic");
    adw_entry_row_add_prefix(ADW_ENTRY_ROW(data->text_edit), text_edit_icon);
    gtk_grid_attach(GTK_GRID(container), data->text_edit, 0, 1, 20, 1);
    g_signal_connect(G_OBJECT(data->text_edit), "changed", G_CALLBACK(on_entry_changed), page);

    GtkWidget *text_selection_btn = gtk_button_new_with_icon_and_label("folder-open-symbolic", APP_STR_OPEN_FILE_TITLE, 8);
    gtk_grid_attach(GTK_GRID(container), text_selection_btn, 20, 1, 4, 1);
    g_signal_connect(G_OBJECT(text_selection_btn), "clicked", G_CALLBACK(on_file_open_pressed), page);

    data->summary_edit = adw_entry_row_new();
    gtk_widget_add_css_class(data->summary_edit, "card");
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->summary_edit), RTMP_STR_ENTRY_SUMMARY_TEXT_TITLE);
    GtkWidget *summary_edit_icon = gtk_image_new_from_icon_name("media-playlist-shuffle");
    adw_entry_row_add_prefix(ADW_ENTRY_ROW(data->summary_edit), summary_edit_icon);
    gtk_editable_set_editable(GTK_EDITABLE(data->summary_edit), FALSE);
    gtk_grid_attach(GTK_GRID(container), data->summary_edit, 0, 2, 20, 1);
    g_signal_connect(G_OBJECT(data->summary_edit), "changed", G_CALLBACK(on_summary_changed), page);

    data->summary_save_btn = gtk_button_new_with_icon_and_label("media-floppy", APP_STR_SAVE_FILE_TITLE, 8);
    gtk_widget_set_sensitive(GTK_WIDGET(data->summary_save_btn), FALSE);
    gtk_grid_attach(GTK_GRID(container), data->summary_save_btn, 20, 2, 4, 1);
    g_signal_connect(G_OBJECT(data->summary_save_btn), "clicked", G_CALLBACK(on_file_save_pressed), page);

    // Center
    GtkWidget *card = adw_bin_new();
    gtk_widget_add_css_class(card, "card");

    GtkWidget *card_content = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(card_content), 10);
    gtk_grid_set_row_spacing(GTK_GRID(card_content), 10);
    gtk_widget_set_margin(GTK_WIDGET(card_content), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(card_content), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(card_content), TRUE);

    GtkWidget *string_matrix_label = gtk_label_new(RTMP_STR_MATRIX_TITLE);
    gtk_label_set_xalign(GTK_LABEL(string_matrix_label), 0.5f);
    {
        PangoAttrList *string_matrix_label_attrlist = pango_attr_list_new();
        PangoFontDescription *string_matrix_label_attrlist_font_desc = pango_font_description_new();
        pango_font_description_set_size(string_matrix_label_attrlist_font_desc, 18 * PANGO_SCALE);
        pango_font_description_set_weight(string_matrix_label_attrlist_font_desc, PANGO_WEIGHT_BOLD);
        PangoAttribute *string_matrix_label_attrlist_attr = pango_attr_font_desc_new(string_matrix_label_attrlist_font_desc);
        pango_attr_list_insert(string_matrix_label_attrlist, string_matrix_label_attrlist_attr);
        gtk_label_set_attributes(GTK_LABEL(string_matrix_label), string_matrix_label_attrlist);
    }
    gtk_grid_attach(GTK_GRID(card_content), string_matrix_label, 0, 0, 24, 1);
    data->matrix = string_matrix_new(4, 4);
    gtk_grid_attach(GTK_GRID(card_content), GTK_WIDGET(string_matrix_get_widget(data->matrix)), 0, 1, 24, 3);

    data->string_matrix_prev_step_btn = gtk_button_new_from_icon_name("pan-start");
    gtk_grid_attach(GTK_GRID(card_content), GTK_WIDGET(data->string_matrix_prev_step_btn), 0, 4, 2, 1);
    g_signal_connect(data->string_matrix_prev_step_btn, "clicked", G_CALLBACK(on_prev_step_pressed), page);
    data->string_matrix_step_label = gtk_label_new(RTMP_STR_MATRIX_STEP_TITLE);
    {
        PangoAttrList *string_matrix_step_label_attrlist = pango_attr_list_new();
        PangoFontDescription *string_matrix_step_label_attrlist_font_desc = pango_font_description_new();
        pango_font_description_set_size(string_matrix_step_label_attrlist_font_desc, 14 * PANGO_SCALE);
        pango_font_description_set_weight(string_matrix_step_label_attrlist_font_desc, PANGO_WEIGHT_BOLD);
        PangoAttribute *string_matrix_label_attrlist_attr = pango_attr_font_desc_new(string_matrix_step_label_attrlist_font_desc);
        pango_attr_list_insert(string_matrix_step_label_attrlist, string_matrix_label_attrlist_attr);
        gtk_label_set_attributes(GTK_LABEL(data->string_matrix_step_label), string_matrix_step_label_attrlist);
    }
    gtk_grid_attach(GTK_GRID(card_content), GTK_WIDGET(data->string_matrix_step_label), 2, 4, 20, 1);
    data->string_matrix_next_step_btn = gtk_button_new_from_icon_name("pan-end");
    gtk_grid_attach(GTK_GRID(card_content), GTK_WIDGET(data->string_matrix_next_step_btn), 22, 4, 2, 1);
    g_signal_connect(data->string_matrix_next_step_btn, "clicked", G_CALLBACK(on_next_step_pressed), page);
    update_steps_page(data);

    adw_bin_set_child(ADW_BIN(card), GTK_WIDGET(card_content));
    gtk_grid_attach(GTK_GRID(container), GTK_WIDGET(card), 0, 4, 24, 4);

    // Bottom
    GtkWidget *encode_btn = gtk_button_new_with_icon_and_label("changes-prevent", APP_STR_ENCRYPT_TITLE, 8);
    gtk_grid_attach(GTK_GRID(container), encode_btn, 0, 8, 10, 1);
    g_signal_connect(G_OBJECT(encode_btn), "clicked", G_CALLBACK(on_encode_pressed), page);

    GtkWidget *decode_btn = gtk_button_new_with_icon_and_label("changes-allow", APP_STR_DECRYPT_TITLE, 8);
    gtk_grid_attach(GTK_GRID(container), decode_btn, 10, 8, 10, 1);
    g_signal_connect(decode_btn, "clicked", G_CALLBACK(on_decode_pressed), page);

    GtkWidget *clear_btn = gtk_button_new_with_icon_and_label("edit-clear", APP_STR_CLEAR_TITLE, 8);
    gtk_grid_attach(GTK_GRID(container), clear_btn, 20, 8, 4, 1);
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
        char text[257];
        memset(text, 0, sizeof(text));
        fgets(text, 256, f);
        gtk_editable_set_text(GTK_EDITABLE(data->text_edit), text);

        fclose(f);
        g_free(path);
        g_object_unref(file);
    } else {
        g_clear_error(&error);
    }
}

void rotation_matrix_page_save_response(AppPage *page, GObject *original_object, GAsyncResult *res)
{
    struct RotationMatrixData *data = page->data;
    GtkFileDialog *dialog = GTK_FILE_DIALOG(original_object);
    GError *error = NULL;
    GFile *file = gtk_file_dialog_save_finish(dialog, res, &error);

    if (file != NULL) {
        char *path = g_file_get_path(file);

        FILE *f = fopen(path, "w+");
        const char *text = gtk_editable_get_text(GTK_EDITABLE(data->summary_edit));
        fputs(text, f);
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
