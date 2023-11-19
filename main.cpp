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

struct RectangleMinMax {
    vec2 min;
    vec2 max;

    bool containsPoint(vec2 p) {
        return (IS_BETWEEN(p.x, min.x, max.x) && IS_BETWEEN(p.y, min.y, max.y));
    }

    void getCornersCCW(vec2 *corners) {
        corners[0] = { min.x, min.y };
        corners[1] = { max.x, min.y };
        corners[2] = { max.x, max.y };
        corners[3] = { min.x, max.y };
    }

    void eso() {
        vec2 corners[4]; getCornersCCW(corners);
        for_(i, 4) eso_vertex(corners[i]);
    }

    bool collidesWith(RectangleMinMax other) {
        return
            (IS_BETWEEN(this->min.x, other.min.x, other.max.x) && IS_BETWEEN(this->min.y, other.min.y, other.max.y)) ||
            (IS_BETWEEN(this->max.x, other.min.x, other.max.x) && IS_BETWEEN(this->max.y, other.min.y, other.max.y)) ||
            (IS_BETWEEN(other.min.x, this->min.x, this->max.x) && IS_BETWEEN(other.min.y, this->min.y, this->max.y)) ||
            (IS_BETWEEN(other.max.x, this->min.x, this->max.x) && IS_BETWEEN(other.max.y, this->min.y, this->max.y));


    }
};

void widget_drag(mat4 PV, RectangleMinMax *rect, bool no_resize) {

    vec2 mouse_change_in_position = mouse_get_change_in_position(PV);

    if (!no_resize) {
        { // corners
            vec2 corners[4]; rect->getCornersCCW(corners);
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
    }

    if (cow.mouse_left_held) {
        rect->min += mouse_change_in_position;
        rect->max += mouse_change_in_position;
    }
}

struct Thing {

    // constructors won't take a slot if it is live or reserved
    bool live; // gets drawn and updated (implies reserved)
    bool reserved;

    bool from_WAD__including_lucy_miao;


    int age;
    int max_age;
    int frames_since_fired;
    int frames_since_hit;
    int damage;
    int max_health;


    int update_group;

    int origin_type;

    union { vec2 s;    struct { real x,     y;      }; };
    vec2 v;
    union { vec2 size; struct { real width, height; }; };
    int color;

    bool walker;
    bool barrier;

    vec2 getRadius() { return size / 2; }
    vec2 getCenter() { return s - cwiseProduct(NORMAL_TYPE_n[origin_type], getRadius()); }

    RectangleMinMax getRect() {
        vec2 r = getRadius();
        vec2 c = getCenter();
        return { c - r, c + r };
    }
    void setRect(RectangleMinMax rect) {
        vec2 c = (rect.min + rect.max) / 2;
        vec2 r = (rect.max - rect.min) / 2;
        s = c + cwiseProduct(NORMAL_TYPE_n[origin_type], r);
        size = 2 * r;
    }

    void debug_draw(mat4 PV, int GL_PRIMITIVE, vec3 drawColor, real drawAlpha = 1.0)  {
        RectangleMinMax rect = getRect();
        if (GL_PRIMITIVE == SOUP_LINE_LOOP) {
            eso_begin(PV, SOUP_LINE_LOOP, 6.0, true); eso_color(monokai.black); rect.eso(); eso_end();
            eso_begin(PV, SOUP_LINE_LOOP, 2.0, true); eso_color(drawColor); rect.eso(); eso_end();
        } else {
            ASSERT(GL_PRIMITIVE == SOUP_QUADS);
            eso_begin(PV, SOUP_QUADS); eso_color(drawColor, drawAlpha); rect.eso(); eso_end();
        }
    }

    bool collidesWith(Thing *other) {
        return this->getRect().collidesWith(other->getRect());
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
    LucyLevelState lucy_;
    MiaoLevelState miao_;


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
LucyLevelState *lucy_ = &level->lucy_;
MiaoLevelState *miao_ = &level->miao_;

// "iterators"
void _for_next_live(Thing **thing_ptr) {
    do { ++*thing_ptr; } while (!((*thing_ptr)->live));
}
#define for_each_live_thing(name) for (Thing *name = things; name < (things + THINGS_ARRAY_LENGTH); _for_next_live(&name))
#define for_each_live_thing_skipping_lucy_and_miao(name) for (Thing *name = (things + 2); name < (things + THINGS_ARRAY_LENGTH); _for_next_live(&name))

#define _for_each_slot(name) for (Thing *name = things; name < (things + THINGS_ARRAY_LENGTH); ++name)






// constructors
// Things are only created through this constructor.
// (zero is sorta kinda not really initialization)
Thing *Reserve_Slot() {
    Thing *result = things;
    while (result->live || result->reserved) ++result;
    result->reserved = true;
    return result;
}

Thing Prefab() {
    return {};
}

Thing *Programmatic_Reserved_Dead_Slot() {
    if (game->reseting_level) {
        ++level->_num_programmatic_things;
        return (level->_programmatic_things_recovery_array[frame->_programmatic_things_recovery_index++] = Reserve_Slot());
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
    for_each_live_thing_skipping_lucy_and_miao(thing) {
        if (!thing->from_WAD__including_lucy_miao) continue;
        fprintf(file, "\nTHING\n");
        fprintf(file, "max_health %d\n", thing->max_health);
        fprintf(file, "origin_type %d\n", thing->origin_type);
        fprintf(file, "update_group %d\n", thing->update_group);
        fprintf(file, "s %.2lf %.2lf\n", thing->s[0], thing->s[1]);
        fprintf(file, "size %.2lf %.2lf\n", thing->size[0], thing->size[1]);
        fprintf(file, "color %d\n", thing->color);
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
                curr = Reserve_Slot();
                curr->live = true;
                curr->from_WAD__including_lucy_miao = true;
            } else {
                if (poe_matches_prefix(line, "max_health ")) {
                    sscanf(line, "%s %d", prefix, &curr->max_health);
                } else if (poe_matches_prefix(line, "origin_type ")) {
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
        mat4 PV = camera_get_PV(&camera);
        vec2 mouse_position = mouse_get_position(PV);








        if (cow.key_pressed[COW_KEY_TAB]) {
            // TODO: Prompt to save when tabbing out of editor
            game->reseting_level = true;
            game->mode = (game->mode == MODE_GAME) ? MODE_EDITOR : MODE_GAME;
        }

        gui_printf((game->mode == MODE_GAME) ? "GAME" : "EDITOR");
        gui_readout("level", &game->level_index);
        {
            int num_live_things = 0;
            int num_reserved_slots = 0;
            int num_empty_slots = THINGS_ARRAY_LENGTH;
            for_each_live_thing(thing) ++num_live_things;
            _for_each_slot(slot) if (slot->reserved) ++num_reserved_slots;
            _for_each_slot(slot) {
                if (slot->reserved || slot->live) --num_empty_slots;
            }
            gui_readout("num_live_things", &num_live_things);
            gui_readout("num_reserved_slots", &num_reserved_slots);
            gui_readout("num_empty_slots", &num_empty_slots);
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
                lucy->reserved = true;
                lucy->live = true;
                lucy->size = { 4, 8 };
                lucy->color = RED;
                lucy->origin_type = NORMAL_TYPE_LOWER_MIDDLE;
                lucy->from_WAD__including_lucy_miao = true;

                miao->reserved = true;
                miao->live = true;
                miao->size = { 4, 4 };
                miao->color = BLUE;
                miao->origin_type = NORMAL_TYPE_LOWER_MIDDLE;
                miao->from_WAD__including_lucy_miao = true;
            }

            printf("[0000] FORNOW\n");
            if (game->level_index == 1) load_WAD();
        }



        // UPDATE_AND_DRAW (could be an instance method of the level class)

        if (game->mode == MODE_EDITOR) { // editor
                                         // TODO: should be able to editor_hot_thing load game logic

            if (gui_button("save", 's')) {
                save_WAD();
            }

            Thing *editor_hot_thing = level->_editor_mouse_currently_pressed_thing;
            if (!editor_hot_thing) {
                for_each_live_thing(thing) {
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
                    editor_hot_thing->debug_draw(PV, SOUP_LINE_LOOP, monokai.orange);
                }
                if (level->editor_selected_thing) {
                    ASSERT(level->editor_selected_thing->from_WAD__including_lucy_miao);
                    level->editor_selected_thing->debug_draw(PV, SOUP_LINE_LOOP, monokai.yellow);
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
                        if (editor_hot_thing) {
                            RectangleMinMax rect = editor_hot_thing->getRect();
                            widget_drag(PV, &rect, (editor_hot_thing == lucy) || (editor_hot_thing == miao));
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
                                Thing *thing = Reserve_Slot();
                                *thing = game->editor_clipboard_thing;
                                thing->s = mouse_position;
                                thing->from_WAD__including_lucy_miao = true;
                            }
                        }
                    }
                }


                { //  sliders and checkboxes
                    Thing *thing = level->editor_selected_thing;
                    if (thing) {
                        gui_readout("live", &thing->live);
                        gui_readout("reserved", &thing->reserved);
                        if ((thing != lucy) && (thing != miao)) {
                            gui_slider("max_health", &thing->max_health, 0, 16);
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
                    // TODO: will this involve prefabs in some way?





                    // TODO: LEVEL tag in WAD




                } else

                    // // TODO: SHIVA level
                    // TODO: hand flipping sides
                    // TODO: programmatic rain
                    // TODO: boss up top
                    if (game->level_index == 1) {

                        // TODO: load by ID again (need both)
                        Thing *hand = Programmatic_Reserved_Dead_Slot();
                        // Thing *hand = WAD_Reserved_Dead_Thing(1);

                        // NOTE: we may actually want a prefab to reset all the crap
                        //       (TODOLATERBUTSOON)

                        if (game->reseting_level) {
                            hand->size = V2(24.0);
                            // hand->live = true;
                        } else if ((game->mode == MODE_GAME) && !game->paused) {


                            if (!hand->live) {
                                // TODO: hand = Prefab("hand");
                                hand->live = true;
                                hand->x = 20;
                            }

                            // TODO: hand->SHIVA_UPDATE();
                            hand->v = 0.3 * normalized(lucy->s - hand->s);
                            hand->s += hand->v;

                        }

                    }
                    else if (game->level_index == 2) {

                    }

                // common (across levels) updates
                if ((game->mode == MODE_GAME) && !game->paused) {
                    { // lucy and miao
                        if (cow.key_held['a']) lucy->x -= 0.4;
                        if (cow.key_held['d']) lucy->x += 0.4;

                        ++lucy->frames_since_fired;
                        if (cow.key_pressed['k']) lucy->frames_since_fired = 0;
                        if (cow.key_held['k'] && IS_DIVISIBLE_BY(lucy->frames_since_fired, 12)) {
                            lucy->frames_since_fired = 0;
                            Thing *bullet = Reserve_Slot();
                            bullet->live = true;
                            bullet->max_age = 128;
                            bullet->update_group = UPDATE_GROUP_BULLET;
                            bullet->s = lucy->s + V2(0.0, 4.0);
                            bullet->v = { 1.0, 0.0 };
                            bullet->color = RED;
                            bullet->size = { 2.0, 2.0 };
                        }
                    }



                    // special updates
                    for_each_live_thing_skipping_lucy_and_miao(thing) {
                        if (thing->update_group == UPDATE_GROUP_BULLET) {
                            thing->s += thing->v;
                        }
                        if ((thing->max_age) && (thing->age++ > thing->max_age)) {
                            *thing = {};
                        }
                    }


                    // bullet collision
                    for_each_live_thing_skipping_lucy_and_miao(bullet) {
                        if (bullet->update_group != UPDATE_GROUP_BULLET) continue;
                        for_each_live_thing_skipping_lucy_and_miao(other) {
                            if (other->update_group == UPDATE_GROUP_BULLET) continue;

                            if (bullet->collidesWith(other)) {
                                other->frames_since_hit = 0;
                                ++other->damage;
                                if (other->damage >= other->max_health) other->live = false;
                                // TODO: shiva update

                                /* bullet->live = false; */
                                /* bullet->reserved = false; */
                                *bullet = {}; // FORNOW
                            }
                        }
                    }



                }
            }

            { // draw
                for_each_live_thing(thing) {
                    vec3 color = V3(thing->color & RED, (thing->color & GREEN) / GREEN, (thing->color & BLUE) / BLUE);
                    real alpha = ((game->mode == MODE_EDITOR) && (!thing->from_WAD__including_lucy_miao)) ? 0.6 : 1.0;
                    thing->debug_draw(PV, SOUP_QUADS, color, alpha);
                }
                if (game->mode == MODE_EDITOR) {
                    { // draw origin
                        Thing *thing = level->editor_selected_thing;
                        if (thing) {
                            eso_begin(PV, SOUP_POINTS, 12, true); eso_color(monokai.black); eso_vertex(thing->s); eso_end();
                            eso_begin(PV, SOUP_POINTS, 8, true); eso_color(monokai.yellow); eso_vertex(thing->s); eso_end();
                        }
                    }
                    { // draw update group
                        for_each_live_thing(thing) {
                            if (thing->update_group) {
                                char text[8] = {};
                                sprintf(text, "%d", thing->update_group);
                                text_draw(PV, text, thing->getCenter(), monokai.black, 0, {}, true);
                            }
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



