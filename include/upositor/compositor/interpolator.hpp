#pragma once

#include <upositor/compositor/server.hpp>
#include <upositor/interpolator/lanczos.hpp>

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
	LanczosScaler *scaler;
	Server *server;
	int src_width, src_height;
	int dest_width, dest_height;
};
