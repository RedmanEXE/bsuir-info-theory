//
// Created by REXE on 25.02.26.
//

#include "independent_wchar.h"

wchar_t *utf8_to_wchar(const gchar *utf8_str, long in_len, long *out_len) {
    if (!utf8_str) {
        return NULL;
    }

#ifdef G_OS_WIN32
    return (wchar_t *)g_utf8_to_utf16(utf8_str, in_len, NULL, out_len, NULL);
#else
    return (wchar_t *)g_utf8_to_ucs4_fast(utf8_str, in_len, out_len);
#endif
}

gchar *wchar_to_utf8(const wchar_t *wchar_str, long in_len, long *out_len) {
    if (!wchar_str) {
        return NULL;
    }

#ifdef G_OS_WIN32
    return (gchar *)g_utf16_to_utf8((const gunichar2 *)wchar_str, in_len, NULL, out_len, NULL);
#else
    return (gchar *)g_ucs4_to_utf8((const gunichar *)wchar_str, in_len, NULL, out_len, NULL);
#endif
}
