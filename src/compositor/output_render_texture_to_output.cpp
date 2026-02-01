#include <upositor/compositor/output.hpp>

extern "C"
{
	#define static
	#include <wlr/render/interface.h>
	#include <wlr/render/pass.h>
	#include <wlr/render/swapchain.h>
	#undef static
}




void Output::render_texture_to_output(wlr_texture *texture)
{
	int width, height;
	wlr_output_effective_resolution(this->wlroots_output, &width, &height);

	wlr_renderer *renderer = this->server->renderer;
	if(renderer == nullptr) throw 1;

	wlr_output_state state = {0};
	wlr_output_state_init(&state);

	wlr_output_configure_primary_swapchain(this->wlroots_output, &state, &this->wlroots_output->swapchain);
	wlr_buffer *screen = wlr_swapchain_acquire(this->wlroots_output->swapchain);
	wlr_output_state_set_buffer(&state, screen);

	wlr_render_pass *pass = wlr_renderer_begin_buffer_pass(renderer, screen, nullptr);

	wlr_buffer_unlock(screen);

	wlr_render_texture_options options = {0};
	struct wlr_box box =
	{
		.x = 0,
		.y = 0,
		.width = width,
		.height = height,
	};
	options.dst_box = box;
	options.texture = texture;
	wlr_render_pass_add_texture(pass, &options);

	wlr_render_pass_submit(pass);


	if(wlr_output_test_state(this->wlroots_output, &state) == false) throw 1;

	if(wlr_output_commit_state(this->wlroots_output, &state) == false) throw 1;

	wlr_output_state_finish(&state);
}
