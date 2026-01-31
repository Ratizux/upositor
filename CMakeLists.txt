cmake_minimum_required(VERSION 3.14)

project(Upositor)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

find_package(PkgConfig REQUIRED)

# TODO allow custom prefix
# set(ENV{PKG_CONFIG_PATH} "prefix/usr/lib64/pkgconfig:prefix/usr/share/pkgconfig")

pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET
    wayland-client
    wayland-server
)

pkg_check_modules(WLROOTS REQUIRED IMPORTED_TARGET
    wlroots-0.19
)

add_executable(Main
    src/keyboard.cpp
    src/main.cpp
    src/output.cpp
    src/output_frame_handler.cpp
    src/output_frame_render.cpp
    src/output_render_surfaces_to_buffer.cpp
    src/output_render_texture_to_output.cpp
    src/outputsurface.cpp
    src/scaler.cpp
    src/server.cpp
    src/toplevel.cpp
)

target_include_directories(Main PRIVATE
    ${WLROOTS_INCLUDE_DIRS}
    protocol
)
target_link_libraries(Main PRIVATE PkgConfig::WAYLAND PkgConfig::WLROOTS)
target_compile_options(Main PRIVATE
    #-fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer
    -Wall
    #-O2
)
target_link_options(Main PRIVATE
    #-fsanitize=undefined -fsanitize=address
)
