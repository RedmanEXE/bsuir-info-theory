//
// Created by REXE on 26.02.26.
//

#include <gtk/gtk.h>
#include <mach-o/dyld.h>
#include <stdlib.h>
#include <string.h>

void setup_env()
{
#if defined(__APPLE__) && defined(IS_MAC_BUNDLE)
    // Just some vars need setup (for GTK and Adwaita)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);

    if (_NSGetExecutablePath(path, &size) == 0) {
        char *last_slash = strrchr(path, '/');
        if (last_slash)
            *last_slash = '\0';

        char resource_path[PATH_MAX];
        snprintf(resource_path, sizeof(resource_path), "%s/../Resources", path);

        char schemas_path[PATH_MAX];
        snprintf(schemas_path, sizeof(schemas_path), "%s/share/glib-2.0/schemas", resource_path);
        g_setenv("GSETTINGS_SCHEMA_DIR", schemas_path, TRUE);

        char data_path[PATH_MAX];
        snprintf(data_path, sizeof(data_path), "%s/share", resource_path);
        g_setenv("XDG_DATA_DIRS", data_path, TRUE);
    }
#endif
}
