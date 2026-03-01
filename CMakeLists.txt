cmake_minimum_required(VERSION 3.14)

project(Upositor)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

find_package(PkgConfig REQUIRED)

# TODO allow custom prefix
# set(ENV{PKG_CONFIG_PATH} "prefix/usr/lib64/pkgconfig:prefix/usr/share/pkgconfig")
# set(CMAKE_PREFIX_PATH "prefix/usr/")

option(UPOSITOR_BUILD_COMPOSITOR "Build compositor" True)
option(UPOSITOR_BUILD_DEMOS "Build demos" False)
option(UPOSITOR_ENABLE_NN "Enable neural network features" False)

add_library(upositor_base INTERFACE)
target_include_directories(upositor_base INTERFACE
    ${PROJECT_SOURCE_DIR}/include
)

add_library(upositor_interpolator
    src/interpolator/lanczos.cpp
    src/interpolator/lanczos_xrgb8888.cpp
    src/interpolator/nn_xrgb8888.cpp
)

target_link_libraries(upositor_interpolator PUBLIC
    upositor_base
)

target_compile_options(upositor_interpolator PRIVATE
    -Wall -O3
)

if(UPOSITOR_ENABLE_NN)
    pkg_check_modules(NCNN REQUIRED IMPORTED_TARGET
        ncnn
    )
    find_package(OpenMP REQUIRED)
    find_package(glslang REQUIRED)

    add_library(upositor_nn INTERFACE)
    target_link_libraries(upositor_nn INTERFACE
        PkgConfig::NCNN
        OpenMP::OpenMP_CXX
    )
    add_definitions(-DUPOSITOR_ENABLE_NN)
    target_compile_options(upositor_nn INTERFACE
        ${OpenMP_CXX_FLAGS}
    )

    target_link_libraries(upositor_interpolator PUBLIC
        upositor_nn
        glslang::glslang
    )
endif()

if(UPOSITOR_BUILD_COMPOSITOR)
    pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET
        wayland-client
        wayland-server
        xkbcommon
    )

    pkg_check_modules(WLROOTS REQUIRED IMPORTED_TARGET
        wlroots-0.19
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
        protocol
    )

    target_link_libraries(upositor_compositor PUBLIC
        PkgConfig::WAYLAND
        PkgConfig::WLROOTS
        upositor_base
    )

    target_compile_options(upositor_compositor PRIVATE
        -Wall
    )

    add_executable(Main
        src/main.cpp
    )

    target_link_libraries(Main PRIVATE upositor_compositor upositor_interpolator)

    target_compile_options(Main PRIVATE
        # -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer
        -Wall
        #-O2
    )
    target_link_options(Main PRIVATE
        # -fsanitize=undefined -fsanitize=address
    )
endif()

if(UPOSITOR_BUILD_DEMOS)
    pkg_check_modules(PNG REQUIRED IMPORTED_TARGET
        libpng
    )

    add_executable(InterpolatorDemo
        src/interpolator_demos.cpp
    )

    target_link_libraries(InterpolatorDemo PRIVATE
        PkgConfig::PNG
        upositor_interpolator
    )

    target_compile_options(InterpolatorDemo PRIVATE
        -O3
    )
    )
endif()
