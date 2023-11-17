// Handmade Hero -- Operating System API, low-level rasterization, ...
// cow.cpp       -- write the game

// #define COW_PATCH_FRAMERATE
// #define COW_PATCH_FRAMERATE_SLEEP
typedef double real;
#define GL_REAL GL_DOUBLE
#define JIM_IS_JIM
#include "include.cpp"


Camera2D camera = { 128.0 }; // FORNOW
bool poe_matches_prefix(char *string, char *prefix) { // FORNOW
    if (strlen(string) < strlen(prefix)) return false;
    for_(i, (int) strlen(prefix)) { // FORNOW
        if (string[i] != prefix[i]) return false;
    }
    return true;
}


// TODO: vertical slice
// - firing bullets
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
// TODO: save_WAD(level_index)
// TODO: load_WAD(level_index)


#define BLACK   0
#define RED     1
#define GREEN   2
#define YELLOW  3
#define BLUE    4
#define MAGENTA 5
#define CYAN    6
#define WHITE   7

// TODO: flags
int2 LOWER_LEFT   = { -1, -1 };
int2 LOWER_MIDLE  = {  0, -1 };
int2 LOWER_RIGHT  = {  1, -1 };
int2 CENTER_LEFT  = { -1,  0 };
int2 CENTER_MIDLE = {  0,  0 };
int2 CENTER_RIGHT = {  1,  0 };
int2 UPPER_LEFT   = { -1,  1 };
int2 UPPER_MIDLE  = {  0,  1 };
int2 UPPER_RIGHT  = {  1,  1 };

// #define ZERO__WAD_ID_PROGRAMMATIC 0

struct Thing {
    bool live;
    /* int WAD_ID; // TODO: should this really exist? */
    bool has_nonzero_WAD_ID;
    bool from_WAD__including_lucy_miao;

    union { vec2 s;    struct { real x,     y;      }; };
    vec2 v;
    union { vec2 size; struct { real width, height; }; };
    int2 origin_flags; // FORNOW
    int color;
    bool mobile;

    bool walker;
    bool barrier;

    bool containsPoint(vec2 p) {
        vec2 r = size / 2;
        vec2 f = V2(origin_flags[0], origin_flags[1]);
        return (
                (p.x > s.x + (-1 - f.x) * r.x) &&
                (p.x < s.x + ( 1 - f.x) * r.x) &&
                (p.y > s.y + (-1 - f.y) * r.y) &&
                (p.y < s.y + ( 1 - f.y) * r.y)
               );
    }
    void eso_quad() {
        vec2 r = size / 2;
        vec2 f = V2(origin_flags[0], origin_flags[1]);
        eso_vertex(x + ( 1 - f.x) * r.x, y + ( 1 - f.y) * r.y);
        eso_vertex(x + ( 1 - f.x) * r.x, y + (-1 - f.y) * r.y);
        eso_vertex(x + (-1 - f.x) * r.x, y + (-1 - f.y) * r.y);
        eso_vertex(x + (-1 - f.x) * r.x, y + ( 1 - f.y) * r.y);
    }
};

// Is this a good idea? Might we want to play the logic in the editor?
#define MODE_GAME 0
#define MODE_EDITOR 1

struct FrameState {
    int _recover_things_index;
};

struct LevelState {
    FrameState frame;
    int powerups_bit_field;

    #define THINGS_ARRAY_LENGTH 256
    Thing things[THINGS_ARRAY_LENGTH];
    Thing *_recover_things[THINGS_ARRAY_LENGTH];
    Thing *_WAD_things[THINGS_ARRAY_LENGTH];

    int num_scriptable_things;
};

struct GameState {
    LevelState level;
    int level_index;
    int mode;
    bool reseting_level;
    bool paused;
};




GameState game;


// aliases
LevelState *level = &game.level;
FrameState *frame = &level->frame;
Thing *things = level->things;
Thing *lucy = &things[0];
Thing *miao = &things[1];
Thing **_WAD_things = level->_WAD_things;

// "iterators"
#define for_each_thing for (Thing *thing = things; thing < (things + THINGS_ARRAY_LENGTH); ++thing)
#define for_each_thing_skipping_lucy_and_miao for (Thing *thing = (things + 2); thing < (things + THINGS_ARRAY_LENGTH); ++thing)

// : how do you envision prototyping?
//  ---------------------------------
// XXX There should be a bunch of THING's with ID 0 (at the bottom of WAD file)
// TODO Then you assign them an ID
// Then you can start scripting
// (Changing the ID has to be done manually across both files when the game is not running)

// TODO: Copy and Paste Thing's


int _WAD_recover_ID__LINEAR_RUNTIME(Thing *thing) {
    ASSERT(thing);
    ASSERT(thing->from_WAD__including_lucy_miao);
    ASSERT(thing->has_nonzero_WAD_ID);
    ASSERT(thing != lucy);
    ASSERT(thing != miao);

    for (Thing **other = _WAD_things; other < _WAD_things + THINGS_ARRAY_LENGTH; ++other) {
        if (thing == *other) return (other - _WAD_things);
    }
    ASSERT(0);
    return 0;
}




// Things are only created through this constructor.
// (zero is sorta kinda not really initialization)
Thing *New_Live_Thing() {
    Thing *result = things;
    while (result->live) ++result;

    result->live = true;

    return result;
}

Thing *Programmatic_Thing() {
    if (game.reseting_level) {
        ++level->num_scriptable_things;
        return (level->_recover_things[frame->_recover_things_index++] = New_Live_Thing());
    }
    ASSERT(frame->_recover_things_index < level->num_scriptable_things);
    return level->_recover_things[frame->_recover_things_index++];
}


Thing *WAD_Thing(int WAD_ID) {
    ASSERT(WAD_ID > 0);
    ASSERT(WAD_ID < THINGS_ARRAY_LENGTH);
    Thing *result = _WAD_things[WAD_ID];
    if (!result) {
        printf("[WAD_Thing] WAD_ID %d not found\n", WAD_ID);
        ASSERT(0);
    }
    ASSERT(result->live);
    return result;
}


// TODO: orientation flags (go with an int)
void _save_WAD_helper(FILE *file, Thing *thing, int WAD_ID) {
    fprintf(file, "\nTHING %d\n", WAD_ID);
    fprintf(file, "s     %.2lf %.2lf\n", thing->s[0], thing->s[1]);
    fprintf(file, "size   %.2lf %.2lf\n", thing->size[0], thing->size[1]);
    fprintf(file, "color  %d\n", thing->color);
}
void save_WAD() {
    FILE *file = fopen("WAD.txt", "w");
    ASSERT(file);

    // part -1: lucy and miao
    fprintf(file, "LUCY\n");
    fprintf(file, "s     %.2lf %.2lf\n", lucy->s[0], lucy->s[1]);
    fprintf(file, "\nMIAO\n");
    fprintf(file, "s     %.2lf %.2lf\n", miao->s[0], miao->s[1]);
    // part 0: nonzero WAD_ID's
    for (Thing **ptr = _WAD_things; ptr < _WAD_things + THINGS_ARRAY_LENGTH; ++ptr) {
        if (!*ptr) continue;
        _save_WAD_helper(file, *ptr, (ptr - _WAD_things));
    }
    // part 1: zero WAD_ID's
    for_each_thing_skipping_lucy_and_miao {
        if (!thing->from_WAD__including_lucy_miao) continue;
        if (thing->has_nonzero_WAD_ID) continue;
        _save_WAD_helper(file, thing, 0);
    }

    fclose(file);
}

void load_WAD() {
    FILE *file = fopen("WAD.txt", "r");
    ASSERT(file);
    {
        static char prefix[512]; // FORNOW
        static char line[512];
        Thing *curr = NULL;
        while (fgets(line, _COUNT_OF(line), file)) {
            if (poe_matches_prefix(line, "LUCY")) {
                curr = lucy;
            } else if (poe_matches_prefix(line, "MIAO")) {
                curr = miao;
            } else if (poe_matches_prefix(line, "THING")) {
                int curr_wad_id = 0;
                sscanf(line, "%s %d", prefix, &curr_wad_id); // TODO: ASSERT all sscanf's
                                                             // TODO: ASSERT that catches THING 1 followed by THING 1
                curr = New_Live_Thing();
                curr->from_WAD__including_lucy_miao = true;
                curr->has_nonzero_WAD_ID = (curr_wad_id != 0);
                if (curr->has_nonzero_WAD_ID) _WAD_things[curr_wad_id] = curr;
            } else {
                if (poe_matches_prefix(line, "s ")) {
                    sscanf(line, "%s %lf %lf", prefix, &curr->s.x, &curr->s.y);
                } else if (poe_matches_prefix(line, "size ")) {
                    sscanf(line, "%s %lf %lf", prefix, &curr->size.x, &curr->size.y);
                } else if (poe_matches_prefix(line, "color ")) {
                    sscanf(line, "%s %d", prefix, &curr->color);
                    ASSERT(curr->color <= WHITE);
                } // TODO: else ASSERT(0);
            }
        }
    } fclose(file);

}






void MiaoTheGame() {

    game.reseting_level = true;
    game.level_index = 1;

    window_set_clear_color(0.5, 0.5, 0.5);
    while (cow_begin_frame()) {

        mat4 transform_for_drawing_and_picking = camera_get_PV(&camera);
        vec2 mouse_position = mouse_get_position(transform_for_drawing_and_picking);
        vec2 mouse_change_in_position = mouse_get_change_in_position(transform_for_drawing_and_picking);


        memset(frame, 0, sizeof(FrameState));



        gui_printf((game.mode == MODE_GAME) ? "GAME" : "EDITOR");
        gui_readout("level", &game.level_index);
        {
            int num_things = 0;
            for_each_thing {
                if (!thing->live) continue;
                ++num_things;
            }
            gui_readout("# things", &num_things);
        }
        gui_printf("-----------");




        if (gui_button("reset", 'r')) game.reseting_level = true;
        if (game.mode == MODE_GAME) gui_checkbox("paused", &game.paused, 'p');

        if (globals.key_pressed[COW_KEY_ARROW_LEFT] || globals.key_pressed[COW_KEY_ARROW_RIGHT]) {
            game.reseting_level = true;
            if (globals.key_pressed[COW_KEY_ARROW_LEFT])  --game.level_index;
            if (globals.key_pressed[COW_KEY_ARROW_RIGHT]) ++game.level_index;
        }

        if (globals.key_pressed[COW_KEY_TAB]) {
            // TODO: Prompt to save when tabbing out of editor
            game.reseting_level = true;
            game.mode = (game.mode == MODE_GAME) ? MODE_EDITOR : MODE_GAME;
        }





        if (game.reseting_level) {
            printf("[game] resetting\n");
            memset(level, 0, sizeof(LevelState));

            {
                lucy->live = true;
                lucy->size = { 4, 8 };
                lucy->color = RED;
                lucy->origin_flags = LOWER_MIDLE;
                lucy->mobile = true;
                lucy->from_WAD__including_lucy_miao = true;

                miao->live = true;
                miao->size = { 4, 4 };
                miao->color = BLUE;
                miao->origin_flags = LOWER_MIDLE;
                miao->mobile = true;
                miao->from_WAD__including_lucy_miao = true;
            }

            load_WAD();
        }

        if (game.mode == MODE_EDITOR) {

            // TODO: should be able to hot load game logic

            if (gui_button("save", 's')) {
                save_WAD();
            }

            static Thing *widgeted;
            static Thing *selected;
            Thing *hot = selected;
            if (!hot) {
                for_each_thing {
                    if (!thing->from_WAD__including_lucy_miao) continue;

                    if (thing->containsPoint(mouse_position)) {
                        // TODO: closest to center (do later)
                        hot = thing;
                        break;
                    }
                }
            }

            { // draw outlines
                if (hot) {
                    ASSERT(hot->from_WAD__including_lucy_miao);
                    eso_begin(transform_for_drawing_and_picking, SOUP_LINE_LOOP, 6.0, true); eso_color(monokai.black); hot->eso_quad(); eso_end();
                    eso_begin(transform_for_drawing_and_picking, SOUP_LINE_LOOP, 2.0, true);
                    eso_color(monokai.orange);
                    hot->eso_quad();
                    eso_end();
                }
                if (widgeted) {
                    ASSERT(widgeted->from_WAD__including_lucy_miao);
                    eso_begin(transform_for_drawing_and_picking, SOUP_LINE_LOOP, 6.0, true); eso_color(monokai.black); widgeted->eso_quad(); eso_end();
                    eso_begin(transform_for_drawing_and_picking, SOUP_LINE_LOOP, 2.0, true); eso_color(monokai.yellow); widgeted->eso_quad(); eso_end();
                }
            }


            { // UI
                if (hot && globals.mouse_left_pressed) {
                    selected = hot;
                    widgeted = selected;
                }
                if (selected && globals.mouse_left_held) {
                    selected->s += mouse_change_in_position; // FORNOW ?
                }
                if (globals.mouse_left_released) {
                    selected = NULL;
                }

                if (widgeted) {
                    if ((widgeted != lucy) && (widgeted != miao)) {
                        gui_slider("color", &widgeted->color, BLACK, WHITE + 1);
                        if (widgeted->has_nonzero_WAD_ID) {
                            gui_printf("WAD_ID %d", _WAD_recover_ID__LINEAR_RUNTIME(widgeted));
                        } else {
                            static int new_WAD_ID = 1;
                            gui_slider("new WAD_ID", &new_WAD_ID, 1, THINGS_ARRAY_LENGTH + 1);
                            while (_WAD_things[new_WAD_ID]) ++new_WAD_ID;
                            if (new_WAD_ID >= THINGS_ARRAY_LENGTH) new_WAD_ID = 1;
                            while (_WAD_things[new_WAD_ID]) ++new_WAD_ID;

                            // if (globals.key_pressed['1'])
                            if (gui_button("set WAD_ID"))
                            {
                                ASSERT(new_WAD_ID != 0);
                               ASSERT(!_WAD_things[new_WAD_ID]);
                                widgeted->has_nonzero_WAD_ID = true;
                                _WAD_things[new_WAD_ID] = widgeted;
                            }
                        }
                    }
                }
            }

        }

        { // game
            if (game.level_index == 0) {
                // TODO: copy and paste level
            } if (game.level_index == 1) {
                // TODO NEXT: actually editing the WAD with the mouse
                Thing *green = WAD_Thing(1);
                Thing *cyan = WAD_Thing(2);
                Thing *magenta = Programmatic_Thing();
                Thing *platform = Programmatic_Thing();
                if (game.reseting_level) {
                    cyan->mobile = true;
                    cyan->v = { 0.5, 0.5 }; // customizing WAD thing

                    platform->size = { 16, 48 };
                    platform->color = WHITE;
                    platform->origin_flags = UPPER_MIDLE;
                    magenta->size = { 4, 4 };
                    magenta->color = MAGENTA;
                    magenta->s = { 0, 12 };
                } else if ((game.mode == MODE_GAME) && !game.paused) {
                    green->y += 0.1;
                    magenta->x -= 0.1;
                }
            } else if (game.level_index == 1) {
                if (game.reseting_level) {
                    Thing *platform = Programmatic_Thing();
                    platform->size = { 128, 32 };
                    platform->color = WHITE;
                    platform->origin_flags = UPPER_MIDLE;
                }
            }

            if ((game.mode == MODE_GAME) && !game.paused) { // common (across levels) updates
                { // lucy and miao
                    if (globals.key_held['a']) lucy->x -= 1.0;
                    if (globals.key_held['d']) lucy->x += 1.0;
                }

                for_each_thing_skipping_lucy_and_miao {
                    if (!thing->live) continue;
                    if (!thing->mobile) continue;
                    thing->v.y -= 0.01;
                    thing->s += thing->v;
                }
            }
        }

        // common across editor and game
        { // draw
            eso_begin(transform_for_drawing_and_picking, SOUP_QUADS);
            for_each_thing {
                if (!thing->live) continue;

                vec3 color = V3(thing->color & RED, (thing->color & GREEN) / GREEN, (thing->color & BLUE) / BLUE);
                real a = ((game.mode == MODE_EDITOR) && (!thing->from_WAD__including_lucy_miao)) ? 0.6 : 1.0;
                eso_color(color, a);
                thing->eso_quad();
            }
            eso_end();
            if (game.mode == MODE_EDITOR) {
                for_each_thing {
                    if (!thing->live) continue;

                    if (thing->has_nonzero_WAD_ID) {
                        int WAD_ID = _WAD_recover_ID__LINEAR_RUNTIME(thing);
                        char text[8] = {};
                        sprintf(text, "%d", WAD_ID);
                        text_draw(
                                transform_for_drawing_and_picking,
                                text,
                                thing->s,
                                monokai.black,
                                0,
                                {},
                                true);

                    }
                }
            }
        }






        game.reseting_level = false;


    }

}



int main() {
    config.hotkeys_app_next = 0;
    config.hotkeys_app_prev = 0;
    _cow_init();
    _cow_reset();
    MiaoTheGame();
    return 0;
}



