// TODO: put this in snail

struct LineLineIntersectionResult {
    bool32 is_valid;
    vec2 position;
};

LineLineIntersectionResult line_line_intersection(vec2 a, vec2 b, vec2 c, vec2 d) {
    double p1[2];
    double p2[2];
    double p3[2];
    double p4[2];
    for (uint32 k = 0; k < 2; ++k) {
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
    for (uint32 k = 0; k < 2; ++k) result.position[k] = p[k];
    return result;
}


real32 three_point_angle(vec2 p, vec2 center, vec2 q) {
    double p1[2];
    double p2[2];
    double p3[2];
    for (uint32 k = 0; k < 2; ++k) {
        p1[k] = p[k];
        p2[k] = center[k];
        p3[k] = q[k];
    }
    return (real32) angle_rad_2d(p1, p2, p3);
}
