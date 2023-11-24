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
    while (*string == ' ') ++string; // FORNOW`
    while (*prefix == ' ') ++prefix; // FORNOW`
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


#define MIAO_STATE_NORMAL 0
#define MIAO_STATE_STACKED 1

#define LUCY_STATE_NOMAL 0


#define UPDATE_GROUP_BULLET 1


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

    bool collidesWith(MinMaxRect other) {
        bool overlapX = ((other.min.x < this->max.x) && (this->min.x < other.max.x));
        bool overlapY = ((other.min.y < this->max.y) && (this->min.y < other.max.y));
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

struct Thing {
    // constructors won't take a slot if it is is_live or is_persistent

    // entity system flags
    bool is_prefab;
    bool is_instance; // shallow copy (so far just used in game -- Nov 21)
    bool is_live;
    bool is_persistent; // even after death 
    bool is_WAD; // includes lucy and miao (so far just used in editor -- Nov 21)


    #define BULLET_OWNER_NONE  0
    #define BULLET_OWNER_LUCY  1
    #define BULLET_OWNER_MIAO  2
    #define BULLET_OWNER_ENEMY 3
    int bullet_owner;


    // // this let's you find a thing by its never-changing name
    // Instantiate_Prefab(name)
    // Find_Live_Thing_By_Name(name)
    int name;

    // // this let's things refer to other things (that may die)
    int ID;
    int parent_ID;



    // game flags
    bool is_platform;
    bool is_killer;
    // TODO: (draw these different; port bullets to general-purpose killers)
    // TODO: permeable platforms

    int state; // FORNOW: could put this in hoo

    // ---

    void advect();
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
    int color;

    int update_group;

    int origin_type;
    bool facing_right() { return ((origin_type == UPPER_RIGHT) || (origin_type == CENTER_RIGHT) || (origin_type == LOWER_RIGHT)); }
    bool facing_left() { return ((origin_type == UPPER_LEFT) || (origin_type == CENTER_LEFT) || (origin_type == LOWER_LEFT)); }
    void flip_x(bool preserve_rect = false) {
        if (origin_type == UPPER_RIGHT) origin_type = UPPER_LEFT;
        else if (origin_type == CENTER_RIGHT) origin_type = CENTER_LEFT;
        else if (origin_type == LOWER_RIGHT) origin_type = LOWER_LEFT;
        else if (origin_type == LOWER_LEFT) origin_type = LOWER_RIGHT;
        else if (origin_type == CENTER_LEFT) origin_type = CENTER_RIGHT;
        else if (origin_type == UPPER_LEFT) origin_type = UPPER_RIGHT;
        else ASSERT(0);
        if (preserve_rect) x += ORIGIN_n[origin_type].x * width;
    }

    void die() {
        ASSERT(!is_prefab);
        ASSERT(is_live);
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

#define GAME 0
#define EDITOR 1

struct FrameState {
    int _programmatic_things_recovery_index;
    int _Integers_recovery_index;
};

struct LucyLevelState {
    int jump_counter;
    int frames_since_fired;
};
struct MiaoLevelState {
    int frames_since_fired;
};
struct LevelState {
    FrameState frame;

    int frame_index;

    #define THINGS_ARRAY_LENGTH 256
    Thing things[THINGS_ARRAY_LENGTH];
    LucyLevelState lucy_;
    MiaoLevelState miao_;

    Thing *_programmatic_things_recovery_array[THINGS_ARRAY_LENGTH];
    int _num_programmatic_things;

    int _Integers[64];

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



void Thing::advect() {
    ASSERT(!is_platform);
    ASSERT (!IS_ZERO(squaredNorm(v)));
    on_platform = false;
    for_(d, 2) {
        s[d] += v[d];
        _for_each_(platform) {
            if (!platform->is_live) continue;
            if (!platform->is_platform) continue;

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
    return (slot->is_live || slot->is_persistent || slot->is_prefab || slot->is_WAD);
}
Thing *_Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_OR_PERSISTENT_OR_WAD_TO_TRUE() {
    Thing *result = things;
    while (_Slot_Is_Full(result)) ++result;
    *result = {};
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
//
void _Instantiate_Prefab_Helper(Thing *slot, int name) {
    _for_each_(prefab) {
        if (!prefab->is_prefab) continue;
        if (prefab->name == name) {
            *slot = *prefab;
            slot->is_prefab = 0; // FORNOW
            slot->name = 0;        // FORNOW
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
int &Integer__MUST_BE_SAME_ORDER_EVERY_TIME() {
    return level->_Integers[frame->_Integers_recovery_index++];
}




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
                                if (!thing->is_WAD) continue;
                                if (thing == lucy) continue;
                                if (thing == miao) continue;
                                if ((pass == 0) && (!thing->is_instance)) {
                                    fprintf(destination, " THING\n");
                                    fprintf(destination, "  color         %d\n", thing->color);
                                    fprintf(destination, "  is_prefab     %d\n", thing->is_prefab);
                                    fprintf(destination, "  is_live       %d\n", thing->is_live);
                                    fprintf(destination, "  is_persistent %d\n", thing->is_persistent);
                                    fprintf(destination, "  name            %d\n", thing->name);
                                    fprintf(destination, "  update_group  %d\n", thing->update_group);
                                    fprintf(destination, "  is_platform   %d\n", thing->is_platform);
                                    fprintf(destination, "  is_killer     %d\n", thing->is_killer);
                                    fprintf(destination, "  max_health    %d\n", thing->max_health);
                                    fprintf(destination, "  origin_type   %d\n", thing->origin_type);
                                    fprintf(destination, "  s    %.2lf %.2lf\n", thing->s[0], thing->s[1]);
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
                    thing->is_WAD = (game->mode == EDITOR); // FORNOW (not used in game)
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
                    } else if (poe_matches_prefix(line, "is_platform ")) {
                        int tmp; sscanf(line, "%s %d", prefix, &tmp); thing->is_platform = tmp;
                    } else if (poe_matches_prefix(line, "is_killer ")) {
                        int tmp; sscanf(line, "%s %d", prefix, &tmp); thing->is_killer = tmp;
                    } else if (poe_matches_prefix(line, "max_health ")) {
                        sscanf(line, "%s %d", prefix, &thing->max_health);
                    } else if (poe_matches_prefix(line, "origin_type ")) {
                        sscanf(line, "%s %d", prefix, &thing->origin_type);
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
                        lucy->color = RED;
                        lucy->origin_type = LOWER_RIGHT;
                        lucy->is_WAD = true;
                        lucy->max_health = 1;

                        miao->is_persistent = true;
                        miao->is_live = true;
                        miao->size = { 4, 4 };
                        miao->color = BLUE;
                        miao->origin_type = LOWER_RIGHT;
                        miao->is_WAD = true;
                        miao->max_health = 1;
                    }

                    load_WAD();
                } UPDATE {
                    { // lucy and miao
                        { // movement 
                            { // lucy
                                { // v.y
                                    lucy->v.y = -0.5;
                                    if (cow.key_pressed['j'] && lucy->on_platform) {
                                        if (lucy_->jump_counter == 0) {
                                            lucy_->jump_counter = 16;
                                        }
                                    }
                                    if (lucy_->jump_counter != 0) {
                                        --lucy_->jump_counter;
                                        lucy->v.y = 0.8;
                                    }
                                }
                                { // v.x
                                    lucy->v.x = 0.0;
                                    real speed = 0.4;
                                    if (cow.key_held['s']) {
                                        if (lucy->facing_right()) lucy->flip_x(true);
                                        lucy->v.x = -speed;
                                    }
                                    if (cow.key_held['f']) {
                                        if (lucy->facing_left()) lucy->flip_x(true);
                                        lucy->v.x = speed;
                                    }
                                }

                                lucy->advect();
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
                                    miao->v.y = -0.5;
                                    miao->advect();
                                } else if (miao->state == MIAO_STATE_STACKED) {
                                    miao->origin_type = lucy->origin_type;
                                    miao->s = lucy->s + V2(0.0, lucy->height);
                                } else {
                                    ASSERT(0);
                                }
                            }
                        }
                        { // fire
                            ++lucy_->frames_since_fired;
                            ++miao_->frames_since_fired;


                            if ((cow.key_held['k'] || cow.key_toggled[';']) && IS_DIVISIBLE_BY(lucy_->frames_since_fired, 12)) {
                                bool firing_vertically = (cow.key_held['e']);
                                int sgn = lucy->facing_right() ? 1 : -1; // TODO: down (once can fall onto platform)

                                // TODO: make lucy bullet a super prefab
                                lucy_->frames_since_fired = 0;
                                Thing *bullet = _Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_OR_PERSISTENT_OR_WAD_TO_TRUE();
                                bullet->is_live = true;
                                bullet->max_age = 128;
                                bullet->update_group = UPDATE_GROUP_BULLET;
                                bullet->s = lucy->getCenter() + ((!firing_vertically) ? V2(sgn * 3.0, 1.0) : V2(0.0, 5.0));
                                bullet->v = (!firing_vertically) ? V2(sgn * 1.0, 0.0) : V2(0.0, 1.0);
                                bullet->color = RED;
                                bullet->size = { 2.0, 2.0 };
                                bullet->bullet_owner = BULLET_OWNER_LUCY;
                            }
                            if ((cow.key_held['l'] || cow.key_toggled[';']) && IS_DIVISIBLE_BY(miao_->frames_since_fired, 12)) {
                                int sgn = miao->facing_right() ? 1 : -1; // TODO: down (once can fall onto platform)

                                // TODO: make miao bullet a super prefab
                                miao_->frames_since_fired = 0;
                                Thing *bullet = _Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_OR_PERSISTENT_OR_WAD_TO_TRUE();
                                bullet->is_live = true;
                                bullet->max_age = 128;
                                bullet->update_group = UPDATE_GROUP_BULLET;
                                bullet->s = miao->getCenter() + V2(sgn * 3.0, 0.0);
                                bullet->v = V2(sgn * 1.0, 0.0);
                                bullet->color = BLUE;
                                bullet->size = { 2.0, 2.0 };
                                bullet->bullet_owner = BULLET_OWNER_MIAO;
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
            }
            { // level-specific reset & update
                int _level_index = 0;
                { // level-specific Thing's and updates
                  // - before adding more features, we need more levels to motivate what we're doing
                  // - editor feature that is still missing is the ability to copy and paste live prefabs into the level
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

                        // TODO: telegraph where things will spawn?
                        // TODO: both hands coming from sides

                        // multiple phases
                        // like juggling

                        Thing *head = Find_Live_Thing_By_Name(1);

                        Thing *hand = Acquire_Or_Recover_Slot__SETS_PERSISTENT_TO_TRUE();
                        Thing *hand2 = Acquire_Or_Recover_Slot__SETS_PERSISTENT_TO_TRUE();
                        int &numberOfHandKills             = Integer__MUST_BE_SAME_ORDER_EVERY_TIME();
                        int &numberOfFramesSinceHandKilled = Integer__MUST_BE_SAME_ORDER_EVERY_TIME();
                        int &rainCounter = Integer__MUST_BE_SAME_ORDER_EVERY_TIME();

                        RESET {
                        } WIN_CONDITION__FORNOW_NOT_REALLY_A_CASE(!head) {
                        } UPDATE {
                            { // hand
                                if (!hand->is_live) {
                                    if (hand2->is_live) hand2->die();

                                    if (numberOfFramesSinceHandKilled == 0 && numberOfHandKills > 2) {
                                        Thing *bullet = Instantiate_Prefab(5);
                                        bullet->is_live = true;
                                        bullet->max_age = 1024;
                                        bullet->update_group = UPDATE_GROUP_BULLET;
                                        if (IS_EVEN(numberOfHandKills)) bullet->x *= -1;
                                        bullet->v = { 0.25 * ORIGIN_n[hand->origin_type].x, 0.0 };
                                        bullet->bullet_owner = BULLET_OWNER_ENEMY;
                                    }

                                    ++numberOfFramesSinceHandKilled;
                                    if (numberOfFramesSinceHandKilled > 60) {
                                        numberOfFramesSinceHandKilled = 0;

                                        Instantiate_Prefab_Into_Persistent_Slot__MAY_SET_PERSISTENT_TO_FALSE(hand, 2);
                                        Instantiate_Prefab_Into_Persistent_Slot__MAY_SET_PERSISTENT_TO_FALSE(hand2, 4);
                                        if (IS_ODD(numberOfHandKills++)) {
                                            hand->x *= -1;
                                            hand->flip_x();
                                            hand2->x *= -1;
                                            hand2->flip_x();
                                        }
                                    }
                                } else {
                                    vec2 ds = 0.3 * normalized(lucy->s - hand->s);
                                    if (hand->hit_counter) ds *= 2.0 * (0.5 - NUM_DEN(hand->hit_counter, HIT_COUNTER_COOLDOWN));
                                    hand->s += ds;
                                    hand2->s += ds;
                                }
                                // if (hand->frames_since_hit < 8) hand->x += 0.1;
                                // hand->SHIVA_UPDATE();
                            }

                            { // head
                              // head->SHIVA_UPDATE();
                            }

                            { // rain
                                if (IS_DIVISIBLE_BY(level->frame_index, 90)) {
                                    rainCounter = (rainCounter + 1) % 4;
                                    Thing *bullet = Instantiate_Prefab(3);
                                    bullet->max_age = 512;
                                    bullet->x = ((rainCounter < 2) ? -1 : 1) * 6.0;
                                    bullet->v = { 0.0, -0.6 };
                                    bullet->bullet_owner = BULLET_OWNER_ENEMY;
                                }
                            }
                        }
                    }
                    LEVEL { // KITCHEN SINK
                        Thing *hand = Acquire_Or_Recover_Slot__SETS_PERSISTENT_TO_TRUE();
                        Thing *head = Find_Live_Thing_By_Name(1);
                        int &numberOfHandKills             = Integer__MUST_BE_SAME_ORDER_EVERY_TIME();
                        int &numberOfFramesSinceHandKilled = Integer__MUST_BE_SAME_ORDER_EVERY_TIME();
                        RESET {
                            Thing *fly = _Acquire_Slot__MUST_BE_IMMEIDATELY_SET_LIVE_OR_PERSISTENT_OR_WAD_TO_TRUE();
                            fly->is_live = true;
                            fly->size = V2(4.0);
                            fly->color = GREEN;
                            fly->s = V2(-5.0, 15.0);
                            fly->v = V2(0.1, 0.1);
                            fly->update_group = UPDATE_GROUP_BULLET;
                        } WIN_CONDITION__FORNOW_NOT_REALLY_A_CASE(!head) {
                        } UPDATE {
                            { // hand
                                if (!hand->is_live) {
                                    ++numberOfFramesSinceHandKilled;
                                    if (numberOfFramesSinceHandKilled > 60) {
                                        numberOfFramesSinceHandKilled = 0;

                                        Instantiate_Prefab_Into_Persistent_Slot__MAY_SET_PERSISTENT_TO_FALSE(hand, 2);
                                        if (IS_ODD(numberOfHandKills++)) {
                                            hand->x *= -1;
                                            hand->flip_x();
                                        }
                                    }
                                } else {
                                    hand->s += 0.3 * normalized(lucy->s - hand->s);
                                }
                                // if (hand->frames_since_hit < 8) hand->x += 0.1;
                                // hand->SHIVA_UPDATE();
                            }

                            { // head
                                head->x = 10.0 * sin(level->frame_index / 60.0);
                                // head->SHIVA_UPDATE();
                            }

                            { // rain
                                if (IS_DIVISIBLE_BY(level->frame_index, 113)) {
                                    Thing *bullet = Instantiate_Prefab(3);
                                    bullet->max_age = 512;
                                    bullet->update_group = UPDATE_GROUP_BULLET;
                                    bullet->x = SGN(lucy->x) * 6.0;
                                    bullet->v = { 0.0, -0.5 };
                                }
                            }

                            _for_each_(thing) {
                                if (!thing->is_live) continue;
                                if (thing->update_group == 1) {
                                    thing->s += 0.1 * normalized(miao->s - thing->s);
                                }
                            }
                        }
                    }
                    LEVEL { // spike hover with cat fire

                    } LEVEL { // dragon ride
                    }
                }
            }
            { // common update & draw
                { // common
                    _for_each_(thing) {
                        if (thing->is_prefab) continue;
                        if (!thing->is_live) continue;

                        if (thing->hit_counter) --thing->hit_counter;
                    }
                }
                { // killers things that kill (happens after all movement)
                    RESET {} UPDATE {
                        // old age
                        _for_each_(thing) { // FORNOW: includes lucy and miao
                            if (thing->is_prefab) continue;
                            if (!thing->is_live) continue;

                            if ((thing->max_age) && (thing->age++ > thing->max_age)) {
                                thing->die();
                            }
                        }
                        // bullet collision
                        _for_each_(bullet) {
                            if (bullet->is_prefab) continue;
                            if (!bullet->is_live) continue;
                            if (bullet->update_group != UPDATE_GROUP_BULLET) continue;
                            bullet->s += bullet->v;

                            _for_each_(other) {
                                if (other->is_prefab) continue;
                                if (!other->is_live) continue;
                                if (other->update_group == UPDATE_GROUP_BULLET) continue;
                                //
                                if (!(bullet->color & other->color)) continue;
                                if ((bullet->bullet_owner == BULLET_OWNER_LUCY) && (other == lucy)) continue;
                                if ((bullet->bullet_owner == BULLET_OWNER_MIAO) && (other == miao)) continue;
                                if ((bullet->bullet_owner == BULLET_OWNER_ENEMY) && ((other != lucy) && (other != miao) && (!other->is_platform))) continue; // FORNOW

                                if (bullet->collidesWith(other)) {
                                    other->hit_counter = HIT_COUNTER_COOLDOWN;
                                    ++other->damage;
                                    if (other->max_health) {
                                        if (other->damage >= other->max_health) other->die();
                                    }

                                    bullet->die();
                                }
                            }
                        }
                    }
                }
                { // draw
                    _for_each_(thing) {
                        vec3 color = V3(thing->color & RED, (thing->color & GREEN) / GREEN, (thing->color & BLUE) / BLUE);

                        if (game->mode == GAME) {
                            if (thing->is_prefab) continue;
                            if (!thing->is_live) continue;

                            thing->soup_draw(PV, SOUP_QUADS, color);
                        } else if (game->mode == EDITOR) {
                            if (!thing->is_prefab && !thing->is_live && !thing->is_persistent && !thing->is_WAD) continue;

                            vec3 inverseColor = V3(1.0) - color;
                            real alpha = (!thing->is_WAD) ? 0.6 : 1.0;

                            thing->soup_draw(PV, SOUP_QUADS, color, alpha);

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
                                if (thing->is_WAD) {
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
                        if (!thing->is_WAD) continue;

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
                                    slot->is_WAD = true;
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
                                ASSERT(thing->is_WAD);
                                gui_checkbox("is_prefab", &thing->is_prefab);
                                gui_checkbox("is_live", &thing->is_live);
                                gui_checkbox("is_persistent", &thing->is_persistent);
                                gui_slider("name", &thing->name, 0, 16);
                                gui_slider("update_group", &thing->update_group, 0, 16);
                                gui_printf("---");
                                gui_checkbox("is_platform", &thing->is_platform);
                                gui_checkbox("is_killer", &thing->is_killer);
                                gui_slider("max_health", &thing->max_health, 0, 255);
                                gui_slider("color", &thing->color, BLACK, WHITE);
                                { // origin_type (with rect preservation)
                                    int tmp = thing->origin_type;
                                    gui_slider("origin_type", &thing->origin_type, 0, 8);
                                    if (tmp != thing->origin_type) {
                                        thing->s += cwiseProduct(ORIGIN_n[thing->origin_type] - ORIGIN_n[tmp], thing->getRadius());
                                    }
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



