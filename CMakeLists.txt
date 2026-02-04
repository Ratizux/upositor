cmake_minimum_required(VERSION 3.14)

project(Upositor)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

find_package(PkgConfig REQUIRED)

# TODO allow custom prefix
# set(ENV{PKG_CONFIG_PATH} "prefix/usr/lib64/pkgconfig:prefix/usr/share/pkgconfig")

option(UPOSITOR_BUILD_DEMOS "Build demos" False)

pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET
    wayland-client
    wayland-server
    xkbcommon
)

pkg_check_modules(WLROOTS REQUIRED IMPORTED_TARGET
    wlroots-0.19
)

add_library(upositor_interpolator
    src/interpolator/lanczos.cpp
    src/interpolator/lanczos_xrgb8888.cpp
)

target_include_directories(upositor_interpolator PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(upositor_interpolator PRIVATE
    -Wall -O3
)

add_library(upositor_compositor
    src/compositor/keyboard.cpp
    src/compositor/interpolator.cpp
    src/compositor/output.cpp
    src/compositor/output_frame_handler.cpp
    src/compositor/output_frame_render.cpp
    src/compositor/output_render_surfaces_to_buffer.cpp
    src/compositor/output_render_texture_to_output.cpp
    src/compositor/server.cpp
    src/compositor/toplevel.cpp
)

target_include_directories(upositor_compositor PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${WLROOTS_INCLUDE_DIRS}
    protocol
)

target_compile_options(upositor_compositor PRIVATE
    -Wall
)

add_executable(Main
    src/main.cpp
)

target_include_directories(Main PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    protocol
)

target_link_libraries(Main PRIVATE PkgConfig::WAYLAND PkgConfig::WLROOTS upositor_compositor upositor_interpolator)

target_compile_options(Main PRIVATE
    # -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer
    -Wall
    #-O2
)
target_link_options(Main PRIVATE
    # -fsanitize=undefined -fsanitize=address
)

if(UPOSITOR_BUILD_DEMOS)
    pkg_check_modules(PNG REQUIRED IMPORTED_TARGET
        libpng
    )

    add_executable(InterpolatorDemo
        src/interpolator_demos.cpp
    )

    target_include_directories(InterpolatorDemo PRIVATE
        ${PROJECT_SOURCE_DIR}/include
    )

    target_link_libraries(InterpolatorDemo PRIVATE
        PkgConfig::PNG
        upositor_interpolator
    )

    target_compile_options(InterpolatorDemo PRIVATE
        -O3
    )
endif()
