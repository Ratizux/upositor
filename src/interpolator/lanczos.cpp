#include <cstdint>
#include <upositor/interpolator/lanczos.hpp>
#include <cstring>
#include <cmath>

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
		const float *a = &alpha[x*Lanczos::rf];

		float sum=0.0f;
		for(int i=0;i<Lanczos::rf;i++)
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
	const float *b = &beta[y*Lanczos::rf];

	for(int x=0;x<dest_width;x++)
	{
		float sum=0.0f;
		for(int i=0;i<Lanczos::rf;i++)
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
	const float *b = &beta[y*Lanczos::rf];

	for(int x=0;x<dest_width;x++)
	{
		float sum=0.0f;
		for(int i=0;i<Lanczos::rf;i++)
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
		int sx = (int)floorf(fx)-(Lanczos::radius-1); // leftmost pixel pos
		x_offset[x] = sx;

		float sum = 0.0f;
		for(int i=0;i<Lanczos::rf;i++)
		{
			float l = lanczos_kernel(fx - (sx+i)); // L(i) for i in reception field
			alpha[x*Lanczos::rf + i] = l;
			sum += l;
		}

		float sum_inv = 1.0f/sum;
		for(int i=0;i<Lanczos::rf;i++)
		{
			alpha[x*Lanczos::rf + i] *= sum_inv;
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
