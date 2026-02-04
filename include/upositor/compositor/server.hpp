#pragma once

#include <type_traits>
#include <unordered_set>

#define WLR_USE_UNSTABLE

extern "C"
{
	#include <wayland/wayland-server-core.h>
	// wlroots: we do not support C++, because fuck you
	// https://gitlab.freedesktop.org/wlroots/wlroots/-/issues/682
	#define static
	#include <wlr/backend.h>
	#include <wlr/render/allocator.h>
	#include <wlr/render/wlr_renderer.h>
	#include <wlr/types/wlr_compositor.h>
	#include <wlr/types/wlr_cursor.h>
	#include <wlr/types/wlr_data_device.h>
	#include <wlr/types/wlr_xdg_shell.h>
	#undef static
}

class Output;
class Toplevel;

struct ServerC
{
	wl_listener compositor_new_surface_listener;
	wl_listener cursor_button_listener;
	wl_listener cursor_frame_listener;
	wl_listener cursor_motion_absolute_listener;
	wl_listener frame_listener;
	wl_listener keyboard_key_listener;
	wl_listener new_input_listener;
	wl_listener new_output_listener;
	wl_listener new_xdg_toplevel_listener;
};

static_assert(std::is_standard_layout<ServerC>::value, "non-standard layout: ServerC");

class Server final
{
public:
	ServerC c;
	wl_display *display;
	wl_event_loop *event_loop;
	//
	wlr_allocator *allocator;
	wlr_backend *backend;
	wlr_compositor *compositor;
	wlr_cursor *cursor;
	wlr_seat *seat;
	wlr_renderer *renderer;
	wlr_xdg_shell *xdg_shell;
	//
	int render_width;
	int render_height;
	std::unordered_set<Output*> outputs;
	std::unordered_set<Toplevel*> toplevels;
	//
	Server();
	~Server();
	void run();
	void add_keyboard(wlr_keyboard *keyboard);
};
