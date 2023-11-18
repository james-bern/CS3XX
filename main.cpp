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
#define cow globals // FORNOW


Camera2D camera = { 128.0 }; // FORNOW
bool poe_matches_prefix(char *string, char *prefix) { // FORNOW
    if (strlen(string) < strlen(prefix)) return false;
    for_(i, (int) strlen(prefix)) { // FORNOW
        if (string[i] != prefix[i]) return false;
    }
    return true;
}






#define BLACK   0
#define RED     1
#define GREEN   2
#define YELLOW  3
#define BLUE    4
#define MAGENTA 5
#define CYAN    6
#define WHITE   7

#define NORMAL_TYPE_CENTER_MIDDLE 0
#define NORMAL_TYPE_UPPER_MIDDLE  1
#define NORMAL_TYPE_UPPER_RIGHT   2
#define NORMAL_TYPE_CENTER_RIGHT  3
#define NORMAL_TYPE_LOWER_RIGHT   4
#define NORMAL_TYPE_LOWER_MIDDLE  5
#define NORMAL_TYPE_LOWER_LEFT    6
#define NORMAL_TYPE_CENTER_LEFT   7
#define NORMAL_TYPE_UPPER_LEFT    8
vec2 NORMAL_TYPE_n[]={{0,0},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0},{1,1}};

#define UPDATE_GROUP_BULLET 255

struct Rectangle {
    vec2 min;
    vec2 max;

    bool containsPoint(vec2 p) {
        return (IS_BETWEEN(p.x, min.x, max.x) && IS_BETWEEN(p.y, min.y, max.y));
    }

    void getCorners(vec2 *corners) {
        corners[0] = { min.x, min.y };
        corners[1] = { max.x, min.y };
        corners[2] = { max.x, max.y };
        corners[3] = { min.x, max.y };
    }

    void eso() {
        vec2 corners[4]; getCorners(corners);
        for_(i, 4) eso_vertex(corners[i]);
    }
};

void widget_drag(mat4 PV, Rectangle *rect) {

    vec2 mouse_change_in_position = mouse_get_change_in_position(PV);

    { // corners
        vec2 corners[4]; rect->getCorners(corners);
        vec2 *corner = widget_drag(PV, 4, corners);
        if (corner) {
            vec2 opposite = corners[(corner - corners + 2) % 4];
            rect->min = cwiseMin(*corner, opposite);
            rect->max = cwiseMax(*corner, opposite);
            return;
        }
    }

    { // lines

    }

    if (cow.mouse_left_held) {
        rect->min += mouse_change_in_position;
        rect->max += mouse_change_in_position;
    }
}

struct Thing {
    bool live;
    bool from_WAD__including_lucy_miao;

    int update_group;

    int origin_type;

    union { vec2 s;    struct { real x,     y;      }; };
    vec2 v;
    union { vec2 size; struct { real width, height; }; };
    int color;
    bool mobile;

    bool walker;
    bool barrier;

    vec2 getRadius() { return size / 2; }
    vec2 getCenter() { return s - cwiseProduct(NORMAL_TYPE_n[origin_type], getRadius()); }
    Rectangle getRect() {
        vec2 r = getRadius();
        vec2 c = getCenter();
        return { c - r, c + r };
    }
    void setRect(Rectangle rect) {
        vec2 c = (rect.min + rect.max) / 2;
        vec2 r = (rect.max - rect.min) / 2;
        s = c + cwiseProduct(NORMAL_TYPE_n[origin_type], r);
        size = 2 * r;
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

    int _num_programmatic_things;

    Thing *_editor_mouse_currently_pressed_thing;
    Thing *editor_selected_thing;

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


// aliases
LevelState *level = &game->level;
FrameState *frame = &level->frame;
Thing *things = level->things;
Thing *lucy = &things[0];
Thing *miao = &things[1];

// "iterators"
void _for_next_live(Thing **thing_ptr) {
    do { ++*thing_ptr; } while (!((*thing_ptr)->live));
}
#define for_each_live_thing for (Thing *thing = things; thing < (things + THINGS_ARRAY_LENGTH); _for_next_live(&thing))
#define for_each_live_thing_skipping_lucy_and_miao for (Thing *thing = (things + 2); thing < (things + THINGS_ARRAY_LENGTH); _for_next_live(&thing))







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
        ++level->_num_programmatic_things;
        return (level->_programmatic_things_recovery_array[frame->_programmatic_things_recovery_index++] = New_Live_Thing());
    }
    ASSERT(frame->_programmatic_things_recovery_index < level->_num_programmatic_things);
    return level->_programmatic_things_recovery_array[frame->_programmatic_things_recovery_index++];
}




// TODO: orientation flags (go with an int)

void save_WAD() {
    FILE *file = fopen("WAD.txt", "w");
    ASSERT(file);

    // part -1: lucy and miao
    fprintf(file, "LUCY\n");
    fprintf(file, "s     %.2lf %.2lf\n", lucy->s[0], lucy->s[1]);
    fprintf(file, "\nMIAO\n");
    fprintf(file, "s     %.2lf %.2lf\n", miao->s[0], miao->s[1]);
    // part 0: everything else
    for_each_live_thing_skipping_lucy_and_miao {
        if (!thing->from_WAD__including_lucy_miao) continue;
        fprintf(file, "\nTHING\n");
        fprintf(file, "origin_type  %d\n", thing->origin_type);
        fprintf(file, "update_group        %d\n", thing->update_group);
        fprintf(file, "s                   %.2lf %.2lf\n", thing->s[0], thing->s[1]);
        fprintf(file, "size                %.2lf %.2lf\n", thing->size[0], thing->size[1]);
        fprintf(file, "color               %d\n", thing->color);
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
                curr = New_Live_Thing();
                curr->from_WAD__including_lucy_miao = true;
            } else {
                if (poe_matches_prefix(line, "origin_type ")) {
                    sscanf(line, "%s %d", prefix, &curr->origin_type);
                } else if (poe_matches_prefix(line, "update_group ")) {
                    sscanf(line, "%s %d", prefix, &curr->update_group);
                } else if (poe_matches_prefix(line, "s ")) {
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

    window_set_clear_color(0.2, 0.2, 0.2);
    while (cow_begin_frame()) {


        memset(frame, 0, sizeof(FrameState));



        // cow stuff (try to keep it up here)
        camera_move(&camera);
        mat4 transform_for_drawing_and_picking = camera_get_PV(&camera);
        vec2 mouse_position = mouse_get_position(transform_for_drawing_and_picking);








        if (cow.key_pressed[COW_KEY_TAB]) {
            // TODO: Prompt to save when tabbing out of editor
            game->reseting_level = true;
            game->mode = (game->mode == MODE_GAME) ? MODE_EDITOR : MODE_GAME;
        }

        gui_printf((game->mode == MODE_GAME) ? "GAME" : "EDITOR");
        gui_readout("level", &game->level_index);
        {
            int num_things = 0;
            for_each_live_thing {
                ++num_things;
            }
            gui_readout("# things", &num_things);
        }

        if (gui_button("reset", 'r')) game->reseting_level = true;
        gui_checkbox("paused", &game->paused, 'p');

        if (cow.key_pressed[COW_KEY_ARROW_LEFT] || cow.key_pressed[COW_KEY_ARROW_RIGHT]) {
            game->reseting_level = true;
            if (cow.key_pressed[COW_KEY_ARROW_LEFT])  --game->level_index;
            if (cow.key_pressed[COW_KEY_ARROW_RIGHT]) ++game->level_index;
        }






        if (game->reseting_level) {
            memset(level, 0, sizeof(LevelState));

            {
                lucy->live = true;
                lucy->size = { 4, 8 };
                lucy->color = RED;
                lucy->origin_type = NORMAL_TYPE_LOWER_MIDDLE;
                lucy->mobile = true;
                lucy->from_WAD__including_lucy_miao = true;

                miao->live = true;
                miao->size = { 4, 4 };
                miao->color = BLUE;
                miao->origin_type = NORMAL_TYPE_LOWER_MIDDLE;
                miao->mobile = true;
                miao->from_WAD__including_lucy_miao = true;
            }

            load_WAD();
        }



        // UPDATE_AND_DRAW (could be an instance method of the level class)

        if (game->mode == MODE_EDITOR) { // editor
                                         // TODO: should be able to editor_hot_thing load game logic

            if (gui_button("save", 's')) {
                save_WAD();
            }

            Thing *editor_hot_thing = level->_editor_mouse_currently_pressed_thing;
            if (!editor_hot_thing) {
                for_each_live_thing {
                    if (!thing->from_WAD__including_lucy_miao) continue;

                    if (thing->getRect().containsPoint(mouse_position)) {
                        // TODO: closest to getCenter (do later)
                        editor_hot_thing = thing;
                        break;
                    }
                }
            }

            { // draw outlines
                if (editor_hot_thing) {
                    ASSERT(editor_hot_thing->from_WAD__including_lucy_miao);
                    Rectangle rect = editor_hot_thing->getRect();
                    eso_begin(transform_for_drawing_and_picking, SOUP_LINE_LOOP, 6.0, true); eso_color(monokai.black); rect.eso(); eso_end();
                    eso_begin(transform_for_drawing_and_picking, SOUP_LINE_LOOP, 2.0, true); eso_color(monokai.orange); rect.eso(); eso_end();
                }
                if (level->editor_selected_thing) {
                    ASSERT(level->editor_selected_thing->from_WAD__including_lucy_miao);
                    Rectangle rect = level->editor_selected_thing->getRect();
                    eso_begin(transform_for_drawing_and_picking, SOUP_LINE_LOOP, 6.0, true); eso_color(monokai.black); rect.eso(); eso_end();
                    eso_begin(transform_for_drawing_and_picking, SOUP_LINE_LOOP, 2.0, true); eso_color(monokai.yellow); rect.eso(); eso_end();
                }
            }


            { // UI

                { // picking
                    if (editor_hot_thing && cow.mouse_left_pressed) {
                        level->_editor_mouse_currently_pressed_thing = editor_hot_thing;
                        level->editor_selected_thing = level->_editor_mouse_currently_pressed_thing;
                    }
                    if (cow.mouse_left_released) {
                        level->_editor_mouse_currently_pressed_thing = NULL;
                    }
                }

                { 
                    { // dragging
                        if (editor_hot_thing && (editor_hot_thing != lucy) && (editor_hot_thing != miao)) {
                            Rectangle rect = editor_hot_thing->getRect();
                            widget_drag(transform_for_drawing_and_picking, &rect);
                            editor_hot_thing->setRect(rect);
                        }

                    }

                    { // cut, copy and paste
                        if (gui_button("cut", 'x')) {
                            if (editor_hot_thing && (editor_hot_thing != lucy) && (editor_hot_thing != miao)) {
                                if (level->editor_selected_thing == editor_hot_thing) {
                                    level->editor_selected_thing = NULL;
                                }

                                memcpy(&game->editor_clipboard_thing, editor_hot_thing, sizeof(Thing));
                                *editor_hot_thing = {};
                                editor_hot_thing = NULL;
                            } else if (level->editor_selected_thing && (level->editor_selected_thing != lucy) && (level->editor_selected_thing != miao)) {
                                memcpy(&game->editor_clipboard_thing, level->editor_selected_thing, sizeof(Thing));
                                *level->editor_selected_thing = {};
                                level->editor_selected_thing = NULL;
                            }
                        }

                        if (gui_button("copy", 'c')) {
                            if (editor_hot_thing && (editor_hot_thing != lucy) && (editor_hot_thing != miao)) {
                                memcpy(&game->editor_clipboard_thing, editor_hot_thing, sizeof(Thing));
                            } else if (level->editor_selected_thing && (level->editor_selected_thing != lucy) && (level->editor_selected_thing != miao)) {
                                memcpy(&game->editor_clipboard_thing, level->editor_selected_thing, sizeof(Thing));
                            }
                        }

                        if (gui_button("paste", 'v')) {
                            if (game->editor_clipboard_thing.live) {
                                Thing *thing = New_Live_Thing();
                                *thing = game->editor_clipboard_thing;
                                thing->s = mouse_position;
                                thing->from_WAD__including_lucy_miao = true;
                            }
                        }
                    }
                }


                { //  sliders
                    Thing *thing = level->editor_selected_thing;
                    if (thing) {
                        if ((thing != lucy) && (thing != miao)) {
                            gui_readout("s", &thing->s);
                            gui_slider("color", &thing->color, BLACK, WHITE);
                            {
                                int tmp = thing->origin_type;
                                gui_slider("origin type", &thing->origin_type, 0, 8);
                                if (tmp != thing->origin_type) {
                                    thing->s += cwiseProduct(NORMAL_TYPE_n[thing->origin_type] - NORMAL_TYPE_n[tmp], thing->getRadius());
                                }
                            }
                            gui_slider("update_group", &thing->update_group, 0, 255);
                            for_(i, 10) if (cow.key_pressed['0' + i]) thing->update_group = i;
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









                } else
                    if (game->level_index == 1) {
                        Thing *magenta = Programmatic_Thing();
                        Thing *platform = Programmatic_Thing();
                        if (game->reseting_level) {
                            platform->size = { 16, 48 };
                            platform->color = WHITE;
                            platform->origin_type = NORMAL_TYPE_UPPER_MIDDLE;
                            magenta->size = { 4, 4 };
                            magenta->color = MAGENTA;
                            magenta->s = { 0, 12 };

                            for_each_live_thing_skipping_lucy_and_miao {
                                if (thing->update_group == 1) {
                                    thing->v = 0.1 * normalized(lucy->s - thing->s);
                                }
                            }
                        } else if ((game->mode == MODE_GAME) && !game->paused) {
                            for_each_live_thing_skipping_lucy_and_miao {
                                if (thing->update_group == 1) {
                                    thing->s += thing->v;
                                }
                            }
                        }
                    }
                    else if (game->level_index == 2) {
                        if (game->reseting_level) {
                            Thing *platform = Programmatic_Thing();
                            platform->size = { 128, 32 };
                            platform->color = WHITE;
                            platform->origin_type = NORMAL_TYPE_UPPER_MIDDLE;
                        }
                    }

                // common (across levels) updates
                if ((game->mode == MODE_GAME) && !game->paused) {
                    { // lucy and miao
                        if (cow.key_held['a']) lucy->x -= 0.4;
                        if (cow.key_held['d']) lucy->x += 0.4;
                    }

                    for_each_live_thing_skipping_lucy_and_miao {
                        if (!thing->mobile) continue;
                        thing->v.y -= 0.01;
                        thing->s += thing->v;
                    }
                }
            }

            { // draw
                eso_begin(transform_for_drawing_and_picking, SOUP_QUADS);
                for_each_live_thing {

                    vec3 color = V3(thing->color & RED, (thing->color & GREEN) / GREEN, (thing->color & BLUE) / BLUE);
                    real a = ((game->mode == MODE_EDITOR) && (!thing->from_WAD__including_lucy_miao)) ? 0.6 : 1.0;
                    eso_color(color, a);
                    thing->getRect().eso();
                }
                eso_end();
                if (game->mode == MODE_EDITOR) {



                    {
                        Thing *thing = level->editor_selected_thing;
                        if (thing) {
                            eso_begin(transform_for_drawing_and_picking, SOUP_POINTS, 12, true); eso_color(monokai.black); eso_vertex(thing->s); eso_end();
                            eso_begin(transform_for_drawing_and_picking, SOUP_POINTS, 8, true); eso_color(monokai.yellow); eso_vertex(thing->s); eso_end();
                        }
                    }



                    for_each_live_thing {
                        if (thing->update_group) {
                            char text[8] = {};
                            sprintf(text, "%d", thing->update_group);
                            text_draw(
                                    transform_for_drawing_and_picking,
                                    text,
                                    thing->getCenter(),
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



