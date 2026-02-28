//
// Created by REXE on 13.02.26.
//

#include "pages.h"
#include "../crypto/vigenere_algorithm.h"

void vigenere_algorithm_page_create(struct AppPage *page, GtkWidget *window);
void vigenere_algorithm_page_open_response(AppPage *page, GObject *original_object, GAsyncResult *res);
void vigenere_algorithm_page_save_response(AppPage *page, GObject *original_object, GAsyncResult *res);
void vigenere_algorithm_page_free(struct AppPage *page);

AppPage vigenere_algorithm_page = {
    .on_create = vigenere_algorithm_page_create,
    .on_file_dialog_open_result = vigenere_algorithm_page_open_response,
    .on_file_dialog_save_result = vigenere_algorithm_page_save_response,
    .on_free = vigenere_algorithm_page_free
};

struct VigenereAlgorithmData
{
    Crypto_VigenereAlgorithm *vigenere_algorithm;

    GtkWidget *text_edit;
    GtkWidget *key_edit;
    GtkWidget *summary_edit;
    GtkWidget *used_key_edit;
    GtkWidget *summary_save_btn;

    StringMatrix *crypt_matrix;
};

static int delete_all_non_russians_chars(wchar_t* str)
{
    const int len = (int)wcslen(str);
    int shift = 0, final_len = len;
    for (int i = 0; i < len; i++)
    {
        if ((L'А' > str[i] || L'Я' < str[i]) && (L'а' > str[i] || L'я' < str[i]) && (L'ё' != str[i]) && (L'Ё' != str[i]))
        {
            shift++;
            final_len--;
            continue;
        }

        if (L'а' <= str[i] && L'я' >= str[i])
            str[i] += L'A' - L'a';

        if (L'ё' == str[i])
            str[i] = L'Ё';

        if (!shift)
            continue;

        str[i - shift] = str[i];
    }

    str[final_len] = L'\0';
    return final_len;
}

static void update_crypt_matrix(
    struct VigenereAlgorithmData *data,
    const wchar_t *input, const unsigned int input_len,
    const wchar_t *key,
    const wchar_t *output
    )
{
    string_matrix_resize_columns(data->crypt_matrix, input_len);
    for (int i = 0; i < input_len; i++)
    {
        char *text = wchar_to_utf8(&input[i], 1, NULL);
        string_matrix_set_cell(data->crypt_matrix, 0, i, text, STRING_MATRIX_CELL_NORMAL);
        g_free(text);
    }

    for (int i = 0; i < input_len; i++)
    {
        char *text = wchar_to_utf8(&key[i], 1, NULL);
        string_matrix_set_cell(data->crypt_matrix, 1, i, text, STRING_MATRIX_CELL_NORMAL);
        g_free(text);
    }

    for (int i = 0; i < input_len; i++)
    {
        char *text = wchar_to_utf8(&output[i], 1, NULL);
        string_matrix_set_cell(data->crypt_matrix, 2, i, text, STRING_MATRIX_CELL_BOLD);
        g_free(text);
    }
}

static void process_input_values(struct VigenereAlgorithmData *data, int decode)
{
    wchar_t output[255];

    const char *raw_input = gtk_editable_get_text(GTK_EDITABLE(data->text_edit));
    long raw_input_len;
    wchar_t *input = (wchar_t*)utf8_to_wchar(raw_input, -1, &raw_input_len);
    const int input_len = delete_all_non_russians_chars(input);
    if (0 < input_len && input_len != raw_input_len)
    {
        EntryDeco_MarkEntryRowAsWarning(
            ADW_ENTRY_ROW(data->text_edit),
            VALP_STR_NON_VALID_CHARS_IN_SOURCE_TEXT_WARNING
            );
    }

    const char *raw_key = gtk_editable_get_text(GTK_EDITABLE(data->key_edit));
    long raw_key_len;
    wchar_t *key = (wchar_t*)utf8_to_wchar(raw_key, -1, &raw_key_len);
    const int key_len = delete_all_non_russians_chars(key);
    if (0 < key_len && key_len != raw_key_len)
    {
        EntryDeco_MarkEntryRowAsWarning(
            ADW_ENTRY_ROW(data->key_edit),
            VALP_STR_NON_VALID_CHARS_IN_KEY_WARNING
            );
    }

    if (0 < input_len && 0 < key_len)
    {
        if (!decode)
            Crypto_VigenereAlgorithm_Encode(
                data->vigenere_algorithm,
                (int)input_len, input,
                (int)key_len, key,
                output
                );
        else
            Crypto_VigenereAlgorithm_Decode(
                data->vigenere_algorithm,
                (int)input_len, input,
                (int)key_len, key,
                output
                );

        char *output_c = wchar_to_utf8(output, -1, NULL);
        gtk_editable_set_text(GTK_EDITABLE(data->summary_edit), output_c);

        char *key_out_c = wchar_to_utf8(data->vigenere_algorithm->process_key, -1, NULL);
        gtk_editable_set_text(GTK_EDITABLE(data->used_key_edit), key_out_c);

        update_crypt_matrix(data, input, input_len, data->vigenere_algorithm->process_key, output);

        g_free(output_c);
        g_free(key_out_c);
    } else
    {
        if (0 == raw_input_len)
        {
            EntryDeco_MarkEntryRowAsError(
                ADW_ENTRY_ROW(data->text_edit),
                VALP_STR_EMPTY_ENTRY_ERROR
                );
        } else if (0 == input_len)
        {
            EntryDeco_MarkEntryRowAsError(
                ADW_ENTRY_ROW(data->text_edit),
                VALP_STR_NO_VALID_CHARS_IN_SOURCE_TEXT_ERROR
                );
        }

        if (0 == raw_key_len)
        {
            EntryDeco_MarkEntryRowAsError(
                ADW_ENTRY_ROW(data->key_edit),
                VALP_STR_EMPTY_ENTRY_ERROR
                );
        } else if (0 == key_len)
        {
            EntryDeco_MarkEntryRowAsError(
                ADW_ENTRY_ROW(data->key_edit),
                VALP_STR_NO_VALID_CHARS_IN_KEY_ERROR
                );
        }
    }

    free(input);
    free(key);
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
    struct VigenereAlgorithmData *data = ((AppPage *) user_data)->data;
    process_input_values(data, 0);
}

static void on_decode_pressed(GtkWidget *widget, gpointer user_data)
{
    struct VigenereAlgorithmData *data = ((AppPage *) user_data)->data;
    process_input_values(data, 1);
}

static void on_entry_changed(GtkWidget *widget, gpointer user_data)
{
    EntryDeco_ClearRowDecorations(ADW_ENTRY_ROW(widget));
}

static void on_summary_changed(GtkWidget *widget, gpointer user_data)
{
    struct VigenereAlgorithmData *data = ((AppPage *)user_data)->data;

    const unsigned int summary_len = adw_entry_row_get_text_length(ADW_ENTRY_ROW(data->summary_edit));
    gtk_widget_set_sensitive(GTK_WIDGET(data->summary_save_btn), summary_len != 0);
}

static void on_clear_pressed(GtkWidget *widget, gpointer user_data)
{
    struct VigenereAlgorithmData *data = ((AppPage *) user_data)->data;

    gtk_editable_set_text(GTK_EDITABLE(data->text_edit), "");
    gtk_editable_set_text(GTK_EDITABLE(data->summary_edit), "");
    gtk_editable_set_text(GTK_EDITABLE(data->used_key_edit), "");
}

void vigenere_algorithm_page_create(struct AppPage *page, GtkWidget *window)
{
    struct VigenereAlgorithmData *data = calloc(1, sizeof(struct VigenereAlgorithmData));
    page->data = data;

    data->vigenere_algorithm = Crypto_VigenereAlgorithm_Create();

    GtkWidget *container = gtk_grid_new();
    page->page = container;
    gtk_grid_set_column_spacing(GTK_GRID(container), 10);
    gtk_grid_set_row_spacing(GTK_GRID(container), 10);
    gtk_widget_set_margin(GTK_WIDGET(container), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(container), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(container), TRUE);

    // Top
    GtkWidget *label = gtk_label_new(VALP_STR_TITLE);
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

    data->key_edit = adw_entry_row_new();
    gtk_widget_add_css_class(data->key_edit, "card");
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->key_edit), VALP_STR_ENTRY_KEY_TITLE);
    GtkWidget *key_edit_icon = gtk_image_new_from_icon_name("dialog-password");
    adw_entry_row_add_prefix(ADW_ENTRY_ROW(data->key_edit), key_edit_icon);
    gtk_grid_attach(GTK_GRID(container), data->key_edit, 0, 1, 12, 1);
    g_signal_connect(G_OBJECT(data->key_edit), "changed", G_CALLBACK(on_entry_changed), data);

    data->used_key_edit = adw_entry_row_new();
    gtk_widget_add_css_class(data->used_key_edit, "card");
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->used_key_edit), VALP_STR_ENTRY_PROGRESSIVE_KEY_TITLE);
    GtkWidget *used_key_edit_icon = gtk_image_new_from_icon_name("input-dialpad");
    adw_entry_row_add_prefix(ADW_ENTRY_ROW(data->used_key_edit), used_key_edit_icon);
    gtk_editable_set_editable(GTK_EDITABLE(data->used_key_edit), FALSE);
    gtk_grid_attach(GTK_GRID(container), data->used_key_edit, 12, 1, 12, 1);

    data->text_edit = adw_entry_row_new();
    gtk_widget_add_css_class(data->text_edit, "card");
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->text_edit), VALP_STR_ENTRY_SOURCE_TEXT_TITLE);
    GtkWidget *text_edit_icon = gtk_image_new_from_icon_name("font-x-generic-symbolic");
    adw_entry_row_add_prefix(ADW_ENTRY_ROW(data->text_edit), text_edit_icon);
    gtk_grid_attach(GTK_GRID(container), data->text_edit, 0, 2, 20, 1);
    g_signal_connect(G_OBJECT(data->text_edit), "changed", G_CALLBACK(on_entry_changed), data);

    GtkWidget *text_selection_btn = gtk_button_new_with_icon_and_label("folder-open-symbolic", APP_STR_OPEN_FILE_TITLE, 8);
    gtk_grid_attach(GTK_GRID(container), text_selection_btn, 20, 2, 4, 1);
    g_signal_connect(G_OBJECT(text_selection_btn), "clicked", G_CALLBACK(on_file_open_pressed), page);

    data->summary_edit = adw_entry_row_new();
    gtk_widget_add_css_class(data->summary_edit, "card");
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->summary_edit), VALP_STR_ENTRY_SUMMARY_TEXT_TITLE);
    GtkWidget *summary_edit_icon = gtk_image_new_from_icon_name("media-playlist-shuffle");
    adw_entry_row_add_prefix(ADW_ENTRY_ROW(data->summary_edit), summary_edit_icon);
    gtk_editable_set_editable(GTK_EDITABLE(data->summary_edit), FALSE);
    gtk_grid_attach(GTK_GRID(container), data->summary_edit, 0, 3, 20, 1);
    g_signal_connect(G_OBJECT(data->summary_edit), "changed", G_CALLBACK(on_summary_changed), page);

    data->summary_save_btn = gtk_button_new_with_icon_and_label("media-floppy", APP_STR_SAVE_FILE_TITLE, 8);
    gtk_widget_set_sensitive(GTK_WIDGET(data->summary_save_btn), FALSE);
    gtk_grid_attach(GTK_GRID(container), data->summary_save_btn, 20, 3, 4, 1);
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

    GtkWidget *crypt_matrix_label = gtk_label_new(VALP_STR_MATRIX_TITLE);
    gtk_label_set_xalign(GTK_LABEL(crypt_matrix_label), 0.5f);
    {
        PangoAttrList *crypt_matrix_label_attrlist = pango_attr_list_new();
        PangoFontDescription *crypt_matrix_label_attrlist_font_desc = pango_font_description_new();
        pango_font_description_set_size(crypt_matrix_label_attrlist_font_desc, 18 * PANGO_SCALE);
        pango_font_description_set_weight(crypt_matrix_label_attrlist_font_desc, PANGO_WEIGHT_BOLD);
        PangoAttribute *crypt_matrix_label_attrlist_attr = pango_attr_font_desc_new(crypt_matrix_label_attrlist_font_desc);
        pango_attr_list_insert(crypt_matrix_label_attrlist, crypt_matrix_label_attrlist_attr);
        gtk_label_set_attributes(GTK_LABEL(crypt_matrix_label), crypt_matrix_label_attrlist);
    }
    gtk_grid_attach(GTK_GRID(card_content), crypt_matrix_label, 0, 0, 24, 1);

    data->crypt_matrix = string_matrix_new(15, 3);
    gtk_grid_attach(GTK_GRID(card_content), string_matrix_get_widget(data->crypt_matrix), 0, 1, 24, 2);

    adw_bin_set_child(ADW_BIN(card), GTK_WIDGET(card_content));
    gtk_grid_attach(GTK_GRID(container), GTK_WIDGET(card), 0, 4, 24, 3);

    // Bottom
    GtkWidget *encode_btn = gtk_button_new_with_icon_and_label("changes-prevent", APP_STR_ENCRYPT_TITLE, 8);
    gtk_grid_attach(GTK_GRID(container), encode_btn, 0, 7, 10, 1);
    g_signal_connect(G_OBJECT(encode_btn), "clicked", G_CALLBACK(on_encode_pressed), page);

    GtkWidget *decode_btn = gtk_button_new_with_icon_and_label("changes-allow", APP_STR_DECRYPT_TITLE, 8);
    gtk_grid_attach(GTK_GRID(container), decode_btn, 10, 7, 10, 1);
    g_signal_connect(decode_btn, "clicked", G_CALLBACK(on_decode_pressed), page);

    GtkWidget *clear_btn = gtk_button_new_with_icon_and_label("edit-clear", APP_STR_CLEAR_TITLE, 8);
    gtk_grid_attach(GTK_GRID(container), clear_btn, 20, 7, 4, 1);
    g_signal_connect(clear_btn, "clicked", G_CALLBACK(on_clear_pressed), page);
}

void vigenere_algorithm_page_open_response(AppPage *page, GObject *original_object, GAsyncResult *res)
{
    struct VigenereAlgorithmData *data = page->data;

    GtkFileDialog *dialog = GTK_FILE_DIALOG(original_object);
    GError *error = NULL;
    GFile *file = gtk_file_dialog_open_finish(dialog, res, &error);

    if (file != NULL) {
        char *path = g_file_get_path(file);

        FILE *f = fopen(path, "r");
        char text[256];
        memset(text, 0, sizeof(text));
        fgets(text, 255, f);
        gtk_editable_set_text(GTK_EDITABLE(data->text_edit), text);

        fclose(f);
        g_free(path);
        g_object_unref(file);
    } else {
        g_clear_error(&error);
    }
}

void vigenere_algorithm_page_save_response(AppPage *page, GObject *original_object, GAsyncResult *res)
{
    struct VigenereAlgorithmData *data = page->data;
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

void vigenere_algorithm_page_free(struct AppPage *page)
{

}
