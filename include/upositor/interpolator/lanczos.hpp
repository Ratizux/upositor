#pragma once

#include <cstdint>
#include <tuple>
#include <upositor/interpolator/utils.hpp>

namespace Lanczos
{
	constexpr int radius = 3;
	constexpr float radius_f = (float)radius;
	constexpr int rf = radius*2; //reception field width
}

// scale by 2
template<typename View>
class LanczosScaler
{
public:
	View &input;
	View &output;
	int src_width;
	int src_height;
	int dest_width;
	int dest_height;
	constexpr static int bpp = 4;
	LanczosScaler(int width, int height, View &input, View &output);
	~LanczosScaler();
	void execute();
private:
	int *x_offset = nullptr;
	float *alpha = nullptr;
	int *y_offset = nullptr;
	float *beta = nullptr;
	float *pass1_out = nullptr;
	float *src_row = nullptr;   // float* converted from pixel, 1 row
	// uint8_t *dest_row;
	float *dest_row_float = nullptr;
	float *image_y = nullptr;   // orig Y plane
	float *scaled_y = nullptr;  // Y plane, lanczos scaled
	float *scaled_cb = nullptr; // Cb plane, integer scaled
	float *scaled_cr = nullptr; // Cr plane, integer scaled
};

class LanczosScalerXRGB8888
{
public:
	uint32_t *input;
	uint32_t *output;
	XRGB8888_View input_view;
	XRGB8888_View output_view;

	LanczosScaler<XRGB8888_View> scaler;

	int src_width;
	int src_height;
	int dest_width;
	int dest_height;
	constexpr static int bpp = 4;
	LanczosScalerXRGB8888(int width, int height);
	~LanczosScalerXRGB8888();
	void execute();
};

#include <upositor/interpolator/lanczos_impl.hpp>
