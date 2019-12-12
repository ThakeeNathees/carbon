
/*** use extern "C" ***/

#include<stdio.h>

// __GNUC__
// __MINGW32__

#ifdef __MINGW32__
	#define DLL __declspec(dllexport)
#else
	#define DLL
#endif



void DLL my_func(const char* arg){
	printf("my func called with : %s\n", arg);
}

/**********************/