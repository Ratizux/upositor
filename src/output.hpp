#pragma once

#include "server.hpp"

class OutputSurface;

class Output
{
public:
	wl_listener *destroy_listener;
	wl_listener *frame_listener;
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
	std::unordered_set<OutputSurface*> surfaces;
	//
	Server *parent;
	Output(Server*,struct wlr_output*);
	~Output();
	//
	static std::unordered_map<wl_listener*, Output*> listener_output_map;
};
