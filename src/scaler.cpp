#include "scaler.hpp"

Scaler::Scaler(int width, int height)
{
	this->width = width;
	this->height = height;

	int size = width*height;
	int scaled_size = width*2*height*2;
	this->input = new uint32_t[size];
	this->output = new uint32_t[scaled_size];
}

Scaler::~Scaler()
{
	delete []this->input;
	delete []this->output;
	this->input = nullptr;
	this->output = nullptr;
}

void Scaler::scale()
{
	// nop
}
