#include <cstdint>
#include <upositor/interpolator/lanczos.hpp>
#include <cstring>
#include <cmath>
#include <tuple>

template<typename T>
inline T clip(T x, T min, T max)
{
	if(x > max) return max;
	if(x < min) return min;
	return x;
}

constexpr int radius = 3;
constexpr float radius_f = (float)radius;
constexpr int rf = radius*2; //reception field width

inline float sinc(float x)
{
	if(x==0.0f) return 1.0f;
	x *= (float)M_PI;
	return sinf(x)/x;
}

inline float lanczos_kernel(float x)
{
	x = fabsf(x);
	if(x<radius_f) return sinc(x)*sinc(x/radius_f);
	return 0.0f;
}

// horizonal component, one line
void lanczos_horizonal(const float *src,    // 1 x src_w
					   float *dest,         // 1 x src_w
					   int src_width,
					   int dest_width,
					   const int *x_offset, // src_w
					   const float *alpha   // src_w * rf
					   )
{
	for(int x=0;x<dest_width;x++)
	{
		const float *a = &alpha[x*rf];

		float sum=0.0f;
		for(int i=0;i<rf;i++)
		{
			int column = clip(x_offset[x]+i, 0, src_width-1);
			sum += a[i]*src[column];
		}

		dest[x] = sum;
	}
}

// vertical component, one line
// currently not used.
void lanczos_vertical(const float *src,    // src_w * rf
					  uint8_t *dest,      // 1 x src_w
					  int src_height,
					  int dest_width,
					  const int *y_offset, // src_h
					  const float *beta,   // src_h * rf
					  int y
					  )
{
	const float *b = &beta[y*rf];

	for(int x=0;x<dest_width;x++)
	{
		float sum=0.0f;
		for(int i=0;i<rf;i++)
		{
			int row = clip(y_offset[y]+i, 0, src_height-1);
			sum += b[i]*src[dest_width*row + x];
		}

		//dest[x] = sum;
		int sum_int = std::lrint(sum);
		sum_int = clip(sum_int, 0, 255);
		dest[x] = (uint8_t)sum_int;
	}
}


// vertical component, one line, float output
void lanczos_vertical_float(const float *src,    // src_w * rf
					  float *dest,      // 1 x src_w
					  int src_height,
					  int dest_width,
					  const int *y_offset, // src_h
					  const float *beta,   // src_h * rf
					  int y
					  )
{
	const float *b = &beta[y*rf];

	for(int x=0;x<dest_width;x++)
	{
		float sum=0.0f;
		for(int i=0;i<rf;i++)
		{
			int row = clip(y_offset[y]+i, 0, src_height-1);
			sum += b[i]*src[dest_width*row + x];
		}

		dest[x] = sum;
	}
}

void init_coeffs(int src_width, int dest_width, int *x_offset, float *alpha)
{
	float scale = (float)src_width/dest_width;

	for(int x=0;x<dest_width;x++)
	{
		float fx = (x+0.5f)*scale-0.5f; // pixel center pos, scaled
		int sx = (int)floorf(fx)-(radius-1); // leftmost pixel pos
		x_offset[x] = sx;

		float sum = 0.0f;
		for(int i=0;i<rf;i++)
		{
			float l = lanczos_kernel(fx - (sx+i)); // L(i) for i in reception field
			alpha[x*rf + i] = l;
			sum += l;
		}

		float sum_inv = 1.0f/sum;
		for(int i=0;i<rf;i++)
		{
			alpha[x*rf + i] *= sum_inv;
		}
	}
}

void scale_1chan(float *in, float *out, int src_w, int src_h, int new_w, int new_h,
		   int *x_offset, float *alpha, int *y_offset, float *beta,
		   float *pass1_out,
		   float *src_row,
		   float *dest_row)
{
    init_coeffs(src_w, new_w, x_offset, alpha);
    init_coeffs(src_h, new_h, y_offset, beta);

	// pass 1
    for (int y = 0; y < src_h; ++y)
    {
        for (int x = 0; x < src_w; ++x)
		{
			src_row[x] = in[y*src_w + x];
		}

        lanczos_horizonal(src_row, &pass1_out[y * new_w], src_w, new_w, x_offset, alpha);
    }

    // pass 2
    for (int y = 0; y < new_h; ++y)
	{
		lanczos_vertical_float(pass1_out, dest_row, src_h, new_w, y_offset, beta, y);
		for (int x = 0; x < new_w; x++)
		{
			out[y*new_w + x] = dest_row[x];
		}
	}
}

// not a conformant implementation. modified to reduce number of float operations.
inline std::tuple<float, float, float> rgb2ycbcr_f(uint8_t r, uint8_t g, uint8_t b)
{
	float y = 0.299f*r + 0.587f*g + 0.114*b;
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

#include <iostream>

// keep y, scale cb/cr, XRGB8888
void image_rgb2ycbcr_f(uint32_t *in, float *out_y, float *out_cb, float *out_cr, int src_w, int src_h)
{
	for(int y=0;y<src_h;y++)
	{
		for(int x=0;x<src_w;x++)
		{
			//std::cout<<(void*)in<<std::endl;
			uint8_t *pixel = reinterpret_cast<uint8_t*>(&in[y*src_w + x]);
			//std::cout<<(void*)pixel<<std::endl;
			uint8_t &r=pixel[2];
			uint8_t &g=pixel[1];
			uint8_t &b=pixel[0];
			//std::cout<<(int)r<<" "<<(int)g<<" "<<(int)b<<std::endl;
			float color_y,color_cb,color_cr;
			std::tie(color_y, color_cb, color_cr) = rgb2ycbcr_f(r, g, b);
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

void image_ycbcr2rgb_f(float *in_y, float *in_cb, float *in_cr, uint32_t *out, int dest_w, int dest_h)
{
	for(int y=0;y<dest_h;y++)
	{
		for(int x=0;x<dest_w;x++)
		{
			float &color_y = in_y[y*dest_w + x];
			float &color_cb = in_cb[y*dest_w + x];
			float &color_cr = in_cr[y*dest_w + x];
			uint8_t r,g,b;
			std::tie(r, g, b) = ycbcr2rgb_f(color_y, color_cb, color_cr);

			uint8_t *pixel = reinterpret_cast<uint8_t*>(&out[y*dest_w + x]);
			pixel[2] = r;
			pixel[1] = g;
			pixel[0] = b;
		}
	}
}

LanczosScaler::LanczosScaler(int width, int height)
{
	this->src_width = width;
	this->src_height = height;

	// TODO support arbitrary scale ratio
	this->dest_width = width*2;
	this->dest_height = height*2;

	int size_in = width*height;
	int size_out = width*2*height*2;
	this->input = new uint32_t[size_in];
	this->output = new uint32_t[size_out];

	this->x_offset = new int[dest_width];
	this->alpha = new float[dest_width * rf];
	this->y_offset = new int[dest_height];
	this->beta = new float[dest_height * rf];
	this->pass1_out = new float[dest_width *src_height];
	this->src_row = new float[src_width];
	this->dest_row_float = new float[dest_width];
	this->image_y = new float[src_width * src_height];
	this->scaled_y = new float[dest_width * dest_height];
	this->scaled_cb = new float[dest_width * dest_height];
	this->scaled_cr = new float[dest_width * dest_height];
}

LanczosScaler::~LanczosScaler()
{
	delete []this->input;
	delete []this->output;
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

	this->input = nullptr;
	this->output = nullptr;
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

void LanczosScaler::execute()
{
	image_rgb2ycbcr_f(this->input,
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
	image_ycbcr2rgb_f(this->scaled_y,
					  this->scaled_cb,
					  this->scaled_cr,
					  this->output,
					  this->dest_width,
					  this->dest_height);
}
