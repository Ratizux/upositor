#pragma once

#include <upositor/compositor/server.hpp>
extern "C"
{
	#define static
	#include <wlr/types/wlr_xdg_shell.h>
	#undef static
}

class Toplevel final
{
public:
	Toplevel(Server*, wlr_xdg_toplevel*);
	Server *server;
	void focus_kbd();
	//
	wl_listener commit_listener;
	wl_listener destory_listener;
	wl_listener map_listener;
	wl_listener request_move_listener;
	wlr_xdg_toplevel *wlroots_xdg_toplevel;
	//
	bool ready=false;
};

static_assert(std::is_standard_layout<Toplevel>::value, "non-standard layout: Toplevel");
