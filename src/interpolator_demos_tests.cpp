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
void test()
{
	// Create single layer and start inference
	uint8_t *buf[1024]={0};
	ncnn::Mat mat = ncnn::Mat(16, 16, 1, buf);

	std::cout<<"dims: "<<mat.dims<<std::endl;
	std::cout<<"w: "<<mat.w<<std::endl;
	std::cout<<"h: "<<mat.h<<std::endl;

	std::cout<<(void*)mat.data<<std::endl;
	std::cout<<(void*)buf<<std::endl;

	float tmp=0.0f;
	for(int y=0;y<mat.h;y++)
	{
		float *row = mat.row(y);
		for(int x=0;x<mat.w;x++)
		{
			row[x] = tmp;
			tmp += 1.0f;
		}
	}

	std::cout<<"buffer:"<<std::endl;
	float *fl = reinterpret_cast<float*>(buf);
	for(int i=0;i<64;i++)
	{
		std::cout<<fl[i]<<" ";
	}
	std::cout<<std::endl;
	std::cout<<"mat data:"<<std::endl;
	fl = reinterpret_cast<float*>(mat.data);
	for(int i=0;i<64;i++)
	{
		std::cout<<fl[i]<<" ";
	}
	std::cout<<std::endl;

	for(int y=0;y<mat.h;y++)
	{
		float *row = mat.row(y);
		for(int x=0;x<mat.w;x++)
		{
			std::cout<<row[x]<<" ";
		}
		std::cout<<std::endl;
	}

	constexpr float norm_vals[]={0.1f};
	int c = 1; //channels
	ncnn::Layer *op;
	op = ncnn::create_layer(ncnn::LayerType::Scale);
	ncnn::ParamDict pd;
	pd.set(0, c);
	op->load_param(pd);
	ncnn::Mat weights[1];
	weights[0] = ncnn::Mat(c);
	for (int q = 0; q < c; q++)
	{
		weights[0][q] = norm_vals[q];
	}
	op->load_model(ncnn::ModelBinFromMatArray(weights));
	ncnn::Option opt;
	opt.num_threads = 1;
	op->create_pipeline(opt);
	op->forward_inplace(mat, opt);
	op->destroy_pipeline(opt);
	delete op;

	for(int y=0;y<mat.h;y++)
	{
		float *row = mat.row(y);
		for(int x=0;x<mat.w;x++)
		{
			std::cout<<row[x]<<" ";
		}
		std::cout<<std::endl;
	}
}

void NN()
{
	// load img
	const char *in_filename = "/tmpfs/in_v.png";
	png::image<png::rgb_pixel> img(in_filename);
	int src_width = img.get_width();
	int src_height = img.get_height();
	int dest_width = src_width*2;
	int dest_height = src_height*2;

	float *y_buffer = new float[src_width*src_height];
	float *cb_2x_buffer = new float[dest_width*dest_height];
	float *cr_2x_buffer = new float[dest_width*dest_height];

	PNG_View input_view = PNG_View(img);
	image_rgb2ycbcr_f<PNG_View>(input_view, y_buffer, cb_2x_buffer, cr_2x_buffer, src_width, src_height);

	// load model
	ncnn::Net srnet;

	int err;
	err = srnet.load_param("model/torchscript.ncnn.param");
	assert(err == false);
	err = srnet.load_model("model/torchscript.ncnn.bin"); //a.k.a. weight
	assert(err == false);

	ncnn::Mat input = ncnn::Mat(src_width, src_height, 1, y_buffer);
	// substract_mean_normalize are designed to work on 3-dim matrixs, so create matrix with channels=1

	constexpr static float normalizer[]={1/255.0f};
	input.substract_mean_normalize(nullptr, normalizer);



	ncnn::Extractor extractor=srnet.create_extractor();
	ncnn::Mat output;

	auto t0 = std::chrono::high_resolution_clock::now();

	extractor.input("in0", input);
	extractor.extract("out0", output);

	std::cout<<"Input: "<<std::endl;
	std::cout<<"C, H, W: "<<input.c<<", "<<input.h<<", "<<input.w<<std::endl;

	std::cout<<"Output: "<<std::endl;
	std::cout<<"C, H, W: "<<output.c<<", "<<output.h<<", "<<output.w<<std::endl;
	//image_rgb2ycbcr_f(, , , , , )
	//ncnn::Mat input = ncnn::Mat::from_pixels(nullptr, ncnn::Mat::PIXEL_GRAY, );

	constexpr static float normalizer_2[]={255.0f};
	output.substract_mean_normalize(nullptr, normalizer_2);

	auto t1 = std::chrono::high_resolution_clock::now();
	auto ms = (t1-t0).count()/1000000;
	std::cout<<ms<<std::endl;



	png::image<png::rgb_pixel> out(dest_width, dest_height);

	PNG_View output_view = PNG_View(out);
	image_ycbcr2rgb_f((float*)output, cb_2x_buffer, cr_2x_buffer, output_view, dest_width, dest_height);
	/*
	for(int y=0;y<output.h;y++)
	{
		float *row=output.row(y);
		for(int x=0;x<output.w;x++)
		{
			uint8_t r = row[x]*255.0f;
			uint8_t g = row[x]*255.0f;
			uint8_t b = row[x]*255.0f;
			output_view.set(y, x, r, g, b);
		}
	}*/
	out.write("/tmpfs/png-out.png");

	delete []y_buffer;
	delete []cb_2x_buffer;
	delete []cr_2x_buffer;
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
		NN();
	}
	#endif
	else
	{
		std::cerr<<"Unrecognized algorithm name"<<std::endl;
	}
}
