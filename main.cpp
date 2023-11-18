// TODO: Copy and Paste Thing's

//  vertical slice
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



// WAD_ID -> update_group_ID


#define BLACK   0
#define RED     1
#define GREEN   2
#define YELLOW  3
#define BLUE    4
#define MAGENTA 5
#define CYAN    6
#define WHITE   7

#define ORIGIN_NORMAL_TYPE_CENTER_MIDDLE 0
#define ORIGIN_NORMAL_TYPE_UPPER_MIDDLE  1
#define ORIGIN_NORMAL_TYPE_UPPER_RIGHT   2
#define ORIGIN_NORMAL_TYPE_CENTER_RIGHT  3
#define ORIGIN_NORMAL_TYPE_LOWER_RIGHT   4
#define ORIGIN_NORMAL_TYPE_LOWER_MIDDLE  5
#define ORIGIN_NORMAL_TYPE_LOWER_LEFT    6
#define ORIGIN_NORMAL_TYPE_CENTER_LEFT   7
#define ORIGIN_NORMAL_TYPE_UPPER_LEFT    8
vec2 ORIGIN_NORMAL_TYPE_n[]={{0,0},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0},{1,1}};

struct Thing {
    bool live;
    bool has_nonzero_WAD_ID;
    bool from_WAD__including_lucy_miao;

    int origin_normal_type;

    union { vec2 s;    struct { real x,     y;      }; };
    vec2 v;
    union { vec2 size; struct { real width, height; }; };
    int color;
    bool mobile;

    bool walker;
    bool barrier;

    bool containsPoint(vec2 p) {
        vec2 r = size / 2;
        vec2 n = ORIGIN_NORMAL_TYPE_n[origin_normal_type];
        return (
                (p.x > s.x + (-1 - n.x) * r.x) &&
                (p.x < s.x + ( 1 - n.x) * r.x) &&
                (p.y > s.y + (-1 - n.y) * r.y) &&
                (p.y < s.y + ( 1 - n.y) * r.y)
               );
    }
    void eso_quad() {
        vec2 r = size / 2;
        vec2 n = ORIGIN_NORMAL_TYPE_n[origin_normal_type];
        eso_vertex(x + ( 1 - n.x) * r.x, y + ( 1 - n.y) * r.y);
        eso_vertex(x + ( 1 - n.x) * r.x, y + (-1 - n.y) * r.y);
        eso_vertex(x + (-1 - n.x) * r.x, y + (-1 - n.y) * r.y);
        eso_vertex(x + (-1 - n.x) * r.x, y + ( 1 - n.y) * r.y);
    }
};

// Is this a good idea? Might we want to play the logic in the editor?
#define MODE_GAME 0
#define MODE_EDITOR 1

struct FrameState {
    int _programmatic_things_recovery_index;
};

struct LucyLevelState {
};

struct MiaoLevelState {
};

struct LevelState {
    FrameState frame;


    // extra state that isn't needed for all Thing's
    struct {

    } lucy_;

    struct { 

    } miao_;


    #define THINGS_ARRAY_LENGTH 256
    Thing things[THINGS_ARRAY_LENGTH];
    Thing *_programmatic_things_recovery_array[THINGS_ARRAY_LENGTH];
    Thing *_WAD_things_recovery_array[THINGS_ARRAY_LENGTH];

    int num_scriptable_things;

    Thing *_editor_mouse_currently_pressed_thing;
    Thing *editor_selected_thing;
    int editor_new_WAD_ID = 1;

};

struct GameState {
    LevelState level;
    int level_index;
    int mode;
    bool reseting_level;
    bool paused;

    Thing editor_clipboard_thing;
};






GameState *game = (GameState *) calloc(1, sizeof(GameState));
#define cow globals // FORNOW


// aliases
LevelState *level = &game->level;
FrameState *frame = &level->frame;
Thing *things = level->things;
Thing *lucy = &things[0];
Thing *miao = &things[1];
Thing **_WAD_things_recovery_array = level->_WAD_things_recovery_array;

// "iterators"
#define for_each_thing for (Thing *thing = things; thing < (things + THINGS_ARRAY_LENGTH); ++thing)
#define for_each_thing_skipping_lucy_and_miao for (Thing *thing = (things + 2); thing < (things + THINGS_ARRAY_LENGTH); ++thing)



int _WAD_recover_ID__LINEAR_RUNTIME(Thing *thing) {
    ASSERT(thing);
    ASSERT(thing->from_WAD__including_lucy_miao);
    ASSERT(thing->has_nonzero_WAD_ID);
    ASSERT(thing != lucy);
    ASSERT(thing != miao);

    for (Thing **other = _WAD_things_recovery_array; other < _WAD_things_recovery_array + THINGS_ARRAY_LENGTH; ++other) {
        if (thing == *other) return (other - _WAD_things_recovery_array);
    }
    ASSERT(0);
    return 0;
}




// constructors
// Things are only created through this constructor.
// (zero is sorta kinda not really initialization)
Thing *New_Live_Thing() {
    Thing *result = things;
    while (result->live) ++result;

    result->live = true;

    return result;
}

Thing *Programmatic_Thing() {
    if (game->reseting_level) {
        ++level->num_scriptable_things;
        return (level->_programmatic_things_recovery_array[frame->_programmatic_things_recovery_index++] = New_Live_Thing());
    }
    ASSERT(frame->_programmatic_things_recovery_index < level->num_scriptable_things);
    return level->_programmatic_things_recovery_array[frame->_programmatic_things_recovery_index++];
}


Thing *WAD_Thing(int WAD_ID) {
    ASSERT(WAD_ID > 0);
    ASSERT(WAD_ID < THINGS_ARRAY_LENGTH);
    Thing *result = _WAD_things_recovery_array[WAD_ID];
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
    for (Thing **ptr = _WAD_things_recovery_array; ptr < _WAD_things_recovery_array + THINGS_ARRAY_LENGTH; ++ptr) {
        if (!*ptr) continue;
        _save_WAD_helper(file, *ptr, (ptr - _WAD_things_recovery_array));
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
        char prefix[512]; // FORNOW
        char line[512];
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
                if (curr->has_nonzero_WAD_ID) _WAD_things_recovery_array[curr_wad_id] = curr;
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






void CatGame() {

    game->reseting_level = true;
    game->level_index = 1;

    window_set_clear_color(0.5, 0.5, 0.5);
    while (cow_begin_frame()) {


        memset(frame, 0, sizeof(FrameState));



        // cow stuff (try to keep it up here)
        camera_move(&camera);
        mat4 transform_for_drawing_and_picking = camera_get_PV(&camera);
        vec2 mouse_position = mouse_get_position(transform_for_drawing_and_picking);
        vec2 mouse_change_in_position = mouse_get_change_in_position(transform_for_drawing_and_picking);





        gui_printf((game->mode == MODE_GAME) ? "GAME" : "EDITOR");
        gui_readout("level", &game->level_index);
        {
            int num_things = 0;
            for_each_thing {
                if (!thing->live) continue;
                ++num_things;
            }
            gui_readout("# things", &num_things);
        }
        gui_printf("-----------");




        if (gui_button("reset", 'r')) game->reseting_level = true;
        if (game->mode == MODE_GAME) gui_checkbox("paused", &game->paused, 'p');

        if (cow.key_pressed[COW_KEY_ARROW_LEFT] || cow.key_pressed[COW_KEY_ARROW_RIGHT]) {
            game->reseting_level = true;
            if (cow.key_pressed[COW_KEY_ARROW_LEFT])  --game->level_index;
            if (cow.key_pressed[COW_KEY_ARROW_RIGHT]) ++game->level_index;
        }

        if (cow.key_pressed[COW_KEY_TAB]) {
            // TODO: Prompt to save when tabbing out of editor
            game->reseting_level = true;
            game->mode = (game->mode == MODE_GAME) ? MODE_EDITOR : MODE_GAME;
        }





        if (game->reseting_level) {
            memset(level, 0, sizeof(LevelState));

            {
                lucy->live = true;
                lucy->size = { 4, 8 };
                lucy->color = RED;
                lucy->origin_normal_type = ORIGIN_NORMAL_TYPE_LOWER_MIDDLE;
                lucy->mobile = true;
                lucy->from_WAD__including_lucy_miao = true;

                miao->live = true;
                miao->size = { 4, 4 };
                miao->color = BLUE;
                miao->origin_normal_type = ORIGIN_NORMAL_TYPE_LOWER_MIDDLE;
                miao->mobile = true;
                miao->from_WAD__including_lucy_miao = true;
            }

            load_WAD();
        }



        // UPDATE_AND_DRAW (could be an instance method of the level class)

        if (game->mode == MODE_EDITOR) { // editor
                                        // TODO: should be able to hot load game logic

            if (gui_button("save", 's')) {
                save_WAD();
            }

            Thing *hot = level->_editor_mouse_currently_pressed_thing;
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
                if (level->editor_selected_thing) {
                    ASSERT(level->editor_selected_thing->from_WAD__including_lucy_miao);
                    eso_begin(transform_for_drawing_and_picking, SOUP_LINE_LOOP, 6.0, true); eso_color(monokai.black); level->editor_selected_thing->eso_quad(); eso_end();
                    eso_begin(transform_for_drawing_and_picking, SOUP_LINE_LOOP, 2.0, true); eso_color(monokai.yellow); level->editor_selected_thing->eso_quad(); eso_end();
                }
            }


            { // UI
                if (hot && cow.mouse_left_pressed) {
                    level->_editor_mouse_currently_pressed_thing = hot;
                    level->editor_selected_thing = level->_editor_mouse_currently_pressed_thing;
                }
                if (level->_editor_mouse_currently_pressed_thing && cow.mouse_left_held) {
                    level->_editor_mouse_currently_pressed_thing->s += mouse_change_in_position; // FORNOW ?
                }
                if (cow.mouse_left_released) {
                    level->_editor_mouse_currently_pressed_thing = NULL;
                }

                { // copy and paste
                    if (level->editor_selected_thing) {
                        if (cow.key_pressed['c']) memcpy(&game->editor_clipboard_thing, level->editor_selected_thing, sizeof(Thing));
                        if (cow.key_pressed['x']) {
                            *level->editor_selected_thing = {};
                            level->editor_selected_thing = NULL;
                        }
                    }

                    if (game->editor_clipboard_thing.live && cow.key_pressed['v']) {
                        Thing *thing = New_Live_Thing();
                        *thing = game->editor_clipboard_thing;
                        thing->s = mouse_position;
                        thing->from_WAD__including_lucy_miao = true;
                        thing->has_nonzero_WAD_ID = false;
                    }
                }

                if (level->editor_selected_thing) {
                    if ((level->editor_selected_thing != lucy) && (level->editor_selected_thing != miao)) {
                        gui_slider("color", &level->editor_selected_thing->color, BLACK, WHITE);
                        gui_slider("origin", &level->editor_selected_thing->origin_normal_type, 0, _COUNT_OF(ORIGIN_NORMAL_TYPE_n) - 1);
                        if (level->editor_selected_thing->has_nonzero_WAD_ID) {
                            gui_printf("WAD_ID %d", _WAD_recover_ID__LINEAR_RUNTIME(level->editor_selected_thing));
                        } else {
                            while (_WAD_things_recovery_array[level->editor_new_WAD_ID]) ++level->editor_new_WAD_ID;
                            gui_slider("new WAD_ID", &level->editor_new_WAD_ID, 1, THINGS_ARRAY_LENGTH - 1);
                            level->editor_new_WAD_ID = CLAMP(level->editor_new_WAD_ID, 1, THINGS_ARRAY_LENGTH - 1);

                            if (gui_button("set WAD_ID")) {
                                ASSERT(level->editor_new_WAD_ID != 0);
                                ASSERT(!_WAD_things_recovery_array[level->editor_new_WAD_ID]);
                                level->editor_selected_thing->has_nonzero_WAD_ID = true;
                                _WAD_things_recovery_array[level->editor_new_WAD_ID] = level->editor_selected_thing;
                            }
                        }
                    }
                }
            }
        }

        { // game
            { // update

                // level-specific Thing's and updates
                if (game->level_index == 0) {
                    // TODO: special level with all the different kinds of everything (like blow did for sokoban)









                } if (game->level_index == 1) {
                    Thing *green = WAD_Thing(1);
                    Thing *cyan = WAD_Thing(2);
                    Thing *magenta = Programmatic_Thing();
                    Thing *platform = Programmatic_Thing();
                    if (game->reseting_level) {
                        cyan->mobile = true;
                        cyan->v = { 0.5, 0.5 }; // customizing WAD thing

                        platform->size = { 16, 48 };
                        platform->color = WHITE;
                        platform->origin_normal_type = ORIGIN_NORMAL_TYPE_UPPER_MIDDLE;
                        magenta->size = { 4, 4 };
                        magenta->color = MAGENTA;
                        magenta->s = { 0, 12 };
                    } else if ((game->mode == MODE_GAME) && !game->paused) {
                        green->y -= 0.1;
                        magenta->x -= 0.1;
                    }
                }
                else if (game->level_index == 2) {
                    if (game->reseting_level) {
                        Thing *platform = Programmatic_Thing();
                        platform->size = { 128, 32 };
                        platform->color = WHITE;
                        platform->origin_normal_type = ORIGIN_NORMAL_TYPE_UPPER_MIDDLE;
                    }
                }

                // common (across levels) updates
                if ((game->mode == MODE_GAME) && !game->paused) {
                    { // lucy and miao
                        if (cow.key_held['a']) lucy->x -= 0.4;
                        if (cow.key_held['d']) lucy->x += 0.4;
                    }

                    for_each_thing_skipping_lucy_and_miao {
                        if (!thing->live) continue;
                        if (!thing->mobile) continue;
                        thing->v.y -= 0.01;
                        thing->s += thing->v;
                    }
                }
            }

            { // draw
                eso_begin(transform_for_drawing_and_picking, SOUP_QUADS);
                for_each_thing {
                    if (!thing->live) continue;

                    vec3 color = V3(thing->color & RED, (thing->color & GREEN) / GREEN, (thing->color & BLUE) / BLUE);
                    real a = ((game->mode == MODE_EDITOR) && (!thing->from_WAD__including_lucy_miao)) ? 0.6 : 1.0;
                    eso_color(color, a);
                    thing->eso_quad();
                }
                eso_end();
                if (game->mode == MODE_EDITOR) {
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
        }






        game->reseting_level = false;


    }

}



int main() {
    config.hotkeys_app_next = 0;
    config.hotkeys_app_prev = 0;
    _cow_init();
    _cow_reset();
    CatGame();
    return 0;
}



