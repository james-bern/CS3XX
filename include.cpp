#define APP_USE_32_BIT_FLOATING_POINT

#include "app.cpp"
#include "jim.cpp"
#include "containers.cpp"
#include "manifoldc.h"

typedef uint32_t uint32;
typedef float real32;

#include "jim2.cpp"
#include "box.cpp"
#include "burkardt.cpp"
#include "header.cpp"
#include "elephant.cpp"

BEGIN_PRE_MAIN {
    setvbuf(stdout, NULL, _IONBF, 0); // don't buffer printf
    srand((unsigned int) time(NULL)); srand(0);
    _eso_init();
    _window_init();
    _soup_init();
    _mesh_init();
    _eso_reset();
} END_PRE_MAIN 
