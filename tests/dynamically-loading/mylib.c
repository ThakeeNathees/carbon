
/*** use extern "C" ***/

#include<stdio.h>

#define DLL __declspec(dllexport)


void DLL my_func(const char* arg){
	printf("my func called with : %s\n", arg);
}

/**********************/