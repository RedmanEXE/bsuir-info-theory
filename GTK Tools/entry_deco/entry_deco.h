//
// Created by REXE on 15.02.26.
//

#ifndef INFO_THEORY_ENTRY_DECO_H
#define INFO_THEORY_ENTRY_DECO_H

#include <gtk/gtk.h>
#include <adwaita.h>

void EntryDeco_MarkEntryAsError(GtkEntry *entry, const char *tooltip_text);
void EntryDeco_MarkEntryAsWarning(GtkEntry *entry, const char *tooltip_text);
void EntryDeco_ClearDecorations(GtkEntry *entry);

void EntryDeco_MarkEntryRowAsError(AdwEntryRow *entry, const char *tooltip_text);
void EntryDeco_MarkEntryRowAsWarning(AdwEntryRow *entry, const char *tooltip_text);
void EntryDeco_ClearRowDecorations(AdwEntryRow *entry);

#endif //INFO_THEORY_ENTRY_DECO_H
