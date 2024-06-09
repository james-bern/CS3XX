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
    bool is_valid;
    vec2 position;
};

LineLineIntersectionResult burkardt_line_line_intersection(vec2 a, vec2 b, vec2 c, vec2 d) {
    double p1[2];
    double p2[2];
    double p3[2];
    double p4[2];
    for_(k, 2) {
        p1[k] = a[k];
        p2[k] = b[k];
        p3[k] = c[k];
        p4[k] = d[k];
    }
    int ival;
    double p[2];
    lines_exp_int_2d(p1, p2, p3, p4, &ival, p);
    LineLineIntersectionResult result;
    result.is_valid = (ival == 1);
    for_(k, 2) result.position[k] = (real) p[k];
    { // we are less stringent than burkardt
        if (squaredNorm(a - result.position) > HUGE_VAL) result.is_valid = false;
    }
    return result;
}


real burkardt_three_point_angle(vec2 p, vec2 center, vec2 q) {
    double p1[2];
    double p2[2];
    double p3[2];
    for_(k, 2) {
        p1[k] = p[k];
        p2[k] = center[k];
        p3[k] = q[k];
    }
    return (real) angle_rad_2d(p1, p2, p3);
}
