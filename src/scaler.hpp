#pragma once

#include <cstdint>

class Scaler
{
public:
	uint32_t *input;
	uint32_t *output;
	int width;
	int height;
	int bpp = 4;
	Scaler(int width, int height);
	~Scaler();
	void scale();
	void scale_scale2x();
};
