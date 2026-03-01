#include <upositor/interpolator/nn.hpp>

#ifdef UPOSITOR_ENABLE_NN

NN_ScalerXRGB8888::NN_ScalerXRGB8888(int width, int height)
:
input(new uint32_t[width*height]),
output(new uint32_t[width*2*height*2]),
input_view(input, width, height),
output_view(output, width*2, height*2),
scaler(width, height)
{
	this->src_width = width;
	this->src_height = height;
	this->dest_width = width*2;
	this->dest_height = height*2;
}

NN_ScalerXRGB8888::~NN_ScalerXRGB8888()
{
	delete []this->input;
	delete []this->output;
}

void NN_ScalerXRGB8888::execute()
{
	this->scaler.execute(this->input_view, this->output_view);
}

#endif
