#pragma once

#include "output.hpp"

class OutputSurface
{
public:
	Output *parent;
	struct wlr_surface *wlroots_surface;
	struct wlr_buffer *buffer;

	wl_listener *commit_listener;
	static std::unordered_map<wl_listener*, OutputSurface*> listener_outputsurface_map;

	OutputSurface(Output *parent, struct wlr_surface *surface);
	~OutputSurface();
};
