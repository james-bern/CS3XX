// TODO: what is the simplest way we can allocate memory

// WorldState is a giant struct that occasionally needs to be snapshotted
// the memory that has to be stored not directly in the struct is
// - the drawing
// - the work mesh
// - the draw mesh
// -- NOTE: this may end up being like 3 different meshes (curr, prev, prev_tool)

// the simplest thing may be to just have one giant arena for the entire world state
// it's probably pretty space-inefficient, etc., but it's also very simple

// clearing the mesh is now a little weird (it's still...there in the arena)

// so maybe two arenas?
// - the main thing the arena will be doing for us with the mesh(es) is that there is just soo much alloc'd data, and we don't want to deal with freeing it all

// the thing we still need to figure out is snapshotting
// let's try adding a basic mesh arena
// tttttttt
// what things do we need to do with the meshes?
// - base level: snapshot and then read out of snap shot

// do we really need to snapshot the drawings?--we do, unless you want to start
// repeating stuff from the very beginning
// some drawing stuff can potentially be slow too
// i think assuming all drawing stuff is super fast is potentially a bad idea

// on the other hand, let's think about drawing vs meshes
// drawing is small, changes super often, but changes are generally super quick
// meshes  is large, changes infrequently, but changes are generally super slow
// - turtle is 500 KB and can certainly be made smaller



// FORNOW: let's assume we push the OpenGL crap to the GPU every frame (TODOLATER)

real HARD_EDGE_THRESHOLD_IN_DEGREES = 30.0f;

// gl

void _POOSH(uint *VBO, uint i, uint num_verts, void *array, uint dim, uint GL_TYPE) {
    ASSERT(VBO);
    ASSERT(array);
    ASSERT(num_verts);

    uint sizeof_type; {
        if (GL_TYPE == GL_FLOAT) {
            sizeof_type = sizeof(real);
        } else { ASSERT(GL_TYPE == GL_INT);
            sizeof_type = sizeof(uint);
        }
    }

    glDisableVertexAttribArray(i);
    if (array) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, num_verts * dim * sizeof_type, array, GL_STATIC_DRAW);

        if (GL_TYPE == GL_FLOAT) {
            glVertexAttribPointer (i, dim, GL_TYPE, GL_FALSE, 0, NULL);
        } else { ASSERT(GL_TYPE == GL_INT);
            glVertexAttribIPointer(i, dim, GL_TYPE,           0, NULL);
        }

        glEnableVertexAttribArray(i);
    }
};
void POOSH(uint *VBO, uint i, uint num_verts, vec3 *array) { _POOSH(VBO, i, num_verts, array, 3, GL_FLOAT); }
void POOSH(uint *VBO, uint i, uint num_verts, vec2 *array) { _POOSH(VBO, i, num_verts, array, 2, GL_FLOAT); }
void POOSH(uint *VBO, uint i, uint num_verts, uint *array) { _POOSH(VBO, i, num_verts, array, 1, GL_INT  ); }

uint UNIFORM(uint shader_program, char *name) { return glGetUniformLocation(shader_program, name); };

////////////////////////////////////////
// Forward-Declarations ////////////////
////////////////////////////////////////

void messagef(vec3 color, char *format, ...);
template <typename T> void JUICEIT_EASYTWEEN(T *a, T b, real multiplier = 1.0f);
// TODO: take entire transform (same used for draw) for manifold_wrapper--strip out incremental nature into function



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
    real pseudo_point_angle_in_degrees;

    vec2 get_pseudo_point() {
        vec2 get_point_on_circle_NOTE_pass_angle_in_radians(vec2, real, real); // what a lovely forward declaration
        return get_point_on_circle_NOTE_pass_angle_in_radians(this->center, this->radius, RAD(this->pseudo_point_angle_in_degrees));
    }

    void set_pseudo_point(vec2 pseudo_point) {
        ASSERT(!ARE_EQUAL(this->center, pseudo_point));
        this->has_pseudo_point = true; // seems safer to set this just in case
        this->pseudo_point_angle_in_degrees = DEG(ATAN2(pseudo_point - this->center));
    }
};


struct Entity {
    EntityType type;

    ColorCode color_code;
    bool is_selected;
    real time_since_is_selected_changed;

    LineEntity line;
    ArcEntity arc;
    CircleEntity circle;
};

struct ProtoMesh {
    uint num_vertices;
    uint num_triangles;
    vec3  *vertex_positions;
    uint3 *triangle_tuples;
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
    uint num_triangles; // note: same as workmesh

    vec3  *vertex_positions;
    uint3 *triangle_tuples;
    uint  *vertex_patch_indices;

    vec3  *vertex_normals;
};


struct {
    uint VAO;
    uint VBO[3];
    uint EBO_faces;
    uint EBO_all_edges;
    uint EBO_hard_edges;
} GL;

run_before_main {
    glGenVertexArrays(1, &GL.VAO);
    glGenBuffers(ARRAY_LENGTH(GL.VBO), GL.VBO);
    glGenBuffers(1, &GL.EBO_faces);
    glGenBuffers(1, &GL.EBO_all_edges);
    glGenBuffers(1, &GL.EBO_hard_edges);
};




struct MeshesReadOnly { // FORNOW is this really read only? sort of based on how snapshooting in history works
    Arena *arena;
    WorkMesh work;
    DrawMesh draw;

    DrawMesh tool_draw;
    DrawMesh prev_draw;

    mat4 M_3D_from_2D;
    bool was_cut;

    //     DrawMesh prev;
    //     DrawMesh prev_tool;
    //     bool prev_was_extrude;
    //     bool prev_was_add;
    //     DrawMesh curr;
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

struct TransformMouseDrawingPositionResult {
    vec2 mouse_position; // TODO: change this name to position?
    bool snapped; // TODO: carefully move this to be the first entry
    uint entity_index_snapped_to;
    uint entity_index_intersect;
    uint entity_index_tangent_2;
    bool split_intersect;
    bool split_tangent_2;
};

struct MagicSnapResult3D {
    vec3 mouse_position;
    uint triangle_index;

    bool snapped;
    bool hit_mesh;
};

struct MouseEventDrawing {
    vec2 unsnapped_position;
    bool shift_held;
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
    // Only affect display
    real x_angle;
    real y_angle;
    real offset;

    bool is_active;
    bool mirror_x;
    bool mirror_y;
    vec3 normal;
    real signed_distance_to_world_origin;
};

struct TwoClickCommandState {
    bool awaiting_second_click;
    vec2 first_click;
    Entity *entity_closest_to_first_click;
    Entity *entity_closest_to_second_click;
    bool tangent_first_click; // first thing that came to mind might be more elegant solution
};

struct MeshTwoClickCommandState {
    bool awaiting_second_click;
    vec3 first_click;
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
    uint rcopy_num_total_copies;
    real rcopy_angle;
    real scale_factor;
    _STRING_CALLOC(open_dxf_filename, POPUP_CELL_LENGTH);
    _STRING_CALLOC(save_dxf_filename, POPUP_CELL_LENGTH);
    _STRING_CALLOC(overwrite_dxf_yn_buffer, POPUP_CELL_LENGTH);
    _STRING_CALLOC(open_stl_filename, POPUP_CELL_LENGTH);
    _STRING_CALLOC(save_stl_filename, POPUP_CELL_LENGTH);
    _STRING_CALLOC(overwrite_stl_yn_buffer, POPUP_CELL_LENGTH);
    _STRING_CALLOC(close_confirmation, POPUP_CELL_LENGTH);
};

struct ToolboxState {
    char *hot_name;
};

struct WorldState_ChangesToThisMustBeRecorded_state {
    MeshesReadOnly meshes;
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
    real feature_plane_offset;
    real feature_plane_alpha;
    vec2 drawing_origin;
    real cursor_subtext_alpha;

    vec2 mouse_transformed__PINK_position;
    vec2 mouse_no_snap_potentially_15_deg__WHITE_position;
    vec2 mouse_from_Draw_Enter__BLUE_position;
    // TODO: restore beatiful color lerp from gray to pink when snap becomes active

    vec2 xy_xy;
    real polygon_num_sides;

    real linear_copy_run;
    real linear_copy_rise;
    real linear_copy_num_additional_copies;

    real rcopy_num_total_copies;
    real rcopy_angle;

    vec2 offset_entity_start;
    vec2 offset_entity_end;
    vec2 offset_entity_middle;
    vec2 offset_entity_opposite;

    real bbox_min_y;

    real manifold_wrapper_tweener;
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

    bool awaiting_close_confirmation;

    bool hide_grid;
    bool show_details;
    bool show_console;
    bool show_help;
    bool show_history;
    bool hide_toolbox;
    bool show_debug;

    bool should_feature_plane_be_active;

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
    vec3 lighter_gray = RGB255(235, 235, 235);
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
        result = V3(0.0f, 0.8f, 0.0f);
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
    } else if (entity->type == EntityType::Arc) {
        ArcEntity *arc = &entity->arc;
        real start_angle;
        real end_angle;
        arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, false);
        return ABS(start_angle - end_angle) * arc->radius;
    } else { ASSERT(entity->type == EntityType::Circle);
        return PI * entity->circle.radius * 2;
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

void eso_entity__SOUP_LINES(Entity *entity, bool cageit = false, real z0 = 0.0f, real z1 = 0.0f) {
    auto Q = [&](vec2 a, vec2 b, bool exclude_vertical_a = false, bool exclude_vertical_b = false) {
        if (!cageit) {
            eso_vertex(a);
            eso_vertex(b);
        } else {
            eso_vertex(a.x, a.y, z0);
            eso_vertex(b.x, b.y, z0);

            eso_vertex(a.x, a.y, z1);
            eso_vertex(b.x, b.y, z1);

            if (!exclude_vertical_a) {
                eso_vertex(a.x, a.y, z0);
                eso_vertex(a.x, a.y, z1);
            }

            if (!exclude_vertical_b) {
                eso_vertex(b.x, b.y, z0);
                eso_vertex(b.x, b.y, z1);
            }
        }
    };
    if (entity->type == EntityType::Line) {
        LineEntity *line = &entity->line;
        Q(line->start, line->end);
    } else if (entity->type == EntityType::Arc) {
        ArcEntity *arc = &entity->arc;
        real start_angle, end_angle;
        arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, false);
        real delta_angle = end_angle - start_angle;
        uint num_segments = uint(1 + (delta_angle / TAU) * 64); // FORNOW: TODO: make dependent on zoom
        real increment = delta_angle / num_segments;
        real current_angle = start_angle;
        for_(i, num_segments) {
            Q(
                    get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, current_angle),
                    get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, current_angle + increment),
                    (i != 0) && (i != num_segments / 2),
                    (i != (num_segments - 1))
             );
            current_angle += increment;
        }
    } else { ASSERT(entity->type == EntityType::Circle);
        CircleEntity *circle = &entity->circle;
        uint num_segments = 64;
        real current_angle = 0.0;
        real increment = TAU / num_segments;
        for_(i, num_segments) {
            Q(
                    get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, current_angle), 
                    get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, current_angle + increment),
                    (i % (num_segments / 4) != 0),
                    true
             );
            current_angle += increment;
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
    ManifoldVec2 **polygonal_loops;
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
    List<List<ManifoldVec2>> stretchy_list = {}; {
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
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], { p.x, p.y });
                        current_angle += increment;
                    }
                } else { ASSERT(entity->type == EntityType::Circle);
                    CircleEntity *circle = &entity->circle;
                    uint num_segments = NUM_SEGMENTS_PER_CIRCLE;
                    for_(i, num_segments) {
                        real angle = real(i) / num_segments * TAU;
                        vec2 p = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle);
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], { p.x, p.y });
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
    result.polygonal_loops = (ManifoldVec2 **) calloc(result.num_polygonal_loops, sizeof(ManifoldVec2 *));
    for_(i, result.num_polygonal_loops) {
        result.num_vertices_in_polygonal_loops[i] = stretchy_list.array[i].length;
        result.polygonal_loops[i] = (ManifoldVec2 *) calloc(result.num_vertices_in_polygonal_loops[i], sizeof(ManifoldVec2));
        memcpy(result.polygonal_loops[i], stretchy_list.array[i].array, result.num_vertices_in_polygonal_loops[i] * sizeof(ManifoldVec2));
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
        ManifoldVec2 *polygonal_loop = cross_section->polygonal_loops[loop_index];
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

WorkMesh build_work_mesh_NOTE_shallow_copies_args(
        Arena *arena,
        int num_vertices,
        vec3 *vertex_positions,
        int num_triangles,
        uint3 *triangle_tuples
        ) {

    WorkMesh result = {};

    result.num_triangles = num_triangles; // TODO: store this on meshes

    result.num_vertices = num_vertices;
    result.vertex_positions = vertex_positions;
    result.triangle_tuples = triangle_tuples;

    { // mesh_bbox_calculate(...)
        result.bbox = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<3>();
        for_(i, result.num_vertices) {
            result.bbox += result.vertex_positions[i];
        }
    }

    { // mesh_triangle_normals_calculate(result);
        result.triangle_normals = (vec3 *) arena->malloc(result.num_triangles * sizeof(vec3));
        vec3 p[3];
        for_(i, result.num_triangles) {
            for_(d, 3) p[d] = result.vertex_positions[result.triangle_tuples[i][d]];
            vec3 n = normalized(cross(p[1] - p[0], p[2] - p[0]));
            result.triangle_normals[i] = n;
        }
    }

    #if 0
    { // mesh_translate_to_origin(...);
        vec3 bbox_center = AVG(result.bbox.min, result.bbox.max);
        for_(i, result.num_vertices) result.vertex_positions[i] -= bbox_center;
    }
    #endif

    return result;
}


DrawMesh build_draw_mesh(
        Arena *arena,
        uint work_num_vertices,
        vec3 *work_vertex_positions,
        uint num_triangles,
        uint3 *work_triangle_tuples,
        vec3 *work_triangle_normals
        ) {
    DrawMesh result = {};

    result.num_triangles = num_triangles; // TODO: store this on meshes

    { // mesh_divide_into_patches
        #if 1
        {
            Arena *function_scratch_arena = ARENA_ACQUIRE();
            defer { ARENA_RELEASE(function_scratch_arena); };

            struct PairPatchIndexOldVertexIndex {
                uint patch_index;
                uint old_vertex_index;
            };

            // prep -- O(t)
            // ------------
            // iterate over all triangles building triangle_index_from_old_half_edge map -- O(t)
            ArenaMap<uint2, uint> triangle_index_from_old_half_edge = { function_scratch_arena };
            map_reserve_for_expected_num_entries(&triangle_index_from_old_half_edge, 3 * num_triangles);
            {
                for_(triangle_index, num_triangles) {
                    uint3 old_triangle_tuple = work_triangle_tuples[triangle_index];
                    for_(d, 3) {
                        uint old_i = old_triangle_tuple[ d         ];
                        uint old_j = old_triangle_tuple[(d + 1) % 3];
                        uint2 old_half_edge = { old_i, old_j }; // NOTE: DON'T sort the edge; we WANT a half-edge
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

            ArenaMap<uint, uint> patch_index_from_triangle_index = { function_scratch_arena };
            map_reserve_for_expected_num_entries(&patch_index_from_triangle_index, num_triangles);

            // TODO: ArenaSmallList
            ArenaMap<uint, ArenaList<uint>> patch_indices_from_old_vertex_index = { function_scratch_arena };
            map_reserve_for_expected_num_entries(&patch_indices_from_old_vertex_index, work_num_vertices);
            {
                for_(old_vertex_index, work_num_vertices) {
                    map_put(&patch_indices_from_old_vertex_index, old_vertex_index, { function_scratch_arena });
                }
            }

            uint num_patches = 0;

            {
                // TODO: ArenaQueue
                Queue<uint> queue = {};

                auto QUEUE_ENQUEUE_AND_MARK = [&](uint triangle_index) {
                    queue_enqueue(&queue, triangle_index);

                    { // mark
                        uint3 old_triangle_tuple = work_triangle_tuples[triangle_index];

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
                        uint3 old_triangle_tuple = work_triangle_tuples[triangle_index];
                        for_(d, 3) {
                            uint old_i = old_triangle_tuple[ d         ];
                            uint old_j = old_triangle_tuple[(d + 1) % 3];
                            uint2 old_half_edge = { old_i, old_j };
                            FORNOW_UNUSED(old_half_edge);
                            uint2 twin_old_half_edge = { old_j, old_i };
                            uint twin_triangle_index; {
                                // NOTE: if this crashes, the mesh wasn't manifold?
                                twin_triangle_index = map_get(&triangle_index_from_old_half_edge, twin_old_half_edge);
                            }
                            bool is_not_already_marked = !map_contains_key(&patch_index_from_triangle_index, twin_triangle_index);
                            bool is_soft_edge; {
                                vec3 n1 = work_triangle_normals[triangle_index];
                                vec3 n2 = work_triangle_normals[twin_triangle_index];
                                // NOTE: clamp ver ver important
                                real angle_in_degrees = DEG(acos(CLAMP(dot(n1, n2), 0.0, 1.0)));
                                ASSERT(!IS_NAN(angle_in_degrees)); // TODO: define your own ACOS that checks
                                is_soft_edge = (angle_in_degrees < HARD_EDGE_THRESHOLD_IN_DEGREES);
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
            uint *patch_num_vertices = (uint *) function_scratch_arena->calloc(num_patches, sizeof(uint)); {
                for_(old_vertex_index, work_num_vertices) {
                    ArenaList<uint> patch_indices = map_get(&patch_indices_from_old_vertex_index, old_vertex_index);
                    for_(_patch_index_index, patch_indices.length) {
                        uint patch_index = patch_indices[_patch_index_index];
                        patch_num_vertices[patch_index]++;
                    }
                }
            }

            #if 0 // DRAWING                                           
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
            #endif //                                                  

            uint new_num_vertices;
            vec3 *new_vertex_positions;
            uint *new_vertex_patch_indices;
            ArenaMap<PairPatchIndexOldVertexIndex, uint> new_vertex_index_from_pair_patch_index_old_vertex_index = { function_scratch_arena };
            // FORNOW: This is a huge overestimate; TODO: map that can grow
            map_reserve_for_expected_num_entries(&new_vertex_index_from_pair_patch_index_old_vertex_index, num_patches * work_num_vertices);
            {
                uint *patch_new_vetex_index_fingers; // [ 0, |PATCH0|, |PATCH0| + |PATCH1|, ... ]
                {
                    patch_new_vetex_index_fingers = (uint *) function_scratch_arena->calloc(num_patches, sizeof(uint));
                    for_(patch_index, num_patches - 1) {
                        patch_new_vetex_index_fingers[patch_index + 1] += patch_new_vetex_index_fingers[patch_index];
                        patch_new_vetex_index_fingers[patch_index + 1] += patch_num_vertices[patch_index];
                    }

                    new_num_vertices = patch_new_vetex_index_fingers[num_patches - 1] + patch_num_vertices[num_patches - 1];
                }

                new_vertex_positions = (vec3 *) arena->malloc(new_num_vertices * sizeof(vec3));
                new_vertex_patch_indices = (uint *) arena->malloc(new_num_vertices * sizeof(uint));

                for_(old_vertex_index, work_num_vertices) {
                    ArenaList<uint> patch_indices = map_get(&patch_indices_from_old_vertex_index, old_vertex_index);
                    for_(_patch_index_index, patch_indices.length) {
                        uint patch_index = patch_indices[_patch_index_index];
                        uint new_vertex_index = (patch_new_vetex_index_fingers[patch_index])++;

                        { // new_vertex_positions
                            vec3 vertex_position = work_vertex_positions[old_vertex_index];
                            new_vertex_positions[new_vertex_index] = vertex_position;
                        }

                        // new_vertex_index_from_pair_patch_index_old_vertex_index
                        PairPatchIndexOldVertexIndex key;
                        key.patch_index = patch_index;
                        key.old_vertex_index = old_vertex_index;
                        map_put(&new_vertex_index_from_pair_patch_index_old_vertex_index, key, new_vertex_index);

                        new_vertex_patch_indices[new_vertex_index] = patch_index;
                    }
                }
            }


            // build massive new arrays -- O(t)
            // ------------------------
            // malloc huge arrays -- O(t)
            //
            // ?? how do we write the vertices
            //
            // for each triangle -- O(t)
            //   write directly into arrays -- O(1)
            {
                result.num_vertices = new_num_vertices;
                result.num_triangles = num_triangles;
                result.vertex_positions = new_vertex_positions;
                result.triangle_tuples = (uint3 *) arena->malloc(num_triangles * sizeof(uint3));
                for_(triangle_index, num_triangles) {
                    uint patch_index = map_get(&patch_index_from_triangle_index, triangle_index);
                    uint3 old_triangle_tuple = work_triangle_tuples[triangle_index];
                    for_(d, 3) {
                        PairPatchIndexOldVertexIndex key;
                        key.patch_index = patch_index;
                        key.old_vertex_index = old_triangle_tuple[d];
                        result.triangle_tuples[triangle_index][d] = map_get(&new_vertex_index_from_pair_patch_index_old_vertex_index, key);
                    }
                }
                result.vertex_patch_indices = new_vertex_patch_indices;
            }
        }
        #else
        result.num_vertices = work_num_vertices;
        result.vertex_positions = work_vertex_positions;
        result.triangle_tuples = work_triangle_tuples;
        result.vertex_patch_indices = (uint *) arena->calloc(result.num_vertices, sizeof(uint));
        #endif
    }

    { // mesh_vertex_normals_calculate
        result.vertex_normals = (vec3 *) arena->calloc(result.num_vertices, sizeof(vec3));

        for_(triangle_index, num_triangles) {
            vec3 triangle_normal = work_triangle_normals[triangle_index];
            uint3 triangle_ijk = result.triangle_tuples[triangle_index];
            real triangle_double_area; {
                vec3 a = result.vertex_positions[triangle_ijk[0]];
                vec3 b = result.vertex_positions[triangle_ijk[1]];
                vec3 c = result.vertex_positions[triangle_ijk[2]];
                vec3 e = (b - a);
                vec3 f = (c - a);
                triangle_double_area = norm(cross(e, f));
            }
            for_(d, 3) {
                uint vertex_index = triangle_ijk[d];
                result.vertex_normals[vertex_index] += triangle_double_area * triangle_normal;
            }
        }
        for_(vertex_index, result.num_vertices) {
            result.vertex_normals[vertex_index] = normalized(result.vertex_normals[vertex_index]);
        }

    }

    return result;
}



// TODO: it would feel better to pass around arena pointers instead of values
//       but then where does the arena live?
//       do we need an arena arena?--i think maybe we do.
//       (could be a free list)
MeshesReadOnly build_meshes(Arena *arena, int num_vertices, vec3 *vertex_positions, int num_triangles, uint3 *triangle_tuples) {
    MeshesReadOnly meshes = { arena };
    meshes.work = build_work_mesh_NOTE_shallow_copies_args(arena, num_vertices, vertex_positions, num_triangles, triangle_tuples);
    meshes.draw = build_draw_mesh(arena, num_vertices, vertex_positions, num_triangles, triangle_tuples, meshes.work.triangle_normals);
    return meshes;
}

void meshes_free_AND_zero(MeshesReadOnly *meshes) {
    if (meshes->arena) ARENA_RELEASE(meshes->arena); // FORNOW
    *meshes = {};
}

//oh no

// FORNOW porting this to arenas but hopefully the need for it goes away
void meshes_deep_copy(MeshesReadOnly *dst, MeshesReadOnly *src) {
    *dst = *src;
    dst->arena = ARENA_ACQUIRE();

    #define _DEEP_COPY(name, count, type) \
    do { \
        ASSERT(src); \
        dst->name = (type *) dst->arena->malloc(src->count * sizeof(type)); \
        memcpy(dst->name, src->name, src->count * sizeof(type)); \
    } while (0);

    _DEEP_COPY(work.vertex_positions, work.num_vertices,   vec3);
    _DEEP_COPY(work.triangle_tuples,  work.num_triangles, uint3);
    _DEEP_COPY(work.triangle_normals, work.num_triangles,  vec3);

    _DEEP_COPY(draw.vertex_positions,     draw.num_vertices ,  vec3);
    _DEEP_COPY(draw.triangle_tuples,      draw.num_triangles, uint3);
    _DEEP_COPY(draw.vertex_patch_indices, draw.num_vertices,   uint);
    _DEEP_COPY(draw.vertex_normals,       draw.num_vertices ,  vec3);

    // TODO: these should be guarded

    _DEEP_COPY(tool_draw.vertex_positions,     tool_draw.num_vertices ,  vec3);
    _DEEP_COPY(tool_draw.triangle_tuples,      tool_draw.num_triangles, uint3);
    _DEEP_COPY(tool_draw.vertex_patch_indices, tool_draw.num_vertices,   uint);
    _DEEP_COPY(tool_draw.vertex_normals,       tool_draw.num_vertices ,  vec3);

    _DEEP_COPY(prev_draw.vertex_positions,     prev_draw.num_vertices ,  vec3);
    _DEEP_COPY(prev_draw.triangle_tuples,      prev_draw.num_triangles, uint3);
    _DEEP_COPY(prev_draw.vertex_patch_indices, prev_draw.num_vertices,   uint);
    _DEEP_COPY(prev_draw.vertex_normals,       prev_draw.num_vertices ,  vec3);

    #undef _DEEP_COPY

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

uint fornow_global_selection_num_triangles;
uint3 *fornow_global_selection_triangle_tuples;
vec2 *fornow_global_selection_vertex_positions;


ProtoMesh extract_from_manifold(Arena *arena, ManifoldManifold *manifold) {
    ASSERT(manifold);

    ManifoldMeshGL *meshgl = manifold_get_meshgl(manifold_alloc_meshgl(), manifold);
    defer { manifold_delete_meshgl(meshgl); };

    ProtoMesh result = {};

    result.num_vertices      = (uint  ) manifold_meshgl_num_vert(meshgl);
    result.vertex_positions  = (vec3 *) manifold_meshgl_vert_properties(
            arena->malloc(manifold_meshgl_vert_properties_length(meshgl) * sizeof(real)),
            meshgl
            );

    result.num_triangles     = (uint   ) manifold_meshgl_num_tri(meshgl);
    result.triangle_tuples   = (uint3 *) manifold_meshgl_tri_verts(
            arena->malloc(manifold_meshgl_tri_length(meshgl) * sizeof(uint)),
            meshgl
            );

    return result;
}

// TODO: don't overwrite  mesh, let the calling code do what it will
// TODO: could this take a printf function pointer?
MeshesReadOnly manifold_wrapper(
        WorkMesh *curr,
        DrawMesh *_curr_draw,
        uint num_polygonal_loops,
        uint *num_vertices_in_polygonal_loops,
        ManifoldVec2 **polygonal_loops,
        mat4 M_3D_from_2D,
        Command Mesh_command,
        real out_quantity,
        real in_quantity,
        vec2 dxf_origin,
        vec2 dxf_axis_base_point,
        real dxf_axis_angle_from_y
        ) {

    bool add     = (command_equals(Mesh_command, commands.ExtrudeAdd)) || (command_equals(Mesh_command, commands.RevolveAdd));
    bool cut     = (command_equals(Mesh_command, commands.ExtrudeCut)) || (command_equals(Mesh_command, commands.RevolveCut));
    bool extrude = (command_equals(Mesh_command, commands.ExtrudeAdd)) || (command_equals(Mesh_command, commands.ExtrudeCut));
    bool revolve = (command_equals(Mesh_command, commands.RevolveAdd)) || (command_equals(Mesh_command, commands.RevolveCut));

    ASSERT(    add ||     cut);
    ASSERT(extrude || revolve);

    bool CURR_is_empty = (curr->num_vertices == 0);

    if (CURR_is_empty) ASSERT(!cut);

    // TODO: OK to regress outlining fornow
    ManifoldManifold *manifold_TOOL; // TODO: for visualization, should actually be the intersection of the TOOL and CURR
    ManifoldManifold *manifold_CURR = NULL;
    ManifoldManifold *manifold_NEXT = NULL;
    defer {
        manifold_delete_manifold(manifold_TOOL);
        if (manifold_CURR) manifold_delete_manifold(manifold_CURR);
        if (manifold_NEXT) manifold_delete_manifold(manifold_NEXT);
    };
    {
        { // manifold_TOOL
            ManifoldSimplePolygon **simple_polygon_array;
            defer {
                for_(i, num_polygonal_loops) manifold_delete_simple_polygon(simple_polygon_array[i]);
                free(simple_polygon_array);
            };
            {
                simple_polygon_array = (ManifoldSimplePolygon **) malloc(num_polygonal_loops * sizeof(ManifoldSimplePolygon *));
                for_(i, num_polygonal_loops) {
                    simple_polygon_array[i] = manifold_simple_polygon(
                            manifold_alloc_simple_polygon(),
                            (ManifoldVec2 *) polygonal_loops[i],
                            num_vertices_in_polygonal_loops[i]
                            );
                }
            } 

            ManifoldPolygons *_polygons;
            defer { manifold_delete_polygons(_polygons); };
            {
                _polygons = manifold_polygons(
                        manifold_alloc_polygons(),
                        simple_polygon_array,
                        num_polygonal_loops
                        );
            }


            ManifoldCrossSection *cross_section;
            defer { manifold_delete_cross_section(cross_section); };
            {
                cross_section = manifold_cross_section_of_polygons(manifold_alloc_cross_section(), _polygons, ManifoldFillRule::MANIFOLD_FILL_RULE_EVEN_ODD);

                // cross_section = manifold_cross_section_translate(cross_section, cross_section, -dxf_origin.x, -dxf_origin.y);

                if (revolve) {
                    manifold_cross_section_translate(cross_section, cross_section, -dxf_axis_base_point.x, -dxf_axis_base_point.y);
                    manifold_cross_section_rotate(cross_section, cross_section, DEG(-dxf_axis_angle_from_y)); // * has both the 90 y-up correction and the angle
                }
            }

            ManifoldPolygons *polygons;
            defer { manifold_delete_polygons(polygons); };
            {
                polygons = manifold_cross_section_to_polygons(manifold_alloc_polygons(), cross_section);
            }

            { // manifold_TOOL
                if (command_equals(Mesh_command, commands.ExtrudeCut)) {
                    do_once { messagef(pallete.red, "FORNOW ExtrudeCut: Inflating as naive solution to avoid thin geometry."); };
                    in_quantity += SGN(in_quantity) * TOLERANCE_DEFAULT;
                    out_quantity += SGN(out_quantity) * TOLERANCE_DEFAULT;
                }

                // NOTE: params are arbitrary sign (and can be same sign)--a typical thing would be like (30, -30)
                //       but we support (30, 40) -- which is equivalent to (40, 0)

                if (extrude) {
                    real length = in_quantity + out_quantity;
                    manifold_TOOL = manifold_extrude(manifold_alloc_manifold(), polygons, length, 0, 0.0f, 1.0f, 1.0f);
                    manifold_TOOL = manifold_translate(manifold_TOOL, manifold_TOOL, 0.0f, 0.0f, -in_quantity);
                } else { ASSERT(revolve);
                    // TODO: M_3D_from_2D 
                    real angle_in_degrees = in_quantity + out_quantity;
                    manifold_TOOL = manifold_revolve(manifold_alloc_manifold(), polygons, NUM_SEGMENTS_PER_CIRCLE, angle_in_degrees);
                    manifold_TOOL = manifold_rotate(manifold_TOOL, manifold_TOOL, 0.0, 0.0, -out_quantity); // *
                    manifold_TOOL = manifold_rotate(manifold_TOOL, manifold_TOOL, 0.0, DEG(-dxf_axis_angle_from_y), 0.0f); // *
                    manifold_TOOL = manifold_rotate(manifold_TOOL, manifold_TOOL, -90.0f, 0.0f, 0.0f);
                    manifold_TOOL = manifold_translate(manifold_TOOL, manifold_TOOL, dxf_axis_base_point.x, dxf_axis_base_point.y, 0.0f);
                }
                manifold_TOOL = manifold_translate(manifold_TOOL, manifold_TOOL, -dxf_origin.x, -dxf_origin.y, 0.0f);
                manifold_TOOL = manifold_transform(manifold_TOOL, manifold_TOOL,
                        M_3D_from_2D(0, 0), M_3D_from_2D(1, 0), M_3D_from_2D(2, 0),
                        M_3D_from_2D(0, 1), M_3D_from_2D(1, 1), M_3D_from_2D(2, 1),
                        M_3D_from_2D(0, 2), M_3D_from_2D(1, 2), M_3D_from_2D(2, 2),
                        M_3D_from_2D(0, 3), M_3D_from_2D(1, 3), M_3D_from_2D(2, 3));
            }
        }


        if (!CURR_is_empty) { // manifold_CURR, manifold_NEXT
            { // manifold_CURR
                ManifoldMeshGL *meshgl;
                defer { manifold_delete_meshgl(meshgl); };
                meshgl = manifold_meshgl(
                        manifold_alloc_meshgl(),
                        (real *) curr->vertex_positions,
                        (size_t) curr->num_vertices,
                        3,
                        (uint *) curr->triangle_tuples,
                        curr->num_triangles
                        );

                manifold_CURR = manifold_of_meshgl(manifold_alloc_manifold(), meshgl);
            }
            { // manifold_NEXT
                manifold_NEXT =
                    manifold_boolean(
                            manifold_alloc_manifold(),
                            manifold_CURR,
                            manifold_TOOL,
                            (add) ? ManifoldOpType::MANIFOLD_ADD : ManifoldOpType::MANIFOLD_SUBTRACT
                            );
            }
        }
    }


    MeshesReadOnly result; {
        Arena *arena = ARENA_ACQUIRE();
        result = { arena };

        if (CURR_is_empty) {
            ProtoMesh tool_proto = extract_from_manifold(arena, manifold_TOOL);

            result.work = build_work_mesh_NOTE_shallow_copies_args(arena, tool_proto.num_vertices, tool_proto.vertex_positions, tool_proto.num_triangles, tool_proto.triangle_tuples);
            result.draw = build_draw_mesh(arena, tool_proto.num_vertices, tool_proto.vertex_positions, tool_proto.num_triangles, tool_proto.triangle_tuples, result.work.triangle_normals);
            result.tool_draw = result.draw;
            result.prev_draw = {};
        } else {
            { // next
                ProtoMesh next_proto = extract_from_manifold(arena, manifold_NEXT);
                result.work = build_work_mesh_NOTE_shallow_copies_args(arena, next_proto.num_vertices, next_proto.vertex_positions, next_proto.num_triangles, next_proto.triangle_tuples);
                result.draw = build_draw_mesh(arena, next_proto.num_vertices, next_proto.vertex_positions, next_proto.num_triangles, next_proto.triangle_tuples, result.work.triangle_normals);
            }

            { // tool
                Arena *scratch_arena = ARENA_ACQUIRE();
                defer { ARENA_RELEASE(scratch_arena); };

                ProtoMesh tool_proto = extract_from_manifold(scratch_arena, manifold_TOOL);
                WorkMesh tool_work = build_work_mesh_NOTE_shallow_copies_args(scratch_arena, tool_proto.num_vertices, tool_proto.vertex_positions, tool_proto.num_triangles, tool_proto.triangle_tuples);
                result.tool_draw = build_draw_mesh(arena, tool_proto.num_vertices, tool_proto.vertex_positions, tool_proto.num_triangles, tool_proto.triangle_tuples, tool_work.triangle_normals);
            }

            // prev
            {
                { // FORNOW so much repeated computation
                    Arena *scratch_arena = ARENA_ACQUIRE();
                    defer { ARENA_RELEASE(scratch_arena); };

                    ProtoMesh curr_proto = extract_from_manifold(scratch_arena, manifold_CURR);
                    WorkMesh curr_work = build_work_mesh_NOTE_shallow_copies_args(scratch_arena, curr_proto.num_vertices, curr_proto.vertex_positions, curr_proto.num_triangles, curr_proto.triangle_tuples);
                    result.prev_draw = build_draw_mesh(arena, curr_proto.num_vertices, curr_proto.vertex_positions, curr_proto.num_triangles, curr_proto.triangle_tuples, curr_work.triangle_normals);
                }
                // this crap doesn't work let's just do ^ an extraction and come back to this
                #if 0
                result.prev_draw = *_curr_draw; // TODO shallow copy be very careful with history
                { // FORNOW deepcopy

                    #define _DEEP_COPY(arena, src, dst, field, count, type) \
                    do { \
                        ASSERT(src); \
                        dst->field = (type *) arena->malloc(src->count * sizeof(type)); \
                        memcpy(dst->field, src->field, src->count * sizeof(type)); \
                    } while (0);

                    _DEEP_COPY(arena, (&(result.prev_draw)), _curr_draw, vertex_positions, num_vertices, vec3);
                    _DEEP_COPY(arena, (&(result.prev_draw)), _curr_draw, triangle_tuples, num_triangles, uint3);
                    _DEEP_COPY(arena, (&(result.prev_draw)), _curr_draw, vertex_patch_indices, num_vertices, uint);
                    _DEEP_COPY(arena, (&(result.prev_draw)), _curr_draw, vertex_normals, num_vertices, vec3);

                    #undef _DEEP_COPY
                }
                #endif
            }
        }

        result.M_3D_from_2D = M_3D_from_2D;
        result.was_cut = cut;
        // TODO: revolve stuff
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

ArcEntity get_arc_entity(const Entity* entity) {
    if (entity->type == EntityType::Circle) {
        return {
            entity->circle.center,
                entity->circle.radius,
                0.0f,  // start angle
                180.0f // end angle
        };
    }
    return entity->arc;
}

ClosestIntersectionResult closest_intersection(Entity *A, Entity *B, vec2 point) {
    if (A->type == EntityType::Line && B->type == EntityType::Line) {
        LineLineXResult res = line_line_intersection(&A->line, &B->line);
        return { res.point, res.lines_are_parallel}; 
    } else if ((A->type == EntityType::Arc || A->type == EntityType::Circle) && (B->type == EntityType::Arc || B->type == EntityType::Circle)) {
        ArcEntity arc_a = get_arc_entity(A);
        ArcEntity arc_b = get_arc_entity(B);
        ArcArcXClosestResult res = arc_arc_intersection_closest(&arc_a, &arc_b, point);
        return { res.point, res.no_possible_intersection };
    } else {
        LineArcXClosestResult res; 
        if (A->type == EntityType::Line) {
            ArcEntity arc = get_arc_entity(B);
            res = line_arc_intersection_closest(&A->line, &arc, point);
        } else {
            ArcEntity arc = get_arc_entity(A);
            res = line_arc_intersection_closest(&B->line, &arc, point);
        }
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


Entity _make_line(vec2 start, vec2 end, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
    Entity entity = {};
    entity.type = EntityType::Line;
    entity.color_code = ColorCode::Emphasis;
    LineEntity *line = &entity.line;
    line->start = start;
    line->end = end;
    entity.is_selected = is_selected;
    entity.color_code = color_code;
    return entity;
};

Entity _make_arc(vec2 center, real radius, real start_angle_in_degrees, real end_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
    Entity entity = {};
    entity.type = EntityType::Arc;
    entity.color_code= ColorCode::Emphasis;
    ArcEntity *arc = &entity.arc;
    arc->center = center;
    arc->radius = radius;
    arc->start_angle_in_degrees = start_angle_in_degrees;
    arc->end_angle_in_degrees = end_angle_in_degrees;
    entity.is_selected = is_selected;
    entity.color_code = color_code;
    return entity;
};

Entity _make_circle(vec2 center, real radius, bool has_pseudo_point, real pseudo_point_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
    Entity entity = {};
    entity.type = EntityType::Circle;
    entity.color_code = ColorCode::Emphasis;
    CircleEntity *circle = &entity.circle;
    circle->center = center;
    circle->radius = radius;
    circle->has_pseudo_point = has_pseudo_point;
    circle->pseudo_point_angle_in_degrees = pseudo_point_angle_in_degrees;
    entity.is_selected = is_selected;
    entity.color_code = color_code;
    return entity;
};

typedef struct {
    bool fillet_success;
    Entity ent_one;
    Entity ent_two;
    Entity fillet_arc;
} FilletResult;

FilletResult preview_fillet(const Entity *EntOne, const Entity *EntTwo, vec2 reference_point, real radius) {
    FilletResult fillet_result = {};
    const Entity *E = EntOne;
    const Entity *F = EntTwo;

    if (E == F) {
        messagef(pallete.orange, "Fillet: clicked same entity twice");
        return fillet_result;
    }

    bool pseudoE = false;
    bool pseudoF = false;
    Entity temp1;
    Entity temp2; // it is 1am my brain is very awake
    if (EntOne->type == EntityType::Circle) { // TODO: do better when awake
        temp1 = _make_arc(EntOne->circle.center, EntOne->circle.radius, 0.0f, 359.99f);
        E = &temp1;
        pseudoE = EntOne->circle.has_pseudo_point;
    }
    if (EntTwo->type == EntityType::Circle) {
        temp2 = _make_arc(EntTwo->circle.center, EntTwo->circle.radius, 0.0f, 359.99f);
        F = &temp2;
        pseudoF = EntTwo->circle.has_pseudo_point;
    }



    bool is_line_line = (E->type == EntityType::Line) && (F->type == EntityType::Line);
    bool is_line_arc_or_arc_line = (E->type == EntityType::Line && F->type == EntityType::Arc) || (E->type == EntityType::Arc && F->type == EntityType::Line);
    bool is_arc_arc = (E->type == EntityType::Arc && F->type == EntityType::Arc);


    if (is_line_line) {
        if (distance(E->line.start, E->line.end) < radius) {
            messagef(pallete.orange, "Fillet: first line too short for given radius");
            return fillet_result;
        }
        if (distance(F->line.start, F->line.end) < radius) {
            messagef(pallete.orange, "Fillet: second line too short for given radius");
            return fillet_result;
        }
    }

    if (is_line_line) {
        //  a -- b   x          a -- B-.  
        //                           |  - 
        //           d    =>         X - D
        //    p      |            p      |
        //           c                   c

        vec2 p = reference_point;
        vec2 a;
        vec2 b;
        vec2 c;
        vec2 d;
        vec2 x;
        vec2 e_ab;
        vec2 e_cd;
        {
            a = E->line.start;
            b = E->line.end;
            c = F->line.start;
            d = F->line.end;

            LineLineXResult _x = line_line_intersection(a, b, c, d);
            if (_x.lines_are_parallel) {
                messagef(pallete.orange, "Fillet: lines are parallel");
                return fillet_result;
            }
            x = _x.point;

            e_ab = normalized(b - a);
            e_cd = normalized(d - c);

            bool swap_ab, swap_cd; {
                vec2 v_xp_in_edge_basis = inverse(hstack(e_ab, e_cd)) * (p - x);
                swap_ab = (v_xp_in_edge_basis.x > 0.0f);
                swap_cd = (v_xp_in_edge_basis.y > 0.0f);
            }

            if (swap_ab) {
                {
                    a = b;
                    b = E->line.start;
                }
                e_ab *= -1;
            }

            if (swap_cd) {
                {
                    c = d;
                    d = F->line.start;
                }
                e_cd *= -1;
            }
        }

        { // add new lines and remove old ones
            real L; {
                real full_angle = get_three_point_angle(a, x, c);
                if (full_angle > PI) full_angle = TAU - full_angle;
                L = radius / TAN(full_angle / 2);
            }
            b = x - (L * e_ab);
            d = x - (L * e_cd);
            Entity new_E = _make_line(a, b, E->is_selected, E->color_code);
            Entity new_F = _make_line(c, d, F->is_selected, F->color_code);

            fillet_result.ent_one = new_E;
            fillet_result.ent_two = new_F;
        }

        // deal with creating the fillet arc
        vec2 X; {
            LineLineXResult _X = line_line_intersection(b, b + perpendicularTo(e_ab), d, d + perpendicularTo(e_cd));
            if (_X.lines_are_parallel) {
                messagef(pallete.orange, "Fillet: ???");
                return fillet_result;
            }
            X = _X.point;
        }

        if (!IS_ZERO(radius)) { // arc
            real theta_b_in_degrees;
            real theta_d_in_degrees;
            {
                theta_b_in_degrees = DEG(angle_from_0_TAU(X, b));
                theta_d_in_degrees = DEG(angle_from_0_TAU(X, d));
                if (get_three_point_angle(b, X, d) > PI) {
                    SWAP(&theta_b_in_degrees, &theta_d_in_degrees);
                }
            }
            fillet_result.fillet_arc = _make_arc(X, radius, theta_b_in_degrees, theta_d_in_degrees, false, E->color_code);
        } 

    } else if (is_line_arc_or_arc_line) { // this is a very straight forward function
                                          // general idea
                                          // 1. find where relative to line/arc intersection click is
                                          // 2. use that to get the fillet point
                                          // 3. ?????
                                          // 4, perfect fillet

        const Entity *EntL = E->type == EntityType::Line ? E : F;
        LineEntity line = EntL->line;
        bool swap_happened = EntL == E;

        const Entity *EntA = E->type == EntityType::Arc  ? E : F;
        ArcEntity arc = EntA->arc;

        LineArcXClosestResult intersection = line_arc_intersection_closest(&line, &arc, reference_point);

        if (intersection.no_possible_intersection) {
            messagef(pallete.orange, "FILLET: no intersection found");
            return fillet_result;
        }

        // Determine if fillet should be inside or outside the circle
        real distance_second_click_center = distance(reference_point, arc.center);
        bool fillet_inside_circle = intersection.point_is_on_line_segment && (distance_second_click_center < arc.radius);

        // Get a line parallel to selected to determine where the fillet arc should be
        vec2 line_vector = line.end - line.start;
        bool line_left = cross(line_vector, reference_point - line.start) < 0;
        vec2 line_adjust = radius * normalized(perpendicularTo(line_vector)) * (line_left ? 1.0f : -1.0f);

        LineEntity new_line; // ! color, etc. undefined
        new_line.start = line.start + line_adjust; 
        new_line.end = line.end + line_adjust; 

        // Same thing but for the arc 
        real start_val = dot(normalized(intersection.point - arc.center), normalized(intersection.point - line.start)); 
        real end_val = dot(normalized(intersection.point - arc.center), normalized(intersection.point - line.end));
        bool start_inside_circle = start_val > -TINY_VAL;
        bool end_inside_circle = end_val > -TINY_VAL;
        if (abs(distance(intersection.point, line.start)) < 0.001f) {
            start_inside_circle = end_inside_circle;
        }
        if (abs(distance(intersection.point, line.end)) < 0.001f) {
            end_inside_circle = start_inside_circle;
        }
        if (start_inside_circle == end_inside_circle) { 
            fillet_inside_circle = end_inside_circle;
        }

        ArcEntity new_arc = arc;
        new_arc.radius += radius * (fillet_inside_circle ? -1 : 1);

        // calculate fillet center and intersections
        LineArcXClosestResult fillet_point = line_arc_intersection_closest(&new_line, &new_arc, reference_point);
        vec2 fillet_center = fillet_point.point;
        vec2 line_fillet_intersect = fillet_center - line_adjust;
        vec2 arc_fillet_intersect = fillet_center - radius * (fillet_inside_circle ? -1 : 1) * normalized(fillet_center - arc.center);

        // calculate fillet angles
        real fillet_line_theta = ATAN2(line_fillet_intersect - fillet_center);
        real fillet_arc_theta = ATAN2(arc_fillet_intersect - fillet_center);

        if (fmod(TAU + fillet_line_theta - fillet_arc_theta, TAU) > PI) {
            real temp = fillet_line_theta;
            fillet_line_theta = fillet_arc_theta;
            fillet_arc_theta = temp;
        }

        // make fillet arc
        Entity fillet_arc = _make_arc(fillet_center, radius, DEG(fillet_arc_theta), DEG(fillet_line_theta), false, E->color_code);
        fillet_result.fillet_arc = fillet_arc;

        // determine which end of the line should be changed
        bool end_in_direction = (dot(normalized(fillet_center - intersection.point), normalized(line.end - intersection.point)) > 0);
        bool start_in_direction = (dot(normalized(fillet_center - intersection.point), normalized(line.start - intersection.point)) > 0);
        bool extend_start;
        if (end_in_direction != start_in_direction) {
            extend_start = end_in_direction;
        } else {
            extend_start = distance(intersection.point, line.end) > distance(intersection.point, line.start);
        }

        // handle zero radius case
        if (radius == 0 && (end_inside_circle != start_inside_circle)) {
            extend_start = fillet_inside_circle != start_inside_circle;
        }

        Entity line_to_add;
        // add the new line
        if (extend_start) {
            line_to_add = _make_line(line_fillet_intersect, EntL->line.end, EntL->is_selected, EntL->color_code);
        } else {
            line_to_add = _make_line(EntL->line.start, line_fillet_intersect, EntL->is_selected, EntL->color_code);
        }


        // arc stuff
        real divide_theta = DEG(ATAN2(fillet_center - arc.center));
        real theta_where_line_was_tangent = DEG(ATAN2(line_fillet_intersect - arc.center));

        // kinda weird but checks if divide theta > theta where line was tangent
        vec2 middle_angle_vec = entity_get_middle(&fillet_arc);
        real fillet_middle_arc = DEG(ATAN2(middle_angle_vec - arc.center));

        // this is a slight nudge to ensure that the correct angle is adjusted
        if (ARE_EQUAL(divide_theta, theta_where_line_was_tangent)) {
            real offset = DEG(ATAN2(reference_point - arc.center)); 
            fillet_middle_arc += ANGLE_IS_BETWEEN_CCW_DEGREES(offset, divide_theta, divide_theta + 180.0f) ? -1.0f : 1.0f; 
        }

        // good luck
        const Entity *arc_or_circle = swap_happened ? EntTwo : EntOne;
        Entity new_arc_or_circle;
        if (arc_or_circle->type == EntityType::Circle && !arc_or_circle->circle.has_pseudo_point) {
            new_arc_or_circle = _make_circle(arc_or_circle->circle.center, arc_or_circle->circle.radius, true, divide_theta, arc_or_circle->is_selected, arc_or_circle->color_code);
        } else {
            real start_angle = arc_or_circle->type == EntityType::Circle ? arc_or_circle->circle.pseudo_point_angle_in_degrees : arc_or_circle->arc.start_angle_in_degrees;
            real end_angle = arc_or_circle->type == EntityType::Circle ? arc_or_circle->circle.pseudo_point_angle_in_degrees : arc_or_circle->arc.end_angle_in_degrees;
            if (!(ANGLE_IS_BETWEEN_CCW_DEGREES(divide_theta, arc.end_angle_in_degrees - 0.001f, arc.end_angle_in_degrees + 0.001f) || 
                        ANGLE_IS_BETWEEN_CCW_DEGREES(divide_theta, arc.start_angle_in_degrees - 0.001f, arc.start_angle_in_degrees + 0.001f))) {
                if (ANGLE_IS_BETWEEN_CCW_DEGREES(fillet_middle_arc, arc.start_angle_in_degrees, divide_theta)) {
                    new_arc_or_circle = _make_arc(arc.center, arc.radius, divide_theta, end_angle, arc_or_circle->is_selected, arc_or_circle->color_code);
                } else {
                    new_arc_or_circle = _make_arc(arc.center, arc.radius, start_angle, divide_theta, arc_or_circle->is_selected, arc_or_circle->color_code);
                }
            } else {
                return fillet_result; // TODO: FORNOW: i have no idea what the if statement is for but am scared to delete it
            }
        }

        fillet_result.ent_one = swap_happened ? line_to_add : new_arc_or_circle;
        fillet_result.ent_two = swap_happened ? new_arc_or_circle : line_to_add;

    } else { ASSERT(is_arc_arc);
        ArcEntity arc_a = E->arc;
        ArcEntity arc_b = F->arc;
        real _other_fillet_radius = radius + (radius == 0 ? .001 : 0);

        bool fillet_inside_arc_a = distance(arc_a.center, reference_point) < arc_a.radius;
        bool fillet_inside_arc_b = distance(arc_b.center, reference_point) < arc_b.radius;

        ArcEntity new_arc_a = arc_a;
        new_arc_a.radius = arc_a.radius + (fillet_inside_arc_a ? -1 : 1) * _other_fillet_radius;

        ArcEntity new_arc_b = arc_b;
        new_arc_b.radius = arc_b.radius + (fillet_inside_arc_b ? -1 : 1) * _other_fillet_radius;


        ArcArcXClosestResult fillet_point = arc_arc_intersection_closest(&new_arc_a, &new_arc_b, reference_point);

        if (fillet_point.no_possible_intersection) {
            messagef(pallete.orange, "FILLET: no intersection found");
            return fillet_result;
        }

        vec2 fillet_center = fillet_point.point;
        vec2 arc_a_fillet_intersect = fillet_center - _other_fillet_radius * (fillet_inside_arc_a ? -1 : 1) * normalized(fillet_center - arc_a.center);
        vec2 arc_b_fillet_intersect = fillet_center - _other_fillet_radius * (fillet_inside_arc_b ? -1 : 1) * normalized(fillet_center - arc_b.center);
        real fillet_arc_a_theta = ATAN2(arc_a_fillet_intersect - fillet_center);
        real fillet_arc_b_theta = ATAN2(arc_b_fillet_intersect - fillet_center);

        // a swap so the fillet goes the right way
        // (smallest angle
        if (fmod(TAU + fillet_arc_a_theta - fillet_arc_b_theta, TAU) < PI) {
            real temp = fillet_arc_b_theta;
            fillet_arc_b_theta = fillet_arc_a_theta;
            fillet_arc_a_theta = temp;
        }
        Entity fillet_arc = _make_arc(fillet_center, _other_fillet_radius, DEG(fillet_arc_a_theta), DEG(fillet_arc_b_theta), false, E->color_code); // if this is changed to radius it breaks, dont ask me why
        if (radius > TINY_VAL) {
            fillet_result.fillet_arc = fillet_arc;
        }

        real divide_theta_a = DEG(ATAN2(fillet_center - arc_a.center));
        real divide_theta_b = DEG(ATAN2(fillet_center - arc_b.center));
        if (radius == 0) {
            ArcArcXClosestResult zero_intersect = arc_arc_intersection_closest(&arc_a, &arc_b, reference_point);
            divide_theta_a = zero_intersect.theta_a;
            divide_theta_b = zero_intersect.theta_b;
        }

        vec2 middle_angle_vec = entity_get_middle(&fillet_arc);
        real fillet_middle_arc_a = DEG(ATAN2(middle_angle_vec - arc_a.center));
        real fillet_middle_arc_b = DEG(ATAN2(middle_angle_vec - arc_b.center));

        Entity ent_one_to_add;
        if (EntOne->type == EntityType::Circle && !pseudoE) {
            ent_one_to_add = _make_circle(EntOne->circle.center, EntOne->circle.radius, true, divide_theta_a, EntOne->is_selected, EntOne->color_code); 
        } else {
            real start_angle = EntOne->type == EntityType::Circle ? EntOne->circle.pseudo_point_angle_in_degrees : E->arc.start_angle_in_degrees;
            real end_angle = EntOne->type == EntityType::Circle ? EntOne->circle.pseudo_point_angle_in_degrees : E->arc.end_angle_in_degrees;
            if ((radius == 0) != ANGLE_IS_BETWEEN_CCW_DEGREES(fillet_middle_arc_a, arc_a.start_angle_in_degrees, divide_theta_a)) {
                ent_one_to_add = _make_arc(E->arc.center, E->arc.radius, divide_theta_a, end_angle, E->is_selected, E->color_code);
            } else {
                ent_one_to_add = _make_arc(E->arc.center, E->arc.radius, start_angle, divide_theta_a, E->is_selected, E->color_code);
            }
        }
        fillet_result.ent_one = ent_one_to_add;


        Entity ent_two_to_add;
        if (EntTwo->type == EntityType::Circle && !pseudoF) {
            ent_two_to_add = _make_circle(EntTwo->circle.center, EntTwo->circle.radius, true, divide_theta_b, EntTwo->is_selected, EntTwo->color_code); 
        } else {
            real start_angle_in_degrees = EntTwo->type == EntityType::Circle ? EntTwo->circle.pseudo_point_angle_in_degrees : F->arc.start_angle_in_degrees;
            real end_angle_in_degrees = EntTwo->type == EntityType::Circle ? EntTwo->circle.pseudo_point_angle_in_degrees : F->arc.end_angle_in_degrees;
            if ((radius == 0) != ANGLE_IS_BETWEEN_CCW_DEGREES(fillet_middle_arc_b, arc_b.start_angle_in_degrees, divide_theta_b)) {
                ent_two_to_add = _make_arc(F->arc.center, F->arc.radius, divide_theta_b, end_angle_in_degrees, F->is_selected, F->color_code);
            } else {
                ent_two_to_add = _make_arc(F->arc.center, F->arc.radius, start_angle_in_degrees, divide_theta_b, F->is_selected, F->color_code);
            }
        }
        fillet_result.ent_two = ent_two_to_add;

    }

    // least sus thing ever
    fillet_result.fillet_success = true;

    return fillet_result;
}

typedef struct {
    bool dogear_success;
    Entity ent_one;
    Entity ent_two;
    Entity fillet_arc_one;
    Entity fillet_arc_two;
    Entity dogear_arc_one;
    Entity dogear_arc_two;
} DogEarResult;

DogEarResult preview_dogear(Entity *E, Entity *F, vec2 reference_point, real radius) {
    DogEarResult dogear_result = {};

    if (E == F) {
        messagef(pallete.orange, "DogEar: clicked same entity twice");
        return dogear_result;
    }

    if (IS_ZERO(radius)) {
        messagef(pallete.orange, "DogEar: FORNOW: must have non-zero radius");
        return dogear_result;
    }

    bool is_line_line = (E->type == EntityType::Line) && (F->type == EntityType::Line);
    if (!is_line_line) {
        messagef(pallete.orange, "DogEar: only line-line is supported");
        return dogear_result;
    }

    //                                    ,--.
    //  a -- b      x          a -- b    e     x
    //                                  :   y  :
    //                                  ,      f
    //                   =>              +.__.'
    //    p         d            p             d
    //              |                          |
    //              c                          c

    // FORNOW: this block is repeated from fillet
    vec2 p = reference_point;
    vec2 a;
    vec2 b;
    vec2 c;
    vec2 d;
    vec2 x;
    vec2 e_ab;
    vec2 e_cd;
    // vec2 *b_ptr;
    // vec2 *d_ptr;
    {
        a     =  E->line.start;
        b     =  E->line.end;
        // b_ptr = &E->line.end;
        c     =  F->line.start;
        d     =  F->line.end;
        // d_ptr = &F->line.end;

        LineLineXResult _x = line_line_intersection(a, b, c, d);
        if (_x.lines_are_parallel) {
            messagef(pallete.orange, "DogEar: lines are parallel");
            return dogear_result;
        }
        x = _x.point;

        e_ab = normalized(b - a);
        e_cd = normalized(d - c);

        bool swap_ab, swap_cd; {
            vec2 v_xp_in_edge_basis = inverse(hstack(e_ab, e_cd)) * (p - x);
            swap_ab = (v_xp_in_edge_basis.x > 0.0f);
            swap_cd = (v_xp_in_edge_basis.y > 0.0f);
        }

        if (swap_ab) {
            SWAP(&a, &b);
            e_ab *= -1;
            // b_ptr = &E->line.start;
        }

        if (swap_cd) {
            SWAP(&c, &d);
            e_cd *= -1;
            // d_ptr = &F->line.start;
        }
    }

    vec2 y;
    vec2 e_xy;
    real theta_yx_in_degrees;
    {
        e_xy = -normalized(e_ab + e_cd);
        y = x + radius * e_xy;
        theta_yx_in_degrees = DEG(ATAN2(-e_xy));
    }

    Entity G = _make_arc(y, radius, theta_yx_in_degrees - 180.0f, theta_yx_in_degrees + 180.0f, false, E->color_code);

    vec2 e;
    vec2 f;
    {
        // FORNOW: sloppy (use of a, c is wrong i think)
        LineArcXClosestResult _e = line_arc_intersection_closest(&E->line, &G.arc, a);
        ASSERT(!_e.no_possible_intersection);
        e = _e.point;
        LineArcXClosestResult _f = line_arc_intersection_closest(&F->line, &G.arc, c);
        ASSERT(!_f.no_possible_intersection);
        f = _f.point;
    }

    FilletResult fillet_one = preview_fillet(E, &G, e + (e - y), radius);
    FilletResult fillet_two = preview_fillet(F, &fillet_one.ent_two, f + (f - y), radius);

    dogear_result.ent_one = fillet_one.ent_one;
    dogear_result.ent_two = fillet_two.ent_one;
    dogear_result.fillet_arc_one = fillet_one.fillet_arc;
    dogear_result.fillet_arc_two = fillet_two.fillet_arc;


    // // split arc version
    Entity G1;
    Entity G2;
    {
        G1 = fillet_two.ent_two;
        G2 = fillet_two.ent_two;
        real half_theta_in_degrees =  0.5f * _WRAP_TO_0_360_INTERVAL(fillet_two.ent_two.arc.end_angle_in_degrees - fillet_two.ent_two.arc.start_angle_in_degrees);
        G1.arc.end_angle_in_degrees -= half_theta_in_degrees;
        G2.arc.start_angle_in_degrees = G1.arc.end_angle_in_degrees;
    }
    dogear_result.dogear_arc_one = G1;
    dogear_result.dogear_arc_two = G2;

    return dogear_result;

}
