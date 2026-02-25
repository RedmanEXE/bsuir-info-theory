//
// Created by REXE on 25.02.26.
//

#ifndef INFO_THEORY_INDEPENDENT_WCHAR_H
#define INFO_THEORY_INDEPENDENT_WCHAR_H

#include <glib.h>

wchar_t *utf8_to_wchar(const gchar *utf8_str, long in_len, long *out_len);
gchar *wchar_to_utf8(const wchar_t *wchar_str, long in_len, long *out_len);

#endif //INFO_THEORY_INDEPENDENT_WCHAR_H
