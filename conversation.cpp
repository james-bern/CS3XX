// sort out movement (do you show both yellow and blue or just blue; what moves; etc)
// 3D sketch plane is a bbox with offsets

// TODO: revolve
// // HW notes
// - plated through holes


// TODO: n



// TODO: incorporate console_params_preview_flip_flag fully into console_param_preview's (preview should have it all in there)

// visualization of 2D origin moving on the left hand side

// 'n'

// TODO: proper undo (including dxf selection, etc.) (using input deltas?--big array of input events)

// TODO: write blow style ability for "game to play[/debug] itself"

// NOTE: what makes this software interesting is PRECSISION--if you don't care about precision just use sketchup
// we want some of sketchups intuitiveness, with the precision of solidworks

// TODO: how do full-fledged undo system's like LAYOUT's work?

// BUG: window select grabbing things outside

// BUG: suspect something in stl (or i guess dxf?) memory management; 
// REPRO: extrude, load nonexistent stl, undo, undo, extrude
// ROOT: pushing to history regardless of whetehr new file exists=>new mesh allocated (double free)
// LESSON: only push to history stack if you are SURE you are going to create a new fancy mesh and manifold
//         ? could we bind these ops together?

// BUG: colors messed up on multiple chained undos
// ROOT:fancy_mesh_cosmetic_edges_calculate and fancy_mesh_triangle_normals_calculate were freeing
//      memory that was passed to them if it wasn't NULL;--we actually did NOT want to free because we were pushing
//      a copy of the mesh object onto the undo stack (which ended up in a horrible partially freed state)
// LESSON: only simple one-per-struct functions called *_free(...) should free

// TODO: MOVE_2D_ORIGIN_TO x, y (later will need relative variant MOVE_2D_ORIGIN BY (...and MOVE_2D_ORIGIN TO_END_OF, etc., ...)

// TODO: 3D (right hand side) layers (extends to colors)--stuff only booleans with stuff on its own layer

// / window selection

// TODO: fix up rotations to work with origin_x, origin_y
// TODO: dxf picks /'c' #'e' #'z' PICK_MODIFIER (to use with the zero)

// IDEA: Translating and Rotating and scaling the (3D) work piece (like in a mill)




// TODO: i don't really understand how memory works with the C bindings (malloc, etc.)
// TODO: biiiig careful clean up pass



// // Conversation
// This is a little CAD program Jim is making :)
//  It takes in an OMAX DXF and let's you rapidly create a 3D-printable STL using Manifold.

// / basic undo (chain of stls and manifold_manifold)
// / messages from app (messagef) for missing path, etc.

// TODO: color codes instead of ` in gui_printf


// ? TODO: the Hard problem of avoiding the creation of ultra-thin features

#define ENTER_MODE_NONE              0
#define ENTER_MODE_EXTRUDE_ADD       1
#define ENTER_MODE_EXTRUDE_CUT       2
#define ENTER_MODE_REVOLVE_ADD       3
#define ENTER_MODE_REVOLVE_CUT       4
#define ENTER_MODE_LOAD              5
#define ENTER_MODE_SAVE              6
#define ENTER_MODE_MOVE_ORIGIN_TO    7
#define ENTER_MODE_OFFSET_PLANE_BY   8
#define _ENTER_MODE_DEFAULT ENTER_MODE_NONE
#define CLICK_MODE_NONE           0
#define CLICK_MODE_SELECT         1
#define CLICK_MODE_DESELECT       2
#define CLICK_MODE_MOVE_2D_ORIGIN_TO 3
#define _CLICK_MODE_DEFAULT CLICK_MODE_NONE
#define CLICK_MODIFIER_NONE      0
#define CLICK_MODIFIER_CONNECTED 1
#define CLICK_MODIFIER_QUALITY   2
#define CLICK_MODIFIER_WINDOW    3
#define CLICK_MODIFIER_CENTER_OF 4
#define CLICK_MODIFIER_END_OF    5
#define CLICK_MODIFIER_MIDDLE_OF 6
#define _CLICK_MODIFIER_DEFAULT CLICK_MODIFIER_NONE

#include "cs345.cpp"
#include "manifoldc.h"
#include "poe.cpp"
#undef real // ???
bool *key_pressed = globals.key_pressed;
bool *key_toggled = globals.key_toggled;

vec3 get(real32 *x, u32 i) {
    vec3 result;
    for (u32 d = 0; d < 3; ++d) result.data[d] = x[3 * i + d];
    return result;
}
void set(real32 *x, u32 i, vec3 v) {
    for (u32 d = 0; d < 3; ++d) x[3 * i + d] = v.data[d];
}
void eso_vertex(real32 *p_j) {
    eso_vertex(p_j[0], p_j[1], p_j[2]);
}
void eso_vertex(real32 *p, u32 j) {
    eso_vertex(p[3 * j + 0], p[3 * j + 1], p[3 * j + 2]);
}






struct BoundingBox {
    real32 min[2];
    real32 max[2];
};
void pprint(BoundingBox bounding_box) {
    printf("(%f, %f) <-> (%f, %f)\n", bounding_box.min[0], bounding_box.min[1], bounding_box.max[0], bounding_box.max[1]);
}
BoundingBox bounding_box_union(u32 num_bounding_boxes, BoundingBox *bounding_boxes) {
    BoundingBox result = { HUGE_VAL, HUGE_VAL, -HUGE_VAL, -HUGE_VAL };
    for (u32 i = 0; i < num_bounding_boxes; ++i) {
        for (u32 d = 0; d < 2; ++d) {
            result.min[d] = MIN(result.min[d], bounding_boxes[i].min[d]);
            result.max[d] = MAX(result.max[d], bounding_boxes[i].max[d]);
        }
    }
    return result;
}
bool32 bounding_box_contains(BoundingBox outer, BoundingBox inner) {
    for (u32 d = 0; d < 2; ++d) {
        if (outer.min[d] > inner.min[d]) return false;
        if (outer.max[d] < inner.max[d]) return false;
    }
    return true;
}


real32 Z_FIGHT_EPS = 0.05f;
real32 TOLERANCE_DEFAULT = 1e-5f;
u32 NUM_SEGMENTS_PER_CIRCLE = 64;




char conversation_message_buffer[256];
u32 conversation_message_cooldown;
void conversation_messagef(char *format, ...) {
    va_list arg;
    va_start(arg, format);
    vsnprintf(conversation_message_buffer, sizeof(conversation_message_buffer), format, arg);
    va_end(arg);
    conversation_message_cooldown = 300;
}




int _grid_box_num_triangles = 12;
int _grid_box_num_vertices = 24;
int3 _grid_box_triangle_indices[] = {
    { 1, 0, 2},{ 2, 0, 3},
    { 4, 5, 6},{ 4, 6, 7},
    { 8, 9,10},{ 8,10,11},
    {13,12,14},{14,12,15},
    {17,16,18},{18,16,19},
    {20,21,22},{20,22,23},
};
vec3 _grid_box_vertex_positions[] = {
    { 1, 1, 1},{ 1, 1,-1},{ 1,-1,-1},{ 1,-1, 1},
    {-1, 1, 1},{-1, 1,-1},{-1,-1,-1},{-1,-1, 1},
    { 1, 1, 1},{ 1, 1,-1},{-1, 1,-1},{-1, 1, 1},
    { 1,-1, 1},{ 1,-1,-1},{-1,-1,-1},{-1,-1, 1},
    { 1, 1, 1},{ 1,-1, 1},{-1,-1, 1},{-1, 1, 1},
    { 1, 1,-1},{ 1,-1,-1},{-1,-1,-1},{-1, 1,-1},
};
vec3 _grid_box_vertex_colors[] = {
    { 0.8f, 0.8f, 0.8f},{ 0.8f, 0.8f,0.4f},{ 0.8f,0.4f,0.4f},{ 0.8f,0.4f, 0.8f},
    {0.4f, 0.8f, 0.8f},{0.4f, 0.8f,0.4f},{0.4f,0.4f,0.4f},{0.4f,0.4f, 0.8f},
    { 0.8f, 0.8f, 0.8f},{ 0.8f, 0.8f,0.4f},{0.4f, 0.8f,0.4f},{0.4f, 0.8f, 0.8f},
    { 0.8f,0.4f, 0.8f},{ 0.8f,0.4f,0.4f},{0.4f,0.4f,0.4f},{0.4f,0.4f, 0.8f},
    { 0.8f, 0.8f, 0.8f},{ 0.8f,0.4f, 0.8f},{0.4f,0.4f, 0.8f},{0.4f, 0.8f, 0.8f},
    { 0.8f, 0.8f,0.4f},{ 0.8f,0.4f,0.4f},{0.4f,0.4f,0.4f},{0.4f, 0.8f,0.4f},
};
vec2 _grid_box_vertex_texCoords[] = {
    {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
    {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
    {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
    {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
    {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
    {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
};
IndexedTriangleMesh3D grid_box = {
    _grid_box_num_vertices,
    _grid_box_num_triangles,
    _grid_box_vertex_positions,
    NULL,
    _grid_box_vertex_colors,
    _grid_box_triangle_indices,
    _grid_box_vertex_texCoords
};
real32 GRID_SIDE_LENGTH = 256.0f;
real32 GRID_SPACING = 10.0f;
BEGIN_PRE_MAIN {
    u32 texture_side_length = 1024;
    u32 number_of_channels = 4;
    u8 *data = (u8 *) malloc(texture_side_length * texture_side_length * number_of_channels * sizeof(u8));
    u32 o = 9;
    for (u32 j = 0; j < texture_side_length; ++j) {
        for (u32 i = 0; i < texture_side_length; ++i) {
            u32 k = number_of_channels * (j * texture_side_length + i);
            u32 n = u32(texture_side_length / GRID_SIDE_LENGTH * 10);
            u32 t = 2;
            bool32 stripe = (((i + o) % n < t) || ((j + o) % n < t));
            u8 value = 0;
            if (stripe) value = 80;
            if (i < t || j < t || i > texture_side_length - t - 1 || j > texture_side_length - t - 1) value = 160;
            for (u32 d = 0; d < 3; ++d) data[k + d] = value;
            data[k + 3] = 160;
        }
    }
    _mesh_texture_create("procedural grid", texture_side_length, texture_side_length, number_of_channels, data);
} END_PRE_MAIN;




////////////////////////////////////////
// 2D //////////////////////////////////
////////////////////////////////////////



#define DXF_COLOR_TRAVERSE        0
#define DXF_COLOR_QUALITY_1       1
#define DXF_COLOR_QUALITY_2       2
#define DXF_COLOR_QUALITY_3       3
#define DXF_COLOR_QUALITY_4       4
#define DXF_COLOR_QUALITY_5       5
#define DXF_COLOR_ETCH            6
#define DXF_COLOR_WATER_ONLY      8
#define DXF_COLOR_LEAD_IO         9
#define DXF_COLOR_QUALITY_SLIT_1 21
#define DXF_COLOR_QUALITY_SLIT_2 22
#define DXF_COLOR_QUALITY_SLIT_3 23
#define DXF_COLOR_QUALITY_SLIT_4 24
#define DXF_COLOR_QUALITY_SLIT_5 25
#define DXF_COLOR_SELECTION     254
#define DXF_COLOR_DONT_OVERRIDE 255


struct DXFLine {
    real32 start_x;
    real32 start_y;
    real32 end_x;
    real32 end_y;
    real32 _;
};

struct DXFArc {
    real32 center_x;
    real32 center_y;
    real32 radius;
    real32 start_angle_in_degrees;
    real32 end_angle_in_degrees;
};

#define DXF_ENTITY_TYPE_LINE 0
#define DXF_ENTITY_TYPE_ARC  1
struct DXFEntity {
    u32 type;
    u32 color;
    union {
        DXFLine line;
        DXFArc arc;
    };
};

void get_point_on_circle_NOTE_pass_angle_in_radians(real32 *x, real32 *y, real32 center_x, real32 center_y, real32 radius, real32 angle_in_radians) {
    *x = center_x + radius * COS(angle_in_radians);
    *y = center_y + radius * SIN(angle_in_radians);
}

void arc_process_angles_into_lerpable_radians_considering_flip_flag(DXFArc *arc, real32 *start_angle, real32 *end_angle, bool32 flip_flag) {
    // The way the DXF spec works is that start_angle and end_angle define points on the circle
    // which are connected counterclockwise from start to end with an arc
    // (start -ccw-> end)
    //
    // To flip an arc entity, we need to go B -cw-> A
    *start_angle = RAD(arc->start_angle_in_degrees);
    *end_angle = RAD(arc->end_angle_in_degrees);
    if (!flip_flag) {
        // start -ccw-> end
        while (*end_angle < *start_angle) *end_angle += TAU;
    } else {
        // swap
        real32 tmp = *start_angle;
        *start_angle = *end_angle;
        *end_angle = tmp;
        // start -cw-> end
        while (*end_angle > *start_angle) *start_angle += TAU;
    }
}

void entity_get_start_and_end_points(DXFEntity *entity, real32 *start_x, real32 *start_y, real32 *end_x, real32 *end_y) {
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLine *line = &entity->line;
        *start_x = line->start_x;
        *start_y = line->start_y;
        *end_x = line->end_x;
        *end_y = line->end_y;
    } else {
        ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
        DXFArc *arc = &entity->arc;
        get_point_on_circle_NOTE_pass_angle_in_radians(start_x, start_y, arc->center_x, arc->center_y, arc->radius, RAD(arc->start_angle_in_degrees));
        get_point_on_circle_NOTE_pass_angle_in_radians(  end_x,   end_y, arc->center_x, arc->center_y, arc->radius,   RAD(arc->end_angle_in_degrees));
    }
}

struct DXF {
    u32 num_entities;
    DXFEntity *entities;
};

void dxf_free(DXF *dxf) {
    if (dxf->entities) free(dxf->entities);
    *dxf = {};
}

void dxf_load(char *filename, DXF *dxf) {
    #if 0
    {
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(filename);
        return {};
        #elif 0
        DXF result = {};
        result.num_entities = 8;
        result.entities = (DXFEntity *) calloc(result.num_entities, sizeof(DXFEntity));
        result.entities[0] = { DXF_ENTITY_TYPE_LINE, 0, 0.0, 0.0, 1.0, 0.0 };
        result.entities[1] = { DXF_ENTITY_TYPE_LINE, 1, 1.0, 0.0, 1.0, 1.0 };
        result.entities[2] = { DXF_ENTITY_TYPE_LINE, 2, 0.0, 1.0, 0.0, 0.0 };
        result.entities[3] = { DXF_ENTITY_TYPE_ARC,  3, 0.5, 1.0, 0.5,    0.0, 180.0 };
        result.entities[4] = { DXF_ENTITY_TYPE_ARC,  4, 0.5, 1.0, 0.25,   0.0, 180.0 };
        result.entities[5] = { DXF_ENTITY_TYPE_ARC,  5, 0.5, 1.0, 0.25, 180.0, 360.0 };
        result.entities[6] = { DXF_ENTITY_TYPE_ARC,  6, 0.5, 1.0, 0.1,    0.0, 180.0 };
        result.entities[7] = { DXF_ENTITY_TYPE_ARC,  7, 0.5, 1.0, 0.1,  180.0, 360.0 };
        return result;
    }
    #endif
    dxf_free(dxf);

    FILE *file = (FILE *) fopen(filename, "r");
    ASSERT(file);

    List<DXFEntity> stretchy_list = {}; {
        {
            #define DXF_LOAD_MODE_NONE 0
            #define DXF_LOAD_MODE_LINE 1
            #define DXF_LOAD_MODE_ARC  2
            u8 mode = 0;
            int code = 0;
            bool32 code_is_hot = false;
            DXFEntity entity = {};
            static char buffer[512];
            while (fgets(buffer, ARRAY_LENGTH(buffer), file)) {
                if (mode == DXF_LOAD_MODE_NONE) {
                    if (poe_prefix_match(buffer, "LINE")) {
                        mode = DXF_LOAD_MODE_LINE;
                        code_is_hot = false;
                        entity = {};
                    } else if (poe_prefix_match(buffer, "ARC")) {
                        mode = DXF_LOAD_MODE_ARC;
                        code_is_hot = false;
                        entity = {};
                    }
                } else {
                    if (!code_is_hot) {
                        sscanf(buffer, "%d", &code);
                        if (code == 0) {
                            if (mode == DXF_LOAD_MODE_LINE) {
                                list_push_back(&stretchy_list, { DXF_ENTITY_TYPE_LINE, entity.color, entity.line.start_x, entity.line.start_y, entity.line.end_x, entity.line.end_y });
                            } else {
                                ASSERT(mode == DXF_LOAD_MODE_ARC);
                                list_push_back(&stretchy_list, { DXF_ENTITY_TYPE_ARC, entity.color, entity.arc.center_x, entity.arc.center_y, entity.arc.radius, entity.arc.start_angle_in_degrees, entity.arc.end_angle_in_degrees });
                            }
                            mode = DXF_LOAD_MODE_NONE;
                            code_is_hot = false;
                        }
                    } else {
                        if (code == 62) {
                            int value;
                            sscanf(buffer, "%d", &value);
                            entity.color = value; 
                        } else {
                            float value;
                            sscanf(buffer, "%f", &value);
                            if (mode == DXF_LOAD_MODE_LINE) {
                                if (code == 10) {
                                    entity.line.start_x = MM(value);
                                } else if (code == 20) {
                                    entity.line.start_y = MM(value);
                                } else if (code == 11) {
                                    entity.line.end_x = MM(value);
                                } else if (code == 21) {
                                    entity.line.end_y = MM(value);
                                }
                            } else {
                                ASSERT(mode == DXF_LOAD_MODE_ARC);
                                if (code == 10) {
                                    entity.arc.center_x = MM(value);
                                } else if (code == 20) {
                                    entity.arc.center_y = MM(value);
                                } else if (code == 40) {
                                    entity.arc.radius = MM(value);
                                } else if (code == 50) {
                                    entity.arc.start_angle_in_degrees = value;
                                } else if (code == 51) {
                                    entity.arc.end_angle_in_degrees = value;
                                }
                            }
                        }
                    }
                    code_is_hot = !code_is_hot;
                }
            }
        }
    }

    fclose(file);

    dxf->num_entities = stretchy_list.length;
    dxf->entities = (DXFEntity *) calloc(dxf->num_entities, sizeof(DXFEntity));
    memcpy(dxf->entities, stretchy_list.data, dxf->num_entities * sizeof(DXFEntity));
    list_free(&stretchy_list);

    conversation_messagef("[load] loaded %s", filename);
}

void _dxf_eso_color(u32 color) {
    if      (color == 0) { eso_color( 83 / 255.0f, 255 / 255.0f,  85 / 255.0f); }
    else if (color == 1) { eso_color(255 / 255.0f,   0 / 255.0f,   0 / 255.0f); }
    else if (color == 2) { eso_color(238 / 255.0f,   0 / 255.0f, 119 / 255.0f); }
    else if (color == 3) { eso_color(255 / 255.0f,   0 / 255.0f, 255 / 255.0f); }
    else if (color == 4) { eso_color(170 / 255.0f,   1 / 255.0f, 255 / 255.0f); }
    else if (color == 5) { eso_color(  0 / 255.0f,  85 / 255.0f, 255 / 255.0f); }
    else if (color == 6) { eso_color(136 / 255.0f, 136 / 255.0f, 136 / 255.0f); }
    else if (color == 7) { eso_color(205 / 255.0f, 205 / 255.0f, 205 / 255.0f); }
    else if (color == 8) { eso_color(  0 / 255.0f, 255 / 255.0f, 255 / 255.0f); }
    else if (color == 9) { eso_color(204 / 255.0f, 136 / 255.0f,   1 / 255.0f); }
    else if (color == DXF_COLOR_SELECTION) { eso_color(1.0f, 1.0f, 0.0f); }
    else {
        // printf("WARNING: slits not implemented\n");
        eso_color(1.0, 1.0, 1.0);
    }
}

void eso_dxf_entity__SOUP_LINES(DXFEntity *entity, int32 override_color = DXF_COLOR_DONT_OVERRIDE) {
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLine *line = &entity->line;
        _dxf_eso_color((override_color != DXF_COLOR_DONT_OVERRIDE) ? override_color : entity->color);
        eso_vertex(line->start_x, line->start_y);
        eso_vertex(line->end_x,   line->end_y);
    } else {
        ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
        DXFArc *arc = &entity->arc;
        real32 start_angle, end_angle;
        arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, false);
        real32 delta_angle = end_angle - start_angle;
        u32 num_segments = (u32) (1 + (delta_angle / TAU) * NUM_SEGMENTS_PER_CIRCLE);
        real32 increment = delta_angle / num_segments;
        real32 current_angle = start_angle;
        _dxf_eso_color((override_color != DXF_COLOR_DONT_OVERRIDE) ? override_color : entity->color);
        for (u32 i = 0; i < num_segments; ++i) {
            real32 x, y;
            get_point_on_circle_NOTE_pass_angle_in_radians(&x, &y, arc->center_x, arc->center_y, arc->radius, current_angle);
            eso_vertex(x, y);
            current_angle += increment;
            get_point_on_circle_NOTE_pass_angle_in_radians(&x, &y, arc->center_x, arc->center_y, arc->radius, current_angle);
            eso_vertex(x, y);
        }
    }
}

void dxf_debug_draw(Camera2D *camera2D, DXF *dxf, int32 override_color = DXF_COLOR_DONT_OVERRIDE) {
    eso_begin(camera_get_PV(camera2D), SOUP_LINES);
    for (DXFEntity *entity = dxf->entities; entity < &dxf->entities[dxf->num_entities]; ++entity) {
        eso_dxf_entity__SOUP_LINES(entity, override_color);
    }
    eso_end();
}




real32 squared_distance_point_point(real32 x_A, real32 y_A, real32 x_B, real32 y_B) {
    real32 dx = (x_A - x_B);
    real32 dy = (y_A - y_B);
    return (dx * dx) + (dy * dy);
};

real32 squared_distance_point_line_segment(real32 x, real32 y, real32 start_x, real32 start_y, real32 end_x, real32 end_y) {
    real32 l2 = squared_distance_point_point(start_x, start_y, end_x, end_y);
    if (l2 < TINY_VAL) return squared_distance_point_point(x, y, start_x, start_y);
    real32 num = (x - start_x) * (end_x - start_x) + (y - start_y) * (end_y - start_y);
    real32 t = MIN(MAX(num / l2, 0), 1);
    real32 P_x = start_x + t * (end_x - start_x);
    real32 P_y = start_y + t * (end_y - start_y);
    return (P_x - x) * (P_x - x) + (P_y - y) * (P_y - y);
}

real32 squared_distance_point_circle(real32 x, real32 y, real32 center_x, real32 center_y, real32 radius) {
    return POW(SQRT(squared_distance_point_point(x, y, center_x, center_y)) - radius, 2);
}

real32 squared_distance_point_arc_NOTE_pass_angles_in_radians(real32 x, real32 y, real32 center_x, real32 center_y, real32 radius, real32 start_angle_in_radians, real32 end_angle_in_radians) {
    bool32 point_in_sector = false; {
        real32 v_x = x - center_x;
        real32 v_y = y - center_y;
        // forgive me rygorous :(
        real32 angle = atan2(v_y, v_x);
        while (start_angle_in_radians < -PI) start_angle_in_radians += TAU;
        while (end_angle_in_radians < start_angle_in_radians) end_angle_in_radians += TAU;
        point_in_sector =
            IS_BETWEEN(angle, start_angle_in_radians, end_angle_in_radians)
            || IS_BETWEEN(angle + TAU, start_angle_in_radians, end_angle_in_radians)
            || IS_BETWEEN(angle - TAU, start_angle_in_radians, end_angle_in_radians);
    }
    if (point_in_sector) {
        return squared_distance_point_circle(x, y, center_x, center_y, radius);
    } else {
        real32 start_x, start_y, end_x, end_y;
        get_point_on_circle_NOTE_pass_angle_in_radians(&start_x, &start_y, center_x, center_y, radius, start_angle_in_radians);
        get_point_on_circle_NOTE_pass_angle_in_radians(  &end_x,   &end_y, center_x, center_y, radius, end_angle_in_radians);
        return MIN(squared_distance_point_point(x, y, start_x, start_y), squared_distance_point_point(x, y, end_x, end_y));
    }
}

real32 squared_distance_point_dxf_line(real32 x, real32 y, DXFLine *line) {
    return squared_distance_point_line_segment(x, y, line->start_x, line->start_y, line->end_x, line->end_y);
}

real32 squared_distance_point_dxf_arc(real32 x, real32 y, DXFArc *arc) {
    return squared_distance_point_arc_NOTE_pass_angles_in_radians(x, y, arc->center_x, arc->center_y, arc->radius, RAD(arc->start_angle_in_degrees), RAD(arc->end_angle_in_degrees));
}

real32 squared_distance_point_dxf_entity(real32 x, real32 y, DXFEntity *entity) {
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLine *line = &entity->line;
        return squared_distance_point_dxf_line(x, y, line);
    } else {
        ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
        DXFArc *arc = &entity->arc;
        return squared_distance_point_dxf_arc(x, y, arc);
    }
}

real32 squared_distance_point_dxf(real32 x, real32 y, DXF *dxf) {
    real32 result = HUGE_VAL;
    for (DXFEntity *entity = dxf->entities; entity < &dxf->entities[dxf->num_entities]; ++entity) {
        result = MIN(result, squared_distance_point_dxf_entity(x, y, entity));
    }
    return result;
}




struct DXFEntityIndexAndFlipFlag {
    u32 entity_index;
    bool32 flip_flag;
};

struct DXFLoopAnalysisResult {
    u32 num_loops;
    u32 *num_entities_in_loops;
    DXFEntityIndexAndFlipFlag **loops;
    u32 *loop_index_from_entity_index;
};

DXFLoopAnalysisResult dxf_loop_analysis_create(DXF *dxf, bool32 *dxf_selection_mask_NOTE_pass_NULL_for_pick = NULL) {
    if (dxf->num_entities == 0) {
        DXFLoopAnalysisResult result = {};
        result.num_loops = 0;
        result.num_entities_in_loops = (u32 *) calloc(result.num_loops, sizeof(u32));
        result.loops = (DXFEntityIndexAndFlipFlag **) calloc(result.num_loops, sizeof(DXFEntityIndexAndFlipFlag *));
        result.loop_index_from_entity_index = (u32 *) calloc(dxf->num_entities, sizeof(u32));
        return result;
    }

    DXFLoopAnalysisResult result = {};
    { // num_entities_in_loops, loops
      // populate List's
        List<List<DXFEntityIndexAndFlipFlag>> stretchy_list = {}; {
            bool32 *entity_already_added = (bool32 *) calloc(dxf->num_entities, sizeof(bool32));
            while (true) {
                #define MACRO_CANDIDATE_VALID(i) (!entity_already_added[i] && (!dxf_selection_mask_NOTE_pass_NULL_for_pick || dxf_selection_mask_NOTE_pass_NULL_for_pick[i]))
                { // seed loop
                    bool32 added_and_seeded_new_loop = false;
                    for (u32 entity_index = 0; entity_index < dxf->num_entities; ++entity_index) {
                        if (MACRO_CANDIDATE_VALID(entity_index)) {
                            added_and_seeded_new_loop = true;
                            entity_already_added[entity_index] = true;
                            list_push_back(&stretchy_list, {});
                            list_push_back(&stretchy_list.data[stretchy_list.length - 1], { entity_index, false });
                            break;
                        }
                    }
                    if (!added_and_seeded_new_loop) break;
                }
                { // continue and complete
                    real32 tolerance = TOLERANCE_DEFAULT;
                    while (true) {
                        bool32 added_new_entity_to_loop = false;
                        for (u32 entity_index = 0; entity_index < dxf->num_entities; ++entity_index) {
                            if (!MACRO_CANDIDATE_VALID(entity_index)) continue;
                            real32 start_x_prev, start_y_prev, end_x_prev, end_y_prev;
                            real32 start_x_i, start_y_i, end_x_i, end_y_i;
                            DXFEntityIndexAndFlipFlag *prev_entity_index_and_flip_flag = &(stretchy_list.data[stretchy_list.length - 1].data[stretchy_list.data[stretchy_list.length - 1].length - 1]);
                            {
                                entity_get_start_and_end_points(
                                        &dxf->entities[prev_entity_index_and_flip_flag->entity_index],
                                        &start_x_prev, &start_y_prev, &end_x_prev, &end_y_prev);
                                entity_get_start_and_end_points(&dxf->entities[entity_index], &start_x_i, &start_y_i, &end_x_i, &end_y_i);
                            }
                            bool32 is_next_entity = false;
                            bool32 flip_flag = false;
                            if (!prev_entity_index_and_flip_flag->flip_flag) {
                                if (squared_distance_point_point(end_x_prev, end_y_prev, start_x_i, start_y_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = false;
                                } else if (squared_distance_point_point(end_x_prev, end_y_prev, end_x_i, end_y_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = true;
                                }
                            } else {
                                if (squared_distance_point_point(start_x_prev, start_y_prev, start_x_i, start_y_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = false;
                                } else if (squared_distance_point_point(start_x_prev, start_y_prev, end_x_i, end_y_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = true;
                                }
                            }
                            if (is_next_entity) {
                                added_new_entity_to_loop = true;
                                entity_already_added[entity_index] = true;
                                list_push_back(&stretchy_list.data[stretchy_list.length - 1], { entity_index, flip_flag });
                                break;
                            }
                        }
                        if (!added_new_entity_to_loop) break;
                    }
                }

                { // reverse_loop if necessary
                    u32 num_entities_in_loop = stretchy_list.data[stretchy_list.length - 1].length;
                    DXFEntityIndexAndFlipFlag *loop = stretchy_list.data[stretchy_list.length - 1].data;
                    bool32 reverse_loop; {
                        #if 0
                        reverse_loop = false;
                        #else
                        real32 twice_the_signed_area; {
                            twice_the_signed_area = 0.0f;
                            for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities_in_loop; ++entity_index_and_flip_flag) {
                                u32 entity_index = entity_index_and_flip_flag->entity_index;
                                bool32 flip_flag = entity_index_and_flip_flag->flip_flag;
                                DXFEntity *entity = &dxf->entities[entity_index];
                                if (entity->type == DXF_ENTITY_TYPE_LINE) {
                                    DXFLine *line = &entity->line;
                                    // shoelace-type formula
                                    twice_the_signed_area += ((flip_flag) ? -1 : 1) * (line->start_x * line->end_y - line->end_x * line->start_y);
                                } else {
                                    ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                                    DXFArc *arc = &entity->arc;
                                    // "Circular approximation using polygons"
                                    // - n = 2 (area-preserving approximation of arc with two segments)
                                    real32 start_angle, end_angle;
                                    arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, flip_flag);
                                    real32 start_x, start_y, end_x, end_y;
                                    get_point_on_circle_NOTE_pass_angle_in_radians(&start_x, &start_y, arc->center_x, arc->center_y, arc->radius, start_angle);
                                    get_point_on_circle_NOTE_pass_angle_in_radians(  &end_x,   &end_y, arc->center_x, arc->center_y, arc->radius,   end_angle);
                                    real32 mid_angle = (start_angle + end_angle) / 2;
                                    real32 d; {
                                        real32 alpha = ABS(start_angle - end_angle) / 2;
                                        d = arc->radius * alpha / SIN(alpha);
                                    }
                                    real32 mid_x, mid_y;
                                    get_point_on_circle_NOTE_pass_angle_in_radians(&mid_x, &mid_y, arc->center_x, arc->center_y, d, mid_angle);
                                    twice_the_signed_area += mid_x * (end_y - start_y) + mid_y * (start_x - end_x);
                                }
                            }
                        }
                        reverse_loop = (twice_the_signed_area < 0.0f);
                        #endif
                    }
                    if (reverse_loop) {
                        for (u32 i = 0, j = (num_entities_in_loop - 1); i < j; ++i, --j) {
                            DXFEntityIndexAndFlipFlag tmp = loop[i];
                            loop[i] = loop[j];
                            loop[j] = tmp;
                        }
                        for (u32 i = 0; i < num_entities_in_loop; ++i) {
                            loop[i].flip_flag = !loop[i].flip_flag;
                        }
                    }
                }
                #undef MACRO_CANDIDATE_VALID
            }
            free(entity_already_added);
        }

        // copy over from List's
        result.num_loops = stretchy_list.length;
        result.num_entities_in_loops = (u32 *) calloc(result.num_loops, sizeof(u32));
        result.loops = (DXFEntityIndexAndFlipFlag **) calloc(result.num_loops, sizeof(DXFEntityIndexAndFlipFlag *));
        for (u32 i = 0; i < result.num_loops; ++i) {
            result.num_entities_in_loops[i] = stretchy_list.data[i].length;
            result.loops[i] = (DXFEntityIndexAndFlipFlag *) calloc(result.num_entities_in_loops[i], sizeof(DXFEntityIndexAndFlipFlag));
            memcpy(result.loops[i], stretchy_list.data[i].data, result.num_entities_in_loops[i] * sizeof(DXFEntityIndexAndFlipFlag));
        }

        // free List's
        for (u32 i = 0; i < stretchy_list.length; ++i) list_free(&stretchy_list.data[i]);
        list_free(&stretchy_list);
    }
    // loop_index_from_entity_index (brute force)
    result.loop_index_from_entity_index = (u32 *) calloc(dxf->num_entities, sizeof(u32));
    for (u32 i = 0; i < dxf->num_entities; ++i) {
        for (u32 j = 0; j < result.num_loops; ++j) {
            for (u32 k = 0; k < result.num_entities_in_loops[j]; ++k) {
                if (i == result.loops[j][k].entity_index) {
                    result.loop_index_from_entity_index[i] = j;
                    break;
                }
            }
        }
    }
    return result;
}

void dxf_loop_analysis_free(DXFLoopAnalysisResult *analysis) {
    if (analysis->num_entities_in_loops) {
        free(analysis->num_entities_in_loops);
    }
    if (analysis->loops) {
        for (u32 i = 0; i < analysis->num_loops; ++i) {
            free(analysis->loops[i]);
        }
        free(analysis->loops);
    }
    if (analysis->loop_index_from_entity_index) {
        free(analysis->loop_index_from_entity_index);
    }
    *analysis = {};
}




struct CrossSectionEvenOdd {
    u32 num_polygonal_loops;
    u32 *num_vertices_in_polygonal_loops;
    vec2 **polygonal_loops;
};

CrossSectionEvenOdd cross_section_create(DXF *dxf, bool32 *dxf_selection_mask) {
    #if 0
    {
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(dxf);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(dxf_selection_mask);
        CrossSectionEvenOdd result = {};
        result.num_polygonal_loops = 2;
        result.num_vertices_in_polygonal_loops = (u32 *) calloc(result.num_polygonal_loops, sizeof(u32));
        result.num_vertices_in_polygonal_loops[0] = 4;
        result.num_vertices_in_polygonal_loops[1] = 6;
        result.polygonal_loops = (vec2 **) calloc(result.num_polygonal_loops, sizeof(vec2 *));
        result.polygonal_loops[0] = (vec2 *) calloc(result.num_vertices_in_polygonal_loops[0], sizeof(vec2));
        result.polygonal_loops[1] = (vec2 *) calloc(result.num_vertices_in_polygonal_loops[1], sizeof(vec2));
        result.polygonal_loops[0][0] = { -2.0f, -2.0f };
        result.polygonal_loops[0][1] = {  2.0f, -2.0f };
        result.polygonal_loops[0][2] = {  2.0f,  2.0f };
        result.polygonal_loops[0][3] = { -2.0f,  2.0f };
        result.polygonal_loops[1][0] = { COS(RAD(  0)), SIN(RAD(  0)) };
        result.polygonal_loops[1][1] = { COS(RAD( 60)), SIN(RAD( 60)) };
        result.polygonal_loops[1][2] = { COS(RAD(120)), SIN(RAD(120)) };
        result.polygonal_loops[1][3] = { COS(RAD(180)), SIN(RAD(180)) };
        result.polygonal_loops[1][4] = { COS(RAD(240)), SIN(RAD(240)) };
        result.polygonal_loops[1][5] = { COS(RAD(300)), SIN(RAD(300)) };
        return result;
    }
    #endif
    // populate List's
    List<List<vec2>> stretchy_list = {}; {
        DXFLoopAnalysisResult analysis = dxf_loop_analysis_create(dxf, dxf_selection_mask);
        for (u32 loop_index = 0; loop_index < analysis.num_loops; ++loop_index) {
            u32 num_entities_in_loop = analysis.num_entities_in_loops[loop_index];
            DXFEntityIndexAndFlipFlag *loop = analysis.loops[loop_index];
            list_push_back(&stretchy_list, {});
            for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities_in_loop; ++entity_index_and_flip_flag) {
                u32 entity_index = entity_index_and_flip_flag->entity_index;
                bool32 flip_flag = entity_index_and_flip_flag->flip_flag;
                DXFEntity *entity = &dxf->entities[entity_index];
                if (entity->type == DXF_ENTITY_TYPE_LINE) {
                    DXFLine *line = &entity->line;
                    if (!flip_flag) {
                        list_push_back(&stretchy_list.data[stretchy_list.length - 1], { line->start_x, line->start_y });
                    } else {
                        list_push_back(&stretchy_list.data[stretchy_list.length - 1], { line->end_x, line->end_y });
                    }
                } else {
                    ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                    DXFArc *arc = &entity->arc;
                    real32 start_angle, end_angle;
                    arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, flip_flag);
                    real32 delta_angle = end_angle - start_angle;
                    u32 num_segments = (u32) (2 + ABS(delta_angle) * (NUM_SEGMENTS_PER_CIRCLE / TAU)); // FORNOW (2 + ...)
                    real32 increment = delta_angle / num_segments;
                    real32 current_angle = start_angle;
                    real32 x, y;
                    for (u32 i = 0; i < num_segments; ++i) {
                        get_point_on_circle_NOTE_pass_angle_in_radians(&x, &y, arc->center_x, arc->center_y, arc->radius, current_angle);
                        list_push_back(&stretchy_list.data[stretchy_list.length - 1], { x, y });
                        current_angle += increment;
                    }
                }
            }
        }
        dxf_loop_analysis_free(&analysis);
    }

    // copy over from List's
    CrossSectionEvenOdd result = {};
    result.num_polygonal_loops = stretchy_list.length;
    result.num_vertices_in_polygonal_loops = (u32 *) calloc(result.num_polygonal_loops, sizeof(u32));
    result.polygonal_loops = (vec2 **) calloc(result.num_polygonal_loops, sizeof(vec2 *));
    for (u32 i = 0; i < result.num_polygonal_loops; ++i) {
        result.num_vertices_in_polygonal_loops[i] = stretchy_list.data[i].length;
        result.polygonal_loops[i] = (vec2 *) calloc(result.num_vertices_in_polygonal_loops[i], sizeof(vec2));
        memcpy(result.polygonal_loops[i], stretchy_list.data[i].data, result.num_vertices_in_polygonal_loops[i] * sizeof(vec2));
    }

    // free List's
    for (u32 i = 0; i < stretchy_list.length; ++i) list_free(&stretchy_list.data[i]);
    list_free(&stretchy_list);

    return result;
}

void cross_section_debug_draw(Camera2D *camera2D, CrossSectionEvenOdd *cross_section) {
    eso_begin(camera_get_PV(camera2D), SOUP_LINES);
    eso_color(monokai.white);
    for (u32 loop_index = 0; loop_index < cross_section->num_polygonal_loops; ++loop_index) {
        vec2 *polygonal_loop = cross_section->polygonal_loops[loop_index];
        int n = cross_section->num_vertices_in_polygonal_loops[loop_index];
        for (int j = 0, i = n - 1; j < n; i = j++) {
            real32 a_x = polygonal_loop[i].x;
            real32 a_y = polygonal_loop[i].y;
            real32 b_x = polygonal_loop[j].x;
            real32 b_y = polygonal_loop[j].y;
            eso_color(color_rainbow_swirl(real32(i) / (n)));
            eso_vertex(a_x, a_y);
            eso_color(color_rainbow_swirl(real32(i + 1) / (n)));
            eso_vertex(b_x, b_y);

            // draw normal
            real32 c_x = (a_x + b_x) / 2;
            real32 c_y = (a_y + b_y) / 2;
            real32 n_x = b_y - a_y;
            real32 n_y = a_x - b_x;
            real32 norm_n = SQRT(n_x * n_x + n_y * n_y);
            real32 L = 0.013f;
            eso_color(color_rainbow_swirl((i + 0.5f) / (n)));
            eso_vertex(c_x, c_y);
            eso_vertex(c_x + L * n_x / norm_n, c_y + L * n_y / norm_n);
        }
    }
    eso_end();
}




////////////////////////////////////////
// 3D //////////////////////////////////
////////////////////////////////////////








struct FancyMesh {
    u32 num_vertices;
    u32 num_triangles;
    real32 *vertex_positions;
    u32 *triangle_indices;
    real32 *triangle_normals;

    u32 num_cosmetic_edges;
    u32 *cosmetic_edges;
};

void fancy_mesh_triangle_normals_calculate(FancyMesh *fancy_mesh) {
    fancy_mesh->triangle_normals = (real32 *) malloc(fancy_mesh->num_triangles * 3 * sizeof(real32));
    vec3 p[3];
    for (u32 i = 0; i < fancy_mesh->num_triangles; ++i) {
        for (u32 j = 0; j < 3; ++j) p[j] = get(fancy_mesh->vertex_positions, fancy_mesh->triangle_indices[3 * i + j]);
        vec3 n = normalized(cross(p[1] - p[0], p[2] - p[0]));
        set(fancy_mesh->triangle_normals, i, n);
    }
}

void fancy_mesh_cosmetic_edges_calculate(FancyMesh *fancy_mesh) {
    // approach: prep a big array that maps edge -> cwiseProduct of face normals (start it at 1, 1, 1) // (faces that edge is part of)
    //           iterate through all edges detministically (ccw in order, flipping as needed so lower_index->higher_index)
    //           then go back and if passes some heuristic add that index to a stretchy buffer
    List<u32> list = {}; {
        Map<Pair<u32, u32>, vec3> map = {}; {
            for (u32 i = 0; i < fancy_mesh->num_triangles; ++i) {
                vec3 n = get(fancy_mesh->triangle_normals, i);
                for (u32 jj0 = 0, jj1 = (3 - 1); jj0 < 3; jj1 = jj0++) {
                    u32 j0 = fancy_mesh->triangle_indices[3 * i + jj0];
                    u32 j1 = fancy_mesh->triangle_indices[3 * i + jj1];
                    if (j0 > j1) {
                        u32 tmp = j0;
                        j0 = j1;
                        j1 = tmp;
                    }
                    Pair<u32, u32> key = { j0, j1 };
                    map_put(&map, key, cwiseProduct(n, map_get(&map, key, V3(1.0f))));
                }
            }
        }
        {
            for (List<Pair<Pair<u32, u32>, vec3>> *bucket = map.buckets; bucket < &map.buckets[map.num_buckets]; ++bucket) {
                for (Pair<Pair<u32, u32>, vec3> *pair = bucket->data; pair < &bucket->data[bucket->length]; ++pair) {
                    vec3 n2 = pair->value;
                    // pprint(n2);
                    if (squaredNorm(n2) < 0.3f) {
                        list_push_back(&list, pair->key.first); // FORNOW
                        list_push_back(&list, pair->key.second); // FORNOW
                    }
                }
            }
        }
        map_free(&map);
    }
    {
        fancy_mesh->num_cosmetic_edges = list.length / 2;
        fancy_mesh->cosmetic_edges = (u32 *) calloc(2 * fancy_mesh->num_cosmetic_edges, sizeof(u32));
        memcpy(fancy_mesh->cosmetic_edges, list.data, 2 * fancy_mesh->num_cosmetic_edges * sizeof(u32)); 
    }
    list_free(&list);
}


void stl_save(FancyMesh *fancy_mesh, char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        conversation_messagef("[save] could not save open %s for writing.", filename);
        return;
    }

    int num_bytes = 80 + 4 + 50 * fancy_mesh->num_triangles;
    char *buffer = (char *) calloc(num_bytes, 1); {
        int offset = 80;
        memcpy(buffer + offset, &fancy_mesh->num_triangles, 4);
        offset += 4;
        for (u32 i = 0; i < fancy_mesh->num_triangles; ++i) {
            real32 triangle_normal[3];
            {
                // 90 degree rotation about x: (x, y, z) <- (x, -z, y)
                triangle_normal[0] =  fancy_mesh->triangle_normals[3 * i + 0];
                triangle_normal[1] = -fancy_mesh->triangle_normals[3 * i + 2];
                triangle_normal[2] =  fancy_mesh->triangle_normals[3 * i + 1];
            }
            memcpy(buffer + offset, &triangle_normal, 12);
            offset += 12;
            real32 triangle_vertex_positions[9];
            for (u32 j = 0; j < 3; ++j) {
                // 90 degree rotation about x: (x, y, z) <- (x, -z, y)
                triangle_vertex_positions[3 * j + 0] =  fancy_mesh->vertex_positions[3 * fancy_mesh->triangle_indices[3 * i + j] + 0];
                triangle_vertex_positions[3 * j + 1] = -fancy_mesh->vertex_positions[3 * fancy_mesh->triangle_indices[3 * i + j] + 2];
                triangle_vertex_positions[3 * j + 2] =  fancy_mesh->vertex_positions[3 * fancy_mesh->triangle_indices[3 * i + j] + 1];
            }
            memcpy(buffer + offset, triangle_vertex_positions, 36);
            offset += 38;
        }
    }
    fwrite(buffer, 1, num_bytes, file);
    free(buffer);

    fclose(file);
    conversation_messagef("[save] saved %s", filename);
}

void fancy_mesh_free(FancyMesh *fancy_mesh) {
    if (fancy_mesh->vertex_positions) free(fancy_mesh->vertex_positions);
    if (fancy_mesh->triangle_indices) free(fancy_mesh->triangle_indices);
    if (fancy_mesh->triangle_normals) free(fancy_mesh->triangle_normals);
    if (fancy_mesh->cosmetic_edges)   free(fancy_mesh->cosmetic_edges);
    *fancy_mesh = {};
}





struct HistoryState {
    ManifoldManifold *manifold_manifold;
    FancyMesh fancy_mesh;
};

struct History {
    List<HistoryState> undo_stack;
    List<HistoryState> redo_stack;
};
void history_record_state(History *history, ManifoldManifold **manifold_manifold, FancyMesh *fancy_mesh) {
    list_push_back(&history->undo_stack, { *manifold_manifold, *fancy_mesh });
    { // free redo_stack
        for (u32 i = 0; i < history->redo_stack.length; ++i) {
            fancy_mesh_free(&history->redo_stack.data[i].fancy_mesh);
            // TODO: manifold_manifold
        }
        list_free(&history->redo_stack);
    }
}
void history_undo(History *history, ManifoldManifold **manifold_manifold, FancyMesh *fancy_mesh) {
    if (history->undo_stack.length != 0) {
        list_push_back(&history->redo_stack, { *manifold_manifold, *fancy_mesh });
        HistoryState state = list_pop_back(&history->undo_stack);
        *manifold_manifold = state.manifold_manifold;
        *fancy_mesh = state.fancy_mesh;
    } else {
        conversation_messagef("[history] undo stack is empty");
    }
}
void history_redo(History *history, ManifoldManifold **manifold_manifold, FancyMesh *fancy_mesh) {
    if (history->redo_stack.length != 0) {
        list_push_back(&history->undo_stack, { *manifold_manifold, *fancy_mesh });
        HistoryState state = list_pop_back(&history->redo_stack);
        *manifold_manifold = state.manifold_manifold;
        *fancy_mesh = state.fancy_mesh;
    } else {
        conversation_messagef("[history] redo stack is empty");
    }
}
void history_free(History *history) {
    { // free undo_stack
        for (u32 i = 0; i < history->undo_stack.length; ++i) {
            fancy_mesh_free(&history->undo_stack.data[i].fancy_mesh);
            // TODO: manifold_manifold
        }
        list_free(&history->undo_stack);
    }

    { // free redo_stack
        for (u32 i = 0; i < history->redo_stack.length; ++i) {
            fancy_mesh_free(&history->redo_stack.data[i].fancy_mesh);
            // TODO: manifold_manifold
        }
        list_free(&history->redo_stack);
    }
}





void wrapper_manifold(
        ManifoldManifold **manifold_manifold,
        FancyMesh *fancy_mesh, // dest__NOTE_GETS_OVERWRITTEN,
        u32 num_polygonal_loops,
        u32 *num_vertices_in_polygonal_loops,
        vec2 **polygonal_loops,
        mat4 M_3D_from_2D,
        u32 enter_mode,
        real32 console_param,
        real32 console_param_2,
        History *history) {


    // FORNOW: this function call isn't a no-op
    history_record_state(history, manifold_manifold, fancy_mesh);


    ASSERT(enter_mode != ENTER_MODE_NONE);

    ManifoldManifold *other_manifold; {
        ManifoldSimplePolygon **simple_polygon_array = (ManifoldSimplePolygon **) malloc(num_polygonal_loops * sizeof(ManifoldSimplePolygon *));
        for (u32 i = 0; i < num_polygonal_loops; ++i) {
            simple_polygon_array[i] = manifold_simple_polygon(malloc(manifold_simple_polygon_size()), (ManifoldVec2 *) polygonal_loops[i], num_vertices_in_polygonal_loops[i]);
        }
        ManifoldPolygons *polygons = manifold_polygons(malloc(manifold_polygons_size()), simple_polygon_array, num_polygonal_loops);
        ManifoldCrossSection *cross_section = manifold_cross_section_of_polygons(malloc(manifold_cross_section_size()), polygons, ManifoldFillRule::MANIFOLD_FILL_RULE_EVEN_ODD);


        { // other_manifold

            if (enter_mode == ENTER_MODE_EXTRUDE_CUT) {
                do_once { printf("[hack] inflating ENTER_MODE_EXTRUDE_CUT\n");};
                console_param += SGN(console_param) * TOLERANCE_DEFAULT;
                console_param_2 += SGN(console_param_2) * TOLERANCE_DEFAULT;
            }

            // NOTE: params are arbitrary sign (and can be same sign)--a typical thing would be like (30, -30)
            //       but we support (30, 40) -- which is equivalent to (40, 0)

            if (enter_mode == ENTER_MODE_EXTRUDE_ADD || enter_mode == ENTER_MODE_EXTRUDE_CUT) {
                real32 min = MIN(0.0f, MIN(console_param, console_param_2));
                real32 max = MAX(0.0f, MAX(console_param, console_param_2));
                real32 length = max - min;

                other_manifold = manifold_extrude(malloc(manifold_manifold_size()), cross_section, length, 0, 0.0f, 1.0f, 1.0f);
                other_manifold = manifold_translate(other_manifold, other_manifold, 0.0f, 0.0f, min);
                // if (console_params_preview_flip_flag) other_manifold = manifold_mirror(other_manifold, other_manifold, 0.0f, 0.0f, 1.0f);

            } else {
                // TODO
                other_manifold = manifold_revolve(malloc(manifold_manifold_size()), cross_section, NUM_SEGMENTS_PER_CIRCLE);
                other_manifold = manifold_rotate(other_manifold, other_manifold, -90.0f, 0.0f, 0.0f);
            }
            other_manifold = manifold_transform(other_manifold, other_manifold,
                    M_3D_from_2D(0, 0), M_3D_from_2D(1, 0), M_3D_from_2D(2, 0),
                    M_3D_from_2D(0, 1), M_3D_from_2D(1, 1), M_3D_from_2D(2, 1),
                    M_3D_from_2D(0, 2), M_3D_from_2D(1, 2), M_3D_from_2D(2, 2),
                    M_3D_from_2D(0, 3), M_3D_from_2D(1, 3), M_3D_from_2D(2, 3));
        }
    }

    // add
    if (!(*manifold_manifold)) {
        ASSERT((enter_mode != ENTER_MODE_EXTRUDE_CUT) && (enter_mode != ENTER_MODE_REVOLVE_CUT));

        *manifold_manifold = other_manifold;
    } else {
        // TODO: ? manifold_delete_manifold(manifold_manifold);
        *manifold_manifold =
            manifold_boolean(
                    malloc(manifold_manifold_size()),
                    *manifold_manifold,
                    other_manifold,
                    ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_ADD)) ? ManifoldOpType::MANIFOLD_ADD : ManifoldOpType::MANIFOLD_SUBTRACT
                    );
    }

    ManifoldMeshGL *meshgl = manifold_get_meshgl(malloc(manifold_meshgl_size()), *manifold_manifold);

    // // NOTE: don't free ANYTHING!--putting the current state on the undo stack
    // XXX fancy_mesh_free(fancy_mesh);
    fancy_mesh->num_vertices = manifold_meshgl_num_vert(meshgl);
    fancy_mesh->num_triangles = manifold_meshgl_num_tri(meshgl);
    fancy_mesh->vertex_positions = manifold_meshgl_vert_properties(malloc(manifold_meshgl_vert_properties_length(meshgl) * sizeof(real32)), meshgl);
    fancy_mesh->triangle_indices = manifold_meshgl_tri_verts(malloc(manifold_meshgl_tri_length(meshgl) * sizeof(u32)), meshgl);
    fancy_mesh_triangle_normals_calculate(fancy_mesh);
    fancy_mesh_cosmetic_edges_calculate(fancy_mesh);
}





void stl_load(char *filename, ManifoldManifold **manifold_manifold, FancyMesh *fancy_mesh, History *history) {

    history_record_state(history, manifold_manifold, fancy_mesh); // FORNOW

    { // fancy_mesh
        u32 num_triangles;
        real32 *soup;
        {
            static char line_of_file[512];

            #define STL_FILETYPE_UNKNOWN 0
            #define STL_FILETYPE_ASCII   1
            #define STL_FILETYPE_BINARY  2
            u32 filetype; {
                FILE *file = (FILE *) fopen(filename, "r");
                fgets(line_of_file, 80, file);
                filetype = (poe_prefix_match(line_of_file, "solid")) ? STL_FILETYPE_ASCII : STL_FILETYPE_BINARY;
                fclose(file);
            }

            if (filetype == STL_FILETYPE_ASCII) {
                u8 ascii_scan_dummy[64];
                real32 ascii_scan_p[3];
                List<real32> ascii_data = {};

                FILE *file = (FILE *) fopen(filename, "r");
                while (fgets(line_of_file, ARRAY_LENGTH(line_of_file), file)) {
                    if (poe_prefix_match(line_of_file, "vertex")) {
                        sscanf(line_of_file, "%s %f %f %f", ascii_scan_dummy, &ascii_scan_p[0], &ascii_scan_p[1], &ascii_scan_p[2]);
                        for (u32 d = 0; d < 3; ++d) list_push_back(&ascii_data, ascii_scan_p[d]);
                    }
                }
                fclose(file);
                num_triangles = ascii_data.length / 9;
                u32 size = ascii_data.length * sizeof(real32);
                soup = (real32 *) malloc(size);
                memcpy(soup, ascii_data.data, size);
                list_free(&ascii_data);
            } else {
                ASSERT(filetype == STL_FILETYPE_BINARY);
                char *entire_file; {
                    FILE *file = fopen(filename, "rb");
                    fseek(file, 0, SEEK_END);
                    long fsize = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    entire_file = (char *) malloc(fsize + 1);
                    fread(entire_file, fsize, 1, file);
                    fclose(file);
                    entire_file[fsize] = 0;
                }
                u32 offset = 80;
                memcpy(&num_triangles, entire_file + offset, 4);
                offset += 4;
                u32 size = num_triangles * 36;
                soup = (real32 *) calloc(1, size);
                for (u32 i = 0; i < num_triangles; ++i) {
                    offset += 12;
                    memcpy(soup + i * 9, entire_file + offset, 36);
                    offset += 38;
                }
            }
            { // -90 degree rotation about x: (x, y, z) <- (x, z, -y)
                u32 num_vertices = 3 * num_triangles;
                for (u32 i = 0; i < num_vertices; ++i) {
                    real32 tmp = soup[3 * i + 1];
                    soup[3 * i + 1] = soup[3 * i + 2];
                    soup[3 * i + 2] = -tmp;
                }
            }
        }

        u32 num_vertices;
        real32 *vertex_positions;
        u32 *triangle_indices;
        { // merge vertices
            num_vertices = 0;
            Map<vec3, u32> map = {};
            u32 _3_TIMES_num_triangles = 3 * num_triangles;
            {
                List<vec3> list = {};
                for (u32 i = 0; i < _3_TIMES_num_triangles; ++i) {
                    vec3 p = get(soup, i);
                    u32 default_value = _3_TIMES_num_triangles + 1;
                    u32 j = map_get(&map, p, default_value);
                    if (j == default_value) {
                        map_put(&map, p, num_vertices++);
                        list_push_back(&list, p);
                    }
                }
                {
                    u32 size = list.length * sizeof(vec3);
                    vertex_positions = (real32 *) malloc(size);
                    memcpy(vertex_positions, list.data, size);
                }
                list_free(&list);
            }
            triangle_indices = (u32 *) malloc(_3_TIMES_num_triangles * sizeof(u32));
            for (u32 k = 0; k < _3_TIMES_num_triangles; ++k) triangle_indices[k] = map_get(&map, ((vec3 *) soup)[k]);
            map_free(&map);
            free(soup);
        }
        fancy_mesh->num_vertices = num_vertices;
        fancy_mesh->num_triangles = num_triangles;
        fancy_mesh->vertex_positions = vertex_positions;
        fancy_mesh->triangle_indices = triangle_indices;
        fancy_mesh_triangle_normals_calculate(fancy_mesh);
        fancy_mesh_cosmetic_edges_calculate(fancy_mesh);
        conversation_messagef("[load] loaded %s", filename);
    }
    { // manifold_manifold
      // FORNOW
        ManifoldMeshGL *meshgl = manifold_meshgl(
                malloc(manifold_meshgl_size()),
                fancy_mesh->vertex_positions,
                fancy_mesh->num_vertices,
                3,
                fancy_mesh->triangle_indices,
                fancy_mesh->num_triangles);
        *manifold_manifold = manifold_of_meshgl(malloc(manifold_manifold_size()), meshgl);
        manifold_delete_meshgl(meshgl);
    }
}



BoundingBox *dxf_entity_bounding_boxes_create(DXF *dxf) {
    BoundingBox *result = (BoundingBox *) malloc(dxf->num_entities * sizeof(BoundingBox));

    for (u32 entity_index = 0; entity_index < dxf->num_entities; ++entity_index) {
        DXFEntity *entity = &dxf->entities[entity_index];
        result[entity_index] = { HUGE_VAL, HUGE_VAL, -HUGE_VAL, -HUGE_VAL };
        real32 s[2][2];
        u32 n = 2;
        entity_get_start_and_end_points(entity, &s[0][0], &s[0][1], &s[1][0], &s[1][1]);
        for (u32 i = 0; i < n; ++i) {
            for (u32 d = 0; d < 2; ++d) {
                result[entity_index].min[d] = MIN(result[entity_index].min[d], s[i][d]);
                result[entity_index].max[d] = MAX(result[entity_index].max[d], s[i][d]);
            }
        }
        if (entity->type == DXF_ENTITY_TYPE_ARC) {
            DXFArc *arc = &entity->arc;
            // NOTE: endpoints already taken are of
            real32 start_angle = RAD(arc->start_angle_in_degrees);
            real32 end_angle = RAD(arc->end_angle_in_degrees);
            { // FORNOW
                while (start_angle < 0.0f) {
                    start_angle += TAU;
                    end_angle += TAU;
                }
                while (start_angle > TAU) {
                    start_angle -= TAU;
                    end_angle -= TAU;
                }
            }
            if (IS_BETWEEN(         0, start_angle, end_angle)) result[entity_index].max[0] = MAX(result[entity_index].max[0], arc->center_x + arc->radius);
            if (IS_BETWEEN(    PI / 2, start_angle, end_angle)) result[entity_index].max[1] = MAX(result[entity_index].max[1], arc->center_y + arc->radius);
            if (IS_BETWEEN(    PI    , start_angle, end_angle)) result[entity_index].min[0] = MIN(result[entity_index].min[0], arc->center_x - arc->radius);
            if (IS_BETWEEN(3 * PI / 2, start_angle, end_angle)) result[entity_index].min[1] = MIN(result[entity_index].min[1], arc->center_y - arc->radius);
        }
    }

    return result;
}










////////////////////////////////////////////////////
// GLOBALS (TODO: wrap in struct ConversationState { ... } conversation;
////////////////////////////////////////////////////

#define HOT_PANE_NONE 0
#define HOT_PANE_2D   1
#define HOT_PANE_3D   2

DXF dxf;
DXFLoopAnalysisResult pick;
BoundingBox *bbox;
bool32 *dxf_selection_mask;
ManifoldManifold *manifold_manifold;
FancyMesh fancy_mesh;
History history;
u32 enter_mode;
char console_buffer[256];
char *console_buffer_write_head;
void console_buffer_reset() {
    memset(console_buffer, 0, ARRAY_LENGTH(console_buffer) * sizeof(char));
    console_buffer_write_head = console_buffer;
};
real32 console_param;
real32 console_param_2;
bool32 console_params_preview_flip_flag;
real32 console_param_preview;
real32 console_param_2_preview;
void console_params_preview_update() {
    if (console_buffer_write_head == console_buffer) {
        console_param_preview = console_param;
        console_param_2_preview = console_param_2;
    } else {
        char buffs[2][64] = {};
        u32 buff_i = 0;
        u32 i = 0;
        for (char *c = console_buffer; (*c) != '\0'; ++c) {
            if (*c == ' ') {
                ++buff_i;
                i = 0;
                continue;
            }
            buffs[buff_i][i++] = (*c);
        }
        real32 sign = (!console_params_preview_flip_flag) ? 1.0f : -1.0f;
        console_param_preview = sign * strtof(buffs[0], NULL);
        console_param_2_preview = sign * strtof(buffs[1], NULL);
        if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
            console_param_2_preview *= -1;
        }
    }
}
Camera2D camera2D;
Camera3D camera3D;
real32 CAMERA_3D_DEFAULT_ANGLE_OF_VIEW = RAD(60.0f);
bool32 show_grid, show_details, show_help;
char conversation_drop_path[512];
u32 hot_pane;
u32 click_mode;
u32 click_modifier; // TODO combine
u32 window_select_click_count;
real32 window_select_x;
real32 window_select_y;
bool32 some_triangle_exists_that_matches_n_selected_and_r_n_selected; // NOTE: if this is false, then a plane is selected
vec3 n_selected;
real32 r_n_selected; // coordinate along n_selected
mat4 M_3D_from_2D;
void selected_reset() {
    some_triangle_exists_that_matches_n_selected_and_r_n_selected = false;
    r_n_selected = 0.0f;
    n_selected = {};
    M_3D_from_2D = {}; // FORNOW??: implicit nothing selected
}
real32 origin_x;
real32 origin_y;



// GLOBAL-TOUCHING FUNCTIONS ///////////////////////////

void conversation_zoom_camera2D_to_dxf_extents() {
    BoundingBox extents = bounding_box_union(dxf.num_entities, bbox);

    real32 new_o_x = AVG(extents.min[0], extents.max[0]);
    real32 new_o_y = AVG(extents.min[1], extents.max[1]);
    real32 new_height = MAX((extents.max[0] - extents.min[0]) * 2 / _window_get_aspect(), (extents.max[1] - extents.min[1])); // factor of 2 since splitscreen
    new_height *= 1.3f; // FORNOW: border
    camera2D.screen_height_World = new_height;
    camera2D.o_x = new_o_x;
    camera2D.o_y = new_o_y;
}

void conversation_load_dxf(char *filename) {
    FILE *file = (FILE *) fopen(filename, "r");
    if (!file) {
        conversation_messagef("[load] \"%s\" not found", filename);
        return;
    }
    fclose(file);

    dxf_free(&dxf);
    dxf_loop_analysis_free(&pick);
    free(dxf_selection_mask);
    free(bbox);

    dxf_load(filename, &dxf);
    dxf_selection_mask = (bool32 *) calloc(dxf.num_entities, sizeof(bool32));
    pick = dxf_loop_analysis_create(&dxf);
    bbox = dxf_entity_bounding_boxes_create(&dxf);

    conversation_zoom_camera2D_to_dxf_extents();
    origin_x = 0.0f;
    origin_y = 0.0f;
}
void conversation_load_stl(char *filename) {
    FILE *file = (FILE *) fopen(filename, "r");
    if (!file) {
        conversation_messagef("[load] \"%s\" not found", filename);
        return;
    }
    fclose(file);

    stl_load(filename, &manifold_manifold, &fancy_mesh, &history);
}
void conversation_load_file(char *filename) {
    if (poe_suffix_match(filename, ".dxf")) {
        conversation_load_dxf(filename);
    } else if (poe_suffix_match(filename, ".stl")) {
        conversation_load_stl(filename);
    } else {
        conversation_messagef("[load] \"%s\" fileype not supported; must be *.dxf or *.stl", filename);
    }
}

void conversation_save_file(char *filename) {
    if (poe_suffix_match(filename, ".stl")) {
        stl_save(&fancy_mesh, filename);
    } else {
        conversation_messagef("[save] \"%s\" filetype not supported; must be *.stl", filename);
    }
}

void conversation_set_drop_path(char *filename) {
    for (u32 i = 0; i < strlen(filename); ++i) {
        conversation_drop_path[i] = filename[i];
        if (filename[i] == '.') {
            while (
                    (i != 0) &&
                    (conversation_drop_path[i - 1] != '\\') &&
                    (conversation_drop_path[i - 1] != '/')
                  ) --i;
            conversation_drop_path[i] = '\0';
            break;
        }
    }
}

void conversation_update_M_3D_from_2D() {
    vec3 up = { 0.0f, 1.0f, 0.0f };
    real32 dot_product = dot(n_selected, up);
    vec3 y = (ARE_EQUAL(ABS(dot_product), 1.0f)) ? V3(0.0f,  0.0f, -1.0f * SGN(dot_product)) : up;
    vec3 x = normalized(cross(y, n_selected));
    vec3 z = cross(x, y);

    // FORNOW
    if (ARE_EQUAL(ABS(dot_product), 1.0f) && SGN(dot_product) < 0.0f) {
        y *= -1;
    }

    M_3D_from_2D = M4_xyzo(x, y, z, (r_n_selected) * n_selected) * M4_Translation(-origin_x, -origin_y);
}



// SPOOKY SPOOKY ///////////////////////////////////////

void drop_callback(GLFWwindow *, int count, const char **paths) {
    if (count > 0) {
        char *filename = (char *) paths[0];
        conversation_load_file(filename);
        conversation_set_drop_path(filename);
    }
} BEGIN_PRE_MAIN { glfwSetDropCallback(COW0._window_glfw_window, drop_callback); } END_PRE_MAIN;


// MAIN ////////////////////////////////////////////////

int main() {
    bool32 reset = true;
    while (cow_begin_frame()) {
        if (reset) {
            reset = false;
            hot_pane = HOT_PANE_NONE;

            click_mode = _CLICK_MODE_DEFAULT;
            click_modifier = _CLICK_MODIFIER_DEFAULT;

            selected_reset(); 

            conversation_load_dxf("splash.dxf");

            manifold_manifold = NULL;
            fancy_mesh = {};
            #if 0
            {
                fancy_mesh = {};
                fancy_mesh.num_vertices = 4;
                fancy_mesh.num_triangles = 4;
                fancy_mesh.vertex_positions = (real32 *) calloc(3 * fancy_mesh.num_vertices, sizeof(real32));
                fancy_mesh.triangle_indices = (u32 *) calloc(3 * fancy_mesh.num_triangles, sizeof(u32));
                float h = (1.0f + SQRT(3.0f)) / 2;
                {
                    u32 k = 0;
                    fancy_mesh.vertex_positions[k++] = 100.0f * COS(RAD(0.0));
                    fancy_mesh.vertex_positions[k++] = 100.0f * 0.0f;
                    fancy_mesh.vertex_positions[k++] = 100.0f * SIN(RAD(0.0));
                    fancy_mesh.vertex_positions[k++] = 100.0f * COS(RAD(120.0));
                    fancy_mesh.vertex_positions[k++] = 100.0f * 0.0f;
                    fancy_mesh.vertex_positions[k++] = 100.0f * SIN(RAD(120.0));
                    fancy_mesh.vertex_positions[k++] = 100.0f * COS(RAD(240.0));
                    fancy_mesh.vertex_positions[k++] = 100.0f * 0.0f;
                    fancy_mesh.vertex_positions[k++] = 100.0f * SIN(RAD(240.0));
                    fancy_mesh.vertex_positions[k++] = 100.0f * 0.0f;
                    fancy_mesh.vertex_positions[k++] = 100.0f * h;
                    fancy_mesh.vertex_positions[k++] = 100.0f * 0.0f;
                }
                {
                    u32 k = 0;
                    fancy_mesh.triangle_indices[k++] = 0;
                    fancy_mesh.triangle_indices[k++] = 1;
                    fancy_mesh.triangle_indices[k++] = 2;
                    fancy_mesh.triangle_indices[k++] = 1;
                    fancy_mesh.triangle_indices[k++] = 0;
                    fancy_mesh.triangle_indices[k++] = 3;
                    fancy_mesh.triangle_indices[k++] = 2;
                    fancy_mesh.triangle_indices[k++] = 1;
                    fancy_mesh.triangle_indices[k++] = 3;
                    fancy_mesh.triangle_indices[k++] = 0;
                    fancy_mesh.triangle_indices[k++] = 2;
                    fancy_mesh.triangle_indices[k++] = 3;
                }
            }
            #endif

            enter_mode = _ENTER_MODE_DEFAULT;
            console_param = 0.0f;
            console_param_2 = 0.0f;
            console_params_preview_flip_flag = false;
            console_buffer_reset();

            { // cameras
                real32 height = 128.0f;
                camera2D = { height, 0.0, 0.0f, -0.5f, -0.25f };
                conversation_zoom_camera2D_to_dxf_extents();
                camera3D = { 200.0f, CAMERA_3D_DEFAULT_ANGLE_OF_VIEW, RAD(33.0f), RAD(-33.0f), 0.0f, 70.0f, 0.5f, 0.25f };
            }

            show_grid = true;
            show_details = false;
            show_help = false;

            history_free(&history);

            conversation_messagef("type h for help // pre-alpha " __DATE__ " " __TIME__);
        }






        console_params_preview_update();

        bool32 dxf_anything_selected; {
            dxf_anything_selected = false;
            for (u32 i = 0; i < dxf.num_entities; ++i) {
                if (dxf_selection_mask[i]) {
                    dxf_anything_selected = true;
                    break;
                }
            }
        }
        bool32 stl_plane_selected; { // FORNOW
            stl_plane_selected = !IS_ZERO(squaredNorm(n_selected));
        }




        if (globals.key_any_key_pressed) { // keyboard input keyboard shortuts
            bool32 key_eaten_by_special__NOTE_dealt_with_up_top;
            {
                key_eaten_by_special__NOTE_dealt_with_up_top = false;
                if (click_modifier == CLICK_MODIFIER_QUALITY) {
                    for (u32 color = 0; color < 6; ++color) {
                        if (key_pressed['0' + color]) {
                            key_eaten_by_special__NOTE_dealt_with_up_top = true;
                            for (u32 i = 0; i < dxf.num_entities; ++i) {
                                if (dxf.entities[i].color == color) {
                                    bool32 value_to_write_to_selection_mask = (click_mode == CLICK_MODE_SELECT);
                                    dxf_selection_mask[i] = value_to_write_to_selection_mask;
                                }
                            }
                            click_modifier = CLICK_MODIFIER_NONE;
                            break;
                        }
                    }
                }
            }
            bool32 send_key_to_console;
            {
                send_key_to_console = false;
                send_key_to_console = (send_key_to_console || key_pressed[COW_KEY_BACKSPACE]);
                if (!key_pressed[COW_KEY_ENTER]) {
                    if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) {
                        send_key_to_console = (send_key_to_console || key_pressed['.']);
                        send_key_to_console = (send_key_to_console || key_pressed['-']);
                        for (u32 i = 0; i < 10; ++i) send_key_to_console = (send_key_to_console || key_pressed['0' + i]);
                        // note: double negative
                        if ((enter_mode != ENTER_MODE_OFFSET_PLANE_BY)) {
                            send_key_to_console = (send_key_to_console || key_pressed[' ']);
                        }
                    } else if ((enter_mode == ENTER_MODE_LOAD) || (enter_mode == ENTER_MODE_SAVE)) {
                        send_key_to_console = (send_key_to_console || key_pressed['.']);
                        send_key_to_console = (send_key_to_console || key_pressed[' ']);
                        send_key_to_console = (send_key_to_console || key_pressed['-']);
                        send_key_to_console = (send_key_to_console || key_pressed['/']);
                        send_key_to_console = (send_key_to_console || key_pressed['\\']);
                        for (u32 i = 0; i < 10; ++i) send_key_to_console = (send_key_to_console || key_pressed['0' + i]);
                        for (u32 i = 0; i < 26; ++i) send_key_to_console = (send_key_to_console || key_pressed['a' + i]);
                        for (u32 i = 0; i < 26; ++i) send_key_to_console = (send_key_to_console || key_pressed['A' + i]);
                    }
                } else {
                    send_key_to_console = true;
                    {
                        bool32 extrude = ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT));
                        bool32 revolve = ((enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT));
                        bool32 cut = ((enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_CUT));
                        if (enter_mode == ENTER_MODE_NONE) {
                            conversation_messagef("[enter] enter mode is none");
                            send_key_to_console = false;
                        } else if (extrude || revolve) {
                            if (!dxf_anything_selected) {
                                conversation_messagef("[enter] no dxf elements selected");
                                send_key_to_console = false;
                            } else if (!stl_plane_selected) { // FORNOW???
                                conversation_messagef("[enter] no plane selected");
                                send_key_to_console = false;
                            } else if (cut && (fancy_mesh.num_triangles == 0)) { // FORNOW
                                conversation_messagef("[enter] nothing to cut");
                                send_key_to_console = false;
                            } else if (extrude) {
                                if (IS_ZERO(console_param_preview) && IS_ZERO(console_param_2_preview)) {
                                    conversation_messagef("[enter] extrude height is zero");
                                    send_key_to_console = false;
                                }
                            } else {
                                ASSERT(revolve);
                                ;
                            }
                        } else if (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) {
                            ;
                        } else if (enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                            ;
                        }
                    }
                }
            }


            if (key_eaten_by_special__NOTE_dealt_with_up_top) {
                ;
            } else if (send_key_to_console) {
                if (!key_pressed[COW_KEY_ENTER] ) {
                    if (key_pressed[COW_KEY_BACKSPACE]) {
                        *--console_buffer_write_head = 0;
                    } else {
                        char c = (char) globals.key_last_key_pressed;
                        if (globals.key_shift_held && key_pressed['-']) c = '_';
                        *console_buffer_write_head++ = c;
                    }
                } else {
                    if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT) || (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) {
                        if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) {
                            if (console_buffer_write_head != console_buffer) {
                                console_param = console_param_preview;
                                console_param_2 = console_param_2_preview;
                                console_buffer_reset();
                            }
                        }
                        if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
                            CrossSectionEvenOdd cross_section = cross_section_create(&dxf, dxf_selection_mask);
                            // cross_section_debug_draw(&camera2D, &cross_section);
                            wrapper_manifold(
                                    &manifold_manifold,
                                    &fancy_mesh,
                                    cross_section.num_polygonal_loops,
                                    cross_section.num_vertices_in_polygonal_loops,
                                    cross_section.polygonal_loops,
                                    M_3D_from_2D,
                                    enter_mode,
                                    console_param,
                                    console_param_2,
                                    &history
                                    );
                            // reset state
                            memset(dxf_selection_mask, 0, dxf.num_entities * sizeof(bool32));
                            selected_reset();
                        } else if (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) {
                            origin_x = console_param;
                            origin_y = console_param_2;
                            conversation_update_M_3D_from_2D();
                        } else {
                            ASSERT(enter_mode == ENTER_MODE_OFFSET_PLANE_BY);
                            if (!IS_ZERO(console_param)) {
                                r_n_selected += console_param;
                                some_triangle_exists_that_matches_n_selected_and_r_n_selected = false; // FORNOW
                                conversation_update_M_3D_from_2D();
                            }
                        }
                        console_param = 0.0f;
                        console_param_2 = 0.0f;
                    } else {
                        ASSERT((enter_mode == ENTER_MODE_LOAD) || (enter_mode == ENTER_MODE_SAVE));
                        static char full_filename_including_path[512];
                        sprintf(full_filename_including_path, "%s%s", conversation_drop_path, console_buffer);
                        if (enter_mode == ENTER_MODE_LOAD) {
                            conversation_load_file(full_filename_including_path);
                        } else {
                            ASSERT(enter_mode == ENTER_MODE_SAVE);
                            conversation_save_file(full_filename_including_path);
                        }
                    }
                    console_buffer_reset();
                    enter_mode = _ENTER_MODE_DEFAULT;
                    // if (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) {
                    click_mode = _CLICK_MODE_DEFAULT;
                    click_modifier = _CLICK_MODIFIER_DEFAULT;
                    // }
                }
            } else if (key_pressed[COW_KEY_TAB]) {
                camera3D.angle_of_view = (IS_ZERO(camera3D.angle_of_view)) ? CAMERA_3D_DEFAULT_ANGLE_OF_VIEW : 0.0f;
            } else if (key_pressed[COW_KEY_ESCAPE]) {
                enter_mode = ENTER_MODE_NONE;
                click_mode = CLICK_MODE_NONE;
                click_modifier = CLICK_MODIFIER_NONE;
                console_buffer_reset();
                selected_reset(); // FORNOW
            } else if (key_pressed['U'] && globals.key_shift_held) {
                history_redo(&history, &manifold_manifold, &fancy_mesh);
            } else if (key_pressed['u']) {
                history_undo(&history, &manifold_manifold, &fancy_mesh);
            } else if (key_pressed['X'] && globals.key_shift_held) {
                conversation_zoom_camera2D_to_dxf_extents();
            } else if (key_pressed['n']) {
                if (stl_plane_selected) {
                    enter_mode = ENTER_MODE_OFFSET_PLANE_BY;
                    console_params_preview_flip_flag = false;
                    console_buffer_reset();
                } else {
                    conversation_messagef("[n] no plane selected");
                }
            } else if (key_pressed['m']) {
                click_mode = CLICK_MODE_MOVE_2D_ORIGIN_TO;
                click_modifier = CLICK_MODIFIER_NONE;
                enter_mode = ENTER_MODE_MOVE_ORIGIN_TO;
                console_params_preview_flip_flag = false;
                console_buffer_reset();
            } else if (key_pressed['S'] && globals.key_shift_held) {
                enter_mode = ENTER_MODE_SAVE;
            } else if (key_pressed['g']) {
                show_grid = !show_grid;
            } else if (key_pressed['h']) {
                show_help = !show_help;
            } else if (key_pressed['.'] && !send_key_to_console) {
                show_details = !show_details;
            } else if (key_pressed['s']) {
                click_mode = CLICK_MODE_SELECT;
                click_modifier = CLICK_MODIFIER_NONE;
            } else if (key_pressed['w']) {
                if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                    click_modifier = CLICK_MODIFIER_WINDOW;
                    window_select_click_count = 0;
                }
            } else if (key_pressed['d']) {
                click_mode = CLICK_MODE_DESELECT;
                click_modifier = CLICK_MODIFIER_NONE;
            } else if (key_pressed['c']) {
                if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                    click_modifier = CLICK_MODIFIER_CONNECTED;
                } else if (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) {
                    click_modifier = CLICK_MODIFIER_CENTER_OF;
                }
            } else if (key_pressed['q']) {
                if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                    click_modifier = CLICK_MODIFIER_QUALITY;
                }
            } else if (key_pressed['a']) {
                if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                    bool32 value_to_write_to_selection_mask = (click_mode == CLICK_MODE_SELECT);
                    for (u32 i = 0; i < dxf.num_entities; ++i) dxf_selection_mask[i] = value_to_write_to_selection_mask;
                }
            } else if (key_pressed['x'] || key_pressed['y'] || key_pressed['z']) {
                some_triangle_exists_that_matches_n_selected_and_r_n_selected = false;
                r_n_selected = 0.0f;
                if (key_pressed['x']) n_selected = { 1.0f, 0.0f, 0.0f };
                if (key_pressed['y']) n_selected = { 0.0f, 1.0f, 0.0f };
                if (key_pressed['z']) n_selected = { 0.0f, 0.0f, 1.0f };
                conversation_update_M_3D_from_2D();
            } else if (key_pressed['E'] && globals.key_shift_held) {
                enter_mode = ENTER_MODE_EXTRUDE_CUT;
                console_params_preview_flip_flag = true;
                console_buffer_reset();
            } else if (key_pressed['e']) {
                if (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) {
                    click_modifier = CLICK_MODIFIER_END_OF;
                } else {
                    enter_mode = ENTER_MODE_EXTRUDE_ADD;
                    console_params_preview_flip_flag = false;
                    console_buffer_reset();
                }
            } else if (key_pressed['R'] && globals.key_shift_held) {
                enter_mode = ENTER_MODE_REVOLVE_CUT;
            } else if (key_pressed['r']) {
                enter_mode = ENTER_MODE_REVOLVE_ADD;
            } else if (key_pressed['L'] && globals.key_shift_held) {
                enter_mode = ENTER_MODE_LOAD;
            } else if (key_pressed['f']) {
                console_params_preview_flip_flag = !console_params_preview_flip_flag;
                console_params_preview_update();
            } else {
                ;
            }
        }


        if (hot_pane == HOT_PANE_2D) {
            camera_move(&camera2D);
        } else if (hot_pane == HOT_PANE_3D) {
            camera_move(&camera3D);
        }
        mat4 PV_2D = camera_get_PV(&camera2D);
        real32 mouse_x, mouse_y; { _input_get_mouse_position_and_change_in_position_in_world_coordinates(PV_2D.data, &mouse_x, &mouse_y, NULL, NULL); }
        mat4 P_3D = camera_get_P(&camera3D);
        mat4 V_3D = camera_get_V(&camera3D);
        mat4 PV_3D = P_3D * V_3D;

        { // ui
            if (globals._input_owner == COW_INPUT_OWNER_NONE) {
                { // pick 2D pick 2d pick
                    if (hot_pane == HOT_PANE_2D) {
                        { // click dxf click dxf_click
                            if (!globals.mouse_left_held) {
                            } else if (click_mode == CLICK_MODE_NONE) {
                            } else if (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) {
                                if (click_modifier == CLICK_MODE_NONE) {
                                    origin_x = mouse_x;
                                    origin_y = mouse_y;
                                } else if (click_modifier == CLICK_MODIFIER_CENTER_OF) {
                                    real32 min_squared_distance = HUGE_VAL;
                                    for (DXFEntity *entity = dxf.entities; entity < &dxf.entities[dxf.num_entities]; ++entity) {
                                        if (entity->type == DXF_ENTITY_TYPE_LINE) {
                                            continue;
                                        } else {
                                            ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                                            DXFArc *arc = &entity->arc;
                                            real32 squared_distance = squared_distance_point_dxf_arc(mouse_x, mouse_y, arc);
                                            if (squared_distance < min_squared_distance) {
                                                min_squared_distance = squared_distance;
                                                origin_x = arc->center_x;
                                                origin_y = arc->center_y;
                                            }
                                        }
                                    }
                                } else if (click_modifier == CLICK_MODIFIER_END_OF) {
                                    real32 min_squared_distance = HUGE_VAL;
                                    for (DXFEntity *entity = dxf.entities; entity < &dxf.entities[dxf.num_entities]; ++entity) {
                                        real32 x[2], y[2];
                                        entity_get_start_and_end_points(entity, &x[0], &y[0], &x[1], &y[1]);
                                        for (u32 d = 0; d < 2; ++d) {
                                            real32 squared_distance = squared_distance_point_point(mouse_x, mouse_y, x[d], y[d]);
                                            if (squared_distance < min_squared_distance) {
                                                min_squared_distance = squared_distance;
                                                origin_x = x[d];
                                                origin_y = y[d];
                                            }
                                        }
                                    }
                                } else if (click_modifier == CLICK_MODIFIER_MIDDLE_OF) {
                                    // TODO
                                }
                                click_mode = CLICK_MODE_NONE;
                                click_modifier = CLICK_MODIFIER_NONE;
                                enter_mode = ENTER_MODE_NONE;
                                conversation_update_M_3D_from_2D();
                            } else {
                                bool32 value_to_write_to_selection_mask = (click_mode == CLICK_MODE_SELECT);
                                bool32 modifier_connected = (click_modifier == CLICK_MODIFIER_CONNECTED);
                                if (click_modifier != CLICK_MODIFIER_WINDOW) {
                                    int hot_entity_index = -1;
                                    double hot_squared_distance = HUGE_VAL;
                                    for (u32 i = 0; i < dxf.num_entities; ++i) {
                                        DXFEntity *entity = &dxf.entities[i];
                                        double squared_distance = squared_distance_point_dxf_entity(mouse_x, mouse_y, entity);
                                        squared_distance /= (camera2D.screen_height_World * camera2D.screen_height_World / 4); // NDC
                                        if (squared_distance < hot_squared_distance) {
                                            hot_squared_distance = squared_distance;
                                            hot_entity_index = i;
                                        }
                                    }
                                    if (hot_entity_index != -1) {
                                        if (globals.mouse_left_held) {
                                            if (!modifier_connected) {
                                                dxf_selection_mask[hot_entity_index] = value_to_write_to_selection_mask;
                                            } else {
                                                u32 loop_index = pick.loop_index_from_entity_index[hot_entity_index];
                                                DXFEntityIndexAndFlipFlag *loop = pick.loops[loop_index];
                                                u32 num_entities = pick.num_entities_in_loops[loop_index];
                                                for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities; ++entity_index_and_flip_flag) {
                                                    dxf_selection_mask[entity_index_and_flip_flag->entity_index] = value_to_write_to_selection_mask;
                                                }
                                            }
                                        }
                                    }
                                } else if (globals.mouse_left_pressed) {
                                    if (globals.mouse_left_pressed) {
                                        if (window_select_click_count == 0) {
                                            window_select_x = mouse_x;
                                            window_select_y = mouse_y;

                                            ++window_select_click_count;
                                        } else {
                                            BoundingBox window = {
                                                MIN(window_select_x, mouse_x),
                                                MIN(window_select_y, mouse_y),
                                                MAX(window_select_x, mouse_x),
                                                MAX(window_select_y, mouse_y)
                                            };
                                            for (u32 i = 0; i < dxf.num_entities; ++i) {
                                                if (bounding_box_contains(window, bbox[i])) {
                                                    dxf_selection_mask[i] = value_to_write_to_selection_mask;
                                                }
                                            }

                                            window_select_click_count = 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                { // pick 3D pick 3d pick
                    if (hot_pane == HOT_PANE_3D) {
                        if (globals.mouse_left_pressed) {
                            vec3 o = transformPoint(inverse(PV_3D), V3(globals.mouse_position_NDC, -1.0f));
                            vec3 dir = normalized(transformPoint(inverse(PV_3D), V3(globals.mouse_position_NDC,  1.0f)) - o);

                            int32 index_of_first_triangle_hit_by_ray = -1;
                            {
                                real32 min_distance = HUGE_VAL;
                                for (u32 i = 0; i < fancy_mesh.num_triangles; ++i) {
                                    vec3 p[3]; {
                                        for (u32 j = 0; j < 3; ++j) p[j] = get(fancy_mesh.vertex_positions, fancy_mesh.triangle_indices[3 * i + j]);
                                    }
                                    RayTriangleIntersectionResult result = ray_triangle_intersection(o, dir, p[0], p[1], p[2]);
                                    if (result.hit) {
                                        if (result.distance < min_distance) {
                                            min_distance = result.distance;
                                            index_of_first_triangle_hit_by_ray = i; // FORNOW
                                        }
                                    }
                                }
                            }

                            if (index_of_first_triangle_hit_by_ray != -1) {
                                some_triangle_exists_that_matches_n_selected_and_r_n_selected = true;
                                { // FORNOW (gross) calculateion of n_selected, r_n_selected
                                    n_selected = get(fancy_mesh.triangle_normals, index_of_first_triangle_hit_by_ray);
                                    {
                                        vec3 p_selected[3]; {
                                            for (u32 j = 0; j < 3; ++j) p_selected[j] = get(fancy_mesh.vertex_positions, fancy_mesh.triangle_indices[3 * index_of_first_triangle_hit_by_ray + j]);
                                        }
                                        r_n_selected = dot(n_selected, p_selected[0]);
                                    }
                                }
                                conversation_update_M_3D_from_2D();
                            }
                        }
                    }
                }
            }
        }


        { // draw
            u32 window_width, window_height; {
                real32 _window_width, _window_height; // FORNOW
                _window_get_size(&_window_width, &_window_height);
                window_width = (u32) _window_width;
                window_height = (u32) _window_height;
            }

            { // panes
                { // draw
                  // glDisable(GL_DEPTH_TEST);
                  // eso_begin(globals.Identity, SOUP_QUADS);
                  // eso_color(0.3f, 0.3f, 0.3f);
                  // eso_vertex(0.0f,  1.0f);
                  // eso_vertex(0.0f, -1.0f);
                  // eso_vertex(1.0f, -1.0f);
                  // eso_vertex(1.0f,  1.0f);
                  // eso_end();
                  // glEnable(GL_DEPTH_TEST);

                    eso_begin(globals.Identity, SOUP_LINES, 5.0f, true);
                    eso_color(136 / 255.0f, 136 / 255.0f, 136 / 255.0f);
                    eso_vertex(0.0f,  1.0f);
                    eso_vertex(0.0f, -1.0f);
                    eso_end();
                }

                if ((!globals.mouse_left_held && !globals.mouse_right_held) || globals.mouse_left_pressed || globals.mouse_right_pressed) {
                    hot_pane = (globals.mouse_position_NDC.x <= 0.0f) ? HOT_PANE_2D : HOT_PANE_3D;
                    if ((click_modifier == CLICK_MODIFIER_WINDOW) && (window_select_click_count == 1)) hot_pane = HOT_PANE_2D;// FORNOW
                }
            }

            { // draw 2D draw 2d draw
                glEnable(GL_SCISSOR_TEST);
                glScissor(0, 0, window_width / 2, window_height);
                {
                    if (show_grid) { // grid 2D grid 2d grid
                        eso_begin(PV_2D, SOUP_LINES, 2.0f);
                        eso_color(80.0f / 255, 80.0f / 255, 80.0f / 255);
                        for (u32 i = 0; i <= u32(GRID_SIDE_LENGTH / GRID_SPACING); ++i) {
                            real32 tmp = i * GRID_SPACING;
                            eso_vertex(tmp, 0.0f);
                            eso_vertex(tmp, GRID_SIDE_LENGTH);
                            eso_vertex(0.0f, tmp);
                            eso_vertex(GRID_SIDE_LENGTH, tmp);
                        }
                        eso_end();
                        eso_begin(PV_2D, SOUP_LINE_LOOP, 2.0f);
                        eso_vertex(0.0f, 0.0f);
                        eso_vertex(0.0f, GRID_SIDE_LENGTH);
                        eso_vertex(GRID_SIDE_LENGTH, GRID_SIDE_LENGTH);
                        eso_vertex(GRID_SIDE_LENGTH, 0.0f);
                        eso_end();
                    }
                    { // entities
                        eso_begin(PV_2D, SOUP_LINES, 3.0f);
                        for (u32 i = 0; i < dxf.num_entities; ++i) {
                            DXFEntity *entity = &dxf.entities[i];
                            int32 color = (dxf_selection_mask[i]) ? DXF_COLOR_SELECTION : DXF_COLOR_DONT_OVERRIDE;
                            eso_dxf_entity__SOUP_LINES(entity, color);
                        }
                        eso_end();
                    }
                    { // dots
                        if (show_details) {
                            eso_begin(camera_get_PV(&camera2D), SOUP_POINTS, 4.0);
                            eso_color(monokai.white);
                            for (DXFEntity *entity = dxf.entities; entity < &dxf.entities[dxf.num_entities]; ++entity) {
                                real32 start_x, start_y, end_x, end_y;
                                entity_get_start_and_end_points(entity, &start_x, &start_y, &end_x, &end_y);
                                eso_vertex(start_x, start_y);
                                eso_vertex(end_x, end_y);
                            }
                            eso_end();
                        }
                    }
                    { // axes 2D axes 2d axes axis 2D axis 2d axes crosshairs cross hairs origin 2d origin 2D origin
                        real32 r = camera2D.screen_height_World / 120.0f;
                            mat4 M = M4_Translation(origin_x, origin_y);
                            vec3 color = V3(0.8f, 0.8f, 1.0f);
                            if (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) {
                                M = M4_Translation(console_param_preview, console_param_2_preview);
                                color = V3(0.0f, 1.0f, 1.0f);
                            }
                            eso_begin(PV_2D * M, SOUP_LINES, 3.0f);
                            eso_color(color);
                            eso_vertex(-r*.7f, 0.0f);
                            eso_vertex( r, 0.0f);
                            eso_vertex(0.0f, -r);
                            eso_vertex(0.0f,  r*.7f);
                            eso_end();
                    }
                    if (click_modifier == CLICK_MODIFIER_WINDOW) { // select window
                        if (window_select_click_count == 1) {
                            eso_begin(PV_2D, SOUP_LINE_LOOP, 2.0f);
                            eso_color(0.0f, 1.0f, 1.0f);
                            real32 x0 = window_select_x;
                            real32 y0 = window_select_y;
                            real32 x1 = mouse_x;
                            real32 y1 = mouse_y;
                            eso_vertex(x0, y0);
                            eso_vertex(x1, y0);
                            eso_vertex(x1, y1);
                            eso_vertex(x0, y1);
                            eso_end();
                        }
                    }
                }
                glDisable(GL_SCISSOR_TEST);
            }

            { // 3D draw 3D 3d draw 3d
                glEnable(GL_SCISSOR_TEST);
                glScissor(window_width / 2, 0, window_width / 2, window_height);

                { // selection 2d selection 2D selection (FORNOW: ew)
                    u32 color = ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_ADD)) ? DXF_COLOR_TRAVERSE : ((enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) ? DXF_COLOR_QUALITY_1 : ((enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) ? DXF_COLOR_WATER_ONLY : DXF_COLOR_SELECTION;

                    u32 NUM_TUBE_STACKS_INCLUSIVE;
                    mat4 M;
                    mat4 M_incr;
                    if (true) {
                        if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                            // NOTE: some repetition with wrapper
                            real32 a = MIN(0.0f, MIN(console_param_preview, console_param_2_preview));
                            real32 b = MAX(0.0f, MAX(console_param_preview, console_param_2_preview));
                            real32 length = b - a;
                            NUM_TUBE_STACKS_INCLUSIVE = MIN(64, u32(roundf(length / 2.5f)) + 2);
                            M = M_3D_from_2D * M4_Translation(0.0f, 0.0f, a + Z_FIGHT_EPS);
                            M_incr = M4_Translation(0.0f, 0.0f, (b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                        } else if ((enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
                            NUM_TUBE_STACKS_INCLUSIVE = NUM_SEGMENTS_PER_CIRCLE;
                            M = M_3D_from_2D;
                            real32 a = 0.0f;
                            real32 b = TAU;
                            M_incr = M4_RotationAboutYAxis((b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                        } else if (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) {
                            // FORNOW
                            NUM_TUBE_STACKS_INCLUSIVE = 1;
                            M = M_3D_from_2D * M4_Translation((origin_x - console_param_preview), (origin_y - console_param_2_preview));
                            M_incr = M4_Identity();
                        } else if (enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                            NUM_TUBE_STACKS_INCLUSIVE = 1;
                            M = M_3D_from_2D * M4_Translation(0.0f, 0.0f, console_param_preview + Z_FIGHT_EPS);
                            M_incr = M4_Identity();
                        } else {
                            NUM_TUBE_STACKS_INCLUSIVE = 1;
                            M = M_3D_from_2D * M4_Translation(0.0f, 0.0f, Z_FIGHT_EPS);
                            M_incr = M4_Identity();
                        }
                        for (u32 tube_stack_index = 0; tube_stack_index < NUM_TUBE_STACKS_INCLUSIVE; ++tube_stack_index) {
                            {
                                eso_begin(PV_3D * M, SOUP_LINES, 5.0f);
                                for (u32 i = 0; i < dxf.num_entities; ++i) {
                                    DXFEntity *entity = &dxf.entities[i];
                                    if (dxf_selection_mask[i]) {
                                        eso_dxf_entity__SOUP_LINES(entity, color);
                                    }
                                }
                                eso_end();
                            }
                            M *= M_incr;
                        }
                    }
                }


                { // arrow
                    if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
                        if (dxf_anything_selected) {
                            real32 H[2] = { console_param_preview, console_param_2_preview };
                            bool32 toggle[2] = { console_params_preview_flip_flag, !console_params_preview_flip_flag };
                            mat4 R2 = M4_Identity();
                            if ((enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
                                H[0] = 50.0f;
                                H[1] = 0.0f;
                                R2 = M4_RotationAboutXAxis(RAD(-90.0f));
                            }
                            mat4 R = M4_RotationAboutXAxis(RAD(90.0f));
                            vec3 color = ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_ADD)) ? V3(83.0f / 255, 255.0f / 255, 83.0f / 255.0f) : V3(1.0f, 0.0f, 0.0f);
                            for (u32 i = 0; i < 2; ++i) {
                                if (!IS_ZERO(H[i])) {
                                    real32 total_height = ABS(H[i]);
                                    real32 cap_height = (total_height > 10.0f) ? 5.0f : (0.5f * total_height);
                                    real32 shaft_height = total_height - cap_height;
                                    real32 s = 1.5f;
                                    mat4 N = M4_Translation(0.0, 0.0, -Z_FIGHT_EPS);
                                    if (toggle[i]) N = M4_Scaling(1.0f, 1.0f, -1.0f) * N;
                                    mat4 M_cyl  = M_3D_from_2D * R2 * N * M4_Scaling(s * 1.0f, s * 1.0f, shaft_height) * R;
                                    mat4 M_cone = M_3D_from_2D * R2 * N * M4_Translation(0.0f, 0.0f, shaft_height) * M4_Scaling(s * 2.0f, s * 2.0f, cap_height) * R;
                                    library.meshes.cylinder.draw(P_3D, V_3D, M_cyl, color);
                                    library.meshes.cone.draw(P_3D, V_3D, M_cone, color);
                                }
                            }
                        }
                    }
                }

                if (stl_plane_selected) { // axes 3D axes 3d axes axis 3D axis 3d axis
                    real32 r = camera3D.ortho_screen_height_World / 120.0f;
                    eso_begin(PV_3D * M_3D_from_2D * M4_Translation(origin_x, origin_y, Z_FIGHT_EPS), SOUP_LINES, 4.0f);
                    eso_color(0.8f, 0.8f, 1.0f);
                    eso_vertex(-r*0.6f, 0.0f);
                    eso_vertex(r, 0.0f);
                    eso_vertex(0.0f, -r);
                    eso_vertex(0.0f, r*0.6f);
                    eso_end();
                }


                { // fancy_mesh; NOTE: includes transparency
                    if (fancy_mesh.cosmetic_edges) {
                        eso_begin(PV_3D, SOUP_LINES, 3.0f); 
                        eso_color(monokai.black);
                        // 3 * num_triangles * 2 / 2
                        for (u32 k = 0; k < 2 * fancy_mesh.num_cosmetic_edges; ++k) eso_vertex(fancy_mesh.vertex_positions, fancy_mesh.cosmetic_edges[k]);
                        eso_end();
                    }
                    for (u32 pass = 0; pass <= 1; ++pass) {
                        eso_begin(PV_3D, (!show_details) ? SOUP_TRIANGLES : SOUP_OUTLINED_TRIANGLES);

                        mat3 inv_transpose_V_3D = inverse(transpose(M3(V_3D(0, 0), V_3D(0, 1), V_3D(0, 2), V_3D(1, 0), V_3D(1, 1), V_3D(1, 2), V_3D(2, 0), V_3D(2, 1), V_3D(2, 2))));

                        for (u32 i = 0; i < fancy_mesh.num_triangles; ++i) {
                            #if 1
                            vec3 n = get(fancy_mesh.triangle_normals, i);
                            vec3 p[3];
                            real32 x_n;
                            {
                                for (u32 j = 0; j < 3; ++j) p[j] = get(fancy_mesh.vertex_positions, fancy_mesh.triangle_indices[3 * i + j]);
                                x_n = dot(n, p[0]);
                            }
                            vec3 color; 
                            real32 alpha;
                            {
                                vec3 n_camera = inv_transpose_V_3D * n;
                                vec3 color_n = V3(0.5f + 0.5f * n_camera.x, 0.5f + 0.5f * n_camera.y, 1.0f);
                                if (some_triangle_exists_that_matches_n_selected_and_r_n_selected && (dot(n, n_selected) > 0.999f) && (ABS(x_n - r_n_selected) < 0.001f)) {
                                    if (pass == 0) continue;
                                    color = V3(0.85f, 0.87f, 0.30f);
                                    alpha = ((enter_mode == ENTER_MODE_EXTRUDE_ADD || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) && ((console_params_preview_flip_flag) || (console_param_2_preview != 0.0f))) ? 0.7f : 1.0f;
                                } else {
                                    if (pass == 1) continue;
                                    color = color_n;
                                    alpha = 1.0f;
                                }
                            }
                            eso_color(color, alpha);
                            eso_vertex(p[0]);
                            eso_vertex(p[1]);
                            eso_vertex(p[2]);
                            #else
                            eso_color(monokai.green, 1.0f);
                            vec3 p[3];
                            {
                                for (u32 j = 0; j < 3; ++j) p[j] = get(fancy_mesh.vertex_positions, fancy_mesh.triangle_indices[3 * i + j]);
                            }
                            eso_vertex(p[0]);
                            eso_vertex(p[1]);
                            eso_vertex(p[2]);
                            #endif

                        }
                        eso_end();
                    }
                }

                if (show_grid) { // grid 3D grid 3d grid
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_FRONT);
                    real32 L = GRID_SIDE_LENGTH;
                    grid_box.draw(P_3D, V_3D, M4_Translation(0.0f, L / 2 - 2 * Z_FIGHT_EPS, 0.0f) * M4_Scaling(L / 2), {}, "procedural grid");
                    glDisable(GL_CULL_FACE);
                }

                { // floating sketch plane; NOTE: transparent
                    real32 r = 30.0f;
                    bool draw = (!some_triangle_exists_that_matches_n_selected_and_r_n_selected);
                    mat4 PVM = PV_3D * M_3D_from_2D;
                    vec3 color = monokai.yellow;
                    real32 sign = -1.0f;
                    if (enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                        PVM *= M4_Translation(0.0f, 0.0f, console_param_preview);
                        color = { 0.0f, 1.0f, 1.0f };
                        sign = 1.0f;
                        draw = true;
                    }
                    if (draw) {
                        eso_begin(PVM, SOUP_OUTLINED_QUADS);
                        eso_color(color, 0.3f);
                        eso_vertex( r,  r, sign * Z_FIGHT_EPS);
                        eso_vertex( r, -r, sign * Z_FIGHT_EPS);
                        eso_vertex(-r, -r, sign * Z_FIGHT_EPS);
                        eso_vertex(-r,  r, sign * Z_FIGHT_EPS);
                        eso_end();
                    }
                }

                glDisable(GL_SCISSOR_TEST);
            }

            { // gui
                gui_printf("[Click] %s %s",
                        (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) ? "MOVE_2D_ORIGIN_TO" :
                        (click_mode == CLICK_MODE_SELECT) ? "SELECT" :
                        (click_mode == CLICK_MODE_DESELECT) ? "DESELECT" :
                        "NONE",
                        (click_modifier == CLICK_MODE_NONE) ? "" :
                        (click_modifier == CLICK_MODIFIER_CONNECTED) ? "CONNECTED" :
                        (click_modifier == CLICK_MODIFIER_WINDOW) ? "WINDOW" :
                        (click_modifier == CLICK_MODIFIER_CENTER_OF) ? "CENTER_OF" :
                        (click_modifier == CLICK_MODIFIER_END_OF) ? "END_OF" :
                        (click_modifier == CLICK_MODIFIER_MIDDLE_OF) ? "MIDDLE_OF" :
                        "");

                char enter_message[256] = {};
                if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO)) {
                    real32 p, p2;
                    char glyph, glyph2;
                    if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                        p      =  console_param_preview;
                        p2     = -console_param_2_preview;
                        if (console_params_preview_flip_flag) { // ??
                            p *= -1;
                            p2 *= -1;
                        }
                        if (IS_ZERO(p)) p = 0.0f; // FORNOW makes minus sign go away in hud (not a big deal)
                        if (IS_ZERO(p2)) p2 = 0.0f; // FORNOW makes minus sign go away in hud (not a big deal)
                        glyph  = (!console_params_preview_flip_flag) ? '^' : 'v';
                        glyph2 = (!console_params_preview_flip_flag) ? 'v' : '^';
                    } else {
                        ASSERT(enter_mode == ENTER_MODE_MOVE_ORIGIN_TO);
                        p      = console_param_preview;
                        p2     = console_param_2_preview;
                        glyph  = 'x';
                        glyph2 = 'y';
                    }
                    sprintf(enter_message, "%c:%gmm %c:%gmm", glyph, p, glyph2, p2);
                    if (((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) && IS_ZERO(console_param_2_preview)) sprintf(enter_message, "%c:%gmm", glyph, p);
                } else if ((enter_mode == ENTER_MODE_LOAD) || (enter_mode == ENTER_MODE_SAVE)) {
                    sprintf(enter_message, "%s%s", conversation_drop_path, console_buffer);
                }

                gui_printf("[Enter] %s %s",
                        (enter_mode == ENTER_MODE_EXTRUDE_ADD) ? "EXTRUDE_ADD" :
                        (enter_mode == ENTER_MODE_EXTRUDE_CUT) ? "EXTRUDE_CUT" :
                        (enter_mode == ENTER_MODE_REVOLVE_ADD) ? "REVOLVE_ADD" :
                        (enter_mode == ENTER_MODE_REVOLVE_CUT) ? "REVOLVE_CUT" :
                        (enter_mode == ENTER_MODE_LOAD) ? "LOAD" :
                        (enter_mode == ENTER_MODE_SAVE) ? "SAVE" :
                        (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) ? "MOVE_2D_ORIGIN_TO" :
                        (enter_mode == ENTER_MODE_OFFSET_PLANE_BY) ? "OFFSET_PLANE_TO" :
                        "NONE",
                        enter_message);

                if ((enter_mode == ENTER_MODE_NONE) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
                    gui_printf("> %s", console_buffer);
                } else {
                    gui_printf("> %s", console_buffer);
                }

                {
                    if (conversation_message_cooldown > 0) {
                        --conversation_message_cooldown;
                    } else {
                        conversation_message_buffer[0] = '\0';
                    }

                    gui_printf("< %s", conversation_message_buffer);
                }

                if (show_details) {
                    gui_printf("%d dxf elements", dxf.num_entities);
                    gui_printf("%d stl triangles", fancy_mesh.num_triangles);
                }
                if (show_help) {
                    { // overlay
                        eso_begin(M4_Identity(), SOUP_QUADS, 0.0f, true);
                        eso_color(0.0f, 0.0f, 0.0f, 0.7f);
                        eso_vertex(-1.0f, -1.0f);
                        eso_vertex(-1.0f,  1.0f);
                        eso_vertex( 1.0f,  1.0f);
                        eso_vertex( 1.0f, -1.0f);
                        eso_end();
                    }
                    gui_printf("(h)elp-show/hide");
                    gui_printf("(Escape)-from-current-enter_and_click_modes");
                    gui_printf("(s)elect (d)eselect (c)onnected + (a)ll (q)uality + (0-5)");
                    gui_printf("(y)-plane (x)-plane");
                    gui_printf("(e)trude-add (E)xtrude-cut + (0123456789. ) (f)lip-direction");
                    gui_printf("(r)evolve-add (R)evolve-cut");
                    gui_printf("(u)ndo (U)-redo");
                    gui_printf("(L)oad (S)ave");
                    gui_printf("(g)rid (.)-show-details");
                    gui_printf("zoom-to-e(X)tents");
                    gui_printf("(Tab)-orthographic-perspective-view");
                    gui_printf("(m)ove-origin + (c)enter-of (e)nd-of (0123456789.- ) ");
                    gui_printf("");
                    gui_printf("you can drag and drop dxf's into Conversation");
                }
            }
        }
    }
}
