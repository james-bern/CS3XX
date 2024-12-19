struct {
    vec3 red     = { 1.0f, 0.0f, 0.0f };
    vec3 green   = { 0.0f, 1.0f, 0.0f };
    vec3 blue    = { 0.0f, 0.0f, 1.0f };
    vec3 yellow  = { 1.0f, 1.0f, 0.0f };
    vec3 cyan    = { 0.0f, 1.0f, 1.0f };
    vec3 magenta = { 1.0f, 0.0f, 1.0f };
    vec3 white         = V3(1.0f);
    vec3 lightest_gray = V3(0.9375f);
    vec3 lighter_gray  = V3(0.875f);
    vec3 light_gray    = V3(0.75f);
    vec3 gray          = V3(0.5f);
    vec3 dark_gray     = V3(0.25f);
    vec3 darker_gray   = V3(0.125f);
    vec3 darkest_gray  = V3(0.0625f);
    vec3 black         = V3(0.0f);
} basic;

vec3 RGB255(uint r, uint g, uint b) {
    return V3(real(r), real(g), real(b)) / 255.0f;
}

struct {
    // NOTE: not actual monokai yellow cause i don't like it
    vec3 red        = RGB255(249,  38, 114);
    vec3 orange     = RGB255(253, 151,  31);
    vec3 yellow     = RGB255(255, 255,  50);
    vec3 green      = RGB255(166, 226,  46);
    vec3 blue       = RGB255(102, 217, 239);
    vec3 indigo     = RGB255(138, 173, 247);
    vec3 violet     = RGB255(174, 129, 255);
    vec3 white      = RGB255(255, 255, 255); // *shrug*
    vec3 light_gray = RGB255(192, 192, 192); // *shrug*
    vec3 gray       = RGB255(127, 127, 127); // *shrug*
    vec3 dark_gray  = RGB255( 64,  64,  64); // *shrug*
    vec3 black      = RGB255(  0,   0,   0); // *shrug*
    vec3 brown      = RGB255(123,  63,   0); // no actual brown
} monokai;

vec3 get_kelly_color(int i) {
    static vec3 _kelly_colors[]={{255.f/255,179.f/255,0.f/255},{128.f/255,62.f/255,117.f/255},{255.f/255,104.f/255,0.f/255},{166.f/255,189.f/255,215.f/255},{193.f/255,0.f/255,32.f/255},{206.f/255,162.f/255,98.f/255},{129.f/255,112.f/255,102.f/255},{0.f/255,125.f/255,52.f/255},{246.f/255,118.f/255,142.f/255},{0.f/255,83.f/255,138.f/255},{255.f/255,122.f/255,92.f/255},{83.f/255,55.f/255,122.f/255},{255.f/255,142.f/255,0.f/255},{179.f/255,40.f/255,81.f/255},{244.f/255,200.f/255,0.f/255},{127.f/255,24.f/255,13.f/255},{147.f/255,170.f/255,0.f/255},{89.f/255,51.f/255,21.f/255},{241.f/255,58.f/255,19.f/255},{35.f/255,44.f/255,22.f/255}};
    return _kelly_colors[MODULO(i, ARRAY_LENGTH(_kelly_colors))];
}

vec3 color_rainbow_swirl(real t) {
    return {
        (0.5f + 0.5f * COS(TAU * ( 0.000f - t))),
        (0.5f + 0.5f * COS(TAU * ( 0.333f - t))),
        (0.5f + 0.5f * COS(TAU * (-0.333f - t)))
    };
}

