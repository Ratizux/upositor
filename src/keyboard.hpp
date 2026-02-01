#pragma once

#include "server.hpp"
extern "C"
{
	#define static
	#include <wlr/types/wlr_keyboard.h>
	#undef static
}

class Keyboard final
{
public:
	wl_listener key_listener;
	wl_listener modifiers_listener;
	//
	wlr_keyboard *wlroots_keyboard;
	//
	Server *server;
	Keyboard(Server*, wlr_input_device*);
};
