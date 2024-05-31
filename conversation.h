// TODO: make Mesh vec3's and ivec3's
// TODO: take entire transform (same used for draw) for wrapper_manifold--strip out incremental nature into function

// TODO: void eso_bounding_box__SOUP_QUADS(bbox2 bounding_box)

////////////////////////////////////////
// FORNOW: forward declarations ////////
////////////////////////////////////////

void conversation_messagef(vec3 color, char *format, ...);
real32 get_x_divider_Pixel();


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

#define RGB256(r, g, b) { (r) / 255.0f, (g) / 255.0f, (b) / 255.0f }

struct {
    vec3 yellow = { 1.0f, 1.0f, 0.0f };
    vec3 cyan = { 0.0f, 1.0f, 1.0f };
} basic;

struct {
    vec3 red = RGB256(255, 0, 0);
    vec3 pink = RGB256(238, 0, 119);
    vec3 yellow = RGB256(255, 255, 0);
    vec3 orange = RGB256(204, 136, 1);
    vec3 green = RGB256(83, 255,  85);
    vec3 cyan = RGB256(0, 255, 255);
    vec3 blue = RGB256(0, 85, 255);
    vec3 purple = RGB256(170, 1, 255);
    vec3 magenta = RGB256(255, 0, 255);
    vec3 dark_gray = RGB256(136, 136, 136);
    vec3 light_gray = RGB256(205, 205, 205);
} omax;

vec3 omax_pallete[] = {
    omax.green,
    omax.red,
    omax.pink,
    omax.magenta,
    omax.purple,
    omax.blue,
    omax.dark_gray,
    omax.light_gray,
    omax.cyan,
    omax.orange,
};

////////////////////////////////////////
// bbox (TODO: strip) //////////////////
////////////////////////////////////////

template <uint32 D> struct BoundingBox {
    SnailVector<D> min;
    SnailVector<D> max;
};

typedef BoundingBox<2> bbox2;
typedef BoundingBox<2> bbox3;


////////////////////////////////////////
// enums ///////////////////////////////
////////////////////////////////////////

enum class EnterMode {
    None,
    ExtrudeAdd,
    ExtrudeCut,
    NudgeFeaturePlane,
    Open,
    RevolveAdd,
    RevolveCut,
    Save,
};

enum class ClickMode {
    None,
    Axis,
    Box,
    Circle,
    Color,
    Deselect,
    Fillet,
    Line,
    Measure,
    MirrorX,
    MirrorY,
    Move,
    Origin,
    Select,
};

enum class ClickModifier {
    None,
    Center,
    Color,
    Connected,
    End,
    Middle,
    Perpendicular,
    Selected,
    Window,
    XYCoordinates,
};

enum class EntityType {
    Arc,
    Line,
};

enum class Pane {
    None,
    DXF,
    STL,
    Divider,
    Popup,
};

enum class CellType {
    None,
    Real32,
    String,
};

#define EVENT_TYPE_NONE  0
#define EVENT_TYPE_KEY   1
#define EVENT_TYPE_MOUSE 2

#define KEY_EVENT_SUBTYPE_NONE   0
#define KEY_EVENT_SUBTYPE_HOTKEY 1
#define KEY_EVENT_SUBTYPE_GUI    2

#define MOUSE_EVENT_SUBTYPE_NONE 0
#define MOUSE_EVENT_SUBTYPE_2D   1
#define MOUSE_EVENT_SUBTYPE_3D   2
#define MOUSE_EVENT_SUBTYPE_GUI  3

enum class ColorCode {
    Traverse,
    Quality1,
    Quality2,
    Quality3,
    Quality4,
    Quality5,
    Etch,
    WaterOnly,
    LeadIO,
    Selection,
    QualitySlit1 = 21,
    QualitySlit2,
    QualitySlit3,
    QualitySlit4,
    QualitySlit5,
};

/////////////////

#define POPUP_CELL_LENGTH 256
#define POPUP_MAX_NUM_CELLS 4

////////////////////////////////////////
// structs /////////////////////////////
////////////////////////////////////////

struct LineEntity {
    vec2 start;
    vec2 end;
};

struct ArcEntity {
    vec2 center;
    real32 radius;
    real32 start_angle_in_degrees;
    real32 end_angle_in_degrees;
};

struct Entity {
    EntityType type;

    ColorCode color_code;
    bool32 is_selected;
    real32 time_since_is_selected_changed;

    LineEntity line_entity;
    ArcEntity arc_entity;
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

// TODO: struct out RawEvent into RawMouseEvent and RawKeyEvent
struct RawEvent {
    uint32 type;
    Pane pane;

    uint32 key;
    bool32 control;
    bool32 shift;

    vec2 mouse_Pixel;
    bool32 mouse_held;
};

struct MouseEvent2D {
    vec2 mouse_position;
};

struct MouseEvent3D {
    vec3 mouse_ray_origin;
    vec3 mouse_ray_direction;
};

struct MouseEventGUI {
    uint32 cell_index;
    uint32 cursor;
    uint32 selection_cursor;
};

struct MouseEvent {
    uint32 subtype;

    bool32 mouse_held;

    MouseEvent2D mouse_event_2D;
    MouseEvent3D mouse_event_3D;
    MouseEventGUI mouse_event_gui;
};

struct KeyEvent {
    uint32 subtype;

    uint32 key;
    bool32 control;
    bool32 shift;
};

struct Event {
    uint32 type;

    bool32 record_me;
    bool32 checkpoint_me;
    bool32 snapshot_me;

    KeyEvent key_event;
    MouseEvent mouse_event;
};

struct DXFState {
    List<Entity> entities;
    vec2            origin;
    vec2            axis_base_point;
    real32          axis_angle_from_y;
};

struct FeaturePlaneState {
    bool32 is_active;
    vec3 normal;
    real32 signed_distance_to_world_origin;
};

struct PopupState {
    CellType cell_type[POPUP_MAX_NUM_CELLS];
    char *name[POPUP_MAX_NUM_CELLS];
    void *value[POPUP_MAX_NUM_CELLS];
    uint32 num_cells;

    char active_cell_buffer[POPUP_CELL_LENGTH];
    uint32 active_cell_index;
    uint32 cursor;
    uint32 selection_cursor;

    CellType _type_of_active_cell;
    void *_active_popup_unique_ID__FORNOW_name0;

    bool32 mouse_is_hovering;
    uint32 hover_cell_index;
    uint32 hover_cursor;

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
    real32 feature_plane_nudge;
    real32 line_length;
    real32 line_angle;
    real32 line_run;
    real32 line_rise;
    real32 move_length;
    real32 move_angle;
    real32 move_run;
    real32 move_rise;
    real32 revolve_add_dummy;
    real32 revolve_cut_dummy;
    char open_filename[POPUP_CELL_LENGTH];
    char save_filename[POPUP_CELL_LENGTH];
};

struct WorldState {
    Mesh mesh;
    DXFState dxf;
    FeaturePlaneState feature_plane;

    ClickMode click_mode;
    ClickModifier click_modifier;
    ColorCode click_color_code;
    EnterMode enter_mode;

    struct {
        bool32 awaiting_second_click;
        vec2 first_click;
    } two_click_command;

    PopupState popup;

    Event space_bar_event;
    Event shift_space_bar_event;
};

struct ScreenState {
    Camera2D camera_2D;
    Camera3D camera_3D;

    bool32   hide_grid;
    bool32   hide_gui;
    bool32   show_details;
    bool32   show_help;
    bool32   show_event_stack;

    Pane hot_pane;
    Pane mouse_left_drag_pane;
    Pane mouse_right_drag_pane;

    vec2 mouse_NDC;
    vec2 mouse_Pixel;
    bool32 shift_held;
    bool32 mouse_left_held;
    bool32 mouse_right_held;

    real32 x_divider_NDC;

    char drop_path[256];
    bool32 DONT_DRAW_ANY_MORE_POPUPS_THIS_FRAME;

};

struct TimeSince {
    real32 cursor_start;
    real32 successful_feature;
    real32 plane_selected;
    real32 going_inside;
    bool32 _helper_going_inside;
};

struct AestheticsState {
    TimeSince time_since;
    bbox2 preview_feature_plane;
    real32 preview_extrude_in_length;
    real32 preview_extrude_out_length;
    vec3 preview_tubes_color;
    vec3 preview_feature_plane_color;
    real32 preview_feature_plane_offset;
};

////////////////////////////////////////
// "constants" /////////////////////////
////////////////////////////////////////

template <uint32 D> BoundingBox<D> BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA() {
    BoundingBox<D> result;
    for (uint32 d = 0; d < D; ++d) {
        result.min[d] = HUGE_VAL;
        result.max[d] = -HUGE_VAL;
    }
    return result;
}


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

template <typename T> void JUICEIT_EASYTWEEN(T *a, T b, real32 f = 0.1f) {
    #ifdef DEBUG_DISABLE_EASY_TWEEN
    f = 1.0f;
    #endif
    *a += f * (b - *a);
}

////////////////////////////////////////
// Squared-Distance (TODO: move non-dxf_entities parts all up here);
////////////////////////////////////////

real32 squared_distance_point_point(real32 x_A, real32 y_A, real32 x_B, real32 y_B) {
    real32 dx = (x_A - x_B);
    real32 dy = (y_A - y_B);
    return (dx * dx) + (dy * dy);
};

////////////////////////////////////////
// bbox2 /////////////////////////
////////////////////////////////////////

void pprint(bbox2 bounding_box) {
    printf("(%f, %f) <-> (%f, %f)\n", bounding_box.min[0], bounding_box.min[1], bounding_box.max[0], bounding_box.max[1]);
}

void bounding_box_center(bbox2 bounding_box, real32 *x, real32 *y) {
    *x = (bounding_box.min[0] + bounding_box.max[0]) / 2;
    *y = (bounding_box.min[1] + bounding_box.max[1]) / 2;
}

bool32 bounding_box_contains(bbox2 outer, bbox2 inner) {
    for (uint32 d = 0; d < 2; ++d) {
        if (outer.min[d] > inner.min[d]) return false;
        if (outer.max[d] < inner.max[d]) return false;
    }
    return true;
}

bool32 bounding_box_contains(bbox2 bounding_box, vec2 point) {
    for (uint32 d = 0; d < 2; ++d) {
        if (!IS_BETWEEN(point[d], bounding_box.min[d], bounding_box.max[d])) return false;
    }
    return true;
}

vec2 bounding_box_clamp(vec2 p, bbox2 bounding_box) {
    for (uint32 d = 0; d < 2; ++d) {
        p[d] = CLAMP(p[d], bounding_box.min[d], bounding_box.max[d]);
    }
    return p;
}

void bounding_box_add_point(bbox2 *bounding_box, vec2 p) {
    for (uint32 d = 0; d < 2; ++d) {
        bounding_box->min[d] = MIN(bounding_box->min[d], p[d]);
        bounding_box->max[d] = MAX(bounding_box->max[d], p[d]);
    }
}

bbox2 bounding_box_union(bbox2 a, bbox2 b) {
    for (uint32 d = 0; d < 2; ++d) {
        a.min[d] = MIN(a.min[d], b.min[d]);
        a.max[d] = MAX(a.max[d], b.max[d]);
    }
    return a;
}

void camera2D_zoom_to_bounding_box(Camera2D *camera_2D, bbox2 bounding_box) {
    real32 new_o_x = AVG(bounding_box.min[0], bounding_box.max[0]);
    real32 new_o_y = AVG(bounding_box.min[1], bounding_box.max[1]);
    real32 new_height = MAX((bounding_box.max[0] - bounding_box.min[0]) * 2 / _window_get_aspect(), (bounding_box.max[1] - bounding_box.min[1])); // factor of 2 since splitscreen
    new_height *= 1.3f; // FORNOW: border
    camera_2D->height_World = new_height;
    camera_2D->center_World.x = new_o_x;
    camera_2D->center_World.y = new_o_y;
}

////////////////////////////////////////
// List<Entity> /////////////////////////////////
////////////////////////////////////////




void get_point_on_circle_NOTE_pass_angle_in_radians(real32 *x, real32 *y, real32 center_x, real32 center_y, real32 radius, real32 angle_in_radians) {
    *x = center_x + radius * COS(angle_in_radians);
    *y = center_y + radius * SIN(angle_in_radians);
}

void arc_process_angles_into_lerpable_radians_considering_flip_flag(ArcEntity *arc_entity, real32 *start_angle, real32 *end_angle, bool32 flip_flag) {
    // The way the List<Entity> spec works is that start_angle and end_angle define points on the circle
    // which are connected counterclockwise from start to end with an arc_entity
    // (start -ccw-> end)
    //
    // To flip an arc_entity entity, we need to go B -cw-> A
    *start_angle = RAD(arc_entity->start_angle_in_degrees);
    *end_angle = RAD(arc_entity->end_angle_in_degrees);
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

real32 entity_length(Entity *entity) {
    if (entity->type == EntityType::Line) {
        LineEntity *line_entity = &entity->line_entity;
        return SQRT(squared_distance_point_point(line_entity->start.x, line_entity->start.y, line_entity->end.x, line_entity->end.y));
    } else {
        ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc_entity = &entity->arc_entity;
        real32 start_angle;
        real32 end_angle;
        arc_process_angles_into_lerpable_radians_considering_flip_flag(arc_entity, &start_angle, &end_angle, false);
        return ABS(start_angle - end_angle) * arc_entity->radius;
    }
}

void entity_get_start_point(Entity *entity, real32 *start_x, real32 *start_y) {
    if (entity->type == EntityType::Line) {
        LineEntity *line_entity = &entity->line_entity;
        *start_x = line_entity->start.x;
        *start_y = line_entity->start.y;
    } else {
        ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc_entity = &entity->arc_entity;
        get_point_on_circle_NOTE_pass_angle_in_radians(start_x, start_y, arc_entity->center.x, arc_entity->center.y, arc_entity->radius, RAD(arc_entity->start_angle_in_degrees));
    }
}

void entity_get_end_point(Entity *entity, real32 *end_x, real32 *end_y) {
    if (entity->type == EntityType::Line) {
        LineEntity *line_entity = &entity->line_entity;
        *end_x = line_entity->end.x;
        *end_y = line_entity->end.y;
    } else {
        ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc_entity = &entity->arc_entity;
        get_point_on_circle_NOTE_pass_angle_in_radians(  end_x,   end_y, arc_entity->center.x, arc_entity->center.y, arc_entity->radius,   RAD(arc_entity->end_angle_in_degrees));
    }
}

void entity_get_start_and_end_points(Entity *entity, real32 *start_x, real32 *start_y, real32 *end_x, real32 *end_y) {
    entity_get_start_point(entity, start_x, start_y);
    entity_get_end_point(entity, end_x, end_y);
}

void entity_lerp_considering_flip_flag(Entity *entity, real32 t, real32 *x, real32 *y, bool32 flip_flag) {
    ASSERT(IS_BETWEEN(t, 0.0f, 1.0f));
    if (entity->type == EntityType::Line) {
        LineEntity *line_entity = &entity->line_entity;
        if (flip_flag) t = 1.0f - t; // FORNOW
        *x = LERP(t, line_entity->start.x, line_entity->end.x);
        *y = LERP(t, line_entity->start.y, line_entity->end.y);
    } else {
        ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc_entity = &entity->arc_entity;
        real32 angle; {
            real32 start_angle, end_angle;
            arc_process_angles_into_lerpable_radians_considering_flip_flag(arc_entity, &start_angle, &end_angle, flip_flag); // FORNOW
            angle = LERP(t, start_angle, end_angle);
        }
        get_point_on_circle_NOTE_pass_angle_in_radians(x, y, arc_entity->center.x, arc_entity->center.y, arc_entity->radius, angle);
    }
}

void entity_get_middle(Entity *entity, real32 *middle_x, real32 *middle_y) {
    entity_lerp_considering_flip_flag(entity, 0.5f, middle_x, middle_y, false);
}

// struct List<Entity> {
//     uint32 num_entities;
//     Entity *entities;
// };

void dxf_entities_load(char *filename, List<Entity> *dxf_entities) {
    #if 0
    {
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(filename);
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
    list_free_AND_zero(dxf_entities);

    FILE *file = (FILE *) fopen(filename, "r");
    ASSERT(file);

    *dxf_entities = {}; {
        #define OPEN_MODE_NONE 0
        #define OPEN_MODE_LINE 1
        #define OPEN_MODE_ARC  2
        u8 mode = 0;
        int code = 0;
        bool32 code_is_hot = false;
        Entity entity = {};
        static char buffer[512];
        while (fgets(buffer, ARRAY_LENGTH(buffer), file)) {
            if (mode == OPEN_MODE_NONE) {
                if (poe_prefix_match(buffer, "LINE")) {
                    mode = OPEN_MODE_LINE;
                    code_is_hot = false;
                    entity = {};
                    entity.type = EntityType::Line;
                } else if (poe_prefix_match(buffer, "ARC")) {
                    mode = OPEN_MODE_ARC;
                    code_is_hot = false;
                    entity = {};
                    entity.type = EntityType::Arc;
                }
            } else {
                if (!code_is_hot) {
                    sscanf(buffer, "%d", &code);
                    // NOTE this initialization is sketchy but works
                    // probably don't make a habit of it
                    if (code == 0) {
                        list_push_back(dxf_entities, entity);
                        mode = OPEN_MODE_NONE;
                        code_is_hot = false;
                    }
                } else {
                    if (code == 62) {
                        int value;
                        sscanf(buffer, "%d", &value);
                        entity.color_code = (ColorCode) value; 
                    } else {
                        float value;
                        sscanf(buffer, "%f", &value);
                        if (mode == OPEN_MODE_LINE) {
                            if (code == 10) {
                                entity.line_entity.start.x = MM(value);
                            } else if (code == 20) {
                                entity.line_entity.start.y = MM(value);
                            } else if (code == 11) {
                                entity.line_entity.end.x = MM(value);
                            } else if (code == 21) {
                                entity.line_entity.end.y = MM(value);
                            }
                        } else {
                            ASSERT(mode == OPEN_MODE_ARC);
                            if (code == 10) {
                                entity.arc_entity.center.x = MM(value);
                            } else if (code == 20) {
                                entity.arc_entity.center.y = MM(value);
                            } else if (code == 40) {
                                entity.arc_entity.radius = MM(value);
                            } else if (code == 50) {
                                entity.arc_entity.start_angle_in_degrees = value;
                            } else if (code == 51) {
                                entity.arc_entity.end_angle_in_degrees = value;
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
    uint32 i = (uint32) color_code;
    if (0 <= i && i <= 9) {
        return omax_pallete[i];
    } else if (20 <= i && i <= 29) {
        do_once { conversation_messagef(monokai.orange, "WARNING: slits not implemented"); };
        return omax_pallete[i - 20];
    } else if (color_code == ColorCode::Selection) {
        return omax.yellow;
    } else {
        ASSERT(false);
        return {};
    }
}

void eso_dxf_entity__SOUP_LINES(Entity *entity, real32 dx = 0.0f, real32 dy = 0.0f) {
    if (entity->type == EntityType::Line) {
        LineEntity *line_entity = &entity->line_entity;
        eso_vertex(line_entity->start.x + dx, line_entity->start.y + dy);
        eso_vertex(line_entity->end.x + dx,   line_entity->end.y + dy);
    } else {
        ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc_entity = &entity->arc_entity;
        real32 start_angle, end_angle;
        arc_process_angles_into_lerpable_radians_considering_flip_flag(arc_entity, &start_angle, &end_angle, false);
        real32 delta_angle = end_angle - start_angle;
        uint32 num_segments = (uint32) (1 + (delta_angle / TAU) * NUM_SEGMENTS_PER_CIRCLE);
        real32 increment = delta_angle / num_segments;
        real32 current_angle = start_angle;
        for (uint32 i = 0; i < num_segments; ++i) {
            real32 x, y;
            get_point_on_circle_NOTE_pass_angle_in_radians(&x, &y, arc_entity->center.x, arc_entity->center.y, arc_entity->radius, current_angle);
            eso_vertex(x + dx, y + dy);
            current_angle += increment;
            get_point_on_circle_NOTE_pass_angle_in_radians(&x, &y, arc_entity->center.x, arc_entity->center.y, arc_entity->radius, current_angle);
            eso_vertex(x + dx, y + dy);
        }
    }
}


void dxf_entities_debug_draw(Camera2D *camera_2D, List<Entity> *dxf_entities) {
    eso_begin(camera_get_PV(camera_2D), SOUP_LINES);
    for (Entity *entity = dxf_entities->array; entity < &dxf_entities->array[dxf_entities->length]; ++entity) {
        eso_dxf_entity__SOUP_LINES(entity);
    }
    eso_end();
}

bbox2 dxf_entity_get_bounding_box(Entity *entity) {
    bbox2 result = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
    real32 s[2][2];
    uint32 n = 2;
    entity_get_start_and_end_points(entity, &s[0][0], &s[0][1], &s[1][0], &s[1][1]);
    for (uint32 i = 0; i < n; ++i) {
        for (uint32 d = 0; d < 2; ++d) {
            result.min[d] = MIN(result.min[d], s[i][d]);
            result.max[d] = MAX(result.max[d], s[i][d]);
        }
    }
    if (entity->type == EntityType::Arc) {
        ArcEntity *arc_entity = &entity->arc_entity;
        // NOTE: endpoints already taken are of; we just have to deal with the quads (if they exist)
        // TODO: angle_is_between_counter_clockwise (TODO TODO TODO)
        if (ANGLE_IS_BETWEEN_CCW(  0.0f, arc_entity->start_angle_in_degrees, arc_entity->end_angle_in_degrees)) result.max[0] = MAX(result.max[0], arc_entity->center.x + arc_entity->radius);
        if (ANGLE_IS_BETWEEN_CCW( 90.0f, arc_entity->start_angle_in_degrees, arc_entity->end_angle_in_degrees)) result.max[1] = MAX(result.max[1], arc_entity->center.y + arc_entity->radius);
        if (ANGLE_IS_BETWEEN_CCW(180.0f, arc_entity->start_angle_in_degrees, arc_entity->end_angle_in_degrees)) result.min[0] = MIN(result.min[0], arc_entity->center.x - arc_entity->radius);
        if (ANGLE_IS_BETWEEN_CCW(270.0f, arc_entity->start_angle_in_degrees, arc_entity->end_angle_in_degrees)) result.min[1] = MIN(result.min[1], arc_entity->center.y - arc_entity->radius);
    }
    return result;
}

bbox2 dxf_entities_get_bounding_box(List<Entity> *dxf_entities, bool32 only_consider_selected_entities = false) {
    bbox2 result = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
    for (uint32 i = 0; i < dxf_entities->length; ++i) {
        if ((only_consider_selected_entities) && (!dxf_entities->array[i].is_selected)) continue;
        bbox2 bounding_box = dxf_entity_get_bounding_box(&dxf_entities->array[i]);
        result = bounding_box_union(result, bounding_box);
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

real32 squared_distance_point_dxf_line(real32 x, real32 y, LineEntity *line_entity) {
    return squared_distance_point_line_segment(x, y, line_entity->start.x, line_entity->start.y, line_entity->end.x, line_entity->end.y);
}

real32 squared_distance_point_dxf_arc(real32 x, real32 y, ArcEntity *arc_entity) {
    return squared_distance_point_arc_NOTE_pass_angles_in_radians(x, y, arc_entity->center.x, arc_entity->center.y, arc_entity->radius, RAD(arc_entity->start_angle_in_degrees), RAD(arc_entity->end_angle_in_degrees));
}

real32 squared_distance_point_dxf_entity(real32 x, real32 y, Entity *entity) {
    if (entity->type == EntityType::Line) {
        LineEntity *line_entity = &entity->line_entity;
        return squared_distance_point_dxf_line(x, y, line_entity);
    } else {
        ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc_entity = &entity->arc_entity;
        return squared_distance_point_dxf_arc(x, y, arc_entity);
    }
}

real32 squared_distance_point_dxf(real32 x, real32 y, List<Entity> *dxf_entities) {
    real32 result = HUGE_VAL;
    for (Entity *entity = dxf_entities->array; entity < &dxf_entities->array[dxf_entities->length]; ++entity) {
        result = MIN(result, squared_distance_point_dxf_entity(x, y, entity));
    }
    return result;
}

int dxf_find_closest_entity(List<Entity> *dxf_entities, real32 x, real32 y) {
    int result = -1;
    double hot_squared_distance = HUGE_VAL;
    for (uint32 i = 0; i < dxf_entities->length; ++i) {
        Entity *entity = &dxf_entities->array[i];
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

DXFLoopAnalysisResult dxf_loop_analysis_create_FORNOW_QUADRATIC(List<Entity> *dxf_entities, bool32 only_consider_selected_entities) {
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
                                Entity *entity = &dxf_entities->array[entity_index];
                                if (entity->type == EntityType::Line) {
                                    LineEntity *line_entity = &entity->line_entity;
                                    // shoelace-type formula
                                    twice_the_signed_area += ((flip_flag) ? -1 : 1) * (line_entity->start.x * line_entity->end.y - line_entity->end.x * line_entity->start.y);
                                } else {
                                    ASSERT(entity->type == EntityType::Arc);
                                    ArcEntity *arc_entity = &entity->arc_entity;
                                    // "Circular approximation using polygons"
                                    // - n = 2 (area-preserving approximation of arc_entity with two segments)
                                    real32 start_angle, end_angle;
                                    arc_process_angles_into_lerpable_radians_considering_flip_flag(arc_entity, &start_angle, &end_angle, flip_flag);
                                    real32 start_x, start_y, end_x, end_y;
                                    get_point_on_circle_NOTE_pass_angle_in_radians(&start_x, &start_y, arc_entity->center.x, arc_entity->center.y, arc_entity->radius, start_angle);
                                    get_point_on_circle_NOTE_pass_angle_in_radians(  &end_x,   &end_y, arc_entity->center.x, arc_entity->center.y, arc_entity->radius,   end_angle);
                                    real32 mid_angle = (start_angle + end_angle) / 2;
                                    real32 d; {
                                        real32 alpha = ABS(start_angle - end_angle) / 2;
                                        d = arc_entity->radius * alpha / SIN(alpha);
                                    }
                                    real32 mid_x, mid_y;
                                    get_point_on_circle_NOTE_pass_angle_in_radians(&mid_x, &mid_y, arc_entity->center.x, arc_entity->center.y, d, mid_angle);
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

CrossSectionEvenOdd cross_section_create_FORNOW_QUADRATIC(List<Entity> *dxf_entities, bool32 only_consider_selected_entities) {
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
                Entity *entity = &dxf_entities->array[entity_index];
                if (entity->type == EntityType::Line) {
                    LineEntity *line_entity = &entity->line_entity;
                    if (!flip_flag) {
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], { line_entity->start.x, line_entity->start.y });
                    } else {
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], { line_entity->end.x, line_entity->end.y });
                    }
                } else {
                    ASSERT(entity->type == EntityType::Arc);
                    ArcEntity *arc_entity = &entity->arc_entity;
                    real32 start_angle, end_angle;
                    arc_process_angles_into_lerpable_radians_considering_flip_flag(arc_entity, &start_angle, &end_angle, flip_flag);
                    real32 delta_angle = end_angle - start_angle;
                    uint32 num_segments = (uint32) (2 + ABS(delta_angle) * (NUM_SEGMENTS_PER_CIRCLE / TAU)); // FORNOW (2 + ...)
                    real32 increment = delta_angle / num_segments;
                    real32 current_angle = start_angle;
                    real32 x, y;
                    for (uint32 i = 0; i < num_segments; ++i) {
                        get_point_on_circle_NOTE_pass_angle_in_radians(&x, &y, arc_entity->center.x, arc_entity->center.y, arc_entity->radius, current_angle);
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
#define MESSAGE_MAX_TIME 6.0f
struct Message {
    char buffer[MESSAGE_MAX_LENGTH];
    real32 time_remaining;
    real32 y;
    vec3 base_color;
};
uint32 message_index;
Message conversation_messages[MESSAGE_MAX_NUM_MESSAGES];
void conversation_messagef(vec3 color, char *format, ...) {
    if (IGNORE_NEW_MESSAGEFS) return;
    va_list arg;
    va_start(arg, format);
    Message *message = &conversation_messages[message_index];
    vsnprintf(message->buffer, MESSAGE_MAX_LENGTH, format, arg);
    va_end(arg);

    message->base_color = color;
    message->time_remaining = MESSAGE_MAX_TIME;
    message_index = (message_index + 1) % MESSAGE_MAX_NUM_MESSAGES;
    message->y = 0.0f;

    // printf("%s\n", message->buffer); // FORNOW print to terminal as well
}
void conversation_message_buffer_update_and_draw() {
    uint32 i_0 =  (message_index == 0) ? (MESSAGE_MAX_NUM_MESSAGES - 1) : message_index - 1;

    real32 y_prev = 0.0f;
    // uint32 num_drawn = 0;
    auto draw_lambda = [&](uint32 message_index) {
        Message *message = &conversation_messages[message_index];

        real32 FADE_TIME = 0.33f;

        real32 a; { // ramp on ramp off
            a = 0
                + CLAMPED_LINEAR_REMAP(message->time_remaining, MESSAGE_MAX_TIME, MESSAGE_MAX_TIME - FADE_TIME, 0.0f, 1.0f)
                - CLAMPED_LINEAR_REMAP(message->time_remaining, FADE_TIME, 0.0f, 0.0f, 1.0f);
        }

        vec3 color = CLAMPED_LINEAR_REMAP(message->time_remaining, MESSAGE_MAX_TIME + FADE_TIME, MESSAGE_MAX_TIME - 4.0f * FADE_TIME, monokai.yellow, message->base_color);
        real32 r = color.x;
        real32 g = color.y;
        real32 b = color.z;

        real32 y_target = y_prev + 16.0f;
        y_prev = LERP(0.0f, y_target, message->y);
        if (message->time_remaining < 2.0f * FADE_TIME) y_target += 8.0f;
        // y_prev = message->y;

        JUICEIT_EASYTWEEN(&message->y, y_target);
        if (message->time_remaining > 0) {
            message->time_remaining -= 0.0167f;;
            _text_draw(
                    (cow_real *) &globals.NDC_from_Screen,
                    message->buffer,
                    get_x_divider_Pixel() + 16,
                    message->y,
                    0.0,

                    r,
                    g,
                    b,
                    a,

                    0,
                    0.0,
                    0.0,
                    true);
        } else {
            message->time_remaining = 0.0f;
        }
    };

    { // this is pretty gross
        uint32 i = i_0;
        while (true) {
            draw_lambda(i);

            if (i > 0) --i;
            else if (i == 0) i = MESSAGE_MAX_NUM_MESSAGES - 1;

            if (i == i_0) break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// key_lambda //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool _key_lambda(KeyEvent *key_event, uint32 key, bool control = false, bool shift = false) {
    ASSERT(!(('a' <= key) && (key <= 'z')));
    bool key_match = (key_event->key == key);
    bool super_match = ((key_event->control && control) || (!key_event->control && !control)); // * bool32
    bool shift_match = ((key_event->shift && shift) || (!key_event->shift && !shift)); // * bool32
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
        EnterMode enter_mode,
        real32 extrude_out_length,
        real32 extrude_in_length,
        vec2   dxf_origin,
        vec2   dxf_axis_base_point,
        real32 dxf_axis_angle_from_y
        ) {


    bool32 add = (enter_mode == EnterMode::ExtrudeAdd) || (enter_mode == EnterMode::RevolveAdd);
    bool32 cut = (enter_mode == EnterMode::ExtrudeCut) || (enter_mode == EnterMode::RevolveCut);
    bool32 extrude = (enter_mode == EnterMode::ExtrudeAdd) || (enter_mode == EnterMode::ExtrudeCut);
    bool32 revolve = (enter_mode == EnterMode::RevolveAdd) || (enter_mode == EnterMode::RevolveCut);
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
            if (enter_mode == EnterMode::ExtrudeCut) {
                do_once { conversation_messagef(omax.red, "[DEBUG] inflating EnterMode::ExtrudeCut\n"); };
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
        if (0) {
        } else if (key_event->key == GLFW_KEY_BACKSPACE) { _key = "BACKSPACE";
        } else if (key_event->key == GLFW_KEY_DELETE) { _key = "DELETE";
        } else if (key_event->key == GLFW_KEY_ENTER) { _key = "ENTER";
        } else if (key_event->key == GLFW_KEY_ESCAPE) { _key = "ESCAPE";
        } else if (key_event->key == GLFW_KEY_LEFT) { _key = "LEFT";
        } else if (key_event->key == GLFW_KEY_RIGHT) { _key = "RIGHT";
        } else if (key_event->key == GLFW_KEY_SPACE) { _key = "SPACE";
        } else if (key_event->key == GLFW_KEY_TAB) { _key = "TAB";
        } else {
            _key_buffer[0] = (char) key_event->key;
            _key_buffer[1] = '\0';
            _key = _key_buffer;
        }
    }

    sprintf(buffer, "%s%s%s", _ctrl_plus, _shift_plus, _key);
    return buffer;
}
