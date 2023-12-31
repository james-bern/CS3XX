// #define COW_PATCH_FRAMERATE
// #define COW_PATCH_FRAMERATE_SLEEP
typedef double real;
#define GL_REAL GL_DOUBLE
#define JIM_IS_JIM
#include "include.cpp"

// TODO: vertical slice
// - music
// - sound cues for powerups (like in aggressive alpine skiing)
// - weapon powerups
// - cat powerups
// - full art
// - blue red green blocks, etc.
// - level editor
// - intro sequence where you can shoot and get warmed up
// - cat following you
// - no good graphics

// TODO: level editor

Camera2D camera = { 512.0 };

#define RED   (1 << 0)
#define BLUE  (1 << 1)
#define GREEN (1 << 2)

struct Thing {
    union { vec2 s; struct { real x, y; }; };
    union { vec2 size; struct { real width, height; }; };
    int color;
    bool live;

    void draw() {
        eso_begin(camera_get_PV(&camera), SOUP_QUADS);
        printf("%d\n", color);
        eso_color(color & RED, color & GREEN, color & BLUE);
        eso_vertex(x + width, y + height);
        eso_vertex(x - width, y + height);
        eso_vertex(x - width, y - height);
        eso_vertex(x + width, y - height);
        eso_end();
    }
};


#define THINGS_NUM_SLOTS 256
Thing things[THINGS_NUM_SLOTS];
Thing *lucy = &things[0];
Thing *miao = &things[1];
Thing *one_past_end_of_things = things + THINGS_NUM_SLOTS;

void game() {

    lucy->size = { 4, 8 };
    lucy->color = RED;
    lucy->live = true;

    miao->size = { 4, 4 };
    miao->color = BLUE;
    miao->live = true;

    while (cow_begin_frame()) {

        for (Thing *thing = things; thing < one_past_end_of_things; ++thing) {
            if (!thing->live) continue;
            thing->draw();
        }
    }

}



int main() {
    APPS {
        APP(game);
    }
    return 0;
}



