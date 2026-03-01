#pragma once

#include <cmath>
#include <tuple>
#include <upositor/interpolator/lanczos.hpp>
#include <upositor/interpolator/utils.hpp>

inline float sinc(float x)
{
	if(x==0.0f) return 1.0f;
	x *= (float)M_PI;
	return sinf(x)/x;
}

inline float lanczos_kernel(float x)
{
	x = fabsf(x);
	if(x<Lanczos::radius_f) return sinc(x)*sinc(x/Lanczos::radius_f);
	return 0.0f;
}

void lanczos_horizonal(const float *src,    // 1 x src_w
					   float *dest,         // 1 x src_w
					   int src_width,
					   int dest_width,
					   const int *x_offset, // src_w
					   const float *alpha   // src_w * rf
					   );

void lanczos_vertical(const float *src,    // src_w * rf
					  uint8_t *dest,      // 1 x src_w
					  int src_height,
					  int dest_width,
					  const int *y_offset, // src_h
					  const float *beta,   // src_h * rf
					  int y
					  );

void lanczos_vertical_float(const float *src,    // src_w * rf
					  float *dest,      // 1 x src_w
					  int src_height,
					  int dest_width,
					  const int *y_offset, // src_h
					  const float *beta,   // src_h * rf
					  int y
					  );

void init_coeffs(int src_width, int dest_width, int *x_offset, float *alpha);

void scale_1chan(float *in, float *out, int src_w, int src_h, int new_w, int new_h,
		   int *x_offset, float *alpha, int *y_offset, float *beta,
		   float *pass1_out,
		   float *src_row,
		   float *dest_row);

template<typename View>
LanczosScaler<View>::LanczosScaler(int width, int height, View &input, View &output)
:
	input(input),
	output(output),
	src_width(width),
	src_height(height),
	dest_width(width*2),
	dest_height(height*2)
{
	this->x_offset = new int[dest_width];
	this->alpha = new float[dest_width * Lanczos::rf];
	this->y_offset = new int[dest_height];
	this->beta = new float[dest_height * Lanczos::rf];
	this->pass1_out = new float[src_height * dest_width];
	this->src_row = new float[src_width];
	this->dest_row_float = new float[dest_width];
	this->image_y = new float[src_width * src_height];
	this->scaled_y = new float[dest_width * dest_height];
	this->scaled_cb = new float[dest_width * dest_height];
	this->scaled_cr = new float[dest_width * dest_height];
}

template<typename View>
LanczosScaler<View>::~LanczosScaler()
{
	delete []this->x_offset;
	delete []this->alpha;
	delete []this->y_offset;
	delete []this->beta;
	delete []this->pass1_out;
	delete []this->src_row;
	delete []this->dest_row_float;
	delete []this->image_y;
	delete []this->scaled_y;
	delete []this->scaled_cb;
	delete []this->scaled_cr;

	this->x_offset = nullptr;
	this->alpha = nullptr;
	this->y_offset = nullptr;
	this->beta = nullptr;
	this->pass1_out = nullptr;
	this->src_row = nullptr;
	this->dest_row_float = nullptr;
	this->image_y = nullptr;
	this->scaled_y = nullptr;
	this->scaled_cb = nullptr;
	this->scaled_cr = nullptr;
}

template<typename View>
void LanczosScaler<View>::execute()
{
	image_rgb2ycbcr_f<View>(this->input,
					  this->image_y,
					  this->scaled_cb,
					  this->scaled_cr,
					  this->src_width,
					  this->src_height);
	scale_1chan(this->image_y,
				this->scaled_y,
				this->src_width, this->src_height,
				this->dest_width, this->dest_height,
				this->x_offset, this->alpha,
				this->y_offset, this->beta,
				this->pass1_out,
				this->src_row,
				this->dest_row_float);
	image_ycbcr2rgb_f<View>(this->scaled_y,
					  this->scaled_cb,
					  this->scaled_cr,
					  this->output,
					  this->dest_width,
					  this->dest_height);
}
