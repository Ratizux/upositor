#pragma once

#include <upositor/compositor/server.hpp>
#include <upositor/interpolator/lanczos.hpp>
#include <upositor/interpolator/nn.hpp>

class Interpolator
{
public:
	Interpolator();
	Interpolator(Server *parent);
	~Interpolator();
	void new_size(int new_src_width, int new_src_height);
	void read(wlr_texture *in_texture);
	void execute();
	wlr_texture *write();
private:
	#ifdef UPOSITOR_ENABLE_NN
	NN_ScalerXRGB8888 *scaler;
	#else
	LanczosScalerXRGB8888 *scaler;
	#endif
	Server *server;
	int src_width, src_height;
	int dest_width, dest_height;
};
