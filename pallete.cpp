struct Pallete2D {
    uint id;

    vec3 background;
    vec3 foreground;
    vec3 grid;
    vec3 grid_accent;
    vec3 axis;
    vec3 button_background;
    vec3 button_foreground;

    vec3 selection;
    vec3 dots;

    vec3 emphasis;         // WHITE
    vec3 snap;             // PINK
    vec3 draw;             // BLUE
    vec3 drawing_underlay; // GRAY

    vec3 color_0;
    vec3 color_1;
    vec3 color_2;
    vec3 color_3;
    vec3 color_4;
    vec3 color_5;
    vec3 color_6;
    vec3 color_7;
    vec3 color_8;
    vec3 color_9;
};

struct Pallete3D {
    uint id;

    vec3 background;
    vec3 foreground; // text
    vec3 hard_edges;
    vec3 feature_plane;
    vec3 grid;
};

#define PALLETE_2D_LIGHT 0
#define PALLETE_2D_DARK  1
#define PALLETE_3D_LIGHT 2
#define PALLETE_3D_DARK  3

union Pallete {
    struct {
        Pallete2D _2D;
        Pallete3D _3D;
    };
    real _data[128];
};

Pallete2D _pallete_2D_light; 
Pallete2D _pallete_2D_dark; 
Pallete3D _pallete_3D_light; 
Pallete3D _pallete_3D_dark; 
run_before_main {
    _pallete_2D_dark.id = PALLETE_2D_DARK;
    _pallete_2D_dark.axis = basic.white;
    _pallete_2D_dark.background = basic.black;
    _pallete_2D_dark.button_background = basic.darker_gray;
    _pallete_2D_dark.button_foreground = basic.white;
    _pallete_2D_dark.color_0 = basic.light_gray;
    _pallete_2D_dark.color_1 = monokai.red;
    _pallete_2D_dark.color_2 = monokai.orange;
    _pallete_2D_dark.color_3 = monokai.yellow;
    _pallete_2D_dark.color_4 = monokai.green;
    _pallete_2D_dark.color_5 = monokai.blue;
    _pallete_2D_dark.color_6 = monokai.indigo;
    _pallete_2D_dark.color_7 = monokai.violet;
    _pallete_2D_dark.color_8 = monokai.brown;
    _pallete_2D_dark.color_9 = basic.dark_gray;
    _pallete_2D_dark.dots = basic.white;
    _pallete_2D_dark.draw = basic.cyan;
    _pallete_2D_dark.drawing_underlay = basic.gray;
    _pallete_2D_dark.emphasis = basic.white;
    _pallete_2D_dark.foreground = basic.white;
    _pallete_2D_dark.grid = basic.darker_gray;
    _pallete_2D_dark.grid_accent = basic.dark_gray;
    _pallete_2D_dark.selection = basic.white;
    _pallete_2D_dark.snap = basic.magenta;

    _pallete_2D_light = _pallete_2D_dark; // !
    _pallete_2D_light.id = PALLETE_2D_LIGHT;
    _pallete_2D_light.axis = basic.black;
    _pallete_2D_light.background = basic.white;
    _pallete_2D_light.button_background = basic.lightest_gray;
    _pallete_2D_light.button_foreground = basic.black;
    _pallete_2D_light.color_0 = V3(0.62f);
    _pallete_2D_light.color_1 = 0.9f * monokai.red;
    _pallete_2D_light.color_2 = 0.9f * monokai.orange;
    _pallete_2D_light.color_3 = 0.7f * monokai.yellow;
    _pallete_2D_light.color_4 = 0.9f * monokai.green;
    _pallete_2D_light.color_5 = 0.9f * monokai.blue;
    _pallete_2D_light.color_6 = 0.9f * monokai.indigo;
    _pallete_2D_light.color_7 = 0.9f * monokai.violet;
    _pallete_2D_light.color_8 = 0.9f * monokai.brown;
    _pallete_2D_light.color_9 = V3(0.82f);
    _pallete_2D_light.dots = basic.black;
    _pallete_2D_light.drawing_underlay = basic.gray;
    _pallete_2D_light.emphasis = basic.black;
    _pallete_2D_light.foreground = basic.black;
    _pallete_2D_light.grid = V3(0.98f);
    _pallete_2D_light.grid_accent = basic.lightest_gray;
    _pallete_2D_light.selection = basic.black;

    _pallete_3D_light.id = PALLETE_3D_LIGHT;
    _pallete_3D_light.background = basic.white;
    _pallete_3D_light.foreground = basic.darker_gray;
    _pallete_3D_light.hard_edges = basic.black;
    _pallete_3D_light.feature_plane = basic.white;
    _pallete_3D_light.grid = basic.black;

    // TODO: other palletes
};

