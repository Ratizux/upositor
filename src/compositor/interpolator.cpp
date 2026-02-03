#include <upositor/compositor/interpolator.hpp>
#include <cassert>

extern "C"
{
	#include <drm_fourcc.h>
	#define static
	#include <wlr/render/wlr_texture.h>
	#undef static
}

Interpolator::Interpolator()
{
	this->server = nullptr;

	this->scaler = nullptr;
}

Interpolator::Interpolator(Server *parent)
{
	this->server = parent;

	this->scaler = nullptr;
}

Interpolator::~Interpolator()
{
	if(this->scaler != nullptr)
	{
		delete this->scaler;
	}
}

void Interpolator::read(wlr_texture *in_texture)
{
	wlr_box read_box =
	{
		.x = 0,
		.y = 0,
		.width = this->src_width,
		.height = this->src_height,
	};

	wlr_texture_read_pixels_options read_option =
	{
		.data = this->scaler->input,
		.format = DRM_FORMAT_XRGB8888, // TODO use NV24/NV12 but wlroots gles2 does not support
		.stride = (unsigned)4*this->src_width,
		.dst_x = 0,
		.dst_y = 0,
		.src_box = read_box,
	};

	wlr_texture_read_pixels(in_texture, &read_option);
}

void Interpolator::execute()
{
	this->scaler->execute();
}

void Interpolator::new_size(int new_src_width, int new_src_height)
{
	int old_src_width = this->src_width;
	int old_src_height = this->src_height;
	if(old_src_height == new_src_height && old_src_width == new_src_width)
	{
		return;
	}

	this->src_width = new_src_width;
	this->src_height = new_src_height;
	this->dest_width = 2*this->src_width;
	this->dest_height = 2*this->src_height;

	// re-create scaler
	if(this->scaler != nullptr)
	{
		delete this->scaler;
	}
	this->scaler = new LanczosScaler(this->src_width, this->src_height);
}

wlr_texture* Interpolator::write()
{
	assert(this->server != nullptr);

	wlr_texture *out_texture = wlr_texture_from_pixels(this->server->renderer,
													   DRM_FORMAT_XRGB8888,
													   (unsigned)4*this->dest_width,
													   this->dest_width,
													   this->dest_height,
													   this->scaler->output);
	return out_texture;
}
