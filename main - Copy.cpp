// TODO: rename hw00.cpp (make build scripts take argument; fix up your vim to accomodate this)
// TODO: same stuff for STL

#include "cs345.cpp"


// NOTES /////////////////////////////////
// - positions and lengths are in inches
// -- i personally think in mm, so you will see, for example, INCHES(30.0)
// -- this returns (30.0 / 25.4), which is the number of inches in 30mm
// - angles are in degrees
// -- i already think in degrees, so you will see, for example,, 180.0
// -- if you think in radians, you could use DEG(PI) to achieve the same thing
// - color is just an integer
// -- useful #defines for colors are below


<<<<<<< HEAD
int bpm = 160;
int frames_per_beat() { return (3600 / bpm); }
=======
// DEFINES ///////////////////////////////

#define COLOR_TRAVERSE        0
#define COLOR_QUALITY_1       1
#define COLOR_QUALITY_2       2
#define COLOR_QUALITY_3       3
#define COLOR_QUALITY_4       4
#define COLOR_QUALITY_5       5
#define COLOR_ETCH            6
#define COLOR_LEAD_IO         9
#define COLOR_QUALITY_SLIT_1 21
#define COLOR_QUALITY_SLIT_2 22
#define COLOR_QUALITY_SLIT_3 23
#define COLOR_QUALITY_SLIT_4 24
#define COLOR_QUALITY_SLIT_5 25
>>>>>>> 48c7822a0d1a3d895cfdcdf88ecc5aae0c6e1974


// STRUCTS ///////////////////////////////

struct Line {
    double start_x;
    double start_y;
    double end_x;
    double end_y;
    int color;
};

<<<<<<< HEAD
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
            if (tmp[i]) *bitfield |= (1LL << i);
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
#define CORPSE   (1 <<  3)
char *LAYER_BIT_STRINGS[] = { "HERO", "ENEMY", "PLATFORM", "CORPSE" };
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


    // FORNOW
    int state;
    int _state_counter;


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


    // ---

    void astep();
    bool on_platform;

    #define HIT_COUNTER_COOLDOWN 16
    int hit_counter;

    int age;
    int max_age;
    int damage;
    int max_health;
    bool is_kamikazee_hitter;

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
    void mirror_x(boolean actually_do_it = true) {
        if (!actually_do_it) return;
        flip_origin_type_x();
        x *= -1;
        v.x *= -1;
    }

    void spawn_corpse();
    void die() {
        ASSERT(is_live_non_prefab());
        if (!(layer & CORPSE)) spawn_corpse();
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
=======
struct Arc {
    double center_x;
    double center_y;
    double radius;
    double start_angle;
    double end_angle;
    int color;
>>>>>>> 48c7822a0d1a3d895cfdcdf88ecc5aae0c6e1974
};

struct DXF {
    int num_lines;
    int num_arcs;
    Line *lines;
    Arc *arcs;
};


// FUNCTIONS /////////////////////////////
// you implement these ///////////////////

void dxf_save(DXF *dxf, char *filename) {
    FILE *file = fopen(filename, "w");
    ASSERT(file != NULL);

    _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(dxf);

    // header
    fprintf(file, "  0\n");
    fprintf(file, "SECTION\n");
    fprintf(file, "  2\n");
    fprintf(file, "ENTITIES\n");
    fprintf(file, "  0\n");

    // lines
    for (Line *line = dxf->lines; line < dxf->lines + dxf->num_lines; ++line) {
        fprintf(file, "LINE\n");
        fprintf(file, "  8\n");
        fprintf(file, "   0\n");
        fprintf(file, "  62\n");
        fprintf(file, "   %d\n", line->color);
        fprintf(file, "  10\n");
        fprintf(file, "   %lf\n", line->start_x);
        fprintf(file, "  20\n");
        fprintf(file, "   %lf\n", line->start_y);
        fprintf(file, "  30\n");
        fprintf(file, "   0.0\n");
        fprintf(file, "  11\n");
        fprintf(file, "   %lf\n", line->end_x);
        fprintf(file, "  21\n");
        fprintf(file, "   %lf\n", line->end_y);
        fprintf(file, "  31\n");
        fprintf(file, "   0.0\n");
        fprintf(file, "  0\n");
    }

    // arcs
    for (Arc *arc = dxf->arcs; arc < dxf->arcs + dxf->num_arcs; ++arc) {
        fprintf(file, "ARC\n");
        fprintf(file, "  8\n");
        fprintf(file, "   0\n");
        fprintf(file, "  62\n");
        fprintf(file, "   %d\n", arc->color);
        fprintf(file, "  10\n");
        fprintf(file, "   %lf\n", arc->center_x);
        fprintf(file, "  20\n");
        fprintf(file, "   %lf\n", arc->center_y);
        fprintf(file, "  30\n");
        fprintf(file, "   0.0\n");
        fprintf(file, "  40\n");
        fprintf(file, "   %lf\n", arc->radius);
        fprintf(file, "  50\n");
        fprintf(file, "   %lf\n", arc->start_angle);
        fprintf(file, "  51\n");
        fprintf(file, "   %lf\n", arc->end_angle);
        fprintf(file, "  0\n");
    }

    // footer
    fprintf(file, "ENDSEC\n");
    fprintf(file, "  0\n");
    fprintf(file, "EOF\n");
    fprintf(file, "\n");

    fclose(file);
}


// FORWARD DELCARATIONS //////////////////
// i already implemented these for you ///

// NOTE: slits drawn as solid lines NOT dotted 
void dxf_draw(mat4 transform, DXF *dxf);


// APP ///////////////////////////////////

void app_dxf() {
    Camera2D camera = { INCHES(300.0) };

    DXF dxf = {};
    dxf.num_lines = 3;
    dxf.num_arcs = 2;
    dxf.lines = (Line *) calloc(dxf.num_lines, sizeof(Line));
    dxf.lines[0] = {  INCHES( 0.0),  INCHES( 0.0),  INCHES( 30.0),  INCHES( 0.0), COLOR_QUALITY_5 };
    dxf.lines[1] = {  INCHES(30.0),  INCHES(40.0),  INCHES( 30.0),  INCHES( 0.0), COLOR_QUALITY_5 };
    dxf.lines[2] = {  INCHES(30.0),  INCHES(40.0),  INCHES(  0.0),  INCHES( 0.0), COLOR_QUALITY_5 };
    dxf.arcs = (Arc *) calloc(dxf.num_arcs, sizeof(Arc));
    dxf.arcs[0] = { INCHES(20.0), INCHES(10.0), INCHES(7.0),   0.0, 180.0, COLOR_QUALITY_2 };
    dxf.arcs[1] = { INCHES(20.0), INCHES(10.0), INCHES(7.0), 180.0, 360.0, COLOR_QUALITY_4 };

    while (cow_begin_frame()) {
        camera_move(&camera);
        mat4 transform = camera_get_PV(&camera);

        dxf_draw(transform, &dxf);

<<<<<<< HEAD
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
                    real MIN_V_Y = -1.2;
                    if (lucy->is_live) { // lucy & miao
                        { // movement 
                            { // lucy

                                { // default movement
                                    lucy->v.x = 0.0;
                                    if (cow.key_held['s']) {
                                        lucy->v.x = -0.7;
                                        if (lucy->facing_right()) lucy->flip_origin_type_x(true);
                                    }
                                    if (cow.key_held['f']) {
                                        lucy->v.x = 0.7;
                                        if (lucy->facing_left()) lucy->flip_origin_type_x(true);
                                    }
                                }


                                if (lucy->state == LUCY_STATE_NORMAL) {
                                    lucy->v.y -= 0.3;
                                    lucy->v.y = MAX(MIN_V_Y, lucy->v.y);

                                    lucy->astep();

                                    if (cow.key_pressed['j'] && lucy->on_platform) {
                                        lucy->state = LUCY_STATE_JUMP; lucy->_state_counter = 0;
                                    }

                                    if ((!lucy->on_platform) && (miao->state == MIAO_STATE_STACKED) && (cow.key_pressed['j'])) {
                                        lucy->state = LUCY_STATE_YOSHI; lucy->_state_counter = 0;
                                        miao->state = MIAO_STATE_NORMAL;
                                        miao->v.y = 1.5;
                                    }
                                } else if (lucy->state == LUCY_STATE_JUMP) {
                                    lucy->v.y = 0.5 * MIAO_CLAMPED_LERP(lucy->_state_counter - 3, 10, 3, 0);

                                    lucy->astep();

                                    if ((!cow.key_held['j'] && lucy->_state_counter > 6) || (lucy->_state_counter == 20)) {
                                        lucy->state = LUCY_STATE_NORMAL; lucy->_state_counter = 0;
                                    }
                                } else if (lucy->state == LUCY_STATE_YOSHI) {
                                    // lucy->v.x *= 2.0;
                                    lucy->v.y = MIAO_CLAMPED_LERP(lucy->_state_counter, 10, 7, 1) * .44;

                                    lucy->astep();

                                    if
                                        ((!cow.key_held['j'] && lucy->_state_counter > 10) || (lucy->_state_counter == 20)) {
                                        // ((lucy->_state_counter == 20)) {
                                            lucy->state = LUCY_STATE_NORMAL; lucy->_state_counter = 0;
                                        }
                                }

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
                                        miao->v.y -= 0.3;
                                        miao->v.y = MAX(MIN_V_Y, miao->v.y);
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
                                    bullet->is_kamikazee_hitter = true;
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
                                    bullet->is_kamikazee_hitter = true;
                                }
                                ++lucy_->frames_since_fired;
                                ++miao_->frames_since_fired;
                            }
                            { // stack?
                                if (!miao->is_live) {
                                    miao->is_live = true;
                                    miao->s = lucy->s + V2(0.0, 6.0);
                                    if (miao->state != MIAO_STATE_STACKED) {
                                        miao->state = MIAO_STATE_STACKED;
                                    } else {
                                        miao->state = MIAO_STATE_NORMAL;
                                    }
                                }
                            }
                        }
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

                        int beats_per_measure = 8;
                        bool is_beat = false;
                        int starting_beat_index = 0; // FORNOW
                        int &_num_beats_played = Integer__MUST_BE_SAME_ORDER_EVERY_TIME();
                        int beat_index = -1;
                        RESET {
                        } UPDATE{
                            if (IS_DIVISIBLE_BY(level->frame_index, frames_per_beat())) {
                                is_beat = true;
                                if (level->frame_index) ++_num_beats_played;
                            }
                            beat_index = (starting_beat_index + _num_beats_played) % beats_per_measure;
                        }

                        if (0) {} LEVEL { // LEVEL 0
                            // TODO: special level with all the different kinds of everything (like blow did for sokoban)
                            // TODO: will this involve prefabs in some way?
                            // TODO: you will want to have some hotkey combo that
                            // "elevates" a prefab out of a level-specific prefab to 
                            // one that can be used on any level
                        } LEVEL { // PLAYGROUND
                        } LEVEL { // SHIVA
                            Thing *head = Find_Live_Thing_By_Name(1);
                            double &head_y0 = Real__MUST_BE_SAME_ORDER_EVERY_TIME();
                            RESET {
                                head_y0 = head->y;
                            } UPDATE {
                                head->y = MIN(head->y, head_y0);
                                if (is_beat) {
                                    if (beat_index == 1 || beat_index == 5) {
                                        sound_play_sound("kick.wav");
                                        Thing *hand_blue = Instantiate_Prefab(3);
                                        hand_blue->max_age = 4 * frames_per_beat() - 1;
                                        hand_blue->mirror_x(beat_index == 5);

                                        Thing *hand_red = Instantiate_Prefab(4);
                                        hand_red->parent_ID = hand_blue->ID; // TODOLATER: this will happen in the editor
                                        hand_red->mirror_x(beat_index == 5);
                                        // TODOLATER: Instantiate_Prefab(3)->variant({});
                                    }
                                    if (beat_index == 0 || beat_index == 2 || beat_index == 4 || beat_index == 6) {
                                        sound_play_sound("hat.wav");
                                        Instantiate_Prefab(1)->mirror_x(beat_index == 4 || beat_index == 6);
                                        Instantiate_Prefab(2)->mirror_x(beat_index == 4 || beat_index == 6);
                                    }
                                }
                            }
                        } LEVEL { // YOSHI
                            int &count = Integer__MUST_BE_SAME_ORDER_EVERY_TIME();
                            RESET {
                                bpm = 188;
                                miao->state = MIAO_STATE_STACKED;
                            } UPDATE {
                                if (is_beat) {
                                    if (IS_EVEN(beat_index)) {
                                        Instantiate_Prefab(1)->mirror_x(beat_index >=4);
                                        ++count;
                                    }
                                    if (beat_index == 0 || beat_index == 4) {
                                        // Thing *thing = Instantiate_Prefab(3);
                                        // Instantiate_Prefab(2)->mirror_x(IS_ODD(count));
                                    }
                                }
                            }
                        } LEVEL {
                        } LEVEL {
                        }
                    }
                }

                { // common update death & draw
                    // TODO: children should die same frame as parents (clean up pass)
                    RESET {} UPDATE {
                        { // update
                            _for_each_(thing) {
                                if (!thing->is_live_non_prefab()) continue;

                                ++thing->age;
                                ++thing->_state_counter;
                                if (thing->hit_counter) --thing->hit_counter;

                                if (thing->update_group == UPDATE_GROUP_SPEQV) {
                                    thing->s += thing->v;
                                }
                                if (thing->update_group == UPDATE_GROUP_SHIVA) {
                                    thing->s += LINEAR_REMAP(thing->hit_counter, HIT_COUNTER_COOLDOWN, 0, -2.0, 1.0) * thing->v;
                                }
                                if (thing->update_group == UPDATE_GROUP_CHILD) {
                                    Thing *parent = Find_Live_Thing_By_ID(thing->parent_ID);
                                    ASSERT(parent);
                                    thing->s = parent->s;
                                }
                            }
                        }
                        { // death
                            for_(pass, 2) _for_each_(thing) { // FORNOW: includes lucy & miao
                                if (!thing->is_live_non_prefab()) continue;

                                if ((pass == 0) && (thing->update_group == UPDATE_GROUP_CHILD)) continue;
                                if ((pass == 1) && (thing->update_group != UPDATE_GROUP_CHILD)) continue;

                                if (thing->update_group == UPDATE_GROUP_CHILD) {
                                    Thing *parent = Find_Live_Thing_By_ID(thing->parent_ID);
                                    if (!parent) {
                                        thing->die();
                                        continue;
                                    }
                                }

                                if ((thing->max_age) && (thing->age > thing->max_age)) {
                                    thing->die();
                                    continue;
                                }

                                { // max_distance
                                    if (norm(thing->s - lucy->s) > 1024.0) {
                                        do_once printf("[max-distance]");
                                        thing->die();
                                        continue;
                                    }
                                }


                                if (!(thing->layer & PHYSICAL)) continue;
                                _for_each_(hitter) {
                                    if (hitter == thing) continue;
                                    if (!(hitter->hits & thing->layer)) continue;
                                    if (!(hitter->color & thing->color)) continue;
                                    if (!hitter->is_live_non_prefab()) continue;
                                    if (!(hitter->hits & PHYSICAL)) continue;



                                    if (hitter->collidesWith(thing)) {
                                        /* sound_play_sound("hit.wav"); */
                                        thing->hit_counter = HIT_COUNTER_COOLDOWN;
                                        ++thing->damage;
                                        if (thing->max_health) {
                                            if (thing->damage >= thing->max_health) {
                                                thing->die();
                                            }
                                        }
                                        if (hitter->is_kamikazee_hitter) hitter->die(); // TODO
                                        break;
                                    }
                                }
                            }

                            if ((!lucy->is_live) && miao->is_live) {
                                miao->die();
                            }
                        }
                    }

                    { // draw
                        for (Thing *thing = things + THINGS_ARRAY_LENGTH - 1; thing >= things; --thing) {
                            vec3 trueColor = V3(double(thing->color & RED) / RED, double(thing->color & GREEN) / GREEN, double(thing->color & BLUE) / BLUE);
                            vec3 highlightColor = monokai.yellow;

                            real f = CLAMP(INVERSE_LERP(thing->age, 0, 12), 0.0, 1.0);


                            if (game->mode == GAME) {
                                if (!thing->is_live_non_prefab()) continue;

                                vec3 color = LERP(f, highlightColor, trueColor);
                                mat4 T = M4_Translation(thing->getCenter());
                                mat4 T_inv = M4_Translation(-thing->getCenter());
                                mat4 S = M4_Scaling(LERP(f, 1.1, 1.0), LERP(f, 1.2, 1.0));
                                real alpha = (!(thing->layer & CORPSE)) ? 1.0 : 1.0 - f;

                                vec3 black = LERP(f, color, monokai.black);

                                mat4 PVM = PV * T * S * T_inv;
                                thing->soup_draw(PVM, SOUP_QUADS,
                                        (thing->layer & PHYSICAL) ? ((thing->layer & PLATFORM) ? 0.6 : 1.0) * color :
                                        black, alpha);
                                if (thing->hits & PHYSICAL) thing->soup_draw(PVM, SOUP_LINE_LOOP, color, alpha);
                            } else if (game->mode == EDITOR) {
                                thing->soup_draw(PV, SOUP_QUADS, monokai.white);
                                if (!_Slot_Is_Full(thing)) continue;

                                real alpha = (!thing->_is_WAD) ? 0.6 : 1.0;

                                thing->soup_draw(PV, SOUP_QUADS, trueColor, alpha);

                                if (thing->is_prefab) { // X
                                    eso_begin(PV, SOUP_LINES, 6.0);
                                    eso_color(highlightColor, 0.8);
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
                                    eso_color(highlightColor, 0.8);
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
                                            text_draw(PV, text, thing->getCenter(), highlightColor, 12, {}, true);
                                        }
                                    }
                                }

                                { // hot, selected annotations
                                    if (thing == level->editor_hot_thing) {
                                        thing->soup_draw(PV, SOUP_LINE_LOOP, highlightColor, 0.5);
                                    }
                                    if (thing == level->editor_selected_thing) {
                                        thing->soup_draw(PV, SOUP_LINE_LOOP, highlightColor);

                                        // dot
                                        eso_begin(PV, SOUP_POINTS, 16, true); eso_color(highlightColor); eso_vertex(thing->s); eso_end();
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
                                    gui_checkbox("is_kamikazee_hitter", &thing->is_kamikazee_hitter);
                                    gui_bitfield__SETS_UNEXPOSED_BITS_TO_ZERO("color", COLOR_BIT_STRINGS, &thing->color, _COUNT_OF(COLOR_BIT_STRINGS));
                                    gui_bitfield__SETS_UNEXPOSED_BITS_TO_ZERO("layer",    LAYER_BIT_STRINGS, &thing->layer,    _COUNT_OF(LAYER_BIT_STRINGS));
                                    gui_bitfield__SETS_UNEXPOSED_BITS_TO_ZERO("hits",  LAYER_BIT_STRINGS, &thing->hits,  _COUNT_OF(LAYER_BIT_STRINGS));
                                    {
                                        int max_age_in_beats = thing->max_age / frames_per_beat();
                                        gui_slider("max_age_in_beats", &max_age_in_beats, 0, 8);
                                        thing->max_age = max_age_in_beats * frames_per_beat();
                                    }
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
=======
        if (gui_button("save", 's')) {
            dxf_save(&dxf, "out.dxf");
>>>>>>> 48c7822a0d1a3d895cfdcdf88ecc5aae0c6e1974
        }
    }

<<<<<<< HEAD


#include <random>

    int main() {
        config.hotkeys_app_next = 0;
        config.hotkeys_app_prev = 0;
        config.tweaks_soup_draw_with_rounded_corners_for_all_line_primitives = false;
        _cow_init();
        config.tweaks_scale_factor_for_everything_involving_pixels_ie_gui_text_soup_NOTE_this_will_init_to_2_on_macbook_retina /= 2.0;
        _cow_reset();
        cat_game();
        return 0;
    }
=======
int main() {
    APPS {
        APP(app_dxf);
    }

    return 0;
}
>>>>>>> 48c7822a0d1a3d895cfdcdf88ecc5aae0c6e1974


// FORWARD DELCARED FUNCTIONS ////////////

void _dxf_eso_color(int color) {
    if      (color == 0) { eso_color( 83 / 255.0, 255 / 255.0,  85 / 255.0); }
    else if (color % 10 == 1) { eso_color(255 / 255.0,   0 / 255.0,   0 / 255.0); }
    else if (color % 10 == 2) { eso_color(238 / 255.0,   0 / 255.0, 119 / 255.0); }
    else if (color % 10 == 3) { eso_color(255 / 255.0,   0 / 255.0, 255 / 255.0); }
    else if (color % 10 == 4) { eso_color(170 / 255.0,   1 / 255.0, 255 / 255.0); }
    else if (color % 10 == 5) { eso_color(  0 / 255.0,  85 / 255.0, 255 / 255.0); }
    else if (color == 6) { eso_color(136 / 255.0, 136 / 255.0, 136 / 255.0); }
    else if (color == 7) { eso_color(205 / 255.0, 205 / 255.0, 205 / 255.0); }
    else if (color == 8) { eso_color(  0 / 255.0, 255 / 255.0, 255 / 255.0); }
    else if (color == 9) { eso_color(204 / 255.0, 136 / 255.0,   1 / 255.0); }
    else { eso_color(1.0, 1.0, 1.0); }
}

void dxf_draw(mat4 transform, DXF *dxf) {
    // lines
    eso_begin(transform, SOUP_LINES);
    for (Line *line = dxf->lines; line < dxf->lines + dxf->num_lines; ++line) {
        _dxf_eso_color(line->color);
        eso_vertex(line->start_x, line->start_y);
        eso_vertex(line->end_x,   line->end_y);
    }

    // arcs
    int NUM_SEGMENTS_PER_CIRCLE = 64;
    for (Arc *arc = dxf->arcs; arc < dxf->arcs + dxf->num_arcs; ++arc) {
        _dxf_eso_color(arc->color);
        double start_angle = RAD(arc->start_angle);
        double end_angle = RAD(arc->end_angle);
        double delta_angle = end_angle - start_angle;
        int num_segments = (int) (1 + (delta_angle / TAU) * NUM_SEGMENTS_PER_CIRCLE);
        double increment = delta_angle / num_segments;
        double current_angle = start_angle;
        for (int i = 0; i <= num_segments; ++i) {
            eso_vertex(
                    arc->center_x + arc->radius * cos(current_angle),
                    arc->center_y + arc->radius * sin(current_angle));
            current_angle += increment;
            eso_vertex(
                    arc->center_x + arc->radius * cos(current_angle),
                    arc->center_y + arc->radius * sin(current_angle));
        }
    }
    eso_end();
}
