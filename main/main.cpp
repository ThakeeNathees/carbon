#include <iostream>
#define PRINT(x) std::cout << (x) << std::endl

int _main(int argc, char** argv)
{
    PRINT("It's goint to crash");
    char* invalid_ptr = NULL;
    *invalid_ptr = 0xff;
	return 0;
}