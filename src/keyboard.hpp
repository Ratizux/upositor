#pragma once

#include "server.hpp"
extern "C"
{
	#define static
	#include <wlr/types/wlr_keyboard.h>
	#undef static
}

class Keyboard
{
public:
	wl_listener key_listener;
	//
	wlr_keyboard *wlroots_keyboard;
	//
	Server *server;
	Keyboard(Server*, wlr_input_device*);
	//
	static std::unordered_map<wl_listener*, Keyboard*> listener_self_map;
};
