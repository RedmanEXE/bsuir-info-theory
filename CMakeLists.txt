cmake_minimum_required(VERSION 3.21)
project("Info Theory" C)

set(CMAKE_C_STANDARD 17)

option(PACK_AS_APP "Build bundled app for selected platform" OFF)

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

                COMMAND dylibbundler -od -b
                -x "$<TARGET_FILE:${TARGET_NAME}>"
                -d "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Frameworks/"
                -p "@executable_path/../Frameworks/"

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

                COMMAND hdiutil create -volname "${APP_NAME}" -srcfolder "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>" -ov -format UDZO "${APP_NAME}.dmg"
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
