#include "toplevel.hpp"

Toplevel::Toplevel(Server *parent, wlr_xdg_toplevel *xdg_toplevel)
{
	std::cout<<"New toplevel"<<std::endl;
	this->server = parent;
	this->wlroots_xdg_toplevel = xdg_toplevel;

	wlr_xdg_surface *wlroots_xdg_surface = xdg_toplevel->base;
	wlr_surface *wlroots_surface = wlroots_xdg_surface->surface;

	this->map_listener.notify = [](wl_listener *listener, void *data)
	{
		Toplevel *self = container_of(listener, Toplevel, map_listener);
		std::cerr<<"map"<<std::endl;
		self->ready = true;
		self->focus_kbd();
	};
	wl_signal_add(&wlroots_surface->events.map, &this->map_listener);

	this->commit_listener.notify = [](wl_listener *listener, void *data)
	{
		Toplevel *self = container_of(listener, Toplevel, commit_listener);
		std::cerr<<"commit"<<std::endl;
		if (self->wlroots_xdg_toplevel->base->initial_commit)
		{
			wlr_xdg_toplevel_set_size(self->wlroots_xdg_toplevel, 0, 0);
		}
	};
	wl_signal_add(&wlroots_surface->events.commit, &this->commit_listener);
}

void Toplevel::focus_kbd()
{
	auto wlroots_seat = server->seat;
	wlr_surface *old_surface = wlroots_seat->keyboard_state.focused_surface;
	wlr_surface *new_surface = this->wlroots_xdg_toplevel->base->surface;
	if(old_surface == new_surface)
	{
		std::cerr<<"Already focused"<<std::endl;
		return;
	}
	if(old_surface != nullptr)
	{
		// un-focus old toplevel
		auto old_toplevel = wlr_xdg_toplevel_try_from_wlr_surface(old_surface);
		if(old_toplevel == nullptr)
		{
			std::cerr<<"wlr_xdg_toplevel_try_from_wlr_surface failed"<<std::endl;
		}
		else
		{
			wlr_xdg_toplevel_set_activated(old_toplevel, false);
		}
	}
	// focus self
	wlr_xdg_toplevel_set_activated(this->wlroots_xdg_toplevel, true);
	wlr_keyboard *kbd = wlr_seat_get_keyboard(server->seat);
	if(kbd != nullptr)
	{
		wlr_seat_keyboard_notify_enter(server->seat,
									   new_surface,
									   kbd->keycodes,
									   kbd->num_keycodes,
									   &kbd->modifiers);
	}
}
