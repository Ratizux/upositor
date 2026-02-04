#include <upositor/compositor/server.hpp>

extern "C"
{
	#define static
	#include <wlr/util/log.h>
	#undef static
}

int main()
{
	wlr_log_init(WLR_INFO, NULL);
	Server server;
	server.run();
}
