struct {
    vec3 red = { 1.0f, 0.0f, 0.0f };
    vec3 green = { 0.0f, 1.0f, 0.0f };
    vec3 blue = { 0.0f, 0.0f, 1.0f };
    vec3 yellow = { 1.0f, 1.0f, 0.0f };
    vec3 cyan = { 0.0f, 1.0f, 1.0f };
    vec3 magenta = { 1.0f, 0.0f, 1.0f };
    vec3 white = { 1.0f, 1.0f, 1.0f };
    vec3 light_gray = { 0.75f, 0.75f, 0.75f };
    vec3 gray = { 0.5f, 0.5f, 0.5f };
    vec3 dark_gray = { 0.25f, 0.25f, 0.25f };
    vec3 black = { 0.0f, 0.0f, 0.0f };
} basic;

vec3 color_rainbow_swirl(real t) {
    return {
        (0.5f + 0.5f * COS(TAU * ( 0.000f - t))),
        (0.5f + 0.5f * COS(TAU * ( 0.333f - t))),
        (0.5f + 0.5f * COS(TAU * (-0.333f - t)))
    };
}

