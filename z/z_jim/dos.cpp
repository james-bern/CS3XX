
////////////////////////////////////////
// Data-Oriented Snail /////////////////
////////////////////////////////////////

vec3 get(real32 *x, uint32 i) {
    vec3 result;
    for (uint32 d = 0; d < 3; ++d) result.data[d] = x[3 * i + d];
    return result;
}
void set(real32 *x, uint32 i, vec3 v) {
    for (uint32 d = 0; d < 3; ++d) x[3 * i + d] = v.data[d];
}
void eso_vertex(real32 *p_j) {
    eso_vertex(p_j[0], p_j[1], p_j[2]);
}
void eso_vertex(real32 *p, uint32 j) {
    eso_vertex(p[3 * j + 0], p[3 * j + 1], p[3 * j + 2]);
}

real32 cross(real32 a_x, real32 a_y, real32 b_x, real32 b_y) {
    return a_x * b_y - a_y * b_x;
}

