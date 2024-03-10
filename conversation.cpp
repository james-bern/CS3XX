// TODO: (M)easure

// TODO: get revolve working

// XXXX move origin to MIDDLE_OF
// XXXX when moving draw preview (TODO including filtering from snaps)
// XXXX: separate origin_x, origin_y from M_3D_from_2D (see if there's still sloppy inversion of the origin_xy piece in your code)

// TODO way to save state to inspect later



// TODO: code plays itself (testing stack) -- ? way of saving these comand chains


// custom shader
// you could redo the entire stack with finer circle discretization params

// SUMMER
// TODO fillets
// TODO: speed up dxf_loop_analysis_create_FORNOW_QUADRATIC


#include "cs345.cpp"
#include "manifoldc.h"
#include "poe.cpp"
#undef real // ???
#define u32 DO_NOT_USE_u32_USE_uint32_INSTEAD
#include "conversation.h"



char conversation_message_buffer[256];
uint32 conversation_message_cooldown;
void conversation_messagef(char *format, ...) {
    va_list arg;
    va_start(arg, format);
    vsnprintf(conversation_message_buffer, sizeof(conversation_message_buffer), format, arg);
    va_end(arg);
    conversation_message_cooldown = 300;
    printf("%s\n", conversation_message_buffer);
}
void conversation_message_buffer_update_and_draw() {
    if (conversation_message_cooldown > 0) {
        --conversation_message_cooldown;
    } else {
        conversation_message_buffer[0] = '\0';
    }
    gui_printf("< %s", conversation_message_buffer);
}




// TODO: could this take a printf function pointer?
void wrapper_manifold(
        ManifoldManifold **manifold_manifold,
        FancyMesh *fancy_mesh, // dest__NOTE_GETS_OVERWRITTEN,
        uint32 num_polygonal_loops,
        uint32 *num_vertices_in_polygonal_loops,
        vec2 **polygonal_loops,
        mat4 M_3D_from_2D,
        uint32 enter_mode,
        real32 console_param,
        real32 console_param_2,
        real32 dxf_origin_x,
        real32 dxf_origin_y) {
    // FORNOW: this function call isn't a no-op
    // history_record_state(history, manifold_manifold, fancy_mesh);
    ASSERT(enter_mode != ENTER_MODE_NONE);

    ManifoldManifold *other_manifold; {
        ManifoldSimplePolygon **simple_polygon_array = (ManifoldSimplePolygon **) malloc(num_polygonal_loops * sizeof(ManifoldSimplePolygon *));
        for (uint32 i = 0; i < num_polygonal_loops; ++i) {
            simple_polygon_array[i] = manifold_simple_polygon(malloc(manifold_simple_polygon_size()), (ManifoldVec2 *) polygonal_loops[i], num_vertices_in_polygonal_loops[i]);
        }
        ManifoldPolygons *polygons = manifold_polygons(malloc(manifold_polygons_size()), simple_polygon_array, num_polygonal_loops);
        ManifoldCrossSection *cross_section = manifold_cross_section_of_polygons(malloc(manifold_cross_section_size()), polygons, ManifoldFillRule::MANIFOLD_FILL_RULE_EVEN_ODD);

        cross_section = manifold_cross_section_translate(cross_section, cross_section, -dxf_origin_x, -dxf_origin_y);

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
            } else {
                // TODO: M_3D_from_2D 
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
    fancy_mesh->triangle_indices = manifold_meshgl_tri_verts(malloc(manifold_meshgl_tri_length(meshgl) * sizeof(uint32)), meshgl);
    fancy_mesh_triangle_normals_calculate(fancy_mesh);
    fancy_mesh_cosmetic_edges_calculate(fancy_mesh);
}







////////////////////////////////////////////////////
// GLOBALS (TODO: wrap in struct ConversationState { ... } conversation;
////////////////////////////////////////////////////

Camera2D camera_2D;
Camera3D camera_3D;

bool32 hide_grid, show_details, show_help, show_stack;


DXF dxf;
DXFLoopAnalysisResult pick;
BoundingBox *bbox;
BoundingBox bbox_union;
bool32 *dxf_selection_mask;
real32 dxf_origin_x;
real32 dxf_origin_y;


ManifoldManifold *manifold_manifold;
FancyMesh fancy_mesh;
uint32 enter_mode;
uint32 click_mode;
uint32 click_modifier; // TODO combine

char console_buffer[256];
char *console_buffer_write_head;
void conversation_console_buffer_reset() {
    memset(console_buffer, 0, ARRAY_LENGTH(console_buffer) * sizeof(char));
    console_buffer_write_head = console_buffer;
};

real32 console_param;
real32 console_param_2;
bool32 console_params_preview_flip_flag;
real32 console_param_preview;
real32 console_param_2_preview;
void console_params_preview_update() {
    char buffs[2][64] = {};
    uint32 buff_i = 0;
    uint32 i = 0;
    for (char *c = console_buffer; (*c) != '\0'; ++c) {
        if (*c == ',') {
            if (++buff_i == 2) break;
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






uint32 hot_pane;
uint32 window_select_click_count;
real32 window_select_x;
real32 window_select_y;


int32 selected_index_of_first_triangle_hit_by_ray; // NOTE: if this is false, then a plane is selected
vec3 n_selected;
real32 r_n_selected; // coordinate along n_selected
mat4 M_3D_from_2D;
void conversation_feature_plane_reset() {
    selected_index_of_first_triangle_hit_by_ray = -1;
    r_n_selected = 0.0f;
    n_selected = {}; // FORNOW??: implicit nothing selected
    M_3D_from_2D = {};
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

    M_3D_from_2D = M4_xyzo(x, y, z, (r_n_selected) * n_selected);
}

char conversation_current_dxf_filename[512];


bool32 click_move_origin_mouse_moved_since_last_key_press_M;
real32 click_move_origin_preview_x;
real32 click_move_origin_preview_y;


// GLOBAL-TOUCHING FUNCTIONS ///////////////////////////

// TODO: see which of these can be easily pulled out into non-global touching

void conversation_dxf_reset() {
    memset(dxf_selection_mask, 0, dxf.num_entities * sizeof(bool32));
    dxf_origin_x = 0.0f;
    dxf_origin_y = 0.0f;
}

void conversation_dxf_load(char *filename, bool preserve_cameras = false) {
    if (!poe_file_exists(filename)) {
        conversation_messagef("[load] \"%s\" not found", filename);
        return;
    }
    {
        dxf_free(&dxf);
        dxf_loop_analysis_free(&pick);
        free(dxf_selection_mask);
        free(bbox);

        dxf_load(filename, &dxf);
        dxf_selection_mask = (bool32 *) calloc(dxf.num_entities, sizeof(bool32));
        pick = dxf_loop_analysis_create_FORNOW_QUADRATIC(&dxf);
        bbox = dxf_entity_bounding_boxes_create(&dxf);
        bbox_union = bounding_box_union(dxf.num_entities, bbox);
        if (!preserve_cameras) camera2D_zoom_to_bounding_box(&camera_2D, bbox_union);
        conversation_dxf_reset();
        strcpy(conversation_current_dxf_filename, filename);

        conversation_messagef("[load] loaded %s", filename);
    }
}
void conversation_stl_load(char *filename, bool preserve_cameras = false) {
    if (!poe_file_exists(filename)) {
        conversation_messagef("[load] \"%s\" not found", filename);
        return;
    }
    {
        stl_load(filename, &manifold_manifold, &fancy_mesh);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(preserve_cameras);
        conversation_messagef("[load] loaded %s", filename);
    }
}
void conversation_load(char *filename, bool preserve_cameras = false) {
    if (poe_suffix_match(filename, ".dxf")) {
        conversation_dxf_load(filename, preserve_cameras);
    } else if (poe_suffix_match(filename, ".stl")) {
        conversation_stl_load(filename, preserve_cameras);
    } else {
        conversation_messagef("[load] \"%s\" fileype not supported; must be *.dxf or *.stl", filename);
    }
}
void conversation_stl_save(char *filename) {
    // TODO: prompt for overwriting
    if (fancy_mesh_save_stl(&fancy_mesh, filename) ) {
        conversation_messagef("[save] saved %s", filename);
    } else {
        conversation_messagef("[save] could not save open %s for writing.", filename);
    }
}
void conversation_save(char *filename) {
    if (poe_suffix_match(filename, ".stl")) {
        conversation_stl_save(filename);
    } else {
        conversation_messagef("[save] \"%s\" filetype not supported; must be *.stl", filename);
    }
}


char conversation_drop_path[512];
void drop_callback(GLFWwindow *, int count, const char **paths) {
    if (count > 0) {
        char *filename = (char *) paths[0];
        conversation_load(filename);
        { // conversation_set_drop_path(filename)
            for (uint32 i = 0; i < strlen(filename); ++i) {
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
    }
}
BEGIN_PRE_MAIN {
    glfwSetDropCallback(COW0._window_glfw_window, drop_callback);
} END_PRE_MAIN;

// fancy_mesh;
// manifold_manifold;


void conversation_cameras_reset() {
    camera_2D = { 0.0f, 0.0, 0.0f, -0.5f, -0.125f };
    camera2D_zoom_to_bounding_box(&camera_2D, bbox_union);
    camera_3D = { 2.0f * camera_2D.screen_height_World, CAMERA_3D_DEFAULT_ANGLE_OF_VIEW, RAD(33.0f), RAD(-44.0f), 0.0f, 0.0f, 0.5f, -0.125f };
}


ManifoldManifold *_reset_manifold_manifold;
FancyMesh _reset_fancy_mesh;
void conversation_save_reset_manifold_manifold_and_fancy_mesh() {
    // FORNOW SHIM SHIM SHIM
    _reset_manifold_manifold = manifold_manifold;
    _reset_fancy_mesh = fancy_mesh;
}
void conversation_load_reset_manifold_manifold_and_fancy_mesh() {
    // FORNOW SHIM SHIM SHIM
    manifold_manifold = _reset_manifold_manifold;
    fancy_mesh = _reset_fancy_mesh;
}



void conversation_reset(bool32 disable_top_layer_resets = false) {
    click_mode = CLICK_MODE_NONE;
    click_modifier = CLICK_MODIFIER_NONE;
    enter_mode = ENTER_MODE_NONE;

    console_param = 0.0f;
    console_param_2 = 0.0f;
    console_params_preview_flip_flag = false;

    conversation_dxf_reset();
    conversation_feature_plane_reset(); 
    conversation_console_buffer_reset();

    conversation_load_reset_manifold_manifold_and_fancy_mesh();

    if (!disable_top_layer_resets) {
        hot_pane = HOT_PANE_NONE;
        hide_grid = false;
        show_details = false;
        show_stack = false;
        show_help = false;
        conversation_cameras_reset();
    }
}

void conversation_init() {
    conversation_dxf_load("splash.dxf", true);

    manifold_manifold = NULL;
    fancy_mesh = {};

    conversation_reset();
}




// IDEA: mouse stuff should be immediately translated into world coordinates (by the callback)
//       the camera should NOT be part of the state; it should be just a pre-filter on user input
// (otherwise...we have to store all the clicking and dragging of the cameras -- which is actually quite involved)
// but the cameras just map the click from NDC to world
// - in 2D, a world point
// - in 3D, a world ray
// advantages: fastest path to goal (avoids having to do weird porting of camera math); avoids having to push a bunch of ui events (camera stuff can be treated separately)
// disadvantage: a user input event now has to be able to store an entire ray (6 real's)

// buffer -> # # # # # # 
//                       
// .                     
// ^                     
// |                     
// A,B,C                 

// / / / / / / / / / / / / / / / / / / / / / / / $ $ $ $ $ $ . . . . . .
// ^                                             ^           ^          
// |                                             |           |          
// A                                             B           C          

// / / / / / / / / / / / / / / / / / / / / / / / / / / / / / .    
// ^                                                         ^    
// |                                                         |    
// 0                                                         A,B,C

#define UI_EVENT_TYPE_KEY_PRESS      0
#define UI_EVENT_TYPE_MOUSE_2D_PRESS 1
#define UI_EVENT_TYPE_MOUSE_3D_PRESS 2

// TODO: why is undo broken now
#define PROCESSED_EVENT_CATEGORY_DONT_RECORD              0
#define PROCESSED_EVENT_CATEGORY_RECORD                   1
#define PROCESSED_EVENT_CATEGORY_CHECKPOINT               2
#define PROCESSED_EVENT_CATEGORY_SELF_AND_PREV_CHECKPOINT 3
#define PROCESSED_EVENT_CATEGORY_KILL_HISTORY             4

struct Event {
    uint32 type;
    union {
        struct {
            uint32 key;
            uint32 mods;
        };
        struct {
            real32 mouse_x;
            real32 mouse_y;
        };
        struct {
            vec3 o;
            vec3 dir;
        };
    }; 
    bool32 checkpoint;
};


Queue<Event> new_event_queue;

void callback_key(GLFWwindow *, int key, int, int action, int mods) {
    // _callback_key(NULL, key, 0, action, mods); // FORNOW TODO TODO TODO SHIM
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_LEFT_CONTROL) return;
        if (key == GLFW_KEY_RIGHT_CONTROL) return;
        if (key == GLFW_KEY_LEFT_SUPER) return;
        if (key == GLFW_KEY_RIGHT_SUPER) return;
        Event event = {};
        event.type = UI_EVENT_TYPE_KEY_PRESS;
        event.key = key;
        event.mods = mods;
        queue_enqueue(&new_event_queue, event);
    }
}

struct {
    real32 mouse_x;
    real32 mouse_y;
    vec3 o;
    vec3 dir;
} world_cursor;

void snap_map(real32 before_x, real32 before_y, real32 *after_x, real32 *after_y) {
    real32 tmp_x = before_x;
    real32 tmp_y = before_y;
    {
        if (click_modifier == CLICK_MODIFIER_CENTER_OF) {
            real32 min_squared_distance = HUGE_VAL;
            for (DXFEntity *entity = dxf.entities; entity < &dxf.entities[dxf.num_entities]; ++entity) {
                if (entity->type == DXF_ENTITY_TYPE_LINE) {
                    continue;
                } else {
                    ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                    DXFArc *arc = &entity->arc;
                    real32 squared_distance = squared_distance_point_dxf_arc(before_x, before_y, arc);
                    if (squared_distance < min_squared_distance) {
                        min_squared_distance = squared_distance;
                        tmp_x = arc->center_x;
                        tmp_y = arc->center_y;
                    }
                }
            }
        } else if (click_modifier == CLICK_MODIFIER_MIDDLE_OF) {
            real32 min_squared_distance = HUGE_VAL;
            real32 middle_x;
            real32 middle_y;
            for (DXFEntity *entity = dxf.entities; entity < &dxf.entities[dxf.num_entities]; ++entity) {
                real32 squared_distance = squared_distance_point_dxf_entity(before_x, before_y, entity);
                if (squared_distance < min_squared_distance) {
                    min_squared_distance = squared_distance;
                    entity_get_middle(entity, &middle_x, &middle_y);
                    tmp_x = middle_x;
                    tmp_y = middle_y;
                }
            }
        } else if (click_modifier == CLICK_MODIFIER_END_OF) {
            real32 min_squared_distance = HUGE_VAL;
            for (DXFEntity *entity = dxf.entities; entity < &dxf.entities[dxf.num_entities]; ++entity) {
                real32 x[2], y[2];
                entity_get_start_and_end_points(entity, &x[0], &y[0], &x[1], &y[1]);
                for (uint32 d = 0; d < 2; ++d) {
                    real32 squared_distance = squared_distance_point_point(before_x, before_y, x[d], y[d]);
                    if (squared_distance < min_squared_distance) {
                        min_squared_distance = squared_distance;
                        tmp_x = x[d];
                        tmp_y = y[d];
                    }
                }
            }
        }
    }
    *after_x = tmp_x;
    *after_y = tmp_y;
}

void callback_cursor_position(GLFWwindow *, double xpos, double ypos) {
    _callback_cursor_position(NULL, xpos, ypos); // FORNOW TODO TODO TODO SHIM
    xpos *= COW0._window_macbook_retina_scale_ONLY_USED_FOR_FIXING_CURSOR_POS;
    ypos *= COW0._window_macbook_retina_scale_ONLY_USED_FOR_FIXING_CURSOR_POS;

    vec2 mouse_s_NDC = transformPoint(_window_get_NDC_from_Screen(), V2((real32) xpos, (real32) ypos));

    { // 2D (with snaps!)
        vec2 s_2D = transformPoint(inverse(camera_get_PV(&camera_2D)), mouse_s_NDC);
        snap_map(s_2D.x, s_2D.y, &world_cursor.mouse_x, &world_cursor.mouse_y);
    }


    // 3D
    mat4 inverse_PV_3D = inverse(camera_get_PV(&camera_3D));
    world_cursor.o = transformPoint(inverse_PV_3D, V3(mouse_s_NDC, -1.0f));
    world_cursor.dir = normalized(transformPoint(inverse_PV_3D, V3(mouse_s_NDC,  1.0f)) - world_cursor.o);

    { // click_move_origin_preview_x, click_move_origin_preview_y
        click_move_origin_mouse_moved_since_last_key_press_M = true;
        if ((click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) && (console_buffer == console_buffer_write_head)) {
            _input_get_mouse_position_and_change_in_position_in_world_coordinates(camera_get_PV(&camera_2D).data, &click_move_origin_preview_x, &click_move_origin_preview_y, NULL, NULL);
            snap_map(click_move_origin_preview_x, click_move_origin_preview_y, &click_move_origin_preview_x, &click_move_origin_preview_y);
        }
    }
}

void callback_mouse_button(GLFWwindow *, int button, int action, int) {
    _callback_mouse_button(NULL, button, action, 0); // FORNOW TODO TODO TODO SHIM

    // TODO switch from NDC -> world (with 3D ray)
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) { 
            if (hot_pane == HOT_PANE_2D) {
                Event event = {};
                event.type = UI_EVENT_TYPE_MOUSE_2D_PRESS;
                event.mouse_x = world_cursor.mouse_x;
                event.mouse_y = world_cursor.mouse_y;
                queue_enqueue(&new_event_queue, event);
            } else if (hot_pane == HOT_PANE_3D) {
                Event event = {};
                event.type = UI_EVENT_TYPE_MOUSE_3D_PRESS;
                event.o = world_cursor.o;
                event.dir = world_cursor.dir;
                queue_enqueue(&new_event_queue, event);
            }
        }
    }
}

void callback_scroll(GLFWwindow *, double, double yoffset) {
    _callback_scroll(NULL, 0, yoffset); // FORNOW TODO TODO TODO SHIM
}

BEGIN_PRE_MAIN {
    glfwSetKeyCallback(        COW0._window_glfw_window, callback_key);
    glfwSetCursorPosCallback(  COW0._window_glfw_window, callback_cursor_position);
    glfwSetMouseButtonCallback(COW0._window_glfw_window, callback_mouse_button);
    glfwSetScrollCallback(     COW0._window_glfw_window, callback_scroll);
} END_PRE_MAIN;


Event history_A[999999];
Event *history_B = history_A;
Event *history_C = history_A;

void history_gui_printf() {
    gui_printf("----------------");
    uint32 i = 0;
    char key_message[] = "[?]";
    for (Event *event = history_A; event < history_C; ++event) {
        {
            const char *leader = (event == history_B - 1) ? "`" : "";
            char number; {
                number = (char) ((event->checkpoint) ? ((i < 10) ? '0' + i : 'A' + (i - 10)) : ' ');
                if (event->checkpoint) ++i;
            }
            char *core; // TODO: CTRL +, SHIFT +
            if (event->type == UI_EVENT_TYPE_KEY_PRESS) {
                if (event->key == COW_KEY_ENTER) {
                    core = "[ENTER]";
                } else if (event->key == COW_KEY_BACKSPACE) {
                    core = "[BACKSPACE]";
                } else if (event->key == COW_KEY_ESCAPE) {
                    core = "[ESCAPE]";
                } else {
                    key_message[1] = (char) (event->key);
                    core = key_message;
                }
            } else if (event->type == UI_EVENT_TYPE_MOUSE_2D_PRESS) {
                core = "[MOUSE_2D]";
            } else {
                ASSERT(event->type == UI_EVENT_TYPE_MOUSE_3D_PRESS);
                core = "[MOUSE_3D]";
            }
            gui_printf("%s %c %s", leader, number, core);
        }
    }
}

// // write key event conditionals in style
//  'Y'        -> Shift + Y
//  'y'        ->         Y
// ('y', true) ->  Ctrl + Y // on Mac, Cmd + Y
// Usage: if (_key_lambda(event, 'Y')) { ... }
//        if (key_lambda('Y')) { ... }
bool _key_lambda(Event event, uint32 code, bool code_super = false) {
    ASSERT(event.type == UI_EVENT_TYPE_KEY_PRESS);

    bool event_shift = (event.mods & GLFW_MOD_SHIFT);
    bool code_shift  = (('A' <= code) && (code <= 'Z'));
    bool shift_match = (event_shift == code_shift);

    if (('a' <= code) && (code <= 'z')) code = 'A' + (code - 'a');
    bool letter_match = (event.key == code);

    bool event_super = event.mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER);
    bool super_match = (event_super == code_super);

    return (shift_match && letter_match && super_match);
};

uint32 event_process(Event event) {
    bool32 result = PROCESSED_EVENT_CATEGORY_RECORD;
    console_params_preview_update(); // FORNOW
    {
        if ((enter_mode == ENTER_MODE_LOAD) || (enter_mode == ENTER_MODE_SAVE)) {
            result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
        }

        // computed bool32's (FORNOW: sloppy--these change mid-frame)
        bool32 dxf_anything_selected;
        bool32 stl_plane_selected;
        {
            dxf_anything_selected = false;
            for (uint32 i = 0; i < dxf.num_entities; ++i) {
                if (dxf_selection_mask[i]) {
                    dxf_anything_selected = true;
                    break;
                }
            }

            stl_plane_selected = !IS_ZERO(squaredNorm(n_selected));
        }


        if (event.type == UI_EVENT_TYPE_KEY_PRESS) {
            auto key_lambda = [event](uint32 code, bool code_super = false) -> bool {
                if (event.type != UI_EVENT_TYPE_KEY_PRESS) return false;
                return _key_lambda(event, code, code_super);
            };

            char character_equivalent; {
                bool shift = (event.mods & GLFW_MOD_SHIFT);
                character_equivalent = (char) event.key;
                if (shift && (event.key == '-')) character_equivalent = '_';
                if ((!shift) && ('A' <= event.key) && (event.key <= 'Z')) character_equivalent = (char) ('a' + (event.key - 'A'));
            }

            {
                bool32 key_eaten_by_special__NOTE_dealt_with_up_top;
                {
                    key_eaten_by_special__NOTE_dealt_with_up_top = false;
                    if (click_modifier == CLICK_MODIFIER_QUALITY) {
                        for (uint32 color = 0; color < 6; ++color) {
                            if (key_lambda('0' + color)) {
                                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                                key_eaten_by_special__NOTE_dealt_with_up_top = true;
                                for (uint32 i = 0; i < dxf.num_entities; ++i) {
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
                bool send_key_to_console;
                {
                    send_key_to_console = false;
                    send_key_to_console |= key_lambda(COW_KEY_BACKSPACE);
                    if (!key_lambda(COW_KEY_ENTER)) {
                        if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) {
                            send_key_to_console |= key_lambda('.');
                            send_key_to_console |= key_lambda('-');
                            for (uint32 i = 0; i < 10; ++i) send_key_to_console |= key_lambda('0' + i);
                            if ((enter_mode != ENTER_MODE_OFFSET_PLANE_BY)) {
                                send_key_to_console |= key_lambda(',');
                            }
                        } else if ((enter_mode == ENTER_MODE_LOAD) || (enter_mode == ENTER_MODE_SAVE)) {
                            send_key_to_console |= key_lambda('.');
                            send_key_to_console |= key_lambda(',');
                            send_key_to_console |= key_lambda('-');
                            send_key_to_console |= key_lambda('/');
                            send_key_to_console |= key_lambda('\\');
                            for (uint32 i = 0; i < 10; ++i) send_key_to_console |= key_lambda('0' + i);
                            for (uint32 i = 0; i < 26; ++i) send_key_to_console |= key_lambda('a' + i);
                            for (uint32 i = 0; i < 26; ++i) send_key_to_console |= key_lambda('A' + i);
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
                    if (!key_lambda(COW_KEY_ENTER)) {
                        if (key_lambda(COW_KEY_BACKSPACE)) {
                            if (console_buffer_write_head != console_buffer) *--console_buffer_write_head = '\0';
                        } else {
                            *console_buffer_write_head++ = character_equivalent;
                        }
                    } else {
                        if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT) || (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) {
                            result = PROCESSED_EVENT_CATEGORY_SELF_AND_PREV_CHECKPOINT;
                            if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) {
                                if (console_buffer_write_head != console_buffer) {
                                    console_param = console_param_preview;
                                    console_param_2 = console_param_2_preview;
                                    conversation_console_buffer_reset();
                                }
                            }
                            if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
                                CrossSectionEvenOdd cross_section = cross_section_create(&dxf, dxf_selection_mask);
                                // cross_section_debug_draw(&camera_2D, &cross_section);
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
                                        dxf_origin_x,
                                        dxf_origin_y);
                                // reset state
                                memset(dxf_selection_mask, 0, dxf.num_entities * sizeof(bool32));
                                conversation_feature_plane_reset();
                            } else if (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) {
                                dxf_origin_x = console_param;
                                dxf_origin_y = console_param_2;
                                conversation_update_M_3D_from_2D();
                            } else {
                                ASSERT(enter_mode == ENTER_MODE_OFFSET_PLANE_BY);
                                if (!IS_ZERO(console_param)) {
                                    r_n_selected += console_param;
                                    selected_index_of_first_triangle_hit_by_ray = -1; // FORNOW (TODO go looking again)
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
                                result = PROCESSED_EVENT_CATEGORY_KILL_HISTORY;
                                if (poe_suffix_match(full_filename_including_path, ".dxf")) {
                                    conversation_dxf_load(full_filename_including_path, (strcmp(full_filename_including_path, conversation_current_dxf_filename) == 0));
                                } else if (poe_suffix_match(full_filename_including_path, ".stl")) {
                                    conversation_stl_load(full_filename_including_path);
                                } else {
                                    conversation_messagef("FORNOW 1265");
                                }
                            } else {
                                ASSERT(enter_mode == ENTER_MODE_SAVE);
                                conversation_save(full_filename_including_path);
                            }
                        }
                        conversation_console_buffer_reset();
                        enter_mode = ENTER_MODE_NONE;
                        if (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) {
                            click_mode = CLICK_MODE_NONE;
                            click_modifier = CLICK_MODIFIER_NONE;
                        }
                    }
                } else if (key_lambda('q', true)) {
                    exit(1);
                } else if (key_lambda(COW_KEY_TAB)) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    camera_3D.angle_of_view = CAMERA_3D_DEFAULT_ANGLE_OF_VIEW - camera_3D.angle_of_view;
                } else if (((enter_mode != ENTER_MODE_SAVE) && (enter_mode != ENTER_MODE_LOAD)) && key_lambda('X')) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    camera2D_zoom_to_bounding_box(&camera_2D, bbox_union);
                } else if (((enter_mode != ENTER_MODE_SAVE) && (enter_mode != ENTER_MODE_LOAD)) && key_lambda('g')) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    hide_grid = !hide_grid;
                } else if (((enter_mode != ENTER_MODE_SAVE) && (enter_mode != ENTER_MODE_LOAD)) && key_lambda('h')) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    show_help = !show_help;
                } else if ((enter_mode == ENTER_MODE_NONE) && key_lambda('.')) { 
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    show_details = !show_details;
                } else if ((enter_mode == ENTER_MODE_NONE) && key_lambda('k')) { 
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    show_stack = !show_stack;
                } else if (key_lambda('L')) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    enter_mode = ENTER_MODE_LOAD;
                } else if (key_lambda('S')) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    enter_mode = ENTER_MODE_SAVE;
                } else if (key_lambda('r', true)) {
                    result = PROCESSED_EVENT_CATEGORY_KILL_HISTORY;
                    conversation_dxf_load(conversation_current_dxf_filename, true);
                } else if (key_lambda(COW_KEY_ESCAPE)) {
                    enter_mode = ENTER_MODE_NONE;
                    click_mode = CLICK_MODE_NONE;
                    click_modifier = CLICK_MODIFIER_NONE;
                    conversation_console_buffer_reset();
                    conversation_feature_plane_reset(); // FORNOW
                } else if (key_lambda('f')) {
                    result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                    console_params_preview_flip_flag = !console_params_preview_flip_flag;
                } else if (key_lambda('n')) {
                    if (stl_plane_selected) {
                        enter_mode = ENTER_MODE_OFFSET_PLANE_BY;
                        console_params_preview_flip_flag = false;
                        conversation_console_buffer_reset();
                    } else {
                        conversation_messagef("[n] no plane selected");
                    }
                } else if (key_lambda('o')) {
                    click_mode = CLICK_MODE_MOVE_2D_ORIGIN_TO;
                    click_modifier = CLICK_MODIFIER_NONE;
                    enter_mode = ENTER_MODE_MOVE_ORIGIN_TO;
                    console_params_preview_flip_flag = false;
                    conversation_console_buffer_reset();
                    click_move_origin_mouse_moved_since_last_key_press_M = false;
                } else if (key_lambda('s')) {
                    click_mode = CLICK_MODE_SELECT;
                    click_modifier = CLICK_MODIFIER_NONE;
                } else if (key_lambda('w')) {
                    if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                        click_modifier = CLICK_MODIFIER_WINDOW;
                        window_select_click_count = 0;
                    }
                } else if (key_lambda('d')) {
                    click_mode = CLICK_MODE_DESELECT;
                    click_modifier = CLICK_MODIFIER_NONE;
                } else if (key_lambda('c')) {
                    if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                        click_modifier = CLICK_MODIFIER_CONNECTED;
                    } else if (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) {
                        result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                        click_modifier = CLICK_MODIFIER_CENTER_OF;
                    }
                } else if (key_lambda('q')) {
                    if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                        click_modifier = CLICK_MODIFIER_QUALITY;
                    }
                } else if (key_lambda('a')) {
                    if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                        result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                        bool32 value_to_write_to_selection_mask = (click_mode == CLICK_MODE_SELECT);
                        for (uint32 i = 0; i < dxf.num_entities; ++i) dxf_selection_mask[i] = value_to_write_to_selection_mask;
                    }
                } else if (key_lambda('x') || key_lambda('y') || key_lambda('z')) {
                    result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                    selected_index_of_first_triangle_hit_by_ray = -1;
                    r_n_selected = 0.0f;
                    if (key_lambda('x')) n_selected = { 1.0f, 0.0f, 0.0f };
                    if (key_lambda('y')) n_selected = { 0.0f, 1.0f, 0.0f };
                    if (key_lambda('z')) n_selected = { 0.0f, 0.0f, 1.0f };
                    conversation_update_M_3D_from_2D();
                } else if (key_lambda('e')) {
                    if (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) {
                        result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                        click_modifier = CLICK_MODIFIER_END_OF;
                    } else {
                        result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                        enter_mode = ENTER_MODE_EXTRUDE_ADD;
                        console_params_preview_flip_flag = false;
                        conversation_console_buffer_reset();
                    }
                } else if (key_lambda('m')) {
                    if (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) {
                        result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                        click_modifier = CLICK_MODIFIER_MIDDLE_OF;
                    }
                } else if (key_lambda('E')) {
                    result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                    enter_mode = ENTER_MODE_EXTRUDE_CUT;
                    console_params_preview_flip_flag = true;
                    conversation_console_buffer_reset();
                } else if (key_lambda('r')) {
                    enter_mode = ENTER_MODE_REVOLVE_ADD;
                } else if (key_lambda('R')) {
                    enter_mode = ENTER_MODE_REVOLVE_CUT;
                } else {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    ;
                }
            }
        } else if (event.type == UI_EVENT_TYPE_MOUSE_2D_PRESS) {
            result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
            auto set_dxf_selection_mask = [&result] (uint32 i, bool32 value_to_write) {
                // Only remember dxf selection operations that actually change the mask
                // NOTE: we could instead do a memcmp at the end, but let's stick with the simple bool32 result = false; ... ret result; approach fornow
                if (dxf_selection_mask[i] != value_to_write) {
                    result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                    dxf_selection_mask[i] = value_to_write;
                }
            };
            if (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) {
                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                dxf_origin_x = event.mouse_x;
                dxf_origin_y = event.mouse_y;
                click_mode = CLICK_MODE_NONE;
                click_modifier = CLICK_MODIFIER_NONE;
                enter_mode = ENTER_MODE_NONE;
                conversation_update_M_3D_from_2D();
            } else if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                bool32 value_to_write_to_selection_mask = (click_mode == CLICK_MODE_SELECT);
                bool32 modifier_connected = (click_modifier == CLICK_MODIFIER_CONNECTED);
                if (click_modifier != CLICK_MODIFIER_WINDOW) {
                    int hot_entity_index = -1;
                    double hot_squared_distance = HUGE_VAL;
                    for (uint32 i = 0; i < dxf.num_entities; ++i) {
                        DXFEntity *entity = &dxf.entities[i];
                        double squared_distance = squared_distance_point_dxf_entity(event.mouse_x, event.mouse_y, entity);
                        if (squared_distance < hot_squared_distance) {
                            hot_squared_distance = squared_distance;
                            hot_entity_index = i;
                        }
                    }
                    if (hot_entity_index != -1) {
                        if (!modifier_connected) {
                            set_dxf_selection_mask(hot_entity_index, value_to_write_to_selection_mask);
                        } else {
                            uint32 loop_index = pick.loop_index_from_entity_index[hot_entity_index];
                            DXFEntityIndexAndFlipFlag *loop = pick.loops[loop_index];
                            uint32 num_entities = pick.num_entities_in_loops[loop_index];
                            for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities; ++entity_index_and_flip_flag) {
                                set_dxf_selection_mask(entity_index_and_flip_flag->entity_index, value_to_write_to_selection_mask);
                            }
                        }
                    }
                } else {
                    if (window_select_click_count == 0) {
                        window_select_x = event.mouse_x;
                        window_select_y = event.mouse_y;

                        ++window_select_click_count;
                    } else {
                        BoundingBox window = {
                            MIN(window_select_x, event.mouse_x),
                            MIN(window_select_y, event.mouse_y),
                            MAX(window_select_x, event.mouse_x),
                            MAX(window_select_y, event.mouse_y)
                        };
                        for (uint32 i = 0; i < dxf.num_entities; ++i) {
                            if (bounding_box_contains(window, bbox[i])) {
                                set_dxf_selection_mask(i, value_to_write_to_selection_mask);
                            }
                        }

                        window_select_click_count = 0;
                    }
                }
            }
        } else {
            ASSERT(event.type == UI_EVENT_TYPE_MOUSE_3D_PRESS);
            result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
            int32 index_of_first_triangle_hit_by_ray = -1;
            {
                real32 min_distance = HUGE_VAL;
                for (uint32 i = 0; i < fancy_mesh.num_triangles; ++i) {
                    vec3 p[3]; {
                        for (uint32 j = 0; j < 3; ++j) p[j] = get(fancy_mesh.vertex_positions, fancy_mesh.triangle_indices[3 * i + j]);
                    }
                    RayTriangleIntersectionResult ray_triangle_intersection_result = ray_triangle_intersection(event.o, event.dir, p[0], p[1], p[2]);
                    if (ray_triangle_intersection_result.hit) {
                        if (ray_triangle_intersection_result.distance < min_distance) {
                            min_distance = ray_triangle_intersection_result.distance;
                            index_of_first_triangle_hit_by_ray = i; // FORNOW
                        }
                    }
                }
            }

            if (index_of_first_triangle_hit_by_ray != -1) { // something hit
                if (index_of_first_triangle_hit_by_ray == selected_index_of_first_triangle_hit_by_ray) { // already selected FORNOW sloppy
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                } else {
                    result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                    selected_index_of_first_triangle_hit_by_ray = index_of_first_triangle_hit_by_ray;
                    {
                        n_selected = get(fancy_mesh.triangle_normals, selected_index_of_first_triangle_hit_by_ray);
                        { // FORNOW (gross) calculateion of r_n_selected
                            vec3 a_selected = get(fancy_mesh.vertex_positions, fancy_mesh.triangle_indices[3 * selected_index_of_first_triangle_hit_by_ray + 0]);
                            r_n_selected = dot(n_selected, a_selected);
                        }
                        conversation_update_M_3D_from_2D();
                    }
                }
            }
        }

    }
    console_params_preview_update(); // FORNOW
    return result;
}

void new_event_process(Event new_event) {
    // ASSERT(history_1 <= history_2);
    // ASSERT((unsigned long) (history_2 - history_0) < ARRAY_LENGTH(history_0));

    auto key_lambda = [new_event](uint32 code, bool code_super = false) -> bool {
        if (new_event.type != UI_EVENT_TYPE_KEY_PRESS) return false;
        return _key_lambda(new_event, code, code_super);
    };

    if (key_lambda('z', true)) { // undo
        if (history_B != history_A) {
            do --history_B; while ((history_A != history_B) && (!history_B->checkpoint)); // pop back _through_ a first checkpoint
            while ((history_A != history_B) && (!(history_B - 1)->checkpoint)) --history_B; // * short-circuit pop back _up to_ a second
            conversation_reset(true);
            for (Event *event = history_A; event < history_B; ++event) event_process(*event);
            conversation_messagef("[undo]");
        } else {
            conversation_messagef("[undo] nothing to undo");
        }
    } else if (key_lambda('y', true) || key_lambda('Z', true)) { // redo
        if (history_B != history_C) {
            do event_process(*history_B++); while ((history_B != history_C) && (!(history_B - 1)->checkpoint));
            conversation_messagef("[redo]");
        } else {
            conversation_messagef("[redo] nothing to redo");
        }
    } else{ // process immediately
        uint32 category = event_process(new_event);
        if (category == PROCESSED_EVENT_CATEGORY_DONT_RECORD) {
        } else if (category == PROCESSED_EVENT_CATEGORY_KILL_HISTORY) {
            history_B = history_C = history_A;
            queue_free(&new_event_queue);
            conversation_save_reset_manifold_manifold_and_fancy_mesh();
        } else {
            if ((category == PROCESSED_EVENT_CATEGORY_CHECKPOINT) || (category == PROCESSED_EVENT_CATEGORY_SELF_AND_PREV_CHECKPOINT)) {
                new_event.checkpoint = true;
            }
            if (category == PROCESSED_EVENT_CATEGORY_SELF_AND_PREV_CHECKPOINT) {
                if (history_B != history_A) (history_B - 1)->checkpoint = true;
            }
            { // history_push_back(...)
                *history_B++ = new_event;
                history_C = history_B; // kill redo "stack"
            }
        }
    }




    // && (new_event.key != GLFW_KEY_LEFT_SHIFT)
    // && (new_event.key != GLFW_KEY_RIGHT_SHIFT)
}

void new_event_queue_process() {
    while (new_event_queue.length) new_event_process(queue_dequeue(&new_event_queue));
}











// TODO: custom shader




void conversation_draw() {
    // FORNOW: repeated computation
    bool32 dxf_anything_selected;
    bool32 stl_plane_selected;
    {
        dxf_anything_selected = false;
        for (uint32 i = 0; i < dxf.num_entities; ++i) {
            if (dxf_selection_mask[i]) {
                dxf_anything_selected = true;
                break;
            }
        }

        stl_plane_selected = !IS_ZERO(squaredNorm(n_selected));
    }
    // FORNOW: repeated computation
    mat4 PV_2D = camera_get_PV(&camera_2D);
    real32 mouse_x, mouse_y; { _input_get_mouse_position_and_change_in_position_in_world_coordinates(PV_2D.data, &mouse_x, &mouse_y, NULL, NULL); }
    mat4 P_3D = camera_get_P(&camera_3D);
    mat4 V_3D = camera_get_V(&camera_3D);
    mat4 PV_3D = P_3D * V_3D;

    uint32 window_width, window_height; {
        real32 _window_width, _window_height; // FORNOW
        _window_get_size(&_window_width, &_window_height);
        window_width = (uint32) _window_width;
        window_height = (uint32) _window_height;
    }

    { // panes
        { // draw
            if (0) { // highlight
                glDisable(GL_DEPTH_TEST);
                if (hot_pane == HOT_PANE_2D) {
                    eso_begin(globals.Identity, SOUP_QUADS);
                    eso_color(0.1f, 0.1f, 0.0f);
                    eso_vertex(-1.0f,  1.0f);
                    eso_vertex(-1.0f, -1.0f);
                    eso_vertex( 0.0f, -1.0f);
                    eso_vertex( 0.0f,  1.0f);
                    eso_end();
                } else if (hot_pane == HOT_PANE_3D) {
                    eso_begin(globals.Identity, SOUP_QUADS);
                    eso_color(0.1f, 0.1f, 0.0f);
                    eso_vertex(0.0f,  1.0f);
                    eso_vertex(0.0f, -1.0f);
                    eso_vertex(1.0f, -1.0f);
                    eso_vertex(1.0f,  1.0f);
                    eso_end();
                }
                glEnable(GL_DEPTH_TEST);
            }

            eso_begin(globals.Identity, SOUP_LINES, 5.0f, true);
            eso_color(136 / 255.0f, 136 / 255.0f, 136 / 255.0f);
            eso_vertex(0.0f,  1.0f);
            eso_vertex(0.0f, -1.0f);
            eso_end();
        }
    }

    real32 axes_2D_draw_translation_x;
    real32 axes_2D_draw_translation_y;
    if (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) {
        if (click_move_origin_mouse_moved_since_last_key_press_M && (console_buffer == console_buffer_write_head)) {
            axes_2D_draw_translation_x = click_move_origin_preview_x;
            axes_2D_draw_translation_y = click_move_origin_preview_y;
        } else {
            axes_2D_draw_translation_x = console_param_preview;
            axes_2D_draw_translation_y = console_param_2_preview;
        }
    }

    { // draw 2D draw 2d draw
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, 0, window_width / 2, window_height);
        {
            if (!hide_grid) { // grid 2D grid 2d grid
                eso_begin(PV_2D, SOUP_LINES, 2.0f);
                eso_color(80.0f / 255, 80.0f / 255, 80.0f / 255);
                for (uint32 i = 0; i <= uint32(GRID_SIDE_LENGTH / GRID_SPACING); ++i) {
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
                eso_begin(PV_2D, SOUP_LINES);
                for (uint32 i = 0; i < dxf.num_entities; ++i) {
                    DXFEntity *entity = &dxf.entities[i];
                    int32 color = (dxf_selection_mask[i]) ? DXF_COLOR_SELECTION : DXF_COLOR_DONT_OVERRIDE;
                    eso_dxf_entity__SOUP_LINES(entity, color);
                }
                eso_end();
            }
            { // dots
                if (show_details) {
                    eso_begin(camera_get_PV(&camera_2D), SOUP_POINTS);
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
                real32 r = camera_2D.screen_height_World / 120.0f;
                mat4 M = M4_Translation(dxf_origin_x, dxf_origin_y);
                vec3 color = V3(0.8f, 0.8f, 1.0f);
                if (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) {
                    M = M4_Translation(axes_2D_draw_translation_x, axes_2D_draw_translation_y);
                    color = V3(0.0f, 1.0f, 1.0f);
                }
                eso_begin(PV_2D * M, SOUP_LINES);
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

        { // selection 2d selection 2D selection tube tubes stack stacks wire wireframe (FORNOW: ew)
            uint32 color = ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_ADD)) ? DXF_COLOR_TRAVERSE : ((enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) ? DXF_COLOR_QUALITY_1 : ((enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) ? DXF_COLOR_WATER_ONLY : DXF_COLOR_SELECTION;

            uint32 NUM_TUBE_STACKS_INCLUSIVE;
            mat4 M;
            mat4 M_incr;
            if (true) {
                if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                    // NOTE: some repetition with wrapper
                    real32 a = MIN(0.0f, MIN(console_param_preview, console_param_2_preview));
                    real32 b = MAX(0.0f, MAX(console_param_preview, console_param_2_preview));
                    real32 length = b - a;
                    NUM_TUBE_STACKS_INCLUSIVE = MIN(64, uint32(roundf(length / 2.5f)) + 2);
                    M = M_3D_from_2D * M4_Translation(-dxf_origin_x, -dxf_origin_y, a + Z_FIGHT_EPS);
                    M_incr = M4_Translation(0.0f, 0.0f, (b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                } else if ((enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
                    NUM_TUBE_STACKS_INCLUSIVE = NUM_SEGMENTS_PER_CIRCLE;
                    M = M_3D_from_2D * M4_Translation(-dxf_origin_x, -dxf_origin_y, 0.0f);
                    real32 a = 0.0f;
                    real32 b = TAU;
                    M_incr =  M4_Translation(dxf_origin_x, dxf_origin_y, 0.0f) * M4_RotationAboutYAxis((b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1)) * M4_Translation(-dxf_origin_x, -dxf_origin_y, 0.0f);
                } else if (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) {
                    // FORNOW
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * M4_Translation(-axes_2D_draw_translation_x, -axes_2D_draw_translation_y);
                    M_incr = M4_Identity();
                } else if (enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * M4_Translation(-dxf_origin_x, -dxf_origin_y, console_param_preview + Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * M4_Translation(-dxf_origin_x, -dxf_origin_y, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                }
                for (uint32 tube_stack_index = 0; tube_stack_index < NUM_TUBE_STACKS_INCLUSIVE; ++tube_stack_index) {
                    {
                        eso_begin(PV_3D * M, SOUP_LINES, 5.0f);
                        for (uint32 i = 0; i < dxf.num_entities; ++i) {
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


        if (dxf_anything_selected) { // arrow
            if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
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
                for (uint32 i = 0; i < 2; ++i) {
                    if (!IS_ZERO(H[i])) {
                        real32 total_height = ABS(H[i]);
                        real32 cap_height = (total_height > 10.0f) ? 5.0f : (0.5f * total_height);
                        real32 shaft_height = total_height - cap_height;
                        real32 s = 1.5f;
                        mat4 N = M4_Translation(0.0, 0.0, -Z_FIGHT_EPS);
                        if (toggle[i]) N = M4_Scaling(1.0f, 1.0f, -1.0f) * N;
                        mat4 M_cyl  = M_3D_from_2D * M4_Translation(-dxf_origin_x, -dxf_origin_y) * R2 * N * M4_Scaling(s * 1.0f, s * 1.0f, shaft_height) * R;
                        mat4 M_cone = M_3D_from_2D * M4_Translation(-dxf_origin_x, -dxf_origin_y) * R2 * N * M4_Translation(0.0f, 0.0f, shaft_height) * M4_Scaling(s * 2.0f, s * 2.0f, cap_height) * R;
                        library.meshes.cylinder.draw(P_3D, V_3D, M_cyl, color);
                        library.meshes.cone.draw(P_3D, V_3D, M_cone, color);
                    }
                }
            }
        }

        if (stl_plane_selected) { // axes 3D axes 3d axes axis 3D axis 3d axis
            real32 r = camera_3D.ortho_screen_height_World / 120.0f;
            eso_begin(PV_3D * M_3D_from_2D * M4_Translation(0.0f, 0.0f, Z_FIGHT_EPS), SOUP_LINES, 4.0f);
            eso_color(0.8f, 0.8f, 1.0f);
            eso_vertex(-r*0.6f, 0.0f);
            eso_vertex(r, 0.0f);
            eso_vertex(0.0f, -r);
            eso_vertex(0.0f, r*0.6f);
            eso_end();
        }


        { // fancy_mesh; NOTE: includes transparency 3d mesh 3D mesh 3d
            if (fancy_mesh.cosmetic_edges) {
                eso_begin(PV_3D, SOUP_LINES); 
                eso_color(monokai.black);
                // 3 * num_triangles * 2 / 2
                for (uint32 k = 0; k < 2 * fancy_mesh.num_cosmetic_edges; ++k) eso_vertex(fancy_mesh.vertex_positions, fancy_mesh.cosmetic_edges[k]);
                eso_end();
            }
            for (uint32 pass = 0; pass <= 1; ++pass) {
                eso_begin(PV_3D, (!show_details) ? SOUP_TRIANGLES : SOUP_OUTLINED_TRIANGLES);

                mat3 inv_transpose_V_3D = inverse(transpose(M3(V_3D(0, 0), V_3D(0, 1), V_3D(0, 2), V_3D(1, 0), V_3D(1, 1), V_3D(1, 2), V_3D(2, 0), V_3D(2, 1), V_3D(2, 2))));

                for (uint32 i = 0; i < fancy_mesh.num_triangles; ++i) {
                    vec3 n = get(fancy_mesh.triangle_normals, i);
                    vec3 p[3];
                    real32 x_n;
                    {
                        for (uint32 j = 0; j < 3; ++j) p[j] = get(fancy_mesh.vertex_positions, fancy_mesh.triangle_indices[3 * i + j]);
                        x_n = dot(n, p[0]);
                    }
                    vec3 color; 
                    real32 alpha;
                    {
                        vec3 n_camera = inv_transpose_V_3D * n;
                        vec3 color_n = V3(0.5f + 0.5f * n_camera.x, 0.5f + 0.5f * n_camera.y, 1.0f);
                        if ((selected_index_of_first_triangle_hit_by_ray != -1) && (dot(n, n_selected) > 0.999f) && (ABS(x_n - r_n_selected) < 0.001f)) {
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
                }
                eso_end();
            }
        }

        if (!hide_grid) { // grid 3D grid 3d grid
            conversation_draw_3D_grid(P_3D, V_3D);
        }

        { // floating sketch plane; NOTE: transparent
            bool draw = (selected_index_of_first_triangle_hit_by_ray == -1);
            mat4 PVM = PV_3D * M_3D_from_2D;
            vec3 color = monokai.yellow;
            real32 sign = -1.0f;
            if (enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                PVM *= M4_Translation(0.0f, 0.0f, console_param_preview);
                color = { 0.0f, 1.0f, 1.0f };
                sign = 1.0f;
                draw = true;
            } else if (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) {
                PVM *= M4_Translation(-axes_2D_draw_translation_x - dxf_origin_x, -axes_2D_draw_translation_y - dxf_origin_y);
                color = { 0.0f, 1.0f, 1.0f };
                sign = 1.0f;
                draw = true;
            }
            real32 r = 30.0f;
            BoundingBox bounding_box = { -r, -r, r, r };
            if (dxf_anything_selected) {
                bounding_box = bounding_box_union(dxf.num_entities, bbox, dxf_selection_mask);
                {
                    bounding_box.min[0] -= dxf_origin_x;
                    bounding_box.max[0] -= dxf_origin_x;
                    bounding_box.min[1] -= dxf_origin_y;
                    bounding_box.max[1] -= dxf_origin_y;
                }
                for (uint32 d = 0; d < 2; ++d) {
                    bounding_box.min[d] /= GRID_SPACING;
                    bounding_box.min[d] -= 1.0;
                    bounding_box.min[d] = floorf(bounding_box.min[d]);
                    bounding_box.min[d] *= GRID_SPACING;

                    bounding_box.max[d] /= GRID_SPACING;
                    bounding_box.max[d] += 1.0;
                    bounding_box.max[d] = ceilf(bounding_box.max[d]);
                    bounding_box.max[d] *= GRID_SPACING;

                }
            }
            if (draw) {
                eso_begin(PVM, SOUP_OUTLINED_QUADS);
                eso_color(color, 0.35f);
                eso_vertex(bounding_box.min[0], bounding_box.min[1], sign * Z_FIGHT_EPS);
                eso_vertex(bounding_box.min[0], bounding_box.max[1], sign * Z_FIGHT_EPS);
                eso_vertex(bounding_box.max[0], bounding_box.max[1], sign * Z_FIGHT_EPS);
                eso_vertex(bounding_box.max[0], bounding_box.min[1], sign * Z_FIGHT_EPS);
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
                (click_mode == CLICK_MODE_NONE) ? "NONE" :
                "???",
                (click_modifier == CLICK_MODE_NONE) ? "" :
                (click_modifier == CLICK_MODIFIER_CONNECTED) ? "CONNECTED" :
                (click_modifier == CLICK_MODIFIER_WINDOW) ? "WINDOW" :
                (click_modifier == CLICK_MODIFIER_CENTER_OF) ? "CENTER_OF" :
                (click_modifier == CLICK_MODIFIER_END_OF) ? "END_OF" :
                (click_modifier == CLICK_MODIFIER_MIDDLE_OF) ? "MIDDLE_OF" :
                "???");

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
                (enter_mode == ENTER_MODE_NONE) ? "NONE" :
                "???",
                enter_message);

        if ((enter_mode == ENTER_MODE_NONE) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
            gui_printf("> %s", console_buffer);
        } else {
            gui_printf("> %s", console_buffer);
        }

        conversation_message_buffer_update_and_draw();

        if (show_details) {
            gui_printf("%d dxf elements", dxf.num_entities);
            gui_printf("%d stl triangles", fancy_mesh.num_triangles);
        }
        if (show_stack) history_gui_printf();
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
            gui_printf("(y)-plane (z)-plane (x)-plane");
            gui_printf("(e)trude-add (E)xtrude-cut + (0123456789.,) (f)lip-direction");
            gui_printf("(r)evolve-add (R)evolve-cut");
            gui_printf("(Ctrl+z)-undo (Ctrl+Z)-redo (Ctrl+y)-redo");
            gui_printf("(L)oad (S)ave");
            gui_printf("(Ctrl+R)eload-dxf");
            gui_printf("show-(g)rid (.)-show-details show-stac(k)");
            gui_printf("zoom-to-e(X)tents");
            gui_printf("(Tab)-toggle-camera-perspective-orthographic");
            gui_printf("m(o)ve-origin + (c)enter-of (e)nd-of (m)iddle-of (Click) / (-0123456789.,) (f)lip-direction");
            gui_printf("");
            gui_printf("you can drag and drop dxf's into Conversation");
        }
    }
}



int main() {
    _window_set_size(1.5 * 640.0, 1.5 * 360.0);

    conversation_init();

    conversation_messagef("type h for help `// pre-alpha " __DATE__ " " __TIME__);

    while (cow_begin_frame()) {
        new_event_queue_process();
        { // stuff that still shims globals.*
            if ((!globals.mouse_left_held && !globals.mouse_right_held) || globals.mouse_left_pressed || globals.mouse_right_pressed) {
                hot_pane = (globals.mouse_position_NDC.x <= 0.0f) ? HOT_PANE_2D : HOT_PANE_3D;
                if ((click_modifier == CLICK_MODIFIER_WINDOW) && (window_select_click_count == 1)) hot_pane = HOT_PANE_2D;// FORNOW
            }
            { // camera_move (using shimmed globals.* state)
                if (hot_pane == HOT_PANE_2D) {
                    camera_move(&camera_2D);
                } else if (hot_pane == HOT_PANE_3D) {
                    camera_move(&camera_3D);
                }
            }
        }
        conversation_draw();
    }
}


