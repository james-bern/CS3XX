vec3 color_rainbow_swirl(real t) {
    #define Q(o) (.5f + .5f * COS(6.28f * ((o) - t)))
    return V3(Q(0.0f), Q(.33f), Q(-.33f));
    #undef Q
}
