#include "output.hpp"

void Output::frame_handler(struct wl_listener *listener, void *data)
{
	static struct timespec last;
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	static unsigned int ms=last.tv_sec*1000 + last.tv_nsec/1000000;
	static unsigned int frame_count = 0;
	unsigned int ms_now = now.tv_sec*1000 + now.tv_nsec/1000000;
	if(ms_now - ms > 1000)
	{
		ms = ms_now;
		std::cout<<frame_count<<std::endl;
		frame_count = 0;
	}
	frame_count++;
	/*
	if(frame_count%4 != 0)
	{
		std::cout<<"Skip"<<std::endl;
		Output *output=Output::listener_output_map[listener];
		wlr_output_state state = {0};
		wlr_output_state_init(&state);
		std::cout<<"wlr_output_test_state...";
		if(wlr_output_test_state(output->wlroots_output, &state)) std::cout<<"OK"<<std::endl;
		else std::cout<<"Failed"<<std::endl;

		std::cout<<"wlr_output_commit_state...";
		if(wlr_output_commit_state(output->wlroots_output, &state)) std::cout<<"OK"<<std::endl;
		else std::cout<<"Failed"<<std::endl;

		wlr_output_state_finish(&state);
		return;
	}*/

	static unsigned int render_count = 0;
	render_count++;
	if(render_count == 200) render_count = 0;
	Output *output=Output::listener_output_map[listener];
	// frame
	if(output->wlroots_output != data)
	{
		std::cout<<"output->wlroots_output != data, this should not happen"<<std::endl;
		return;
	}
	std::cout<<"vsync"<<std::endl;
	output->frame_render();
}
