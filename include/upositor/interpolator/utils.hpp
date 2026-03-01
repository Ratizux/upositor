#pragma once

#include <cstdint>
#include <tuple>
#include <functional>

template<typename T>
inline T clip(T x, T min, T max)
{
	if(x > max) return max;
	if(x < min) return min;
	return x;
}

// not a conformant implementation. modified to reduce number of float operations.
inline std::tuple<float, float, float> rgb2ycbcr_like_f(uint8_t r, uint8_t g, uint8_t b)
{
	float y = 0.299f*r + 0.587f*g + 0.114f*b;
	float cb = -0.1687f*r - 0.3313f*g + 0.5f*b;
	float cr = 0.5f*r - 0.4187f*g - 0.0813f*b;
	return {y, cb, cr};
}

inline std::tuple<uint8_t, uint8_t, uint8_t> ycbcr2rgb_like_f(float y, float cb, float cr)
{
	float r_f = (y + 1.402f*cr);
	float g_f = (y - 0.344136f*cb - 0.714136f*cr);
	float b_f = (y + 1.772f*cb);
	uint8_t r = clip(r_f, 0.0f, 255.0f);
	uint8_t g = clip(g_f, 0.0f, 255.0f);
	uint8_t b = clip(b_f, 0.0f, 255.0f);
	return {r, g, b};
}

inline std::tuple<float, float, float> rgb2ycbcr_f(uint8_t r, uint8_t g, uint8_t b)
{
	float y = 0.299f*r + 0.587f*g + 0.114f*b;
	float cb = -0.1687f*r - 0.3313f*g + 0.5f*b;
	float cr = 0.5f*r - 0.4187f*g - 0.0813f*b;
	return {y, cb, cr};
}

inline std::tuple<uint8_t, uint8_t, uint8_t> ycbcr2rgb_f(float y, float cb, float cr)
{
	float r_f = (y + 1.402f*cr);
	float g_f = (y - 0.344136f*cb - 0.714136f*cr);
	float b_f = (y + 1.772f*cb);
	uint8_t r = clip(r_f, 0.0f, 255.0f);
	uint8_t g = clip(g_f, 0.0f, 255.0f);
	uint8_t b = clip(b_f, 0.0f, 255.0f);
	return {r, g, b};
}

template<typename fn, typename View>
std::function<void(View&, float*, float*, float*, int, int)> _image_rgb2ycbcr_f()
{
	return [](View &in, float *out_y, float *out_cb, float *out_cr, int src_w, int src_h){};
}

static auto a = _image_rgb2ycbcr_f<int, int>();

// keep y, scale cb/cr
template<typename View>
void image_rgb2ycbcr_f(View &in, float *out_y, float *out_cb, float *out_cr, int src_w, int src_h)
{
	for(int y=0;y<src_h;y++)
	{
		for(int x=0;x<src_w;x++)
		{
			//std::cout<<(void*)in<<std::endl;
			uint8_t r,g,b;
			std::tie(r,g,b) = in.at(y,x);

			//std::cout<<(int)r<<" "<<(int)g<<" "<<(int)b<<std::endl;
			float color_y,color_cb,color_cr;
			std::tie(color_y, color_cb, color_cr) = rgb2ycbcr_like_f(r, g, b);
			out_y[y*src_w + x] = color_y;

			out_cb[(y*2)*(src_w*2) + x*2] = color_cb;
			out_cb[(y*2)*(src_w*2) + x*2 + 1] = color_cb;
			out_cb[(y*2+1)*(src_w*2) + x*2] = color_cb;
			out_cb[(y*2+1)*(src_w*2) + x*2 + 1] = color_cb;

			out_cr[(y*2)*(src_w*2) + x*2] = color_cr;
			out_cr[(y*2)*(src_w*2) + x*2 + 1] = color_cr;
			out_cr[(y*2+1)*(src_w*2) + x*2] = color_cr;
			out_cr[(y*2+1)*(src_w*2) + x*2 + 1] = color_cr;
		}
	}
}

template<typename View>
void image_ycbcr2rgb_f(float *in_y, float *in_cb, float *in_cr, View &out, int dest_w, int dest_h)
{
	for(int y=0;y<dest_h;y++)
	{
		for(int x=0;x<dest_w;x++)
		{
			float &color_y = in_y[y*dest_w + x];
			float &color_cb = in_cb[y*dest_w + x];
			float &color_cr = in_cr[y*dest_w + x];
			uint8_t r,g,b;
			std::tie(r, g, b) = ycbcr2rgb_like_f(color_y, color_cb, color_cr);
			out.set(y, x, r, g, b);
		}
	}
}

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
