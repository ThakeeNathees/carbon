
/***************************************************************************************************************************/
/*                                                CRASH HANDLER MAIN                                                       */
/***************************************************************************************************************************/

#include "crash_handler.h"

int _main(int, char**);

#if defined(PLATFORM_WINDOWS)

	int main(int argc, char** argv) {
	#ifdef CRASH_HANDLER_EXCEPTION
		__try {
			return _main(argc, argv);
		} __except (CrashHandlerException(GetExceptionInformation())) {
			return 1;
		}
	#else
		return _main(argc, argv);
	#endif
	}

#elif defined(PLATOFRM_LINUX)

	int main(int argc, char** argv) {
		CrashHandler crash_handler;
		crash_handler.initialize();
	
		_main(argc, argv);
	
		return 0;
	}

#else
	#define _main main

#endif