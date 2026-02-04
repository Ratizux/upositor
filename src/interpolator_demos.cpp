#include <cstdint>
#include <png++/rgb_pixel.hpp>
#include <upositor/interpolator/lanczos.hpp>
#include <png++/png.hpp>

class PNG_View
{
public:
	PNG_View(png::image<png::rgb_pixel> &in)
	:target(in){}
	std::tuple<uint8_t,uint8_t,uint8_t> at(int y, int x)
	{
		uint8_t r = target[y][x].red;
		uint8_t g = target[y][x].green;
		uint8_t b = target[y][x].blue;
		return {r,g,b};
	}
	void set(int y, int x, uint8_t r, uint8_t g, uint8_t b)
	{
		target[y][x].red = r;
		target[y][x].green = g;
		target[y][x].blue = b;
	}
	//
	png::image<png::rgb_pixel> &target;
};

void ScaleLanczos(const char* in_filename, const char* out_filename, int loop_count)
{
	png::image<png::rgb_pixel> img(in_filename);
	int src_width = img.get_width();
	int src_height = img.get_height();
	int dest_width = src_width*2;
	int dest_height = src_height*2;

	png::image<png::rgb_pixel> out(dest_width, dest_height);

	PNG_View input = PNG_View(img);
	PNG_View output = PNG_View(out);

	LanczosScaler<PNG_View> scaler(src_width, src_height, input, output);

	for(int i=0; i<loop_count; i++)
	{
		scaler.execute();
	}

	out.write(out_filename);
}

int main(int argc, char** argv)
{
	if (argc != 3 && argc != 4)
    {
        std::cerr << "Usage: "<<argv[0]<<" input.png output.png [loop_count]\n";
        return 1;
    }
    if(argc == 4)
	{
		int loop_count = atoi(argv[3]);
		ScaleLanczos(argv[1], argv[2], loop_count);
		return 0;
	}
	ScaleLanczos(argv[1], argv[2], 1);
}
