// TODO: make Mesh vec3's and ivec3's
// TODO: take entire transform (same used for draw) for wrapper_manifold--strip out incremental nature into function

// TODO: void eso_bounding_box__SOUP_QUADS(BoundingBox bounding_box)

////////////////////////////////////////
// Config-Tweaks ///////////////////////
////////////////////////////////////////

real32 Z_FIGHT_EPS = 0.05f;
real32 TOLERANCE_DEFAULT = 5e-4f;
uint32 NUM_SEGMENTS_PER_CIRCLE = 64;
real32 GRID_SIDE_LENGTH = 256.0f;
real32 GRID_SPACING = 10.0f;
real32 CAMERA_3D_DEFAULT_ANGLE_OF_VIEW = RAD(60.0f);

////////////////////////////////////////
// colors //////////////////////////////
////////////////////////////////////////

struct {
    vec3 cyan = { 0.0f, 1.0f, 1.0f };
} omax;

////////////////////////////////////////
// #defines ////////////////////////////
////////////////////////////////////////

#define ENTER_MODE_NONE               0
#define ENTER_MODE_EXTRUDE_ADD        1
#define ENTER_MODE_EXTRUDE_CUT        2
#define ENTER_MODE_REVOLVE_ADD        3
#define ENTER_MODE_REVOLVE_CUT        4
#define ENTER_MODE_OPEN               5
#define ENTER_MODE_SAVE               6
#define ENTER_MODE_SET_ORIGIN         7
#define ENTER_MODE_OFFSET_PLANE_BY    8

#define CLICK_MODE_NONE              0
#define CLICK_MODE_SELECT            1
#define CLICK_MODE_DESELECT          2
#define CLICK_MODE_SET_ORIGIN        3
#define CLICK_MODE_SET_AXIS          4
#define CLICK_MODE_MEASURE           5
#define CLICK_MODE_CREATE_LINE       6
#define CLICK_MODE_CREATE_BOX        7
#define CLICK_MODE_CREATE_CIRCLE     8
#define CLICK_MODE_CREATE_FILLET     9
#define CLICK_MODE_MOVE_DXF_ENTITIES 10
#define CLICK_MODE_X_MIRROR          11
#define CLICK_MODE_Y_MIRROR          12

#define CLICK_MODIFIER_NONE                  0
#define CLICK_MODIFIER_CONNECTED             1
#define CLICK_MODIFIER_QUALITY               2
#define CLICK_MODIFIER_WINDOW                3
#define CLICK_MODIFIER_SNAP_TO_CENTER_OF     4
#define CLICK_MODIFIER_SNAP_TO_END_OF        5
#define CLICK_MODIFIER_SNAP_TO_MIDDLE_OF     6
#define CLICK_MODIFIER_SNAP_PERPENDICULAR_TO 7
#define CLICK_MODIFIER_EXACT_X_Y_COORDINATES 8 // TODO

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

#define DXF_ENTITY_TYPE_LINE 0
#define DXF_ENTITY_TYPE_ARC  1

#define HOT_PANE_NONE 0
#define HOT_PANE_2D   1
#define HOT_PANE_3D   2

#define USER_EVENT_TYPE_NONE            0
#define USER_EVENT_TYPE_KEY_PRESS       1
#define USER_EVENT_TYPE_MOUSE_2D_PRESS  2
#define USER_EVENT_TYPE_MOUSE_3D_PRESS  3
#define USER_EVENT_TYPE_GUI_MOUSE_PRESS 4

#define CELL_TYPE_NONE    0
#define CELL_TYPE_REAL32  1
#define CELL_TYPE_CSTRING 2

////////////////////////////////////////
// structs /////////////////////////////
////////////////////////////////////////

struct DXFLine {
    vec2 start;
    vec2 end;
    real32 _;
};

struct DXFArc {
    vec2 center;
    real32 radius;
    real32 start_angle_in_degrees;
    real32 end_angle_in_degrees;
};

struct DXFEntity {
    uint32 type;
    uint32 color;
    // NOTE: naive "fat struct"
    union {
        DXFLine line;
        DXFArc arc;
    };
    // FORNOW: this goes last
    bool32 is_selected;
};

struct BoundingBox {
    vec2 min;
    vec2 max;
};

struct Mesh {
    uint32 num_vertices;
    uint32 num_triangles;
    real32 *vertex_positions;
    uint32 *triangle_indices;
    real32 *triangle_normals;

    uint32 num_cosmetic_edges;
    uint32 *cosmetic_edges;

    vec3 min;
    vec3 max;
};

struct UserEvent {
    uint32 type;

    // union {
    // struct {
    uint32 key;
    bool32 super;
    bool32 shift;
    // };
    // struct {
    vec2 mouse;
    bool32 mouse_held;
    // };
    // struct {
    vec3 o;
    vec3 dir;
    // };
    // }; 

    bool32 record_me;
    bool32 checkpoint_me;
    bool32 snapshot_me;
};

struct ScreenState {
    Camera2D camera_2D;
    Camera3D camera_3D;

    bool32   hide_grid;
    bool32   hide_gui;
    bool32   show_details;
    bool32   show_help;
    bool32   show_event_stack;

    uint32   hot_pane;

    char drop_path[256];

    real32 popup_blinker_time;
    real32 successful_feature_time;
    real32 plane_selection_time;
    real32 going_inside_time;
    bool32 going_inside;

    bool32 DONT_DRAW_ANY_MORE_POPUPS_THIS_FRAME;
};

struct PopupState {
    #define POPUP_CELL_LENGTH 256
    real32 extrude_add_out_length;
    real32 extrude_add_in_length;
    real32 extrude_cut_in_length;
    real32 extrude_cut_out_length;
    real32 circle_diameter;
    real32 circle_radius;
    real32 circle_circumference;
    real32 fillet_radius;
    real32 box_width;
    real32 box_height;
    real32 x_coordinate;
    real32 y_coordinate;
    real32 plane_offset_distance;
    real32 line_length;
    real32 line_angle;
    real32 line_run;
    real32 line_rise;
    char filename[POPUP_CELL_LENGTH];

    #define POPUP_MAX_NUM_CELLS 4
    // char cells[POPUP_MAX_NUM_CELLS][POPUP_CELL_LENGTH];
    char active_cell_buffer[POPUP_CELL_LENGTH];

    uint32 index_of_active_cell;
    uint32 cursor;
    uint32 selection_cursor;
    // uint32 selection_left;
    // uint32 selection_right;

    uint32 _type_of_active_cell;
    void *_active_popup_unique_ID__FORNOW_name0;

    uint32 num_cells;
    BoundingBox field_boxes[POPUP_MAX_NUM_CELLS];

    // TODO: checkboxes
};



struct WorldState {
    Mesh mesh;

    struct {
        List<DXFEntity> entities;
        vec2            origin;
        vec2            axis_base_point;
        real32          axis_angle_from_y;
    } dxf;

    struct {
        bool32 is_active;
        vec3 normal;
        real32 signed_distance_to_world_origin;
    } feature_plane;

    struct {
        uint32 click_mode;
        uint32 click_modifier;
        uint32 enter_mode;
    } modes;

    struct {
        bool32 awaiting_second_click;
        vec2 first_click;
    } two_click_command;

    PopupState popup;
};



////////////////////////////////////////
// Data-Oriented Snail /////////////////
////////////////////////////////////////

vec3 get(real32 *x, uint32 i) {
    vec3 result;
    for (uint32 d = 0; d < 3; ++d) result.data[d] = x[3 * i + d];
    return result;
}
void set(real32 *x, uint32 i, vec3 v) {
    for (uint32 d = 0; d < 3; ++d) x[3 * i + d] = v.data[d];
}
void eso_vertex(real32 *p_j) {
    eso_vertex(p_j[0], p_j[1], p_j[2]);
}
void eso_vertex(real32 *p, uint32 j) {
    eso_vertex(p[3 * j + 0], p[3 * j + 1], p[3 * j + 2]);
}

real32 cross(real32 a_x, real32 a_y, real32 b_x, real32 b_y) {
    return a_x * b_y - a_y * b_x;
}

////////////////////////////////////////
// Cow Additions ///////////////////////
////////////////////////////////////////

bool32 ANGLE_IS_BETWEEN_CCW(real32 p, real32 a, real32 b) {
    p -= a;
    b -= a;
    // vec2 A = { 1.0f, 0.0f };
    // vec2 B = { COS(b), SIN(b) };
    // vec2 P = { COS(p), SIN(p) };
    // vec2 E = B - A;
    // vec2 F = P - A;
    // return (cross(E, F) > 0.0f);
    real32 E_x = COS(b) - 1.0f;
    real32 E_y = SIN(b);
    real32 F_x = COS(p) - 1.0f;
    real32 F_y = SIN(p);
    return (cross(E_x, E_y, F_x, F_y) > 0.0f);
}

#define warn_once(warning) do_once { printf(warning); printf("\n"); };

////////////////////////////////////////
// Squared-Distance (TODO: move non-dxf_entities parts all up here);
////////////////////////////////////////

real32 squared_distance_point_point(real32 x_A, real32 y_A, real32 x_B, real32 y_B) {
    real32 dx = (x_A - x_B);
    real32 dy = (y_A - y_B);
    return (dx * dx) + (dy * dy);
};

////////////////////////////////////////
// BoundingBox /////////////////////////
////////////////////////////////////////

void pprint(BoundingBox bounding_box) {
    printf("(%f, %f) <-> (%f, %f)\n", bounding_box.min[0], bounding_box.min[1], bounding_box.max[0], bounding_box.max[1]);
}

void bounding_box_center(BoundingBox bounding_box, real32 *x, real32 *y) {
    *x = (bounding_box.min[0] + bounding_box.max[0]) / 2;
    *y = (bounding_box.min[1] + bounding_box.max[1]) / 2;
}

bool32 bounding_box_contains(BoundingBox outer, BoundingBox inner) {
    for (uint32 d = 0; d < 2; ++d) {
        if (outer.min[d] > inner.min[d]) return false;
        if (outer.max[d] < inner.max[d]) return false;
    }
    return true;
}

bool32 bounding_box_contains(BoundingBox bounding_box, vec2 point) {
    for (uint32 d = 0; d < 2; ++d) {
        if (!IS_BETWEEN(point[d], bounding_box.min[d], bounding_box.max[d])) return false;
    }
    return true;
}

vec2 bounding_box_clamp(vec2 p, BoundingBox bounding_box) {
    for (uint32 d = 0; d < 2; ++d) {
        p[d] = CLAMP(p[d], bounding_box.min[d], bounding_box.max[d]);
    }
    return p;
}

void camera2D_zoom_to_bounding_box(Camera2D *camera_2D, BoundingBox bounding_box) {
    real32 new_o_x = AVG(bounding_box.min[0], bounding_box.max[0]);
    real32 new_o_y = AVG(bounding_box.min[1], bounding_box.max[1]);
    real32 new_height = MAX((bounding_box.max[0] - bounding_box.min[0]) * 2 / _window_get_aspect(), (bounding_box.max[1] - bounding_box.min[1])); // factor of 2 since splitscreen
    new_height *= 1.3f; // FORNOW: border
    camera_2D->screen_height_World = new_height;
    camera_2D->o_x = new_o_x;
    camera_2D->o_y = new_o_y;
}

////////////////////////////////////////
// List<DXFEntity> /////////////////////////////////
////////////////////////////////////////




void get_point_on_circle_NOTE_pass_angle_in_radians(real32 *x, real32 *y, real32 center_x, real32 center_y, real32 radius, real32 angle_in_radians) {
    *x = center_x + radius * COS(angle_in_radians);
    *y = center_y + radius * SIN(angle_in_radians);
}

void arc_process_angles_into_lerpable_radians_considering_flip_flag(DXFArc *arc, real32 *start_angle, real32 *end_angle, bool32 flip_flag) {
    // The way the List<DXFEntity> spec works is that start_angle and end_angle define points on the circle
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

real32 entity_length(DXFEntity *entity) {
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLine *line = &entity->line;
        return SQRT(squared_distance_point_point(line->start.x, line->start.y, line->end.x, line->end.y));
    } else {
        ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
        DXFArc *arc = &entity->arc;
        real32 start_angle;
        real32 end_angle;
        arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, false);
        return ABS(start_angle - end_angle) * arc->radius;
    }
}

void entity_get_start_point(DXFEntity *entity, real32 *start_x, real32 *start_y) {
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLine *line = &entity->line;
        *start_x = line->start.x;
        *start_y = line->start.y;
    } else {
        ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
        DXFArc *arc = &entity->arc;
        get_point_on_circle_NOTE_pass_angle_in_radians(start_x, start_y, arc->center.x, arc->center.y, arc->radius, RAD(arc->start_angle_in_degrees));
    }
}

void entity_get_end_point(DXFEntity *entity, real32 *end_x, real32 *end_y) {
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLine *line = &entity->line;
        *end_x = line->end.x;
        *end_y = line->end.y;
    } else {
        ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
        DXFArc *arc = &entity->arc;
        get_point_on_circle_NOTE_pass_angle_in_radians(  end_x,   end_y, arc->center.x, arc->center.y, arc->radius,   RAD(arc->end_angle_in_degrees));
    }
}

void entity_get_start_and_end_points(DXFEntity *entity, real32 *start_x, real32 *start_y, real32 *end_x, real32 *end_y) {
    entity_get_start_point(entity, start_x, start_y);
    entity_get_end_point(entity, end_x, end_y);
}

void entity_lerp_considering_flip_flag(DXFEntity *entity, real32 t, real32 *x, real32 *y, bool32 flip_flag) {
    ASSERT(IS_BETWEEN(t, 0.0f, 1.0f));
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLine *line = &entity->line;
        if (flip_flag) t = 1.0f - t; // FORNOW
        *x = LERP(t, line->start.x, line->end.x);
        *y = LERP(t, line->start.y, line->end.y);
    } else {
        ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
        DXFArc *arc = &entity->arc;
        real32 angle; {
            real32 start_angle, end_angle;
            arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, flip_flag); // FORNOW
            angle = LERP(t, start_angle, end_angle);
        }
        get_point_on_circle_NOTE_pass_angle_in_radians(x, y, arc->center.x, arc->center.y, arc->radius, angle);
    }
}

void entity_get_middle(DXFEntity *entity, real32 *middle_x, real32 *middle_y) {
    entity_lerp_considering_flip_flag(entity, 0.5f, middle_x, middle_y, false);
}

// struct List<DXFEntity> {
//     uint32 num_entities;
//     DXFEntity *entities;
// };

void dxf_entities_load(char *filename, List<DXFEntity> *dxf_entities) {
    #if 0
    {
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(filename);
        return {};
        #elif 0
        List<DXFEntity> result = {};
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
    list_free_AND_zero(dxf_entities);

    FILE *file = (FILE *) fopen(filename, "r");
    ASSERT(file);

    *dxf_entities = {}; {
        #define DXF_OPEN_MODE_NONE 0
        #define DXF_OPEN_MODE_LINE 1
        #define DXF_OPEN_MODE_ARC  2
        u8 mode = 0;
        int code = 0;
        bool32 code_is_hot = false;
        DXFEntity entity = {};
        static char buffer[512];
        while (fgets(buffer, ARRAY_LENGTH(buffer), file)) {
            if (mode == DXF_OPEN_MODE_NONE) {
                if (poe_prefix_match(buffer, "LINE")) {
                    mode = DXF_OPEN_MODE_LINE;
                    code_is_hot = false;
                    entity = {};
                    entity.type = DXF_ENTITY_TYPE_LINE;
                } else if (poe_prefix_match(buffer, "ARC")) {
                    mode = DXF_OPEN_MODE_ARC;
                    code_is_hot = false;
                    entity = {};
                    entity.type = DXF_ENTITY_TYPE_ARC;
                }
            } else {
                if (!code_is_hot) {
                    sscanf(buffer, "%d", &code);
                    // NOTE this initialization is sketchy but works
                    // probably don't make a habit of it
                    if (code == 0) {
                        list_push_back(dxf_entities, entity);
                        mode = DXF_OPEN_MODE_NONE;
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
                        if (mode == DXF_OPEN_MODE_LINE) {
                            if (code == 10) {
                                entity.line.start.x = MM(value);
                            } else if (code == 20) {
                                entity.line.start.y = MM(value);
                            } else if (code == 11) {
                                entity.line.end.x = MM(value);
                            } else if (code == 21) {
                                entity.line.end.y = MM(value);
                            }
                        } else {
                            ASSERT(mode == DXF_OPEN_MODE_ARC);
                            if (code == 10) {
                                entity.arc.center.x = MM(value);
                            } else if (code == 20) {
                                entity.arc.center.y = MM(value);
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

    fclose(file);
}

void _dxf_eso_color(uint32 color) {
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
        warn_once("WARNING: slits not implemented");
        eso_color(0.3f, 0.3f, 0.3f);
    }
}

void eso_dxf_entity__SOUP_LINES(DXFEntity *entity, int32 override_color = DXF_COLOR_DONT_OVERRIDE, real32 dx = 0.0f, real32 dy = 0.0f) {
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLine *line = &entity->line;
        _dxf_eso_color((override_color != DXF_COLOR_DONT_OVERRIDE) ? override_color : entity->color);
        eso_vertex(line->start.x + dx, line->start.y + dy);
        eso_vertex(line->end.x + dx,   line->end.y + dy);
    } else {
        ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
        DXFArc *arc = &entity->arc;
        real32 start_angle, end_angle;
        arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, false);
        real32 delta_angle = end_angle - start_angle;
        uint32 num_segments = (uint32) (1 + (delta_angle / TAU) * NUM_SEGMENTS_PER_CIRCLE);
        real32 increment = delta_angle / num_segments;
        real32 current_angle = start_angle;
        _dxf_eso_color((override_color != DXF_COLOR_DONT_OVERRIDE) ? override_color : entity->color);
        for (uint32 i = 0; i < num_segments; ++i) {
            real32 x, y;
            get_point_on_circle_NOTE_pass_angle_in_radians(&x, &y, arc->center.x, arc->center.y, arc->radius, current_angle);
            eso_vertex(x + dx, y + dy);
            current_angle += increment;
            get_point_on_circle_NOTE_pass_angle_in_radians(&x, &y, arc->center.x, arc->center.y, arc->radius, current_angle);
            eso_vertex(x + dx, y + dy);
        }
    }
}


void dxf_entities_debug_draw(Camera2D *camera_2D, List<DXFEntity> *dxf_entities, int32 override_color = DXF_COLOR_DONT_OVERRIDE) {
    eso_begin(camera_get_PV(camera_2D), SOUP_LINES);
    for (DXFEntity *entity = dxf_entities->array; entity < &dxf_entities->array[dxf_entities->length]; ++entity) {
        eso_dxf_entity__SOUP_LINES(entity, override_color);
    }
    eso_end();
}

BoundingBox dxf_entity_get_bounding_box(DXFEntity *entity) {
    BoundingBox result = { HUGE_VAL, HUGE_VAL, -HUGE_VAL, -HUGE_VAL };
    real32 s[2][2];
    uint32 n = 2;
    entity_get_start_and_end_points(entity, &s[0][0], &s[0][1], &s[1][0], &s[1][1]);
    for (uint32 i = 0; i < n; ++i) {
        for (uint32 d = 0; d < 2; ++d) {
            result.min[d] = MIN(result.min[d], s[i][d]);
            result.max[d] = MAX(result.max[d], s[i][d]);
        }
    }
    if (entity->type == DXF_ENTITY_TYPE_ARC) {
        DXFArc *arc = &entity->arc;
        // NOTE: endpoints already taken are of; we just have to deal with the quads (if they exist)
        // TODO: angle_is_between_counter_clockwise (TODO TODO TODO)
        if (ANGLE_IS_BETWEEN_CCW(  0.0f, arc->start_angle_in_degrees, arc->end_angle_in_degrees)) result.max[0] = MAX(result.max[0], arc->center.x + arc->radius);
        if (ANGLE_IS_BETWEEN_CCW( 90.0f, arc->start_angle_in_degrees, arc->end_angle_in_degrees)) result.max[1] = MAX(result.max[1], arc->center.y + arc->radius);
        if (ANGLE_IS_BETWEEN_CCW(180.0f, arc->start_angle_in_degrees, arc->end_angle_in_degrees)) result.min[0] = MIN(result.min[0], arc->center.x - arc->radius);
        if (ANGLE_IS_BETWEEN_CCW(270.0f, arc->start_angle_in_degrees, arc->end_angle_in_degrees)) result.min[1] = MIN(result.min[1], arc->center.y - arc->radius);
    }
    return result;
}

BoundingBox dxf_entities_get_bounding_box(List<DXFEntity> *dxf_entities, bool32 only_consider_selected_entities = false) {
    BoundingBox result = { HUGE_VAL, HUGE_VAL, -HUGE_VAL, -HUGE_VAL }; 
    for (uint32 i = 0; i < dxf_entities->length; ++i) {
        if ((only_consider_selected_entities) && (!dxf_entities->array[i].is_selected)) continue;
        for (uint32 d = 0; d < 2; ++d) {
            BoundingBox bounding_box = dxf_entity_get_bounding_box(&dxf_entities->array[i]);
            result.min[d] = MIN(result.min[d], bounding_box.min[d]);
            result.max[d] = MAX(result.max[d], bounding_box.max[d]);
        }
    }
    return result;
}

////////////////////////////////////////
// SquaredDistance /////////////////////
////////////////////////////////////////


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
    return squared_distance_point_line_segment(x, y, line->start.x, line->start.y, line->end.x, line->end.y);
}

real32 squared_distance_point_dxf_arc(real32 x, real32 y, DXFArc *arc) {
    return squared_distance_point_arc_NOTE_pass_angles_in_radians(x, y, arc->center.x, arc->center.y, arc->radius, RAD(arc->start_angle_in_degrees), RAD(arc->end_angle_in_degrees));
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

real32 squared_distance_point_dxf(real32 x, real32 y, List<DXFEntity> *dxf_entities) {
    real32 result = HUGE_VAL;
    for (DXFEntity *entity = dxf_entities->array; entity < &dxf_entities->array[dxf_entities->length]; ++entity) {
        result = MIN(result, squared_distance_point_dxf_entity(x, y, entity));
    }
    return result;
}

int dxf_find_closest_entity(List<DXFEntity> *dxf_entities, real32 x, real32 y) {
    int result = -1;
    double hot_squared_distance = HUGE_VAL;
    for (uint32 i = 0; i < dxf_entities->length; ++i) {
        DXFEntity *entity = &dxf_entities->array[i];
        double squared_distance = squared_distance_point_dxf_entity(x, y, entity);
        if (squared_distance < hot_squared_distance) {
            hot_squared_distance = squared_distance;
            result = i;
        }
    }
    return result;
}


////////////////////////////////////////
// LoopAnalysis ////////////////////////
////////////////////////////////////////

struct DXFEntityIndexAndFlipFlag {
    uint32 entity_index;
    bool32 flip_flag;
};

struct DXFLoopAnalysisResult {
    uint32 num_loops;
    uint32 *num_entities_in_loops;
    DXFEntityIndexAndFlipFlag **loops;
    uint32 *loop_index_from_entity_index;
};

DXFLoopAnalysisResult dxf_loop_analysis_create_FORNOW_QUADRATIC(List<DXFEntity> *dxf_entities, bool32 only_consider_selected_entities) {
    if (dxf_entities->length == 0) {
        DXFLoopAnalysisResult result = {};
        result.num_loops = 0;
        result.num_entities_in_loops = (uint32 *) calloc(result.num_loops, sizeof(uint32));
        result.loops = (DXFEntityIndexAndFlipFlag **) calloc(result.num_loops, sizeof(DXFEntityIndexAndFlipFlag *));
        result.loop_index_from_entity_index = (uint32 *) calloc(dxf_entities->length, sizeof(uint32));
        return result;
    }

    DXFLoopAnalysisResult result = {};
    { // num_entities_in_loops, loops
      // populate List's
        List<List<DXFEntityIndexAndFlipFlag>> stretchy_list = {}; {
            bool32 *entity_already_added = (bool32 *) calloc(dxf_entities->length, sizeof(bool32));
            while (true) {
                #define MACRO_CANDIDATE_VALID(i) (!entity_already_added[i] && (!only_consider_selected_entities || dxf_entities->array[i].is_selected))
                { // seed loop
                    bool32 added_and_seeded_new_loop = false;
                    for (uint32 entity_index = 0; entity_index < dxf_entities->length; ++entity_index) {
                        if (MACRO_CANDIDATE_VALID(entity_index)) {
                            added_and_seeded_new_loop = true;
                            entity_already_added[entity_index] = true;
                            list_push_back(&stretchy_list, {});
                            list_push_back(&stretchy_list.array[stretchy_list.length - 1], { entity_index, false });
                            break;
                        }
                    }
                    if (!added_and_seeded_new_loop) break;
                }
                { // continue and complete
                    real32 tolerance = TOLERANCE_DEFAULT;
                    while (true) {
                        bool32 added_new_entity_to_loop = false;
                        for (uint32 entity_index = 0; entity_index < dxf_entities->length; ++entity_index) {
                            if (!MACRO_CANDIDATE_VALID(entity_index)) continue;
                            real32 start_x_prev, start_y_prev, end_x_prev, end_y_prev;
                            real32 start_x_i, start_y_i, end_x_i, end_y_i;
                            DXFEntityIndexAndFlipFlag *prev_entity_index_and_flip_flag = &(stretchy_list.array[stretchy_list.length - 1].array[stretchy_list.array[stretchy_list.length - 1].length - 1]);
                            {
                                entity_get_start_and_end_points(
                                        &dxf_entities->array[prev_entity_index_and_flip_flag->entity_index],
                                        &start_x_prev, &start_y_prev, &end_x_prev, &end_y_prev);
                                entity_get_start_and_end_points(&dxf_entities->array[entity_index], &start_x_i, &start_y_i, &end_x_i, &end_y_i);
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
                                list_push_back(&stretchy_list.array[stretchy_list.length - 1], { entity_index, flip_flag });
                                break;
                            }
                        }
                        if (!added_new_entity_to_loop) break;
                    }
                }

                { // reverse_loop if necessary
                    uint32 num_entities_in_loop = stretchy_list.array[stretchy_list.length - 1].length;
                    DXFEntityIndexAndFlipFlag *loop = stretchy_list.array[stretchy_list.length - 1].array;
                    bool32 reverse_loop; {
                        #if 0
                        reverse_loop = false;
                        #else
                        real32 twice_the_signed_area; {
                            twice_the_signed_area = 0.0f;
                            for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities_in_loop; ++entity_index_and_flip_flag) {
                                uint32 entity_index = entity_index_and_flip_flag->entity_index;
                                bool32 flip_flag = entity_index_and_flip_flag->flip_flag;
                                DXFEntity *entity = &dxf_entities->array[entity_index];
                                if (entity->type == DXF_ENTITY_TYPE_LINE) {
                                    DXFLine *line = &entity->line;
                                    // shoelace-type formula
                                    twice_the_signed_area += ((flip_flag) ? -1 : 1) * (line->start.x * line->end.y - line->end.x * line->start.y);
                                } else {
                                    ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                                    DXFArc *arc = &entity->arc;
                                    // "Circular approximation using polygons"
                                    // - n = 2 (area-preserving approximation of arc with two segments)
                                    real32 start_angle, end_angle;
                                    arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, flip_flag);
                                    real32 start_x, start_y, end_x, end_y;
                                    get_point_on_circle_NOTE_pass_angle_in_radians(&start_x, &start_y, arc->center.x, arc->center.y, arc->radius, start_angle);
                                    get_point_on_circle_NOTE_pass_angle_in_radians(  &end_x,   &end_y, arc->center.x, arc->center.y, arc->radius,   end_angle);
                                    real32 mid_angle = (start_angle + end_angle) / 2;
                                    real32 d; {
                                        real32 alpha = ABS(start_angle - end_angle) / 2;
                                        d = arc->radius * alpha / SIN(alpha);
                                    }
                                    real32 mid_x, mid_y;
                                    get_point_on_circle_NOTE_pass_angle_in_radians(&mid_x, &mid_y, arc->center.x, arc->center.y, d, mid_angle);
                                    twice_the_signed_area += mid_x * (end_y - start_y) + mid_y * (start_x - end_x);
                                }
                            }
                        }
                        reverse_loop = (twice_the_signed_area < 0.0f);
                        #endif
                    }
                    if (reverse_loop) {
                        for (uint32 i = 0, j = (num_entities_in_loop - 1); i < j; ++i, --j) {
                            DXFEntityIndexAndFlipFlag tmp = loop[i];
                            loop[i] = loop[j];
                            loop[j] = tmp;
                        }
                        for (uint32 i = 0; i < num_entities_in_loop; ++i) {
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
        result.num_entities_in_loops = (uint32 *) calloc(result.num_loops, sizeof(uint32));
        result.loops = (DXFEntityIndexAndFlipFlag **) calloc(result.num_loops, sizeof(DXFEntityIndexAndFlipFlag *));
        for (uint32 i = 0; i < result.num_loops; ++i) {
            result.num_entities_in_loops[i] = stretchy_list.array[i].length;
            result.loops[i] = (DXFEntityIndexAndFlipFlag *) calloc(result.num_entities_in_loops[i], sizeof(DXFEntityIndexAndFlipFlag));
            memcpy(result.loops[i], stretchy_list.array[i].array, result.num_entities_in_loops[i] * sizeof(DXFEntityIndexAndFlipFlag));
        }

        // free List's
        for (uint32 i = 0; i < stretchy_list.length; ++i) list_free_AND_zero(&stretchy_list.array[i]);
        list_free_AND_zero(&stretchy_list);
    }
    // loop_index_from_entity_index (brute force)
    result.loop_index_from_entity_index = (uint32 *) calloc(dxf_entities->length, sizeof(uint32));
    for (uint32 i = 0; i < dxf_entities->length; ++i) {
        for (uint32 j = 0; j < result.num_loops; ++j) {
            for (uint32 k = 0; k < result.num_entities_in_loops[j]; ++k) {
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
        for (uint32 i = 0; i < analysis->num_loops; ++i) {
            free(analysis->loops[i]);
        }
        free(analysis->loops);
    }
    if (analysis->loop_index_from_entity_index) {
        free(analysis->loop_index_from_entity_index);
    }
    *analysis = {};
}

////////////////////////////////////////
// LoopAnalysis ////////////////////////
////////////////////////////////////////

struct CrossSectionEvenOdd {
    uint32 num_polygonal_loops;
    uint32 *num_vertices_in_polygonal_loops;
    vec2 **polygonal_loops;
};

CrossSectionEvenOdd cross_section_create_FORNOW_QUADRATIC(List<DXFEntity> *dxf_entities, bool32 only_consider_selected_entities) {
    #if 0
    {
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(dxf_entities);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(include);
        CrossSectionEvenOdd result = {};
        result.num_polygonal_loops = 2;
        result.num_vertices_in_polygonal_loops = (uint32 *) calloc(result.num_polygonal_loops, sizeof(uint32));
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
        DXFLoopAnalysisResult analysis = dxf_loop_analysis_create_FORNOW_QUADRATIC(dxf_entities, only_consider_selected_entities);
        for (uint32 loop_index = 0; loop_index < analysis.num_loops; ++loop_index) {
            uint32 num_entities_in_loop = analysis.num_entities_in_loops[loop_index];
            DXFEntityIndexAndFlipFlag *loop = analysis.loops[loop_index];
            list_push_back(&stretchy_list, {});
            for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities_in_loop; ++entity_index_and_flip_flag) {
                uint32 entity_index = entity_index_and_flip_flag->entity_index;
                bool32 flip_flag = entity_index_and_flip_flag->flip_flag;
                DXFEntity *entity = &dxf_entities->array[entity_index];
                if (entity->type == DXF_ENTITY_TYPE_LINE) {
                    DXFLine *line = &entity->line;
                    if (!flip_flag) {
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], { line->start.x, line->start.y });
                    } else {
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], { line->end.x, line->end.y });
                    }
                } else {
                    ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                    DXFArc *arc = &entity->arc;
                    real32 start_angle, end_angle;
                    arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, flip_flag);
                    real32 delta_angle = end_angle - start_angle;
                    uint32 num_segments = (uint32) (2 + ABS(delta_angle) * (NUM_SEGMENTS_PER_CIRCLE / TAU)); // FORNOW (2 + ...)
                    real32 increment = delta_angle / num_segments;
                    real32 current_angle = start_angle;
                    real32 x, y;
                    for (uint32 i = 0; i < num_segments; ++i) {
                        get_point_on_circle_NOTE_pass_angle_in_radians(&x, &y, arc->center.x, arc->center.y, arc->radius, current_angle);
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], { x, y });
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
    result.num_vertices_in_polygonal_loops = (uint32 *) calloc(result.num_polygonal_loops, sizeof(uint32));
    result.polygonal_loops = (vec2 **) calloc(result.num_polygonal_loops, sizeof(vec2 *));
    for (uint32 i = 0; i < result.num_polygonal_loops; ++i) {
        result.num_vertices_in_polygonal_loops[i] = stretchy_list.array[i].length;
        result.polygonal_loops[i] = (vec2 *) calloc(result.num_vertices_in_polygonal_loops[i], sizeof(vec2));
        memcpy(result.polygonal_loops[i], stretchy_list.array[i].array, result.num_vertices_in_polygonal_loops[i] * sizeof(vec2));
    }

    // free List's
    for (uint32 i = 0; i < stretchy_list.length; ++i) list_free_AND_zero(&stretchy_list.array[i]);
    list_free_AND_zero(&stretchy_list);

    return result;
}

void cross_section_debug_draw(Camera2D *camera_2D, CrossSectionEvenOdd *cross_section) {
    eso_begin(camera_get_PV(camera_2D), SOUP_LINES);
    eso_color(monokai.white);
    for (uint32 loop_index = 0; loop_index < cross_section->num_polygonal_loops; ++loop_index) {
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

void cross_section_free(CrossSectionEvenOdd *cross_section) {
    free(cross_section->num_vertices_in_polygonal_loops);
    for (uint32 i = 0; i < cross_section->num_polygonal_loops; ++i) free(cross_section->polygonal_loops[i]);
    free(cross_section->polygonal_loops);
}

////////////////////////////////////////
// Mesh, STL //////////////////////
////////////////////////////////////////


void mesh_triangle_normals_calculate(Mesh *mesh) {
    mesh->triangle_normals = (real32 *) malloc(mesh->num_triangles * 3 * sizeof(real32));
    vec3 p[3];
    for (uint32 i = 0; i < mesh->num_triangles; ++i) {
        for (uint32 j = 0; j < 3; ++j) p[j] = get(mesh->vertex_positions, mesh->triangle_indices[3 * i + j]);
        vec3 n = normalized(cross(p[1] - p[0], p[2] - p[0]));
        set(mesh->triangle_normals, i, n);
    }
}

void mesh_cosmetic_edges_calculate(Mesh *mesh) {
    // approach: prep a big array that maps edge -> cwiseProduct of face normals (start it at 1, 1, 1) // (faces that edge is part of)
    //           iterate through all edges detministically (ccw in order, flipping as needed so lower_index->higher_index)
    //           then go back and if passes some heuristic add that index to a stretchy buffer
    List<uint32> list = {}; {
        Map<Pair<uint32, uint32>, vec3> map = {}; {
            for (uint32 i = 0; i < mesh->num_triangles; ++i) {
                vec3 n = get(mesh->triangle_normals, i);
                for (uint32 jj0 = 0, jj1 = (3 - 1); jj0 < 3; jj1 = jj0++) {
                    uint32 j0 = mesh->triangle_indices[3 * i + jj0];
                    uint32 j1 = mesh->triangle_indices[3 * i + jj1];
                    if (j0 > j1) {
                        uint32 tmp = j0;
                        j0 = j1;
                        j1 = tmp;
                    }
                    Pair<uint32, uint32> key = { j0, j1 };
                    map_put(&map, key, cwiseProduct(n, map_get(&map, key, V3(1.0f))));
                }
            }
        }
        {
            for (List<Pair<Pair<uint32, uint32>, vec3>> *bucket = map.buckets; bucket < &map.buckets[map.num_buckets]; ++bucket) {
                for (Pair<Pair<uint32, uint32>, vec3> *pair = bucket->array; pair < &bucket->array[bucket->length]; ++pair) {
                    vec3 n2 = pair->value;
                    // pprint(n2);
                    real32 angle = DEG(acos(n2.x + n2.y + n2.z)); // [0.0f, 180.0f]
                    if
                        // (squaredNorm(n2) < 0.3f)
                        (angle > 30.0f)
                        {
                            list_push_back(&list, pair->key.first); // FORNOW
                            list_push_back(&list, pair->key.second); // FORNOW
                        }
                }
            }
        }
        map_free_and_zero(&map);
    }
    {
        mesh->num_cosmetic_edges = list.length / 2;
        mesh->cosmetic_edges = (uint32 *) calloc(2 * mesh->num_cosmetic_edges, sizeof(uint32));
        memcpy(mesh->cosmetic_edges, list.array, 2 * mesh->num_cosmetic_edges * sizeof(uint32)); 
    }
    list_free_AND_zero(&list);
}

void mesh_bounding_box_calculate(Mesh *mesh) {
    mesh->min = V3(HUGE_VAL);
    mesh->max = V3(-HUGE_VAL);
    for (uint32 i = 0; i < mesh->num_vertices; ++i) {
        for (uint32 d = 0; d < 3; ++d) {
            mesh->min[d] = MIN(mesh->min[d], mesh->vertex_positions[3 * i + d]);
            mesh->max[d] = MAX(mesh->max[d], mesh->vertex_positions[3 * i + d]);
        }
    }
}

bool32 mesh_save_stl(Mesh *mesh, char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        return false;
    }

    int num_bytes = 80 + 4 + 50 * mesh->num_triangles;
    char *buffer = (char *) calloc(num_bytes, 1); {
        int offset = 80;
        memcpy(buffer + offset, &mesh->num_triangles, 4);
        offset += 4;
        for (uint32 i = 0; i < mesh->num_triangles; ++i) {
            real32 triangle_normal[3];
            {
                // 90 degree rotation about x: (x, y, z) <- (x, -z, y)
                triangle_normal[0] =  mesh->triangle_normals[3 * i + 0];
                triangle_normal[1] = -mesh->triangle_normals[3 * i + 2];
                triangle_normal[2] =  mesh->triangle_normals[3 * i + 1];
            }
            memcpy(buffer + offset, &triangle_normal, 12);
            offset += 12;
            real32 triangle_vertex_positions[9];
            for (uint32 j = 0; j < 3; ++j) {
                // 90 degree rotation about x: (x, y, z) <- (x, -z, y)
                triangle_vertex_positions[3 * j + 0] =  mesh->vertex_positions[3 * mesh->triangle_indices[3 * i + j] + 0];
                triangle_vertex_positions[3 * j + 1] = -mesh->vertex_positions[3 * mesh->triangle_indices[3 * i + j] + 2];
                triangle_vertex_positions[3 * j + 2] =  mesh->vertex_positions[3 * mesh->triangle_indices[3 * i + j] + 1];
            }
            memcpy(buffer + offset, triangle_vertex_positions, 36);
            offset += 38;
        }
    }
    fwrite(buffer, 1, num_bytes, file);
    free(buffer);

    fclose(file);
    return true;
}

void mesh_free_AND_zero(Mesh *mesh) {
    if (mesh->vertex_positions) free(mesh->vertex_positions);
    if (mesh->triangle_indices) free(mesh->triangle_indices);
    if (mesh->triangle_normals) free(mesh->triangle_normals);
    if (mesh->cosmetic_edges)   free(mesh->cosmetic_edges);
    *mesh = {};
}

void mesh_deep_copy(Mesh *dst, Mesh *src) {
    *dst = *src;
    if (src->vertex_positions) {
        uint32 size = 3 * src->num_vertices * sizeof(real32);
        dst->vertex_positions = (real32 *) malloc(size);
        memcpy(dst->vertex_positions, src->vertex_positions, size);
    }
    if (src->triangle_indices) {
        uint32 size = 3 * src->num_triangles * sizeof(uint32);
        dst->triangle_indices = (uint32 *) malloc(size);
        memcpy(dst->triangle_indices, src->triangle_indices, size);
    }
    if (src->triangle_normals) {
        uint32 size = 3 * src->num_triangles * sizeof(real32); 
        dst->triangle_normals = (real32 *) malloc(size);
        memcpy(dst->triangle_normals, src->triangle_normals, size);
    }
    if (src->cosmetic_edges) {
        uint32 size = 2 * src->num_cosmetic_edges * sizeof(uint32);
        dst->cosmetic_edges = (uint32 *) malloc(size);
        memcpy(dst->cosmetic_edges, src->cosmetic_edges, size);
    }
}

void stl_load(char *filename, Mesh *mesh) {
    // history_record_state(history, manifold_manifold, mesh); // FORNOW

    { // mesh
        uint32 num_triangles;
        real32 *soup;
        {
            static char line_of_file[512];

            #define STL_FILETYPE_UNKNOWN 0
            #define STL_FILETYPE_ASCII   1
            #define STL_FILETYPE_BINARY  2
            uint32 filetype; {
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
                        for (uint32 d = 0; d < 3; ++d) list_push_back(&ascii_data, ascii_scan_p[d]);
                    }
                }
                fclose(file);
                num_triangles = ascii_data.length / 9;
                uint32 size = ascii_data.length * sizeof(real32);
                soup = (real32 *) malloc(size);
                memcpy(soup, ascii_data.array, size);
                list_free_AND_zero(&ascii_data);
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
                uint32 offset = 80;
                memcpy(&num_triangles, entire_file + offset, 4);
                offset += 4;
                uint32 size = num_triangles * 36;
                soup = (real32 *) calloc(1, size);
                for (uint32 i = 0; i < num_triangles; ++i) {
                    offset += 12;
                    memcpy(soup + i * 9, entire_file + offset, 36);
                    offset += 38;
                }
            }
            { // -90 degree rotation about x: (x, y, z) <- (x, z, -y)
                uint32 num_vertices = 3 * num_triangles;
                for (uint32 i = 0; i < num_vertices; ++i) {
                    real32 tmp = soup[3 * i + 1];
                    soup[3 * i + 1] = soup[3 * i + 2];
                    soup[3 * i + 2] = -tmp;
                }
            }
        }

        uint32 num_vertices;
        real32 *vertex_positions;
        uint32 *triangle_indices;
        { // merge vertices
            num_vertices = 0;
            Map<vec3, uint32> map = {};
            uint32 _3_TIMES_num_triangles = 3 * num_triangles;
            {
                List<vec3> list = {};
                for (uint32 i = 0; i < _3_TIMES_num_triangles; ++i) {
                    vec3 p = get(soup, i);
                    uint32 default_value = _3_TIMES_num_triangles + 1;
                    uint32 j = map_get(&map, p, default_value);
                    if (j == default_value) {
                        map_put(&map, p, num_vertices++);
                        list_push_back(&list, p);
                    }
                }
                {
                    uint32 size = list.length * sizeof(vec3);
                    vertex_positions = (real32 *) malloc(size);
                    memcpy(vertex_positions, list.array, size);
                }
                list_free_AND_zero(&list);
            }
            triangle_indices = (uint32 *) malloc(_3_TIMES_num_triangles * sizeof(uint32));
            for (uint32 k = 0; k < _3_TIMES_num_triangles; ++k) triangle_indices[k] = map_get(&map, ((vec3 *) soup)[k]);
            map_free_and_zero(&map);
            free(soup);
        }
        mesh->num_vertices = num_vertices;
        mesh->num_triangles = num_triangles;
        mesh->vertex_positions = vertex_positions;
        mesh->triangle_indices = triangle_indices;
        mesh_triangle_normals_calculate(mesh);
        mesh_cosmetic_edges_calculate(mesh);
        mesh_bounding_box_calculate(mesh);
    }
}

////////////////////////////////////////////////////////////////////////////////
// freaky local_persist stuff //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void conversation_draw_3D_grid_box(mat4 P_3D, mat4 V_3D) {
    static IndexedTriangleMesh3D grid_box;
    if (grid_box.num_vertices == 0) {
        static int _grid_box_num_vertices = 24;
        static int _grid_box_num_triangles = 12;
        static int3 _grid_box_triangle_indices[] = {
            { 1, 0, 2},{ 2, 0, 3},
            { 4, 5, 6},{ 4, 6, 7},
            { 8, 9,10},{ 8,10,11},
            {13,12,14},{14,12,15},
            {17,16,18},{18,16,19},
            {20,21,22},{20,22,23},
        };
        static vec3 _grid_box_vertex_positions[] = {
            { 1, 1, 1},{ 1, 1,-1},{ 1,-1,-1},{ 1,-1, 1},
            {-1, 1, 1},{-1, 1,-1},{-1,-1,-1},{-1,-1, 1},
            { 1, 1, 1},{ 1, 1,-1},{-1, 1,-1},{-1, 1, 1},
            { 1,-1, 1},{ 1,-1,-1},{-1,-1,-1},{-1,-1, 1},
            { 1, 1, 1},{ 1,-1, 1},{-1,-1, 1},{-1, 1, 1},
            { 1, 1,-1},{ 1,-1,-1},{-1,-1,-1},{-1, 1,-1},
        };
        static vec2 _grid_box_vertex_texCoords[] = {
            {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
            {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
            {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
            {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
            {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
            {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
        };
        grid_box = {
            _grid_box_num_vertices,
            _grid_box_num_triangles,
            _grid_box_vertex_positions,
            NULL,
            NULL,
            _grid_box_triangle_indices,
            _grid_box_vertex_texCoords
        };

        uint32 texture_side_length = 1024;
        uint32 number_of_channels = 4;
        u8 *array = (u8 *) malloc(texture_side_length * texture_side_length * number_of_channels * sizeof(u8));
        uint32 o = 9;
        for (uint32 j = 0; j < texture_side_length; ++j) {
            for (uint32 i = 0; i < texture_side_length; ++i) {
                uint32 k = number_of_channels * (j * texture_side_length + i);
                uint32 n = uint32(texture_side_length / GRID_SIDE_LENGTH * 10);
                uint32 t = 2;
                bool32 stripe = (((i + o) % n < t) || ((j + o) % n < t));
                u8 a = 160;
                u8 value = (u8)((255.0f/166.0f) * 0.07f * 255);
                if (stripe) value = 80;
                if (i < t || j < t || i > texture_side_length - t - 1 || j > texture_side_length - t - 1) value = 160;
                for (uint32 d = 0; d < 3; ++d) array[k + d] = value;
                array[k + 3] = a;
            }
        }
        _mesh_texture_create("procedural grid", texture_side_length, texture_side_length, number_of_channels, array);
    }
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    real32 L = GRID_SIDE_LENGTH;
    grid_box.draw(P_3D, V_3D, M4_Translation(0.0f, - 2 * Z_FIGHT_EPS, 0.0f) * M4_Scaling(L / 2), {}, "procedural grid");
    glDisable(GL_CULL_FACE);
}

////////////////////////////////////////////////////////////////////////////////
// messagef API ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool32 IGNORE_NEW_MESSAGEFS;

#define MESSAGE_MAX_LENGTH 256
#define MESSAGE_MAX_NUM_MESSAGES 64
#define MESSAGE_COOLDOWN 300
struct Message {
    char buffer[MESSAGE_MAX_LENGTH];
    uint32 cooldown;
};
uint32 message_index;
Message conversation_messages[MESSAGE_MAX_NUM_MESSAGES];
void conversation_messagef(char *format, ...) {
    if (IGNORE_NEW_MESSAGEFS) return;
    va_list arg;
    va_start(arg, format);
    Message *message = &conversation_messages[message_index];
    vsnprintf(message->buffer, MESSAGE_MAX_LENGTH, format, arg);
    va_end(arg);

    message->cooldown = MESSAGE_COOLDOWN;
    message_index = (message_index + 1) % MESSAGE_MAX_NUM_MESSAGES;

    printf("%s\n", message->buffer); // FORNOW print to terminal as well
}
void conversation_message_buffer_update_and_draw() {
    uint32 i_0 =  (message_index == 0) ? (MESSAGE_MAX_NUM_MESSAGES - 1) : message_index - 1;
    real32 y = 32;

    auto draw_lambda = [&](uint32 i) {
        real32 a = real32(conversation_messages[i].cooldown) / MESSAGE_COOLDOWN;
        if (conversation_messages[i].cooldown > 0) {
            --conversation_messages[i].cooldown;
            _text_draw(
                    (cow_real *) &globals._gui_NDC_from_Screen,
                    conversation_messages[i].buffer,
                    512,
                    y,
                    0.0,
                    0.5,
                    1.0,
                    1.0,
                    a,
                    0,
                    0.0,
                    0.0,
                    true);
            y += 32;
        } else {
            conversation_messages[i].cooldown = 0;
        }
    };

    for (uint32 i = i_0; i > 0; --i) draw_lambda(i);
    draw_lambda(0);
    for (uint32 i = MESSAGE_MAX_NUM_MESSAGES - 1; i > i_0; --i) draw_lambda(i);

}

////////////////////////////////////////////////////////////////////////////////
// key_lambda //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool _key_lambda(UserEvent event, uint32 key, bool super = false, bool shift = false) {
    ASSERT(event.type == USER_EVENT_TYPE_KEY_PRESS);
    ASSERT(!(('a' <= key) && (key <= 'z')));
    bool key_match = (event.key == key);
    bool super_match = ((event.super && super) || (!event.super && !super)); // * bool32
    bool shift_match = ((event.shift && shift) || (!event.shift && !shift)); // * bool32
    return (key_match && super_match && shift_match);
};

////////////////////////////////////////////////////////////////////////////////
// world_state /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void world_state_deep_copy(WorldState *dst, WorldState *src) {
    *dst = *src;
    dst->dxf.entities = {};
    list_clone(&dst->dxf.entities,    &src->dxf.entities   );
    mesh_deep_copy(&dst->mesh, &src->mesh);
}

void world_state_free_AND_zero(WorldState *world_state) {
    mesh_free_AND_zero(&world_state->mesh);
    list_free_AND_zero(&world_state->dxf.entities);
}

////////////////////////////////////////////////////////////////////////////////
// uh oh ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// TODO: don't overwrite fancy mesh, let the calling code do what it will
// TODO: could this take a printf function pointer?
Mesh wrapper_manifold(
        Mesh *mesh, // dest__NOTE_GETS_OVERWRITTEN,
        uint32 num_polygonal_loops,
        uint32 *num_vertices_in_polygonal_loops,
        vec2 **polygonal_loops,
        mat4 M_3D_from_2D,
        uint32 enter_mode,
        real32 extrude_out_length,
        real32 extrude_in_length,
        vec2   dxf_origin,
        vec2   dxf_axis_base_point,
        real32 dxf_axis_angle_from_y
        ) {


    bool32 add = (enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_ADD);
    bool32 cut = (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_CUT);
    bool32 extrude = (enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT);
    bool32 revolve = (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT);
    ASSERT(add || cut);
    ASSERT(extrude || revolve);

    ManifoldManifold *manifold_A; {
        if (mesh->num_vertices == 0) {
            manifold_A = NULL;
        } else { // manifold <- mesh
            ManifoldMeshGL *meshgl = manifold_meshgl(
                    malloc(manifold_meshgl_size()),
                    mesh->vertex_positions,
                    mesh->num_vertices,
                    3,
                    mesh->triangle_indices,
                    mesh->num_triangles);

            manifold_A = manifold_of_meshgl(malloc(manifold_manifold_size()), meshgl);

            manifold_delete_meshgl(meshgl);
        }
    }

    ManifoldManifold *manifold_B; {
        ManifoldSimplePolygon **simple_polygon_array; {
            simple_polygon_array = (ManifoldSimplePolygon **) malloc(num_polygonal_loops * sizeof(ManifoldSimplePolygon *));
            for (uint32 i = 0; i < num_polygonal_loops; ++i) {
                simple_polygon_array[i] = manifold_simple_polygon(malloc(manifold_simple_polygon_size()), (ManifoldVec2 *) polygonal_loops[i], num_vertices_in_polygonal_loops[i]);
            }
        } 
        ManifoldPolygons *polygons; {
            polygons = manifold_polygons(malloc(manifold_polygons_size()), simple_polygon_array, num_polygonal_loops);
        }
        ManifoldCrossSection *cross_section; {
            cross_section = manifold_cross_section_of_polygons(malloc(manifold_cross_section_size()), polygons, ManifoldFillRule::MANIFOLD_FILL_RULE_EVEN_ODD);
            cross_section = manifold_cross_section_translate(cross_section, cross_section, -dxf_origin.x, -dxf_origin.y);

            if (revolve) {
                manifold_cross_section_translate(cross_section, cross_section, -dxf_axis_base_point.x, -dxf_axis_base_point.y);
                manifold_cross_section_rotate(cross_section, cross_section, DEG(-dxf_axis_angle_from_y)); // * has both the 90 y-up correction and the angle
            }
        }

        { // manifold_B
            if (enter_mode == ENTER_MODE_EXTRUDE_CUT) {
                warn_once("[DEBUG] inflating ENTER_MODE_EXTRUDE_CUT\n");
                extrude_in_length += SGN(extrude_in_length) * TOLERANCE_DEFAULT;
                extrude_out_length += SGN(extrude_out_length) * TOLERANCE_DEFAULT;
            }

            // NOTE: params are arbitrary sign (and can be same sign)--a typical thing would be like (30, -30)
            //       but we support (30, 40) -- which is equivalent to (40, 0)

            if (extrude) {
                real32 length = extrude_in_length + extrude_out_length;
                manifold_B = manifold_extrude(malloc(manifold_manifold_size()), cross_section, length, 0, 0.0f, 1.0f, 1.0f);
                manifold_B = manifold_translate(manifold_B, manifold_B, 0.0f, 0.0f, -extrude_in_length);
            } else { ASSERT(revolve);
                // TODO: M_3D_from_2D 
                manifold_B = manifold_revolve(malloc(manifold_manifold_size()), cross_section, NUM_SEGMENTS_PER_CIRCLE);
                manifold_B = manifold_rotate(manifold_B, manifold_B, 0.0, DEG(-dxf_axis_angle_from_y), 0.0f); // *
                manifold_B = manifold_rotate(manifold_B, manifold_B, -90.0f, 0.0f, 0.0f);
                manifold_B = manifold_translate(manifold_B, manifold_B, dxf_axis_base_point.x, dxf_axis_base_point.y, 0.0f);
            }
            manifold_B = manifold_transform(manifold_B, manifold_B,
                    M_3D_from_2D(0, 0), M_3D_from_2D(1, 0), M_3D_from_2D(2, 0),
                    M_3D_from_2D(0, 1), M_3D_from_2D(1, 1), M_3D_from_2D(2, 1),
                    M_3D_from_2D(0, 2), M_3D_from_2D(1, 2), M_3D_from_2D(2, 2),
                    M_3D_from_2D(0, 3), M_3D_from_2D(1, 3), M_3D_from_2D(2, 3));
        }

        { // free(simple_polygon_array)
            for (uint32 i = 0; i < num_polygonal_loops; ++i) manifold_delete_simple_polygon(simple_polygon_array[i]);
            free(simple_polygon_array);
        }
        manifold_delete_polygons(polygons);
        manifold_delete_cross_section(cross_section);
    }

    Mesh result; { // C <- f(A, B)
        ManifoldMeshGL *meshgl; {
            ManifoldManifold *manifold_C;
            if (manifold_A == NULL) {
                ASSERT(!cut);
                manifold_C = manifold_B;
            } else {
                // TODO: ? manifold_delete_manifold(manifold_A);
                manifold_C =
                    manifold_boolean(
                            malloc(manifold_manifold_size()),
                            manifold_A,
                            manifold_B,
                            (add) ? ManifoldOpType::MANIFOLD_ADD : ManifoldOpType::MANIFOLD_SUBTRACT
                            );
                manifold_delete_manifold(manifold_A);
                manifold_delete_manifold(manifold_B);
            }

            meshgl = manifold_get_meshgl(malloc(manifold_meshgl_size()), manifold_C);
        }

        { // result <- meshgl
            result = {};
            result.num_vertices = manifold_meshgl_num_vert(meshgl);
            result.num_triangles = manifold_meshgl_num_tri(meshgl);
            result.vertex_positions = manifold_meshgl_vert_properties(malloc(manifold_meshgl_vert_properties_length(meshgl) * sizeof(real32)), meshgl);
            result.triangle_indices = manifold_meshgl_tri_verts(malloc(manifold_meshgl_tri_length(meshgl) * sizeof(uint32)), meshgl);
            mesh_triangle_normals_calculate(&result);
            mesh_cosmetic_edges_calculate(&result);
            mesh_bounding_box_calculate(&result);
        }

        manifold_delete_meshgl(meshgl);
    }

    return result;
}
