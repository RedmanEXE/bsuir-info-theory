//
// Created by REXE on 22.03.26.
//

#include "pages.h"
#include "../crypto/lfsr.h"
#include "../../GTK Tools/file_selector_deco/selector_ext_icons.h"

#define BUFFER_SIZE                         512

void lfsr_page_create(struct AppPage *page, GtkWidget *window);
void lfsr_page_open_response(AppPage *page, GObject *original_object, GAsyncResult *res);
void lfsr_page_save_response(AppPage *page, GObject *original_object, GAsyncResult *res);
void lfsr_page_free(struct AppPage *page);

AppPage lfsr_page = {
    .on_create = lfsr_page_create,
    .on_file_dialog_open_result = lfsr_page_open_response,
    .on_file_dialog_save_result = lfsr_page_save_response,
    .on_free = lfsr_page_free
};

struct LFSRAlgorithmData
{
    Crypto_LFSRAlgorithm *lfsr_algorithm;

    GtkWidget *reg_edit;
    GtkWidget *in_file_selector;
    GtkWidget *in_file_selector_icon;
    GtkWidget *out_file_selector;
    GtkWidget *out_file_selector_icon;

    GFile *in_file;
    GFile *out_file;
};

static void process_file(Crypto_LFSRAlgorithm *manager, GFile *in_file, GFile *out_file)
{
    uint8_t in[BUFFER_SIZE], out[BUFFER_SIZE];

    GFileInputStream *in_stream = g_file_read(in_file, NULL, NULL);
    GFileOutputStream *out_stream = g_file_replace(out_file, NULL, TRUE, G_FILE_CREATE_NONE, NULL, NULL);

    int64_t read_bytes_len = 0;
    while (read_bytes_len = g_input_stream_read(G_INPUT_STREAM(in_stream), in, BUFFER_SIZE, NULL, NULL), 0 < read_bytes_len)
    {
        for (uint32_t i = 0; i < read_bytes_len; i++)
        {
            const uint8_t byte = Crypto_LFSRAlgorithm_GenerateByte(manager);
            out[i] = in[i] ^ byte;
        }
        g_output_stream_write(G_OUTPUT_STREAM(out_stream), out, read_bytes_len, NULL, NULL);
    }

    g_input_stream_close(G_INPUT_STREAM(in_stream), NULL, NULL);
    g_output_stream_close(G_OUTPUT_STREAM(out_stream), NULL, NULL);
}

static void set_info_for_action_row_by_name(GtkWidget *action_row, GtkWidget **icon_widget, GFile *file)
{
    if (NULL == file)
    {
        adw_action_row_remove(ADW_ACTION_ROW(action_row), *icon_widget);

        *icon_widget = gtk_image_new_from_icon_name("dialog-question");
        adw_action_row_add_prefix(ADW_ACTION_ROW(action_row), *icon_widget);

        adw_action_row_set_subtitle(ADW_ACTION_ROW(action_row), "");
        return;
    }

    char *basename = g_file_get_basename(file);
    const uint32_t basename_len = strlen(basename);
    uint32_t dot_pos = -1;

    for (uint32_t i = basename_len - 1; i > 0; i--)
        if ('.' == basename[i])
        {
            dot_pos = i;
            break;
        }

    adw_action_row_remove(ADW_ACTION_ROW(action_row), *icon_widget);
    if (-1 != dot_pos)
    {
        char *ext = strndup(basename + dot_pos, basename_len - dot_pos);
        const char *icon = FileSelector_GetIconByExtension(ext);

        *icon_widget = gtk_image_new_from_icon_name(icon);
        adw_action_row_add_prefix(ADW_ACTION_ROW(action_row), *icon_widget);

        g_free(ext);
    } else
    {
        *icon_widget = gtk_image_new_from_icon_name("application-x-sharedlib-symbolic");
        adw_action_row_add_prefix(ADW_ACTION_ROW(action_row), *icon_widget);
    }
    adw_action_row_set_subtitle(ADW_ACTION_ROW(action_row), basename);

    g_free(basename);
}

static void on_launch_button_clicked(GtkWidget *widget, gpointer user_data)
{
    struct LFSRAlgorithmData *data = (struct LFSRAlgorithmData *)user_data;

    const char *reg_as_str = gtk_editable_get_text(GTK_EDITABLE(data->reg_edit));
    const uint64_t reg_len = adw_entry_row_get_text_length(ADW_ENTRY_ROW(data->reg_edit));
    uint64_t reg = 0;

    for (uint64_t i = 0; i < reg_len; i++)
    {
        reg <<= 1;
        if ('1' == reg_as_str[i])
            reg |= 1;
    }

    Crypto_LFSRAlgorithm_SetRegister(data->lfsr_algorithm, reg);
    process_file(data->lfsr_algorithm, data->in_file, data->out_file);
}

static void on_change_button_clicked(GtkWidget *widget, gpointer user_data)
{
    struct LFSRAlgorithmData *data = (struct LFSRAlgorithmData *)user_data;

    GFile *buf = data->in_file;
    data->in_file = data->out_file;
    data->out_file = buf;

    set_info_for_action_row_by_name(data->in_file_selector, &data->in_file_selector_icon, data->in_file);
    set_info_for_action_row_by_name(data->out_file_selector, &data->out_file_selector_icon, data->out_file);
}

static void on_out_file_selector_activated(GtkWidget *widget, gpointer user_data)
{
    AppPage *page = (AppPage *)user_data;
    open_file_save_dialog(page);
}

static void on_in_file_selector_activated(GtkWidget *widget, gpointer user_data)
{
    AppPage *page = (AppPage *)user_data;
    open_file_open_dialog(page);
}

static void on_reg_edit_insert_text(
    GtkWidget *widget,
    gchar *new_text,
    gint new_text_length,
    gint *position,
    gpointer user_data
)
{
    struct LFSRAlgorithmData *data = (struct LFSRAlgorithmData *)user_data;

    uint32_t copied = 0;
    for (uint32_t i = 0; i < new_text_length; i++)
    {
        if ('0' == new_text[i] || '1' == new_text[i])
            new_text[copied++] = new_text[i];
    }
    new_text[copied] = '\0';
}

void lfsr_page_create(struct AppPage *page, GtkWidget *window)
{
    struct LFSRAlgorithmData *data = calloc(1, sizeof(struct LFSRAlgorithmData));
    data->lfsr_algorithm = Crypto_LFSRAlgorithm_Create();
    page->data = data;

    GtkWidget *container = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(container), 10);
    gtk_grid_set_row_spacing(GTK_GRID(container), 10);
    gtk_widget_set_margin(GTK_WIDGET(container), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(container), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(container), TRUE);

    // Top
    GtkWidget *label = gtk_label_new("Поточное LFSR шифрование");
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

    // Center
    GtkWidget *reg_edit_wrapper = adw_preferences_group_new();

    data->reg_edit = adw_entry_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->reg_edit), "Начальное значение регистра");
    GtkWidget *reg_edit_icon = gtk_image_new_from_icon_name("dialog-password");
    adw_entry_row_add_prefix(ADW_ENTRY_ROW(data->reg_edit), reg_edit_icon);
    adw_entry_row_set_max_length(ADW_ENTRY_ROW(data->reg_edit), REGISTER_LENGTH);
    g_signal_connect(G_OBJECT(gtk_editable_get_delegate(GTK_EDITABLE(data->reg_edit))), "insert-text",
                     G_CALLBACK(on_reg_edit_insert_text), data);

    adw_preferences_group_add(ADW_PREFERENCES_GROUP(reg_edit_wrapper), data->reg_edit);
    gtk_grid_attach(GTK_GRID(container), reg_edit_wrapper, 0, 1, 24, 1);

    GtkWidget *in_file_selector_wrapper = adw_preferences_group_new();

    data->in_file_selector = adw_action_row_new();
    gtk_widget_add_css_class(data->in_file_selector, "property");
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->in_file_selector), "Входной файл");
    adw_action_row_set_subtitle_lines(ADW_ACTION_ROW(data->in_file_selector), 1);
    data->in_file_selector_icon = gtk_image_new_from_icon_name("dialog-question");
    adw_action_row_add_prefix(ADW_ACTION_ROW(data->in_file_selector), data->in_file_selector_icon);
    GtkWidget *in_file_selector_file_select = gtk_button_new();
    gtk_widget_add_css_class(in_file_selector_file_select, "circular");
    gtk_widget_add_css_class(in_file_selector_file_select, "flat");
    gtk_button_set_icon_name(GTK_BUTTON(in_file_selector_file_select), "folder-open-symbolic");
    gtk_widget_set_margin_top(in_file_selector_file_select, 8);
    gtk_widget_set_margin_bottom(in_file_selector_file_select, 8);
    adw_action_row_add_suffix(ADW_ACTION_ROW(data->in_file_selector), in_file_selector_file_select);
    g_signal_connect(G_OBJECT(in_file_selector_file_select), "clicked", G_CALLBACK(on_in_file_selector_activated), page);
    adw_action_row_set_activatable_widget(ADW_ACTION_ROW(data->in_file_selector), in_file_selector_file_select);

    adw_preferences_group_add(ADW_PREFERENCES_GROUP(in_file_selector_wrapper), data->in_file_selector);
    gtk_grid_attach(GTK_GRID(container), in_file_selector_wrapper, 0, 2, 11, 1);

    GtkWidget *btn_change = gtk_button_new_with_icon_and_label("object-flip-horizontal", "", 0);
    g_signal_connect(G_OBJECT(btn_change), "clicked", G_CALLBACK(on_change_button_clicked), data);
    gtk_grid_attach(GTK_GRID(container), btn_change, 11, 2, 2, 1);

    GtkWidget *out_file_selector_wrapper = adw_preferences_group_new();

    data->out_file_selector = adw_action_row_new();
    gtk_widget_add_css_class(data->out_file_selector, "property");
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->out_file_selector), "Итоговый файл");
    adw_action_row_set_subtitle_lines(ADW_ACTION_ROW(data->out_file_selector), 1);
    data->out_file_selector_icon = gtk_image_new_from_icon_name("dialog-question");
    adw_action_row_add_prefix(ADW_ACTION_ROW(data->out_file_selector), data->out_file_selector_icon);
    GtkWidget *out_file_selector_file_select = gtk_button_new();
    gtk_widget_add_css_class(out_file_selector_file_select, "circular");
    gtk_widget_add_css_class(out_file_selector_file_select, "flat");
    gtk_button_set_icon_name(GTK_BUTTON(out_file_selector_file_select), "folder-open-symbolic");
    gtk_widget_set_margin_top(out_file_selector_file_select, 8);
    gtk_widget_set_margin_bottom(out_file_selector_file_select, 8);
    adw_action_row_add_suffix(ADW_ACTION_ROW(data->out_file_selector), out_file_selector_file_select);
    g_signal_connect(G_OBJECT(out_file_selector_file_select), "clicked", G_CALLBACK(on_out_file_selector_activated), page);
    adw_action_row_set_activatable_widget(ADW_ACTION_ROW(data->out_file_selector), out_file_selector_file_select);

    adw_preferences_group_add(ADW_PREFERENCES_GROUP(out_file_selector_wrapper), data->out_file_selector);
    gtk_grid_attach(GTK_GRID(container), out_file_selector_wrapper, 13, 2, 11, 1);

    // Bottom
    GtkWidget *btn_launch = gtk_button_new_with_icon_and_label("media-playback-start", "Запуск", 8);
    g_signal_connect(G_OBJECT(btn_launch), "clicked", G_CALLBACK(on_launch_button_clicked), data);
    gtk_grid_attach(GTK_GRID(container), btn_launch, 0, 3, 18, 1);
    GtkWidget *btn_clear = gtk_button_new_with_icon_and_label("edit-clear", "Очистить", 8);
    gtk_widget_add_css_class(btn_clear, "destructive-action");
    gtk_grid_attach(GTK_GRID(container), btn_clear, 18, 3, 6, 1);

    page->page = container;
}

void lfsr_page_open_response(AppPage *page, GObject *original_object, GAsyncResult *res)
{
    struct LFSRAlgorithmData *data = page->data;

    GtkFileDialog *dialog = GTK_FILE_DIALOG(original_object);
    GError *error = NULL;
    GFile *file = gtk_file_dialog_open_finish(dialog, res, &error);

    if (NULL != file) {
        data->in_file = file;
        set_info_for_action_row_by_name(data->in_file_selector, &data->in_file_selector_icon, data->in_file);
    } else {
        g_clear_error(&error);
    }
}

void lfsr_page_save_response(AppPage *page, GObject *original_object, GAsyncResult *res)
{
    struct LFSRAlgorithmData *data = page->data;

    GtkFileDialog *dialog = GTK_FILE_DIALOG(original_object);
    GError *error = NULL;
    GFile *file = gtk_file_dialog_save_finish(dialog, res, &error);

    if (NULL != file) {
        data->out_file = file;
        set_info_for_action_row_by_name(data->out_file_selector, &data->out_file_selector_icon, data->out_file);
    } else {
        g_clear_error(&error);
    }
}

void lfsr_page_free(struct AppPage *page)
{

}