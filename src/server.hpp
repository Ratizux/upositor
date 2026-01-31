#pragma once

#include <wayland/wayland-server-core.h>
#define WLR_USE_UNSTABLE

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <ctime>

extern "C"
{
	#include <wayland/wayland-client.h>
	#include <wayland/wayland-server.h>
	#include <wayland-server-core.h>
	#include <drm_fourcc.h>
	// wlroots: we do not support C++, because fuck you
	// https://gitlab.freedesktop.org/wlroots/wlroots/-/issues/682
	#define static
	#include <wlr/backend.h>
	#include <wlr/render/allocator.h>
	#include <wlr/render/interface.h>
	#include <wlr/render/pass.h>
	#include <wlr/render/swapchain.h>
	#include <wlr/render/wlr_renderer.h>
	#include <wlr/render/wlr_texture.h>
	#include <wlr/types/wlr_buffer.h>
	#include <wlr/types/wlr_compositor.h>
	#include <wlr/types/wlr_cursor.h>
	#include <wlr/types/wlr_data_device.h>
	#include <wlr/types/wlr_output.h>
	#include <wlr/types/wlr_output_layout.h>
	#include <wlr/types/wlr_scene.h>
	#include <wlr/types/wlr_subcompositor.h>
	#include <wlr/types/wlr_xdg_shell.h>
	#include <wlr/util/log.h>
	#undef static
	#include <sys/mman.h>
}

#include <cstddef>

#define container_of(ptr, type, member) ({ \
const typeof( ((type *)0)->member ) *__mptr = (ptr); \
(type *)( (char *)__mptr - offsetof(type,member) );})

class Output;
class Toplevel;

class Server final
{
public:
	wl_display *display;
	wl_event_loop *event_loop;
	wl_listener new_output_listener;
	wl_listener compositor_new_surface_listener;
	wl_listener new_xdg_toplevel_listener;
	wl_listener frame_listener;
	wl_listener new_input_listener;
	wl_listener cursor_motion_listener;
	wl_listener keyboard_key_listener;
	//
	wlr_allocator *allocator;
	wlr_backend *backend;
	wlr_compositor *compositor;
	wlr_cursor *cursor;
	wlr_scene *scene;
	wlr_scene_output_layout *scene_output_layout;
	wlr_seat *seat;
	wlr_output_layout *output_layout;
	wlr_renderer *renderer;
	wlr_xdg_shell *xdg_shell;
	//
	std::unordered_set<Toplevel*> toplevels;
	std::unordered_set<Output*> outputs;
	//
	Server();
	~Server();
	void run();
	void add_keyboard(wlr_keyboard *keyboard);
};
