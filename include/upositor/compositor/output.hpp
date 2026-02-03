#pragma once

#include <upositor/compositor/server.hpp>
#include <upositor/compositor/interpolator.hpp>

class OutputSurface;

class Output
{
public:
	wl_listener destroy_listener;
	wl_listener frame_listener;
	wl_listener request_state_listener;
	//
	//static void destroy_handler(struct wl_listener *listener, void *data);

	static void frame_handler(struct wl_listener *listener, void *data);
	void frame_render();

	// Render any OutputSurface in this->surfaces to given buffer. The buffer can be either on-screen or off-screen
	void render_surfaces_to_buffer(wlr_buffer *buffer);

	// Render given texture to output
	void render_texture_to_output(wlr_texture *texture);
	//
	wlr_output *wlroots_output;
	//
	Server *server;
	Interpolator interpolator;
	Output(Server*,struct wlr_output*);
	~Output();
};
