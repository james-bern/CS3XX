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

vec3 RGB255(uint r, uint g, uint b) {
    return V3(r, g, b) / 255.0f;
}

struct {
    vec3 red        = RGB255(249,  38, 114);
    vec3 orange     = RGB255(253, 151,  31);
    // not actual monokai yellow cause i don't like it
    vec3 yellow     = RGB255(255, 255,  50);
    vec3 green      = RGB255(166, 226,  46);
    vec3 blue       = RGB255(102, 217, 239);
    vec3 purple     = RGB255(174, 129, 255);
    vec3 white      = RGB255(255, 255, 255); // *shrug*
    vec3 light_gray = RGB255(192, 192, 192); // *shrug*
    vec3 gray       = RGB255(127, 127, 127); // *shrug*
    vec3 dark_gray  = RGB255( 64,  64,  64); // *shrug*
    vec3 black      = RGB255(  0,   0,   0); // *shrug*
    vec3 brown      = RGB255(123,  63,   0); // no actual brown
} monokai;

vec3 color_rainbow_swirl(real t) {
    return {
        (0.5f + 0.5f * COS(TAU * ( 0.000f - t))),
        (0.5f + 0.5f * COS(TAU * ( 0.333f - t))),
        (0.5f + 0.5f * COS(TAU * (-0.333f - t)))
    };
}

