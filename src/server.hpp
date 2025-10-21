#pragma once

#define WLR_USE_UNSTABLE

#include <iostream>
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

class Output;

class Server
{
public:
	wl_display *display;
	wl_event_loop *event_loop;
	wl_listener new_output_listener;
	wl_listener compositor_new_surface_listener;
	//wl_listener new_xdg_toplevel_listener;
	wl_listener frame_listener;
	//
	wlr_backend *backend;
	wlr_scene *scene;
	wlr_scene_output_layout *scene_output_layout;
	wlr_renderer *renderer;
	wlr_output_layout *output_layout;
	wlr_allocator *allocator;
	wlr_xdg_shell *xdg_shell;
	wlr_compositor *compositor;
	//
	std::unordered_set<Output*> outputs;
	//
	Server();
	~Server();
	void run();
	//
	static std::unordered_map<wl_listener*, Server*> listener_server_map;
};
