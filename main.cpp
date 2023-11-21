// TODO: ability for hoo to die

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

// FORNOW: this also stores "facing"
#define CENTER_MIDDLE 0
#define UPPER_MIDDLE  1
#define UPPER_LEFT   2
#define CENTER_LEFT  3
#define LOWER_LEFT   4
#define LOWER_MIDDLE  5
#define LOWER_RIGHT    6
#define CENTER_RIGHT   7
#define UPPER_RIGHT    8
vec2 ORIGIN_n[]={{0,0},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0},{1,1}};

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
        bool overlapX = ((other.min.x < this->max.x) && (this->min.x < other.max.x));
        bool overlapY = ((other.min.y < this->max.y) && (this->min.y < other.max.y));
        return overlapX && overlapY;
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

    // constructors won't take a slot if it is is_live or is_persistent

    bool is_persistent; // even after death 

    union {
        bool is_live;
        bool is_NOT_prefab; // (if it's not live in the editor, it's a prefab)
    };

    bool is_from_WAD__INCLUDES_LUCY_AND_MIAO;

    union {
        int prefab_ID; // !is_live
        int unique_ID; // is_live
    };



    int age;
    int max_age;
    int frames_since_fired;
    int frames_since_hit;
    int damage;
    int max_health;


    int update_group;

    int origin_type;

    bool facing_right() { return ((origin_type == UPPER_RIGHT) || (origin_type == CENTER_RIGHT) || (origin_type == LOWER_RIGHT)); }
    bool facing_left() { return ((origin_type == UPPER_LEFT) || (origin_type == CENTER_LEFT) || (origin_type == LOWER_LEFT)); }
    void flip_x(bool preserve_rectangle = false) {
        if (origin_type == UPPER_RIGHT) origin_type = UPPER_LEFT;
        else if (origin_type == CENTER_RIGHT) origin_type = CENTER_LEFT;
        else if (origin_type == LOWER_RIGHT) origin_type = LOWER_LEFT;
        else if (origin_type == LOWER_LEFT) origin_type = LOWER_RIGHT;
        else if (origin_type == CENTER_LEFT) origin_type = CENTER_RIGHT;
        else if (origin_type == UPPER_LEFT) origin_type = UPPER_RIGHT;
        else ASSERT(0);
        if (preserve_rectangle) x += ORIGIN_n[origin_type].x * width;
    }

    union { vec2 s;    struct { real x,     y;      }; };
    vec2 v;
    union { vec2 size; struct { real width, height; }; };
    int color;

    bool walker;
    bool barrier;

    vec2 getRadius() { return size / 2; }
    vec2 getCenter() { return s - cwiseProduct(ORIGIN_n[origin_type], getRadius()); }

    RectangleMinMax getRect() {
        vec2 r = getRadius();
        vec2 c = getCenter();
        return { c - r, c + r };
    }
    void setRect(RectangleMinMax rect) {
        vec2 c = (rect.min + rect.max) / 2;
        vec2 r = (rect.max - rect.min) / 2;
        s = c + cwiseProduct(ORIGIN_n[origin_type], r);
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
    int _counters_recovery_index;
};

struct LucyLevelState { };
struct MiaoLevelState { };
struct LevelState {
    FrameState frame;

    int frame_index;

    int _counters[64];



    int i;


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
/* void _for_next_persistent(Thing **thing_ptr) { do { ++*thing_ptr; } while (!((*thing_ptr)->is_persistent)); } */
/* #define for_each_persistent_thing(name) for (Thing *name = things; name < (things + THINGS_ARRAY_LENGTH); _for_next_persistent(&name)) */
/* #define for_each_persistent_thing_skipping_lucy_and_miao(name) for (Thing *name = (things + 2); name < (things + THINGS_ARRAY_LENGTH); _for_next_persistent(&name)) */

#define _for_each_slot(name) for (Thing *name = things; name < (things + THINGS_ARRAY_LENGTH); ++name)


// NOTE: if it's dead in the editor, then it's a prefab (you can't be both dead and not a prefab)



// // constructors
// // FORNOW: These are all slow
//
// Things are only created through this constructor.
// (zero is sorta kinda not really initialization)
Thing *_Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_PERSISTENT_OR_FROM_WAD() {
    Thing *result = things;
    while (result->is_live || result->is_persistent || result->is_from_WAD__INCLUDES_LUCY_AND_MIAO) ++result; // TODO: live should imply reserved (AUTOMATED CONSISTENCY CHECKS EACH FRAME)
    *result = {};
    result->is_persistent = true;
    return result;
}
Thing *Acquire_Or_Recover_Persistent_Slot__MUST_BE_SAME_ORDER_EVERY_TIME() {
    if (game->reseting_level) {
        ++level->_num_programmatic_things;
        Thing *result = _Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_PERSISTENT_OR_FROM_WAD();
        result->is_persistent = true;
        level->_programmatic_things_recovery_array[frame->_programmatic_things_recovery_index++] = result;
        return result;
    }
    ASSERT(frame->_programmatic_things_recovery_index < level->_num_programmatic_things);
    return level->_programmatic_things_recovery_array[frame->_programmatic_things_recovery_index++];
}
Thing *Find_Live_Thing_By_Unique_ID(int unique_ID) {
    _for_each_slot(thing) {
        if (!thing->is_live) continue;
        if (thing->unique_ID == unique_ID) {
            return thing;
        }
    }
    return NULL;
}
//
void Prefab_Copy(Thing *dest, int i) {
    _for_each_slot(thing) {
        if (!thing->is_persistent) continue;
        if (thing->is_NOT_prefab) continue;
        if (thing->prefab_ID == i) {
            ASSERT(!thing->is_NOT_prefab);
            *dest = *thing;
            dest->prefab_ID = 0;
            return;
        }
    }
    printf("[Prefab_Copy] prefab_ID %d not found.\n", i);
    ASSERT(0);
}
//
int &Counter__MUST_BE_SAME_ORDER_EVERY_TIME() {
    return level->_counters[frame->_counters_recovery_index++];
}





// TODO: orientation flags (go with an int)

void save_WAD() {
    static char line[512];

    { // write to WAD2
        FILE *source = fopen("WAD.txt", "r"); ASSERT(source);
        FILE *destination = fopen("WAD2.txt", "w"); ASSERT(destination);
        {
            int check_level_index = 0;
            bool is_level_index = false;
            bool puked_current_level = false;
            while (fgets(line, _COUNT_OF(line), source)) {
                if (poe_matches_prefix(line, "LEVEL")) {
                    is_level_index = (game->level_index == check_level_index++);
                }

                if (!is_level_index) { 
                    fprintf(destination, "%s", line);
                } else if (!puked_current_level) {
                    puked_current_level = true;
                    {
                        // part -1: lucy and miao
                        fprintf(destination, "LEVEL\n");
                        fprintf(destination, "LUCY\n");
                        fprintf(destination, "s %.2lf %.2lf\n", lucy->s[0], lucy->s[1]);
                        fprintf(destination, "MIAO\n");
                        fprintf(destination, "s %.2lf %.2lf\n", miao->s[0], miao->s[1]);
                        // part 0: everything else
                        _for_each_slot(thing) {
                            if (!thing->is_from_WAD__INCLUDES_LUCY_AND_MIAO) continue;
                            if (thing == lucy) continue;
                            if (thing == miao) continue;
                            fprintf(destination, "THING\n");
                            fprintf(destination, "color         %d\n", thing->color);
                            fprintf(destination, "is_live       %d\n", thing->is_live);
                            fprintf(destination, "is_persistent %d\n", thing->is_persistent);
                            fprintf(destination, "prefab_ID     %d\n", thing->prefab_ID);
                            fprintf(destination, "max_health    %d\n", thing->max_health);
                            fprintf(destination, "origin_type   %d\n", thing->origin_type);
                            fprintf(destination, "update_group  %d\n", thing->update_group);
                            fprintf(destination, "s    %.2lf %.2lf\n", thing->s[0], thing->s[1]);
                            fprintf(destination, "size %.2lf %.2lf\n", thing->size[0], thing->size[1]);
                        }
                    }
                }
            }
        }
        fclose(source);
        fclose(destination);
    }

    { // WAD <- WAD2
        FILE *source = fopen("WAD2.txt", "r"); ASSERT(source);
        FILE *destination = fopen("WAD.txt", "w"); ASSERT(destination);
        {
            while (fgets(line, _COUNT_OF(line), source)) {
                fprintf(destination, "%s", line);
            }
        }
        fclose(source);
        fclose(destination);
    }
}

void load_WAD() {
    FILE *file = fopen("WAD.txt", "r");
    ASSERT(file);
    {
        static char prefix[512];
        static char line[512];
        Thing *thing = NULL;
        int check_level_index = 0;
        bool is_level_index = false;
        while (fgets(line, _COUNT_OF(line), file)) {
            if (poe_matches_prefix(line, "LEVEL")) {
                is_level_index = (game->level_index == check_level_index++);
            }
            if (is_level_index) {
                if (poe_matches_prefix(line, "LUCY")) {
                    thing = lucy;
                } else if (poe_matches_prefix(line, "MIAO")) {
                    thing = miao;
                } else if (poe_matches_prefix(line, "THING")) {
                    thing = _Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_PERSISTENT_OR_FROM_WAD();
                    thing->is_from_WAD__INCLUDES_LUCY_AND_MIAO = true;
                } else {
                    if (poe_matches_prefix(line, "is_live ")) {
                        int tmp; sscanf(line, "%s %d", prefix, &tmp); thing->is_live = tmp;
                    } else if (poe_matches_prefix(line, "is_persistent ")) {
                        int tmp; sscanf(line, "%s %d", prefix, &tmp); thing->is_persistent = tmp;
                    } else if (poe_matches_prefix(line, "prefab_ID ")) {
                        sscanf(line, "%s %d", prefix, &thing->prefab_ID);
                    } else if (poe_matches_prefix(line, "max_health ")) {
                        sscanf(line, "%s %d", prefix, &thing->max_health);
                    } else if (poe_matches_prefix(line, "origin_type ")) {
                        sscanf(line, "%s %d", prefix, &thing->origin_type);
                    } else if (poe_matches_prefix(line, "update_group ")) {
                        sscanf(line, "%s %d", prefix, &thing->update_group);
                    } else if (poe_matches_prefix(line, "s ")) {
                        sscanf(line, "%s %lf %lf", prefix, &thing->s.x, &thing->s.y);
                    } else if (poe_matches_prefix(line, "size ")) {
                        sscanf(line, "%s %lf %lf", prefix, &thing->size.x, &thing->size.y);
                    } else if (poe_matches_prefix(line, "color ")) {
                        sscanf(line, "%s %d", prefix, &thing->color);
                        ASSERT(thing->color <= WHITE);
                    } // TODO: else ASSERT(0);
                }
            }
        }
    } fclose(file);

}



// TODO: outline color



void CatGame() {

    bool _request_reset = true;
    game->level_index = 1;

    window_set_clear_color(0.2, 0.2, 0.2);
    while (cow_begin_frame()) {
        camera_move(&camera);
        mat4 PV = camera_get_PV(&camera);
        vec2 mouse_position = mouse_get_position(PV);


        { // start of frame
            if (_request_reset) {
                _request_reset = false;
                game->reseting_level = true;
            } else {
                // FORNOW
                if ((game->mode == MODE_GAME) && !game->reseting_level && (!game->paused || cow.key_pressed['.'])) { // FORNOW
                    ++level->frame_index;
                }

                game->reseting_level = false;
                memset(frame, 0, sizeof(FrameState));
            }
        }

        { // ui
            if (cow.key_pressed[COW_KEY_TAB]) {
                // TODO: Prompt to save when tabbing out of editor
                game->reseting_level = true;
                game->mode = (game->mode == MODE_GAME) ? MODE_EDITOR : MODE_GAME;
            }
            gui_printf((game->mode == MODE_GAME) ? "GAME" : "EDITOR");
            gui_readout("LEVEL", &game->level_index);
            gui_readout("frame_index", &level->frame_index);
            {
                int num_persistent_live = 0;
                int num_persistent_dead = 0;
                int num_ephemeral_live = 0;
                int num_empty_slots = THINGS_ARRAY_LENGTH;
                _for_each_slot(slot) {
                    if ( slot->is_persistent &&  slot->is_live) ++num_persistent_live;
                    if (!slot->is_persistent &&  slot->is_live) ++num_ephemeral_live;
                    if ( slot->is_persistent && !slot->is_live) ++num_persistent_dead;
                    if (slot->is_persistent || slot->is_live) --num_empty_slots;
                }
                gui_readout("num_ephemeral_live", &num_ephemeral_live);
                gui_readout("num_persistent_live", &num_persistent_live);
                gui_readout("num_persistent_dead", &num_persistent_dead);
                gui_readout("num_empty_slots", &num_empty_slots);
            }

            if (gui_button("reset", 'r')) game->reseting_level = true;
            gui_checkbox("paused", &game->paused, 'p');
            if (cow.key_pressed[COW_KEY_ARROW_LEFT] || cow.key_pressed[COW_KEY_ARROW_RIGHT]) {
                game->reseting_level = true;
                if (cow.key_pressed[COW_KEY_ARROW_LEFT])  --game->level_index;
                if (cow.key_pressed[COW_KEY_ARROW_RIGHT]) ++game->level_index;
            }
        }

        { // reset update and draw 
            #define RESET if (game->reseting_level) 
            #define WIN_CONDITION__FORNOW_NOT_REALLY_A_CASE(win_condition) else if (win_condition) { memset(level->things + 2, 0, (THINGS_ARRAY_LENGTH  - 2) * sizeof(Thing)); } else if (0)
            #define UPDATE else if ((game->mode == MODE_GAME) && (!game->paused || globals.key_pressed['.'])) 
            { // common
                RESET {
                    memset(level, 0, sizeof(LevelState));

                    {
                        lucy->is_persistent = true;
                        lucy->is_live = true;
                        lucy->size = { 4, 8 };
                        lucy->color = RED;
                        lucy->origin_type = LOWER_RIGHT;
                        lucy->is_from_WAD__INCLUDES_LUCY_AND_MIAO = true;
                        lucy->max_health = 1;

                        miao->is_persistent = true;
                        miao->is_live = true;
                        miao->size = { 4, 4 };
                        miao->color = BLUE;
                        miao->origin_type = LOWER_RIGHT;
                        miao->is_from_WAD__INCLUDES_LUCY_AND_MIAO = true;
                        miao->max_health = 1;
                    }

                    load_WAD();
                } UPDATE {


                    // named group updates
                    _for_each_slot(thing) { // FORNOW: includes lucy and miao
                        if (!thing->is_persistent) continue;

                        if (thing->update_group == UPDATE_GROUP_BULLET) {
                            thing->s += thing->v;
                        }
                        if ((thing->max_age) && (thing->age++ > thing->max_age)) {
                            *thing = {};
                        }
                    }

                    { // lucy and miao
                        {
                            real speed = 0.4;
                            if (cow.key_held['a']) {
                                if (lucy->facing_right()) lucy->flip_x(true);
                                lucy->x -= speed;
                            }
                            if (cow.key_held['d']) {
                                if (lucy->facing_left()) lucy->flip_x(true);
                                lucy->x += speed;
                            }
                        }

                        { // fire
                            ++lucy->frames_since_fired;
                            if (cow.key_pressed['k']) lucy->frames_since_fired = 0;

                            bool firing_vertically = (cow.key_held['w']);
                            int sgn = lucy->facing_right() ? 1 : -1; // TODO: down (once can fall onto platform)

                            if (cow.key_held['k'] && IS_DIVISIBLE_BY(lucy->frames_since_fired, 12)) {
                                lucy->frames_since_fired = 0;
                                Thing *bullet = _Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_PERSISTENT_OR_FROM_WAD();
                                bullet->is_live = true;
                                bullet->max_age = 128;
                                bullet->update_group = UPDATE_GROUP_BULLET;
                                if (!firing_vertically) {
                                    bullet->s = lucy->s + V2(sgn * 4.0, 4.0);
                                } else {
                                    bullet->s = lucy->s + V2(0.0, 10.0);
                                }
                                if (!firing_vertically) {
                                    bullet->v = { sgn * 1.0, 0.0 };
                                } else {
                                    bullet->v = { 0.0, 1.0 };
                                }
                                bullet->color = RED;
                                bullet->size = { 2.0, 2.0 };
                            }
                        }
                    }

                    // killers
                    _for_each_slot(bullet) {
                        if (!bullet->is_live) continue;
                        if (bullet->update_group != UPDATE_GROUP_BULLET) continue;

                        _for_each_slot(other) {
                            if (!other->is_live) continue;
                            if (other->update_group == UPDATE_GROUP_BULLET) continue;

                            if (bullet->collidesWith(other)) {
                                other->frames_since_hit = 0;
                                ++other->damage;
                                if (other->max_health) {
                                    if (other->damage >= other->max_health) other->is_live = false;
                                }
                                // TODO: shiva update

                                bullet->is_live = false;
                            }
                        }
                    }
                }
            }


            if (!lucy->is_live) {
                printf("[lucy dead]\n");
                _request_reset = true; // TODO: why doesn't this reset work?
                continue;
            }

            if (!miao->is_live) {
                miao->is_live = true;
                miao->s = lucy->s + V2(0.0, 6.0);
            }


            { // levels
                { // update
                    int _level_index = 0;
                    #define LEVEL else if (game->level_index == _level_index++)

                    { // level-specific Thing's and updates
                        if (0) {} LEVEL { // LEVEL 0
                                          // TODO: special level with all the different kinds of everything (like blow did for sokoban)
                                          // TODO: will this involve prefabs in some way?
                                          // NEXT: LEVEL tag in WAD
                        } LEVEL { // SHIVA
                            Thing *hand = Acquire_Or_Recover_Persistent_Slot__MUST_BE_SAME_ORDER_EVERY_TIME();
                            Thing *head = Find_Live_Thing_By_Unique_ID(1);
                            int &numberOfHandKills             = Counter__MUST_BE_SAME_ORDER_EVERY_TIME();
                            int &numberOfFramesSinceHandKilled = Counter__MUST_BE_SAME_ORDER_EVERY_TIME();
                            RESET {
                            } WIN_CONDITION__FORNOW_NOT_REALLY_A_CASE(!head) {
                            } UPDATE {
                                { // hand
                                    if (!hand->is_live) {
                                        ++numberOfFramesSinceHandKilled;
                                    }
                                    if (numberOfFramesSinceHandKilled > 60) {
                                        numberOfFramesSinceHandKilled = 0;
                                        Prefab_Copy(hand, 1);
                                        hand->is_live = true;
                                        if (IS_ODD(numberOfHandKills++)) { // FORNOW
                                            hand->x *= -1;
                                            hand->flip_x();
                                        }
                                    }
                                    hand->s += 0.3 * normalized(lucy->s - hand->s);
                                    /* if (hand->frames_since_hit < 8) hand->x += 0.1; */ // NEXT
                                                                                          // hand->SHIVA_UPDATE();
                                }

                                { // head
                                    head->y -= 0.1;
                                    // head->SHIVA_UPDATE();
                                }

                                { // rain
                                    if (IS_DIVISIBLE_BY(level->frame_index, 113)) {
                                        Thing *bullet = _Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_PERSISTENT_OR_FROM_WAD();
                                        bullet->is_live = true;
                                        bullet->max_age = 512;
                                        bullet->update_group = UPDATE_GROUP_BULLET;
                                        bullet->s = V2(SGN(lucy->x) * 6.0, 63.0);
                                        bullet->v = { 0.0, -0.5 };
                                        bullet->color = RED;
                                        bullet->size = { 16.0, 16.0 };
                                    }
                                }
                            }
                        } LEVEL { // spike hover with cat fire

                        } LEVEL { // dragon ride
                        }
                    }
                }

                { // draw
                    _for_each_slot(thing) {
                        if (!thing->is_live && !thing->is_persistent) continue;
                        vec3 color = V3(thing->color & RED, (thing->color & GREEN) / GREEN, (thing->color & BLUE) / BLUE);
                        vec3 inverseColor = V3(1.0) - color;
                        real alpha = ((game->mode == MODE_EDITOR) && (!thing->is_from_WAD__INCLUDES_LUCY_AND_MIAO)) ? 0.6 : 1.0;


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

                            if (thing->is_from_WAD__INCLUDES_LUCY_AND_MIAO) { // text
                                if (thing->prefab_ID || thing->update_group) {
                                    char text[16] = {};
                                    sprintf(text, "%d-%d", thing->prefab_ID, thing->update_group);
                                    if (!thing->prefab_ID) text[0] = ' ';
                                    if (!thing->update_group) text[2] = ' ';
                                    text_draw(PV, text, thing->getCenter(), inverseColor, 12, {}, true);
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
        }

        { // editor
            if (game->mode == MODE_EDITOR) {
                // TODO: should be able to hotload game logic
                if (gui_button("save", 's')) {
                    save_WAD();
                }

                level->editor_hot_thing = level->_editor_mouse_currently_pressed_thing;
                if (!level->editor_hot_thing) {
                    _for_each_slot(thing) {
                        if (!thing->is_from_WAD__INCLUDES_LUCY_AND_MIAO) continue;

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

                        { // snaps
                            if (gui_button("snap zero", 'z')) {
                                if (level->editor_selected_thing) {
                                    level->editor_selected_thing->s = {};
                                }
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
                                    /* Thing *thing = _Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_PERSISTENT_OR_FROM_WAD(); */
                                    /* *thing = game->editor_clipboard_thing; */
                                    /* thing->s = mouse_position; */
                                    /* thing->is_from_WAD__INCLUDES_LUCY_AND_MIAO = true; */
                                }
                            }
                        }
                    }


                    { //  sliders and checkboxes
                        Thing *thing = level->editor_selected_thing;
                        if (thing) {
                            /* ASSERT(thing->is_persistent); */
                            {
                                {
                                    gui_checkbox("is_persistent", &thing->is_persistent);
                                }
                                {
                                    bool is_prefab = !thing->is_NOT_prefab;
                                    gui_checkbox("is_prefab", &is_prefab);
                                    thing->is_NOT_prefab = !is_prefab;
                                }

                                char *text = "prefab_ID"; if (thing->is_live) text = "unique_ID";
                                gui_slider(text, &thing->prefab_ID, 0, 16);
                            }
                            gui_slider("update_group", &thing->update_group, 0, 16);
                            for_(i, 10) if (cow.key_pressed['0' + i]) thing->update_group = i;

                            if ((thing != lucy) && (thing != miao)) {
                                gui_slider("max_health", &thing->max_health, 0, 255);
                                gui_slider("color", &thing->color, BLACK, WHITE);
                                {
                                    int tmp = thing->origin_type;
                                    gui_slider("origin_type", &thing->origin_type, 0, 8);
                                    if (tmp != thing->origin_type) {
                                        thing->s += cwiseProduct(ORIGIN_n[thing->origin_type] - ORIGIN_n[tmp], thing->getRadius());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


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



