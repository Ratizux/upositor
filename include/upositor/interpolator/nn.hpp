#pragma once

#include "ncnn/gpu.h"
#ifdef UPOSITOR_ENABLE_NN

#include <cassert>
#include <ncnn/mat.h>
#include <ncnn/net.h>
#include <upositor/interpolator/utils.hpp>

template <typename View>
class NN_Scaler
{
public:
	int src_width;
	int src_height;
	int dest_width;
	int dest_height;
	NN_Scaler(int width, int height);
	~NN_Scaler();
	void execute(View &input, View &output_view);
private:
	float *y_buffer;
	float *cb_2x_buffer;
	float *cr_2x_buffer;
	ncnn::Net srnet;
	ncnn::VulkanDevice *vkdev = nullptr;
	ncnn::VkAllocator *blob_vkallocator = nullptr;
	ncnn::VkAllocator *staging_vkallocator = nullptr;

	constexpr static bool use_gpu = true;
};

template <typename View>
NN_Scaler<View>::NN_Scaler(int width, int height)
:
	src_width(width),
	src_height(height),
	dest_width(width*2),
	dest_height(height*2)
{
	this->y_buffer = new float[src_width*src_height];
	this->cb_2x_buffer = new float[dest_width*dest_height];
	this->cr_2x_buffer = new float[dest_width*dest_height];

	if(use_gpu)
	{
		srnet.opt.use_vulkan_compute=true;
		this->vkdev = ncnn::get_gpu_device(0);
		this->blob_vkallocator = this->vkdev->acquire_blob_allocator();
		this->staging_vkallocator = this->vkdev->acquire_staging_allocator();

		srnet.opt.blob_vkallocator = blob_vkallocator;
		srnet.opt.staging_vkallocator = staging_vkallocator;
		srnet.opt.workspace_vkallocator = blob_vkallocator;
	}
	// TODO
	/*
	ncnn::VulkanDevice *vkdev = ncnn::get_gpu_device(0);
	ncnn::VkAllocator *blob_vkallocator = vkdev->acquire_blob_allocator();
	ncnn::VkAllocator *staging_vkallocator = vkdev->acquire_staging_allocator();
	srnet.opt.blob_vkallocator = blob_vkallocator;
	srnet.opt.staging_vkallocator = staging_vkallocator;
	srnet.opt.workspace_vkallocator = blob_vkallocator;
	srnet.opt.use_fp16_packed = false;
	srnet.opt.use_fp16_storage = false;
	srnet.opt.use_fp16_arithmetic = false;
	srnet.opt.use_int8_packed = false;
	srnet.opt.use_int8_storage = false;
	srnet.opt.use_int8_arithmetic = false;

	srnet.opt.use_fp16_packed = false;
	srnet.opt.use_fp16_storage = false;
	srnet.opt.use_fp16_arithmetic = false;
	srnet.opt.use_int8_packed = false;
	srnet.opt.use_int8_storage = false;
	srnet.opt.use_int8_arithmetic = false;*/

	int err;
	err = srnet.load_param("model/torchscript.ncnn.param");
	assert(err == false);
	err = srnet.load_model("model/torchscript.ncnn.bin"); //a.k.a. weight
	assert(err == false);
}

template <typename View>
NN_Scaler<View>::~NN_Scaler()
{
	delete []this->y_buffer;
	delete []this->cb_2x_buffer;
	delete []this->cr_2x_buffer;

	this->y_buffer = nullptr;
	this->cb_2x_buffer = nullptr;
	this->cr_2x_buffer = nullptr;

	if(use_gpu)
	{
		vkdev->reclaim_blob_allocator(blob_vkallocator);
		vkdev->reclaim_staging_allocator(staging_vkallocator);
	}
}

template <typename View>
void NN_Scaler<View>::execute(View &input_view, View &output_view)
{
	if(use_gpu)
	{
		blob_vkallocator->clear();
		staging_vkallocator->clear();
	}

	image_rgb2ycbcr_f(input_view, y_buffer, cb_2x_buffer, cr_2x_buffer, src_width, src_height);

	ncnn::Mat input = ncnn::Mat(src_width, src_height, 1, this->y_buffer);
	// substract_mean_normalize are designed to work on 3-dim matrixs, so create matrix with channels=1

	constexpr static float norm_255_1[]={1/255.0f};
	input.substract_mean_normalize(nullptr, norm_255_1);

	ncnn::Extractor extractor = this->srnet.create_extractor();
	ncnn::Mat output;
	extractor.input("in0", input);
	extractor.extract("out0", output);

	constexpr static float norm_1_255[]={255.0f};
	output.substract_mean_normalize(nullptr, norm_1_255);

	image_ycbcr2rgb_f((float*)output, this->cb_2x_buffer, this->cr_2x_buffer, output_view, this->dest_width, this->dest_height);
}

class NN_ScalerXRGB8888
{
public:
	uint32_t *input;
	uint32_t *output;
	XRGB8888_View input_view;
	XRGB8888_View output_view;

	NN_Scaler<XRGB8888_View> scaler;

	int src_width;
	int src_height;
	int dest_width;
	int dest_height;
	constexpr static int bpp = 4;
	NN_ScalerXRGB8888(int width, int height);
	~NN_ScalerXRGB8888();
	void execute();
};

#endif
