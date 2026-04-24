cmake_minimum_required(VERSION 3.21)
project("Info Theory" C)

set(CMAKE_C_STANDARD 17)

option(PACK_AS_APP "Build bundled app for selected platform" OFF)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum OS X deployment version" FORCE)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(Adwaita REQUIRED IMPORTED_TARGET libadwaita-1)

function(add_task TARGET_NAME APP_NAME)
    add_executable(${TARGET_NAME} ${ARGN})
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        PkgConfig::GTK4
        PkgConfig::Adwaita
    )

    if(APPLE AND PACK_AS_APP)
        execute_process(
            COMMAND xcrun -find vtool
            OUTPUT_VARIABLE VTOOL_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if(NOT VTOOL_PATH)
            find_program(VTOOL_PATH vtool)
        endif()

        target_compile_definitions(${TARGET_NAME} PRIVATE IS_MAC_BUNDLE=1)

        set(MAC_ICON "${CMAKE_SOURCE_DIR}/Extras/MacOS/AppIcon.icns")
        set(MAC_ICON_ADAPTIVE "${CMAKE_SOURCE_DIR}/Extras/MacOS/Assets.car")

        set_target_properties(${TARGET_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "dev.rexe.thinfo.${TARGET_NAME}"
            MACOSX_BUNDLE_BUNDLE_NAME "${APP_NAME}"
            MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
            MACOSX_BUNDLE_ICON_FILE "AppIcon"
        )

        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMENT "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>"

            COMMAND ${VTOOL_PATH} -set-build-version macos 11.0 26.0 -replace -output "$<TARGET_FILE:${TARGET_NAME}>" "$<TARGET_FILE:${TARGET_NAME}>"
            COMMAND dylibbundler -od -b
            -x "$<TARGET_FILE:${TARGET_NAME}>"
            -d "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Frameworks/"
            -p "@executable_path/../Frameworks/"

            COMMAND find "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Frameworks" -name "*.dylib" -exec ${VTOOL_PATH} -set-build-version macos 11.0 11.0 -replace -output {} {} "\;"

            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Resources/share/glib-2.0/schemas"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Resources/share/icons/Adwaita"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Resources/share/icons/hicolor/"
            COMMAND cp ${CMAKE_PREFIX_PATH}/share/glib-2.0/schemas/org.gtk.gtk4.Settings.* "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Resources/share/glib-2.0/schemas/"
            COMMAND glib-compile-schemas "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Resources/share/glib-2.0/schemas/"

            COMMAND cp -RL ${CMAKE_PREFIX_PATH}/share/icons/Adwaita/ "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Resources/share/icons/Adwaita/"
            COMMAND cp -RL ${CMAKE_PREFIX_PATH}/share/icons/hicolor/ "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Resources/share/icons/hicolor/"

            COMMAND gtk4-update-icon-cache -f -t "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Resources/share/icons/Adwaita"
            COMMAND gtk4-update-icon-cache -f -t "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Resources/share/icons/hicolor"

            COMMAND cp -L ${MAC_ICON} "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Resources/"
            COMMAND cp -L ${MAC_ICON_ADAPTIVE} "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Resources/"

            COMMAND codesign --force --deep --sign - "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>"

            COMMAND hdiutil create -volname "${APP_NAME}" -srcfolder "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>" -ov -format UDZO "${APP_NAME}.dmg"
        )
    elseif(WIN32 AND PACK_AS_APP)
        execute_process(
            COMMAND pkgconf --variable=prefix gtk4
            OUTPUT_VARIABLE GTK_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        file(TO_CMAKE_PATH "${GTK_PREFIX}" GTK_PREFIX)

        set(WINDOWS_EXTRAS_DIR "${CMAKE_SOURCE_DIR}/Extras/Windows/${APP_NAME}")
        set(BUNDLE_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-cache")
        set(SFX_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-bundle.exe")
        set(APP_ARCHIVE "${CMAKE_CURRENT_BINARY_DIR}/app_${TARGET_NAME}.7z")

        file(TO_NATIVE_PATH "${WINDOWS_EXTRAS_DIR}/7zsd.sfx" NATIVE_SFX_MODULE)
        file(TO_NATIVE_PATH "${WINDOWS_EXTRAS_DIR}/config.txt" NATIVE_SFX_CONFIG)
        file(TO_NATIVE_PATH "${APP_ARCHIVE}" NATIVE_APP_7Z)
        file(TO_NATIVE_PATH "${SFX_OUTPUT}" NATIVE_SFX_OUTPUT)

        set(BUILD_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/build_sfx_${TARGET_NAME}.sh")
        file(WRITE "${BUILD_SCRIPT}"
            "#!/bin/bash
            set -e

            TARGET_FILE=$(cygpath -u \"$1\")
            BUNDLE_DIR=$(cygpath -u \"$2\")
            APP_ARCHIVE=$(cygpath -u \"$3\")
            PREFIX=$(cygpath -u \"$4\")
            SFX_MODULE=$(cygpath -u \"$5\")
            SFX_CONFIG=$(cygpath -u \"$6\")
            SFX_OUTPUT=$(cygpath -u \"$7\")

            rm -rf \"$BUNDLE_DIR\"
            mkdir -p \"$BUNDLE_DIR/share/glib-2.0/schemas\"
            mkdir -p \"$BUNDLE_DIR/share/icons\"
            mkdir -p \"$BUNDLE_DIR/lib/gdk-pixbuf-2.0\"

            cp \"$TARGET_FILE\" \"$BUNDLE_DIR/\"
            ldd \"$TARGET_FILE\" | grep -i '\\.dll' | awk '{print $3}' | grep -v -i '^/c/windows' | xargs -I {} cp \"{}\" \"$BUNDLE_DIR/\"
            cp \"$PREFIX/bin/vulkan-1.dll\" \"$BUNDLE_DIR/\" 2>/dev/null || true

            cp \"$PREFIX\"/share/glib-2.0/schemas/org.gtk.gtk4.*.gschema.xml \"$BUNDLE_DIR/share/glib-2.0/schemas/\"
            glib-compile-schemas \"$BUNDLE_DIR/share/glib-2.0/schemas/\"

            cp -r \"$PREFIX\"/share/icons/Adwaita \"$BUNDLE_DIR/share/icons/\"
            cp -r \"$PREFIX\"/share/icons/hicolor \"$BUNDLE_DIR/share/icons/\"
            cp -r \"$PREFIX\"/lib/gdk-pixbuf-2.0/* \"$BUNDLE_DIR/lib/gdk-pixbuf-2.0/\" 2>/dev/null || true

            mkdir -p \"$BUNDLE_DIR/lib/gtk-4.0\"
            cp -r \"$PREFIX\"/lib/gtk-4.0/* \"$BUNDLE_DIR/lib/gtk-4.0/\" 2>/dev/null || true

            rm -f \"$APP_ARCHIVE\"
            cd \"$BUNDLE_DIR\"
            7z a -t7z -mx=9 \"$APP_ARCHIVE\" * > /dev/null

            cat \"$SFX_MODULE\" \"$SFX_CONFIG\" \"$APP_ARCHIVE\" > \"$SFX_OUTPUT\"
        ")

        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND bash "${BUILD_SCRIPT}"
                "$<TARGET_FILE:${TARGET_NAME}>"
                "${BUNDLE_DIR}"
                "${APP_ARCHIVE}"
                "${GTK_PREFIX}"
                "${WINDOWS_EXTRAS_DIR}/7zsd.sfx"
                "${WINDOWS_EXTRAS_DIR}/config.txt"
                "${SFX_OUTPUT}"

            VERBATIM
        )
    endif()
endfunction()

add_task(Task1 "Task 1"
    "Build Utils/platform_build.c"
    "Task 1/main.c"
    "Task 1/pages/page_rotation_matrix.c"
    "Task 1/pages/page_vigenere_algorithm.c"
    "GTK Tools/string_matrix/string_matrix.c"
    "GTK Tools/entry_deco/entry_deco.c"
    "GTK Tools/independent_wchar/independent_wchar.c"
)

add_task(Task2 "Task 2"
    "Build Utils/platform_build.c"
    "Task 2/main.c"
    "Task 2/pages/page_lfsr.c"
    "GTK Tools/entry_deco/entry_deco.c"
    "GTK Tools/independent_wchar/independent_wchar.c"
    "GTK Tools/file_selector_deco/selector_ext_icons.c"
)

add_task(Task3 "Task 3"
    "Build Utils/platform_build.c"
    "Task 3/main.c"
    "Task 3/pages/page_rabin.c"
    "GTK Tools/entry_deco/entry_deco.c"
    "GTK Tools/independent_wchar/independent_wchar.c"
    "GTK Tools/file_selector_deco/selector_ext_icons.c"
)