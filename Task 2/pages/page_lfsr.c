//
// Created by REXE on 22.03.26.
//

#include "pages.h"
#include "../crypto/lfsr.h"
#include "../../GTK Tools/file_selector_deco/selector_ext_icons.h"
#include "../strings/ru_strings.h"

#define BUFFER_SIZE                         2048

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

struct LFSREndData
{
    struct LFSRAlgorithmData *data;

    uint8_t start_gen[8];
    uint8_t start_in[8];
    uint8_t start_out[8];

    uint8_t end_gen[8];
    uint8_t end_in[8];
    uint8_t end_out[8];

    uint64_t total_bytes;
};

struct LFSRProgressData
{
    struct LFSRAlgorithmData *data;

    uint64_t processed_bytes;
    uint64_t total_bytes;
};

struct LFSRAlgorithmData
{
    Crypto_LFSRAlgorithm *lfsr_algorithm;

    GtkWidget *parent;

    AdwDialog *progress_dialog;
    GtkWidget *progress_bar;
    GtkWidget *progress_label;

    GtkWidget *reg_edit;
    GtkWidget *reg_edit_counter;
    GtkWidget *in_file_selector;
    GtkWidget *in_file_selector_icon;
    GtkWidget *out_file_selector;
    GtkWidget *out_file_selector_icon;

    GtkWidget *in_file_bytes_edit;
    GtkWidget *out_bytes_edit;
    GtkWidget *out_file_bytes_edit;

    GFile *in_file;
    GFile *out_file;
    int (*on_progress_changed)(gpointer msg);
    int (*on_process_ended)(gpointer data);
};

static void apply_inline_css_styles(void)
{
    GtkCssProvider *provider = gtk_css_provider_new();

    const char *pb_css =
        "progressbar.button-style trough {"
        "   min-height: 40px;"
        "   border-radius: 8px;"
        "}"
        "progressbar.button-style progress {"
        "   min-height: 40px;"
        "   border-radius: 7px;"
        "   transition: all 200ms cubic-bezier(0.25, 0.46, 0.45, 0.94);"
        "}"
        ".progress-label {"
        "   font-weight: bold;"
        "   color: white;"
        "}"
        "row.floating-row .title {"
        "   transition: all 180ms cubic-bezier(0.25, 0.46, 0.45, 0.94);"
        "   transform-origin: left top;"
        "   transform: translateY(4px) scale(1.08);"
        "}"
        "row.floating-row .subtitle {"
        "   transition: all 180ms cubic-bezier(0.25, 0.46, 0.45, 0.94);"
        "   opacity: 0;"
        "}"
        "row.floating-row.has-subtitle .title {"
        "   transform: translateY(0) scale(1);"
        "}"
        "row.floating-row.has-subtitle .subtitle {"
        "   opacity: 1;"
        "}"
        "textview,"
        "textview > text,"
        "textview > border {"
        "   background: transparent;"
        "}"
        "button, row, textview, label {"
        "   font-weight: bold;"
        "}"
        "list, .no-shadow {"
        "   box-shadow: none;"
        "}";

    gtk_css_provider_load_from_string(provider, pb_css);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

static GtkWidget *create_fancy_text_view(GtkWidget **edit, const char *text, const char *icon_name)
{
    GtkWidget *card_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(card_box, "card");
    gtk_widget_add_css_class(card_box, "no-shadow");
    gtk_widget_set_vexpand(card_box, TRUE);

    GtkWidget *icon = gtk_image_new_from_icon_name(icon_name);
    gtk_widget_set_valign(icon, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_start(icon, 14);

    GtkWidget *heading_label = gtk_label_new(text);
    gtk_widget_set_valign(heading_label, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_start(heading_label, 12);
    gtk_widget_set_margin_end(heading_label, 12);

    gtk_label_set_width_chars(GTK_LABEL(heading_label), 15);
    gtk_label_set_max_width_chars(GTK_LABEL(heading_label), 15);
    gtk_widget_set_halign(heading_label, GTK_ALIGN_START);
    gtk_label_set_xalign(GTK_LABEL(heading_label), 0.0);
    gtk_label_set_ellipsize(GTK_LABEL(heading_label), PANGO_ELLIPSIZE_END);
    gtk_widget_add_css_class(heading_label, "dim-label");

    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_margin_top(sep, 8);
    gtk_widget_set_margin_bottom(sep, 8);

    *edit = gtk_text_view_new();
    gtk_widget_add_css_class(*edit, "flat");
    gtk_widget_add_css_class(*edit, "monospace");
    gtk_text_view_set_justification(GTK_TEXT_VIEW(*edit), GTK_JUSTIFY_CENTER);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(*edit), FALSE);
    gtk_widget_set_margin(*edit, 8);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(*edit), GTK_WRAP_WORD_CHAR);

    GtkWidget *out_scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(out_scrolled_window), *edit);
    gtk_widget_set_hexpand(out_scrolled_window, TRUE);
    gtk_widget_set_vexpand(out_scrolled_window, TRUE);

    gtk_box_append(GTK_BOX(card_box), icon);
    gtk_box_append(GTK_BOX(card_box), heading_label);
    gtk_box_append(GTK_BOX(card_box), sep);
    gtk_box_append(GTK_BOX(card_box), out_scrolled_window);

    return card_box;
}

void action_row_set_subtitle_animated(AdwActionRow *row, const char *text)
{
    if (text && 0 < strlen(text))
    {
        adw_action_row_set_subtitle(row, text);
        gtk_widget_add_css_class(GTK_WIDGET(row), "has-subtitle");
    }
    else
    {
        gtk_widget_remove_css_class(GTK_WIDGET(row), "has-subtitle");
        adw_action_row_set_subtitle(row, " ");
    }
}

GtkWidget* create_fancy_progress_bar(struct LFSRAlgorithmData *data)
{
    GtkWidget *overlay = gtk_overlay_new();

    GtkWidget *pb = gtk_progress_bar_new();
    gtk_widget_add_css_class(pb, "button-style");
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pb), 0.0f);

    GtkWidget *label = gtk_label_new("");
    data->progress_label = label;
    gtk_widget_add_css_class(label, "progress-label");

    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(label, GTK_ALIGN_CENTER);

    gtk_overlay_set_child(GTK_OVERLAY(overlay), pb);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), label);

    return overlay;
}

static void show_progress_dialog(struct LFSRAlgorithmData *data)
{
    data->progress_dialog = adw_dialog_new();
    adw_dialog_set_follows_content_size(ADW_DIALOG(data->progress_dialog), TRUE);
    adw_dialog_set_presentation_mode(ADW_DIALOG(data->progress_dialog), ADW_DIALOG_FLOATING);
    adw_dialog_set_can_close(ADW_DIALOG(data->progress_dialog), FALSE);

    GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // GtkWidget *header = adw_header_bar_new();
    // gtk_box_append(GTK_BOX(container), header);

    GtkWidget *root_container = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(root_container), 10);
    gtk_grid_set_row_spacing(GTK_GRID(root_container), 10);
    gtk_widget_set_margin(GTK_WIDGET(root_container), 20);
    gtk_grid_set_column_homogeneous(GTK_GRID(root_container), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(root_container), TRUE);

    GtkWidget *title = gtk_label_new(LFSRP_STR_DIALOG_PROGRESS_TITLE);
    gtk_label_set_yalign(GTK_LABEL(title), 0.5f);
    {
        PangoAttrList *label_attrlist = pango_attr_list_new();
        PangoFontDescription *label_attrlist_font_desc = pango_font_description_new();
        pango_font_description_set_size(label_attrlist_font_desc, 20 * PANGO_SCALE);
        pango_font_description_set_weight(label_attrlist_font_desc, PANGO_WEIGHT_HEAVY);
        PangoAttribute *label_attrlist_attr = pango_attr_font_desc_new(label_attrlist_font_desc);
        pango_attr_list_insert(label_attrlist, label_attrlist_attr);
        gtk_label_set_attributes(GTK_LABEL(title), label_attrlist);
    }
    gtk_grid_attach(GTK_GRID(root_container), title, 0, 0, 4, 1);

    GtkWidget *desc = gtk_label_new(LFSRP_STR_DIALOG_PROGRESS_DESCRIPTION);
    gtk_label_set_justify(GTK_LABEL(desc), GTK_JUSTIFY_CENTER);
    gtk_label_set_xalign(GTK_LABEL(desc), 0.5f);
    gtk_grid_attach(GTK_GRID(root_container), desc, 0, 1, 4, 1);

    data->progress_bar = create_fancy_progress_bar(data);
    gtk_grid_attach(GTK_GRID(root_container), data->progress_bar, 0, 2, 4, 1);

    gtk_box_append(GTK_BOX(container), root_container);

    adw_dialog_set_child(ADW_DIALOG(data->progress_dialog), container);
    adw_dialog_present(ADW_DIALOG(data->progress_dialog), data->parent);
}

char *format_bytes_to_string(uint64_t bytes)
{
    const char *units[] = {
        LFSRP_STR_DIALOG_MEMORY_UNITS_B,
        LFSRP_STR_DIALOG_MEMORY_UNITS_KB,
        LFSRP_STR_DIALOG_MEMORY_UNITS_MB,
        LFSRP_STR_DIALOG_MEMORY_UNITS_GB,
        LFSRP_STR_DIALOG_MEMORY_UNITS_TB,
        LFSRP_STR_DIALOG_MEMORY_UNITS_PB
    };
    int unit_index = 0;
    double formatted_size = (double)bytes;

    while (formatted_size >= 1024.0 && unit_index < 5)
    {
        formatted_size /= 1024.0;
        unit_index++;
    }

    if (unit_index == 0)
        return g_strdup_printf(LFSRP_STR_DIALOG_PROGRESS_VALUE_WOF_TEXT, formatted_size, units[unit_index]);
    return g_strdup_printf(LFSRP_STR_DIALOG_PROGRESS_VALUE_WF_TEXT, formatted_size, units[unit_index]);
}

void format_arrays_to_binary(const uint64_t total_bytes, const uint8_t arr1[8],
                             const uint8_t arr2[8], char *out_buffer)
{
    char *ptr = out_buffer;
    int n1 = total_bytes > 8 ? 8 : (int)total_bytes;

    for (int i = 0; i < n1; i++)
    {
        for (int bit = 7; bit >= 0; bit--)
            *ptr++ = (arr1[i] & (1 << bit)) ? '1' : '0';

        if (i < n1 - 1)
            *ptr++ = ' ';
    }

    if (total_bytes > 8)
    {
        *ptr++ = '\n';
        if (total_bytes > 16)
        {
            *ptr++ = '.';
            *ptr++ = '.';
            *ptr++ = '.';
            *ptr++ = '\n';
        }

        int n2 = total_bytes > 16 ? 8 : (int)(total_bytes - 8);
        for (int i = 0; i < n2; i++)
        {
            for (int bit = 7; bit >= 0; bit--)
                *ptr++ = (arr2[i] & (1 << bit)) ? '1' : '0';

            if (i < n2 - 1)
                *ptr++ = ' ';
        }
    }

    *ptr = '\0';
}

static int on_process_ended(gpointer user_data)
{
    struct LFSREndData *data = (struct LFSREndData *)user_data;

    char bin_text[148];
    format_arrays_to_binary(data->total_bytes, data->start_in, data->end_in, bin_text);
    gtk_text_buffer_set_text(
        GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->data->in_file_bytes_edit))), bin_text, -1);
    format_arrays_to_binary(data->total_bytes, data->start_gen, data->end_gen, bin_text);
    gtk_text_buffer_set_text(
        GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->data->out_bytes_edit))), bin_text, -1);
    format_arrays_to_binary(data->total_bytes, data->start_out, data->end_out, bin_text);
    gtk_text_buffer_set_text(
        GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->data->out_file_bytes_edit))), bin_text, -1);

    adw_dialog_set_can_close(data->data->progress_dialog, TRUE);
    adw_dialog_close(data->data->progress_dialog);

    g_free(user_data);

    return G_SOURCE_REMOVE;
}

static int on_progress_changed(gpointer user_data)
{
    struct LFSRProgressData *data = (struct LFSRProgressData *)user_data;

    GtkWidget *pb = gtk_overlay_get_child(GTK_OVERLAY(data->data->progress_bar));
    double fraction = ((double)data->processed_bytes) / ((double)data->total_bytes);
    if (fraction > 1.0)
        fraction = 1.0;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pb), fraction);

    char *processed_str = format_bytes_to_string(data->processed_bytes);
    char *total_str = format_bytes_to_string(data->total_bytes);
    char *status_text = g_strdup_printf(LFSRP_STR_DIALOG_PROGRESS_TEXT, processed_str, total_str);
    gtk_label_set_text(GTK_LABEL(data->data->progress_label), status_text);

    g_free(processed_str);
    g_free(total_str);
    g_free(status_text);
    g_free(user_data);

    return G_SOURCE_REMOVE;
}

static gpointer process_file(gpointer user_data) {
    struct LFSRAlgorithmData *data = (struct LFSRAlgorithmData *)user_data;

    uint8_t in[BUFFER_SIZE], out[BUFFER_SIZE];

    GFileInfo *in_info = g_file_query_info(data->in_file, G_FILE_ATTRIBUTE_STANDARD_SIZE, G_FILE_QUERY_INFO_NONE, NULL,
                                           NULL);
    goffset in_size = g_file_info_get_size(in_info);
    g_object_unref(in_info);

    GFileInputStream *in_stream = g_file_read(data->in_file, NULL, NULL);
    GFileOutputStream *out_stream = g_file_replace(data->out_file, NULL, TRUE, G_FILE_CREATE_NONE, NULL, NULL);

    uint8_t begin_bytes_gen[8], begin_bytes_in[8], begin_bytes_out[8];
    uint8_t end_bytes_gen[8], end_bytes_in[8], end_bytes_out[8];
    int64_t last_update_time = 0;
    int64_t read_bytes_len = 0;
    uint64_t total_bytes_processed = 0;
    while (read_bytes_len = g_input_stream_read(G_INPUT_STREAM(in_stream), in, BUFFER_SIZE, NULL, NULL), 0 < read_bytes_len)
    {
        for (uint32_t i = 0; i < read_bytes_len; i++)
        {
            const uint8_t byte = Crypto_LFSRAlgorithm_GenerateByte(data->lfsr_algorithm);
            out[i] = in[i] ^ byte;

            if (total_bytes_processed < 8 && i < 8)
            {
                begin_bytes_in[i] = in[i];
                begin_bytes_gen[i] = byte;
                begin_bytes_out[i] = out[i];
            }

            if (read_bytes_len - i - 1 < 8)
            {
                end_bytes_in[read_bytes_len - i - 1] = in[i];
                end_bytes_gen[read_bytes_len - i - 1] = byte;
                end_bytes_out[read_bytes_len - i - 1] = out[i];
            }
        }
        g_output_stream_write(G_OUTPUT_STREAM(out_stream), out, read_bytes_len, NULL, NULL);

        total_bytes_processed += read_bytes_len;
        int64_t current_time = g_get_monotonic_time();
        if (current_time - last_update_time > 33000)
        {
            struct LFSRProgressData *msg = (struct LFSRProgressData *)g_new(struct LFSRProgressData, 1);
            msg->data = data;
            msg->processed_bytes = total_bytes_processed;
            msg->total_bytes = in_size;
            g_idle_add(data->on_progress_changed, msg);
            last_update_time = current_time;
        }
    }

    g_input_stream_close(G_INPUT_STREAM(in_stream), NULL, NULL);
    g_output_stream_close(G_OUTPUT_STREAM(out_stream), NULL, NULL);

    struct LFSREndData *end_data = (struct LFSREndData *)g_new(struct LFSREndData, 1);
    end_data->data = data;
    end_data->total_bytes = in_size;
    memcpy(end_data->start_in, begin_bytes_in, sizeof(begin_bytes_in));
    memcpy(end_data->start_gen, begin_bytes_gen, sizeof(begin_bytes_gen));
    memcpy(end_data->start_out, begin_bytes_out, sizeof(begin_bytes_out));
    memcpy(end_data->end_in, end_bytes_in, sizeof(end_bytes_in));
    memcpy(end_data->end_gen, end_bytes_gen, sizeof(end_bytes_gen));
    memcpy(end_data->end_out, end_bytes_out, sizeof(end_bytes_out));
    g_idle_add(data->on_process_ended, end_data);

    return NULL;
}

static void set_info_for_action_row_by_name(GtkWidget *action_row, GtkWidget **icon_widget, GFile *file)
{
    if (NULL == file)
    {
        adw_action_row_remove(ADW_ACTION_ROW(action_row), *icon_widget);

        *icon_widget = gtk_image_new_from_icon_name("dialog-question");
        adw_action_row_add_prefix(ADW_ACTION_ROW(action_row), *icon_widget);

        action_row_set_subtitle_animated(ADW_ACTION_ROW(action_row), "");
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
        char *ext = g_strndup(basename + dot_pos, basename_len - dot_pos);
        const char *icon = FileSelector_GetIconByExtension(ext);

        *icon_widget = gtk_image_new_from_icon_name(icon);
        adw_action_row_add_prefix(ADW_ACTION_ROW(action_row), *icon_widget);

        g_free(ext);
    } else
    {
        *icon_widget = gtk_image_new_from_icon_name("application-x-sharedlib-symbolic");
        adw_action_row_add_prefix(ADW_ACTION_ROW(action_row), *icon_widget);
    }
    action_row_set_subtitle_animated(ADW_ACTION_ROW(action_row), basename);

    g_free(basename);
}

static void on_clear_button_clicked(GtkWidget *widget, gpointer user_data)
{
    struct LFSRAlgorithmData *data = (struct LFSRAlgorithmData *)user_data;

    gtk_editable_set_text(GTK_EDITABLE(data->reg_edit), "");
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->out_bytes_edit))), "", -1);

    if (NULL != data->in_file)
        g_object_unref(data->in_file);
    data->in_file = NULL;
    set_info_for_action_row_by_name(data->in_file_selector, &data->in_file_selector_icon, NULL);

    if (NULL != data->out_file)
        g_object_unref(data->out_file);
    data->out_file = NULL;
    set_info_for_action_row_by_name(data->out_file_selector, &data->out_file_selector_icon, NULL);
}

static void on_launch_button_clicked(GtkWidget *widget, gpointer user_data)
{
    struct LFSRAlgorithmData *data = (struct LFSRAlgorithmData *)user_data;

    const uint64_t reg_len = adw_entry_row_get_text_length(ADW_ENTRY_ROW(data->reg_edit));
    const char *reg_as_str = gtk_editable_get_text(GTK_EDITABLE(data->reg_edit));
    if (NULL == data->in_file || NULL == data->out_file || 0 == reg_len)
    {
        if (NULL == data->in_file)
            gtk_widget_add_css_class(data->in_file_selector, "error");

        if (NULL == data->out_file)
            gtk_widget_add_css_class(data->out_file_selector, "error");

        if (2 > reg_len)
            EntryDeco_MarkEntryRowAsError(ADW_ENTRY_ROW(data->reg_edit), LFSRP_STR_EMPTY_REGISTER_ERROR);

        return;
    }

    uint64_t reg = 0;

    for (uint64_t i = 0; i < reg_len; i++)
    {
        reg <<= 1;
        if ('1' == reg_as_str[i])
            reg |= 1;
    }

    if (0 == reg)
    {
        EntryDeco_MarkEntryRowAsError(ADW_ENTRY_ROW(data->reg_edit), LFSRP_STR_NO_ONES_IN_REGISTER_ERROR);
        return;
    }

    show_progress_dialog(user_data);
    Crypto_LFSRAlgorithm_SetRegister(data->lfsr_algorithm, reg);
    g_thread_new("file-processor", process_file, data);
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
    struct LFSRAlgorithmData *data = (struct LFSRAlgorithmData *)page->data;
    gtk_widget_remove_css_class(data->out_file_selector, "error");

    open_file_save_dialog(page);
}

static void on_in_file_selector_activated(GtkWidget *widget, gpointer user_data)
{
    AppPage *page = (AppPage *)user_data;
    struct LFSRAlgorithmData *data = (struct LFSRAlgorithmData *)page->data;
    gtk_widget_remove_css_class(data->in_file_selector, "error");

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
        if (copied + (*position) >= REGISTER_LENGTH)
            break;
        if ('0' == new_text[i] || '1' == new_text[i])
            new_text[copied++] = new_text[i];
    }
    new_text[copied] = '\0';
}

static void on_reg_edit_changed(GtkWidget *widget, gpointer user_data)
{
    struct LFSRAlgorithmData *data = (struct LFSRAlgorithmData *)user_data;

    EntryDeco_ClearRowDecorations(ADW_ENTRY_ROW(widget));

    const char *text = gtk_editable_get_text(GTK_EDITABLE(widget));
    glong length = g_utf8_strlen(text, -1);

    char *counter_str = g_strdup_printf(LFSRP_STR_ENTRY_BEGIN_REGISTER_STATE_COUNTER_TEXT, length, REGISTER_LENGTH);
    gtk_label_set_text(GTK_LABEL(data->reg_edit_counter), counter_str);
    g_free(counter_str);
}

void lfsr_page_create(struct AppPage *page, GtkWidget *window)
{
    struct LFSRAlgorithmData *data = calloc(1, sizeof(struct LFSRAlgorithmData));
    data->lfsr_algorithm = Crypto_LFSRAlgorithm_Create();
    data->parent = window;
    data->on_progress_changed = on_progress_changed;
    data->on_process_ended = on_process_ended;
    page->data = data;

    apply_inline_css_styles();

    GtkWidget *container = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(container), 10);
    gtk_grid_set_row_spacing(GTK_GRID(container), 10);
    gtk_widget_set_margin(GTK_WIDGET(container), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(container), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(container), TRUE);

    // Top
    GtkWidget *label = gtk_label_new(LFSRP_STR_TITLE);
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
    gtk_grid_attach(GTK_GRID(container), label, 0, 0, 36, 1);

    // Center
    GtkWidget *reg_edit_wrapper = adw_preferences_group_new();

    data->reg_edit = adw_entry_row_new();
    gtk_widget_set_vexpand(GTK_WIDGET(data->reg_edit), TRUE);
    gtk_widget_set_valign(GTK_WIDGET(data->reg_edit), GTK_ALIGN_FILL);
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->reg_edit), LFSRP_STR_ENTRY_BEGIN_REGISTER_STATE_TITLE);
    GtkWidget *reg_edit_icon = gtk_image_new_from_icon_name("dialog-password");
    adw_entry_row_add_prefix(ADW_ENTRY_ROW(data->reg_edit), reg_edit_icon);

    char *counter_str = g_strdup_printf(LFSRP_STR_ENTRY_BEGIN_REGISTER_STATE_COUNTER_TEXT, 0, REGISTER_LENGTH);
    data->reg_edit_counter = gtk_label_new(counter_str);
    gtk_widget_add_css_class(data->reg_edit_counter, "dim-label");
    g_free(counter_str);
    adw_entry_row_add_suffix(ADW_ENTRY_ROW(data->reg_edit), data->reg_edit_counter);

    // adw_entry_row_set_max_length(ADW_ENTRY_ROW(data->reg_edit), REGISTER_LENGTH);
    g_signal_connect(G_OBJECT(data->reg_edit), "changed", G_CALLBACK(on_reg_edit_changed), data);
    g_signal_connect(G_OBJECT(gtk_editable_get_delegate(GTK_EDITABLE(data->reg_edit))), "insert-text",
                     G_CALLBACK(on_reg_edit_insert_text), data);

    adw_preferences_group_add(ADW_PREFERENCES_GROUP(reg_edit_wrapper), data->reg_edit);
    gtk_grid_attach(GTK_GRID(container), reg_edit_wrapper, 0, 1, 36, 1);

    GtkWidget *in_file_selector_wrapper = adw_preferences_group_new();

    data->in_file_selector = adw_action_row_new();
    gtk_widget_set_vexpand(GTK_WIDGET(data->in_file_selector), TRUE);
    gtk_widget_set_valign(GTK_WIDGET(data->in_file_selector), GTK_ALIGN_FILL);
    gtk_widget_add_css_class(data->in_file_selector, "property");
    gtk_widget_add_css_class(data->in_file_selector, "floating-row");
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->in_file_selector), LFSRP_STR_ENTRY_INPUT_FILE_TITLE);
    adw_action_row_set_subtitle(ADW_ACTION_ROW(data->in_file_selector), " ");
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
    gtk_grid_attach(GTK_GRID(container), in_file_selector_wrapper, 0, 2, 17, 1);

    GtkWidget *btn_change = gtk_button_new_with_icon_and_label("object-flip-horizontal", "", 0);
    g_signal_connect(G_OBJECT(btn_change), "clicked", G_CALLBACK(on_change_button_clicked), data);
    gtk_grid_attach(GTK_GRID(container), btn_change, 17, 2, 2, 1);

    GtkWidget *out_file_selector_wrapper = adw_preferences_group_new();

    data->out_file_selector = adw_action_row_new();
    gtk_widget_set_vexpand(GTK_WIDGET(data->out_file_selector), TRUE);
    gtk_widget_set_valign(GTK_WIDGET(data->out_file_selector), GTK_ALIGN_FILL);
    gtk_widget_add_css_class(data->out_file_selector, "property");
    gtk_widget_add_css_class(data->out_file_selector, "floating-row");
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(data->out_file_selector), LFSRP_STR_ENTRY_OUTPUT_FILE_TITLE);
    adw_action_row_set_subtitle(ADW_ACTION_ROW(data->out_file_selector), " ");
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
    gtk_grid_attach(GTK_GRID(container), out_file_selector_wrapper, 19, 2, 17, 1);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin(vbox, 10);
    GtkWidget *in_file_bytes_box = create_fancy_text_view(&data->in_file_bytes_edit, LFSRP_STR_ENTRY_INPUT_FILE_BYTES_TITLE, "format-indent-more");
    GtkWidget *out_bytes_box = create_fancy_text_view(&data->out_bytes_edit, LFSRP_STR_ENTRY_GENERATED_BYTES_TITLE, "media-view-subtitles");
    GtkWidget *out_file_bytes_box = create_fancy_text_view(&data->out_file_bytes_edit, LFSRP_STR_ENTRY_OUTPUT_FILE_BYTES_TITLE, "format-indent-less");
    gtk_box_append(GTK_BOX(vbox), in_file_bytes_box);
    gtk_box_append(GTK_BOX(vbox), out_bytes_box);
    gtk_box_append(GTK_BOX(vbox), out_file_bytes_box);
    GtkWidget *main_bin = adw_bin_new();
    gtk_widget_add_css_class(main_bin, "card");
    adw_bin_set_child(ADW_BIN(main_bin), vbox);
    gtk_grid_attach(GTK_GRID(container), main_bin, 0, 3, 36, 3);

    // Bottom
    GtkWidget *btn_launch = gtk_button_new_with_icon_and_label("media-playback-start", APP_STR_LAUNCH_TITLE, 8);
    gtk_widget_add_css_class(btn_launch, "bottom-button-left");
    g_signal_connect(G_OBJECT(btn_launch), "clicked", G_CALLBACK(on_launch_button_clicked), data);
    gtk_grid_attach(GTK_GRID(container), btn_launch, 0, 6, 28, 1);
    GtkWidget *btn_clear = gtk_button_new_with_icon_and_label("edit-clear", APP_STR_CLEAR_TITLE, 8);
    gtk_widget_add_css_class(btn_clear, "bottom-button-right");
    gtk_widget_add_css_class(btn_clear, "destructive-action");
    g_signal_connect(G_OBJECT(btn_clear), "clicked", G_CALLBACK(on_clear_button_clicked), data);
    gtk_grid_attach(GTK_GRID(container), btn_clear, 28, 6, 8, 1);

    page->page = container;
}

void lfsr_page_open_response(AppPage *page, GObject *original_object, GAsyncResult *res)
{
    struct LFSRAlgorithmData *data = page->data;

    GtkFileDialog *dialog = GTK_FILE_DIALOG(original_object);
    GError *error = NULL;
    GFile *file = gtk_file_dialog_open_finish(dialog, res, &error);

    if (NULL != file)
    {
        if (NULL != data->in_file)
            g_object_unref(G_OBJECT(data->in_file));

        data->in_file = file;
        set_info_for_action_row_by_name(data->in_file_selector, &data->in_file_selector_icon, data->in_file);
    } else
    {
        g_clear_error(&error);
    }
}

void lfsr_page_save_response(AppPage *page, GObject *original_object, GAsyncResult *res)
{
    struct LFSRAlgorithmData *data = page->data;

    GtkFileDialog *dialog = GTK_FILE_DIALOG(original_object);
    GError *error = NULL;
    GFile *file = gtk_file_dialog_save_finish(dialog, res, &error);

    if (NULL != file)
    {
        if (NULL != data->out_file)
            g_object_unref(G_OBJECT(data->out_file));

        data->out_file = file;
        set_info_for_action_row_by_name(data->out_file_selector, &data->out_file_selector_icon, data->out_file);
    } else
    {
        g_clear_error(&error);
    }
}

void lfsr_page_free(struct AppPage *page)
{

}