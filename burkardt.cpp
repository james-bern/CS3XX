turns out this was a bad idea

// the only good mathematical software is written by John Burkardt

#ifdef OPERATING_SYSTEM_WINDOWS
#pragma warning(push)
#pragma warning(disable : 4100 4244 4701)
#endif
#ifdef OPERATING_SYSTEM_APPLE
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wsometimes-uninitialized"
#pragma clang diagnostic ignored "-Warray-parameter"
#endif
#include "geometry.h"
#include "geometry.c"
#ifdef OPERATING_SYSTEM_WINDOWS
#pragma warning(pop)
#endif
#ifdef OPERATING_SYSTEM_APPLE
#pragma clang diagnostic pop
#endif

// some snail friendly burkardt wrappers
// - vecX everywhere
// - radians everywhere

