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

enum class LineLineXResultType {
    NoIntersection, // NOTE: combines LinesAreCollinear, LinesAreParallelButNotCollinear,
    SegmentsIntersectAndThereforeLinesAlsoIntersect,
    LinesIntersectButSegmentsDoNot,
};

struct LineLineXResult {
    LineLineXResultType type;
    bool success;
    vec2 point;
    real t_ab;
    real t_cd;
};


LineLineXResult burkardt_line_line_intersection(vec2 p, vec2 p_plus_r, vec2 q, vec2 q_plus_s) {
    vec2 r = p_plus_r - p;
    vec2 s = q_plus_s - q;
    real r_cross_s = cross(r, s);
    vec2 q_minus_p = q - p;

    // r x s = 0 => segments parallel (or anti-parallel)
    bool lines_parallel = IS_ZERO(r_cross_s);
    // // q_m_p x r = 0 => q is on line p, p+r
    // bool q_on_other_line = IS_ZERO(cross(q_minus_p, r));

    LineLineXResult result = {};

    if (lines_parallel) {
        result.success = false;
        result.type = LineLineXResultType::NoIntersection;
    } else {
        result.success = true;
        result.t_ab = cross(q_minus_p, s) / r_cross_s;
        result.t_cd = cross(q_minus_p, r) / r_cross_s;
        result.point = p + result.t_ab * p_plus_r;
        if (IS_BETWEEN_TIGHT(result.t_ab, 0.0f, 1.0f) && IS_BETWEEN_TIGHT(result.t_cd, 0.0f, 1.0f)) {
            result.type = LineLineXResultType::SegmentsIntersectAndThereforeLinesAlsoIntersect;
        } else {
            result.type = LineLineXResultType::LinesIntersectButSegmentsDoNot;
        }
    }

    return result;
}

#define BURKARDT_VEC2(p, a) double p[] = { a.x, a.y };

real burkardt_three_point_angle(vec2 p, vec2 center, vec2 q) {
    BURKARDT_VEC2(_1, p);
    BURKARDT_VEC2(_2, center);
    BURKARDT_VEC2(_3, q);
    return (real) angle_rad_2d(_1, _2, _3);
}
