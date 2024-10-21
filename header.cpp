////////////////////////////////////////
// Forward-Declarations ////////////////
////////////////////////////////////////

void messagef(vec3 color, char *format, ...);
template <typename T> void JUICEIT_EASYTWEEN(T *a, T b, real multiplier = 1.0f);
// TODO: take entire transform (same used for draw) for wrapper_manifold--strip out incremental nature into function



///////////


enum class ToolboxGroup {
    None,
    Draw,
    Snap,
    Mesh,
    Xsel, // Select
    Colo, // SetColor
    Both, // DrawingAndMesh
    NUMBER_OF,
};

struct Shortcut {
    uint key;
    u8 mods;
};

#define CMD_FLAG_ (1 << 00)
struct Command {
    ToolboxGroup group;
    bool is_mode;
    u64 flags;
    String name;
    #define COMMAND_MAX_NUM_SHORTCUTS 4
    Shortcut shortcuts[COMMAND_MAX_NUM_SHORTCUTS];
};

bool command_equals(Command A, Command B) {
    return (A.name.data == B.name.data);
}

// FORNOW ew
#define state_Draw_command_is_(Name) command_equals(state.Draw_command, commands.Name)
#define state_Mesh_command_is_(Name) command_equals(state.Mesh_command, commands.Name)
#define state_Snap_command_is_(Name) command_equals(state.Snap_command, commands.Name)
#define state_Xsel_command_is_(Name) command_equals(state.Xsel_command, commands.Name)
#define state_Colo_command_is_(Name) command_equals(state.Colo_command, commands.Name)

// FORNOW ew
#define set_state_Draw_command(Name) do { /*ASSERT(command_equals(commands.Name, commands.None) || (commands.Name.group == ToolboxGroup::Draw));*/ state.Draw_command = commands.Name; } while (0)
#define set_state_Mesh_command(Name) do { /*ASSERT(command_equals(commands.Name, commands.None) || (commands.Name.group == ToolboxGroup::Mesh));*/ state.Mesh_command = commands.Name; } while (0)
#define set_state_Snap_command(Name) do { /*ASSERT(command_equals(commands.Name, commands.None) || (commands.Name.group == ToolboxGroup::Snap));*/ state.Snap_command = commands.Name; } while (0)
#define set_state_Xsel_command(Name) do { /*ASSERT(command_equals(commands.Name, commands.None) || (commands.Name.group == ToolboxGroup::Xsel));*/ state.Xsel_command = commands.Name; } while (0)
#define set_state_Colo_command(Name) do { /*ASSERT(command_equals(commands.Name, commands.None) || (commands.Name.group == ToolboxGroup::Colo));*/ state.Colo_command = commands.Name; } while (0)


#include "commands.cpp"

Command commands_Color[] = { 
    commands.Color0,
    commands.Color1,
    commands.Color2,
    commands.Color3,
    commands.Color4,
    commands.Color5,
    commands.Color6,
    commands.Color7,
    commands.Color8,
    commands.Color9
};


enum class EntityType {
    Arc,
    Line,
    Circle,
};

enum class Pane {
    None,
    Drawing,
    Mesh,
    Popup,
    Separator,
    Toolbox,
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
    ToolboxButton,
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
    _WaterOnly,
    LeadIO,
    QualitySlit1 = 21,
    QualitySlit2,
    QualitySlit3,
    QualitySlit4,
    QualitySlit5,
    Selection = 255,
    Emphasis = 254,
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

struct CircleEntity {
    vec2 center;
    real radius;
    bool has_pseudo_point;
    real pseudo_point_angle;

    vec2 get_pseudo_point() {
        vec2 get_point_on_circle_NOTE_pass_angle_in_radians(vec2, real, real);
        return get_point_on_circle_NOTE_pass_angle_in_radians(this->center, this->radius, this->pseudo_point_angle);
    }

    void set_pseudo_point(vec2 pseudo_point) {
        ASSERT(!ARE_EQUAL(this->center, pseudo_point));
        this->pseudo_point_angle = ATAN2(pseudo_point - this->center);
    }
};


struct Entity {
    EntityType type;

    ColorCode color_code;
    bool is_selected;
    vec3 preview_color;
    real time_since_is_selected_changed;

    LineEntity line;
    ArcEntity arc;
    CircleEntity circle;
};

struct WorkMesh {
    uint num_vertices;
    uint num_triangles;

    vec3  *vertex_positions;
    uint3 *triangle_tuples;
    vec3  *triangle_normals;

    bbox3 bbox;
};

struct DrawMesh {
    uint num_vertices;
    uint num_triangles; // NOTE: same as WorkMesh
    uint num_hard_edges;

    vec3  *vertex_positions;
    vec3  *vertex_normals;
    vec3  *triangle_normals; // TODO: eliminate this variable (preserve order of triangles work -> draw)
    uint3 *triangle_tuples;

    uint2 *hard_edges;
};

struct Meshes {
    WorkMesh work;
    DrawMesh draw;
};

struct RawKeyEvent {
    uint key;
    bool control;
    bool shift;
    bool alt;
};

struct RawMouseEvent {
    Pane pane;
    vec2 mouse_Pixel;
    bool mouse_held;
    bool mouse_double_click;
    bool mouse_double_click_held;
};

struct RawEvent {
    EventType type;

    RawKeyEvent raw_key_event;
    RawMouseEvent raw_mouse_event;
};

struct MagicSnapResult {
    vec2 mouse_position;
    bool snapped;
    uint entity_index_snapped_to;
    uint entity_index_intersect;
    uint entity_index_tangent_2;
    bool split_intersect;
    bool split_tangent_2;
};

struct MouseEventDrawing {
    MagicSnapResult snap_result;
};

struct MouseEventMesh {
    vec3 mouse_ray_origin;
    vec3 mouse_ray_direction;
};

struct MouseEventPopup {
    uint cell_index;
    uint cursor;
};

struct MouseEventToolboxButton {
    char *name;
};

struct MouseEvent {
    MouseEventSubtype subtype;

    vec2 mouse_Pixel;
    bool mouse_held;
    bool mouse_double_click;
    bool mouse_double_click_held;

    MouseEventDrawing mouse_event_drawing;
    MouseEventMesh mouse_event_mesh;
    MouseEventPopup mouse_event_popup;
    MouseEventToolboxButton mouse_event_toolbox_button;
};

struct KeyEvent {
    KeyEventSubtype subtype;

    uint key;
    bool control;
    bool shift;
    bool alt;
    char *_name_of_spoofing_button;
};

struct Event {
    EventType type;

    // ew
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
    Entity *entity_closest_to_first_click;
    bool tangent_first_click; // first thing that came to mind might be more elegant solution
};

struct MeshTwoClickCommandState {
    bool awaiting_second_click;
    vec3 first_click;
    int triangle_index_for_first_click;
};

struct PopupManager {
    char *tags[uint(ToolboxGroup::NUMBER_OF)];
    //
    char *get_tag(ToolboxGroup group) { return tags[uint(group)]; }
    void set_tag(ToolboxGroup group, char *_name0) { tags[uint(group)] = _name0; }

    bool _popup_popup_called_this_process[uint(ToolboxGroup::NUMBER_OF)];
    bool focus_group_was_set_manually;
    ToolboxGroup focus_group;

    //////////

    // TODO: problem is here (calling being_process interacts poorly with the recursion)
    void begin_process() {
        // // NOTE: end of previous call to process
        // -- (so we don't also need an end_process())
        bool any_active; {
            any_active = false;
            for (uint i = 1; i < uint(ToolboxGroup::NUMBER_OF); ++i) {
                if (!_popup_popup_called_this_process[i]) {
                    tags[i] = NULL;
                } else {
                    any_active = true;
                }
            }
        }
        if (!any_active) focus_group = ToolboxGroup::None;
        // // NOTE: beginning of this call to process
        focus_group_was_set_manually = false;
        memset(_popup_popup_called_this_process, 0, sizeof(_popup_popup_called_this_process));
    }

    // void end_process() {
    //     bool tag_corresponding_to_focus_group_became_NULL = (focus_group != ToolboxGroup::None) && (get_tag(focus_group) == NULL);
    //     if (tag_corresponding_to_focus_group_became_NULL) focus_group = ToolboxGroup::None;
    // }

    void manually_set_focus_group(ToolboxGroup new_focus_group) {
        // ASSERT(get_tag(new_focus_group)); // TODO: really important to get this assert working
        focus_group_was_set_manually = true;
        focus_group = new_focus_group;
    }

    void register_call_to_popup_popup(ToolboxGroup group) {
        _popup_popup_called_this_process[uint(group)] = true;
    }
};

#define POPUP_MAX_NUM_CELLS 5
#define POPUP_CELL_LENGTH 256
struct PopupState {
    _STRING_CALLOC(active_cell_buffer, POPUP_CELL_LENGTH);

    PopupManager manager;
    bool a_popup_from_this_group_was_already_called_this_frame[uint(ToolboxGroup::NUMBER_OF)];

    uint active_cell_index;
    uint cursor;
    uint selection_cursor;

    CellType _type_of_active_cell;

    bool _FORNOW_info_mouse_is_hovering;

    real extrude_add_out_length;
    real extrude_add_in_length;
    real extrude_cut_in_length;
    real extrude_cut_out_length;
    real circle_diameter;
    real circle_radius;
    real circle_circumference;
    real fillet_radius;
    real dogear_radius;
    real box_width;
    real box_height;
    real xy_x_coordinate;
    real xy_y_coordinate;
    real feature_plane_nudge;
    real line_length;
    real line_angle;
    real line_run;
    real line_rise;
    real drag_length;
    real drag_angle;
    real drag_run;
    uint drag_extend_line; // TODO: THIS SHOULD BE BOOL
    real drag_rise;
    real move_length;
    real move_angle;
    real move_run;
    real move_rise;
    real linear_copy_length;
    real linear_copy_angle;
    real linear_copy_run;
    real linear_copy_rise;
    uint linear_copy_num_additional_copies;
    real offset_distance;
    uint polygon_num_sides = 6;
    real polygon_distance_to_side;
    real polygon_distance_to_corner;
    real polygon_side_length;
    real revolve_add_in_angle;
    real revolve_add_out_angle;
    real revolve_cut_in_angle;
    real revolve_cut_out_angle;
    real rotate_angle;
    uint rotate_copy_num_total_copies;
    real rotate_copy_angle;
    real scale_factor;
    _STRING_CALLOC(open_dxf_filename, POPUP_CELL_LENGTH);
    _STRING_CALLOC(save_dxf_filename, POPUP_CELL_LENGTH);
    _STRING_CALLOC(overwrite_dxf_yn_buffer, POPUP_CELL_LENGTH);
    _STRING_CALLOC(open_stl_filename, POPUP_CELL_LENGTH);
    _STRING_CALLOC(save_stl_filename, POPUP_CELL_LENGTH);
    _STRING_CALLOC(overwrite_stl_yn_buffer, POPUP_CELL_LENGTH);
};

struct ToolboxState {
    char *hot_name;
};

struct WorldState_ChangesToThisMustBeRecorded_state {
    Meshes meshes;
    Drawing drawing;
    FeaturePlaneState feature_plane;
    TwoClickCommandState two_click_command;
    MeshTwoClickCommandState mesh_two_click_command;
    PopupState popup;
    ToolboxState toolbox;

    Command Draw_command = commands.None; // FORNOW (command_equals)
    Command Mesh_command = commands.None; // FORNOW
    Command Snap_command = commands.None; // FORNOW
    Command Xsel_command = commands.None; // FORNOW
    Command Colo_command = commands.None; // FORNOW

    Event space_bar_event;
    Event shift_space_bar_event;
};

struct PreviewState {
    bbox2 feature_plane;
    real extrude_in_length;
    real extrude_out_length;
    real revolve_in_angle;
    real revolve_out_angle;
    vec3 tubes_color;
    vec3 feature_plane_color;
    real feature_plane_offset;
    vec2 drawing_origin;
    vec2 mouse;
    real cursor_subtext_alpha;

    vec2 popup_second_click;
    vec2 xy_xy;
    vec2 mouse_snap;
    real polygon_num_sides;

    // FORNOW: These are bad names
    vec3 color_mouse;
    vec3 color_draw;
    vec3 color_snap;

    vec2 offset_entity_start;
    vec2 offset_entity_end;
    vec2 offset_entity_middle;
    vec2 offset_entity_opposite;
};

struct Cursors {
    // pass NULL to glfwSetCursor to go back to the arrow
    GLFWcursor *curr;
    GLFWcursor *crosshair;
    GLFWcursor *ibeam;
    GLFWcursor *hresize;
    GLFWcursor *hand;
};

struct ScreenState_ChangesToThisDo_NOT_NeedToBeRecorded_other {
    mat4 OpenGL_from_Pixel;
    mat4 transform_Identity = M4_Identity();

    Cursors cursors;

    Camera camera_drawing;
    Camera camera_mesh;

    bool hide_grid;
    bool show_details;
    bool show_help;
    bool show_event_stack;
    bool hide_toolbox;

    Pane hot_pane;
    real x_divider_drawing_mesh_OpenGL = 0.15f;
    Pane mouse_left_drag_pane;
    Pane mouse_right_drag_pane;

    long timestamp_mouse_left_click;
    bool mouse_double_left_click_held;

    bool shift_held;
    vec2 mouse_OpenGL;
    vec2 mouse_Pixel;

    bool _please_suppress_drawing_popup_popup; // NOTE: THIS IS A TERRRRRIBLE VARIABLE NAME
    bool please_suppress_messagef;
    bool _please_suppress_drawing_toolbox;
    bool _please_suppress_drawing_toolbox_snaps;

    bool slowmo;
    bool paused;
    bool stepping_one_frame_while_paused;

    real time_since_cursor_start;
    real time_since_successful_feature;
    real time_since_plane_selected;
    real time_since_plane_deselected;
    real time_since_going_inside;
    real time_since_mouse_moved;
    real time_since_popup_second_click_not_the_same;


    vec2 snap_divide_dot;
    real size_snap_divide_dot;

    PreviewState preview;

};

struct StandardEventProcessResult {
    bool record_me;
    bool checkpoint_me;
    bool snapshot_me;
};

//////////////////////////////////

////////////////////////////////////////
// colors //////////////////////////////
////////////////////////////////////////

struct {
    #if 0
    vec3 red = RGB255(255, 0, 0);
    vec3 orange = RGB255(204, 136, 1);
    vec3 green = RGB255(83, 255,  85);
    vec3 blue = RGB255(0, 85, 255);
    vec3 purple = RGB255(170, 1, 255);
    vec3 pink = RGB255(238, 0, 119);
    #else
    vec3 red = monokai.red;
    vec3 orange = monokai.orange;
    vec3 green = monokai.green;
    vec3 blue = monokai.blue;
    vec3 purple = monokai.purple;
    vec3 brown = monokai.brown;
    #endif
    vec3 cyan = RGB255(0, 255, 255);
    vec3 magenta = RGB255(255, 0, 255);
    vec3 yellow = RGB255(255, 255, 0);
    vec3 black = RGB255(0, 0, 0);

    vec3 white = RGB255(255, 255, 255);
    vec3 light_gray = RGB255(160, 160, 160);
    vec3 gray = RGB255(115, 115, 115);
    vec3 dark_gray = RGB255(70, 70, 70);
    vec3 darker_gray = RGB255(20, 20, 20);

    vec3 dark_yellow = RGB255(200, 200, 0);
} pallete;

vec3 Q_pallete[10] = {
    #if 0
    pallete.green,
    pallete.red,
    pallete.pink,
    pallete.magenta,
    pallete.purple,
    pallete.blue,
    pallete.gray,
    pallete.light_gray, // TODO: what is this
    pallete.cyan,
    pallete.orange,
    #else
    pallete.light_gray,
    pallete.red,
    pallete.orange,
    pallete.yellow,
    pallete.green,
    pallete.blue,
    pallete.purple,
    pallete.brown,
    pallete.dark_gray,
    #endif
};

vec3 get_accent_color(ToolboxGroup group) {
    vec3 result;
    if (group == ToolboxGroup::Draw) {
        result = V3(0.5f, 1.0f, 1.0f);
    } else if (group == ToolboxGroup::Both) {
        result = V3(0.75f, 1.0f, 0.75f);
    } else if (group == ToolboxGroup::Mesh) {
        result = V3(1.0f, 1.0f, 0.5f);
    } else if (group == ToolboxGroup::Snap) {
        result = V3(1.0f, 0.5f, 1.0f);
    } else if (group == ToolboxGroup::Xsel) {
        result = V3(0.75f, 0.75f, 1.0f);
    } else if (group == ToolboxGroup::Colo) {
        result = pallete.white;
    } else { ASSERT(group == ToolboxGroup::None);
        result = {};
    }
    return result;
}


////////////////////////////////////////
// Config-Tweaks ///////////////////////
////////////////////////////////////////

real Z_FIGHT_EPS = 0.05f;
real TOLERANCE_DEFAULT = 5e-4f;
uint NUM_SEGMENTS_PER_CIRCLE = 64;
real GRID_SIDE_LENGTH = 240.0f;
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

real _WRAP_TO_0_360_INTERVAL(real theta_in_degrees) {
    theta_in_degrees = fmod(theta_in_degrees, 360.0f);
    if (theta_in_degrees < 0.0) theta_in_degrees += 360.0f;
    return theta_in_degrees;
}

bool ANGLE_IS_BETWEEN_CCW(real t, real a, real b) {
    return (WRAP_TO_0_TAU_INTERVAL(t - a) < WRAP_TO_0_TAU_INTERVAL(t - b));
}

bool ANGLE_IS_BETWEEN_CCW_DEGREES(real t, real a, real b) {
    return (WRAP_TO_0_TAU_INTERVAL(RAD(t) - RAD(a)) < WRAP_TO_0_TAU_INTERVAL(RAD(t) - RAD(b)));
}

bool ANGLE_IS_BETWEEN_CCW_DEGREES_TIGHT(real t, real a, real b) {
    return ANGLE_IS_BETWEEN_CCW_DEGREES(t, a + 100 * TINY_VAL, b - 100 * TINY_VAL);
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
    *start_angle = WRAP_TO_0_TAU_INTERVAL(RAD(arc->start_angle_in_degrees));
    *end_angle = WRAP_TO_0_TAU_INTERVAL(RAD(arc->end_angle_in_degrees));
    if (*end_angle < *start_angle) *end_angle += TAU;
    if (flip_flag) { // swap
        real tmp = *start_angle;
        *start_angle = *end_angle;
        *end_angle = tmp;
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


vec3 get_color(ColorCode color_code) {
    uint i = uint(color_code);
    if (0 <= i && i <= 9) {
        return Q_pallete[i];
    } else if (20 <= i && i <= 29) {
        do_once { messagef(pallete.orange, "WARNING: slits not implemented"); };
        return Q_pallete[i - 20];
    } else if (color_code == ColorCode::Selection) {
        return pallete.white;
    } else if (color_code == ColorCode::Emphasis) {
        return pallete.white;
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
    } else if (entity->type == EntityType::Arc) {
        ArcEntity *arc = &entity->arc;
        real start_angle, end_angle;
        arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, false);
        real delta_angle = end_angle - start_angle;
        uint num_segments = uint(1 + (delta_angle / TAU) * 64); // FORNOW: TODO: make dependent on zoom
        real increment = delta_angle / num_segments;
        real current_angle = start_angle;
        for_(i, num_segments) {
            eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, current_angle));
            current_angle += increment;
            eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, current_angle));
        }
    } else { ASSERT(entity->type == EntityType::Circle);
        CircleEntity *circle = &entity->circle;
        uint num_segments = 64;
        real current_angle = 0.0;
        real increment = TAU / num_segments;
        for_(i, num_segments) {
            eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, current_angle));
            current_angle += increment;
            eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, current_angle));
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
    // special case
    if (entity->type == EntityType::Circle) {
        CircleEntity *circle = &entity->circle;
        return
        {
            circle->center - V2(circle->radius),
                circle->center + V2(circle->radius)
        };
    }


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
    bool point_in_sector = ANGLE_IS_BETWEEN_CCW(angle_from_0_TAU(center, p), start_angle_in_radians, end_angle_in_radians);
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

real squared_distance_point_dxf_circle_entity(vec2 p, CircleEntity *circle) {
    return squared_distance_point_circle(p, circle->center, circle->radius);
}

real squared_distance_point_entity(vec2 p, Entity *entity) {
    if (entity->type == EntityType::Line) {
        LineEntity *line = &entity->line;
        return squared_distance_point_dxf_line_entity(p, line);
    } else if (entity->type == EntityType::Arc) {
        ArcEntity *arc = &entity->arc;
        return squared_distance_point_dxf_arc_entity(p, arc);
    } else { ASSERT(entity->type == EntityType::Circle);
        CircleEntity *circle = &entity->circle;
        return squared_distance_point_dxf_circle_entity(p, circle);
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
    // uint index;
    Entity *closest_entity;
    vec2 line_nearest_point;
    real arc_nearest_angle_in_degrees;
    // TODO: t
};
DXFFindClosestEntityResult dxf_find_closest_entity(List<Entity> *entities, vec2 p) {
    DXFFindClosestEntityResult result = {};
    double hot_squared_distance = HUGE_VAL;
    for (Entity *entity = entities->array; entity < entities->array + entities->length; ++entity) {
        double squared_distance = squared_distance_point_entity(p, entity);
        if (squared_distance < hot_squared_distance) {
            hot_squared_distance = squared_distance;
            result.success = true;
            result.closest_entity = entity;
            if (result.closest_entity->type == EntityType::Line) {
                LineEntity *line = &result.closest_entity->line;
                real l2 = squaredDistance(line->start, line->end);
                if (l2 < TINY_VAL) {
                    result.line_nearest_point = line->start;
                } else {
                    real num = dot(p - line->start, line->end - line->start);
                    result.line_nearest_point = CLAMPED_LERP(num / l2, line->start, line->end);
                }
            } else if (result.closest_entity->type == EntityType::Arc) {
                ArcEntity *arc = &result.closest_entity->arc;
                result.arc_nearest_angle_in_degrees = DEG(ATAN2(p - arc->center));
            } else { ASSERT(result.closest_entity->type == EntityType::Circle);
                // CircleEntity *circle = &result.closest_entity->circle;
                // result.arc_nearest_angle_in_degrees = DEG(ATAN2(p - circle->center));
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

                DXFEntityIndexAndFlipFlag *FORNOW_seed = &stretchy_list.array[stretchy_list.length - 1].array[stretchy_list.array[stretchy_list.length - 1].length - 1];

                if (entities->array[FORNOW_seed->entity_index].type != EntityType::Circle) { // continue and complete
                    real tolerance = TOLERANCE_DEFAULT;
                    while (true) {
                        bool added_new_entity_to_loop = false;
                        for_(entity_index, entities->length) {
                            if (entities->array[entity_index].type == EntityType::Circle) continue;
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
                } else if (entity->type == EntityType::Arc) {
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
                } else { ASSERT(entity->type == EntityType::Circle);
                    CircleEntity *circle = &entity->circle;
                    uint num_segments = NUM_SEGMENTS_PER_CIRCLE;
                    for_(i, num_segments) {
                        real angle = real(i) / num_segments * TAU;
                        vec2 p = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle);
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], p);
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
    eso_color(pallete.white);
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

void mesh_bbox_calculate(WorkMesh *mesh) {
    mesh->bbox = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<3>();
    for_(i, mesh->num_vertices) {
        mesh->bbox += mesh->vertex_positions[i];
    }
}

void mesh_triangle_normals_calculate(WorkMesh *mesh) {
    mesh->triangle_normals = (vec3 *) malloc(mesh->num_triangles * sizeof(vec3));
    vec3 p[3];
    for_(i, mesh->num_triangles) {
        for_(d, 3) p[d] = mesh->vertex_positions[mesh->triangle_tuples[i][d]];
        vec3 n = normalized(cross(p[1] - p[0], p[2] - p[0]));
        mesh->triangle_normals[i] = n;
    }
}


// TODO: make a better map


// void mesh_hard_edges_calculate(Meshes *meshes) {
//     // prep a map from edge -> cwiseProduct of face normals (start it at 1, 1, 1) // (faces that edge is part of)
//     // iterate through all edges detministically (ccw in order, flipping as needed so lower_index->higher_index)
//     // then go back and if passes some heuristic add that index to a stretchy buffer
//     List<uint2> list = {}; {
//         WorkMesh *mesh = &meshes->work;
//         Map<uint2, vec3> map = {}; {
//             for_(i, mesh->num_triangles) {
//                 vec3 n = mesh->triangle_normals[i];
//                 uint3 tri = mesh->triangle_tuples[i];
// 
//                 for_(d, 3) {
//                     uint j0 = tri[d];
//                     uint j1 = tri[(d + 1) % 3];
//                     uint2 key = { MIN(j0, j1), MAX(j0, j1) };
// 
//                     map_put(&map, key, cwiseProduct(n, map_get(&map, key, V3(1.0f))));
//                 }
//             }
//         }
//         {
//             // NOTE: this double for loop is just iterating over the map; will need to replace when we upgrade the map
//             for (List<Pair<uint2, vec3>> *bucket = map.buckets; bucket < &map.buckets[map.num_buckets]; ++bucket) {
//                 for (Pair<uint2, vec3> *pair = bucket->array; pair < &bucket->array[bucket->length]; ++pair) {
// 
//                     vec3 n2 = pair->value;
//                     real angle = DEG(acos(n2.x + n2.y + n2.z)); // [0.0f, 180.0f]
//                     if (angle > 30.0f) {
//                         list_push_back(&list, pair->key); // FORNOW
//                     }
// 
//                 }
//             }
//         }
//         map_free_and_zero(&map);
//     }
//     {
//         DrawMesh *mesh = &meshes->draw;
//         mesh->num_hard_edges = list.length;
//         mesh->hard_edges = list.array; // FORNOW: sloppy
//     }
// }

// // TODO: undo and redo are broken now :(

// TODO: this crap is so slow -- O(bad)
// TODO: 
// // TODO: frees (arena?)
// defer  {
//     // queue_free_AND_zero(&queue);
//     // free(visited);
// };
void mesh_divide_into_patches(Meshes *meshes) {
    {
        WorkMesh *work = &meshes->work;
        DrawMesh *draw = &meshes->draw;
        ASSERT(work->num_triangles == draw->num_triangles);
        // if (draw->num_hard_edges == 0) return; // torus shouldn't bother doing this crap
    }

    Arena arena = NEW_BUMP_ALLOCATED_ARENA();
    defer { arena.free(); };


    #if 1

    struct PairPatchIndexOldVertexIndex {
        uint patch_index;
        uint old_vertex_index;
    };


    WorkMesh *old = &meshes->work;
    uint num_triangles = old->num_triangles;


    // prep -- O(t)
    // ------------
    // iterate over all triangles building triangle_index_from_old_half_edge map -- O(t)
    ArenaMap<uint2, uint> triangle_index_from_old_half_edge = { &arena };
    map_reserve_for_expected_num_entries(&triangle_index_from_old_half_edge, 3 * num_triangles);
    {
        for_(triangle_index, num_triangles) {
            uint3 old_triangle_tuple = old->triangle_tuples[triangle_index];
            for_(d, 3) {
                uint i = old_triangle_tuple[ d         ];
                uint j = old_triangle_tuple[(d + 1) % 3];
                uint2 old_half_edge = { i, j }; // NOTE: DON'T sort the edge; we WANT a half-edge
                map_put(&triangle_index_from_old_half_edge, old_half_edge, triangle_index);
            }
        }
    }


    // paint each triangle with its patch index -- O(t)
    // -----------------------------------------------------
    // for each patch warm started search for seed off of finger using patch_index_from_triangle_index for whether seen -- O(t)
    // NOTE: only reads through the entire array of triangles at most once
    //  ++num_patches; -- O(1)
    // (for each triangle) -- O(t)
    //  infect neighbors conditional on whether is hard edge -- O(1)
    //  NOTE: can do the hard edge computation here using the triangle_index_from_old_half_edge map
    //  TODOLATER: also store the hard_edges for tube drawing later
    // NOTE: patch_indices_from_old_vertex_index is for the next step
    // TODO: SmallList

    ArenaMap<uint, uint> patch_index_from_triangle_index = { &arena };
    map_reserve_for_expected_num_entries(&patch_index_from_triangle_index, num_triangles);

    // TODO: ArenaSmallList
    ArenaMap<uint, ArenaList<uint>> patch_indices_from_old_vertex_index = { &arena };
    map_reserve_for_expected_num_entries(&patch_indices_from_old_vertex_index, old->num_vertices);
    {
        for_(old_vertex_index, old->num_vertices) {
            map_put(&patch_indices_from_old_vertex_index, old_vertex_index, { &arena });
        }
    }

    uint num_patches = 0;

    {
        // TODO: ArenaQueue
        Queue<uint> queue = {};

        auto QUEUE_ENQUEUE_AND_MARK = [&](uint triangle_index) {
            queue_enqueue(&queue, triangle_index);

            { // mark
                uint3 old_triangle_tuple = old->triangle_tuples[triangle_index];

                uint patch_index = num_patches;


                map_put(&patch_index_from_triangle_index, triangle_index, patch_index);

                { // patch_indices_from_old_vertex_index
                    for_(d, 3) {
                        uint old_vertex_index = old_triangle_tuple[d];
                        ArenaList<uint> *patch_indices = _map_get_pointer(&patch_indices_from_old_vertex_index, old_vertex_index);
                        // sorted unique push_back
                        bool last_element_is_patch_index; {
                            bool is_empty = (patch_indices->length == 0);
                            if (is_empty) {
                                last_element_is_patch_index = false;
                            } else {
                                last_element_is_patch_index = (patch_indices->_array[patch_indices->length - 1] == patch_index);
                            }
                        }
                        if (!last_element_is_patch_index) patch_indices->push_back(patch_index);
                    }
                }
            }
        };

        uint seed_triangle_index = 0;
        while (true) {
            while (map_contains_key(&patch_index_from_triangle_index, seed_triangle_index)) ++seed_triangle_index;
            if (seed_triangle_index == num_triangles) break;
            QUEUE_ENQUEUE_AND_MARK(seed_triangle_index);
            while (queue.length) {
                uint triangle_index = queue_dequeue(&queue);
                uint3 old_triangle_tuple = old->triangle_tuples[triangle_index];
                for_(d, 3) {
                    uint twin_triangle_index; {
                        uint2 twin_old_half_edge; {
                            uint i = old_triangle_tuple[ d         ];
                            uint j = old_triangle_tuple[(d + 1) % 3];
                            twin_old_half_edge = { j, i };
                        }
                        // NOTE: if this crashes, the mesh wasn't manifold?
                        twin_triangle_index = map_get(&triangle_index_from_old_half_edge, twin_old_half_edge);
                    }
                    bool is_not_already_marked = !map_contains_key(&patch_index_from_triangle_index, twin_triangle_index);
                    bool is_soft_edge; {
                        vec3 n1 = old->triangle_normals[triangle_index];
                        vec3 n2 = old->triangle_normals[twin_triangle_index];
                        // NOTE: clamp ver ver important
                        real angle_in_degrees = DEG(acos(CLAMP(dot(n1, n2), 0.0, 1.0)));
                        ASSERT(!IS_NAN(angle_in_degrees)); // TODO: define your own ACOS that checks
                        is_soft_edge = (angle_in_degrees < 30.0f);
                    }
                    if (is_not_already_marked && is_soft_edge) QUEUE_ENQUEUE_AND_MARK(twin_triangle_index);
                }
            }

            ++num_patches;
        }
    }

    // NOTE: this feels unnecessarily slow
    //       but we didn't know how many patches there were back when we were flooding
    //       (but we do have an upper bound, which is the number of triangles)
    // TODO: consider trading space for time here (after profiling)
    uint *patch_num_vertices = (uint *) arena.calloc(num_patches, sizeof(uint)); {
        for_(old_vertex_index, old->num_vertices) {
            ArenaList<uint> patch_indices = map_get(&patch_indices_from_old_vertex_index, old_vertex_index);
            for_(_patch_index_index, patch_indices.length) {
                uint patch_index = patch_indices[_patch_index_index];
                patch_num_vertices[patch_index]++;
            }
        }
    }

    #if 0

    //                                                         
    // triangle-order preserving division into patches         
    //                                                         
    //                      |         0    6-----8             
    //     0-----4          |        / \    \ C . \            
    //    / \ C . \         |       / A \    \ .   \           
    //   / A \ .   \        |      1-----2    7-----9          
    //  1-----2-----5       |                                  
    //   \ B /              |      3-----4                     
    //    \ /               |       \ B /                      
    //     3                |        \ /                       
    //                      |         5                        
    //                      |                                  
    //                      |          ...:::: patch           
    //                      |       0121230245 vertex          
    // V 012345             |    V  0123456789 new_vertex_index
    //   AAA                |       AAABBBCCCC                 
    //    BBB               |       ^  ^  ^    fingers         
    //   C C CC             |          ... ::: :::             
    //                      |      012 132 024 254             
    // T 012 132 024 254    |    T 012 465 678 798             
    //                                                         

    #endif

    uint new_num_vertices;
    vec3 *new_vertex_positions;
    ArenaMap<PairPatchIndexOldVertexIndex, uint> new_vertex_index_from_pair_patch_index_old_vertex_index = { &arena };
    // FORNOW: This is a huge overestimate; TODO: map that can grow
    map_reserve_for_expected_num_entries(&new_vertex_index_from_pair_patch_index_old_vertex_index, num_patches * old->num_vertices);
    {
        uint *patch_new_vetex_index_fingers; // [ 0, |PATCH0|, |PATCH0| + |PATCH1|, ... ]
        {
            patch_new_vetex_index_fingers = (uint *) arena.calloc(num_patches, sizeof(uint));
            for_(patch_index, num_patches - 1) {
                patch_new_vetex_index_fingers[patch_index + 1] += patch_new_vetex_index_fingers[patch_index];
                patch_new_vetex_index_fingers[patch_index + 1] += patch_num_vertices[patch_index];
            }

            new_num_vertices = patch_new_vetex_index_fingers[num_patches - 1] + patch_num_vertices[num_patches - 1];
        }

        new_vertex_positions = (vec3 *) malloc(new_num_vertices * sizeof(vec3));

        for_(old_vertex_index, old->num_vertices) {
            ArenaList<uint> patch_indices = map_get(&patch_indices_from_old_vertex_index, old_vertex_index);
            for_(_patch_index_index, patch_indices.length) {
                uint patch_index = patch_indices[_patch_index_index];
                uint new_vertex_index = (patch_new_vetex_index_fingers[patch_index])++;

                { // new_vertex_positions
                    vec3 vertex_position = old->vertex_positions[old_vertex_index];
                    new_vertex_positions[new_vertex_index] = vertex_position;
                }

                // new_vertex_index_from_pair_patch_index_old_vertex_index
                PairPatchIndexOldVertexIndex key;
                key.patch_index = patch_index;
                key.old_vertex_index = old_vertex_index;
                map_put(&new_vertex_index_from_pair_patch_index_old_vertex_index, key, new_vertex_index);
            }
        }
    }

    messagef(pallete.blue, "%d", new_num_vertices);



    // build massive new arrays -- O(t)
    // ------------------------
    // malloc huge arrays -- O(t)
    //
    // ?? how do we write the vertices
    //
    // for each triangle -- O(t)
    //   write directly into arrays -- O(1)
    {
        DrawMesh *draw = &meshes->draw;
        draw->num_vertices = new_num_vertices;
        draw->num_triangles = num_triangles;
        draw->vertex_positions = new_vertex_positions;
        draw->triangle_tuples = (uint3 *) malloc(num_triangles * sizeof(uint3));
        for_(triangle_index, num_triangles) {
            uint patch_index = map_get(&patch_index_from_triangle_index, triangle_index);
            uint3 old_triangle_tuple = old->triangle_tuples[triangle_index];
            for_(d, 3) {
                PairPatchIndexOldVertexIndex key;
                key.patch_index = patch_index;
                key.old_vertex_index = old_triangle_tuple[d];
                draw->triangle_tuples[triangle_index][d] = map_get(&new_vertex_index_from_pair_patch_index_old_vertex_index, key);
            }
        }
        // TODOLATER: eliminate triangle_normals
        // FORNOW: just deepcopy from WorkMesh
        draw->triangle_normals = (vec3 *) malloc(old->num_triangles * sizeof(vec3));
        memcpy(draw->triangle_normals, old->triangle_normals, old->num_triangles * sizeof(vec3));

        // TODOLATER
        draw->num_hard_edges   = 0;//new_hard_edges.length;
        draw->hard_edges       = NULL;//new_hard_edges.array;
    }


    #else

    List<vec3> new_vertex_positions = {};
    List<uint3> new_triangle_indices = {};
    List<vec3> new_triangle_normals = {}; // TODO: eliminate this list
    List<uint2> new_hard_edges = {}; // FORNOW: doubling up
    {
        WorkMesh *mesh = &meshes->work;
        uint num_hard_edges = meshes->draw.num_hard_edges;
        uint2 *hard_edges = meshes->draw.hard_edges;

        bool *visited = (bool *) calloc(mesh->num_triangles, sizeof(bool));
        uint num_patches = 0;
        while (true) {
            // flood fill off triangle indices
            List<uint> patch = {};
            List<uint2> patch_hard_edges = {};
            {
                Queue<uint> queue = {};

                auto VISIT = [&](uint triangle_index) {
                    ASSERT(!visited[triangle_index]);
                    visited[triangle_index] = true;
                    patch.push_back(triangle_index);
                    queue_enqueue(&queue, triangle_index);
                };

                { // seed (and if seed fails, break out of while (true); O(num_triangles)
                    bool seeded = false;
                    for_(triangle_index, mesh->num_triangles) {
                        if (!visited[triangle_index]) {
                            // messagef(pallete.yellow, "%d", triangle_index);
                            VISIT(triangle_index);
                            seeded = true;
                            break;
                        }
                    }
                    if (!seeded) break;
                    ++num_patches;
                }

                // flood
                while (queue.length) {
                    uint current_triangle_index = queue_dequeue(&queue);
                    uint3 tri = mesh->triangle_tuples[current_triangle_index];
                    for_(d, 3) {
                        uint i = tri[d];
                        uint j = tri[(d + 1) % 3];
                        uint2 edge = { MIN(i, j), MAX(i, j) };

                        { // skip hard edge; FORNOW: O(num_hard_edges)
                            bool skip_because_hard_edge__NOTE_also_records; {
                                skip_because_hard_edge__NOTE_also_records = false;
                                for_(hard_edge_index, num_hard_edges) {
                                    if (edge == hard_edges[hard_edge_index]) {
                                        skip_because_hard_edge__NOTE_also_records = true;
                                        break;
                                    }
                                }
                            }
                            if (skip_because_hard_edge__NOTE_also_records) {
                                list_push_back(&patch_hard_edges, edge);
                                continue;
                            }
                        }

                        { // flood step; FORNOW: O(num_triangles)
                            for_(cand, mesh->num_triangles) {
                                if (visited[cand]) continue; // NOTE: also handles i == i case
                                uint3 tri2 = mesh->triangle_tuples[cand];
                                for_(d2, 3) {
                                    uint i2 = tri2[d2];
                                    uint j2 = tri2[(d2 + 1) % 3];
                                    uint2 edge2 = { MIN(i2, j2), MAX(i2, j2) };
                                    if (edge == edge2) {
                                        VISIT(cand);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            List<uint> VERTINDEX_old_of_new = {}; {
                // populate list of patch vertices including duplicates
                for_(triangle_index, patch.length) {
                    uint3 tri = mesh->triangle_tuples[patch.array[triangle_index]];
                    for_(d, 3) list_push_back(&VERTINDEX_old_of_new, tri[d]);
                }

                // sort O(n^2)
                qsort(
                        VERTINDEX_old_of_new.array,
                        VERTINDEX_old_of_new.length,
                        sizeof(uint),
                        [](const void *_a, const void *_b) -> int {
                        uint a = *((uint *) _a);
                        uint b = *((uint *) _b);
                        if (a < b) return 1;
                        if (a > b) return -1;
                        return 0;
                        }
                     );

                { // sloppily remove duplicates
                    uint *prev = VERTINDEX_old_of_new.array;
                    for (
                            uint *curr = VERTINDEX_old_of_new.array + 1;
                            curr < VERTINDEX_old_of_new.array + VERTINDEX_old_of_new.length;
                            ++curr
                        ) {
                        if (*curr != *prev) *(++prev) = *curr;
                    }
                    VERTINDEX_old_of_new.length = 1 + (prev - VERTINDEX_old_of_new.array); // update length
                }
            }
            auto VERTINDEX_newOfOld = [&](uint old) -> uint {
                for_(i, VERTINDEX_old_of_new.length) {
                    if (old == VERTINDEX_old_of_new.array[i]) {
                        return (new_vertex_positions.length + i); // !
                    }
                }
                ASSERT(false);
                return 0U;
            };

            uint patch_num_vertices = VERTINDEX_old_of_new.length;
            vec3 *patch_vertex_positions = (vec3 *) malloc(patch_num_vertices * sizeof(vec3));
            for_(vertex_index, patch_num_vertices) {
                patch_vertex_positions[vertex_index] = mesh->vertex_positions[VERTINDEX_old_of_new.array[vertex_index]];
            }

            uint patch_num_triangles = patch.length;
            uint3 *patch_triangle_indices = (uint3 *) malloc(patch_num_triangles * sizeof(uint3));
            for_(triangle_index, patch_num_triangles) {
                uint3 tri = mesh->triangle_tuples[patch.array[triangle_index]];
                for_(d, 3) patch_triangle_indices[triangle_index][d] = VERTINDEX_newOfOld(tri[d]);
            }
            vec3 *patch_triangle_normals = (vec3 *) calloc(patch_num_triangles, sizeof(vec3));
            for_(triangle_index, patch_num_triangles) {
                patch_triangle_normals[triangle_index] = mesh->triangle_normals[patch.array[triangle_index]];
            }

            for_(triangle_index, patch_num_triangles) {
                list_push_back(&new_triangle_indices, patch_triangle_indices[triangle_index]);
                list_push_back(&new_triangle_normals, patch_triangle_normals[triangle_index]);
            }

            // NOTE: A has to go before B
            for_(hard_edge_index, patch_hard_edges.length) { // A
                uint2 edge = patch_hard_edges.array[hard_edge_index];
                for_(d, 2) edge[d] = VERTINDEX_newOfOld(edge[d]);
                list_push_back(&new_hard_edges, edge);
            }

            for_(vertex_index, patch_num_vertices) { // B
                list_push_back(&new_vertex_positions, patch_vertex_positions[vertex_index]);
            }
        }
        messagef(pallete.blue, "%d", num_patches);
    }


    { // FORNOW sloppy
        DrawMesh *mesh = &meshes->draw;
        mesh->num_vertices     = new_vertex_positions.length;
        messagef(pallete.blue, "%d", mesh->num_vertices);
        ASSERT(mesh->num_triangles == new_triangle_indices.length);
        mesh->num_hard_edges   = new_hard_edges.length;
        mesh->vertex_positions = new_vertex_positions.array;
        mesh->triangle_tuples = new_triangle_indices.array;
        mesh->triangle_normals = new_triangle_normals.array;
        mesh->hard_edges       = new_hard_edges.array;
    }

    #endif
}

void mesh_vertex_normals_calculate(DrawMesh *mesh) {
    mesh->vertex_normals = (vec3 *) calloc(mesh->num_vertices, sizeof(vec3));
    for_(triangle_index, mesh->num_triangles) {
        vec3 triangle_normal = mesh->triangle_normals[triangle_index];
        uint3 triangle_ijk = mesh->triangle_tuples[triangle_index];
        real triangle_double_area; {
            vec3 a = mesh->vertex_positions[triangle_ijk[0]];
            vec3 b = mesh->vertex_positions[triangle_ijk[1]];
            vec3 c = mesh->vertex_positions[triangle_ijk[2]];
            vec3 e = (b - a);
            vec3 f = (c - a);
            triangle_double_area = norm(cross(e, f));
        }
        for_(d, 3) {
            uint vertex_index = triangle_ijk[d];
            mesh->vertex_normals[vertex_index] += triangle_double_area * triangle_normal;
        }
    }
    for_(vertex_index, mesh->num_vertices) {
        mesh->vertex_normals[vertex_index] = normalized(mesh->vertex_normals[vertex_index]);
    }
}


void meshes_init(Meshes *meshes, int num_vertices, int num_triangles, vec3 *vertex_positions, uint3 *triangle_tuples) {
    {
        WorkMesh *mesh = &meshes->work;

        mesh->num_triangles = num_triangles;
        mesh->num_vertices = num_vertices;

        mesh->vertex_positions = vertex_positions;
        mesh->triangle_tuples = triangle_tuples;

        mesh_bbox_calculate(&meshes->work);
        mesh_triangle_normals_calculate(&meshes->work);
    }

    {
        DrawMesh *mesh = &meshes->draw;
        mesh->num_vertices = num_vertices; // FORNOW
        mesh->num_triangles = num_triangles;
    }

    {
        // mesh_hard_edges_calculate(meshes);
    }

    {
        #if 1
        mesh_divide_into_patches(meshes);
        #else
        // TODO: support a simple fallback option
        WorkMesh *work = &meshes->work;
        DrawMesh *draw = &meshes->draw;
        draw->num_vertices = work->num_vertices; // FORNOW
        draw->vertex_positions = work->vertex_positions;
        draw->triangle_tuples = work->triangle_tuples;
        draw->triangle_normals = work->triangle_normals;
        #endif
    }

    {
        mesh_vertex_normals_calculate(&meshes->draw);
    }
}

void meshes_free_AND_zero(Meshes *meshes) {
    {
        WorkMesh *mesh = &meshes->work;
        GUARDED_free(mesh->vertex_positions);
        GUARDED_free(mesh->triangle_tuples);
        GUARDED_free(mesh->triangle_normals);
    }

    {
        DrawMesh *mesh = &meshes->draw;
        GUARDED_free(mesh->vertex_positions);
        GUARDED_free(mesh->vertex_normals);
        GUARDED_free(mesh->triangle_tuples);
        GUARDED_free(mesh->triangle_normals);
        GUARDED_free(mesh->hard_edges);
    }

    *meshes = {};
}

// TODO: arenasssssssssss (the mesh needs an arenaaaaaaaaa)

//oh no
#define GUARDED_MALLOC_MEMCPY(dst, src, count, type) \
    do { \
        if (src) { \
            dst = (type *) malloc(count * sizeof(type)); \
            memcpy(dst, src, count * sizeof(type)); \
        } \
    } while (0);

void meshes_deep_copy(Meshes *_dst, Meshes *_src) {
    *_dst = *_src;

    {
        WorkMesh *dst = &_dst->work;
        WorkMesh *src = &_src->work;
        GUARDED_MALLOC_MEMCPY(dst->vertex_positions, src->vertex_positions, src->num_vertices , vec3 );
        GUARDED_MALLOC_MEMCPY(dst->triangle_tuples, src->triangle_tuples, src->num_triangles, uint3);
        GUARDED_MALLOC_MEMCPY(dst->triangle_normals, src->triangle_normals, src->num_triangles, vec3 );
    }

    {
        DrawMesh *dst = &_dst->draw;
        DrawMesh *src = &_src->draw;
        GUARDED_MALLOC_MEMCPY(dst->vertex_positions, src->vertex_positions, src->num_vertices  , vec3 );
        GUARDED_MALLOC_MEMCPY(dst->vertex_normals,   src->vertex_normals,   src->num_vertices  , vec3 );
        GUARDED_MALLOC_MEMCPY(dst->triangle_tuples, src->triangle_tuples, src->num_triangles , uint3);
        GUARDED_MALLOC_MEMCPY(dst->triangle_normals, src->triangle_normals, src->num_triangles , vec3 );
        GUARDED_MALLOC_MEMCPY(dst->hard_edges,       src->hard_edges,       src->num_hard_edges, uint2);
    }
}

////////////////////////////////////////////////////////////////////////////////
// key_lambda //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool _key_lambda(KeyEvent *key_event, uint key, bool control = false, bool shift = false, bool alt = false) {
    ASSERT(!(('a' <= key) && (key <= 'z')));
    bool key_match = (key_event->key == key);
    bool super_match = ((key_event->control && control) || (!key_event->control && !control)); // * bool
    bool shift_match = ((key_event->shift && shift) || (!key_event->shift && !shift)); // * bool
    bool alt_match = ((key_event->alt && alt) || (!key_event->alt && !alt)); // * bool
    return (key_match && super_match && shift_match && alt_match);
};

////////////////////////////////////////////////////////////////////////////////
// world_state /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void world_state_deep_copy(WorldState_ChangesToThisMustBeRecorded_state *dst, WorldState_ChangesToThisMustBeRecorded_state *src) {
    *dst = *src;
    dst->drawing.entities = {};
    list_clone(&dst->drawing.entities, &src->drawing.entities);
    meshes_deep_copy(&dst->meshes, &src->meshes);
}

void world_state_free_AND_zero(WorldState_ChangesToThisMustBeRecorded_state *world_state) {
    meshes_free_AND_zero(&world_state->meshes);
    list_free_AND_zero(&world_state->drawing.entities);
}

////////////////////////////////////////////////////////////////////////////////
// uh oh ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// TODO: don't overwrite  mesh, let the calling code do what it will
// TODO: could this take a printf function pointer?
Meshes wrapper_manifold(
        Meshes *meshes, // dest__NOTE_GETS_OVERWRITTEN,
        uint num_polygonal_loops,
        uint *num_vertices_in_polygonal_loops,
        vec2 **polygonal_loops,
        mat4 M_3D_from_2D,
        Command Mesh_command,
        real out_quantity,
        real in_quantity,
        vec2 dxf_origin,
        vec2 dxf_axis_base_point,
        real dxf_axis_angle_from_y
        ) {

    WorkMesh *mesh = &meshes->work;

    bool add = (command_equals(Mesh_command, commands.ExtrudeAdd)) || (command_equals(Mesh_command, commands.RevolveAdd));
    bool cut = (command_equals(Mesh_command, commands.ExtrudeCut)) || (command_equals(Mesh_command, commands.RevolveCut));
    bool extrude = (command_equals(Mesh_command, commands.ExtrudeAdd)) || (command_equals(Mesh_command, commands.ExtrudeCut));
    bool revolve = (command_equals(Mesh_command, commands.RevolveAdd)) || (command_equals(Mesh_command, commands.RevolveCut));
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
                    (uint *) mesh->triangle_tuples,
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

        ManifoldPolygons *_polygons; {
            _polygons = manifold_polygons(malloc(manifold_polygons_size()), simple_polygon_array, num_polygonal_loops);
        }

        ManifoldCrossSection *cross_section; {
            cross_section = manifold_cross_section_of_polygons(malloc(manifold_cross_section_size()), _polygons, ManifoldFillRule::MANIFOLD_FILL_RULE_EVEN_ODD);
            // cross_section = manifold_cross_section_translate(cross_section, cross_section, -dxf_origin.x, -dxf_origin.y);

            if (revolve) {
                manifold_cross_section_translate(cross_section, cross_section, -dxf_axis_base_point.x, -dxf_axis_base_point.y);
                manifold_cross_section_rotate(cross_section, cross_section, DEG(-dxf_axis_angle_from_y)); // * has both the 90 y-up correction and the angle
            }
        }

        ManifoldPolygons *polygons = manifold_cross_section_to_polygons(malloc(manifold_polygons_size()), cross_section);



        { // manifold_B
            if (command_equals(Mesh_command, commands.ExtrudeCut)) {
                do_once { messagef(pallete.red, "FORNOW ExtrudeCut: Inflating as naive solution to avoid thin geometry."); };
                in_quantity += SGN(in_quantity) * TOLERANCE_DEFAULT;
                out_quantity += SGN(out_quantity) * TOLERANCE_DEFAULT;
            }

            // NOTE: params are arbitrary sign (and can be same sign)--a typical thing would be like (30, -30)
            //       but we support (30, 40) -- which is equivalent to (40, 0)

            if (extrude) {
                real length = in_quantity + out_quantity;
                manifold_B = manifold_extrude(malloc(manifold_manifold_size()), polygons, length, 0, 0.0f, 1.0f, 1.0f);
                manifold_B = manifold_translate(manifold_B, manifold_B, 0.0f, 0.0f, -in_quantity);
            } else { ASSERT(revolve);
                // TODO: M_3D_from_2D 
                real angle_in_degrees = in_quantity + out_quantity;
                manifold_B = manifold_revolve(malloc(manifold_manifold_size()), polygons, NUM_SEGMENTS_PER_CIRCLE, angle_in_degrees);
                manifold_B = manifold_rotate(manifold_B, manifold_B, 0.0, 0.0, -out_quantity); // *
                manifold_B = manifold_rotate(manifold_B, manifold_B, 0.0, DEG(-dxf_axis_angle_from_y), 0.0f); // *
                manifold_B = manifold_rotate(manifold_B, manifold_B, -90.0f, 0.0f, 0.0f);
                manifold_B = manifold_translate(manifold_B, manifold_B, dxf_axis_base_point.x, dxf_axis_base_point.y, 0.0f);
            }
            manifold_B = manifold_translate(manifold_B, manifold_B, -dxf_origin.x, -dxf_origin.y, 0.0f);
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
        manifold_delete_polygons(_polygons);
    }

    Meshes result; { // C <- f(A, B)
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
            uint num_vertices = manifold_meshgl_num_vert(meshgl);
            uint num_triangles = manifold_meshgl_num_tri(meshgl);
            vec3 *vertex_positions = (vec3 *) manifold_meshgl_vert_properties(malloc(manifold_meshgl_vert_properties_length(meshgl) * sizeof(real)), meshgl);
            uint3 *triangle_tuples = (uint3 *) manifold_meshgl_tri_verts(malloc(manifold_meshgl_tri_length(meshgl) * sizeof(uint)), meshgl);
            meshes_init(&result, num_vertices, num_triangles, vertex_positions, triangle_tuples);
        }

        manifold_delete_meshgl(meshgl);
    }

    return result;
}

char *key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(KeyEvent *key_event) { // inline
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

    char *_alt_plus; {
        if (!key_event->alt) {
            _alt_plus = "";
        } else {
            _alt_plus = "ALT+";
        }
    }

    char _key_buffer[2];
    char *_key; {
        if (0) ;
        else if (key_event->key == '\0') _key = "";
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
        else if (key_event->key == GLFW_KEY_PAGE_UP) _key = "PAGE_UP";
        else if (key_event->key == GLFW_KEY_PAGE_DOWN) _key = "PAGE_DOWN";
        else if (key_event->key == GLFW_KEY_HOME) _key = "HOME";
        else if (key_event->key == GLFW_KEY_END) _key = "END";
        else {
            _key_buffer[0] = (char) key_event->key;
            _key_buffer[1] = '\0';
            _key = _key_buffer;
        }
    }

    sprintf(buffer, "%s%s%s%s", _ctrl_plus, _shift_plus, _alt_plus, _key);
    return buffer;
}

////////////////////////////////////////
// intersection ////////////////////////
////////////////////////////////////////


struct LineLineXResult {
    vec2 point;
    real t_ab;
    real t_cd;
    bool point_is_on_segment_ab;
    bool point_is_on_segment_cd;
    bool lines_are_parallel;
};


LineLineXResult line_line_intersection(//vec2 a, vec2 b, vec2 c, vec2 d) {
vec2 p, vec2 p_plus_r, vec2 q, vec2 q_plus_s) {
    // https://stackoverflow.com/a/565282
    vec2 r = p_plus_r - p;
    vec2 s = q_plus_s - q;
    real r_cross_s = cross(r, s);

    LineLineXResult result = {};
    result.lines_are_parallel = ABS(r_cross_s) < 0.0001;
    if (result.lines_are_parallel) {
    } else {
        vec2 q_minus_p = q - p;
        result.t_ab = cross(q_minus_p, s) / r_cross_s;
        result.t_cd = cross(q_minus_p, r) / r_cross_s;
        result.point = p + result.t_ab * r;
        result.point_is_on_segment_ab = IS_BETWEEN_TIGHT(result.t_ab, 0.0f, 1.0f);
        result.point_is_on_segment_cd = IS_BETWEEN_TIGHT(result.t_cd, 0.0f, 1.0f);
    }
    return result;
}

LineLineXResult line_line_intersection(LineEntity *a, LineEntity *b) {
    return line_line_intersection(a->start, a->end, b->start, b->end);
}

struct ArcArcXResult {
    vec2 point1;
    vec2 point2;
    real theta_1a;
    real theta_1b;
    real theta_2a;
    real theta_2b;
    bool point1_is_on_arc_a;
    bool point1_is_on_arc_b;
    bool point2_is_on_arc_a;
    bool point2_is_on_arc_b;
    bool no_possible_intersection;
};

// burkardt is amazing, he even uses our arc struct
ArcArcXResult arc_arc_intersection(ArcEntity *arc_a, ArcEntity *arc_b) {

    ArcArcXResult result = {};

    float d = distance(arc_a->center, arc_b->center);

    //TODO: find fucntion that checks to see if they are close enough for floats
    if (d > arc_a->radius + arc_b->radius) {                // non intersecting
        result.no_possible_intersection = true;
    } else if (d < abs(arc_a->radius-arc_b->radius)) {      // One circle within other
        result.no_possible_intersection = true;
    } else if (d == 0 && arc_a->radius == arc_b->radius) {  // coincident circles
        result.no_possible_intersection = true;
    } else {
        real a = (POW(arc_a->radius, 2) - POW(arc_b->radius, 2) + POW(d, 2)) / (2 * d);
        real h = SQRT(POW(arc_a->radius, 2) - POW(a, 2));

        vec2 v = arc_a->center + a * (arc_b->center - arc_a->center) / d; 

        result.point1 = { v.x + h * (arc_b->center.y - arc_a->center.y) / d, v.y - h * (arc_b->center.x - arc_a->center.x) / d };
        result.point2 = { v.x - h * (arc_b->center.y - arc_a->center.y) / d, v.y + h * (arc_b->center.x - arc_a->center.x) / d };

        result.theta_1a = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(result.point1 - arc_a->center)));
        result.theta_2a = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(result.point2 - arc_a->center)));
        result.theta_1b = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(result.point1 - arc_b->center)));
        result.theta_2b = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(result.point2 - arc_b->center)));


        result.point1_is_on_arc_a = ANGLE_IS_BETWEEN_CCW_DEGREES_TIGHT(result.theta_1a, arc_a->start_angle_in_degrees + TINY_VAL, arc_a->end_angle_in_degrees - TINY_VAL);
        result.point1_is_on_arc_b = ANGLE_IS_BETWEEN_CCW_DEGREES_TIGHT(result.theta_1b, arc_b->start_angle_in_degrees, arc_b->end_angle_in_degrees);
        result.point2_is_on_arc_a = ANGLE_IS_BETWEEN_CCW_DEGREES_TIGHT(result.theta_2a, arc_a->start_angle_in_degrees, arc_a->end_angle_in_degrees);
        result.point2_is_on_arc_b = ANGLE_IS_BETWEEN_CCW_DEGREES_TIGHT(result.theta_2b, arc_b->start_angle_in_degrees, arc_b->end_angle_in_degrees);

        result.no_possible_intersection = false;
    }

    return result;
}

struct LineArcXResult {
    vec2 point1;
    vec2 point2;
    real theta_1;
    real theta_2;
    real t1;
    real t2;
    bool point1_is_on_arc;
    bool point1_is_on_line_segment;
    bool point2_is_on_arc;
    bool point2_is_on_line_segment;
    bool no_possible_intersection;
};

LineArcXResult line_arc_intersection(LineEntity *line, ArcEntity *arc) {
    // using determinant to find num intersects https://www.nagwa.com/en/explainers/987161873194/#:~:text=the%20discriminant%20%ce%94%20%3d%20%f0%9d%90%b5%20%e2%88%92%204,and%20the%20circle%20are%20disjoint.
    LineArcXResult result = {};

    vec2 v1 = line->end - line->start;
    vec2 v2 = line->start - arc->center;

    float a = dot(v1, v1);
    float b = 2 * dot(v1, v2);
    float c = dot(v2, v2) - POW(arc->radius, 2);
    float d = POW(b, 2) - 4 * a * c;


    if (d < 0) {                // no intersect
        result.no_possible_intersection = true; // can we exit early???
    } else {                    // two intersects
        result.t1 = (-b + SQRT(d)) / (2 * a); 
        result.t2 = (-b - SQRT(d)) / (2 * a); 

        result.point1 = line->start + result.t1 * v1;
        result.point2 = line->start + result.t2 * v1;

        result.theta_1 = DEG(angle_from_0_TAU(arc->center, result.point1));
        result.theta_2 = DEG(angle_from_0_TAU(arc->center, result.point2));
        result.point1_is_on_arc = ANGLE_IS_BETWEEN_CCW_DEGREES_TIGHT(result.theta_1, arc->start_angle_in_degrees, arc->end_angle_in_degrees);
        result.point2_is_on_arc = ANGLE_IS_BETWEEN_CCW_DEGREES_TIGHT(result.theta_2, arc->start_angle_in_degrees, arc->end_angle_in_degrees);

        result.point1_is_on_line_segment = IS_BETWEEN_TIGHT(result.t1, 0.0f, 1.0f);
        result.point2_is_on_line_segment = IS_BETWEEN_TIGHT(result.t2, 0.0f, 1.0f);

    }

    return result;
}

struct ArcArcXClosestResult {
    vec2 point;
    real theta_a;
    real theta_b;
    bool point_is_on_arc_a;
    bool point_is_on_arc_b;
    bool no_possible_intersection;
};

ArcArcXClosestResult arc_arc_intersection_closest(ArcEntity *arc_a, ArcEntity *arc_b, vec2 point) {
    ArcArcXClosestResult result;
    ArcArcXResult two_point_result = arc_arc_intersection(arc_a, arc_b);
    if (distance(point, two_point_result.point1) < distance(point, two_point_result.point2)) {
        result.point = two_point_result.point1;
        result.theta_a = two_point_result.theta_1a;
        result.theta_b = two_point_result.theta_1b;
        result.point_is_on_arc_a = two_point_result.point1_is_on_arc_a;
        result.point_is_on_arc_b = two_point_result.point1_is_on_arc_b;
    } else {
        result.point = two_point_result.point2;
        result.theta_a = two_point_result.theta_2a;
        result.theta_b = two_point_result.theta_2b;
        result.point_is_on_arc_a = two_point_result.point2_is_on_arc_a;
        result.point_is_on_arc_b = two_point_result.point2_is_on_arc_b;
    }
    result.no_possible_intersection = two_point_result.no_possible_intersection;
    return result;
}

struct LineArcXClosestResult {
    vec2 point;
    real theta;
    real t;
    bool point_is_on_arc;
    bool point_is_on_line_segment;
    bool no_possible_intersection;
};

LineArcXClosestResult line_arc_intersection_closest(LineEntity *line, ArcEntity *arc, vec2 point) {
    LineArcXClosestResult result;
    LineArcXResult two_point_result = line_arc_intersection(line, arc);
    if (distance(point, two_point_result.point1) < distance(point, two_point_result.point2)) {
        result.point = two_point_result.point1;
        result.theta = two_point_result.theta_1;
        result.t = two_point_result.t1;
        result.point_is_on_arc = two_point_result.point1_is_on_arc;
        result.point_is_on_line_segment = two_point_result.point1_is_on_line_segment;
    } else {
        result.point = two_point_result.point2;
        result.theta = two_point_result.theta_2;
        result.t = two_point_result.t2;
        result.point_is_on_arc = two_point_result.point2_is_on_arc;
        result.point_is_on_line_segment = two_point_result.point2_is_on_line_segment;
    }
    result.no_possible_intersection = two_point_result.no_possible_intersection;
    return result;
}

struct ClosestIntersectionResult {
    vec2 point;
    bool no_possible_intersection;
};

ClosestIntersectionResult closest_intersection(Entity *A, Entity *B, vec2 point) {
    if (A->type == EntityType::Line && B->type == EntityType::Line) {
        LineLineXResult res = line_line_intersection(&A->line, &B->line);
        return { res.point, res.lines_are_parallel}; 
    } else if (A->type == EntityType::Arc && B->type == EntityType::Arc) {
        ArcArcXClosestResult res = arc_arc_intersection_closest(&A->arc, &B->arc, point);
        return { res.point, res.no_possible_intersection };
    } else {
        LineArcXClosestResult res; 
        if (A->type == EntityType::Line) res = line_arc_intersection_closest(&A->line, &B->arc, point);
        else res = line_arc_intersection_closest(&B->line, &A->arc, point);
        return { res.point, res.no_possible_intersection };
    }
}

real get_three_point_angle(vec2 p, vec2 center, vec2 q) {
    real theta_p = angle_from_0_TAU(center, p);
    real theta_q = angle_from_0_TAU(center, q);
    real result = theta_q - theta_p;
    if (result < 0.0f) result += TAU;
    return result;
}
