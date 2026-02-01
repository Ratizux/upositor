#include "output.hpp"
#include <sys/mman.h>
extern "C"
{
	#define static
	#include <wlr/render/wlr_texture.h>
	#undef static
}
#include "scaler.hpp"

void Output::frame_render()
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	wlr_renderer *renderer = this->server->renderer;
	if(renderer == nullptr) throw 1;
	int output_width, output_height;
	wlr_output_effective_resolution(this->wlroots_output, &output_width, &output_height);
	if((output_width&1) != 0)
	{
		// width could not be divided by 2
		output_width--;
	}
	if((output_height&1) != 0)
	{
		// width could not be divided by 2
		output_height--;
	}
	int render_width, render_height;
	render_width = output_width/2;
	render_height = output_height/2;

	const wlr_drm_format *target_format = wlr_drm_format_set_get(wlr_output_get_primary_formats(this->wlroots_output, WLR_BUFFER_CAP_DMABUF), DRM_FORMAT_ABGR8888);

	wlr_buffer *intermediate = wlr_allocator_create_buffer(this->server->allocator, render_width, render_height, target_format);
	//std::cout<<"Locks: "<<intermediate->n_locks<<std::endl;
	//std::cout<<intermediate->width<<"x"<<intermediate->height<<std::endl;

	// don't touch output, render to an offscreen buffer
	this->render_surfaces_to_buffer(intermediate);

	// tamper with the buffer
	Scaler scaler(render_width, render_height);
	wlr_texture *intermediate_texture = wlr_texture_from_buffer(renderer, intermediate);
	wlr_box read_box =
	{
		.x = 0,
		.y = 0,
		.width = render_width,
		.height = render_height,
	};
	wlr_texture_read_pixels_options read_option =
	{
		.data = scaler.input,
		.format = DRM_FORMAT_ABGR8888,
		.stride = (unsigned)4*render_width,
		.dst_x = 0,
		.dst_y = 0,
		.src_box = read_box,
	};
	wlr_texture_read_pixels(intermediate_texture, &read_option);
	scaler.scale_scale2x();

	//wlr_texture_from_pixels;
	wlr_texture_destroy(intermediate_texture); // this would unlock the intermediate buffer automatically
	wlr_buffer_drop(intermediate);
	//std::cout<<intermediate->n_locks<<std::endl;

	wlr_texture *target_texture = wlr_texture_from_pixels(this->server->renderer, DRM_FORMAT_ABGR8888, (unsigned)4*output_width, output_width, output_height, scaler.output);

	// OK, create texture from buffer and render to screen buffer
	this->render_texture_to_output(target_texture);

	wlr_texture_destroy(target_texture);
}

/*wlr_dmabuf_attributes attrs = {0};
	if(wlr_buffer_get_dmabuf(intermediate, &attrs))
	{
		//std::cout<<"Get DMABUF OK"<<std::endl;
		//std::cout<<(attrs.format == DRM_FORMAT_ABGR8888)<<std::endl;
		//std::cout<<attrs.n_planes<<std::endl;
		int size = attrs.width*attrs.height*4;
		//std::cout<<attrs.width<<"x"<<attrs.height<<std::endl;
		//std::cout<<attrs.stride[0]<<std::endl;
		//std::cout<<IS_AMD_FMT_MOD(attrs.modifier)<<std::endl;
		//std::cout<<size<<std::endl;

		uint32_t *data = nullptr;
		data = static_cast<uint32_t*>(mmap(nullptr, size, PROT_READ, MAP_SHARED, attrs.fd[0], attrs.offset[0]));
		if(data != MAP_FAILED)
		{
			//std::cout<<"mmap OK"<<std::endl;
			munmap(data, size);
		}
		else
		{
			//std::cout<<"mmap failed: "<<strerror(errno)<<std::endl;
		}
	}*/

	//std::cout<<"Locks: "<<intermediate->n_locks<<std::endl;
