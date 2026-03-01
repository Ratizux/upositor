#include <chrono>
#include <cstdint>
#include <png++/png.hpp>
#include <png++/rgb_pixel.hpp>
#include <string>
#include <upositor/interpolator/lanczos.hpp>
#include <upositor/interpolator/utils.hpp>
#include <upositor/interpolator/nn.hpp>

#ifdef UPOSITOR_ENABLE_NN
#include <ncnn/net.h>
#include <ncnn/gpu.h>
#include <ncnn/layer.h>
#include <ncnn/layer_type.h>
#endif

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

#ifdef UPOSITOR_ENABLE_NN
void NN(const char* in_filename, const char* out_filename)
{
	// load img
	png::image<png::rgb_pixel> img(in_filename);
	int src_width = img.get_width();
	int src_height = img.get_height();
	int dest_width = src_width*2;
	int dest_height = src_height*2;

	NN_Scaler<PNG_View> scaler(src_width, src_height);

	auto t0 = std::chrono::high_resolution_clock::now();

	PNG_View input_view = PNG_View(img);
	png::image<png::rgb_pixel> out(dest_width, dest_height);
	PNG_View output_view = PNG_View(out);

	scaler.execute(input_view, output_view);

	auto t1 = std::chrono::high_resolution_clock::now();
	auto ms = (t1-t0).count()/1000000;
	std::cout<<ms<<std::endl;

	out.write(out_filename);
}
#endif


int main(int argc, char** argv)
{
	if (argc != 4 && argc != 5)
    {
        std::cerr<<"Usage: "<<argv[0]<<" algorithm input.png output.png [loop_count]"<<std::endl;
        return 1;
    }
    auto &algorithm_name = argv[1];
    auto &input_filename = argv[2];
	auto &output_filename = argv[3];
    int loop_count=1;
    if(argc == 5)
	{
		loop_count = atoi(argv[4]);
	}

	std::string name = algorithm_name;
	if(name == "lanczos")
	{
		ScaleLanczos(input_filename, output_filename, loop_count);
	}
	#ifdef UPOSITOR_ENABLE_NN
	else if(name == "nn")
	{
		NN(input_filename, output_filename);
	}
	#endif
	else
	{
		std::cerr<<"Unrecognized algorithm name"<<std::endl;
	}
}
