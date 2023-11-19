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

    // constructors won't take a slot if it is is_live or is_reserved
    union {
        bool is_live; // gets drawn and updated (implies is_reserved)
        bool _is_NOT_prefab;
    };
    bool is_reserved; // all things in EDITOR are reserved
                      // reserved but _not_ reserved is a Prefab in GAME
                      // in the editor it is drawn with an X

    int prefab_ID;
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
            eso_begin(PV, SOUP_LINE_LOOP, 4.0, true); eso_color(drawColor, drawAlpha); rect.eso(); eso_end();
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

    int frame_index;


    // extra state that isn't needed for all Thing's
    LucyLevelState lucy_;
    MiaoLevelState miao_;


    #define THINGS_ARRAY_LENGTH 256
    Thing things[THINGS_ARRAY_LENGTH];
    Thing *_programmatic_things_recovery_array[THINGS_ARRAY_LENGTH];

    int _num_programmatic_things;

    Thing *_editor_mouse_currently_pressed_thing;
    Thing *editor_hot_thing;
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
void _for_next_reserved(Thing **thing_ptr) {
    do { ++*thing_ptr; } while (!((*thing_ptr)->is_reserved));
}
#define for_each_reserved_thing(name) for (Thing *name = things; name < (things + THINGS_ARRAY_LENGTH); _for_next_reserved(&name))

#define for_each_reserved_thing_skipping_lucy_and_miao(name) for (Thing *name = (things + 2); name < (things + THINGS_ARRAY_LENGTH); _for_next_reserved(&name))

#define _for_each_slot(name) for (Thing *name = things; name < (things + THINGS_ARRAY_LENGTH); ++name)






// // constructors
// // FORNOW: These are all slow
//
// Things are only created through this constructor.
// (zero is sorta kinda not really initialization)
Thing *_Reserve_Zeroed_Slot() {
    Thing *result = things;
    while (result->is_live || result->is_reserved) ++result; // TODO: live should imply reserved (AUTOMATED CONSISTENCY CHECKS EACH FRAME)
    *result = {};
    result->is_reserved = true;
    return result;
}
Thing *Reserve_Zeroed__Or__Recover_Slot__MUST_BE_SAME_ORDER_EVERY_TIME() {
    if (game->reseting_level) {
        ++level->_num_programmatic_things;
        return (level->_programmatic_things_recovery_array[frame->_programmatic_things_recovery_index++] = _Reserve_Zeroed_Slot());
    }
    ASSERT(frame->_programmatic_things_recovery_index < level->_num_programmatic_things);
    return level->_programmatic_things_recovery_array[frame->_programmatic_things_recovery_index++];
}
//
void Prefab_Copy(Thing *dest, int i) {
    for_each_reserved_thing_skipping_lucy_and_miao(thing) {
        if (thing->prefab_ID == i) {
            ASSERT(!thing->_is_NOT_prefab);
            *dest = *thing;
            dest->prefab_ID = 0;
            return;
        }
    }
    printf("[Prefab_Copy] prefab_ID %d not found.\n", i);
    ASSERT(0);
}
//




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
    for_each_reserved_thing_skipping_lucy_and_miao(thing) {
        if (!thing->from_WAD__including_lucy_miao) continue;
        fprintf(file, "\nTHING\n");
        fprintf(file, "is_live %d\n", thing->is_live);
        fprintf(file, "prefab_ID %d\n", thing->prefab_ID);
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
                curr = _Reserve_Zeroed_Slot();
                curr->from_WAD__including_lucy_miao = true;
            } else {
                if (poe_matches_prefix(line, "is_live ")) {
                    int tmp;
                    sscanf(line, "%s %d", prefix, &tmp);
                    curr->is_live = tmp;
                } else if (poe_matches_prefix(line, "prefab_ID ")) {
                    sscanf(line, "%s %d", prefix, &curr->prefab_ID);
                } else if (poe_matches_prefix(line, "max_health ")) {
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
            _for_each_slot(slot) {
                if (slot->is_live) {
                    ASSERT(slot->is_reserved);
                    ++num_live_things;
                }
                if (slot->is_reserved) ++num_reserved_slots;
                if (slot->is_reserved || slot->is_live) --num_empty_slots;
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
                lucy->is_reserved = true;
                lucy->is_live = true;
                lucy->size = { 4, 8 };
                lucy->color = RED;
                lucy->origin_type = NORMAL_TYPE_LOWER_MIDDLE;
                lucy->from_WAD__including_lucy_miao = true;

                miao->is_reserved = true;
                miao->is_live = true;
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

            // TODO: should be able to hotload game logic

            if (gui_button("save", 's')) {
                save_WAD();
            }

            level->editor_hot_thing = level->_editor_mouse_currently_pressed_thing;
            if (!level->editor_hot_thing) {
                for_each_reserved_thing(thing) {
                    if (!thing->from_WAD__including_lucy_miao) continue;

                    if (thing->getRect().containsPoint(mouse_position)) {
                        // TODO: closest to getCenter (do later)
                        level->editor_hot_thing = thing;
                        break;
                    }
                }
            }



            { // UI

                { // picking
                    if (level->editor_hot_thing && cow.mouse_left_pressed) {
                        level->_editor_mouse_currently_pressed_thing = level->editor_hot_thing;
                        level->editor_selected_thing = level->_editor_mouse_currently_pressed_thing;
                    }
                    if (cow.mouse_left_released) {
                        level->_editor_mouse_currently_pressed_thing = NULL;
                    }
                }

                { 
                    { // dragging
                        if (level->editor_hot_thing) {
                            RectangleMinMax rect = level->editor_hot_thing->getRect();
                            widget_drag(PV, &rect, (level->editor_hot_thing == lucy) || (level->editor_hot_thing == miao));
                            level->editor_hot_thing->setRect(rect);
                        }

                    }

                    { // cut, copy and paste
                        if (gui_button("cut", 'x')) {
                            if (level->editor_hot_thing && (level->editor_hot_thing != lucy) && (level->editor_hot_thing != miao)) {
                                if (level->editor_selected_thing == level->editor_hot_thing) {
                                    level->editor_selected_thing = NULL;
                                }

                                memcpy(&game->editor_clipboard_thing, level->editor_hot_thing, sizeof(Thing));
                                *level->editor_hot_thing = {};
                                level->editor_hot_thing = NULL;
                            } else if (level->editor_selected_thing && (level->editor_selected_thing != lucy) && (level->editor_selected_thing != miao)) {
                                memcpy(&game->editor_clipboard_thing, level->editor_selected_thing, sizeof(Thing));
                                *level->editor_selected_thing = {};
                                level->editor_selected_thing = NULL;
                            }
                        }

                        if (gui_button("copy", 'c')) {
                            if (level->editor_hot_thing && (level->editor_hot_thing != lucy) && (level->editor_hot_thing != miao)) {
                                memcpy(&game->editor_clipboard_thing, level->editor_hot_thing, sizeof(Thing));
                            } else if (level->editor_selected_thing && (level->editor_selected_thing != lucy) && (level->editor_selected_thing != miao)) {
                                memcpy(&game->editor_clipboard_thing, level->editor_selected_thing, sizeof(Thing));
                            }
                        }

                        if (gui_button("paste", 'v')) {
                            if (game->editor_clipboard_thing.is_live) {
                                Thing *thing = _Reserve_Zeroed_Slot();
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
                        ASSERT(thing->is_reserved);
                        {
                            {
                                bool tmp = !thing->is_live;
                                gui_checkbox("is_prefab", &tmp);
                                thing->is_live = !tmp;
                            }

                            gui_slider("prefab_ID", &thing->prefab_ID, 0, 16);
                        }
                        gui_slider("update_group", &thing->update_group, 0, 16);
                        for_(i, 10) if (cow.key_pressed['0' + i]) thing->update_group = i;

                        if ((thing != lucy) && (thing != miao)) {
                            gui_slider("max_health", &thing->max_health, 0, 16);
                            gui_slider("color", &thing->color, BLACK, WHITE);
                            {
                                int tmp = thing->origin_type;
                                gui_slider("origin_type", &thing->origin_type, 0, 8);
                                if (tmp != thing->origin_type) {
                                    thing->s += cwiseProduct(NORMAL_TYPE_n[thing->origin_type] - NORMAL_TYPE_n[tmp], thing->getRadius());
                                }
                            }
                        }
                    }
                }
            }
        }

        { // game
            { // update

                int _level_index = 0;
                #define LEVEL else if (game->level_index == _level_index++)
                #define RESET if (game->reseting_level) 
                #define UPDATE else if ((game->mode == MODE_GAME) && !game->paused) 

                { // level-specific Thing's and updates
                    if (0) {} // *
                    LEVEL {
                        // TODO: special level with all the different kinds of everything (like blow did for sokoban)
                        // TODO: will this involve prefabs in some way?
                        // NEXT: LEVEL tag in WAD
                    } LEVEL {

                        Thing *hand = Reserve_Zeroed__Or__Recover_Slot__MUST_BE_SAME_ORDER_EVERY_TIME();

                        RESET {

                        } UPDATE {
                            if (!hand->is_live) {
                                Prefab_Copy(hand, 1);
                                hand->is_live = true;
                            }
                            hand->s += 0.3 * normalized(lucy->s - hand->s);
                            /* if (hand->frames_since_hit < 8) hand->x += 0.1; */ // NEXT

                            if (IS_DIVISIBLE_BY(level->frame_index, 64)) {
                                Thing *bullet = _Reserve_Zeroed_Slot();
                                bullet->is_live = true;
                                bullet->max_age = 512;
                                bullet->update_group = UPDATE_GROUP_BULLET;
                                bullet->s = lucy->s + V2(0.0, 63.0);
                                bullet->v = { 0.0, -0.5 };
                                bullet->color = RED;
                                bullet->size = { 8.0, 8.0 };
                            }
                        }

                    } LEVEL {

                    }
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
                            Thing *bullet = _Reserve_Zeroed_Slot();
                            bullet->is_live = true;
                            bullet->max_age = 128;
                            bullet->update_group = UPDATE_GROUP_BULLET;
                            bullet->s = lucy->s + V2(0.0, 4.0);
                            bullet->v = { 1.0, 0.0 };
                            bullet->color = RED;
                            bullet->size = { 2.0, 2.0 };
                        }
                    }



                    // special updates
                    for_each_reserved_thing_skipping_lucy_and_miao(thing) {
                        if (!thing->is_live) continue;

                        if (thing->update_group == UPDATE_GROUP_BULLET) {
                            thing->s += thing->v;
                        }
                        if ((thing->max_age) && (thing->age++ > thing->max_age)) {
                            *thing = {};
                        }
                    }


                    // bullet collision
                    for_each_reserved_thing_skipping_lucy_and_miao(bullet) {
                        if (!bullet->is_live) continue;
                        if (bullet->update_group != UPDATE_GROUP_BULLET) continue;

                        for_each_reserved_thing_skipping_lucy_and_miao(other) {
                            if (!other->is_live) continue;
                            if (other->update_group == UPDATE_GROUP_BULLET) continue;

                            if (bullet->collidesWith(other)) {
                                other->frames_since_hit = 0;
                                ++other->damage;
                                if (other->max_health) {
                                    if (other->damage >= other->max_health) other->is_live = false;
                                }
                                // TODO: shiva update

                                /* bullet->is_live = false; */
                                /* bullet->is_reserved = false; */
                                *bullet = {}; // FORNOW
                            }
                        }
                    }



                }
            }

            { // draw
                for_each_reserved_thing(thing) {
                    vec3 color = V3(thing->color & RED, (thing->color & GREEN) / GREEN, (thing->color & BLUE) / BLUE);
                    vec3 inverseColor = V3(1.0) - color;
                    real alpha = ((game->mode == MODE_EDITOR) && (!thing->from_WAD__including_lucy_miao)) ? 0.6 : 1.0;


                    if (game->mode == MODE_GAME) {
                        if (thing->is_live) {
                            thing->debug_draw(PV, SOUP_QUADS, color, alpha);
                        }
                    } else if (game->mode == MODE_EDITOR) {
                        thing->debug_draw(PV, SOUP_QUADS, color, alpha);

                        // X
                        if (!thing->is_live) {
                            eso_begin(PV, SOUP_LINES, 2.0);
                            eso_color(inverseColor, 0.8);
                            vec2 corners[4]; thing->getRect().getCornersCCW(corners);
                            eso_vertex(corners[0]);
                            eso_vertex(corners[2]);
                            eso_vertex(corners[1]);
                            eso_vertex(corners[3]);
                            eso_end();
                        }

                        if ((thing != lucy) && (thing != miao)) { // text
                            if (thing->from_WAD__including_lucy_miao) {
                                if (thing->prefab_ID || thing->update_group) {
                                    char text[16] = {};
                                    sprintf(text, "%d-%d", thing->prefab_ID, thing->update_group);
                                    if (!thing->prefab_ID) text[0] = ' ';
                                    if (!thing->update_group) text[2] = ' ';
                                    text_draw(PV, text, thing->getCenter(), inverseColor, 12, {}, true);
                                }
                            }
                        }

                        { // hot, selected annotations
                            if (thing == level->editor_hot_thing) {
                                thing->debug_draw(PV, SOUP_LINE_LOOP, inverseColor, 0.5);
                            }
                            if (thing == level->editor_selected_thing) {
                                thing->debug_draw(PV, SOUP_LINE_LOOP, inverseColor);

                                // dot
                                eso_begin(PV, SOUP_POINTS, 16, true); eso_color(inverseColor); eso_vertex(thing->s); eso_end();
                            }
                        }
                    }
                }
            }
        }






        // FORNOW down here
        ++level->frame_index;
        game->reseting_level = false;


    }

}



int main() {
    config.hotkeys_app_next = 0;
    config.hotkeys_app_prev = 0;
    config.tweaks_soup_draw_with_rounded_corners_for_all_line_primitives = false;
    _cow_init();
    _cow_reset();
    CatGame();
    return 0;
}



