// TODO: take entire transform (same used for draw) for wrapper_manifold--strip out incremental nature into function

enum class EnterMode {
    None,
    ExtrudeAdd,
    ExtrudeCut,
    NudgeFeaturePlane,
    Load,
    RevolveAdd,
    RevolveCut,
    Save,
    Size,
};

enum class ClickMode {
    None,
    Axis,
    BoundingBox,
    Circle,
    Color,
    Deselect,
    DivideNearest,
    Fillet,
    Line,
    LinearCopy,
    Measure,
    MirrorLine,
    MirrorX,
    MirrorY,
    Move,
    Origin,
    Polygon,
    Rotate,
    RotateCopy,
    Select,
    TwoEdgeCircle,
    TwoClickDivide,
};

enum class ClickModifier {
    None,
    Center,
    Color,
    Connected,
    End,
    Middle,
    Perpendicular,
    Quad,
    Selected,
    Window,
    XY,
};

enum class EntityType {
    Arc,
    Line,
};

enum class Pane {
    None,
    Drawing,
    Mesh,
    Popup,
    Separator,
};

enum class CellType {
    None,
    Real,
    String,
    Uint,
};

enum class EventType {
    None,
    Key,
    Mouse,
};

enum class KeyEventSubtype {
    None,
    Hotkey,
    Popup,
};

enum class MouseEventSubtype {
    None,
    Drawing,
    Mesh,
    Popup,
};

enum class ColorCode {
    Traverse,
    Quality1,
    Quality2,
    Quality3,
    Quality4,
    Quality5,
    Etch,
    Unknown,
    WaterOnly,
    LeadIO,
    QualitySlit1 = 21,
    QualitySlit2,
    QualitySlit3,
    QualitySlit4,
    QualitySlit5,
    Selection = 255,
};

/////////////////


////////////////////////////////////////
// structs /////////////////////////////
////////////////////////////////////////

struct LineEntity {
    vec2 start;
    vec2 end;
};

struct ArcEntity {
    vec2 center;
    real radius;
    real start_angle_in_degrees;
    real end_angle_in_degrees;
};

struct Entity {
    EntityType type;

    ColorCode color_code;
    bool is_selected;
    real time_since_is_selected_changed;

    LineEntity line;
    ArcEntity arc;
};

struct Mesh {
    uint num_vertices;
    uint num_triangles;
    vec3 *vertex_positions;
    uint3 *triangle_indices;
    vec3 *triangle_normals;

    // ??
    uint num_cosmetic_edges;
    uint2 *cosmetic_edges;

    bbox3 bbox;
};

struct RawKeyEvent {
    uint key;
    bool control;
    bool shift;
};

struct RawMouseEvent {
    Pane pane;
    vec2 mouse_Pixel;
    bool mouse_held;
};

struct RawEvent {
    EventType type;

    RawKeyEvent raw_key_event;
    RawMouseEvent raw_mouse_event;
};

struct MouseEventDrawing {
    vec2 mouse_position;
};

struct MouseEventMesh {
    vec3 mouse_ray_origin;
    vec3 mouse_ray_direction;
};

struct MouseEventPopup {
    uint cell_index;
    uint cursor;
};

struct MouseEvent {
    MouseEventSubtype subtype;

    bool mouse_held;

    MouseEventDrawing mouse_event_drawing;
    MouseEventMesh mouse_event_mesh;
    MouseEventPopup mouse_event_popup;
};

struct KeyEvent {
    KeyEventSubtype subtype;

    uint key;
    bool control;
    bool shift;
};

struct Event {
    EventType type;

    bool record_me;
    bool checkpoint_me;
    bool snapshot_me;

    KeyEvent key_event;
    MouseEvent mouse_event;
};

struct Drawing {
    List<Entity> entities;
    vec2 origin;
    vec2 axis_base_point;
    real axis_angle_from_y;
};

struct FeaturePlaneState {
    bool is_active;
    vec3 normal;
    real signed_distance_to_world_origin;
};

struct TwoClickCommandState {
    bool awaiting_second_click;
    vec2 first_click;
    
    Entity* stored_entity;
    uint entity_index;
};

#define POPUP_MAX_NUM_CELLS 5
#define POPUP_CELL_LENGTH 256
struct PopupState {
    _STRING_CALLOC(active_cell_buffer, POPUP_CELL_LENGTH);

    uint active_cell_index;
    uint cursor;
    uint selection_cursor;

    CellType cell_type[POPUP_MAX_NUM_CELLS];
    String name[POPUP_MAX_NUM_CELLS];
    void *value[POPUP_MAX_NUM_CELLS];
    uint num_cells;

    CellType _type_of_active_cell;
    char *_FORNOW_active_popup_unique_ID__FORNOW_name0;
    bool _popup_actually_called_this_event; // FORNOW

    bool _FORNOW_info_mouse_is_hovering;
    uint info_hover_cell_index;
    uint info_hover_cell_cursor;
    uint info_active_cell_cursor;

    real extrude_add_out_length;
    real extrude_add_in_length;
    real extrude_cut_in_length;
    real extrude_cut_out_length;
    real circle_diameter;
    real circle_radius;
    real circle_circumference;
    real fillet_radius;
    real box_width;
    real box_height;
    real x_coordinate;
    real y_coordinate;
    real feature_plane_nudge;
    real line_length;
    real line_angle;
    real line_run;
    real line_rise;
    real move_length;
    real move_angle;
    real move_run;
    real move_rise;
    uint polygon_num_sides = 5;
    real polygon_distance_to_side;
    real polygon_distance_to_corner;
    real polygon_side_length;
    real revolve_add_dummy;
    real revolve_cut_dummy;
    uint num_copies = 2;
    uint have_fields_been_edited = 0;
    real angle_of_rotation_in_degrees = 0;
    real angle_of_rotation_in_radians = 0;
    real scale_factor;
    _STRING_CALLOC(load_filename, POPUP_CELL_LENGTH);
    _STRING_CALLOC(save_filename, POPUP_CELL_LENGTH);
};

struct WorldState_ChangesToThisMustBeRecorded_state {
    Mesh mesh;
    Drawing drawing;
    FeaturePlaneState feature_plane;
    TwoClickCommandState two_click_command;
    PopupState popup;

    ClickMode click_mode;
    ClickModifier click_modifier;
    ColorCode click_color_code;
    EnterMode enter_mode;

    Event space_bar_event;
    Event shift_space_bar_event;
};

struct PreviewState {
    bbox2 feature_plane;
    real extrude_in_length;
    real extrude_out_length;
    vec3 tubes_color;
    vec3 feature_plane_color;
    real feature_plane_offset;
    vec2 drawing_origin;
    vec2 mouse;
};

struct ScreenState_ChangesToThisDo_NOT_NeedToBeRecorded_other {
    mat4 OpenGL_from_Pixel;
    mat4 transform_Identity = M4_Identity();

    Camera camera_drawing;
    Camera camera_mesh;

    bool hide_grid;
    bool show_details;
    bool show_help;
    bool show_event_stack;


    Pane hot_pane;
    real x_divider_OpenGL;
    Pane mouse_left_drag_pane;
    Pane mouse_right_drag_pane;

    bool shift_held;
    vec2 mouse_OpenGL;
    vec2 mouse_Pixel;

    bool _please_suppress_drawing_popup_popup;
    bool please_suppress_messagef;

    bool paused;
    bool stepping_one_frame_while_paused;

    real time_since_cursor_start;
    real time_since_successful_feature;
    real time_since_plane_selected;
    real time_since_going_inside;

    PreviewState preview;

};

struct StandardEventProcessResult {
    bool record_me;
    bool checkpoint_me;
    bool snapshot_me;
};

////////////////////////////////////////
// colors //////////////////////////////
////////////////////////////////////////

struct {
    vec3 red = RGB255(255, 0, 0);
    vec3 yellow = RGB255(255, 255, 0);
    vec3 orange = RGB255(204, 136, 1);
    vec3 green = RGB255(83, 255,  85);
    vec3 cyan = RGB255(0, 255, 255);
    vec3 blue = RGB255(0, 85, 255);
    vec3 purple = RGB255(170, 1, 255);
    vec3 magenta = RGB255(255, 0, 255);
    vec3 pink = RGB255(238, 0, 119);
    vec3 black = RGB255(0, 0, 0);
    vec3 dark_gray = RGB255(50, 50, 50);
    vec3 gray = RGB255(152, 152, 152);
    vec3 light_gray = RGB255(205, 205, 205);
    vec3 white = RGB255(255, 255, 255);
} omax;

vec3 omax_pallete[] = {
    omax.green,
    omax.red,
    omax.pink,
    omax.magenta,
    omax.purple,
    omax.blue,
    omax.light_gray,
    basic.magenta,
    omax.cyan,
    omax.orange,
};

////////////////////////////////////////
// Forward-Declarations ////////////////
////////////////////////////////////////

void messagef(vec3 color, char *format, ...);
template <typename T> void JUICEIT_EASYTWEEN(T *a, T b);

////////////////////////////////////////
// Config-Tweaks ///////////////////////
////////////////////////////////////////

real Z_FIGHT_EPS = 0.05f;
real TOLERANCE_DEFAULT = 5e-4f;
uint NUM_SEGMENTS_PER_CIRCLE = 64;
real GRID_SIDE_LENGTH = 250.0f;
real GRID_SPACING = 10.0f;
real CAMERA_3D_PERSPECTIVE_ANGLE_OF_VIEW = RAD(45.0f);

////////////////////////////////////////
// Cow Additions ///////////////////////
////////////////////////////////////////

real WRAP_TO_0_TAU_INTERVAL(real theta) {
    theta = fmod(theta, TAU);
    if (theta < 0.0) theta += TAU;
    return theta;
}

bool ANGLE_IS_BETWEEN_CCW(real t, real a, real b) {
    return (WRAP_TO_0_TAU_INTERVAL(t - a) < WRAP_TO_0_TAU_INTERVAL(t - b));
}

bool ANGLE_IS_BETWEEN_CCW_DEGREES(real t, real a, real b) {
    return (WRAP_TO_0_TAU_INTERVAL(RAD(t) - RAD(a)) < WRAP_TO_0_TAU_INTERVAL(RAD(t) - RAD(b)));
}

////////////////////////////////////////
// List<Entity> /////////////////////////////////
////////////////////////////////////////

vec2 get_point_on_circle_NOTE_pass_angle_in_radians(vec2 center, real radius, real angle_in_radians) {
    return center + radius * V2(COS(angle_in_radians), SIN(angle_in_radians));
}

// NOTE: this is real gross
void arc_process_angles_into_lerpable_radians_considering_flip_flag(ArcEntity *arc, real *start_angle, real *end_angle, bool flip_flag) {
    // The way the List<Entity> spec works is that start_angle and end_angle define points on the circle
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
        real tmp = *start_angle;
        *start_angle = *end_angle;
        *end_angle = tmp;
        // start -cw-> end
        while (*end_angle > *start_angle) *start_angle += TAU;
    }
}

real entity_length(Entity *entity) {
    if (entity->type == EntityType::Line) {
        LineEntity *line = &entity->line;
        return norm(line->start - line->end);
    } else { ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc = &entity->arc;
        real start_angle;
        real end_angle;
        arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, false);
        return ABS(start_angle - end_angle) * arc->radius;
    }
}

vec2 entity_get_start_point(Entity *entity) {
    if (entity->type == EntityType::Line) {
        LineEntity *line = &entity->line;
        return line->start;
    } else { ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc = &entity->arc;
        return get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, RAD(arc->start_angle_in_degrees));
    }
}

vec2 entity_get_end_point(Entity *entity) {
    if (entity->type == EntityType::Line) {
        LineEntity *line = &entity->line;
        return line->end;
    } else { ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc = &entity->arc;
        return get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, RAD(arc->end_angle_in_degrees));
    }
}

void entity_get_start_and_end_points(Entity *entity, vec2 *start, vec2 *end) {
    *start = entity_get_start_point(entity);
    *end = entity_get_end_point(entity);
}

vec2 entity_lerp_considering_flip_flag(Entity *entity, real t, bool flip_flag) {
    ASSERT(IS_BETWEEN_LOOSE(t, 0.0f, 1.0f));
    if (entity->type == EntityType::Line) {
        LineEntity *line = &entity->line;
        if (flip_flag) t = 1.0f - t; // FORNOW
        return LERP(t, line->start, line->end);
    } else {
        ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc = &entity->arc;
        real angle; {
            real start_angle, end_angle;
            arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, flip_flag); // FORNOW
            angle = LERP(t, start_angle, end_angle);
        }
        return get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, angle);
    }
}

vec2 entity_get_middle(Entity *entity) {
    return entity_lerp_considering_flip_flag(entity, 0.5f, false);
}

// struct List<Entity> {
//     uint num_entities;
//     Entity *entities;
// };

void entities_load(String filename, List<Entity> *entities) {
    #if 0
    {
        FORNOW_UNUSED(filename);
        return {};
        #elif 0
        List<Entity> result = {};
        result.num_entities = 8;
        result.entities = (Entity *) calloc(result.num_entities, sizeof(Entity));
        result.entities[0] = { EntityType::Line, 0, 0.0, 0.0, 1.0, 0.0 };
        result.entities[1] = { EntityType::Line, 1, 1.0, 0.0, 1.0, 1.0 };
        result.entities[2] = { EntityType::Line, 2, 0.0, 1.0, 0.0, 0.0 };
        result.entities[3] = { EntityType::Arc,  3, 0.5, 1.0, 0.5,    0.0, 180.0 };
        result.entities[4] = { EntityType::Arc,  4, 0.5, 1.0, 0.25,   0.0, 180.0 };
        result.entities[5] = { EntityType::Arc,  5, 0.5, 1.0, 0.25, 180.0, 360.0 };
        result.entities[6] = { EntityType::Arc,  6, 0.5, 1.0, 0.1,    0.0, 180.0 };
        result.entities[7] = { EntityType::Arc,  7, 0.5, 1.0, 0.1,  180.0, 360.0 };
        return result;
    }
    #endif
    list_free_AND_zero(entities);

    FILE *file = (FILE *) FILE_OPEN(filename, "r");
    ASSERT(file);

    *entities = {}; {
        #define PARSE_NONE 0
        #define PARSE_LINE 1
        #define PARSE_ARC  2
        uint mode = 0;
        int code = 0;
        bool code_is_hot = false;
        Entity entity = {};
        #define MAX_LINE_LENGTH 1024
        static _STRING_CALLOC(line_from_file, MAX_LINE_LENGTH);
        while (string_read_line_from_file(&line_from_file, MAX_LINE_LENGTH, file)) {
            if (mode == PARSE_NONE) {
                if (string_matches_prefix(line_from_file, STRING("LINE"))) {
                    mode = PARSE_LINE;
                    code_is_hot = false;
                    entity = {};
                    entity.type = EntityType::Line;
                } else if (string_matches_prefix(line_from_file, STRING("ARC"))) {
                    mode = PARSE_ARC;
                    code_is_hot = false;
                    entity = {};
                    entity.type = EntityType::Arc;
                }
            } else {
                if (!code_is_hot) {
                    sscanf(line_from_file.data, "%d", &code);
                    // NOTE this initialization is sketchy but works
                    // probably don't make a habit of it
                    if (code == 0) {
                        list_push_back(entities, entity);
                        mode = PARSE_NONE;
                        code_is_hot = false;
                    }
                } else {
                    if (code == 62) {
                        int value;
                        sscanf(line_from_file.data, "%d", &value);
                        entity.color_code = (ColorCode) value; 
                    } else {
                        float value;
                        sscanf(line_from_file.data, "%f", &value);
                        if (mode == PARSE_LINE) {
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
                            ASSERT(mode == PARSE_ARC);
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

vec3 get_color(ColorCode color_code) {
    uint i = uint(color_code);
    if (0 <= i && i <= 9) {
        return omax_pallete[i];
    } else if (20 <= i && i <= 29) {
        do_once { messagef(omax.orange, "WARNING: slits not implemented"); };
        return omax_pallete[i - 20];
    } else if (color_code == ColorCode::Selection) {
        return omax.yellow;
    } else {
        ASSERT(false);
        return {};
    }
}

void eso_entity__SOUP_LINES(Entity *entity) {
    if (entity->type == EntityType::Line) {
        LineEntity *line = &entity->line;
        eso_vertex(line->start);
        eso_vertex(line->end);
    } else {
        ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc = &entity->arc;
        real start_angle, end_angle;
        arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, false);
        real delta_angle = end_angle - start_angle;
        uint num_segments = uint(1 + (delta_angle / TAU) * 256); // FORNOW: TODO: make dependent on zoom
        real increment = delta_angle / num_segments;
        real current_angle = start_angle;
        for_(i, num_segments) {
            eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, current_angle));
            current_angle += increment;
            eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, current_angle));
        }
    }
}


void entities_debug_draw(Camera *camera_drawing, List<Entity> *entities) {
    eso_begin(camera_drawing->get_PV(), SOUP_LINES);
    for (Entity *entity = entities->array; entity < &entities->array[entities->length]; ++entity) {
        eso_entity__SOUP_LINES(entity);
    }
    eso_end();
}

bbox2 entity_get_bbox(Entity *entity) {
    bbox2 result = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
    vec2 s[2];
    uint n = 2;
    entity_get_start_and_end_points(entity, &s[0], &s[1]);
    for_(i, n) {
        for_(d, 2) {
            result.min[d] = MIN(result.min[d], s[i][d]);
            result.max[d] = MAX(result.max[d], s[i][d]);
        }
    }
    if (entity->type == EntityType::Arc) {
        ArcEntity *arc = &entity->arc;
        // NOTE: endpoints already taken are of; we just have to deal with the quads (if they exist)
        // TODO: angle_is_between_counter_clockwise (TODO TODO TODO)
        real start_angle = RAD(arc->start_angle_in_degrees);
        real end_angle = RAD(arc->end_angle_in_degrees);
        if (ANGLE_IS_BETWEEN_CCW(RAD(  0.0f), start_angle, end_angle)) result.max[0] = MAX(result.max[0], arc->center.x + arc->radius);
        if (ANGLE_IS_BETWEEN_CCW(RAD( 90.0f), start_angle, end_angle)) result.max[1] = MAX(result.max[1], arc->center.y + arc->radius);
        if (ANGLE_IS_BETWEEN_CCW(RAD(180.0f), start_angle, end_angle)) result.min[0] = MIN(result.min[0], arc->center.x - arc->radius);
        if (ANGLE_IS_BETWEEN_CCW(RAD(270.0f), start_angle, end_angle)) result.min[1] = MIN(result.min[1], arc->center.y - arc->radius);
    }
    return result;
}

bbox2 entities_get_bbox(List<Entity> *entities, bool only_consider_selected_entities = false) {
    bbox2 result = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
    for_(i, entities->length) {
        if ((only_consider_selected_entities) && (!entities->array[i].is_selected)) continue;
        bbox2 bbox = entity_get_bbox(&entities->array[i]);
        result += bbox;
    }
    return result;
}

////////////////////////////////////////
// SquaredDistance /////////////////////
////////////////////////////////////////

real squared_distance_point_line_segment(vec2 p, vec2 start, vec2 end) {
    real l2 = squaredDistance(start, end);
    if (l2 < TINY_VAL) return squaredDistance(p, start);
    real num = dot(p - start, end - start);
    vec2 q = CLAMPED_LERP(num / l2, start, end);//
    return squaredDistance(p, q);
}

real squared_distance_point_circle(vec2 p, vec2 center, real radius) {
    return POW(distance(p, center) - radius, 2);
}

real squared_distance_point_arc_NOTE_pass_angles_in_radians(vec2 p, vec2 center, real radius, real start_angle_in_radians, real end_angle_in_radians) {
    bool point_in_sector = false; {
        vec2 v = p - center;
        // forgive me rygorous :(
        real angle = ATAN2(v);
        while (start_angle_in_radians < -PI) start_angle_in_radians += TAU;
        while (end_angle_in_radians < start_angle_in_radians) end_angle_in_radians += TAU;
        point_in_sector =
            IS_BETWEEN_LOOSE(angle, start_angle_in_radians, end_angle_in_radians)
            || IS_BETWEEN_LOOSE(angle + TAU, start_angle_in_radians, end_angle_in_radians)
            || IS_BETWEEN_LOOSE(angle - TAU, start_angle_in_radians, end_angle_in_radians);
    }
    if (point_in_sector) {
        return squared_distance_point_circle(p, center, radius);
    } else {
        vec2 start = get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, start_angle_in_radians);
        vec2 end = get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, end_angle_in_radians);
        return MIN(squaredDistance(p, start), squaredDistance(p, end));
    }
}

real squared_distance_point_dxf_line_entity(vec2 p, LineEntity *line) {
    return squared_distance_point_line_segment(p, line->start, line->end);
}

real squared_distance_point_dxf_arc_entity(vec2 p, ArcEntity *arc) {
    return squared_distance_point_arc_NOTE_pass_angles_in_radians(p, arc->center, arc->radius, RAD(arc->start_angle_in_degrees), RAD(arc->end_angle_in_degrees));
}

real squared_distance_point_entity(vec2 p, Entity *entity) {
    if (entity->type == EntityType::Line) {
        LineEntity *line = &entity->line;
        return squared_distance_point_dxf_line_entity(p, line);
    } else {
        ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc = &entity->arc;
        return squared_distance_point_dxf_arc_entity(p, arc);
    }
}

real squared_distance_point_dxf(vec2 p, List<Entity> *entities) {
    real result = HUGE_VAL;
    for (Entity *entity = entities->array; entity < &entities->array[entities->length]; ++entity) {
        result = MIN(result, squared_distance_point_entity(p, entity));
    }
    return result;
}

struct DXFFindClosestEntityResult {
    bool success;
    uint index;
    Entity *closest_entity;
    vec2 line_nearest_point;
    real arc_nearest_angle_in_degrees;
};
DXFFindClosestEntityResult dxf_find_closest_entity(List<Entity> *entities, vec2 p) {
    DXFFindClosestEntityResult result = {};
    double hot_squared_distance = HUGE_VAL;
    for_(i, entities->length) {
        Entity *entity = &entities->array[i];
        double squared_distance = squared_distance_point_entity(p, entity);
        if (squared_distance < hot_squared_distance) {
            hot_squared_distance = squared_distance;
            result.success = true;
            result.index = i;
            result.closest_entity = &entities->array[i];
            if (result.closest_entity->type == EntityType::Line) {
                LineEntity *line = &result.closest_entity->line;
                real l2 = squaredDistance(line->start, line->end);
                if (l2 < TINY_VAL) {
                    result.line_nearest_point = line->start;
                } else {
                    real num = dot(p - line->start, line->end - line->start);
                    result.line_nearest_point = CLAMPED_LERP(num / l2, line->start, line->end);
                }
            } else { ASSERT(result.closest_entity->type == EntityType::Arc);
                ArcEntity *arc = &result.closest_entity->arc;
                result.arc_nearest_angle_in_degrees = DEG(ATAN2(p - arc->center));
            }
        }
    }
    return result;
}


////////////////////////////////////////
// LoopAnalysis ////////////////////////
////////////////////////////////////////

struct DXFEntityIndexAndFlipFlag {
    uint entity_index;
    bool flip_flag;
};

struct DXFLoopAnalysisResult {
    uint num_loops;
    uint *num_entities_in_loops;
    DXFEntityIndexAndFlipFlag **loops;
    uint *loop_index_from_entity_index;
};

DXFLoopAnalysisResult dxf_loop_analysis_create_FORNOW_QUADRATIC(List<Entity> *entities, bool only_consider_selected_entities) {
    if (entities->length == 0) {
        DXFLoopAnalysisResult result = {};
        result.num_loops = 0;
        result.num_entities_in_loops = (uint *) calloc(result.num_loops, sizeof(uint));
        result.loops = (DXFEntityIndexAndFlipFlag **) calloc(result.num_loops, sizeof(DXFEntityIndexAndFlipFlag *));
        result.loop_index_from_entity_index = (uint *) calloc(entities->length, sizeof(uint));
        return result;
    }

    DXFLoopAnalysisResult result = {};
    { // num_entities_in_loops, loops
      // populate List's
        List<List<DXFEntityIndexAndFlipFlag>> stretchy_list = {}; {
            bool *entity_already_added = (bool *) calloc(entities->length, sizeof(bool));
            while (true) {
                #define MACRO_CANDIDATE_VALID(i) (!entity_already_added[i] && (!only_consider_selected_entities || entities->array[i].is_selected))
                { // seed loop
                    bool added_and_seeded_new_loop = false;
                    for_(entity_index, entities->length) {
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
                    real tolerance = TOLERANCE_DEFAULT;
                    while (true) {
                        bool added_new_entity_to_loop = false;
                        for_(entity_index, entities->length) {
                            if (!MACRO_CANDIDATE_VALID(entity_index)) continue;
                            vec2 start_prev;
                            vec2 end_prev;
                            vec2 start_i;
                            vec2 end_i;
                            DXFEntityIndexAndFlipFlag *prev_entity_index_and_flip_flag = &(stretchy_list.array[stretchy_list.length - 1].array[stretchy_list.array[stretchy_list.length - 1].length - 1]);
                            {
                                entity_get_start_and_end_points(&entities->array[prev_entity_index_and_flip_flag->entity_index], &start_prev, &end_prev);
                                entity_get_start_and_end_points(&entities->array[entity_index], &start_i, &end_i);
                            }
                            bool is_next_entity = false;
                            bool flip_flag = false;
                            if (!prev_entity_index_and_flip_flag->flip_flag) {
                                if (squaredDistance(end_prev, start_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = false;
                                } else if (squaredDistance(end_prev, end_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = true;
                                }
                            } else {
                                if (squaredDistance(start_prev, start_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = false;
                                } else if (squaredDistance(start_prev, end_i) < tolerance) {
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
                    uint num_entities_in_loop = stretchy_list.array[stretchy_list.length - 1].length;
                    DXFEntityIndexAndFlipFlag *loop = stretchy_list.array[stretchy_list.length - 1].array;
                    bool reverse_loop; {
                        #if 0
                        reverse_loop = false;
                        #else
                        real twice_the_signed_area; {
                            twice_the_signed_area = 0.0f;
                            for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities_in_loop; ++entity_index_and_flip_flag) {
                                uint entity_index = entity_index_and_flip_flag->entity_index;
                                bool flip_flag = entity_index_and_flip_flag->flip_flag;
                                Entity *entity = &entities->array[entity_index];
                                if (entity->type == EntityType::Line) {
                                    LineEntity *line = &entity->line;
                                    // shoelace-type formula
                                    twice_the_signed_area += ((flip_flag) ? -1 : 1) * (line->start.x * line->end.y - line->end.x * line->start.y);
                                } else {
                                    ASSERT(entity->type == EntityType::Arc);
                                    ArcEntity *arc = &entity->arc;
                                    // "Circular approximation using polygons"
                                    // - n = 2 (area-preserving approximation of arc with two segments)
                                    real start_angle, end_angle;
                                    arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, flip_flag);
                                    vec2 start = get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, start_angle);
                                    vec2 end = get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, end_angle);
                                    real mid_angle = (start_angle + end_angle) / 2;
                                    real d; {
                                        real alpha = ABS(start_angle - end_angle) / 2;
                                        d = arc->radius * alpha / SIN(alpha);
                                    }
                                    vec2 mid = get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, d, mid_angle);
                                    twice_the_signed_area += mid.x * (end.y - start.y) + mid.y * (start.x - end.x); // TODO cross(...)
                                }
                            }
                        }
                        reverse_loop = (twice_the_signed_area < 0.0f);
                        #endif
                    }
                    if (reverse_loop) {
                        for (uint i = 0, j = (num_entities_in_loop - 1); i < j; ++i, --j) {
                            DXFEntityIndexAndFlipFlag tmp = loop[i];
                            loop[i] = loop[j];
                            loop[j] = tmp;
                        }
                        for_(i, num_entities_in_loop) {
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
        result.num_entities_in_loops = (uint *) calloc(result.num_loops, sizeof(uint));
        result.loops = (DXFEntityIndexAndFlipFlag **) calloc(result.num_loops, sizeof(DXFEntityIndexAndFlipFlag *));
        for_(i, result.num_loops) {
            result.num_entities_in_loops[i] = stretchy_list.array[i].length;
            result.loops[i] = (DXFEntityIndexAndFlipFlag *) calloc(result.num_entities_in_loops[i], sizeof(DXFEntityIndexAndFlipFlag));
            memcpy(result.loops[i], stretchy_list.array[i].array, result.num_entities_in_loops[i] * sizeof(DXFEntityIndexAndFlipFlag));
        }

        // free List's
        for_(i, stretchy_list.length) list_free_AND_zero(&stretchy_list.array[i]);
        list_free_AND_zero(&stretchy_list);
    }
    // loop_index_from_entity_index (brute force)
    result.loop_index_from_entity_index = (uint *) calloc(entities->length, sizeof(uint));
    for_(i, entities->length) {
        for_(j, result.num_loops) {
            for_(k, result.num_entities_in_loops[j]) {
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
        for_(i, analysis->num_loops) {
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
    uint num_polygonal_loops;
    uint *num_vertices_in_polygonal_loops;
    vec2 **polygonal_loops;
};

CrossSectionEvenOdd cross_section_create_FORNOW_QUADRATIC(List<Entity> *entities, bool only_consider_selected_entities) {
    #if 0
    {
        FORNOW_UNUSED(entities);
        FORNOW_UNUSED(include);
        CrossSectionEvenOdd result = {};
        result.num_polygonal_loops = 2;
        result.num_vertices_in_polygonal_loops = (uint *) calloc(result.num_polygonal_loops, sizeof(uint));
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
        DXFLoopAnalysisResult analysis = dxf_loop_analysis_create_FORNOW_QUADRATIC(entities, only_consider_selected_entities);
        for_(loop_index, analysis.num_loops) {
            uint num_entities_in_loop = analysis.num_entities_in_loops[loop_index];
            DXFEntityIndexAndFlipFlag *loop = analysis.loops[loop_index];
            list_push_back(&stretchy_list, {});
            for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities_in_loop; ++entity_index_and_flip_flag) {
                uint entity_index = entity_index_and_flip_flag->entity_index;
                bool flip_flag = entity_index_and_flip_flag->flip_flag;
                Entity *entity = &entities->array[entity_index];
                if (entity->type == EntityType::Line) {
                    LineEntity *line = &entity->line;
                    if (!flip_flag) {
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], { line->start.x, line->start.y });
                    } else {
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], { line->end.x, line->end.y });
                    }
                } else {
                    ASSERT(entity->type == EntityType::Arc);
                    ArcEntity *arc = &entity->arc;
                    real start_angle, end_angle;
                    arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, flip_flag);
                    real delta_angle = end_angle - start_angle;
                    uint num_segments = uint(2 + ABS(delta_angle) * (NUM_SEGMENTS_PER_CIRCLE / TAU)); // FORNOW (2 + ...)
                    real increment = delta_angle / num_segments;
                    real current_angle = start_angle;
                    vec2 p;
                    for_(i, num_segments) {
                        p = get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, current_angle);
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], p);
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
    result.num_vertices_in_polygonal_loops = (uint *) calloc(result.num_polygonal_loops, sizeof(uint));
    result.polygonal_loops = (vec2 **) calloc(result.num_polygonal_loops, sizeof(vec2 *));
    for_(i, result.num_polygonal_loops) {
        result.num_vertices_in_polygonal_loops[i] = stretchy_list.array[i].length;
        result.polygonal_loops[i] = (vec2 *) calloc(result.num_vertices_in_polygonal_loops[i], sizeof(vec2));
        memcpy(result.polygonal_loops[i], stretchy_list.array[i].array, result.num_vertices_in_polygonal_loops[i] * sizeof(vec2));
    }

    // free List's
    for_(i, stretchy_list.length) list_free_AND_zero(&stretchy_list.array[i]);
    list_free_AND_zero(&stretchy_list);

    return result;
}

void cross_section_debug_draw(Camera *camera_drawing, CrossSectionEvenOdd *cross_section) {
    eso_begin(camera_drawing->get_PV(), SOUP_LINES);
    eso_color(omax.white);
    for_(loop_index, cross_section->num_polygonal_loops) {
        vec2 *polygonal_loop = cross_section->polygonal_loops[loop_index];
        int n = cross_section->num_vertices_in_polygonal_loops[loop_index];
        for (int j = 0, i = n - 1; j < n; i = j++) {
            real a_x = polygonal_loop[i].x;
            real a_y = polygonal_loop[i].y;
            real b_x = polygonal_loop[j].x;
            real b_y = polygonal_loop[j].y;
            eso_color(color_rainbow_swirl(real(i) / (n)));
            eso_vertex(a_x, a_y);
            eso_color(color_rainbow_swirl(real(i + 1) / (n)));
            eso_vertex(b_x, b_y);

            // draw normal
            real c_x = (a_x + b_x) / 2;
            real c_y = (a_y + b_y) / 2;
            real n_x = b_y - a_y;
            real n_y = a_x - b_x;
            real norm_n = SQRT(n_x * n_x + n_y * n_y);
            real L = 0.013f;
            eso_color(color_rainbow_swirl((i + 0.5f) / (n)));
            eso_vertex(c_x, c_y);
            eso_vertex(c_x + L * n_x / norm_n, c_y + L * n_y / norm_n);
        }
    }
    eso_end();
}

void cross_section_free(CrossSectionEvenOdd *cross_section) {
    free(cross_section->num_vertices_in_polygonal_loops);
    for_(i, cross_section->num_polygonal_loops) free(cross_section->polygonal_loops[i]);
    free(cross_section->polygonal_loops);
}

////////////////////////////////////////
// Mesh, Mesh //////////////////////
////////////////////////////////////////


void mesh_triangle_normals_calculate(Mesh *mesh) {
    mesh->triangle_normals = (vec3 *) malloc(mesh->num_triangles * sizeof(vec3));
    vec3 p[3];
    for_(i, mesh->num_triangles) {
        for_(d, 3) p[d] = mesh->vertex_positions[mesh->triangle_indices[i][d]];
        vec3 n = normalized(cross(p[1] - p[0], p[2] - p[0]));
        mesh->triangle_normals[i] = n;
    }
}

void mesh_cosmetic_edges_calculate(Mesh *mesh) {
    // approach: prep a big array that maps edge -> cwiseProduct of face normals (start it at 1, 1, 1) // (faces that edge is part of)
    //           iterate through all edges detministically (ccw in order, flipping as needed so lower_index->higher_index)
    //           then go back and if passes some heuristic add that index to a stretchy buffer
    List<uint2> list = {}; {
        Map<uint2, vec3> map = {}; {
            for_(i, mesh->num_triangles) {
                vec3 n = mesh->triangle_normals[i];
                for (uint jj0 = 0, jj1 = (3 - 1); jj0 < 3; jj1 = jj0++) {
                    uint j0 = mesh->triangle_indices[i][jj0];
                    uint j1 = mesh->triangle_indices[i][jj1];
                    if (j0 > j1) {
                        uint tmp = j0;
                        j0 = j1;
                        j1 = tmp;
                    }
                    uint2 key = { j0, j1 };
                    map_put(&map, key, cwiseProduct(n, map_get(&map, key, V3(1.0f))));
                }
            }
        }
        {
            for (List<Pair<uint2, vec3>> *bucket = map.buckets; bucket < &map.buckets[map.num_buckets]; ++bucket) {
                for (Pair<uint2, vec3> *pair = bucket->array; pair < &bucket->array[bucket->length]; ++pair) {
                    vec3 n2 = pair->value;
                    // pprint(n2);
                    real angle = DEG(acos(n2.x + n2.y + n2.z)); // [0.0f, 180.0f]
                    if (angle > 30.0f) {
                        list_push_back(&list, pair->key); // FORNOW
                    }
                }
            }
        }
        map_free_and_zero(&map);
    }
    {
        mesh->num_cosmetic_edges = list.length;
        mesh->cosmetic_edges = (uint2 *) calloc(mesh->num_cosmetic_edges, sizeof(uint2));
        memcpy(mesh->cosmetic_edges, list.array, mesh->num_cosmetic_edges * sizeof(uint2)); 
    }
    list_free_AND_zero(&list);
}

void mesh_bbox_calculate(Mesh *mesh) {
    mesh->bbox = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<3>();
    for_(i, mesh->num_vertices) {
        mesh->bbox += mesh->vertex_positions[i];
    }
}

bool mesh_save_stl(Mesh *mesh, String filename) {
    FILE *file = FILE_OPEN(filename, "wb");
    if (!file) {
        return false;
    }

    int num_bytes = 80 + 4 + 50 * mesh->num_triangles;
    char *buffer = (char *) calloc(num_bytes, 1); {
        int offset = 80;
        memcpy(buffer + offset, &mesh->num_triangles, 4);
        offset += 4;
        for_(i, mesh->num_triangles) {
            vec3 triangle_normal; {
                triangle_normal = mesh->triangle_normals[i];
                // 90 degree rotation about x: (x, y, z) <- (x, -z, y)
                triangle_normal = { triangle_normal.x, -triangle_normal.z, triangle_normal.y };
            }
            memcpy(buffer + offset, &triangle_normal, 12);
            offset += 12;
            vec3 triangle_vertex_positions[3];
            for_(j, 3) {
                triangle_vertex_positions[j] = mesh->vertex_positions[mesh->triangle_indices[i][j]];
                // 90 degree rotation about x: (x, y, z) <- (x, -z, y)
                triangle_vertex_positions[j] = { triangle_vertex_positions[j].x, -triangle_vertex_positions[j].z, triangle_vertex_positions[j].y };

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
        uint size = src->num_vertices * sizeof(vec3);
        dst->vertex_positions = (vec3 *) malloc(size);
        memcpy(dst->vertex_positions, src->vertex_positions, size);
    }
    if (src->triangle_indices) {
        uint size = src->num_triangles * sizeof(uint3);
        dst->triangle_indices = (uint3 *) malloc(size);
        memcpy(dst->triangle_indices, src->triangle_indices, size);
    }
    if (src->triangle_normals) {
        uint size = src->num_triangles * sizeof(vec3); 
        dst->triangle_normals = (vec3 *) malloc(size);
        memcpy(dst->triangle_normals, src->triangle_normals, size);
    }
    if (src->cosmetic_edges) {
        uint size = src->num_cosmetic_edges * sizeof(uint2);
        dst->cosmetic_edges = (uint2 *) malloc(size);
        memcpy(dst->cosmetic_edges, src->cosmetic_edges, size);
    }
}

void stl_load(String filename, Mesh *mesh) {
    // history_record_state(history, manifold_manifold, mesh); // FORNOW

    { // mesh
        uint num_triangles;
        vec3 *triangle_soup;
        {
            #define MAX_LINE_LENGTH 1024
            static _STRING_CALLOC(line_of_file, MAX_LINE_LENGTH);

            #define STL_FILETYPE_UNKNOWN 0
            #define STL_FILETYPE_ASCII   1
            #define STL_FILETYPE_BINARY  2
            uint filetype; {
                FILE *file = FILE_OPEN(filename, "r");
                string_read_line_from_file(&line_of_file, 80, file);
                filetype = (string_matches_prefix(line_of_file, STRING("solid"))) ? STL_FILETYPE_ASCII : STL_FILETYPE_BINARY;
                fclose(file);
            }

            if (filetype == STL_FILETYPE_ASCII) {
                char ascii_scan_dummy[64];
                real ascii_scan_p[3];
                List<real> ascii_data = {};

                FILE *file = FILE_OPEN(filename, "r");
                while (string_read_line_from_file(&line_of_file, MAX_LINE_LENGTH, file)) {
                    if (string_matches_prefix(line_of_file, STRING("vertex"))) {
                        sscanf(line_of_file.data, "%s %f %f %f", ascii_scan_dummy, &ascii_scan_p[0], &ascii_scan_p[1], &ascii_scan_p[2]);
                        for_(d, 3) list_push_back(&ascii_data, ascii_scan_p[d]);
                    }
                }
                fclose(file);
                num_triangles = ascii_data.length / 9;
                uint size = ascii_data.length * sizeof(real);
                triangle_soup = (vec3 *) malloc(size);
                memcpy(triangle_soup, ascii_data.array, size);
                list_free_AND_zero(&ascii_data);
            } else {
                ASSERT(filetype == STL_FILETYPE_BINARY);
                char *entire_file; {
                    FILE *file = FILE_OPEN(filename, "rb");
                    fseek(file, 0, SEEK_END);
                    long fsize = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    entire_file = (char *) malloc(fsize + 1);
                    fread(entire_file, fsize, 1, file);
                    fclose(file);
                    entire_file[fsize] = 0;
                }
                uint offset = 80;
                memcpy(&num_triangles, entire_file + offset, 4);
                offset += 4;
                uint size = num_triangles * 36;
                triangle_soup = (vec3 *) calloc(1, size);
                for_(i, num_triangles) {
                    offset += 12;
                    memcpy(&triangle_soup[3 * i], entire_file + offset, 36);
                    offset += 38;
                }
            }
            { // -90 degree rotation about x: (x, y, z) <- (x, z, -y)
                uint num_vertices = 3 * num_triangles;
                for_(i, num_vertices) {
                    triangle_soup[i] = { triangle_soup[i].x, triangle_soup[i].z, -triangle_soup[i].y };
                }
            }
        }

        uint num_vertices;
        vec3 *vertex_positions;
        uint3 *triangle_indices;
        { // merge vertices (NOTE: only merges vertices that overlap exactly)
            num_vertices = 0;
            Map<vec3, uint> map = {};
            uint _3__times__num_triangles = 3 * num_triangles;
            uint default_value = _3__times__num_triangles + 1;
            {
                List<vec3> list = {};
                for_(i, _3__times__num_triangles) {
                    vec3 p = triangle_soup[i];
                    uint j = map_get(&map, p, default_value);
                    if (j == default_value) {
                        map_put(&map, p, num_vertices++);
                        list_push_back(&list, p);
                    }
                }
                {
                    uint size = list.length * sizeof(vec3);
                    vertex_positions = (vec3 *) malloc(size);
                    memcpy(vertex_positions, list.array, size);
                }
                list_free_AND_zero(&list);
            }
            triangle_indices = (uint3 *) malloc(num_triangles * sizeof(uint3));
            for_(k, _3__times__num_triangles) triangle_indices[k / 3][k % 3] = map_get(&map, triangle_soup[k]);
            map_free_and_zero(&map);
        }

        free(triangle_soup);

        mesh->num_vertices = num_vertices;
        mesh->num_triangles = num_triangles;
        mesh->vertex_positions = vertex_positions;
        mesh->triangle_indices = triangle_indices;
        mesh_triangle_normals_calculate(mesh);
        mesh_cosmetic_edges_calculate(mesh);
        mesh_bbox_calculate(mesh);
    }
}



////////////////////////////////////////////////////////////////////////////////
// key_lambda //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool _key_lambda(KeyEvent *key_event, uint key, bool control = false, bool shift = false) {
    ASSERT(!(('a' <= key) && (key <= 'z')));
    bool key_match = (key_event->key == key);
    bool super_match = ((key_event->control && control) || (!key_event->control && !control)); // * bool
    bool shift_match = ((key_event->shift && shift) || (!key_event->shift && !shift)); // * bool
    return (key_match && super_match && shift_match);
};

////////////////////////////////////////////////////////////////////////////////
// world_state /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void world_state_deep_copy(WorldState_ChangesToThisMustBeRecorded_state *dst, WorldState_ChangesToThisMustBeRecorded_state *src) {
    *dst = *src;
    dst->drawing.entities = {};
    list_clone(&dst->drawing.entities,    &src->drawing.entities   );
    mesh_deep_copy(&dst->mesh, &src->mesh);
}

void world_state_free_AND_zero(WorldState_ChangesToThisMustBeRecorded_state *world_state) {
    mesh_free_AND_zero(&world_state->mesh);
    list_free_AND_zero(&world_state->drawing.entities);
}

////////////////////////////////////////////////////////////////////////////////
// uh oh ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// TODO: don't overwrite fancy mesh, let the calling code do what it will
// TODO: could this take a printf function pointer?
Mesh wrapper_manifold(
        Mesh *mesh, // dest__NOTE_GETS_OVERWRITTEN,
        uint num_polygonal_loops,
        uint *num_vertices_in_polygonal_loops,
        vec2 **polygonal_loops,
        mat4 M_3D_from_2D,
        EnterMode enter_mode,
        real extrude_out_length,
        real extrude_in_length,
        vec2   dxf_origin,
        vec2   dxf_axis_base_point,
        real dxf_axis_angle_from_y
        ) {


    bool add = (enter_mode == EnterMode::ExtrudeAdd) || (enter_mode == EnterMode::RevolveAdd);
    bool cut = (enter_mode == EnterMode::ExtrudeCut) || (enter_mode == EnterMode::RevolveCut);
    bool extrude = (enter_mode == EnterMode::ExtrudeAdd) || (enter_mode == EnterMode::ExtrudeCut);
    bool revolve = (enter_mode == EnterMode::RevolveAdd) || (enter_mode == EnterMode::RevolveCut);
    ASSERT(add || cut);
    ASSERT(extrude || revolve);

    ManifoldManifold *manifold_A; {
        if (mesh->num_vertices == 0) {
            manifold_A = NULL;
        } else { // manifold <- mesh
            ManifoldMeshGL *meshgl = manifold_meshgl(
                    malloc(manifold_meshgl_size()),
                    (real *) mesh->vertex_positions,
                    mesh->num_vertices,
                    3,
                    (uint *) mesh->triangle_indices,
                    mesh->num_triangles);

            manifold_A = manifold_of_meshgl(malloc(manifold_manifold_size()), meshgl);

            manifold_delete_meshgl(meshgl);
        }
    }

    ManifoldManifold *manifold_B; {
        ManifoldSimplePolygon **simple_polygon_array; {
            simple_polygon_array = (ManifoldSimplePolygon **) malloc(num_polygonal_loops * sizeof(ManifoldSimplePolygon *));
            for_(i, num_polygonal_loops) {
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
            if (enter_mode == EnterMode::ExtrudeCut) {
                do_once { messagef(omax.pink, "FORNOW ExtrudeCut: Inflating as naive solution to avoid thin geometry."); };
                extrude_in_length += SGN(extrude_in_length) * TOLERANCE_DEFAULT;
                extrude_out_length += SGN(extrude_out_length) * TOLERANCE_DEFAULT;
            }

            // NOTE: params are arbitrary sign (and can be same sign)--a typical thing would be like (30, -30)
            //       but we support (30, 40) -- which is equivalent to (40, 0)

            if (extrude) {
                real length = extrude_in_length + extrude_out_length;
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
            for_(i, num_polygonal_loops) manifold_delete_simple_polygon(simple_polygon_array[i]);
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
            result.vertex_positions = (vec3 *) manifold_meshgl_vert_properties(malloc(manifold_meshgl_vert_properties_length(meshgl) * sizeof(real)), meshgl);
            result.triangle_indices = (uint3 *) manifold_meshgl_tri_verts(malloc(manifold_meshgl_tri_length(meshgl) * sizeof(uint)), meshgl);
            mesh_triangle_normals_calculate(&result);
            mesh_cosmetic_edges_calculate(&result);
            mesh_bbox_calculate(&result);
        }

        manifold_delete_meshgl(meshgl);
    }

    return result;
}

char *key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(KeyEvent *key_event) {
    static char buffer[256];

    char *_ctrl_plus; {
        if (!key_event->control) {
            _ctrl_plus = "";
        } else {
            _ctrl_plus = "CTRL+";
        }
    }

    char *_shift_plus; {
        if (!key_event->shift) {
            _shift_plus = "";
        } else {
            _shift_plus = "SHIFT+";
        }
    }

    char _key_buffer[2];
    char *_key; {
        if (0) ;
        else if (key_event->key == GLFW_KEY_BACKSPACE) _key = "BACKSPACE";
        else if (key_event->key == GLFW_KEY_DELETE) _key = "DELETE";
        else if (key_event->key == GLFW_KEY_ENTER) _key = "ENTER";
        else if (key_event->key == GLFW_KEY_ESCAPE) _key = "ESCAPE";
        else if (key_event->key == GLFW_KEY_LEFT) _key = "LEFT";
        else if (key_event->key == GLFW_KEY_RIGHT) _key = "RIGHT";
        else if (key_event->key == GLFW_KEY_SPACE) _key = "SPACE";
        else if (key_event->key == GLFW_KEY_TAB) _key = "TAB";
        else if (key_event->key == GLFW_KEY_UP) _key = "UP";
        else if (key_event->key == GLFW_KEY_DOWN) _key = "DOWN";
        else {
            _key_buffer[0] = (char) key_event->key;
            _key_buffer[1] = '\0';
            _key = _key_buffer;
        }
    }

    sprintf(buffer, "%s%s%s", _ctrl_plus, _shift_plus, _key);
    return buffer;
}
