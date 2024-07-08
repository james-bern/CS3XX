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

struct LineLineXResult {
    vec2 point;
    real t_ab;
    real t_cd;
    bool point_is_on_segment_ab;
    bool point_is_on_segment_cd;
    bool lines_are_parallel;
};


LineLineXResult burkardt_line_line_intersection(//vec2 a, vec2 b, vec2 c, vec2 d) {
    vec2 p, vec2 p_plus_r, vec2 q, vec2 q_plus_s) {
    // https://stackoverflow.com/a/565282
    vec2 r = p_plus_r - p;
    vec2 s = q_plus_s - q;
    real r_cross_s = cross(r, s);

    LineLineXResult result = {};
    result.lines_are_parallel = abs(r_cross_s) < 0.0001;
    if (result.lines_are_parallel) {
    } else {
        vec2 q_minus_p = q - p;
        result.t_ab = cross(q_minus_p, s) / r_cross_s;
        result.t_cd = cross(q_minus_p, r) / r_cross_s;
        result.point = p + result.t_ab * r;
        result.point_is_on_segment_ab = IS_BETWEEN_TIGHT(result.t_ab, 0.0f, 1.0f);
        result.point_is_on_segment_cd = IS_BETWEEN_TIGHT(result.t_cd, 0.0f, 1.0f);
    }
    return result;
}

struct ArcLineXResult {
    vec2 point;
    real theta_1a;
    real theta_1b;
    bool point_is_on_segment_ab;
    bool point_is_on_arc;
    bool no_possible_intersection;
};

#if 0
// burkardt is amazing, he even uses our arc struct
LineLineXResult burkardt_arc_arc_intersection(//vec2 a, vec2 b, vec2 c, vec2 d) {
    ArcEntity arc_a, ArcEntity arc_b) {


        float d = distance(arcA.center, arcB.center);
        //TODO: find fucntion that checks to see if they are close enough for floats
        if (d > arcA.radius + arcB.radius) {                // non intersecting
        } else if (d < abs(arcA.radius-arcB.radius)) {      // One circle within other
        } else if (d == 0 && arcA.radius == arcB.radius) {  // coincident circles
        } else {
            real a = (POW(arcA.radius, 2) - POW(arcB.radius, 2) + POW(d, 2)) / (2 * d);
            real h = SQRT(POW(arcA.radius, 2) - POW(a, 2));

            vec2 v = arcA.center + a * (arcB.center - arcA.center) / d; 

            vec2 p1 = { v.x + h * (arcB.center.y - arcA.center.y) / d, v.y - h * (arcB.center.x - arcA.center.x) / d };
            vec2 p2 = { v.x - h * (arcB.center.y - arcA.center.y) / d, v.y + h * (arcB.center.x - arcA.center.x) / d };

            real theta1a = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(p1 - arcA.center)));
            real theta2a = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(p2 - arcA.center)));
            real theta1b = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(p1 - arcB.center)));
            real theta2b = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(p2 - arcB.center)));
}
#endif


#define BURKARDT_VEC2(p, a) double p[] = { a.x, a.y };

real burkardt_three_point_angle(vec2 p, vec2 center, vec2 q) {
    BURKARDT_VEC2(_1, p);
    BURKARDT_VEC2(_2, center);
    BURKARDT_VEC2(_3, q);
    return (real) angle_rad_2d(_1, _2, _3);
}
