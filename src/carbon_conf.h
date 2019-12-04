#ifndef CARBON_CONF_H
#define CARBON_CONF_H

typedef unsigned char bool;
#define true  1
#define false 0

#define ARR_SIZE(arr) sizeof(arr) / sizeof(*arr)

// for windows dll define CARBON_BUILD_DLL, CARBON_LIB
#if defined(CARBON_BUILD_DLL)

	#if defined(CARBON_LIB)
		#define CARBON_API __declspec(dllexport)
	#else
		#define CARBON_API __declspec(dllimport)
	#endif

#else
	#define CARBON_API
#endif

// CARBON_CONF_H
#endif