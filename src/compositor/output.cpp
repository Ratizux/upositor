#include <upositor/compositor/output.hpp>
#include <upositor/compositor/server.hpp>
#include <upositor/utils/container_of.hpp>
#include <iostream>

Output::Output(Server *parent, struct wlr_output *wlr_output)
{
	this->server = parent;
	this->interpolator = Interpolator(this->server);
	this->wlroots_output = wlr_output;

	wlr_output_init_render(this->wlroots_output, this->server->allocator, this->server->renderer);

	struct wlr_output_state state;
	wlr_output_state_init(&state);
	wlr_output_state_set_enabled(&state, true);

	// modesetting
	struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
	if (mode != NULL)
	{
		wlr_output_state_set_mode(&state, mode);
	}

	/*
	wlr_output_layout_output *layout_output = wlr_output_layout_add_auto(parent->output_layout, wlr_output);
	struct wlr_scene_output *scene_output = wlr_scene_output_create(parent->scene, wlr_output);
	wlr_scene_output_layout_add_output(parent->scene_output_layout, layout_output, scene_output);
	*/

	this->destroy_listener.notify = [](wl_listener *listener, void *data)
	{
		Output *self=container_of(listener, Output, destroy_listener);
		self->server->outputs.erase(self);
		// To be implemented
	};
	wl_signal_add(&this->wlroots_output->events.destroy, &this->destroy_listener);

	this->frame_listener.notify = Output::frame_handler;
	wl_signal_add(&this->wlroots_output->events.frame, &this->frame_listener);

	this->request_state_listener.notify = [](wl_listener *listener, void *data)
	{
		// state such as output resolution
		Output *self=container_of(listener, Output, request_state_listener);
		auto *event = static_cast<wlr_output_event_request_state*>(data);
		wlr_output_commit_state(self->wlroots_output, event->state);
	};
	wl_signal_add(&this->wlroots_output->events.request_state, &this->request_state_listener);

	wlr_output_commit_state(wlr_output, &state);
	wlr_output_state_finish(&state);
	wlr_output_create_global(wlr_output, parent->display);
	std::cout<<"Output constructor OK"<<std::endl;
}

Output::~Output()
{
	//
}
