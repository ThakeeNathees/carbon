#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

int main(int argc, char** argv)
{
    void *handle;
    void (*my_func)(const char*);

    if (argc<2){
        printf("usage: <shared lib name>\n");
        exit(1);
    }
	
	handle = dlopen(argv[1], RTLD_LAZY);// "./mylib.dll" or "./mylib.so"
    
    if (!handle) {
        /* fail to load the library */
        fprintf(stderr, "Error: %s\n", dlerror());
        return EXIT_FAILURE;
    }

    *(void**)(&my_func) = dlsym(handle, "my_func");
    if (!my_func) {
        /* no such symbol */
        fprintf(stderr, "Error: %s\n", dlerror());
        dlclose(handle);
        return EXIT_FAILURE;
    }

    my_func("hello!");
    dlclose(handle);

    return EXIT_SUCCESS;
}
