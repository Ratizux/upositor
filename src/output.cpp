#include "output.hpp"

std::unordered_map<wl_listener*, Output*> Output::listener_output_map;

Output::Output(Server *parent, struct wlr_output *wlr_output)
{
	this->parent=parent;
	this->wlroots_output=wlr_output;

	wlr_output_init_render(this->wlroots_output, this->parent->allocator, this->parent->renderer);

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

	this->destroy_listener = new wl_listener;
	this->frame_listener = new wl_listener;
	Output::listener_output_map[this->destroy_listener] = this;
	Output::listener_output_map[this->frame_listener] = this;

	this->destroy_listener->notify = [](wl_listener *listener, void *data)
	{
		Output *output=Output::listener_output_map[listener];
		output->parent->outputs.erase(output);
		//
		listener_output_map.erase(output->destroy_listener);
		listener_output_map.erase(output->frame_listener);
	};
	wl_signal_add(&this->wlroots_output->events.destroy, this->destroy_listener);

	this->frame_listener->notify = Output::frame_handler;
	wl_signal_add(&this->wlroots_output->events.frame, this->frame_listener);

	wlr_output_commit_state(wlr_output, &state);
	wlr_output_state_finish(&state);
	wlr_output_create_global(wlr_output, parent->display);
	std::cout<<"Output constructor OK"<<std::endl;
}

Output::~Output()
{
	delete this->destroy_listener;
	delete this->frame_listener;
}
