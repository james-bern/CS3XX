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

struct LineLineIntersectionResult {
    bool success;
    vec2 position;
};

#define BURKARDT_VEC2(p, a) double p[] = { a.x, a.y };

LineLineIntersectionResult burkardt_line_line_intersection(vec2 a, vec2 b, vec2 c, vec2 d) {
    BURKARDT_VEC2(p1, a);
    BURKARDT_VEC2(p2, b);
    BURKARDT_VEC2(p3, c);
    BURKARDT_VEC2(p4, d);
    int ival;
    double p[2];
    lines_exp_int_2d(p1, p2, p3, p4, &ival, p);
    LineLineIntersectionResult result;
    result.success = (ival == 1);
    for_(k, 2) result.position[k] = (real) p[k];
    { // NOTE: we are less stringent than burkardt
        if (squaredNorm(a - result.position) > HUGE_VAL) result.success = false;
    }
    return result;
}


real burkardt_three_point_angle(vec2 p, vec2 center, vec2 q) {
    BURKARDT_VEC2(p1, p);
    BURKARDT_VEC2(p2, center);
    BURKARDT_VEC2(p3, q);
    return (real) angle_rad_2d(p1, p2, p3);
}
