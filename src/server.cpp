#include "server.hpp"
#include "output.hpp"
#include "keyboard.hpp"
#include "toplevel.hpp"
#include <cstddef>

extern "C"
{
	#define static
	#include <wayland/wayland-server-core.h>
	#include <wlr/types/wlr_seat.h>
	#include <wlr/backend/wayland.h>
	#undef static
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
	this->new_output_listener.notify = [](wl_listener *listener, void *data)
	{
		Server *server = container_of(listener, Server, new_output_listener);
		wlr_output *wlroots_output = static_cast<struct wlr_output*>(data);

		std::cout<<"New output"<<std::endl;

		Output *new_output = new Output(server, wlroots_output);
		server->outputs.insert(new_output);
	};
	wl_signal_add(&this->backend->events.new_output, &this->new_output_listener);


	this->xdg_shell = wlr_xdg_shell_create(this->display, 3);
	// new_xdg_toplevel handler
	this->new_xdg_toplevel_listener.notify = [](wl_listener *listener, void *data)
	{
		// TODO
		Server *server = container_of(listener, Server, new_xdg_toplevel_listener);
		auto xdg_toplevel = static_cast<wlr_xdg_toplevel*>(data);
		auto target = new Toplevel(server, xdg_toplevel);
		server->toplevels.insert(target);
	};
	wl_signal_add(&this->xdg_shell->events.new_toplevel, &this->new_xdg_toplevel_listener);

	// cursor image and motion
	this->cursor=wlr_cursor_create();
	// TODO image
	this->cursor_motion_listener.notify = [](wl_listener *listener, void *data)
	{
		Server *server = container_of(listener, Server, cursor_motion_listener);
		auto event = static_cast<wlr_pointer_motion_event*>(data);
		//std::cout<<event->delta_x<<","<<event->delta_y<<std::endl;
	};
	wl_signal_add(&this->cursor->events.motion, &this->cursor_motion_listener);

	// TODO wlr_xcursor_manager

	this->new_input_listener.notify = [](wl_listener *listener, void *data)
	{
		Server *server = container_of(listener, Server, new_input_listener);
		auto device = static_cast<wlr_input_device*>(data);
		switch(device->type)
		{
			case WLR_INPUT_DEVICE_KEYBOARD:
				std::cout<<"WLR_INPUT_DEVICE_KEYBOARD"<<std::endl;
				// TODO prevent memory leak
				new Keyboard(server, device);
				break;
			case WLR_INPUT_DEVICE_POINTER:
				std::cout<<"WLR_INPUT_DEVICE_POINTER"<<std::endl;
				wlr_cursor_attach_input_device(server->cursor, device);
				break;
			default:
				std::cout<<"Others"<<std::endl;
		}
	};
	wl_signal_add(&this->backend->events.new_input, &this->new_input_listener);

	// seat setup
	this->seat = wlr_seat_create(this->display, "seat0");
	wlr_seat_set_capabilities(this->seat, WL_SEAT_CAPABILITY_POINTER|WL_SEAT_CAPABILITY_KEYBOARD);

	const char *socket = wl_display_add_socket_auto(this->display);
	std::cout<<"wayland display created as "<<socket<<std::endl;

	std::cout<<"Server Ready"<<std::endl;

	// TODO eliminate dead code
	/*
	// xdg_new_surface handler
	listener_server_map[&this->compositor_new_surface_listener] = this;
	this->compositor_new_surface_listener.notify = [](wl_listener *listener, void *data)
	{
		std::cout<<"New surface"<<std::endl;
		Server *server = Server::listener_server_map[listener];

		wlr_surface *wlroots_surface = static_cast<struct wlr_surface*>(data);

		for(auto output:server->outputs)
		{
			OutputSurface *new_surface = new OutputSurface(output, wlroots_surface);
			output->surfaces.insert(new_surface);
		}
	};

	wl_signal_add(&this->compositor->events.new_surface, &this->compositor_new_surface_listener);
	*/
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
