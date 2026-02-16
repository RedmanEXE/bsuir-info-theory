//
// Created by REXE on 13.02.26.
//

#include "pages.h"
#include "../crypto/vigenere_algorithm.h"

void vigenere_algorithm_page_create(struct AppPage *page, GtkWidget *window);
void vigenere_algorithm_page_open_response(AppPage *page, GObject *original_object, GAsyncResult *res);
void vigenere_algorithm_page_free(struct AppPage *page);

AppPage vigenere_algorithm_page = {
    .on_create = vigenere_algorithm_page_create,
    .on_file_dialog_result = vigenere_algorithm_page_open_response,
    .on_free = vigenere_algorithm_page_free
};

struct VigenereAlgorithmData
{
    Crypto_VigenereAlgorithm *vigenere_algorithm;

    GtkWidget *text_edit;
    GtkWidget *key_edit;
    GtkWidget *summary_edit;
    GtkWidget *used_key_edit;
};

static int delete_all_non_russians_chars(wchar_t* str)
{
    const int len = (int)wcslen(str);
    int shift = 0, final_len = len;
    for (int i = 0; i < len; i++)
    {
        if ((L'А' > str[i] || L'Я' < str[i]) && (L'а' > str[i] || L'я' < str[i]))
        {
            shift++;
            final_len--;
            continue;
        }

        if (!shift)
            continue;

        str[i - shift] = str[i];
    }

    str[final_len] = L'\0';
    return final_len;
}

static void on_file_open_pressed(GtkWidget *widget, gpointer user_data)
{
    open_file_dialog(user_data);
}

static void on_encode_pressed(GtkWidget *widget, gpointer user_data)
{
    struct VigenereAlgorithmData *data = ((AppPage *) user_data)->data;
    GError *error = NULL;
    wchar_t output[255];

    const char *raw_input = gtk_editable_get_text(GTK_EDITABLE(data->text_edit));
    wchar_t *input = (wchar_t*)g_utf8_to_ucs4(raw_input, 256, NULL, NULL, &error);
    const int raw_input_len = (int)wcslen(input);
    const int input_len = delete_all_non_russians_chars(input);
    if (0 < input_len && input_len != raw_input_len)
    {
        EntryDeco_MarkEntryRowAsWarning(
            ADW_ENTRY_ROW(data->text_edit),
            VALP_STR_NON_VALID_CHARS_IN_SOURCE_TEXT_WARNING
            );
    }

    const char *raw_key = gtk_editable_get_text(GTK_EDITABLE(data->key_edit));
    wchar_t *key = (wchar_t*)g_utf8_to_ucs4(raw_key, 256, NULL, NULL, &error);
    const int raw_key_len = (int)wcslen(key);
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
        Crypto_VigenereAlgorithm_Decode(
            data->vigenere_algorithm,
            (int)input_len, input,
            (int)key_len, key,
            output
            );

        char *output_c = g_ucs4_to_utf8((gunichar *)output, 256, NULL, NULL, &error);
        gtk_editable_set_text(GTK_EDITABLE(data->summary_edit), output_c);

        char *key_out_c = g_ucs4_to_utf8((gunichar *)data->vigenere_algorithm->process_key, 256, NULL, NULL, &error);
        gtk_editable_set_text(GTK_EDITABLE(data->used_key_edit), key_out_c);

        free(output_c);
        free(key_out_c);
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

static void on_decode_pressed(GtkWidget *widget, gpointer user_data)
{
    struct VigenereAlgorithmData *data = ((AppPage *) user_data)->data;
    GError *error = NULL;
    wchar_t output[255];

    const char *input = gtk_editable_get_text(GTK_EDITABLE(data->text_edit));
    wchar_t *input_w = (wchar_t*)g_utf8_to_ucs4(input, -1, NULL, NULL, &error);
    long input_len = delete_all_non_russians_chars(input_w);

    const char *key = gtk_editable_get_text(GTK_EDITABLE(data->key_edit));
    wchar_t *key_w = (wchar_t*)g_utf8_to_ucs4(key, -1, NULL, NULL, &error);
    long key_len = delete_all_non_russians_chars(key_w);

    Crypto_VigenereAlgorithm_Encode(
        data->vigenere_algorithm,
        (int)input_len, input_w,
        (int)key_len, key_w,
        output);

    char *output_c = g_ucs4_to_utf8((gunichar *)output, -1, NULL, NULL, &error);
    gtk_editable_set_text(GTK_EDITABLE(data->summary_edit), output_c);

    char *key_out_c = g_ucs4_to_utf8((gunichar *)data->vigenere_algorithm->process_key, -1, NULL, NULL, &error);
    gtk_editable_set_text(GTK_EDITABLE(data->used_key_edit), key_out_c);

    free(input_w);
    free(key_w);
    free(output_c);
    free(key_out_c);
}

static void on_entry_changed(GtkWidget *widget, gpointer user_data)
{
    EntryDeco_ClearRowDecorations(ADW_ENTRY_ROW(widget));
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
    GtkWidget *label = gtk_label_new("Алгоритм Виженера (прогрессивный ключ)");
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
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->key_edit), "Ключ");
    GtkWidget *key_edit_icon = gtk_image_new_from_icon_name("dialog-password");
    adw_entry_row_add_prefix(ADW_ENTRY_ROW(data->key_edit), key_edit_icon);
    gtk_grid_attach(GTK_GRID(container), data->key_edit, 0, 1, 24, 1);
    g_signal_connect(G_OBJECT(data->key_edit), "changed", G_CALLBACK(on_entry_changed), data);

    data->text_edit = adw_entry_row_new();
    gtk_widget_add_css_class(data->text_edit, "card");
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->text_edit), "Исходный текст");
    GtkWidget *text_edit_icon = gtk_image_new_from_icon_name("font-x-generic-symbolic");
    adw_entry_row_add_prefix(ADW_ENTRY_ROW(data->text_edit), text_edit_icon);
    adw_entry_row_set_max_length(ADW_ENTRY_ROW(data->text_edit), 256);
    gtk_grid_attach(GTK_GRID(container), data->text_edit, 0, 2, 22, 1);
    g_signal_connect(G_OBJECT(data->text_edit), "changed", G_CALLBACK(on_entry_changed), data);

    GtkWidget *text_selection_btn = gtk_button_new_with_icon_and_label("folder-open-symbolic", "Файл...", 8);
    gtk_grid_attach(GTK_GRID(container), text_selection_btn, 22, 2, 2, 1);
    g_signal_connect(G_OBJECT(text_selection_btn), "clicked", G_CALLBACK(on_file_open_pressed), page);

    data->summary_edit = adw_entry_row_new();
    gtk_widget_add_css_class(data->summary_edit, "card");
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->summary_edit), "Итоговый текст");
    GtkWidget *summary_edit_icon = gtk_image_new_from_icon_name("media-playlist-shuffle");
    adw_entry_row_add_prefix(ADW_ENTRY_ROW(data->summary_edit), summary_edit_icon);
    gtk_editable_set_editable(GTK_EDITABLE(data->summary_edit), FALSE);
    gtk_grid_attach(GTK_GRID(container), data->summary_edit, 0, 3, 24, 1);

    data->used_key_edit = adw_entry_row_new();
    gtk_widget_add_css_class(data->used_key_edit, "card");
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->used_key_edit), "Прогрессивный ключ, полученный из оригинального");
    GtkWidget *used_key_edit_icon = gtk_image_new_from_icon_name("input-dialpad");
    adw_entry_row_add_prefix(ADW_ENTRY_ROW(data->used_key_edit), used_key_edit_icon);
    gtk_editable_set_editable(GTK_EDITABLE(data->used_key_edit), FALSE);
    gtk_grid_attach(GTK_GRID(container), data->used_key_edit, 0, 4, 24, 1);

    // Bottom
    GtkWidget *encode_btn = gtk_button_new_with_icon_and_label("changes-prevent", "Шифровать", 8);
    gtk_grid_attach(GTK_GRID(container), encode_btn, 0, 5, 10, 1);
    g_signal_connect(G_OBJECT(encode_btn), "clicked", G_CALLBACK(on_encode_pressed), page);

    GtkWidget *decode_btn = gtk_button_new_with_icon_and_label("changes-allow", "Дешифровать", 8);
    gtk_grid_attach(GTK_GRID(container), decode_btn, 10, 5, 10, 1);
    g_signal_connect(decode_btn, "clicked", G_CALLBACK(on_decode_pressed), page);

    GtkWidget *clear_btn = gtk_button_new_with_icon_and_label("edit-clear", "Очистить", 8);
    gtk_grid_attach(GTK_GRID(container), clear_btn, 20, 5, 4, 1);
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

void vigenere_algorithm_page_free(struct AppPage *page)
{

}
