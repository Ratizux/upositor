#include <upositor/interpolator/lanczos.hpp>

LanczosScalerXRGB8888::LanczosScalerXRGB8888(int width, int height)
:
input(new uint32_t[width*height]),
output(new uint32_t[width*2*height*2]),
input_view(input, width, height),
output_view(output, width*2, height*2),
scaler(width, height, input_view, output_view)
{
	this->src_width = width;
	this->src_height = height;
	this->dest_width = width*2;
	this->dest_height = height*2;
}

LanczosScalerXRGB8888::~LanczosScalerXRGB8888()
{
	delete []this->input;
	delete []this->output;
}

void LanczosScalerXRGB8888::execute()
{
	this->scaler.execute();
}
