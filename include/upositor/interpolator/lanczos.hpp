#pragma once

#include <cstdint>

// XRGB, scale by 2
class LanczosScaler
{
public:
	uint32_t *input;
	uint32_t *output;
	int src_width;
	int src_height;
	int dest_width;
	int dest_height;
	constexpr static int bpp = 4;
	LanczosScaler(int width, int height);
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
