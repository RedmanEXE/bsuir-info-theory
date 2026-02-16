cmake_minimum_required(VERSION 4.1)
project("Info Theory" C)

set(CMAKE_C_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(Adwaita REQUIRED IMPORTED_TARGET libadwaita-1)

add_executable(Task1
        "Task 1/main.c"
        "Task 1/pages/page_rotation_matrix.c"
        "Task 1/pages/page_vigenere_algorithm.c"
        "GTK Tools/string_matrix/string_matrix.c"
        "GTK Tools/entry_deco/entry_deco.c"
)
target_link_libraries(Task1
        PRIVATE
        PkgConfig::GTK4
        PkgConfig::Adwaita
)
