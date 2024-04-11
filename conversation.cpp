// // cleanup pass
// TODO: get remedy (or rad debugger) working in parallels
// TODO try the naive fat struct
// TODO reduce paths through the code

// TODO open file inside conversation
// TODO space_bar_event
// TODO: cleanup pass (what is conversation_reset__NOTE_basically_just_a_memset_0(); // FORNOW)))???
// // make it clear that you push an empty struct to superstack to get things started (this is always on the stack so we can peek)

// TODO: find a better way to do || (enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO) || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)
// -----------> what i mean by this is we need to have a better way of doing 'x' input in general (FORNOW: i'm going to rollback the text input on these features)
// || (enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO)
// || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)
// || (enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO)
// || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)
// } else if (enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO) {
//     dxf_origin_x = console_param_1;
//     dxf_origin_y = console_param_2;
//     conversation_update_M_3D_from_2D();
// } else {
//     ASSERT(enter_mode == ENTER_MODE_OFFSET_PLANE_BY);
//     if (!IS_ZERO(console_param_1)) {
//         r_n_selected += console_param_1;
//         selected_index_of_first_triangle_hit_by_ray = -1; // FORNOW (TODO go looking again)
//         conversation_update_M_3D_from_2D();
//     }
// }
// NOTE: rolling back the checkpointing of prev on extrudes
// ----> // if (history_B_redo != history_A_undo) (history_B_redo - 1)->checkpoint_type = CHECKPOINT_TYPE_CHECKPOINT;

// NOTE: a super checkpoint pushes to the feature stack
// TODO: super checkpoint (save state) redo stack
// TODO: do a renaming pass (save_state, history, category, ...)


// TODO: make an undo that stores a stack of the 3D stuff so you only undo from a 3D checkpoint

// TODO: move holding shift pre-snaps the mouse to 

// NOTE: backspace is used both for deleting sketch entities and changing console stuff (e.g., for an extrude) TODO give console priority

// // /TODO misc
// TODO get a mac debugger
// TODO get THREE.js/p5.js back up and running for 136 "slides"

// NOTE TODO take a step back and look at the code from a 100 mile view

// TODO ability to hide the 2D stuff just to hide it (so you can be fully in a layout clone)

// NOTE FORNOW the following operations are considered "fast"--we don't bother storing them
// - computing a dxf_entities's bounding box

// // TODO important
// true undo stack for boolean operations (checkpoints)

// // TODO easy (< 30 min)
// XXXX two_click_commands
// TODO support color schemes
// XXXX 'z' snaps to origin (TODO what is the z-plane)
// TODO 'r' conflicts with 'r'otate from layout
// TODO 'e' conflicts with 'e'rase from layout
// TODO layout has a command 'Z'

// // TODO more snaps
// TODO quad
// TODO perpendicular
// TODO limit
// TODO

// // TODO dxf editor
// TODO create/extrude/whatever-we-call-it state wrapped up into a struct
// XXXX drawing lines
// XXXX drawing lines with snaps
// XXXX undoing the drawing of a line
// XXXX actually incorporating drawn lines into the dxf_entities
// XXXX deleting lines
// XXXX creating circle
// XXXX creating fillet
// XXXX creating fillet user specifies size
// TODO (b)ox size
// TODO (b)ox key in start position
// TODO (l)ine key in start position
// TODO remember fillet radius
// TODO space bar to repeat command
// TODO better create fillet
// TODO join
// TODO divide 2
// TODO divide 1
// TODO divide N
// TODO typing in coordinates of points instead of clicking
// TODO holding shift to snap to 15 degree increments
// TODO c(o)py
// TODO (m)ove
// TODO (r)otate
// TODO (R)otate
// TODO mirror-(X)
// TODO mirror-(Y)
// TODO (e)rase

// // TODO graphics
// TODO custom shader for dotted lines
// TODO custom shader with lighting
// TODO custom shader with cosmetic edges (TODO read NIVIDIA whitepaper on mesh wireframe)
// TODO custom cursors

// // TODO small features / qol
// TODO prompt for overwriting files
// TODO you could redo the entire stack with finer circle discretization params
// TODO should have a relative version of move (MOVE_DXF_ORIGIN_BY)

// // TODO cow improvements?
// TODO rename list.array -> list.internal_array; list.length -> list.num_full_slots
// TODO soup/eso can take size per vertex
// TODO soup/eso can have dotted lines
// TODO mouse position first frame
// TODO better Map (fornow SO BAD)
// TODO better text fondxf_loop_analysis_create_FORNOW_QUADRATIC



#include "cs345.cpp"
#include "manifoldc.h"
#include "poe.cpp"
#undef real // ??
#define u32 DO_NOT_USE_u32_USE_uint32_INSTEAD
#include "burkardt.cpp"
#include "conversation.h"

////////////////////////////////////////////////////
// GLOBALS (TODO: wrap in struct ConversationState { ... } conversation;
////////////////////////////////////////////////////

Camera2D camera_2D;
Camera3D camera_3D;

bool32 hide_grid;
bool32 show_details;
bool32 show_help;
bool32 show_command_stack;
bool32 hide_gui;


List<DXFEntity> dxf_entities;
List<bool32> dxf_selection_mask;
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

real32 console_param_1;
real32 console_param_2;
bool32 console_params_preview_flip_flag;
real32 console_param_1_preview;
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
    real32 sign = 1.0f;
    if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
        sign = (!console_params_preview_flip_flag) ? 1.0f : -1.0f;
    }
    console_param_1_preview = sign * strtof(buffs[0], NULL);
    console_param_2_preview = sign * strtof(buffs[1], NULL);
    if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
        console_param_2_preview *= -1;
    }
}

bool32 revolve_use_x_instead;






uint32 hot_pane;


// TODO: combine these

bool32 two_click_command_awaiting_second_click;
real32 two_click_command_x_0;
real32 two_click_command_y_0;




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


bool32 click_move_origin_broken;
real32 click_move_origin_preview_x;
real32 click_move_origin_preview_y;


// GLOBAL-TOUCHING FUNCTIONS ///////////////////////////

// TODO: see which of these can be easily pulled out into non-global touching

void conversation_dxf_reset() {
}

void conversation_dxf_load(char *filename, bool preserve_cameras_and_dxf_origin = false) {
    if (!poe_file_exists(filename)) {
        conversation_messagef("[load] \"%s\" not found", filename);
        return;
    }

    list_free_AND_zero(&dxf_entities);
    list_free_AND_zero(&dxf_selection_mask);

    dxf_load(filename, &dxf_entities);
    list_calloc_NOT_reserve(&dxf_selection_mask, dxf_entities.length, sizeof(bool32));

    if (!preserve_cameras_and_dxf_origin) {
        camera2D_zoom_to_bounding_box(&camera_2D, dxf_get_bounding_box(&dxf_entities));
        dxf_origin_x = 0.0f;
        dxf_origin_y = 0.0f;
    }

    strcpy(conversation_current_dxf_filename, filename);

    conversation_messagef("[load] loaded %s", filename);
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
    camera_2D = { 100.0f, 0.0, 0.0f, -0.5f, -0.125f };
    camera2D_zoom_to_bounding_box(&camera_2D, dxf_get_bounding_box(&dxf_entities));
    camera_3D = { 2.0f * camera_2D.screen_height_World, CAMERA_3D_DEFAULT_ANGLE_OF_VIEW, RAD(33.0f), RAD(-44.0f), 0.0f, 0.0f, 0.5f, -0.125f };
}


struct ConversationSaveState {
    ManifoldManifold *manifold_manifold;
    FancyMesh fancy_mesh;
    List<DXFEntity> dxf_entities;
    List<bool32> dxf_selection_mask;
};
// TODO: replace this with an ElephantStack
Stack<ConversationSaveState> super_undo_stack;
Stack<ConversationSaveState> super_redo_stack;

void _load_in_save_state(ConversationSaveState *save_state) {
    // TODO manifold-memory 
    manifold_manifold = save_state->manifold_manifold;
    fancy_mesh = save_state->fancy_mesh;
    list_clone(&dxf_entities, &save_state->dxf_entities);
    list_clone(&dxf_selection_mask, &save_state->dxf_selection_mask);
}

void super_stacks_do__NOTE_kills_redo_super_stack() {
    { // free redo stack
        for (uint32 i = 0; i < super_redo_stack.length; ++i) {
            // TODO manifold-memory 
            fancy_mesh_free(&super_redo_stack.array[i].fancy_mesh);
            list_free_AND_zero(&super_redo_stack.array[i].dxf_entities);
            list_free_AND_zero(&super_redo_stack.array[i].dxf_selection_mask);
        }
    }
    // TODO manifold-memory 
    ConversationSaveState save_state = {};
    save_state.manifold_manifold = manifold_manifold;
    save_state.fancy_mesh = fancy_mesh;
    list_clone(&save_state.dxf_entities, &dxf_entities);
    list_clone(&save_state.dxf_selection_mask, &dxf_selection_mask);
    stack_push(&super_undo_stack, save_state);
}
void super_stacks_peek_and_load() {
    ASSERT(super_undo_stack.length);
    ConversationSaveState save_state = stack_peek(&super_undo_stack); // FORNOW; TODO: pointer versions
    _load_in_save_state(&save_state);
}
void super_stacks_undo_and_load() {
    ASSERT(super_undo_stack.length);
    // TODO manifold-memory 
    ConversationSaveState save_state = stack_pop(&super_undo_stack); // FORNOW; TODO: pointer versions
    stack_push(&super_redo_stack, save_state);
    super_stacks_peek_and_load();
}
void super_stacks_redo_and_load() {
    ConversationSaveState save_state = stack_pop(&super_redo_stack); // FORNOW; TODO: pointer versions
    stack_push(&super_undo_stack, save_state);
    super_stacks_peek_and_load();
}

void conversation_reset__NOTE_basically_just_a_memset_0(bool32 disable_top_layer_resets = false, bool32 disable_dxf_origin_resets = false) {
    click_mode = CLICK_MODE_NONE;
    click_modifier = CLICK_MODIFIER_NONE;
    enter_mode = ENTER_MODE_NONE;

    console_param_1 = 0.0f;
    console_param_2 = 0.0f;
    console_params_preview_flip_flag = false;

    if (!disable_dxf_origin_resets) {
        list_memset(&dxf_selection_mask, 0, dxf_entities.length * sizeof(bool32));
        dxf_origin_x = 0.0f;
        dxf_origin_y = 0.0f;
    }
    conversation_feature_plane_reset(); 
    conversation_console_buffer_reset();

    if (!disable_top_layer_resets) {
        hot_pane = HOT_PANE_NONE;
        hide_grid = false;
        show_details = false;
        show_command_stack = true;
        show_help = false;
        conversation_cameras_reset();
    }
}



#define UI_EVENT_TYPE_KEY_PRESS      0
#define UI_EVENT_TYPE_MOUSE_2D_PRESS 1
#define UI_EVENT_TYPE_MOUSE_3D_PRESS 2

#define CHECKPOINT_TYPE_NONE 0
#define CHECKPOINT_TYPE_CHECKPOINT 1
#define CHECKPOINT_TYPE_SUPER_CHECKPOINT_MESH_SAVED_IN_UNDO_STACK 2

struct Event {
    uint32 type;
    union {
        struct {
            uint32 key;
            bool32 super;
            bool32 shift;
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
    bool32 checkpoint_ineligible; // not_checkpoint_eligible__NOTE_set_before_event_process
    uint32 checkpoint_type; // checkpoint_type__NOTE_set_in_new_event_process
};

Queue<Event> new_event_queue;


void spoof_KEY(uint32 key, bool32 super = false, bool32 shift = false) {
    Event event = {};
    event.type = UI_EVENT_TYPE_KEY_PRESS;
    event.key = key;
    event.super = super;
    event.shift = shift;
    queue_enqueue(&new_event_queue, event);
}

void spoof_MOUSE_2D(real32 mouse_x, real32 mouse_y) {
    Event event = {};
    event.type = UI_EVENT_TYPE_MOUSE_2D_PRESS;
    event.mouse_x = mouse_x;
    event.mouse_y = mouse_y;
    queue_enqueue(&new_event_queue, event);
}

void spoof_MOUSE_3D(real32 o_x, real32 o_y, real32 o_z, real32 dir_x, real32 dir_y, real32 dir_z) {
    Event event = {};
    event.type = UI_EVENT_TYPE_MOUSE_3D_PRESS;
    event.o = { o_x, o_y, o_z };
    event.dir = { dir_x, dir_y, dir_z };
    queue_enqueue(&new_event_queue, event);
}

void conversation_init() {
    { // conversation_dxf_load
        if (0) {
            conversation_dxf_load("omax.dxf", true);
            if (0) {
                Event event = {};
                event.type = UI_EVENT_TYPE_KEY_PRESS;
                for (int i = 0; i < 5; ++i) {
                    spoof_KEY('Y');
                    spoof_KEY('S');
                    spoof_KEY('Q');
                    spoof_KEY('0' + i);
                    spoof_KEY('E');
                    spoof_KEY('1' + i);
                    spoof_KEY(COW_KEY_ENTER);
                }
            }
        } else if (0) {
            conversation_dxf_load("ik.dxf", true);
        } else if (0) {
            conversation_dxf_load("debug.dxf", true);
        } else {
            conversation_dxf_load("splash.dxf", true);
            if (1) {
                spoof_KEY('Y');
                spoof_KEY('S');
                spoof_KEY('C');
                spoof_MOUSE_2D( 20.0,  20.0);
                spoof_MOUSE_2D( 16.0,  16.0);
                spoof_MOUSE_2D( 16.0, -16.0);
                spoof_MOUSE_2D(-16.0, -16.0);
                spoof_MOUSE_2D(-16.0,  16.0);
                spoof_KEY('E');
                spoof_KEY('5');
                spoof_KEY('0');
                spoof_KEY(COW_KEY_ENTER); // FORNOW
                spoof_MOUSE_3D(50.0, 93.0, 76.0, -0.47, -0.43, -0.77);
                spoof_MOUSE_2D(16.4, -9.5);
                spoof_KEY('E', false, true);
                spoof_KEY('4');
                spoof_KEY('7');
                spoof_KEY(COW_KEY_ENTER); // FORNOW
            }
        }
    }
    conversation_reset__NOTE_basically_just_a_memset_0();
    super_stacks_do__NOTE_kills_redo_super_stack();
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

#define PROCESSED_EVENT_CATEGORY_DONT_RECORD              0
#define PROCESSED_EVENT_CATEGORY_RECORD                   1
#define PROCESSED_EVENT_CATEGORY_CHECKPOINT               2
#define PROCESSED_EVENT_CATEGORY_EXPENSIVE_MESH_OPERATION 3
#define PROCESSED_EVENT_CATEGORY_KILL_HISTORY             4




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
        event.super = (mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER));
        event.shift = (mods & GLFW_MOD_SHIFT);
        queue_enqueue(&new_event_queue, event);
    }
}

void snap_map(real32 before_x, real32 before_y, real32 *after_x, real32 *after_y) {
    real32 tmp_x = before_x;
    real32 tmp_y = before_y;
    {
        if (click_modifier == CLICK_MODIFIER_SNAP_TO_CENTER_OF) {
            real32 min_squared_distance = HUGE_VAL;
            for (DXFEntity *entity = dxf_entities.array; entity < &dxf_entities.array[dxf_entities.length]; ++entity) {
                if (entity->type == DXF_ENTITY_TYPE_LINE) {
                    continue;
                } else { ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                    DXFArc *arc = &entity->arc;
                    real32 squared_distance = squared_distance_point_dxf_arc(before_x, before_y, arc);
                    if (squared_distance < min_squared_distance) {
                        min_squared_distance = squared_distance;
                        tmp_x = arc->center_x;
                        tmp_y = arc->center_y;
                    }
                }
            }
        } else if (click_modifier == CLICK_MODIFIER_SNAP_TO_MIDDLE_OF) {
            real32 min_squared_distance = HUGE_VAL;
            real32 middle_x;
            real32 middle_y;
            for (DXFEntity *entity = dxf_entities.array; entity < &dxf_entities.array[dxf_entities.length]; ++entity) {
                real32 squared_distance = squared_distance_point_dxf_entity(before_x, before_y, entity);
                if (squared_distance < min_squared_distance) {
                    min_squared_distance = squared_distance;
                    entity_get_middle(entity, &middle_x, &middle_y);
                    tmp_x = middle_x;
                    tmp_y = middle_y;
                }
            }
        } else if (click_modifier == CLICK_MODIFIER_SNAP_TO_END_OF) {
            real32 min_squared_distance = HUGE_VAL;
            for (DXFEntity *entity = dxf_entities.array; entity < &dxf_entities.array[dxf_entities.length]; ++entity) {
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


bool32 callback_mouse_left_held;
real32 callback_xpos; // FORNOW
real32 callback_ypos; // FORNOW

void callback_cursor_position(GLFWwindow *, double xpos, double ypos) {
    _callback_cursor_position(NULL, xpos, ypos); // FORNOW TODO TODO TODO SHIM

    callback_xpos = (real32) (xpos * COW0._window_macbook_retina_scale_ONLY_USED_FOR_FIXING_CURSOR_POS);
    callback_ypos = (real32) (ypos * COW0._window_macbook_retina_scale_ONLY_USED_FOR_FIXING_CURSOR_POS);

    // // mouse held generates mouse presses
    // FORNOW repeated from callback_mouse_button
    if ((hot_pane == HOT_PANE_2D) && (callback_mouse_left_held) && (((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) && (click_modifier != CLICK_MODIFIER_WINDOW))) {
        vec2 mouse_s_NDC = transformPoint(_window_get_NDC_from_Screen(), V2(callback_xpos, callback_ypos));
        Event event = {};
        event.type = UI_EVENT_TYPE_MOUSE_2D_PRESS;
        {
            vec2 s_2D = transformPoint(inverse(camera_get_PV(&camera_2D)), mouse_s_NDC);
            snap_map(s_2D.x, s_2D.y, &event.mouse_x, &event.mouse_y);
        }
        event.checkpoint_ineligible = true;
        queue_enqueue(&new_event_queue, event);
    }
}

void callback_mouse_button(GLFWwindow *, int button, int action, int) {
    _callback_mouse_button(NULL, button, action, 0); // FORNOW TODO TODO TODO SHIM

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) { 
            callback_mouse_left_held = true;

            vec2 mouse_s_NDC = transformPoint(_window_get_NDC_from_Screen(), V2(callback_xpos, callback_ypos));
            if (hot_pane == HOT_PANE_2D) {
                Event event = {};
                event.type = UI_EVENT_TYPE_MOUSE_2D_PRESS;
                {
                    vec2 s_2D = transformPoint(inverse(camera_get_PV(&camera_2D)), mouse_s_NDC);
                    snap_map(s_2D.x, s_2D.y, &event.mouse_x, &event.mouse_y);
                }
                queue_enqueue(&new_event_queue, event);
            } else if (hot_pane == HOT_PANE_3D) {
                Event event = {};
                event.type = UI_EVENT_TYPE_MOUSE_3D_PRESS;
                {
                    mat4 inverse_PV_3D = inverse(camera_get_PV(&camera_3D));
                    event.o = transformPoint(inverse_PV_3D, V3(mouse_s_NDC, -1.0f));
                    event.dir = normalized(transformPoint(inverse_PV_3D, V3(mouse_s_NDC,  1.0f)) - event.o);
                }
                queue_enqueue(&new_event_queue, event);
            }
        } else if (action == GLFW_RELEASE) {
            callback_mouse_left_held = false;
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


Event history_A_undo[999999];
Event *history_B_redo = history_A_undo;
Event *history_C_one_past_end_of_redo = history_A_undo;

void history_gui_printf() {
    gui_printf("----------------");
    gui_printf("super_undo_stack.length = %d", super_undo_stack.length);
    gui_printf("note: top of the undo stack in blue");
    gui_printf("----------------");
    uint32 i = 0;
    for (Event *event = history_A_undo; event < history_C_one_past_end_of_redo; ++event) {
        {
            char leader_leader = (event->checkpoint_type == CHECKPOINT_TYPE_SUPER_CHECKPOINT_MESH_SAVED_IN_UNDO_STACK) ? '+' : ' ';
            const char *leader = (event == history_B_redo - 1) ? "`" : "";
            char number; {
                number = ' ';
                if (event->checkpoint_type != CHECKPOINT_TYPE_NONE) {
                    number = (char) ((i < 10) ? '0' + i : 'A' + (i - 10));
                    ++i;
                }
            }
            char message[256]; {
                if (event->type == UI_EVENT_TYPE_KEY_PRESS) {
                    if (event->key == COW_KEY_ENTER) {
                        sprintf(message, "[ENTER]");
                    } else if (event->key == COW_KEY_BACKSPACE) {
                        sprintf(message, "[BACKSPACE]");
                    } else if (event->key == COW_KEY_ESCAPE) {
                        sprintf(message, "[ESCAPE]");
                    } else {
                        sprintf(message, "[%s%s%c]", (event->super) ? "CTRL+" : "", (event->shift) ? "SHIFT+" : "", (char) (event->key));
                    }
                } else if (event->type == UI_EVENT_TYPE_MOUSE_2D_PRESS) {
                    // core = "[MOUSE_2D]";
                    sprintf(message, "[MOUSE_2D] %g %g", event->mouse_x, event->mouse_y);
                } else { ASSERT(event->type == UI_EVENT_TYPE_MOUSE_3D_PRESS);
                    // core = "[MOUSE_3D]";
                    sprintf(message, "[MOUSE_3D] %g %g %g %g %g %g", event->o.x, event->o.y, event->o.z, event->dir.x, event->dir.y, event->dir.z);
                }
            }
            gui_printf("%c%s %c %s", leader_leader, leader, number, message);
        }
    }
}


// // write key event conditionals in style
//  'Y'        -> Shift + Y
//  'y'        ->         Y
// ('y', true) ->  Ctrl + Y // on Mac, Cmd + Y
// Usage: if (_key_lambda(event, 'Y')) { ... }
//        if (key_lambda('Y')) { ... }
bool _key_lambda(Event event, uint32 key, bool super = false, bool shift = false) {
    ASSERT(event.type == UI_EVENT_TYPE_KEY_PRESS);
    ASSERT(!(('a' <= key) && (key <= 'z')));
    bool key_match = (event.key == key);
    bool super_match = ((event.super && super) || (!event.super && !super)); // * bool32
    bool shift_match = ((event.shift && shift) || (!event.shift && !shift)); // * bool32
    return (key_match && super_match && shift_match);
};

Event space_bar_event = { UI_EVENT_TYPE_KEY_PRESS };

uint32 event_process(Event event, bool32 skip_mesh_generation_because_we_are_loading_from_the_redo_stack = false) {
    bool32 result = PROCESSED_EVENT_CATEGORY_RECORD;
    console_params_preview_update(); // FORNOW
    {


        auto push_back_dxf_entity_and_mask_bool_lambda = [&](DXFEntity entity) {
            list_push_back(&dxf_entities, entity);
            list_push_back(&dxf_selection_mask, (bool32) false);
        };
        auto delete_dxf_entity_and_mask_bool_lambda_NOTE_if_iterating_must_be_going_backwards = [&](uint32 i) {
            list_delete_at(&dxf_entities, i);
            list_delete_at(&dxf_selection_mask, i);
        };



        if ((enter_mode == ENTER_MODE_OPEN) || (enter_mode == ENTER_MODE_SAVE)) {
            result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
        }

        // computed bool32's (FORNOW: sloppy--these change mid-frame)
        bool32 dxf_anything_selected;
        bool32 stl_plane_selected;
        {
            dxf_anything_selected = false;
            for (uint32 i = 0; i < dxf_entities.length; ++i) {
                if (dxf_selection_mask.array[i]) {
                    dxf_anything_selected = true;
                    break;
                }
            }

            stl_plane_selected = !IS_ZERO(squaredNorm(n_selected));
        }
        bool32 extrude = ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT));
        bool32 revolve = ((enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT));
        bool32 cut = ((enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_CUT));


        if (event.type == UI_EVENT_TYPE_KEY_PRESS) {
            auto key_lambda = [event](uint32 key, bool super = false, bool shift = false) -> bool {
                if (event.type != UI_EVENT_TYPE_KEY_PRESS) return false; // TODO: ASSERT
                return _key_lambda(event, key, super, shift);
            };

            bool32 _click_mode_SNAP_ELIGIBLE_ =
                0
                || (click_mode == CLICK_MODE_ORIGIN_MOVE)
                || (click_mode == CLICK_MODE_MEASURE)
                || (click_mode == CLICK_MODE_CREATE_LINE)
                || (click_mode == CLICK_MODE_CREATE_BOX)
                || (click_mode == CLICK_MODE_CREATE_CIRCLE)
                || (click_mode == CLICK_MODE_DXF_MOVE)
                ;

            bool32 _click_mode_NEEDS_CONSOLE = 0
                || (click_mode == CLICK_MODE_CREATE_FILLET);

            // TODO: _enter_mode_NEEDS_CONSOLE
            // FORNOW: assume all enters require console (even though technically rotate doesn't right now)


            char character_equivalent; {
                character_equivalent = (char) event.key;
                if (event.shift && (event.key == '-')) character_equivalent = '_';
                if ((!event.shift) && ('A' <= event.key) && (event.key <= 'Z')) character_equivalent = (char) ('a' + (event.key - 'A'));
            }

            {
                bool32 key_eaten_by_special__NOTE_dealt_with_up_top;
                {
                    key_eaten_by_special__NOTE_dealt_with_up_top = false;
                    if (click_modifier == CLICK_MODIFIER_QUALITY) {
                        for (uint32 color = 0; color <= 9; ++color) {
                            if (key_lambda('0' + color)) {
                                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                                key_eaten_by_special__NOTE_dealt_with_up_top = true;
                                for (uint32 i = 0; i < dxf_entities.length; ++i) {
                                    if (dxf_entities.array[i].color == color) {
                                        bool32 value_to_write_to_selection_mask = (click_mode == CLICK_MODE_SELECT);
                                        dxf_selection_mask.array[i] = value_to_write_to_selection_mask;
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
                    if (!key_lambda(COW_KEY_ENTER)) {
                        if (0
                                || (enter_mode == ENTER_MODE_EXTRUDE_ADD)
                                || (enter_mode == ENTER_MODE_EXTRUDE_CUT)
                                || (enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO)
                                || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)
                                || (click_mode == CLICK_MODE_CREATE_FILLET)
                           ) {
                            send_key_to_console |= key_lambda(COW_KEY_BACKSPACE);
                            send_key_to_console |= key_lambda('.');
                            send_key_to_console |= key_lambda('-');
                            for (uint32 i = 0; i < 10; ++i) send_key_to_console |= key_lambda('0' + i);
                            if ((enter_mode != ENTER_MODE_OFFSET_PLANE_BY)) {
                                send_key_to_console |= key_lambda(',');
                            }
                        } else if ((enter_mode == ENTER_MODE_OPEN) || (enter_mode == ENTER_MODE_SAVE)) {
                            send_key_to_console |= key_lambda(COW_KEY_BACKSPACE);
                            send_key_to_console |= key_lambda('.');
                            send_key_to_console |= key_lambda(',');
                            send_key_to_console |= key_lambda('-');
                            send_key_to_console |= key_lambda('/');
                            send_key_to_console |= key_lambda('\\');
                            for (uint32 i = 0; i < 10; ++i) send_key_to_console |= key_lambda('0' + i);
                            for (uint32 i = 0; i < 26; ++i) send_key_to_console |= key_lambda('A' + i);
                            for (uint32 i = 0; i < 26; ++i) send_key_to_console |= key_lambda('A' + i, false, true);
                        }
                    } else {
                        send_key_to_console = true;
                        {
                            if (enter_mode == ENTER_MODE_NONE) {
                                conversation_messagef("[enter] enter mode is none");
                                send_key_to_console = false;
                            } else if (extrude || revolve) {
                                if (!dxf_anything_selected) {
                                    conversation_messagef("[enter] no dxf_entities elements selected");
                                    send_key_to_console = false;
                                } else if (!stl_plane_selected) { // FORNOW??
                                    conversation_messagef("[enter] no plane selected");
                                    send_key_to_console = false;
                                } else if (cut && (fancy_mesh.num_triangles == 0)) { // FORNOW
                                    conversation_messagef("[enter] nothing to cut");
                                    send_key_to_console = false;
                                } else if (extrude) {
                                    if (IS_ZERO(console_param_1_preview) && IS_ZERO(console_param_2_preview)) {
                                        conversation_messagef("[enter] extrude height is zero");
                                        send_key_to_console = false;
                                    }
                                } else {
                                    ASSERT(revolve);
                                    ;
                                }
                            } else if (enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO) {
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
                        if (extrude || revolve) {
                            result = PROCESSED_EVENT_CATEGORY_EXPENSIVE_MESH_OPERATION;

                            if (extrude) {
                                if (console_buffer_write_head != console_buffer) {
                                    console_param_1 = console_param_1_preview;
                                    console_param_2 = console_param_2_preview;
                                    conversation_console_buffer_reset();
                                }
                            }

                            if (!skip_mesh_generation_because_we_are_loading_from_the_redo_stack) {
                                CrossSectionEvenOdd cross_section = cross_section_create_FORNOW_QUADRATIC(&dxf_entities, dxf_selection_mask.array);
                                wrapper_manifold(
                                        &manifold_manifold,
                                        &fancy_mesh,
                                        cross_section.num_polygonal_loops,
                                        cross_section.num_vertices_in_polygonal_loops,
                                        cross_section.polygonal_loops,
                                        M_3D_from_2D,
                                        enter_mode,
                                        console_param_1,
                                        console_param_2,
                                        dxf_origin_x,
                                        dxf_origin_y,
                                        revolve_use_x_instead);
                            }

                            list_memset(&dxf_selection_mask, 0, dxf_entities.length * sizeof(bool32));
                            conversation_feature_plane_reset();
                            console_param_1 = 0.0f;
                            console_param_2 = 0.0f;
                        } else {
                            ASSERT((enter_mode == ENTER_MODE_OPEN) || (enter_mode == ENTER_MODE_SAVE));
                            static char full_filename_including_path[512];
                            sprintf(full_filename_including_path, "%s%s", conversation_drop_path, console_buffer);
                            if (enter_mode == ENTER_MODE_OPEN) {
                                result = PROCESSED_EVENT_CATEGORY_KILL_HISTORY;
                                if (poe_suffix_match(full_filename_including_path, ".dxf")) {
                                    conversation_dxf_load(full_filename_including_path, (strcmp(full_filename_including_path, conversation_current_dxf_filename) == 0));
                                } else if (poe_suffix_match(full_filename_including_path, ".stl")) {
                                    conversation_stl_load(full_filename_including_path);
                                } else {
                                    conversation_messagef("[open] %s not found", full_filename_including_path);
                                }
                            } else { ASSERT(enter_mode == ENTER_MODE_SAVE);
                                conversation_save(full_filename_including_path);
                            }
                        }
                        conversation_console_buffer_reset();
                        enter_mode = ENTER_MODE_NONE;
                        if (click_mode == CLICK_MODE_ORIGIN_MOVE) {
                            click_mode = CLICK_MODE_NONE;
                            click_modifier = CLICK_MODIFIER_NONE;
                        }
                    }
                } else if (key_lambda('Q', true)) {
                    exit(1);
                } else if (key_lambda('0')) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    camera_3D.angle_of_view = CAMERA_3D_DEFAULT_ANGLE_OF_VIEW - camera_3D.angle_of_view;
                } else if (key_lambda('X', false, true)) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    camera2D_zoom_to_bounding_box(&camera_2D, dxf_get_bounding_box(&dxf_entities));
                } else if (key_lambda('G')) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    hide_grid = !hide_grid;
                } else if (key_lambda('H')) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    show_help = !show_help;
                } else if (key_lambda('.')) { 
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    show_details = !show_details;
                } else if (key_lambda('K')) { 
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    show_command_stack = !show_command_stack;
                } else if (key_lambda('K', false, true)) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    hide_gui = !hide_gui;
                } else if (key_lambda('O', true)) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    enter_mode = ENTER_MODE_OPEN;
                } else if (key_lambda('S', true)) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    enter_mode = ENTER_MODE_SAVE;
                } else if (key_lambda('R', true, true)) {
                    result = PROCESSED_EVENT_CATEGORY_KILL_HISTORY;
                    conversation_dxf_load(conversation_current_dxf_filename, true);
                } else if (key_lambda(COW_KEY_ESCAPE)) {
                    enter_mode = ENTER_MODE_NONE;
                    click_mode = CLICK_MODE_NONE;
                    click_modifier = CLICK_MODIFIER_NONE;
                    conversation_console_buffer_reset();
                    conversation_feature_plane_reset(); // FORNOW
                } else if (key_lambda(COW_KEY_TAB)) {
                    result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                    console_params_preview_flip_flag = !console_params_preview_flip_flag;
                    if ((enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) revolve_use_x_instead = !revolve_use_x_instead;
                } else if (key_lambda('N')) {
                    if (stl_plane_selected) {
                        enter_mode = ENTER_MODE_OFFSET_PLANE_BY;
                        console_params_preview_flip_flag = false;
                        conversation_console_buffer_reset();
                    } else {
                        conversation_messagef("[n] no plane selected");
                    }
                } else if (key_lambda('Z', false, true)) {
                    click_mode = CLICK_MODE_ORIGIN_MOVE;
                    click_modifier = CLICK_MODIFIER_NONE;
                    enter_mode = ENTER_MODE_MOVE_DXF_ORIGIN_TO;
                    console_params_preview_flip_flag = false;
                    conversation_console_buffer_reset();
                    click_move_origin_broken = false;
                } else if (key_lambda('S')) {
                    click_mode = CLICK_MODE_SELECT;
                    click_modifier = CLICK_MODIFIER_NONE;
                } else if (key_lambda('D')) {
                    click_mode = CLICK_MODE_DESELECT;
                    click_modifier = CLICK_MODIFIER_NONE;
                } else if (key_lambda('C')) {
                    if (((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) && (click_modifier != CLICK_MODIFIER_CONNECTED)) {
                        click_modifier = CLICK_MODIFIER_CONNECTED;
                    } else if (_click_mode_SNAP_ELIGIBLE_) {
                        result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                        click_modifier = CLICK_MODIFIER_SNAP_TO_CENTER_OF;
                    } else {
                        result = PROCESSED_EVENT_CATEGORY_RECORD;
                        click_mode = CLICK_MODE_CREATE_CIRCLE;
                        click_modifier = CLICK_MODIFIER_NONE;
                        two_click_command_awaiting_second_click = false;
                        space_bar_event.key = 'C'; // FORNOW; TODO perhaps key_lambda can store the entirety of the event
                                                   // update_space_bar_event_off_of_most_recent_key_lambda
                    }
                } else if (key_lambda('Q')) {
                    if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                        click_modifier = CLICK_MODIFIER_QUALITY;
                    }
                } else if (key_lambda('A')) {
                    if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                        result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                        bool32 value_to_write_to_selection_mask = (click_mode == CLICK_MODE_SELECT);
                        for (uint32 i = 0; i < dxf_entities.length; ++i) dxf_selection_mask.array[i] = value_to_write_to_selection_mask;
                    }
                } else if (key_lambda('Y')) {
                    result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;

                    // already one of the three primary planes
                    if ((selected_index_of_first_triangle_hit_by_ray == -1) && ARE_EQUAL(r_n_selected, 0.0f) && ARE_EQUAL(squaredNorm(n_selected), 1.0f) && ARE_EQUAL(max(n_selected), 1.0f)) {
                        n_selected = { n_selected[2], n_selected[0], n_selected[1] };
                    } else {
                        selected_index_of_first_triangle_hit_by_ray = -1;
                        r_n_selected = 0.0f;
                        n_selected = { 0.0f, 1.0f, 0.0f };
                    }
                    conversation_update_M_3D_from_2D();
                } else if (key_lambda('E')) {
                    if (_click_mode_SNAP_ELIGIBLE_) {
                        result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                        click_modifier = CLICK_MODIFIER_SNAP_TO_END_OF;
                    } else {
                        // result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                        enter_mode = ENTER_MODE_EXTRUDE_ADD;
                        if (_click_mode_NEEDS_CONSOLE) click_mode = CLICK_MODE_NONE;
                        console_params_preview_flip_flag = false;
                        conversation_console_buffer_reset();
                    }
                } else if (key_lambda('M')) {
                    if (_click_mode_SNAP_ELIGIBLE_) {
                        result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                        click_modifier = CLICK_MODIFIER_SNAP_TO_MIDDLE_OF;
                        two_click_command_awaiting_second_click = false;
                    } else {
                        result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                        click_mode = CLICK_MODE_DXF_MOVE;
                        click_modifier = CLICK_MODIFIER_NONE;
                        two_click_command_awaiting_second_click = false;
                    }
                } else if (key_lambda('E', false, true)) {
                    // result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                    enter_mode = ENTER_MODE_EXTRUDE_CUT;
                    if (_click_mode_NEEDS_CONSOLE) click_mode = CLICK_MODE_NONE;
                    console_params_preview_flip_flag = true;
                    conversation_console_buffer_reset();
                } else if (key_lambda('R')) {
                    enter_mode = ENTER_MODE_REVOLVE_ADD;
                    revolve_use_x_instead = false;
                } else if (key_lambda('R', false, true)) {
                    enter_mode = ENTER_MODE_REVOLVE_CUT;
                    revolve_use_x_instead = false;
                } else if (key_lambda('W')) {
                    if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                        click_modifier = CLICK_MODIFIER_WINDOW;
                        two_click_command_awaiting_second_click = false;
                    }
                } else if (key_lambda('M')) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    click_mode = CLICK_MODE_MEASURE;
                    click_modifier = CLICK_MODIFIER_NONE;
                    two_click_command_awaiting_second_click = false;
                } else if (key_lambda('L')) {
                    result = PROCESSED_EVENT_CATEGORY_RECORD;
                    click_mode = CLICK_MODE_CREATE_LINE;
                    click_modifier = CLICK_MODIFIER_NONE;
                    two_click_command_awaiting_second_click = false;
                    space_bar_event.key = 'L'; // FORNOW 
                } else if (key_lambda('F')) {
                    result = PROCESSED_EVENT_CATEGORY_RECORD;
                    click_mode = CLICK_MODE_CREATE_FILLET;
                    click_modifier = CLICK_MODIFIER_NONE;
                    enter_mode = ENTER_MODE_NONE;
                    two_click_command_awaiting_second_click = false;
                } else if (key_lambda('B')) {
                    result = PROCESSED_EVENT_CATEGORY_RECORD;
                    click_mode = CLICK_MODE_CREATE_BOX;
                    click_modifier = CLICK_MODIFIER_NONE;
                    two_click_command_awaiting_second_click = false;
                    space_bar_event.key = 'B'; // FORNOW
                } else if (key_lambda(COW_KEY_BACKSPACE)) {
                    for (int32 i = dxf_entities.length - 1; i >= 0; --i) {
                        if (dxf_selection_mask.array[i]) {
                            delete_dxf_entity_and_mask_bool_lambda_NOTE_if_iterating_must_be_going_backwards(i);
                        }
                    }
                    list_memset(&dxf_selection_mask, 0, dxf_entities.length * sizeof(bool32));
                } else {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    ;
                }
            }
        } else if (event.type == UI_EVENT_TYPE_MOUSE_2D_PRESS) {
            result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;

            auto set_dxf_selection_mask = [&result] (uint32 i, bool32 value_to_write) {
                // Only remember dxf_entities selection operations that actually change the mask
                // NOTE: we could instead do a memcmp at the end, but let's stick with the simple bool32 result = false; ... ret result; approach fornow
                if (dxf_selection_mask.array[i] != value_to_write) {
                    result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                    dxf_selection_mask.array[i] = value_to_write;
                }
            };


            bool32 value_to_write_to_selection_mask = (click_mode == CLICK_MODE_SELECT);
            if ((
                        (click_mode == CLICK_MODE_MEASURE) ||
                        (click_mode == CLICK_MODE_CREATE_LINE) ||
                        (click_mode == CLICK_MODE_CREATE_BOX) ||
                        (click_mode == CLICK_MODE_CREATE_CIRCLE) ||
                        (click_mode == CLICK_MODE_CREATE_FILLET) ||
                        (click_mode == CLICK_MODE_DXF_MOVE) ||
                        (((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) && (click_modifier == CLICK_MODIFIER_WINDOW)))
                    && (!two_click_command_awaiting_second_click)) { 
                result = (click_mode == CLICK_MODE_MEASURE) ? PROCESSED_EVENT_CATEGORY_DONT_RECORD : PROCESSED_EVENT_CATEGORY_RECORD; 
                if (!(((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) && (click_modifier == CLICK_MODIFIER_WINDOW))) click_modifier = CLICK_MODIFIER_NONE;
                two_click_command_awaiting_second_click = true;
                two_click_command_x_0 = event.mouse_x;
                two_click_command_y_0 = event.mouse_y;
            } else if (click_mode == CLICK_MODE_MEASURE) {
                ASSERT(two_click_command_awaiting_second_click);
                two_click_command_awaiting_second_click = false;
                click_mode = CLICK_MODE_NONE;
                click_modifier = CLICK_MODIFIER_NONE;
                real32 angle = DEG(atan2(event.mouse_y - two_click_command_y_0, event.mouse_x - two_click_command_x_0));
                if (angle < 0.0f) angle += 360.0f;
                conversation_messagef("%gmm %gdeg", sqrt(squared_distance_point_point(two_click_command_x_0, two_click_command_y_0, event.mouse_x, event.mouse_y)), angle);
            } else if (click_mode == CLICK_MODE_CREATE_LINE) {
                ASSERT(two_click_command_awaiting_second_click);
                two_click_command_awaiting_second_click = false;
                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                click_mode = CLICK_MODE_NONE;
                click_modifier = CLICK_MODIFIER_NONE;
                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, two_click_command_x_0, two_click_command_y_0, event.mouse_x, event.mouse_y });
            } else if (click_mode == CLICK_MODE_CREATE_BOX) {
                ASSERT(two_click_command_awaiting_second_click);
                two_click_command_awaiting_second_click = false;
                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                click_mode = CLICK_MODE_NONE;
                click_modifier = CLICK_MODIFIER_NONE;
                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, two_click_command_x_0, two_click_command_y_0, event.mouse_x, two_click_command_y_0 });
                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, two_click_command_x_0, two_click_command_y_0, two_click_command_x_0, event.mouse_y });
                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, event.mouse_x, event.mouse_y, event.mouse_x, two_click_command_y_0 });
                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, event.mouse_x, event.mouse_y, two_click_command_x_0, event.mouse_y });
            } else if (click_mode == CLICK_MODE_DXF_MOVE) {
                ASSERT(two_click_command_awaiting_second_click);
                two_click_command_awaiting_second_click = false;
                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                click_mode = CLICK_MODE_NONE;
                click_modifier = CLICK_MODIFIER_NONE;
                real32 dx = event.mouse_x - two_click_command_x_0;
                real32 dy = event.mouse_y - two_click_command_y_0;
                for (uint32 i = 0; i < dxf_entities.length; ++i) {
                    DXFEntity *entity = &dxf_entities.array[i];
                    if (dxf_selection_mask.array[i]) {
                        entity->line.start_x += dx;
                        entity->line.start_y += dy;
                        entity->line.end_x   += dx;
                        entity->line.end_y   += dy;
                        entity->arc.center_x += dx;
                        entity->arc.center_y += dy;
                    }
                }
            } else if (click_mode == CLICK_MODE_CREATE_CIRCLE) {
                ASSERT(two_click_command_awaiting_second_click);
                two_click_command_awaiting_second_click = false;
                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                click_mode = CLICK_MODE_NONE;
                click_modifier = CLICK_MODIFIER_NONE;
                real32 theta_a_in_degrees = DEG(atan2(event.mouse_y - two_click_command_y_0, event.mouse_x - two_click_command_x_0));
                real32 theta_b_in_degrees = theta_a_in_degrees + 180.0f;
                real32 r = SQRT(squared_distance_point_point(two_click_command_x_0, two_click_command_y_0, event.mouse_x, event.mouse_y));
                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_ARC, DXF_COLOR_TRAVERSE, two_click_command_x_0, two_click_command_y_0, r, theta_a_in_degrees, theta_b_in_degrees });
                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_ARC, DXF_COLOR_TRAVERSE, two_click_command_x_0, two_click_command_y_0, r, theta_b_in_degrees, theta_a_in_degrees });
            } else if (click_mode == CLICK_MODE_ORIGIN_MOVE) {
                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                dxf_origin_x = event.mouse_x;
                dxf_origin_y = event.mouse_y;
                click_mode = CLICK_MODE_NONE;
                click_modifier = CLICK_MODIFIER_NONE;
                enter_mode = ENTER_MODE_NONE;
                conversation_update_M_3D_from_2D();
            } else if (click_mode == CLICK_MODE_CREATE_FILLET) {
                ASSERT(two_click_command_awaiting_second_click);
                two_click_command_awaiting_second_click = false;
                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                click_modifier = CLICK_MODIFIER_NONE;
                int i = dxf_find_closest_entity(&dxf_entities, two_click_command_x_0, two_click_command_y_0);
                int j = dxf_find_closest_entity(&dxf_entities, event.mouse_x, event.mouse_y);
                if ((i != j) && (i != -1) && (j != -1)) {
                    real32 radius = console_param_1_preview;
                    DXFEntity *E_i = &dxf_entities.array[i];
                    DXFEntity *E_j = &dxf_entities.array[j];
                    if ((E_i->type == DXF_ENTITY_TYPE_LINE) && (E_j->type == DXF_ENTITY_TYPE_LINE)) {
                        vec2 a, b, c, d;
                        entity_get_start_and_end_points(E_i, &a.x, &a.y, &b.x, &b.y);
                        entity_get_start_and_end_points(E_j, &c.x, &c.y, &d.x, &d.y);

                        LineLineIntersectionResult _p = line_line_intersection(a, b, c, d);
                        if (_p.is_valid) {
                            vec2 p = _p.position;

                            //  a -- b   p          s -- t-.  
                            //                              - 
                            //           d    =>             t
                            //     m     |             m     |
                            //           c                   s

                            //         d                              
                            //         |                              
                            //         |                              
                            //  a ---- p ---- b   =>   s - t.         
                            //         |                     -t       
                            //    m    |                 m    |       
                            //         c                      s       

                            vec2 m; {
                                vec2 m1 = { two_click_command_x_0, two_click_command_y_0 };
                                vec2 m2 = { event.mouse_x, event.mouse_y };
                                m = AVG(m1, m2);
                            }

                            vec2 e_ab = normalized(b - a);
                            vec2 e_cd = normalized(d - c);

                            bool32 keep_a, keep_c; {
                                vec2 vector_p_m_in_edge_basis = inverse(hstack(e_ab, e_cd)) * (m - p);
                                keep_a = (vector_p_m_in_edge_basis.x < 0.0f);
                                keep_c = (vector_p_m_in_edge_basis.y < 0.0f);
                            }

                            // TODO: in general, just use burkardt's angle stuff

                            vec2 s_ab = (keep_a) ? a : b;
                            vec2 s_cd = (keep_c) ? c : d;
                            real32 half_angle; {
                                real32 angle = three_point_angle(s_ab, p, s_cd); // FORNOW TODO consider using burkardt's special interior version
                                if (angle > PI) angle = TAU - angle;
                                half_angle = angle / 2;
                            }
                            real32 length = radius / tan(half_angle);
                            vec2 t_ab = p + (keep_a ? -1 : 1) * length * e_ab;
                            vec2 t_cd = p + (keep_c ? -1 : 1) * length * e_cd;

                            LineLineIntersectionResult _center = line_line_intersection(t_ab, t_ab + perpendicularTo(e_ab), t_cd, t_cd + perpendicularTo(e_cd));
                            if (_center.is_valid) {
                                vec2 center = _center.position;

                                uint32 color_i = E_i->color;
                                uint32 color_j = E_j->color;
                                delete_dxf_entity_and_mask_bool_lambda_NOTE_if_iterating_must_be_going_backwards(MAX(i, j));
                                delete_dxf_entity_and_mask_bool_lambda_NOTE_if_iterating_must_be_going_backwards(MIN(i, j));

                                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_LINE, color_i, s_ab.x, s_ab.y, t_ab.x, t_ab.y });
                                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_LINE, color_j, s_cd.x, s_cd.y, t_cd.x, t_cd.y });

                                real32 theta_ab_in_degrees = DEG(atan2(t_ab.y - center.y, t_ab.x - center.x));
                                real32 theta_cd_in_degrees = DEG(atan2(t_cd.y - center.y, t_cd.x - center.x));

                                if (!IS_ZERO(radius)) {
                                    if (three_point_angle(t_ab, center, t_cd) < PI) {
                                        // FORNOW TODO consider swap
                                        real32 tmp = theta_ab_in_degrees;
                                        theta_ab_in_degrees = theta_cd_in_degrees;
                                        theta_cd_in_degrees = tmp;
                                    }

                                    // TODO: consider tabbing to create chamfer

                                    push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_ARC, DXF_COLOR_TRAVERSE, center.x, center.y, radius, theta_ab_in_degrees, theta_cd_in_degrees });
                                }
                            }
                        }
                    } else {
                        conversation_messagef("TODO: line-arc fillet; arc-arc fillet");
                    }
                }
            } else if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                if (click_modifier != CLICK_MODIFIER_WINDOW) {
                    int hot_entity_index = dxf_find_closest_entity(&dxf_entities, event.mouse_x, event.mouse_y);
                    if (hot_entity_index != -1) {
                        if (click_modifier == CLICK_MODIFIER_CONNECTED) {
                            #if 1 // TODO: consider just using the O(n*m) algorithm here instead

                            #define GRID_CELL_WIDTH 0.001f

                            auto scalar_bucket = [&](real32 a) -> real32 {
                                real32 result = roundf(a / GRID_CELL_WIDTH) * GRID_CELL_WIDTH;
                                return result;
                            };

                            auto make_key = [&](real32 x, real32 y) -> vec2 {
                                return { scalar_bucket(x), scalar_bucket(y) };

                            };

                            auto nudge_key = [&](vec2 key, int dx, int dy) -> vec2 {
                                return make_key(key.x + dx * GRID_CELL_WIDTH, key.y + dy * GRID_CELL_WIDTH);
                            };

                            struct GridPointSlot {
                                bool32 populated;
                                int32 entity_index;
                                bool32 end_NOT_start;
                            };

                            struct GridCell {
                                GridPointSlot slots[2];
                            };

                            Map<vec2, GridCell> grid; { // TODO: build grid
                                grid = {};

                                auto push_into_grid_unless_cell_full__make_cell_if_none_exists = [&](real32 x, real32 y, uint32 entity_index, bool32 end_NOT_start) {
                                    vec2 key = make_key(x, y);
                                    GridCell *cell = _map_get_pointer(&grid, key);
                                    if (cell == NULL) {
                                        map_put(&grid, key, {});
                                        cell = _map_get_pointer(&grid, key);
                                    }
                                    for (uint32 i = 0; i < ARRAY_LENGTH(cell->slots); ++i) {
                                        GridPointSlot *slot = &cell->slots[i];
                                        if (slot->populated) continue;
                                        slot->populated = true;
                                        slot->entity_index = entity_index;
                                        slot->end_NOT_start = end_NOT_start;
                                        // printf("%f %f [%d]\n", key.x, key.y, i);
                                        break;
                                    }
                                };

                                for (uint32 entity_index = 0; entity_index < dxf_entities.length; ++entity_index) {
                                    DXFEntity *entity = &dxf_entities.array[entity_index];

                                    real32 start_x, start_y, end_x, end_y;
                                    entity_get_start_and_end_points(entity, &start_x, &start_y, &end_x, &end_y);
                                    push_into_grid_unless_cell_full__make_cell_if_none_exists(start_x, start_y, entity_index, false);
                                    push_into_grid_unless_cell_full__make_cell_if_none_exists(end_x, end_y, entity_index, true);
                                }
                            }

                            bool32 *edge_marked = (bool32 *) calloc(dxf_entities.length, sizeof(bool32));

                            ////////////////////////////////////////////////////////////////////////////////
                            // NOTE: We are now done adding to the grid, so we can now operate directly on GridCell *'s
                            //       We will use _map_get_pointer(...)
                            ////////////////////////////////////////////////////////////////////////////////


                            auto get_key = [&](GridPointSlot *point, bool32 other_endpoint) {
                                bool32 end_NOT_start; {
                                    end_NOT_start = point->end_NOT_start;
                                    if (other_endpoint) end_NOT_start = !end_NOT_start;
                                }
                                real32 x, y; {
                                    DXFEntity *entity = &dxf_entities.array[point->entity_index];
                                    if (end_NOT_start) {
                                        entity_get_end_point(entity, &x, &y);
                                    } else {
                                        entity_get_start_point(entity, &x, &y);
                                    }
                                }
                                return make_key(x, y);
                            };

                            auto get_any_point_not_part_of_an_marked_entity = [&](vec2 key) -> GridPointSlot * {
                                GridCell *cell = _map_get_pointer(&grid, key);
                                if (!cell) return NULL;

                                for (uint32 i = 0; i < ARRAY_LENGTH(cell->slots); ++i) {
                                    GridPointSlot *slot = &cell->slots[i];
                                    if (!slot->populated) continue;
                                    if (edge_marked[slot->entity_index]) continue;
                                    return slot;
                                }
                                return NULL;
                            };



                            // NOTE: we will mark the hot entity, and then shoot off from both its endpoints
                            edge_marked[hot_entity_index] = true;
                            set_dxf_selection_mask(hot_entity_index, value_to_write_to_selection_mask);

                            for (uint32 pass = 0; pass <= 1; ++pass) {

                                vec2 seed; {
                                    real32 x, y;
                                    if (pass == 0) {
                                        entity_get_start_point(&dxf_entities.array[hot_entity_index], &x, &y);
                                    } else {
                                        entity_get_end_point(&dxf_entities.array[hot_entity_index], &x, &y);
                                    }
                                    seed = make_key(x, y);
                                }

                                GridPointSlot *curr = get_any_point_not_part_of_an_marked_entity(seed);
                                while (true) {
                                    if (curr == NULL) break;
                                    set_dxf_selection_mask(curr->entity_index, value_to_write_to_selection_mask);
                                    edge_marked[curr->entity_index] = true;
                                    curr = get_any_point_not_part_of_an_marked_entity(get_key(curr, true)); // get other end
                                    if (curr == NULL) break;
                                    { // curr <- next (9-cell)
                                        vec2 key = get_key(curr, false);
                                        {
                                            curr = NULL;
                                            for (int dx = -1; dx <= 1; ++dx) {
                                                for (int dy = -1; dy <= 1; ++dy) {
                                                    GridPointSlot *tmp = get_any_point_not_part_of_an_marked_entity(nudge_key(key, dx, dy));
                                                    if (tmp) curr = tmp;
                                                }
                                            }
                                        }
                                    }
                                }
                            }




                            map_free_and_zero(&grid);
                            free(edge_marked);



















                            #else // old O(n^2) version
                            uint32 loop_index = dxf_pick_loops.loop_index_from_entity_index[hot_entity_index];
                            DXFEntityIndexAndFlipFlag *loop = dxf_pick_loops.loops[loop_index];
                            uint32 num_entities = dxf_pick_loops.num_entities_in_loops[loop_index];
                            for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < &loop[num_entities]; ++entity_index_and_flip_flag) {
                                set_dxf_selection_mask(entity_index_and_flip_flag->entity_index, value_to_write_to_selection_mask);
                            }
                            #endif
                        } else {
                            set_dxf_selection_mask(hot_entity_index, value_to_write_to_selection_mask);
                        }
                    }
                } else { ASSERT(click_modifier == CLICK_MODIFIER_WINDOW);
                    ASSERT(two_click_command_awaiting_second_click);
                    two_click_command_awaiting_second_click = false;
                    result = PROCESSED_EVENT_CATEGORY_RECORD;
                    BoundingBox window = {
                        MIN(two_click_command_x_0, event.mouse_x),
                        MIN(two_click_command_y_0, event.mouse_y),
                        MAX(two_click_command_x_0, event.mouse_x),
                        MAX(two_click_command_y_0, event.mouse_y)
                    };
                    for (uint32 i = 0; i < dxf_entities.length; ++i) {
                        if (bounding_box_contains(window, entity_get_bounding_box(&dxf_entities.array[i]))) {
                            set_dxf_selection_mask(i, value_to_write_to_selection_mask);
                        }
                    }
                }
            }
        } else { ASSERT(event.type == UI_EVENT_TYPE_MOUSE_3D_PRESS);
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

    auto key_lambda = [new_event](uint32 key, bool super = false, bool shift = false) -> bool {
        if (new_event.type != UI_EVENT_TYPE_KEY_PRESS) return false;
        return _key_lambda(new_event, key, super, shift);
    };

    { // space bar repeats previous command (TODO NOTE: This approach won't actually work; it needs to be in event_process)
      // (otherwise we can't differentiate between (c)enter and (c)icle)
        if (key_lambda(' ')) {
            new_event = space_bar_event;
        }
    }

    { // we handle the user pressing z by spoofing a key press at the origin
        if (key_lambda('Z')) {
            new_event = { UI_EVENT_TYPE_MOUSE_2D_PRESS, 0.0f, 0.0f };
        }
    }



    if (key_lambda('Z', true) || key_lambda('U')) { // undo
        #define _UNDO_STACK_NONEMPTY_ (history_A_undo != history_B_redo)

        if (_UNDO_STACK_NONEMPTY_) {
            bool32 just_undid_super_checkpoint = false;
            // // move history_B_redo
            // pop back _through_ a first checkpoint__NOTE_set_in_new_event_process
            do {
                --history_B_redo;
                just_undid_super_checkpoint |= (history_B_redo->checkpoint_type == CHECKPOINT_TYPE_SUPER_CHECKPOINT_MESH_SAVED_IN_UNDO_STACK);
            } while ((_UNDO_STACK_NONEMPTY_) && (history_B_redo->checkpoint_type == CHECKPOINT_TYPE_NONE));
            // * short-circuit pop back _up to_ a second checkpoint
            while ((_UNDO_STACK_NONEMPTY_) && ((history_B_redo - 1)->checkpoint_type == CHECKPOINT_TYPE_NONE)) {
                --history_B_redo;
            }

            {
                if (!just_undid_super_checkpoint) {
                    super_stacks_peek_and_load();
                } else {
                    super_stacks_undo_and_load();
                }

                conversation_reset__NOTE_basically_just_a_memset_0(true);
            }

            Event *history_D_one_after_last_super_checkpoint; {
                history_D_one_after_last_super_checkpoint = history_B_redo;
                while ((history_A_undo != history_D_one_after_last_super_checkpoint) && (history_D_one_after_last_super_checkpoint - 1)->checkpoint_type != CHECKPOINT_TYPE_SUPER_CHECKPOINT_MESH_SAVED_IN_UNDO_STACK) --history_D_one_after_last_super_checkpoint;
            }

            for (Event *event = history_D_one_after_last_super_checkpoint; event < history_B_redo; ++event) event_process(*event);

            // conversation_messagef("[undo] undo successful");
        } else {
            conversation_messagef("[undo] nothing to undo");
        }

    } else if (key_lambda('Y', true) || key_lambda('Z', true, true) || key_lambda('U', false, true)) { // redo

        if (history_B_redo != history_C_one_past_end_of_redo) {
            do {

                if (history_B_redo->checkpoint_type != CHECKPOINT_TYPE_SUPER_CHECKPOINT_MESH_SAVED_IN_UNDO_STACK) {
                    event_process(*history_B_redo);
                } else {
                    event_process(*history_B_redo, true);
                    super_stacks_redo_and_load();
                }

                ++history_B_redo;

            } while ((history_B_redo != history_C_one_past_end_of_redo) && ((history_B_redo - 1)->checkpoint_type == CHECKPOINT_TYPE_NONE));
            // FORNOW (TODO: replace with fancy pants redo)
            // conversation_messagef("[redo] redo successful");
        } else {
            conversation_messagef("[redo] nothing to redo");
        }



    } else { // process immediately
        uint32 category = event_process(new_event);

        if (category == PROCESSED_EVENT_CATEGORY_DONT_RECORD) {
        } else if (category == PROCESSED_EVENT_CATEGORY_KILL_HISTORY) {
            history_B_redo = history_C_one_past_end_of_redo = history_A_undo;
            queue_free_AND_zero(&new_event_queue);
            super_stacks_do__NOTE_kills_redo_super_stack();
            ASSERT(false); // TODO
        } else {
            { // FORNOW (sloppy): assign checkpoint_type
                new_event.checkpoint_type = CHECKPOINT_TYPE_NONE; // FORNOW (not necessary)
                if ((
                            0
                            || (category == PROCESSED_EVENT_CATEGORY_CHECKPOINT)
                            || (category == PROCESSED_EVENT_CATEGORY_EXPENSIVE_MESH_OPERATION)
                    )
                        && (!new_event.checkpoint_ineligible)
                   ) {
                    if (category == PROCESSED_EVENT_CATEGORY_CHECKPOINT) {
                        new_event.checkpoint_type = CHECKPOINT_TYPE_CHECKPOINT;
                    } else if (category == PROCESSED_EVENT_CATEGORY_EXPENSIVE_MESH_OPERATION) {
                        new_event.checkpoint_type = CHECKPOINT_TYPE_SUPER_CHECKPOINT_MESH_SAVED_IN_UNDO_STACK;
                    }
                } 
            }

            { // history_push_back(...)
                *history_B_redo++ = new_event;
                history_C_one_past_end_of_redo = history_B_redo; // kill redo "stack"
            }

            if (category == PROCESSED_EVENT_CATEGORY_EXPENSIVE_MESH_OPERATION) { // push super checkpoint if necessary
                super_stacks_do__NOTE_kills_redo_super_stack();
            }

        }
    }




    // && (new_event.key != GLFW_KEY_LEFT_SHIFT)
    // && (new_event.key != GLFW_KEY_RIGHT_SHIFT)
}

void new_event_queue_process() {
    while (new_event_queue.length) new_event_process(queue_dequeue(&new_event_queue));
}















void conversation_draw() {
    // FORNOW: repeated computation
    bool32 dxf_anything_selected;
    bool32 stl_plane_selected;
    {
        dxf_anything_selected = false;
        for (uint32 i = 0; i < dxf_entities.length; ++i) {
            if (dxf_selection_mask.array[i]) {
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
            // if (app_mode == APP_MODE_CREATE_STL) {
            //     eso_begin(globals.Identity, SOUP_QUADS);
            //     eso_color(0.07f, 0.07f, 0.07f);
            //     eso_vertex(-1.0f,  1.0f);
            //     eso_vertex(-1.0f, -1.0f);
            //     eso_vertex( 0.0f, -1.0f);
            //     eso_vertex( 0.0f,  1.0f);
            //     eso_end();
            // }
        }
    }

    { // draw 2D draw 2d draw
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(0, 0, window_width / 2, window_height);
        }
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
                for (uint32 i = 0; i < dxf_entities.length; ++i) {
                    DXFEntity *entity = &dxf_entities.array[i];
                    int32 color = (dxf_selection_mask.array[i]) ? DXF_COLOR_SELECTION : DXF_COLOR_DONT_OVERRIDE;
                    real32 dx = 0.0f;
                    real32 dy = 0.0f;
                    if ((click_mode == CLICK_MODE_DXF_MOVE) && (two_click_command_awaiting_second_click)) {
                        if (dxf_selection_mask.array[i]) {
                            dx = mouse_x - two_click_command_x_0;
                            dy = mouse_y - two_click_command_y_0;
                            color = DXF_COLOR_WATER_ONLY;
                        }
                    }
                    eso_dxf_entity__SOUP_LINES(entity, color, dx, dy);
                }
                eso_end();
            }
            { // dots
                if (show_details) {
                    eso_begin(camera_get_PV(&camera_2D), SOUP_POINTS, 8.0f);
                    eso_color(monokai.white);
                    for (DXFEntity *entity = dxf_entities.array; entity < &dxf_entities.array[dxf_entities.length]; ++entity) {
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
                eso_begin(PV_2D * M, SOUP_LINES);
                eso_color(color);
                eso_vertex(-r*.7f, 0.0f);
                eso_vertex( r, 0.0f);
                eso_vertex(0.0f, -r);
                eso_vertex(0.0f,  r*.7f);
                eso_end();
            }

            if (two_click_command_awaiting_second_click) {
                if (click_modifier == CLICK_MODIFIER_WINDOW) {
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(0.0f, 1.0f, 1.0f);
                    real32 x0 = two_click_command_x_0;
                    real32 y0 = two_click_command_y_0;
                    real32 x1 = mouse_x;
                    real32 y1 = mouse_y;
                    eso_vertex(x0, y0);
                    eso_vertex(x1, y0);
                    eso_vertex(x1, y1);
                    eso_vertex(x0, y1);
                    eso_end();
                }
                if (click_mode == CLICK_MODE_MEASURE) { // measure line
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(0.0f, 1.0f, 1.0f);
                    eso_vertex(two_click_command_x_0, two_click_command_y_0);
                    eso_vertex(mouse_x, mouse_y);
                    eso_end();
                }
                if (click_mode == CLICK_MODE_CREATE_LINE) { // measure line
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(0.0f, 1.0f, 1.0f);
                    eso_vertex(two_click_command_x_0, two_click_command_y_0);
                    eso_vertex(mouse_x, mouse_y);
                    eso_end();
                }
                if (click_mode == CLICK_MODE_CREATE_BOX) {
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(0.0f, 1.0f, 1.0f);
                    eso_vertex(two_click_command_x_0, two_click_command_y_0);
                    eso_vertex(mouse_x,               two_click_command_y_0);
                    eso_vertex(mouse_x,               mouse_y              );
                    eso_vertex(two_click_command_x_0, mouse_y              );
                    eso_end();
                }
                if (click_mode == CLICK_MODE_CREATE_CIRCLE) {
                    vec2 c = { two_click_command_x_0, two_click_command_y_0 };
                    vec2 p = { mouse_x, mouse_y };
                    real32 r = norm(c - p);
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(0.0f, 1.0f, 1.0f);
                    for (uint32 i = 0; i < NUM_SEGMENTS_PER_CIRCLE; ++i) eso_vertex(c + r * e_theta(NUM_DEN(i, NUM_SEGMENTS_PER_CIRCLE) * TAU));
                    eso_end();
                }
                if (click_mode == CLICK_MODE_CREATE_FILLET) {
                    // FORNOW
                    int i = dxf_find_closest_entity(&dxf_entities, two_click_command_x_0, two_click_command_y_0);
                    if (i != -1) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(0.0f, 1.0f, 1.0f);
                        eso_dxf_entity__SOUP_LINES(&dxf_entities.array[i], DXF_COLOR_WATER_ONLY);
                        eso_end();
                    }
                }
            }
        }
        glDisable(GL_SCISSOR_TEST);
    }

    { // 3D draw 3D 3d draw 3d
        glEnable(GL_SCISSOR_TEST);
        glScissor(window_width / 2, 0, window_width / 2, window_height);

        { // selection 2d selection 2D selection tube tubes stack stacks wire wireframe (FORNOW: ew)
            uint32 color = ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_ADD)) ? DXF_COLOR_TRAVERSE : ((enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) ? DXF_COLOR_QUALITY_1 : ((enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO) || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) ? DXF_COLOR_WATER_ONLY : DXF_COLOR_SELECTION;

            uint32 NUM_TUBE_STACKS_INCLUSIVE;
            mat4 M;
            mat4 M_incr;
            if (true) {
                if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                    // NOTE: some repetition with wrapper
                    real32 a = MIN(0.0f, MIN(console_param_1_preview, console_param_2_preview));
                    real32 b = MAX(0.0f, MAX(console_param_1_preview, console_param_2_preview));
                    real32 length = b - a;
                    NUM_TUBE_STACKS_INCLUSIVE = MIN(64, uint32(roundf(length / 2.5f)) + 2);
                    M = M_3D_from_2D * M4_Translation(-dxf_origin_x, -dxf_origin_y, a + Z_FIGHT_EPS);
                    M_incr = M4_Translation(0.0f, 0.0f, (b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                } else if ((enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
                    NUM_TUBE_STACKS_INCLUSIVE = 64;
                    M = M_3D_from_2D * M4_Translation(-dxf_origin_x, -dxf_origin_y, 0.0f);
                    real32 a = 0.0f;
                    real32 b = TAU;
                    real32 argument  = (b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1);
                    mat4 R = ((revolve_use_x_instead) ? M4_RotationAboutXAxis(argument) : M4_RotationAboutYAxis(argument));
                    M_incr = M4_Translation(dxf_origin_x, dxf_origin_y, 0.0f) * R * M4_Translation(-dxf_origin_x, -dxf_origin_y, 0.0f);
                } else if (enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO) {
                    // FORNOW
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D;
                    M_incr = M4_Identity();
                } else if (enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * M4_Translation(-dxf_origin_x, -dxf_origin_y, console_param_1_preview + Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * M4_Translation(-dxf_origin_x, -dxf_origin_y, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                }
                for (uint32 tube_stack_index = 0; tube_stack_index < NUM_TUBE_STACKS_INCLUSIVE; ++tube_stack_index) {
                    {
                        eso_begin(PV_3D * M, SOUP_LINES, 5.0f);
                        for (uint32 i = 0; i < dxf_entities.length; ++i) {
                            DXFEntity *entity = &dxf_entities.array[i];
                            if (dxf_selection_mask.array[i]) {
                                eso_dxf_entity__SOUP_LINES(entity, color);
                            }
                        }
                        eso_end();
                    }
                    M *= M_incr;
                }
            }
        }

        BoundingBox selection_bounding_box = dxf_get_bounding_box(&dxf_entities, dxf_selection_mask.array);

        if (dxf_anything_selected) { // arrow
            if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {

                vec3 color = ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_ADD)) ? V3(83.0f / 255, 255.0f / 255, 83.0f / 255.0f) : V3(1.0f, 0.0f, 0.0f);

                real32 arrow_x = 0.0f;
                real32 arrow_y = 0.0f;
                real32 H[2] = { console_param_1_preview, console_param_2_preview };
                bool32 toggle[2] = { console_params_preview_flip_flag, !console_params_preview_flip_flag };
                mat4 A = M_3D_from_2D;
                if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                    bounding_box_center(selection_bounding_box, &arrow_x, &arrow_y);
                    if (dxf_anything_selected) {
                        arrow_x -= dxf_origin_x;
                        arrow_y -= dxf_origin_y;
                    }
                } else { ASSERT((enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT));
                    H[0] = 10.0f + selection_bounding_box.max[(!revolve_use_x_instead) ? 1 : 0];
                    H[1] = 0.0f;
                    toggle[0] = false;
                    toggle[1] = false;
                    { // A
                        A *= M4_RotationAboutXAxis(RAD(-90.0f));
                        if (revolve_use_x_instead) {
                            A *= M4_RotationAboutYAxis(RAD(90.0f));
                        }
                    }
                }

                for (uint32 i = 0; i < 2; ++i) {
                    if (!IS_ZERO(H[i])) {
                        real32 total_height = ABS(H[i]);
                        real32 cap_height = (total_height > 10.0f) ? 5.0f : (0.5f * total_height);
                        real32 shaft_height = total_height - cap_height;
                        real32 s = 1.5f;
                        mat4 N = M4_Translation(0.0, 0.0, -Z_FIGHT_EPS);
                        if (toggle[i]) N = M4_Scaling(1.0f, 1.0f, -1.0f) * N;
                        mat4 M_cyl  = A * M4_Translation(arrow_x, arrow_y) * N * M4_Scaling(s * 1.0f, s * 1.0f, shaft_height) * M4_RotationAboutXAxis(RAD(90.0f));
                        mat4 M_cone = A * M4_Translation(arrow_x, arrow_y) * N * M4_Translation(0.0f, 0.0f, shaft_height) * M4_Scaling(s * 2.0f, s * 2.0f, cap_height) * M4_RotationAboutXAxis(RAD(90.0f));
                        library.meshes.cylinder.draw(P_3D, V_3D, M_cyl, color);
                        library.meshes.cone.draw(P_3D, V_3D, M_cone, color);
                    }
                }
            }
        }

        if (stl_plane_selected) { // axes 3D axes 3d axes axis 3D axis 3d axis
            real32 r = camera_3D.ortho_screen_height_World / 120.0f;
            eso_begin(PV_3D * M_3D_from_2D * M4_Translation(0.0f, 0.0f, Z_FIGHT_EPS), SOUP_LINES, 4.0f);
            eso_color(0.7f, 0.7f, 1.0f);
            // eso_color(0.0f, 0.0f, 0.0f);
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
            conversation_draw_3D_grid_box(P_3D, V_3D);
        }

        { // floating sketch plane; NOTE: transparent
            bool draw = (selected_index_of_first_triangle_hit_by_ray == -1);
            mat4 PVM = PV_3D * M_3D_from_2D;
            vec3 color = monokai.yellow;
            real32 sign = -1.0f;
            if (enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                PVM *= M4_Translation(-dxf_origin_x, -dxf_origin_y, console_param_1_preview);
                color = { 0.0f, 1.0f, 1.0f };
                sign = 1.0f;
                draw = true;
            } else if (enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO) {
                color = { 0.0f, 1.0f, 1.0f };
                sign = 1.0f;
                draw = true;
            } else {
                if (dxf_anything_selected) PVM *= M4_Translation(-dxf_origin_x, -dxf_origin_y, 0.0f); // FORNOW
            }
            real32 r = 30.0f;
            BoundingBox bounding_box;
            if (dxf_anything_selected) {
                bounding_box = selection_bounding_box;
                real32 eps = 10.0f;
                bounding_box.min[0] -= eps;
                bounding_box.max[0] += eps;
                bounding_box.min[1] -= eps;
                bounding_box.max[1] += eps;
            } else {
                bounding_box = { -r, -r, r, r };
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

    if (!hide_gui) { // gui
        {
            char click_message[256] = {};
            if (click_mode == CLICK_MODE_CREATE_FILLET) {
                sprintf(click_message, "radius:%gmm", console_param_1_preview);
            }
            gui_printf("[Click] %s %s %s",
                    (click_mode == CLICK_MODE_NONE) ? "NONE" :
                    (click_mode == CLICK_MODE_ORIGIN_MOVE) ? "MOVE_ORIGIN_TO" :
                    (click_mode == CLICK_MODE_SELECT) ? "SELECT" :
                    (click_mode == CLICK_MODE_DESELECT) ? "DESELECT" :
                    (click_mode == CLICK_MODE_MEASURE) ? "MEASURE" :
                    (click_mode == CLICK_MODE_CREATE_LINE) ? "CREATE_LINE" :
                    (click_mode == CLICK_MODE_CREATE_BOX) ? "CREATE_BOX" :
                    (click_mode == CLICK_MODE_CREATE_CIRCLE) ? "CREATE_CIRCLE" :
                    (click_mode == CLICK_MODE_CREATE_FILLET) ? "CREATE_FILLET" :
                    (click_mode == CLICK_MODE_DXF_MOVE) ? "MOVE_DXF_TO" :
                    "???",
                    (click_modifier == CLICK_MODE_NONE) ? "" :
                    (click_modifier == CLICK_MODIFIER_CONNECTED) ? "CONNECTED" :
                    (click_modifier == CLICK_MODIFIER_WINDOW) ? "WINDOW" :
                    (click_modifier == CLICK_MODIFIER_SNAP_TO_CENTER_OF) ? "CENTER_OF" :
                    (click_modifier == CLICK_MODIFIER_SNAP_TO_END_OF) ? "END_OF" :
                    (click_modifier == CLICK_MODIFIER_SNAP_TO_MIDDLE_OF) ? "MIDDLE_OF" :
                    (click_modifier == CLICK_MODIFIER_QUALITY) ? "QUALITY" :
                    "???",
                    click_message);
        }

        {
            char enter_message[256] = {};
            if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO)) {
                real32 p, p2;
                char glyph, glyph2;
                if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                    p      =  console_param_1_preview;
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
                    ASSERT(enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO);
                    p      = console_param_1_preview;
                    p2     = console_param_2_preview;
                    glyph  = 'x';
                    glyph2 = 'y';
                }
                sprintf(enter_message, "%c:%gmm %c:%gmm", glyph, p, glyph2, p2);
                if (((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) && IS_ZERO(console_param_2_preview)) sprintf(enter_message, "%c:%gmm", glyph, p);
            } else if ((enter_mode == ENTER_MODE_OPEN) || (enter_mode == ENTER_MODE_SAVE)) {
                sprintf(enter_message, "%s%s", conversation_drop_path, console_buffer);
            }

            gui_printf("[Enter] %s %s",
                    (enter_mode == ENTER_MODE_EXTRUDE_ADD) ? "EXTRUDE_ADD" :
                    (enter_mode == ENTER_MODE_EXTRUDE_CUT) ? "EXTRUDE_CUT" :
                    (enter_mode == ENTER_MODE_REVOLVE_ADD) ? "REVOLVE_ADD" :
                    (enter_mode == ENTER_MODE_REVOLVE_CUT) ? "REVOLVE_CUT" :
                    (enter_mode == ENTER_MODE_OPEN) ? "OPEN" :
                    (enter_mode == ENTER_MODE_SAVE) ? "SAVE" :
                    (enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO) ? "MOVE_DXF_ORIGIN_TO" :
                    (enter_mode == ENTER_MODE_OFFSET_PLANE_BY) ? "OFFSET_PLANE_TO" :
                    (enter_mode == ENTER_MODE_NONE) ? "NONE" :
                    "???",
                    enter_message);
        }

        if ((enter_mode == ENTER_MODE_NONE) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
            gui_printf("> %s", console_buffer);
        } else {
            gui_printf("> %s", console_buffer);
        }

        conversation_message_buffer_update_and_draw();

        if (show_details) {
            gui_printf("%d dxf_entities elements", dxf_entities.length);
            gui_printf("%d stl triangles", fancy_mesh.num_triangles);
        }
        if (show_command_stack) history_gui_printf();
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
            gui_printf("show/hide-(h)elp");
            gui_printf("(Escape)-from-current-enter_and_click_modes");
            gui_printf("(s)elect (d)eselect + (c)onected (a)ll [Click] / (q)uality + (012345)");
            gui_printf("(y)-cycle-through-top-front-right-planes");
            gui_printf("(e)trude-add (E)xtrude-cut + (0123456789.,) (Tab)-flip-direction [Enter]");
            gui_printf("(Ctrl + z)-undo (Ctrl+Z)-redo (Ctrl+y)-redo");
            gui_printf("(Ctrl + o)pen (Ctrl+s)ave + ... + [Enter]");
            gui_printf("(Ctrl + r)eload-dxf-if-edited-elsewhere (Ctrl + R)-clear-stl");
            gui_printf("show-(g)rid (.)-show-details show-event-stac(k)-for-debugging");
            gui_printf("zoom-to-e(X)tents");
            gui_printf("(0)-toggle-camera-perspective-orthographic");
            gui_printf("(Z)-move-origin + (c)enter-of (e)nd-of (m)iddle-of [Click] / (-0123456789.,) (f)lip-direction [Enter]");
            gui_printf("(M)easure + (c)enter-of (e)nd-of (m)iddle-of (z)-origin [Click]");
            gui_printf("");
            gui_printf("EXPERIMENTAL: (r)evolve-add (R)evolve-cut");
            gui_printf("");
            gui_printf("you can drag and drop *.dxf and *.stl into Conversation");
        }
    }
}



int main() {
    // _window_set_size(1.5 * 640.0, 1.5 * 360.0);

    conversation_init();

    conversation_messagef("type h for help `// pre-alpha " __DATE__ " " __TIME__);

    {
        // FORNOW: first frame position
        double xpos, ypos;
        glfwGetCursorPos(COW0._window_glfw_window, &xpos, &ypos);
        callback_cursor_position(NULL, xpos, ypos);
    }

    while (cow_begin_frame()) {

        // invariants
        ASSERT(dxf_entities.length == dxf_selection_mask.length);

        new_event_queue_process();
        { // stuff that still shims globals.*
            if ((!globals.mouse_left_held && !globals.mouse_right_held) || globals.mouse_left_pressed || globals.mouse_right_pressed) {
                hot_pane = (callback_xpos <= window_get_width() / 2) ? HOT_PANE_2D : HOT_PANE_3D;
                if ((click_modifier == CLICK_MODIFIER_WINDOW) && (two_click_command_awaiting_second_click)) hot_pane = HOT_PANE_2D;// FORNOW
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


