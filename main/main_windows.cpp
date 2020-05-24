#include "crash_handler_windows.h"

int _main(int argc, char** argv);

int main(int argc, char** argv)
{
#ifdef CRASH_HANDLER_EXCEPTION
    __try {
        return _main(argc, argv);
    }
    __except(CrashHandlerException(GetExceptionInformation())) {
        return 1;
    }
#else
    return _main(argc, argv);
#endif
}
