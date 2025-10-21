#include "outputsurface.hpp"

std::unordered_map<wl_listener*, OutputSurface*> OutputSurface::listener_outputsurface_map;

OutputSurface::OutputSurface(Output *parent, struct wlr_surface *surface)
{
	this->parent = parent;
	this->wlroots_surface = surface;
	this->buffer = nullptr;

	this->commit_listener = new wl_listener;
	this->listener_outputsurface_map[commit_listener] = this;

	this->commit_listener->notify = [](wl_listener *listener, void *data)
	{
		OutputSurface *surface = OutputSurface::listener_outputsurface_map[listener];
		wlr_xdg_toplevel *xdg_toplevel = wlr_xdg_toplevel_try_from_wlr_surface(surface->wlroots_surface);
		if (xdg_toplevel != nullptr && xdg_toplevel->base->initial_commit)
		{
			wlr_xdg_toplevel_set_size(xdg_toplevel, 0, 0);
		}
		struct wlr_buffer *buffer = nullptr;
		if (surface->wlroots_surface->buffer != nullptr)
		{
			buffer = wlr_buffer_lock(&surface->wlroots_surface->buffer->base);
		}
		wlr_buffer_unlock(surface->buffer);
		surface->buffer = buffer;
	};
	wl_signal_add(&this->wlroots_surface->events.commit, this->commit_listener);
}

OutputSurface::~OutputSurface()
{
	delete this->commit_listener;
}
