// TODO: immortal

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

#define COW_PATCH_FRAMERATE
// #define COW_PATCH_FRAMERATE_SLEEP
typedef double real;
#define GL_REAL GL_DOUBLE
#define JIM_IS_JIM
#include "include.cpp"
#define cow globals // FORNOW
typedef unsigned long long int u64;

Camera2D camera = { 128.0 }; // FORNOW
bool poe_matches_prefix(char *string, char *prefix) { // FORNOW
    while (*string == ' ') ++string; // FORNOW`
    while (*prefix == ' ') ++prefix; // FORNOW`
    if (strlen(string) < strlen(prefix)) return false;
    for_(i, (int) strlen(prefix)) { // FORNOW
        if (string[i] != prefix[i]) return false;
    }
    return true;
}




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
// TODO: HLEFT VRIGHT (bitfield)
vec2 ORIGIN_n[]={{0,0},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0},{1,1}};


#define MIAO_STATE_NORMAL 0
#define MIAO_STATE_STACKED 1

#define LUCY_STATE_NOMAL 0


#define UPDATE_GROUP_SPEQV 1
#define UPDATE_GROUP_SHIVA 2
#define UPDATE_GROUP_CHILD 3
#define UPDATE_GROUP_ASTEP 4


struct MinMaxRect {
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

    bool collidesWith(MinMaxRect target) {
        bool overlapX = ((target.min.x < this->max.x) && (this->min.x < target.max.x));
        bool overlapY = ((target.min.y < this->max.y) && (this->min.y < target.max.y));
        return overlapX && overlapY;
    }
};

void widget_drag(mat4 PV, MinMaxRect *rect, bool no_resize) {
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

void gui_bitfield__SETS_UNEXPOSED_BITS_TO_ZERO(char *bitfield_name, char **bit_names, u64 *bitfield, int num_bits_to_expose) {
    bool tmp[64] = {};
    char buffer[128];
    for_(i, 64) {
        if (i < num_bits_to_expose) {
            tmp[i] = *bitfield & (1LL << i);
            sprintf(buffer, "%s %s", bitfield_name, bit_names[i]);
            gui_checkbox(buffer, &tmp[i]);
            *bitfield &= ~(1LL << i);
            if (tmp[i]) *bitfield |= (1 << i);
        } else {
            *bitfield &= ~(1LL << i);
        }
    }
}

#define RED   (1 <<  0)
#define GREEN (1 <<  1)
#define BLUE  (1 <<  2)
char *COLOR_BIT_STRINGS[] = { "RED", "GREEN", "BLUE" };
#define BLACK   (0)
#define YELLOW  (RED | GREEN)
#define MAGENTA (RED | BLUE)
#define CYAN    (GREEN | BLUE)
#define WHITE   (RED | GREEN | BLUE)

#define HERO     (1 <<  0)
#define ENEMY    (1 <<  1)
#define PLATFORM (1 <<  2)
char *LAYER_BIT_STRINGS[] = { "HERO", "ENEMY", "PLATFORM" };
#define ACTOR (HERO | ENEMY)
#define PHYSICAL (ACTOR | PLATFORM)


struct Thing {
    u64 color;
    u64 layer;
    u64 hits;

    // NOTE: making these a bitfield was a disaster for the usage-code`
    bool _is_WAD; // is_at_least_partially_defined_in_the_WAD_file
    bool is_persistent; // owns_slot_persistently_even_after_dying (let's the thing be killed without giving up its slot)
    bool is_live; // is_currently_alive
    bool is_prefab; // is_a_prefab
    bool is_instance; // is_an_instance_of_a_prefab
    bool is_live_non_prefab() { return is_live && !is_prefab; } // FORNOW




    // // this let's you find a thing by its never-changing name
    // Instantiate_Prefab(name)
    // Find_Live_Thing_By_Name(name)
    int name; // ? TOKILL

    // // this let's things refer to target things (that may die)
    int ID;
    int parent_ID;



    // entity system flags
    // game flags
    // identity = PLATFORM | ACTOR
    // hits = PLATFORM | ACTOR




    // TODO: how to gui expose a bitfield

    // TODO: (draw these different; port bullets to general-purpose killers)
    // TODO: permeable platforms

    int state; // FORNOW: could put this in hoo

    // ---

    void astep();
    bool on_platform;

    #define HIT_COUNTER_COOLDOWN 16
    int hit_counter;

    int age;
    int max_age;
    int damage;
    int max_health;

    union { vec2 s;    struct { real x,     y;      }; };
    vec2 v;
    union { vec2 size; struct { real width, height; }; };

    int update_group;

    int origin_type;
    bool facing_right() { return ((origin_type == UPPER_RIGHT) || (origin_type == CENTER_RIGHT) || (origin_type == LOWER_RIGHT)); }
    bool facing_left() { return ((origin_type == UPPER_LEFT) || (origin_type == CENTER_LEFT) || (origin_type == LOWER_LEFT)); }
    void flip_origin_type_x(bool preserve_rect = false) {
        if (origin_type == UPPER_RIGHT) origin_type = UPPER_LEFT;
        else if (origin_type == CENTER_RIGHT) origin_type = CENTER_LEFT;
        else if (origin_type == LOWER_RIGHT) origin_type = LOWER_LEFT;
        else if (origin_type == LOWER_LEFT) origin_type = LOWER_RIGHT;
        else if (origin_type == CENTER_LEFT) origin_type = CENTER_RIGHT;
        else if (origin_type == UPPER_LEFT) origin_type = UPPER_RIGHT;
        else { ; }
        if (preserve_rect) x += ORIGIN_n[origin_type].x * width;
    }
    void mirror_x() {
        flip_origin_type_x();
        x *= -1;
        v.x *= -1;
    }

    void die() {
        ASSERT(is_live_non_prefab());
        if (is_persistent) is_live = false;
        else *this = {};
    }

    vec2 getRadius() { return size / 2; }
    vec2 getCenter() { return s - cwiseProduct(ORIGIN_n[origin_type], getRadius()); }

    MinMaxRect getRect() {
        vec2 r = getRadius();
        vec2 c = getCenter();
        return { c - r, c + r };
    }
    void setRect(MinMaxRect rect) {
        vec2 c = (rect.min + rect.max) / 2;
        vec2 r = (rect.max - rect.min) / 2;
        s = c + cwiseProduct(ORIGIN_n[origin_type], r);
        size = 2 * r;
    }

    void soup_draw(mat4 PV, int GL_PRIMITIVE, vec3 drawColor, real drawAlpha = 1.0)  {
        MinMaxRect rect = getRect();
        if (GL_PRIMITIVE == SOUP_LINE_LOOP) {
            eso_begin(PV, SOUP_LINE_LOOP, 8.0, true); eso_color(drawColor, drawAlpha); rect.eso(); eso_end();
        } else {
            ASSERT(GL_PRIMITIVE == SOUP_QUADS);
            eso_begin(PV, SOUP_QUADS); eso_color(drawColor, drawAlpha); rect.eso(); eso_end();
        }
    }

    bool collidesWith(Thing *target) {
        return this->getRect().collidesWith(target->getRect());
    }
};

#define GAME 0
#define EDITOR 1

struct FrameState {
    int _programmatic_things_recovery_index;
    int _Integers_recovery_index;
    int _Reals_recovery_index;
};

struct LucyLevelState {
    int jump_counter;
    int jump_released_since_last_jump;

    int frames_since_fired;
};
struct MiaoLevelState {
    int frames_since_fired;
};
struct LevelState {
    FrameState frame;

    int frame_index;
    real time;

    #define THINGS_ARRAY_LENGTH 256
    Thing things[THINGS_ARRAY_LENGTH];
    LucyLevelState lucy_;
    MiaoLevelState miao_;

    Thing *_programmatic_things_recovery_array[THINGS_ARRAY_LENGTH];
    int _num_programmatic_things;

    int _Integers[64];
    real _Reals[64];

    int _next_unique_ID__FORNOW_VERY_SLOPPY;

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
#define _for_each_(name) for (Thing *name = things; name < (things + THINGS_ARRAY_LENGTH); ++name)



void Thing::astep() {
    /* ASSERT (!IS_ZERO(squaredNorm(v))); */
    ASSERT(this->layer & ACTOR);
    on_platform = false;
    for_(d, 2) {
        s[d] += v[d];
        _for_each_(platform) {
            if (!platform->is_live) continue;
            if (!(platform->layer & PLATFORM)) continue;
            if (!(this->color & platform->color)) continue;

            // ? something like this (jank)
            // ***T <--A-- P******P --B--> T***
            MinMaxRect T = this->getRect();
            MinMaxRect P = platform->getRect();
            vec2 A = T.max - P.min;
            vec2 B = T.min - P.max;
            bool overlap = true; for_(k, 2) overlap &= ((A[k] > 0) && (0 > B[k]));
            if (overlap) {
                real sgn = SGN(v[d]);
                s[d] -= (sgn > 0) ? A[d] : B[d];
                s[d] -= sgn * 0.001;
                if ((d == 1) && (sgn < 0)) on_platform = true;
            }
        }
    }
}




bool _Slot_Is_Full(Thing *slot) {
    return (slot->is_live || slot->is_persistent || slot->is_prefab || slot->_is_WAD);
}
Thing *_Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_OR_PERSISTENT_OR_WAD_TO_TRUE() {
    Thing *result = things;
    while (_Slot_Is_Full(result)) ++result;
    *result = {};
    result->ID = level->_next_unique_ID__FORNOW_VERY_SLOPPY++;
    return result;
}
Thing *Acquire_Or_Recover_Slot__SETS_PERSISTENT_TO_TRUE() {
    if (game->reseting_level) {
        ++level->_num_programmatic_things;
        Thing *result = _Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_OR_PERSISTENT_OR_WAD_TO_TRUE();
        result->is_persistent = true;
        level->_programmatic_things_recovery_array[frame->_programmatic_things_recovery_index++] = result;
        return result;
    }
    ASSERT(frame->_programmatic_things_recovery_index < level->_num_programmatic_things);
    return level->_programmatic_things_recovery_array[frame->_programmatic_things_recovery_index++];
}
Thing *Find_Live_Thing_By_Name(int name) {
    _for_each_(thing) {
        if (!thing->is_live) continue;
        if (thing->name == name) {
            return thing;
        }
    }
    return NULL;
}
// ^ TODO: unify v
Thing *Find_Live_Thing_By_ID(int ID) {
    _for_each_(thing) {
        if (!thing->is_live) continue;
        if (thing->ID == ID) {
            return thing;
        }
    }
    return NULL;
}
//
void _Instantiate_Prefab_Helper(Thing *slot, int name) {
    _for_each_(prefab) {
        if (!prefab->is_prefab) continue;
        if (prefab->name == name) {
            *slot = *prefab;
            slot->is_prefab = 0;
            slot->name = 0; // TODO
            slot->ID = ++level->_next_unique_ID__FORNOW_VERY_SLOPPY;
            return;
        }
    }
    printf("[error] name %d not found.\n", name);
    ASSERT(0);
}
Thing *Instantiate_Prefab(int name) {
    Thing *slot = _Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_OR_PERSISTENT_OR_WAD_TO_TRUE();
    _Instantiate_Prefab_Helper(slot, name);
    return slot;
}
void Instantiate_Prefab_Into_Persistent_Slot__MAY_SET_PERSISTENT_TO_FALSE(Thing *slot, int name) {
    ASSERT(slot->is_persistent);
    _Instantiate_Prefab_Helper(slot, name);
}
//
int &Integer__MUST_BE_SAME_ORDER_EVERY_TIME() { return level->_Integers[frame->_Integers_recovery_index++]; }
real &Real__MUST_BE_SAME_ORDER_EVERY_TIME() { return level->_Reals[frame->_Reals_recovery_index++]; }




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
                        // part -1: lucy & miao
                        fprintf(destination, "LEVEL\n");
                        fprintf(destination, " LUCY\n");
                        fprintf(destination, "  s %.2lf %.2lf\n", lucy->s[0], lucy->s[1]);
                        fprintf(destination, " MIAO\n");
                        fprintf(destination, "  s %.2lf %.2lf\n", miao->s[0], miao->s[1]);
                        // part 0: things and prefabs
                        // part 1: instances
                        for_(pass, 2) {
                            _for_each_(thing) {
                                if (!thing->_is_WAD) continue;
                                if (thing == lucy) continue;
                                if (thing == miao) continue;
                                if ((pass == 0) && (!thing->is_instance)) {
                                    fprintf(destination, " THING\n");
                                    fprintf(destination, "  is_prefab     %d\n", thing->is_prefab);
                                    fprintf(destination, "  is_live       %d\n", thing->is_live);
                                    fprintf(destination, "  is_persistent %d\n", thing->is_persistent);
                                    fprintf(destination, "  name          %d\n", thing->name);
                                    fprintf(destination, "  update_group  %d\n", thing->update_group);
                                    fprintf(destination, "  color       %lld\n", thing->color);
                                    fprintf(destination, "  layer          %lld\n", thing->layer);
                                    fprintf(destination, "  hits        %lld\n", thing->hits);
                                    fprintf(destination, "  max_age       %d\n", thing->max_age);
                                    fprintf(destination, "  max_health    %d\n", thing->max_health);
                                    fprintf(destination, "  origin_type   %d\n", thing->origin_type);
                                    fprintf(destination, "  s    %.2lf %.2lf\n", thing->s[0], thing->s[1]);
                                    fprintf(destination, "  v    %.2lf %.2lf\n", thing->v[0], thing->v[1]);
                                    fprintf(destination, "  size %.2lf %.2lf\n", thing->size[0], thing->size[1]);
                                } else if ((pass == 1) && (thing->is_instance)) {
                                    fprintf(destination, " INSTANCE %d %.2lf %.2lf\n", thing->name, thing->x, thing->y);
                                }
                            }
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
                } else if (poe_matches_prefix(line, "THING") || poe_matches_prefix(line, "INSTANCE")) {
                    thing = _Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_OR_PERSISTENT_OR_WAD_TO_TRUE();
                    thing->_is_WAD = (game->mode == EDITOR); // FORNOW (not used in game)
                    if (poe_matches_prefix(line, "INSTANCE")) {
                        real x;
                        real y;
                        int name;
                        sscanf(line, "%s %d %lf %lf", prefix, &name, &x, &y);
                        thing->is_persistent = true; // fornow
                        Instantiate_Prefab_Into_Persistent_Slot__MAY_SET_PERSISTENT_TO_FALSE(thing, name);
                        thing->is_instance = true;
                        thing->name = name;
                        thing->x = x;
                        thing->y = y;
                    }
                } else { // NOT used for instance
                         // NOTE: we want "s " instead of "s" or else it will also match "size 5.0 10.0"
                    if (poe_matches_prefix(line, "is_prefab ")) {
                        int tmp; sscanf(line, "%s %d", prefix, &tmp); thing->is_prefab = tmp;
                    } else if (poe_matches_prefix(line, "is_live ")) {
                        int tmp; sscanf(line, "%s %d", prefix, &tmp); thing->is_live = tmp;
                    } else if (poe_matches_prefix(line, "is_persistent ")) {
                        int tmp; sscanf(line, "%s %d", prefix, &tmp); thing->is_persistent = tmp;
                    } else if (poe_matches_prefix(line, "name ")) {
                        sscanf(line, "%s %d", prefix, &thing->name);
                    } else if (poe_matches_prefix(line, "update_group ")) {
                        sscanf(line, "%s %d", prefix, &thing->update_group);
                    } else if (poe_matches_prefix(line, "color ")) {
                        sscanf(line, "%s %lld", prefix, &thing->color);
                    } else if (poe_matches_prefix(line, "layer ")) {
                        sscanf(line, "%s %lld", prefix, &thing->layer);
                    } else if (poe_matches_prefix(line, "hits ")) {
                        sscanf(line, "%s %lld", prefix, &thing->hits);
                    } else if (poe_matches_prefix(line, "max_age ")) {
                        sscanf(line, "%s %d", prefix, &thing->max_age);
                    } else if (poe_matches_prefix(line, "max_health ")) {
                        sscanf(line, "%s %d", prefix, &thing->max_health);
                    } else if (poe_matches_prefix(line, "origin_type ")) {
                        sscanf(line, "%s %d", prefix, &thing->origin_type);
                    } else if (poe_matches_prefix(line, "s ")) {
                        sscanf(line, "%s %lf %lf", prefix, &thing->s.x, &thing->s.y);
                    } else if (poe_matches_prefix(line, "v ")) {
                        sscanf(line, "%s %lf %lf", prefix, &thing->v.x, &thing->v.y);
                    } else if (poe_matches_prefix(line, "size ")) {
                        sscanf(line, "%s %lf %lf", prefix, &thing->size.x, &thing->size.y);
                    } // TODO: else ASSERT(0);
                }
            }
        }
    } fclose(file);
}



// TODO: outline color


void cat_game() {
    bool _request_reset = true;
    game->level_index = 2;
    window_set_clear_color(0.1, 0.1, 0.1);
    while (cow_begin_frame()) {
        camera_move(&camera);
        mat4 PV = camera_get_PV(&camera);
        vec2 mouse_position = mouse_get_position(PV);
        { // integrity checks
            _for_each_(slot) {
                ASSERT(!((slot->is_prefab) && (slot->is_instance)));
            }
        }
        { // begin frame start of frame
            if (_request_reset) {
                _request_reset = false;
                game->reseting_level = true;
            } else {
                // FORNOW
                if ((game->mode == GAME) && !game->reseting_level && (!game->paused || cow.key_pressed['.'])) { // FORNOW
                    ++level->frame_index;
                    level->time += 1.0 / 60.0;
                }

                game->reseting_level = false;
                memset(frame, 0, sizeof(FrameState));
            }
        }
        { // gui
            if (cow.key_pressed[COW_KEY_TAB]) {
                // TODO: Prompt to save when tabbing out of editor
                game->reseting_level = true;
                game->mode = (game->mode == GAME) ? EDITOR : GAME;
            }
            gui_printf((game->mode == GAME) ? "GAME" : "EDITOR");
            gui_readout("LEVEL", &game->level_index);
            gui_readout("frame_index", &level->frame_index);
            {
                int num_prefabs = 0;
                int num_nonprefab_persistent_live = 0;
                int num_nonprefab_persistent_dead = 0;
                int num_nonprefab_ephemeral_live = 0;
                int num_empty_slots = THINGS_ARRAY_LENGTH;
                _for_each_(slot) {
                    if (slot->is_prefab) ++num_prefabs;
                    else if ( slot->is_persistent &&  slot->is_live) ++num_nonprefab_persistent_live;
                    else if (!slot->is_persistent &&  slot->is_live) ++num_nonprefab_ephemeral_live;
                    else if ( slot->is_persistent && !slot->is_live) ++num_nonprefab_persistent_dead;

                    if (_Slot_Is_Full(slot)) --num_empty_slots;
                }
                gui_readout("num_prefabs", &num_prefabs);
                gui_readout("num_nonprefab_ephemeral_live", &num_nonprefab_ephemeral_live);
                gui_readout("num_nonprefab_persistent_live", &num_nonprefab_persistent_live);
                gui_readout("num_nonprefab_persistent_dead", &num_nonprefab_persistent_dead);
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
        { // reset, update, & draw 
            { // macros
                #define RESET if (game->reseting_level) 
                #define WIN_CONDITION__FORNOW_NOT_REALLY_A_CASE(win_condition) else if ((game->mode == GAME) && (win_condition)) { memset(level->things + 2, 0, (THINGS_ARRAY_LENGTH  - 2) * sizeof(Thing)); } else if (0)
                #define UPDATE else if ((game->mode == GAME) && (!game->paused || globals.key_pressed['.'])) 
                #define LEVEL else if (game->level_index == _level_index++)
            }
            { // lucy & miao
                RESET {
                    memset(level, 0, sizeof(LevelState));

                    {
                        lucy->is_persistent = true;
                        lucy->is_live = true;
                        lucy->size = { 4, 8 };
                        lucy->origin_type = LOWER_RIGHT;
                        lucy->_is_WAD = true;
                        lucy->max_health = 1;
                        lucy->color = RED;
                        lucy->layer = HERO;

                        miao->is_persistent = true;
                        miao->is_live = true;
                        miao->size = { 4, 4 };
                        miao->origin_type = LOWER_RIGHT;
                        miao->_is_WAD = true;
                        miao->max_health = 1;
                        miao->color = BLUE;
                        miao->layer = HERO;
                    }

                    load_WAD();
                } UPDATE {
                    { // lucy & miao
                        { // movement 
                            { // lucy
                                { // v.y
                                    lucy->v.y = -1.5;
                                    if (cow.key_pressed['j'] && lucy->on_platform) {
                                        lucy_->jump_released_since_last_jump = false;
                                        if (lucy_->jump_counter == 0) {
                                            lucy_->jump_counter = 16;
                                        }
                                    }
                                    if (cow.key_released['j']) lucy_->jump_released_since_last_jump = true;
                                    if (lucy_->jump_counter != 0) {
                                        --lucy_->jump_counter;
                                        if (lucy_->jump_counter > 6) {
                                            lucy->v.y = 1.0;
                                        } else {
                                            if (lucy_->jump_released_since_last_jump) lucy_->jump_counter = 0;
                                            lucy->v.y = 0.0;
                                        }
                                    }
                                }
                                { // v.x
                                    lucy->v.x = 0.0;
                                    real speed = 0.8; // TODO: rescale everything so hoodie's speed layer 1?
                                    if (cow.key_held['s']) {
                                        if (lucy->facing_right()) lucy->flip_origin_type_x(true);
                                        lucy->v.x = -speed;
                                    }
                                    if (cow.key_held['f']) {
                                        if (lucy->facing_left()) lucy->flip_origin_type_x(true);
                                        lucy->v.x = speed;
                                    }
                                }

                                lucy->astep();
                            }
                            { // stacking
                                if (cow.key_pressed[' ']) {
                                    if ((miao->state == MIAO_STATE_NORMAL) && (lucy->collidesWith(miao))) {
                                        miao->state = MIAO_STATE_STACKED;
                                        // TODO: frames_since_last_state_transition
                                        // miao->state_transition()
                                    } else if (miao->state == MIAO_STATE_STACKED) {
                                        miao->state = MIAO_STATE_NORMAL;
                                    }
                                }
                            }
                            { // miao
                                if (miao->state == MIAO_STATE_NORMAL) {
                                    miao->v.y = -1.5;
                                    miao->astep();
                                } else if (miao->state == MIAO_STATE_STACKED) {
                                    miao->origin_type = lucy->origin_type;
                                    miao->s = lucy->s + V2(0.0, lucy->height);
                                } else {
                                    ASSERT(0);
                                }
                            }
                        }
                        { // fire


                            int FIRE_COOLDOWN = int(3600.0 / 160 / 1);
                            // lucy fire
                            if (!cow.key_toggled['k'] && cow.key_pressed['k']) lucy_->frames_since_fired = 0;
                            if (!cow.key_toggled['l'] && cow.key_pressed['l']) miao_->frames_since_fired = 0;
                            if ((cow.key_held['k'] || !cow.key_toggled[';']) && IS_DIVISIBLE_BY(lucy_->frames_since_fired, FIRE_COOLDOWN)) {
                                sound_play_sound("short.wav");
                                bool firing_vertically = (cow.key_held['e']);
                                int sgn = lucy->facing_right() ? 1 : -1; // TODO: down (once can fall onto platform)

                                // TODO: make lucy bullet a super prefab
                                lucy_->frames_since_fired = 0;
                                Thing *bullet = _Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_OR_PERSISTENT_OR_WAD_TO_TRUE();
                                bullet->is_live = true;
                                bullet->update_group = UPDATE_GROUP_SPEQV;
                                bullet->s = lucy->getCenter() + ((!firing_vertically) ? V2(sgn * 3.0, 1.0) : V2(0.0, 5.0));
                                bullet->v = (!firing_vertically) ? V2(sgn * 2.0, 0.0) : V2(0.0, 2.0);
                                bullet->size = { 2.0, 2.0 };
                                bullet->color = RED;
                                bullet->hits = ENEMY | PLATFORM;
                            }
                            // miao fire
                            if ((cow.key_held['l'] || !cow.key_toggled[';']) && IS_DIVISIBLE_BY(miao_->frames_since_fired, FIRE_COOLDOWN)) {
                                int sgn = miao->facing_right() ? 1 : -1; // TODO: down (once can fall onto platform)

                                // TODO: make miao bullet a super prefab
                                miao_->frames_since_fired = 0;
                                Thing *bullet = _Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_OR_PERSISTENT_OR_WAD_TO_TRUE();
                                bullet->is_live = true;
                                bullet->update_group = UPDATE_GROUP_SPEQV;
                                bullet->s = miao->getCenter() + V2(sgn * 3.0, 0.0);
                                bullet->v = V2(sgn * 2.0, 0.0);
                                bullet->size = { 2.0, 2.0 };
                                bullet->color = BLUE;
                                bullet->hits = ENEMY | PLATFORM;
                            }
                            ++lucy_->frames_since_fired;
                            ++miao_->frames_since_fired;
                        }
                    }
                }

                if (!lucy->is_live) {
                    _request_reset = true;
                    continue;
                }

                if (!miao->is_live) {
                    miao->is_live = true;
                    miao->s = lucy->s + V2(0.0, 6.0);
                }
            }
            { // level-specific reset & update
                int _level_index = 0;
                { // level-specific Thing's and updates
                  // - before adding more features, we need more levels to motivate what we're doing
                  // - editor feature that layer still missing layer the ability to copy and paste live prefabs into the level
                  //   bool is_instance
                  //   (very plausible you might want a bunch of shallow copies of the exact same thing)
                  //   ((but do we actually have a use case?)


                    if (0) {} LEVEL { // LEVEL 0
                                      // TODO: special level with all the different kinds of everything (like blow did for sokoban)
                                      // TODO: will this involve prefabs in some way?
                                      // TODO: you will want to have some hotkey combo that
                                      // "elevates" a prefab out of a level-specific prefab to 
                                      // one that can be used on any level
                    } LEVEL { // PLAYGROUND
                    } LEVEL { // SHIVA

                        // NOTE: there may be a cool editor that looks kinda like sheet music
                        // our current level would look something like this
                        // 0  1  2  3  4  5  6  7
                        // 1     1    X1    X1
                        //          2          X2

                        // LATER: telegraph where things will spawn?
                        // LATER: draw a metronome
                        // LATER: both hands coming from sides

                        // multiple phases
                        // like juggling
                        // todo actually learn siteswap
                        // todo all events must be driven off the same clock
                        // (FORNOW: don't make them reactive--the pushing things away layer fine, but may go away)
                        // -- player shouldn't have to guess how many shots are necessary to kill something?
                        // ! the hand can be killed by time, not by your shots (shots just delay it)
                        // !! same with head

                        // XXX: damage -> age
                        // XXX: timing off of level->frame_index
                        // XXX: better way of creating things with logic (update group actually seems good)
                        // XXX: handle bullet age as just getting far away
                        // XXX: any way of handling child objects


                        /* Thing *hand = Acquire_Or_Recover_Slot__SETS_PERSISTENT_TO_TRUE(); */
                        /* Thing *hand2 = Acquire_Or_Recover_Slot__SETS_PERSISTENT_TO_TRUE(); */
                        /* int &numberOfHandKills             = Integer__MUST_BE_SAME_ORDER_EVERY_TIME(); */
                        /* int &numberOfFramesSinceHandKilled = Integer__MUST_BE_SAME_ORDER_EVERY_TIME(); */
                        // FORNOW: 4-4 time
                        // https://soundcloud.com/soulon10/deep-beats-1-jungle-and-footwork-mix-2023-edition-160-bpm
                        // LATER: redo with real's

                        // LATER: lucy and miao should fire at the rate of the music

                        Thing *head = Find_Live_Thing_By_Name(1);
                        double &head_y0 = Real__MUST_BE_SAME_ORDER_EVERY_TIME();
                        // TODO: how to keep the head from flying up forever?
                        // (has to be fixgured out; but doesn't really impact gameplay)



                        int bpm = 160;
                        int frames_per_beat = (3600 / bpm);
                        int beats_per_measure = 8;
                        bool is_beat = false;
                        int starting_beat_index = 1;
                        int &_num_beats_played = Integer__MUST_BE_SAME_ORDER_EVERY_TIME();

                        // TODO: get hits_actors, is_platform, is_actor worked out
                        // (intimately tied to collision, at least eventually)
                        // these things may become a bit field


                        RESET {
                            head_y0 = head->y;
                        } WIN_CONDITION__FORNOW_NOT_REALLY_A_CASE(!head) {
                        } UPDATE {

                            head->y = MIN(head->y, head_y0);


                            if (IS_DIVISIBLE_BY(level->frame_index, frames_per_beat)) {
                                is_beat = true;
                                if (level->frame_index) ++_num_beats_played;
                            }
                            int beat_index = (starting_beat_index + _num_beats_played) % beats_per_measure;
                            /* int _beat_index_no_mod = 100;//starting_beat_index + _num_beats_played; */


                            if (is_beat) {
                                if (beat_index == 1 || beat_index == 5) {
                                    sound_play_sound("kick.wav");
                                    Thing *hand_blue = Instantiate_Prefab(3);
                                    hand_blue->max_age = 4.5 * frames_per_beat;
                                    if (beat_index == 5) hand_blue->mirror_x();

                                    Thing *hand_red = Instantiate_Prefab(4);
                                    hand_red->parent_ID = hand_blue->ID; // TODOLATER: this will happen in the editor
                                    if (beat_index == 5) hand_red->mirror_x();
                                    // TODOLATER: Instantiate_Prefab(3)->variant({});
                                }
                                if (beat_index == 0 || beat_index == 2 || beat_index == 4 || beat_index == 6) {
                                    sound_play_sound("hat.wav");
                                    Thing *raindrop = Instantiate_Prefab(1);
                                    if (beat_index == 4 || beat_index == 6) raindrop->mirror_x();
                                    Thing *fireball = Instantiate_Prefab(2);
                                    if (beat_index == 4 || beat_index == 6) fireball->mirror_x();
                                }
                            }
                            if (beat_index == 2 || beat_index == 6) {
                                /* sound_play_sound("codebase/sound.wav"); */
                                /* Thing *fireball = Instantiate_Prefab(2); */
                                /* if (beat_index == 6) fireball->mirror_x(); */
                            }
                        }
                    } LEVEL {
                    } LEVEL {
                    } LEVEL {
                    }
                }
            }

            { // common update death & draw
                RESET {} UPDATE {
                    { // update
                        _for_each_(thing) {
                            if (!thing->is_live_non_prefab()) continue;

                            ++thing->age;

                            if (thing->hit_counter) --thing->hit_counter;

                            if (thing->update_group == UPDATE_GROUP_SPEQV) {
                                thing->s += thing->v;
                            }
                            if (thing->update_group == UPDATE_GROUP_SHIVA) {
                                thing->s += LINEAR_REMAP(thing->hit_counter, HIT_COUNTER_COOLDOWN, 0, -2.0, 1.0) * thing->v;
                            }
                            if (thing->update_group == UPDATE_GROUP_CHILD) {
                                Thing *parent = Find_Live_Thing_By_ID(thing->parent_ID);
                                if (!parent) {
                                    thing->die();
                                } else {
                                    thing->s = parent->s;
                                }
                            }
                        }
                    }
                    { // death
                        { // old age
                            _for_each_(thing) { // FORNOW: includes lucy & miao
                                if (!thing->is_live_non_prefab()) continue;

                                // max_age
                                if ((thing->max_age) && (thing->age++ > thing->max_age)) {
                                    thing->die();
                                }

                                // max_distance
                                if (norm(thing->s - lucy->s) > 1024.0) {
                                    do_once printf("[max-distance]");
                                    thing->die();
                                }

                            }
                        }
                        { // hitters
                            _for_each_(hitter) {
                                if (!hitter->is_live_non_prefab()) continue;
                                if (!(hitter->hits & PHYSICAL)) continue;


                                _for_each_(target) {
                                    if (!target->is_live_non_prefab()) continue;
                                    if (target == hitter) continue;
                                    if (!(target->layer & PHYSICAL)) continue;
                                    if (!(hitter->hits & target->layer)) continue;
                                    if (!(hitter->color & target->color)) continue;

                                    if (hitter->collidesWith(target)) {
                                        sound_play_sound("hit.wav");
                                        target->hit_counter = HIT_COUNTER_COOLDOWN;
                                        ++target->damage;
                                        if (target->max_health) {
                                            if (target->damage >= target->max_health) target->die();
                                        }
                                        hitter->die();

                                    }
                                }
                            }
                        }
                    }
                }

                { // draw
                    _for_each_(thing) {
                        vec3 trueColor = V3((thing->color & RED) / RED, (thing->color & GREEN) / GREEN, (thing->color & BLUE) / BLUE);
                        vec3 inverseColor = V3(1.0) - trueColor;

                        if (game->mode == GAME) {
                            if (!thing->is_live_non_prefab()) continue;

                            real f = CLAMP(INVERSE_LERP(thing->age, 0, 12), 0, 1.0);
                            vec3 color = LERP(f, inverseColor, trueColor);
                            mat4 T = M4_Translation(thing->getCenter());
                            mat4 T_inv = M4_Translation(-thing->getCenter());
                            mat4 S = M4_Scaling(LERP(f, 1.1, 1.0));
                            mat4 PVM = PV * T * S * T_inv;
                            thing->soup_draw(PVM, SOUP_QUADS,
                                    (thing->layer & PHYSICAL) ? ((thing->layer & PLATFORM) ? 0.6 : 1.0) * color :
                                    (thing->hits & HERO) ? monokai.black : monokai.gray);
                            if (thing->hits & PHYSICAL) thing->soup_draw(PVM, SOUP_LINE_LOOP, color);
                        } else if (game->mode == EDITOR) {
                            thing->soup_draw(PV, SOUP_QUADS, monokai.white);
                            if (!_Slot_Is_Full(thing)) continue;

                            real alpha = (!thing->_is_WAD) ? 0.6 : 1.0;

                            thing->soup_draw(PV, SOUP_QUADS, trueColor, alpha);

                            if (thing->is_prefab) { // X
                                eso_begin(PV, SOUP_LINES, 6.0);
                                eso_color(inverseColor, 0.8);
                                vec2 corners[4]; thing->getRect().getCornersCCW(corners);
                                eso_vertex(corners[0]);
                                eso_vertex(corners[2]);
                                eso_vertex(corners[1]);
                                eso_vertex(corners[3]);
                                eso_end();
                            }
                            if (thing->is_instance) { // +
                                vec2 c = thing->getCenter();
                                vec2 r = thing->getRadius();
                                eso_begin(PV, SOUP_LINES, 3.0);
                                eso_color(inverseColor, 0.8);
                                eso_vertex(c + V2(0.0,  r.y));
                                eso_vertex(c + V2(0.0, -r.y));
                                eso_vertex(c + V2( r.x, 0.0));
                                eso_vertex(c + V2(-r.x, 0.0));
                                eso_end();
                            }

                            { // text
                                if (thing->_is_WAD) {
                                    if (thing->name) {
                                        char text[16] = {};
                                        sprintf(text, "name: %d", thing->name);
                                        text_draw(PV, text, thing->getCenter(), inverseColor, 12, {}, true);
                                    }
                                }
                            }

                            { // hot, selected annotations
                                if (thing == level->editor_hot_thing) {
                                    thing->soup_draw(PV, SOUP_LINE_LOOP, inverseColor, 0.5);
                                }
                                if (thing == level->editor_selected_thing) {
                                    thing->soup_draw(PV, SOUP_LINE_LOOP, inverseColor);

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
            if (game->mode == EDITOR) {
                // TODO: should be able to hotload game logic
                if (gui_button("save", 's')) {
                    save_WAD();
                }

                level->editor_hot_thing = level->_editor_mouse_currently_pressed_thing;
                if (!level->editor_hot_thing) {
                    _for_each_(thing) {
                        if (!thing->_is_WAD) continue;

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
                            Thing *thing = level->editor_hot_thing;
                            if (thing) {
                                MinMaxRect rect = thing->getRect();
                                widget_drag(PV, &rect, (thing == lucy) || (thing == miao) || (thing->is_instance));
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

                        { // cut, copy and paste (TODO: how does this work with prefabs)
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

                            // TODO: is_instance??
                            if (gui_button("paste", 'v')) {
                                Thing thing = game->editor_clipboard_thing;
                                if (thing.is_live) {
                                    Thing *slot = _Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_OR_PERSISTENT_OR_WAD_TO_TRUE();
                                    *slot = game->editor_clipboard_thing;
                                    slot->s = mouse_position;
                                    slot->_is_WAD = true;
                                    slot->is_prefab = false;
                                    if (thing.is_prefab || thing.is_instance) {
                                        slot->is_instance = true;
                                    }
                                }
                            }
                        }
                    }


                    { //  gui checkboxes and sliders
                        Thing *thing = level->editor_selected_thing;
                        if (thing) {
                            if ((thing != lucy) && (thing != miao) && (!thing->is_instance)) {
                                ASSERT(thing->_is_WAD);
                                gui_checkbox("is_prefab", &thing->is_prefab);
                                gui_checkbox("is_live", &thing->is_live);
                                gui_checkbox("is_persistent", &thing->is_persistent);
                                gui_slider("name", &thing->name, 0, 16);
                                gui_slider("update_group", &thing->update_group, 0, 5);
                                gui_printf("---");
                                gui_bitfield__SETS_UNEXPOSED_BITS_TO_ZERO("color", COLOR_BIT_STRINGS, &thing->color, _COUNT_OF(COLOR_BIT_STRINGS));
                                gui_bitfield__SETS_UNEXPOSED_BITS_TO_ZERO("layer",    LAYER_BIT_STRINGS, &thing->layer,    _COUNT_OF(LAYER_BIT_STRINGS));
                                gui_bitfield__SETS_UNEXPOSED_BITS_TO_ZERO("hits",  LAYER_BIT_STRINGS, &thing->hits,  _COUNT_OF(LAYER_BIT_STRINGS));
                                gui_slider("max_age", &thing->max_age, 0, 511);
                                gui_slider("max_health", &thing->max_health, 0, 255);
                                { // origin_type (with rect preservation)
                                    int tmp = thing->origin_type;
                                    gui_slider("origin_type", &thing->origin_type, 0, 8);
                                    if (tmp != thing->origin_type) {
                                        thing->s += cwiseProduct(ORIGIN_n[thing->origin_type] - ORIGIN_n[tmp], thing->getRadius());
                                    }
                                }
                                {
                                    gui_slider("v.x", &thing->v.x, -2.0, 2.0);
                                    gui_slider("v.y", &thing->v.y, -2.0, 2.0);
                                }
                            } else if (thing->is_instance) {
                                gui_readout("is_instance", &thing->is_instance);
                                gui_readout("name", &thing->name);
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
    config.tweaks_scale_factor_for_everything_involving_pixels_ie_gui_text_soup_NOTE_this_will_init_to_2_on_macbook_retina = 1;
    _cow_reset();
    cat_game();
    return 0;
}



