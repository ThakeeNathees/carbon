#include "crash_handler_x11.h"

int _main(int argc, char** argv);

int main(int argc, char** argv)
{

	CrashHandler crash_handler;
	crash_handler.initialize();

	_main(argc, argv);

	return 0;
}


