#include <upositor/compositor/output.hpp>
extern "C"
{
	#include <drm_fourcc.h>
	#include <sys/mman.h>
	#define static
	#include <wlr/render/drm_format_set.h>
	#include <wlr/render/wlr_texture.h>
	#undef static
}


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

	this->server->render_width = render_width;
	this->server->render_height = render_height;

	this->interpolator.new_size(render_width, render_height);

	// don't touch output, render to an offscreen buffer
	const wlr_drm_format *target_format = wlr_drm_format_set_get(wlr_output_get_primary_formats(this->wlroots_output, WLR_BUFFER_CAP_DMABUF), DRM_FORMAT_XRGB8888);
	wlr_buffer *intermediate = wlr_allocator_create_buffer(this->server->allocator, render_width, render_height, target_format);
	//std::cout<<"Locks: "<<intermediate->n_locks<<std::endl;
	this->render_surfaces_to_buffer(intermediate);

	// tamper with the buffer
	wlr_texture *intermediate_texture = wlr_texture_from_buffer(renderer, intermediate);
	this->interpolator.read(intermediate_texture);
	wlr_texture_destroy(intermediate_texture); // this would unlock the intermediate buffer automatically
	wlr_buffer_drop(intermediate);
	//std::cout<<intermediate->n_locks<<std::endl;
	this->interpolator.execute();
	wlr_texture *target_texture = this->interpolator.write();

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
