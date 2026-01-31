#pragma once

#include "server.hpp"
extern "C"
{
	#define static
	#include <wlr/types/wlr_xdg_shell.h>
	#undef static
}

class Toplevel
{
public:
	Toplevel(Server*, wlr_xdg_toplevel*);
	Server *server;
	void focus_kbd();
	//
	wl_listener map_listener;
	wl_listener commit_listener;
	wlr_xdg_toplevel *wlroots_xdg_toplevel;
	//
	bool ready=false;
};
