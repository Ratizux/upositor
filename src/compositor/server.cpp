#include <upositor/compositor/server.hpp>
#include <upositor/compositor/output.hpp>
#include <upositor/compositor/keyboard.hpp>
#include <upositor/compositor/toplevel.hpp>
#include <upositor/utils/container_of.hpp>
#include <cstddef>
#include <iostream>

extern "C"
{
	#define static
	#include <wayland/wayland-server-core.h>
	#include <wlr/types/wlr_seat.h>
	#include <wlr/types/wlr_subcompositor.h>
	#include <wlr/backend/wayland.h>
	#undef static
	#include <sys/mman.h>
}

Server::Server()
{
	this->display = wl_display_create();
	if(this->display == nullptr) throw 1;

	this->event_loop = wl_display_get_event_loop(this->display);
	if(this->event_loop == nullptr) throw 1;

	this->backend = wlr_backend_autocreate(this->event_loop, nullptr);
	if(this->backend == nullptr) throw 1;

	this->renderer = wlr_renderer_autocreate(this->backend);
	if(this->renderer == nullptr) throw 1;
	wlr_renderer_init_wl_display(this->renderer, this->display);

	this->allocator = wlr_allocator_autocreate(this->backend, this->renderer);
	if(this->allocator == nullptr) throw 1;

	this->compositor = wlr_compositor_create(this->display, 5, this->renderer);
	wlr_subcompositor_create(this->display);
	wlr_data_device_manager_create(this->display);

	// new_output handler
	this->c.new_output_listener.notify = [](wl_listener *listener, void *data)
	{
		Server *self = reinterpret_cast<Server*>(container_of(listener, ServerC, new_output_listener));
		wlr_output *wlroots_output = static_cast<struct wlr_output*>(data);

		std::cout<<"New output"<<std::endl;

		Output *new_output = new Output(self, wlroots_output);
		self->outputs.insert(new_output);
	};
	wl_signal_add(&this->backend->events.new_output, &this->c.new_output_listener);


	this->xdg_shell = wlr_xdg_shell_create(this->display, 3);
	// new_xdg_toplevel handler
	this->c.new_xdg_toplevel_listener.notify = [](wl_listener *listener, void *data)
	{
		Server *self = reinterpret_cast<Server*>(container_of(listener, ServerC, new_xdg_toplevel_listener));
		auto xdg_toplevel = static_cast<wlr_xdg_toplevel*>(data);
		auto target = new Toplevel(self, xdg_toplevel);
		self->toplevels.insert(target);
	};
	wl_signal_add(&this->xdg_shell->events.new_toplevel, &this->c.new_xdg_toplevel_listener);

	// cursor image and motion
	this->cursor=wlr_cursor_create();
	// TODO image
	this->c.cursor_motion_absolute_listener.notify = [](wl_listener *listener, void *data)
	{
		Server *self = reinterpret_cast<Server*>(container_of(listener, ServerC, cursor_motion_absolute_listener));
		auto event = static_cast<wlr_pointer_motion_absolute_event*>(data);
		//std::cout<<event->delta_x<<","<<event->delta_y<<std::endl;
		// motion
		// TODO auto detect related toplevel?
		if(self->toplevels.size() == 0)
		{
			wlr_seat_pointer_clear_focus(self->seat);
			return;
		}
		Toplevel *active_toplevel = *self->toplevels.begin();
		wlr_surface *active_surface = active_toplevel->wlroots_xdg_toplevel->base->surface;

		int w = self->render_width;
		int h = self->render_height;
		wlr_seat_pointer_notify_enter(self->seat, active_surface, event->x*w, event->y*h);
		wlr_seat_pointer_notify_motion(self->seat, event->time_msec, event->x*w, event->y*h);
		std::cout<<"move"<<std::endl;
	};
	wl_signal_add(&this->cursor->events.motion_absolute, &this->c.cursor_motion_absolute_listener);

	this->c.cursor_button_listener.notify = [](wl_listener *listener, void *data)
	{
		Server *self = reinterpret_cast<Server*>(container_of(listener, ServerC, cursor_button_listener));
		auto event = static_cast<wlr_pointer_button_event*>(data);
		wlr_seat_pointer_notify_button(self->seat, event->time_msec, event->button, event->state);
	};
	wl_signal_add(&this->cursor->events.button, &this->c.cursor_button_listener);

	// TODO wlr_xcursor_manager

	this->c.new_input_listener.notify = [](wl_listener *listener, void *data)
	{
		Server *self = reinterpret_cast<Server*>(container_of(listener, ServerC, new_input_listener));
		auto device = static_cast<wlr_input_device*>(data);
		switch(device->type)
		{
			case WLR_INPUT_DEVICE_KEYBOARD:
				std::cout<<"WLR_INPUT_DEVICE_KEYBOARD"<<std::endl;
				// TODO prevent memory leak
				new Keyboard(self, device);
				break;
			case WLR_INPUT_DEVICE_POINTER:
				std::cout<<"WLR_INPUT_DEVICE_POINTER"<<std::endl;
				wlr_cursor_attach_input_device(self->cursor, device);
				break;
			default:
				std::cout<<"Others"<<std::endl;
		}
	};
	wl_signal_add(&this->backend->events.new_input, &this->c.new_input_listener);

	// seat setup
	this->seat = wlr_seat_create(this->display, "seat0");
	wlr_seat_set_capabilities(this->seat, WL_SEAT_CAPABILITY_POINTER|WL_SEAT_CAPABILITY_KEYBOARD);

	const char *socket = wl_display_add_socket_auto(this->display);
	std::cout<<"wayland display created as "<<socket<<std::endl;

	std::cout<<"Server Ready"<<std::endl;
}

Server::~Server()
{
	wl_display_destroy(this->display);
	std::cout<<"Server destroyed"<<std::endl;
}

void Server::run()
{
	if(!wlr_backend_start(this->backend))
	{
		std::cout<<"Failed to start backend"<<std::endl;
		return;
	}
	wl_display_run(this->display);
	return;
}
