//
// Created by REXE on 15.02.26.
//

#include "entry_deco.h"

#define KEY_ERROR_ICON          "thinfo-error-icon"
#define KEY_WARNING_ICON        "thinfo-warning-icon"

static void internals_remove_status_icon(AdwEntryRow *entry, const char *key)
{
    GtkWidget *icon = g_object_get_data(G_OBJECT(entry), key);
    if(icon)
    {
        adw_entry_row_remove(entry, icon);
        g_object_set_data(G_OBJECT(entry), key, NULL);
    }
}

void EntryDeco_MarkEntryAsError(GtkEntry *entry, const char *tooltip_text)
{
    gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_SECONDARY, "dialog-error");
    gtk_entry_set_icon_tooltip_text(entry, GTK_ENTRY_ICON_SECONDARY, tooltip_text);
    gtk_widget_add_css_class(GTK_WIDGET(entry), "error");
    gtk_widget_remove_css_class(GTK_WIDGET(entry), "warning");
}
void EntryDeco_MarkEntryAsWarning(GtkEntry *entry, const char *tooltip_text)
{
    gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_SECONDARY, "dialog-warning");
    gtk_entry_set_icon_tooltip_text(entry, GTK_ENTRY_ICON_SECONDARY, tooltip_text);
    gtk_widget_add_css_class(GTK_WIDGET(entry), "warning");
    gtk_widget_remove_css_class(GTK_WIDGET(entry), "error");
}
void EntryDeco_ClearDecorations(GtkEntry *entry)
{
    gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_SECONDARY, "");
    gtk_entry_set_icon_tooltip_text(entry, GTK_ENTRY_ICON_SECONDARY, "");

    gtk_widget_remove_css_class(GTK_WIDGET(entry), "error");
    gtk_widget_remove_css_class(GTK_WIDGET(entry), "warning");
}

void EntryDeco_ClearRowDecorations(AdwEntryRow *entry)
{
    internals_remove_status_icon(entry, KEY_ERROR_ICON);
    internals_remove_status_icon(entry, KEY_WARNING_ICON);
    
    gtk_widget_remove_css_class(GTK_WIDGET(entry), "error");
    gtk_widget_remove_css_class(GTK_WIDGET(entry), "warning");
}

void EntryDeco_MarkEntryRowAsError(AdwEntryRow *entry, const char *tooltip_text)
{
    internals_remove_status_icon(entry, KEY_WARNING_ICON);
    gtk_widget_remove_css_class(GTK_WIDGET(entry), "warning");
    
    GtkWidget *icon = g_object_get_data(G_OBJECT(entry), KEY_ERROR_ICON);

    if (!icon)
    {
        icon = gtk_image_new_from_icon_name("dialog-error-symbolic");
        adw_entry_row_add_suffix(entry, icon);
        g_object_set_data(G_OBJECT(entry), KEY_ERROR_ICON, icon);
    }

    gtk_widget_set_tooltip_text(icon, tooltip_text);
    gtk_widget_add_css_class(GTK_WIDGET(entry), "error");
}

void EntryDeco_MarkEntryRowAsWarning(AdwEntryRow *entry, const char *tooltip_text)
{
    internals_remove_status_icon(entry, KEY_ERROR_ICON);
    gtk_widget_remove_css_class(GTK_WIDGET(entry), "error");

    GtkWidget *icon = g_object_get_data(G_OBJECT(entry), KEY_WARNING_ICON);

    if (!icon)
    {
        icon = gtk_image_new_from_icon_name("dialog-warning-symbolic");
        adw_entry_row_add_suffix(entry, icon);
        g_object_set_data(G_OBJECT(entry), KEY_WARNING_ICON, icon);
    }

    gtk_widget_set_tooltip_text(icon, tooltip_text);
    gtk_widget_add_css_class(GTK_WIDGET(entry), "warning");
}
