#include "server.hpp"
#include "output.hpp"
#include "outputsurface.hpp"

std::unordered_map<wl_listener*, Server*> Server::listener_server_map;

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
	//wlr_subcompositor_create(this->display);
	wlr_data_device_manager_create(this->display);

	this->output_layout = wlr_output_layout_create(this->display);

	// new_output handler
	listener_server_map[&this->new_output_listener] = this;
	this->new_output_listener.notify = [](wl_listener *listener, void *data)
	{
		Server *server = Server::listener_server_map[listener];
		wlr_output *wlroots_output = static_cast<struct wlr_output*>(data);

		std::cout<<"New output"<<std::endl;

		Output *new_output = new Output(server, wlroots_output);
		server->outputs.insert(new_output);
	};
	wl_signal_add(&this->backend->events.new_output, &this->new_output_listener);

	/*
	this->scene = wlr_scene_create();
	this->scene_output_layout = wlr_scene_attach_output_layout(this->scene, this->output_layout);
	*/

	this->xdg_shell = wlr_xdg_shell_create(this->display, 1);

	// xdg_new_surface handler
	listener_server_map[&this->compositor_new_surface_listener] = this;
	this->compositor_new_surface_listener.notify = [](wl_listener *listener, void *data)
	{
		std::cout<<"New surface"<<std::endl;
		Server *server = Server::listener_server_map[listener];

		wlr_surface *wlroots_surface = static_cast<struct wlr_surface*>(data);
		/*wlr_scene_tree *scene_tree = wlr_scene_xdg_surface_create(&server->scene->tree, xdg_surface->toplevel->base);
		xdg_surface->data = scene_tree;*/
		for(auto output:server->outputs)
		{
			OutputSurface *new_surface = new OutputSurface(output, wlroots_surface);
			output->surfaces.insert(new_surface);
		}
	};

	wl_signal_add(&this->compositor->events.new_surface, &this->compositor_new_surface_listener);

	/*
	// new_xdg_toplevel handler
	listener_server_map[&this->new_xdg_toplevel_listener] = this;
	this->new_xdg_toplevel_listener.notify = [](wl_listener *listener, void *data)
	{
		//
		throw "Not implemented!";
	};
	wl_signal_add(&this->xdg_shell->events.new_toplevel, &server.new_xdg_toplevel);
	server.new_xdg_popup.notify = server_new_xdg_popup;
	wl_signal_add(&server.xdg_shell->events.new_popup, &server.new_xdg_popup);
	*/

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
