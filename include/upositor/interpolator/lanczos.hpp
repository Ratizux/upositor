#pragma once

#include <cstdint>
#include <tuple>

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

#include <upositor/interpolator/lanczos_impl.hpp>

class XRGB8888_View
{
public:
	XRGB8888_View(uint32_t *in, int width, int height)
	{
		this->target = in;
		this->w = width;
		this->h = height;
	}
	inline std::tuple<uint8_t, uint8_t, uint8_t> at(int y, int x)
	{
		uint32_t pixel = this->target[y*w + x];
		uint8_t r = static_cast<uint8_t>(pixel>>16 & 0xff);
		uint8_t g = static_cast<uint8_t>(pixel>>8 & 0xff);
		uint8_t b = static_cast<uint8_t>(pixel & 0xff);
		return {r, g, b};
	}
	inline void set(int y, int x, uint8_t r, uint8_t g, uint8_t b)
	{
		uint32_t pixel = b;
		pixel |= (static_cast<uint32_t>(g) << 8);
		pixel |= (static_cast<uint32_t>(r) << 16);
		this->target[y*w + x] = pixel;
	}
	uint32_t *target;
	int w;
	int h;
};

class XRGB8888_View_Reference
{
public:
	XRGB8888_View_Reference(uint32_t *in, int width, int height)
	{
		this->target = in;
		this->w = width;
		this->h = height;
	}
	inline std::tuple<uint8_t, uint8_t, uint8_t> at(int y, int x)
	{
		uint8_t *pixel = reinterpret_cast<uint8_t*>(&this->target[y*w + x]);
		uint8_t r = pixel[2];
		uint8_t g = pixel[1];
		uint8_t b = pixel[0];
		return {r, g, b};
	}
	inline void set(int y, int x, uint8_t r, uint8_t g, uint8_t b)
	{
		uint8_t *pixel = reinterpret_cast<uint8_t*>(&this->target[y*w + x]);
		pixel[2] = r;
		pixel[1] = g;
		pixel[0] = b;
	}
	uint32_t *target;
	int w;
	int h;
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
