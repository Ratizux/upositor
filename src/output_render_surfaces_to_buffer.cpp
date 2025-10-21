#include "output.hpp"
#include "outputsurface.hpp"

void Output::render_surfaces_to_buffer(wlr_buffer *buffer)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	wlr_renderer *renderer = this->parent->renderer;
	if(renderer == nullptr) throw 1;

	struct wlr_render_pass *pass = wlr_renderer_begin_buffer_pass(renderer, buffer, nullptr);

	// indicator of a successful render+output
	{
		struct wlr_render_rect_options options = {0};
		options.box.width = 100;
		options.box.height = 100;
		options.color = { 0.3, 0.3, 0.3, 1.0 };
		wlr_render_pass_add_rect(pass, &options);
	}

	std::cout<<"Surfaces: "<<this->surfaces.size()<<std::endl;
	for(auto surface:this->surfaces)
	{
		std::cout<<"Render "<<(void*)surface<<std::endl;
		struct render_helper_data
		{
			struct timespec *time;
			struct wlr_render_pass *pass;
		};
		auto function = [](struct wlr_surface *wlroots_surface, int sx, int sy, void *data)
		{
			wlr_texture *texture = wlr_surface_get_texture(wlroots_surface);
			if(texture == nullptr) return;
			struct render_helper_data *helper_data = static_cast<struct render_helper_data*>(data);

			struct wlr_box box =
			{
				.x = 0,
				.y = 0,
				.width = wlroots_surface->current.width,
				.height = wlroots_surface->current.height,
			};
			wlr_render_texture_options options = {0};
			options.texture = texture;
			options.dst_box = box;
			wlr_render_pass_add_texture(helper_data->pass, &options);
			wlr_surface_send_frame_done(wlroots_surface, helper_data->time);
		};
		render_helper_data data =
		{
			.time = &now,
			.pass = pass,
		};
		function(surface->wlroots_surface, 0, 0, &data);
	}
	std::cout<<"wlr_render_pass_submit...";
	if(wlr_render_pass_submit(pass)) std::cout<<"OK"<<std::endl;
	else std::cout<<"Failed"<<std::endl;
}
