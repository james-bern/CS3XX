// TODO: get text input into the popup system
// TODO: figure out system for exact (x, y) coordinates
// TODO: figure out system for space bar

// // TODO: fidget spinner
// holding shift to magic_snap mouse to 15 deg
// CLICK_MODE_MOVE_FEATURE_REVOLVE_AXIS (two_click_command)

// TODO: hotpanegui (mouse stuff)

// // TODO (soon): feature freeeze
// TODO: rewrite cow/snail in C

#include "cs345.cpp"
#include "manifoldc.h"
#include "poe.cpp"
#define u32 DO_NOT_USE_u32_USE_uint32_INSTEAD
#include "burkardt.cpp"
#include "conversation.h"
#include "elephant.cpp"


// TODO: ghostly section view on 2D side

// TODO: ? remove cow callback dependencies

// TODO: pressing x to have coordinates input with boxes
// TODO: a pop up window api (moving away from console)
// TODO: being able to type equations into boxes

// TODO: draw snapshotted_world_states
// TODO: remove special case on record, the mouse event should include whether this is a drag type event
// TODO: gui printf that doesn't automatically add a newline
// TODOLATER: don't record saving
// TODO: repeated keys shouldn't be recorded (unless enter mode changes) -- enter mode lambda in process

//////////////////////////////////////////////////
// DEBUG /////////////////////////////////////////
//////////////////////////////////////////////////

// #define DEBUG_DISABLE_HISTORY


//////////////////////////////////////////////////
// GLOBAL STATE //////////////////////////////////
//////////////////////////////////////////////////

WorldState global_world_state;
ScreenState _global_screen_state;


bool32 HACK_DONT_DRAW_IMGUI_UNDO_REDO_OTHERWISE_POPUPS_WEIRDNESS;


//////////////////////////////////////////////////
// NON-ZERO INITIALIZERS /////////////////////////
//////////////////////////////////////////////////

void init_cameras() {
    _global_screen_state.camera_2D = { 100.0f, 0.0, 0.0f, -0.5f, -0.125f };
    // FORNOW
    if (global_world_state.dxf.entities.length) camera2D_zoom_to_bounding_box(&_global_screen_state.camera_2D, dxf_entities_get_bounding_box(&global_world_state.dxf.entities));
    _global_screen_state.camera_3D = { 2.0f * _global_screen_state.camera_2D.screen_height_World, CAMERA_3D_DEFAULT_ANGLE_OF_VIEW, RAD(33.0f), RAD(-44.0f), 0.0f, 0.0f, 0.5f, -0.125f };
}


//////////////////////////////////////////////////
// GETTERS (STATE NOT WORTH TROUBLE OF STORING) //
//////////////////////////////////////////////////

mat4 get_M_3D_from_2D() {
    vec3 up = { 0.0f, 1.0f, 0.0f };
    real32 dot_product = dot(global_world_state.feature_plane.normal, up);
    vec3 y = (ARE_EQUAL(ABS(dot_product), 1.0f)) ? V3(0.0f,  0.0f, -1.0f * SGN(dot_product)) : up;
    vec3 x = normalized(cross(y, global_world_state.feature_plane.normal));
    vec3 z = cross(x, y);

    // FORNOW
    if (ARE_EQUAL(ABS(dot_product), 1.0f) && SGN(dot_product) < 0.0f) {
        y *= -1;
    }

    return M4_xyzo(x, y, z, (global_world_state.feature_plane.signed_distance_to_world_origin) * global_world_state.feature_plane.normal);
}


void get_console_params(real32 *console_param_1, real32 *console_param_2) {
    char buffs[2][64] = {};
    uint32 buff_i = 0;
    uint32 i = 0;
    for (char *c = global_world_state.console.buffer; (*c) != '\0'; ++c) {
        if (*c == ',') {
            if (++buff_i == 2) break;
            i = 0;
            continue;
        }
        buffs[buff_i][i++] = (*c);
    }
    real32 sign = 1.0f;
    if ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
        sign = (!global_world_state.console.flip_flag) ? 1.0f : -1.0f;
    }
    *console_param_1 = sign * strtof(buffs[0], NULL);
    *console_param_2 = sign * strtof(buffs[1], NULL);
    if ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
        *console_param_2 *= -1;
    }
}


//////////////////////////////////////////////////
// STATE-DEPENDENT UTILITY FUNCTIONS /////////////
//////////////////////////////////////////////////

bool32 callback_mouse_shift_held; // TODO (where does this go?)

vec2 magic_snap(vec2 before) {
    vec2 result = before;
    {
        if (global_world_state.modes.click_modifier == CLICK_MODIFIER_SNAP_TO_CENTER_OF) {
            real32 min_squared_distance = HUGE_VAL;
            for (DXFEntity *entity = global_world_state.dxf.entities.array; entity < &global_world_state.dxf.entities.array[global_world_state.dxf.entities.length]; ++entity) {
                if (entity->type == DXF_ENTITY_TYPE_LINE) {
                    continue;
                } else { ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                    DXFArc *arc = &entity->arc;
                    real32 squared_distance = squared_distance_point_dxf_arc(before.x, before.y, arc);
                    if (squared_distance < min_squared_distance) {
                        min_squared_distance = squared_distance;
                        result = arc->center;
                    }
                }
            }
        } else if (global_world_state.modes.click_modifier == CLICK_MODIFIER_SNAP_TO_MIDDLE_OF) {
            real32 min_squared_distance = HUGE_VAL;
            for (DXFEntity *entity = global_world_state.dxf.entities.array; entity < &global_world_state.dxf.entities.array[global_world_state.dxf.entities.length]; ++entity) {
                real32 squared_distance = squared_distance_point_dxf_entity(before.x, before.y, entity);
                if (squared_distance < min_squared_distance) {
                    min_squared_distance = squared_distance;
                    entity_get_middle(entity, &result.x, &result.y);
                }
            }
        } else if (global_world_state.modes.click_modifier == CLICK_MODIFIER_SNAP_TO_END_OF) {
            real32 min_squared_distance = HUGE_VAL;
            for (DXFEntity *entity = global_world_state.dxf.entities.array; entity < &global_world_state.dxf.entities.array[global_world_state.dxf.entities.length]; ++entity) {
                real32 x[2], y[2];
                entity_get_start_and_end_points(entity, &x[0], &y[0], &x[1], &y[1]);
                for (uint32 d = 0; d < 2; ++d) {
                    real32 squared_distance = squared_distance_point_point(before.x, before.y, x[d], y[d]);
                    if (squared_distance < min_squared_distance) {
                        min_squared_distance = squared_distance;
                        result.x = x[d];
                        result.y = y[d];
                    }
                }
            }
        } else if (
                (global_world_state.modes.click_mode == CLICK_MODE_CREATE_LINE)
                && (global_world_state.two_click_command.awaiting_second_click)
                && (callback_mouse_shift_held)) {
            vec2 a = global_world_state.two_click_command.first_click;
            vec2 b = before;
            vec2 r = b - a; 
            real32 norm_r = norm(r);
            real32 factor = 360 / 15 / TAU;
            real32 theta = roundf(atan2(r) * factor) / factor;
            result = a + norm_r * e_theta(theta);
        }
    }
    return result;
}

// TODO: angle_map

void mesh_draw(mat4 P_3D, mat4 V_3D, mat4 M_3D) {
    mat4 PVM_3D = P_3D * V_3D * M_3D;
    real32 console_param_1, console_param_2; {
        get_console_params(&console_param_1, &console_param_2);
    }

    if (global_world_state.mesh.cosmetic_edges) {
        eso_begin(PVM_3D, SOUP_LINES); 
        eso_color(monokai.black);
        // 3 * num_triangles * 2 / 2
        for (uint32 k = 0; k < 2 * global_world_state.mesh.num_cosmetic_edges; ++k) eso_vertex(global_world_state.mesh.vertex_positions, global_world_state.mesh.cosmetic_edges[k]);
        eso_end();
    }
    for (uint32 pass = 0; pass <= 1; ++pass) {
        eso_begin(PVM_3D, (!_global_screen_state.show_details) ? SOUP_TRIANGLES : SOUP_OUTLINED_TRIANGLES);

        mat3 inv_transpose_V_3D = inverse(transpose(M3(V_3D(0, 0), V_3D(0, 1), V_3D(0, 2), V_3D(1, 0), V_3D(1, 1), V_3D(1, 2), V_3D(2, 0), V_3D(2, 1), V_3D(2, 2))));

        for (uint32 i = 0; i < global_world_state.mesh.num_triangles; ++i) {
            vec3 n = get(global_world_state.mesh.triangle_normals, i);
            vec3 p[3];
            real32 x_n;
            {
                for (uint32 j = 0; j < 3; ++j) p[j] = get(global_world_state.mesh.vertex_positions, global_world_state.mesh.triangle_indices[3 * i + j]);
                x_n = dot(n, p[0]);
            }
            vec3 color; 
            real32 alpha;
            {
                vec3 n_camera = inv_transpose_V_3D * n;
                vec3 color_n = V3(0.5f + 0.5f * n_camera.x, 0.5f + 0.5f * n_camera.y, 1.0f);
                if ((global_world_state.feature_plane.is_active) && (dot(n, global_world_state.feature_plane.normal) > 0.999f) && (ABS(x_n - global_world_state.feature_plane.signed_distance_to_world_origin) < 0.001f)) {
                    if (pass == 0) continue;
                    color = V3(0.85f, 0.87f, 0.30f);
                    alpha = ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) && ((global_world_state.console.flip_flag) || (console_param_2 != 0.0f))) ? 0.7f : 1.0f;
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

//////////////////////////////////////////////////
// LOADING AND SAVING STATE TO/FROM DISK /////////
//////////////////////////////////////////////////

void conversation_dxf_load(char *filename, bool preserve_cameras_and_dxf_origin = false) {
    if (!poe_file_exists(filename)) {
        conversation_messagef("[load] \"%s\" not found", filename);
        return;
    }

    list_free_AND_zero(&global_world_state.dxf.entities);

    dxf_entities_load(filename, &global_world_state.dxf.entities);

    if (!preserve_cameras_and_dxf_origin) {
        // camera2D_zoom_to_bounding_box(&_global_screen_state.camera_2D, dxf_entities_get_bounding_box(&global_world_state.dxf.entities));
        init_cameras();
        global_world_state.dxf.origin.x = 0.0f;
        global_world_state.dxf.origin.y = 0.0f;
    }

    strcpy(_global_screen_state.dxf_filename_for_reload, filename);

    conversation_messagef("[load] loaded %s", filename);
}

void conversation_stl_load(char *filename, bool preserve_cameras = false) {
    if (!poe_file_exists(filename)) {
        conversation_messagef("[load] \"%s\" not found", filename);
        return;
    }
    {
        stl_load(filename, &global_world_state.mesh);
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
    if (mesh_save_stl(&global_world_state.mesh, filename) ) {
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

void drop_callback(GLFWwindow *, int count, const char **paths) {
    if (count > 0) {
        char *filename = (char *) paths[0];
        conversation_load(filename);
        { // conversation_set_drop_path(filename)
            for (uint32 i = 0; i < strlen(filename); ++i) {
                _global_screen_state.drop_path[i] = filename[i];
                if (filename[i] == '.') {
                    while (
                            (i != 0) &&
                            (_global_screen_state.drop_path[i - 1] != '\\') &&
                            (_global_screen_state.drop_path[i - 1] != '/')
                          ) --i;
                    _global_screen_state.drop_path[i] = '\0';
                    break;
                }
            }
        }
    }
}
BEGIN_PRE_MAIN { glfwSetDropCallback(COW0._window_glfw_window, drop_callback); } END_PRE_MAIN;


//////////////////////////////////////////////////
// GATHERING NEW EVENTS FROM USER ////////////////
//////////////////////////////////////////////////

Queue<UserEvent> queue_of_fresh_events_from_user;
// TODO clean up
bool32 callback_mouse_left_held;
real32 callback_xpos; // FORNOW
real32 callback_ypos; // FORNOW

void callback_key(GLFWwindow *, int key, int, int action, int mods) {
    if ((action == GLFW_PRESS) && (key == GLFW_KEY_LEFT_SHIFT)) callback_mouse_shift_held = true;
    if ((action == GLFW_RELEASE) && (key == GLFW_KEY_LEFT_SHIFT)) callback_mouse_shift_held = true;


    if (action == GLFW_PRESS || (action == GLFW_REPEAT)) {
        // NOTE: we do NOT forward bare modifier presses
        if (key == GLFW_KEY_LEFT_CONTROL)  return;
        if (key == GLFW_KEY_RIGHT_CONTROL) return;
        if (key == GLFW_KEY_LEFT_SUPER)    return;
        if (key == GLFW_KEY_RIGHT_SUPER)   return;
        if (key == GLFW_KEY_LEFT_SHIFT)    return;
        if (key == GLFW_KEY_RIGHT_SHIFT)   return;

        UserEvent event = {};
        event.type = USER_EVENT_TYPE_KEY_PRESS;
        event.key = key;
        event.super = (mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER));
        event.shift = (mods & GLFW_MOD_SHIFT);
        queue_enqueue(&queue_of_fresh_events_from_user, event);

        // if (event.
    }
}

void callback_cursor_position(GLFWwindow *, double xpos, double ypos) {
    _callback_cursor_position(NULL, xpos, ypos); // FORNOW TODO TODO TODO SHIM

    callback_xpos = (real32) (xpos * COW0._window_macbook_retina_scale_ONLY_USED_FOR_FIXING_CURSOR_POS);
    callback_ypos = (real32) (ypos * COW0._window_macbook_retina_scale_ONLY_USED_FOR_FIXING_CURSOR_POS);

    // // mouse held generates mouse presses
    // FORNOW repeated from callback_mouse_button
    if ((_global_screen_state.hot_pane == HOT_PANE_2D) && (callback_mouse_left_held) && (((global_world_state.modes.click_mode == CLICK_MODE_SELECT) || (global_world_state.modes.click_mode == CLICK_MODE_DESELECT)) && (global_world_state.modes.click_modifier != CLICK_MODIFIER_WINDOW))) {
        vec2 mouse_s_NDC = transformPoint(_window_get_NDC_from_Screen(), V2(callback_xpos, callback_ypos));
        UserEvent event = {};
        event.type = USER_EVENT_TYPE_MOUSE_2D_PRESS;
        {
            vec2 s_2D = transformPoint(inverse(camera_get_PV(&_global_screen_state.camera_2D)), mouse_s_NDC);
            event.mouse = magic_snap(s_2D);
        }
        event.mouse_held = true;
        queue_enqueue(&queue_of_fresh_events_from_user, event);
    }
}

void callback_mouse_button(GLFWwindow *, int button, int action, int) {
    _callback_mouse_button(NULL, button, action, 0); // FORNOW TODO TODO TODO SHIM

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) { 
            callback_mouse_left_held = true;

            vec2 mouse_s_NDC = transformPoint(_window_get_NDC_from_Screen(), V2(callback_xpos, callback_ypos));
            if (_global_screen_state.hot_pane == HOT_PANE_2D) {
                UserEvent event = {};
                event.type = USER_EVENT_TYPE_MOUSE_2D_PRESS;
                {
                    vec2 s_2D = transformPoint(inverse(camera_get_PV(&_global_screen_state.camera_2D)), mouse_s_NDC);
                    event.mouse = magic_snap(s_2D);
                }
                queue_enqueue(&queue_of_fresh_events_from_user, event);
            } else if (_global_screen_state.hot_pane == HOT_PANE_3D) {
                UserEvent event = {};
                event.type = USER_EVENT_TYPE_MOUSE_3D_PRESS;
                {
                    mat4 inverse_PV_3D = inverse(camera_get_PV(&_global_screen_state.camera_3D));
                    event.o = transformPoint(inverse_PV_3D, V3(mouse_s_NDC, -1.0f));
                    event.dir = normalized(transformPoint(inverse_PV_3D, V3(mouse_s_NDC,  1.0f)) - event.o);
                }
                queue_enqueue(&queue_of_fresh_events_from_user, event);
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

// TODO: this API should match what is printed to the terminal in verbose output mode
//       (so we can copy and paste a session for later use as an end to end test)

UserEvent KEY_event(uint32 key, bool32 super = false, bool32 shift = false) {
    ASSERT(!(('a' <= key) && (key <= 'z')));
    UserEvent event = {};
    event.type = USER_EVENT_TYPE_KEY_PRESS;
    event.key = key;
    event.super = super;
    event.shift = shift;
    return event;
}

UserEvent MOUSE_2D_event(real32 mouse_x, real32 mouse_y) {
    UserEvent event = {};
    event.type = USER_EVENT_TYPE_MOUSE_2D_PRESS;
    event.mouse.x = mouse_x;
    event.mouse.y = mouse_y;
    return event;
}

UserEvent MOUSE_3D_event(real32 o_x, real32 o_y, real32 o_z, real32 dir_x, real32 dir_y, real32 dir_z) {
    UserEvent event = {};
    event.type = USER_EVENT_TYPE_MOUSE_3D_PRESS;
    event.o = { o_x, o_y, o_z };
    event.dir = { dir_x, dir_y, dir_z };
    return event;
}



/////////////////////////////////////////////////////////
// PROCESSING A SINGLE STANDARD EVENT ///////////////////
/////////////////////////////////////////////////////////

// NOTE: this function is called on "fresh event" (direct from user) AND by undo and redo routines
// NOTE: this sometimes writes to standard_event (setting its category)--it never reads
//       (FORNOW: it will (over-)write the same data every time. *shrug*)
//       really it should just write on the fresh event, and then leave alone for undo/redo
// NOTE: this sometimes modifies global_world_state.dxf
// NOTE: this sometimes modifies global_world_state.mesh (and frees what used to be there)

void standard_event_process(
        UserEvent *_standard_event,
        bool32 skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack = false
        ) {


    // TODO: cleaner handling of NONE-type event

    // // short names (we have no reason to ever overwrite these things)
    uint32 enter_mode            = global_world_state.modes.enter_mode;
    uint32 click_mode            = global_world_state.modes.click_mode;
    uint32 click_modifier        = global_world_state.modes.click_modifier;
    bool32 awaiting_second_click = global_world_state.two_click_command.awaiting_second_click;
    real32 first_click_x         = global_world_state.two_click_command.first_click.x;
    real32 first_click_y         = global_world_state.two_click_command.first_click.y;

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    UserEvent effective_event = *_standard_event;

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // TRANSFORMATION LAYER (EFFECTIVE_EVENT) ////////////////////////////////////////////////////////////
    // (popups, TODO: move Z in here) ////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////


    // TODO: cursor in popup
    // TODO: fleury ops
    // TODOLATER: gui weirdness on undo (do we need to draw anything?--no) -- this is unimportant; and is solveable by just turning off graphics updates globally during all the undo
    // -- TODO: it's worth thinking about this more but fornow at least i really think the only "problem" is that gui_printf is getting called multiple times per frame

    bool32 popup_popup_ate_this_event = false;
    bool32 popup_popup_actually_called_this_event = false;
#include "popup_lambda.cpp"
    { // popup_popups
        real32 *param0               = &global_world_state.popup.param0;
        real32 *param1               = &global_world_state.popup.param1;
        real32 *circle_diameter      = &global_world_state.popup.circle_diameter;
        real32 *circle_radius        = &global_world_state.popup.circle_radius;
        real32 *circle_circumference = &global_world_state.popup.circle_circumference;
        real32 *fillet_radius   = &global_world_state.popup.fillet_radius;
        {
            if (click_mode == CLICK_MODE_CREATE_CIRCLE) {
                if (awaiting_second_click) {
                    real32 prev_circle_diameter = *circle_diameter;
                    real32 prev_circle_radius = *circle_radius;
                    real32 prev_circle_circumference = *circle_circumference;
                    if (popup_popup("circle_diameter", circle_diameter, "circle_radius", circle_radius, "circle_circumference", circle_circumference)) {
                        effective_event = MOUSE_2D_event(first_click_x + *circle_radius, first_click_y); // FORNOW
                    } else {
                        if (*circle_diameter != prev_circle_diameter) {
                            *circle_radius = *circle_diameter / 2;
                            *circle_circumference = PI * *circle_diameter;
                        } else if (*circle_radius != prev_circle_radius) {
                            *circle_diameter = 2 * *circle_radius;
                            *circle_circumference = PI * *circle_diameter;
                        } else if (*circle_circumference != prev_circle_circumference) {
                            *circle_diameter = *circle_circumference / PI;
                            *circle_radius = *circle_diameter / 2;
                        }
                    }
                }
            } else if (click_mode == CLICK_MODE_CREATE_BOX) {
                if (awaiting_second_click) {
                    if (popup_popup("box_width", param0, "box_length", param1)) {
                        effective_event = MOUSE_2D_event(first_click_x + *param0, first_click_y + *param1);
                    }
                }
            } else if (click_mode == CLICK_MODE_CREATE_FILLET) {
                popup_popup("fillet radius", fillet_radius);
            } else if (click_modifier == CLICK_MODIFIER_EXACT_X_Y_COORDINATES) { // sus calling this a modifier but okay
                if (popup_popup("x coordinate", param0, "y coordinate", param1)) {
                    effective_event = MOUSE_2D_event(*param0, *param1);
                }
            } else {
                *param0 = 0;
                *param1 = 0;
            }
        }
    }
    if (!popup_popup_actually_called_this_event) global_world_state.popup._active_popup_unique_ID__FORNOW_name0 = NULL;
    if (popup_popup_ate_this_event) {
        _standard_event->record_me = true;
        return;
    }


    bool32 is_key_event = (effective_event.type == USER_EVENT_TYPE_KEY_PRESS);
    bool32 is_mouse_2d_event = (effective_event.type == USER_EVENT_TYPE_MOUSE_2D_PRESS);
    bool32 is_mouse_3d_event = (effective_event.type == USER_EVENT_TYPE_MOUSE_3D_PRESS);
    // bool32 enter = (is_key_event && (effective_event.key == GLFW_KEY_ENTER));

    // // computed variables
    bool32 extrude = ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT));
    bool32 revolve = ((enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT));
    bool32 add     = ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_ADD));
    bool32 cut     = ((enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_CUT));
    bool32 dxf_anything_selected; {
        dxf_anything_selected = false;
        for (uint32 i = 0; i < global_world_state.dxf.entities.length; ++i) {
            if (global_world_state.dxf.entities.array[i].is_selected) {
                dxf_anything_selected = true;
                break;
            }
        }
    }
    real32 console_param_1, console_param_2; get_console_params(&console_param_1, &console_param_2); // FORNOW

    { // _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(extrude);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(revolve);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(add);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(cut);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(is_key_event);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(is_mouse_2d_event);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(is_mouse_3d_event);
    }

    // easy read functions
    auto DXF_ADD = [&](DXFEntity entity) { list_push_back(&global_world_state.dxf.entities, entity); };
    auto DXF_DELETE = [&](uint32 i) { list_delete_at(&global_world_state.dxf.entities, i); };

    // TODO: aliases (TODO: collect key aliases into here too)
    vec2 first_click = global_world_state.two_click_command.first_click;
    vec2 mouse = { effective_event.mouse.x, effective_event.mouse.y };
    vec2 second_click = mouse;

    //////////////////////////////////////////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (effective_event.type == USER_EVENT_TYPE_NONE) {
        _standard_event->record_me = false;
    } else if (effective_event.type == USER_EVENT_TYPE_KEY_PRESS) {
        _standard_event->record_me = true; // FORNOW: event recorded is the default
        auto key_lambda = [effective_event](uint32 key, bool super = false, bool shift = false) -> bool {
            if (effective_event.type != USER_EVENT_TYPE_KEY_PRESS) return false; // TODO: ASSERT
            return _key_lambda(effective_event, key, super, shift);
        };

        bool32 click_mode_SNAP_ELIGIBLE_ =
            0
            || (global_world_state.modes.click_mode == CLICK_MODE_SET_ORIGIN)
            || (global_world_state.modes.click_mode == CLICK_MODE_SET_AXIS)
            || (global_world_state.modes.click_mode == CLICK_MODE_MEASURE)
            || (global_world_state.modes.click_mode == CLICK_MODE_CREATE_LINE)
            || (global_world_state.modes.click_mode == CLICK_MODE_CREATE_BOX)
            || (global_world_state.modes.click_mode == CLICK_MODE_CREATE_CIRCLE)
            || (global_world_state.modes.click_mode == CLICK_MODE_MOVE_DXF_ENTITIES)
            ;

        // TODO: _modes.enter_mode_NEEDS_CONSOLE
        // FORNOW: assume all enters require console (even though technically rotate doesn't right now)


        char character_equivalent; {
            character_equivalent = (char) effective_event.key;
            if (effective_event.shift && (effective_event.key == '-')) character_equivalent = '_';
            if ((!effective_event.shift) && ('A' <= effective_event.key) && (effective_event.key <= 'Z')) character_equivalent = (char) ('a' + (effective_event.key - 'A'));
        }

        {
            bool32 key_eaten_by_quality_number_special_case;
            {
                key_eaten_by_quality_number_special_case = false;
                if (global_world_state.modes.click_modifier == CLICK_MODIFIER_QUALITY) {
                    for (uint32 color = 0; color <= 9; ++color) {
                        if (key_lambda('0' + color)) {
                            _standard_event->checkpoint_me = true;
                            key_eaten_by_quality_number_special_case = true;
                            for (uint32 i = 0; i < global_world_state.dxf.entities.length; ++i) {
                                if (global_world_state.dxf.entities.array[i].color == color) {
                                    bool32 value_to_write_to_selection_mask = (global_world_state.modes.click_mode == CLICK_MODE_SELECT);
                                    global_world_state.dxf.entities.array[i].is_selected = value_to_write_to_selection_mask;
                                }
                            }
                            global_world_state.modes.click_mode = CLICK_MODE_NONE;
                            global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
                            break;
                        }
                    }
                }
            }
            bool send_key_to_console;
            {
                send_key_to_console = false;
                if (!key_lambda(GLFW_KEY_ENTER)) {
                    if (0
                            || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD)
                            || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)
                            || (global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN)
                            || (global_world_state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY)
                            || (global_world_state.modes.click_mode == CLICK_MODE_CREATE_FILLET)
                       ) {
                        send_key_to_console |= key_lambda(GLFW_KEY_BACKSPACE);
                        send_key_to_console |= key_lambda('.');
                        send_key_to_console |= key_lambda('-');
                        for (uint32 i = 0; i < 10; ++i) send_key_to_console |= key_lambda('0' + i);
                        if ((global_world_state.modes.enter_mode != ENTER_MODE_OFFSET_PLANE_BY)) {
                            send_key_to_console |= key_lambda(',');
                        }
                    } else if ((global_world_state.modes.enter_mode == ENTER_MODE_OPEN) || (global_world_state.modes.enter_mode == ENTER_MODE_SAVE)) {
                        send_key_to_console |= key_lambda(GLFW_KEY_BACKSPACE);
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
                        if (global_world_state.modes.enter_mode == ENTER_MODE_NONE) {
                            conversation_messagef("[enter] enter mode is none");
                            send_key_to_console = false;
                        } else if (extrude || revolve) {
                            if (!dxf_anything_selected) {
                                conversation_messagef("[enter] no global_world_state.dxf.entities elements is_selected");
                                send_key_to_console = false;
                            } else if (!global_world_state.feature_plane.is_active) {
                                conversation_messagef("[enter] no plane is_selected");
                                send_key_to_console = false;
                            } else if (cut && (global_world_state.mesh.num_triangles == 0)) { // FORNOW
                                conversation_messagef("[enter] nothing to cut");
                                send_key_to_console = false;
                            } else if (extrude) {
                                if (IS_ZERO(console_param_1) && IS_ZERO(console_param_2)) {
                                    conversation_messagef("[enter] extrude height is zero");
                                    send_key_to_console = false;
                                }
                            } else {
                                ASSERT(revolve);
                                ;
                            }
                        } else if (global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN) {
                            ;
                        } else if (global_world_state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                            ;
                        }
                    }
                }
            }

            if (key_eaten_by_quality_number_special_case) {
                ;
            } else if (send_key_to_console) {
                if (!key_lambda(GLFW_KEY_ENTER)) {
                    if (key_lambda(GLFW_KEY_BACKSPACE)) {
                        if (global_world_state.console.num_bytes_written != 0) global_world_state.console.buffer[--global_world_state.console.num_bytes_written] = '\0';
                    } else {
                        global_world_state.console.buffer[global_world_state.console.num_bytes_written++] = character_equivalent;
                    }
                } else { // wrapper
                    if (extrude || revolve) {
                        if (!skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack) {
                            _standard_event->snapshot_me = _standard_event->checkpoint_me = true;
                            { // result.mesh
                                CrossSectionEvenOdd cross_section = cross_section_create_FORNOW_QUADRATIC(&global_world_state.dxf.entities, true);
                                Mesh tmp = wrapper_manifold(
                                        &global_world_state.mesh,
                                        cross_section.num_polygonal_loops,
                                        cross_section.num_vertices_in_polygonal_loops,
                                        cross_section.polygonal_loops,
                                        get_M_3D_from_2D(),
                                        global_world_state.modes.enter_mode,
                                        console_param_1,
                                        console_param_2,
                                        global_world_state.dxf.origin,
                                        global_world_state.dxf.axis_base_point,
                                        global_world_state.dxf.axis_angle_from_y);
                                cross_section_free(&cross_section);

                                mesh_free_AND_zero(&global_world_state.mesh); // FORNOW
                                global_world_state.mesh = tmp; // FORNOW
                            }


                            if (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) conversation_messagef("[extrude-add] success");
                            if (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) conversation_messagef("[extrude-cut] success");
                            if (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) conversation_messagef("[revolve-add] success");
                            if (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT) conversation_messagef("[revolve-cut] success");
                            if (revolve) conversation_messagef("[revolve] success");
                        }

                        { // reset some stuff
                            for (uint32 i = 0; i < global_world_state.dxf.entities.length; ++i) global_world_state.dxf.entities.array[i].is_selected = false;
                        }
                    } else if (global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN) {
                        global_world_state.dxf.origin.x = console_param_1;
                        global_world_state.dxf.origin.y = console_param_2;
                    } else { ASSERT((global_world_state.modes.enter_mode == ENTER_MODE_OPEN) || (global_world_state.modes.enter_mode == ENTER_MODE_SAVE));
                        static char full_filename_including_path[512];
                        sprintf(full_filename_including_path, "%s%s", _global_screen_state.drop_path, global_world_state.console.buffer);
                        if (global_world_state.modes.enter_mode == ENTER_MODE_OPEN) {
                            if (poe_suffix_match(full_filename_including_path, ".dxf")) {
                                _standard_event->snapshot_me = _standard_event->checkpoint_me = true;
                                conversation_dxf_load(full_filename_including_path,
                                        skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack
                                        || (strcmp(full_filename_including_path, _global_screen_state.dxf_filename_for_reload) == 0));
                            } else if (poe_suffix_match(full_filename_including_path, ".stl")) {
                                conversation_stl_load(full_filename_including_path);
                            } else {
                                conversation_messagef("[open] %s not found", full_filename_including_path);
                            }
                        } else { ASSERT(global_world_state.modes.enter_mode == ENTER_MODE_SAVE);
                            conversation_save(full_filename_including_path);
                        }
                    }
                    global_world_state.modes.enter_mode = {};
                    global_world_state.console = {};
                }
            } else if (key_lambda('Q', true)) {
                exit(1);
            } else if (key_lambda('0')) {
                _standard_event->record_me = false;
                _global_screen_state.camera_3D.angle_of_view = CAMERA_3D_DEFAULT_ANGLE_OF_VIEW - _global_screen_state.camera_3D.angle_of_view;
            } else if (key_lambda('X', false, true)) {
                _standard_event->record_me = false;
                camera2D_zoom_to_bounding_box(&_global_screen_state.camera_2D, dxf_entities_get_bounding_box(&global_world_state.dxf.entities));
            } else if (key_lambda('G')) {
                _standard_event->record_me = false;
                _global_screen_state.hide_grid = !_global_screen_state.hide_grid;
            } else if (key_lambda('H')) {
                _standard_event->record_me = false;
                _global_screen_state.show_help = !_global_screen_state.show_help;
            } else if (key_lambda('.')) { 
                _standard_event->record_me = false;
                _global_screen_state.show_details = !_global_screen_state.show_details;
            } else if (key_lambda('K')) { 
                _standard_event->record_me = false;
                _global_screen_state.show_event_stack = !_global_screen_state.show_event_stack;
            } else if (key_lambda('K', false, true)) {
                _standard_event->record_me = false;
                _global_screen_state.hide_gui = !_global_screen_state.hide_gui;
            } else if (key_lambda('O', true)) {
                global_world_state.modes.enter_mode = ENTER_MODE_OPEN;
            } else if (key_lambda('S', true)) {
                global_world_state.modes.enter_mode = ENTER_MODE_SAVE;
            } else if (key_lambda(COW_KEY_ESCAPE)) {
                global_world_state.modes = {};
                global_world_state.console = {};
            } else if (key_lambda('1')) { // FORNOW
                _standard_event->checkpoint_me = true;
                // FORNOW
                global_world_state.console.flip_flag = !global_world_state.console.flip_flag;
            } else if (key_lambda('N')) {
                if (global_world_state.feature_plane.is_active) {
                    global_world_state.modes.enter_mode = ENTER_MODE_OFFSET_PLANE_BY;
                    global_world_state.console = {};
                } else {
                    conversation_messagef("[n] no plane is_selected");
                }
            } else if (key_lambda('Z', false, true)) {
                global_world_state.modes.click_mode = CLICK_MODE_SET_ORIGIN;
                global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
            } else if (key_lambda('A', false, true)) {
                global_world_state.modes.click_mode = CLICK_MODE_SET_AXIS;
                global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
                global_world_state.two_click_command.awaiting_second_click = false;
            } else if (key_lambda('S')) {
                global_world_state.modes.click_mode = CLICK_MODE_SELECT;
                global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
            } else if (key_lambda('D')) {
                global_world_state.modes.click_mode = CLICK_MODE_DESELECT;
                global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
            } else if (key_lambda('C')) {
                if (((global_world_state.modes.click_mode == CLICK_MODE_SELECT) || (global_world_state.modes.click_mode == CLICK_MODE_DESELECT)) && (global_world_state.modes.click_modifier != CLICK_MODIFIER_CONNECTED)) {
                    global_world_state.modes.click_modifier = CLICK_MODIFIER_CONNECTED;
                } else if (click_mode_SNAP_ELIGIBLE_) {
                    _standard_event->record_me = false;
                    global_world_state.modes.click_modifier = CLICK_MODIFIER_SNAP_TO_CENTER_OF;
                } else {
                    global_world_state.modes.click_mode = CLICK_MODE_CREATE_CIRCLE;
                    global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
                    global_world_state.two_click_command.awaiting_second_click = false;
                    _global_screen_state.space_bar_event_key = 'C'; // FORNOW; TODO perhaps key_lambda can store the entirety of the standard_event
                    // update_space_bar_event_off_of_most_recent_key_lambda
                }
            } else if (key_lambda('Q')) {
                if ((global_world_state.modes.click_mode == CLICK_MODE_SELECT) || (global_world_state.modes.click_mode == CLICK_MODE_DESELECT)) {
                    global_world_state.modes.click_modifier = CLICK_MODIFIER_QUALITY;
                }
            } else if (key_lambda('A')) {
                if ((global_world_state.modes.click_mode == CLICK_MODE_SELECT) || (global_world_state.modes.click_mode == CLICK_MODE_DESELECT)) {
                    _standard_event->checkpoint_me = true;
                    bool32 value_to_write_to_selection_mask = (global_world_state.modes.click_mode == CLICK_MODE_SELECT);
                    for (uint32 i = 0; i < global_world_state.dxf.entities.length; ++i) global_world_state.dxf.entities.array[i].is_selected = value_to_write_to_selection_mask;
                    global_world_state.modes.click_mode = CLICK_MODE_NONE;
                    global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
                }
            } else if (key_lambda('Y')) {
                // TODO: 'Y' remembers last terminal choice of plane for next time
                _standard_event->checkpoint_me = true;

                // already one of the three primary planes
                if ((global_world_state.feature_plane.is_active) && ARE_EQUAL(global_world_state.feature_plane.signed_distance_to_world_origin, 0.0f) && ARE_EQUAL(squaredNorm(global_world_state.feature_plane.normal), 1.0f) && ARE_EQUAL(maxComponent(global_world_state.feature_plane.normal), 1.0f)) {
                    global_world_state.feature_plane.normal = { global_world_state.feature_plane.normal[2], global_world_state.feature_plane.normal[0], global_world_state.feature_plane.normal[1] };
                } else {
                    global_world_state.feature_plane.is_active = true;
                    global_world_state.feature_plane.signed_distance_to_world_origin = 0.0f;
                    global_world_state.feature_plane.normal = { 0.0f, 1.0f, 0.0f };
                }
            } else if (key_lambda('E')) {
                if (click_mode_SNAP_ELIGIBLE_) {
                    _standard_event->record_me = false;
                    global_world_state.modes.click_modifier = CLICK_MODIFIER_SNAP_TO_END_OF;
                } else {
                    // _standard_event->checkpoint_me = true;
                    global_world_state.modes.enter_mode = ENTER_MODE_EXTRUDE_ADD;
                    global_world_state.console = {};
                }
            } else if (key_lambda('M')) {
                if (click_mode_SNAP_ELIGIBLE_) {
                    _standard_event->record_me = false;
                    global_world_state.modes.click_modifier = CLICK_MODIFIER_SNAP_TO_MIDDLE_OF;
                } else {
                    _standard_event->checkpoint_me = true;
                    global_world_state.modes.click_mode = CLICK_MODE_MOVE_DXF_ENTITIES;
                    global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
                    global_world_state.two_click_command.awaiting_second_click = false;
                }
            } else if (key_lambda('E', false, true)) {
                // _standard_event->checkpoint_me = true;
                global_world_state.modes.enter_mode = ENTER_MODE_EXTRUDE_CUT;
                global_world_state.console = {};
                global_world_state.console.flip_flag = true;
            } else if (key_lambda('R')) {
                global_world_state.modes.enter_mode = ENTER_MODE_REVOLVE_ADD;
            } else if (key_lambda('R', false, true)) {
                global_world_state.modes.enter_mode = ENTER_MODE_REVOLVE_CUT;
            } else if (key_lambda('W')) {
                if ((global_world_state.modes.click_mode == CLICK_MODE_SELECT) || (global_world_state.modes.click_mode == CLICK_MODE_DESELECT)) {
                    global_world_state.modes.click_modifier = CLICK_MODIFIER_WINDOW;
                    global_world_state.two_click_command.awaiting_second_click = false;
                }
            } else if (key_lambda('M', false, true)) {
                _standard_event->record_me = false;
                global_world_state.modes.click_mode = CLICK_MODE_MEASURE;
                global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
                global_world_state.two_click_command.awaiting_second_click = false;
            } else if (key_lambda('L')) {
                global_world_state.modes.click_mode = CLICK_MODE_CREATE_LINE;
                global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
                global_world_state.two_click_command.awaiting_second_click = false;
                _global_screen_state.space_bar_event_key = 'L'; // FORNOW 
            } else if (key_lambda('F')) {
                global_world_state.modes.click_mode = CLICK_MODE_CREATE_FILLET;
                global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
                global_world_state.modes.enter_mode = ENTER_MODE_NONE;
                global_world_state.two_click_command.awaiting_second_click = false;
            } else if (key_lambda('B')) {
                global_world_state.modes.click_mode = CLICK_MODE_CREATE_BOX;
                global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
                global_world_state.two_click_command.awaiting_second_click = false;
                _global_screen_state.space_bar_event_key = 'B'; // FORNOW
            } else if (key_lambda(GLFW_KEY_BACKSPACE) || key_lambda(COW_KEY_DELETE)) {
                for (int32 i = global_world_state.dxf.entities.length - 1; i >= 0; --i) {
                    if (global_world_state.dxf.entities.array[i].is_selected) {
                        DXF_DELETE(i);
                    }
                }
                for (uint32 i = 0; i < global_world_state.dxf.entities.length; ++i) global_world_state.dxf.entities.array[i].is_selected = false;
            } else {
                _standard_event->record_me = false;
                ;
            }
        }
    } else if (effective_event.type == USER_EVENT_TYPE_MOUSE_2D_PRESS) {
        _standard_event->record_me = true; // FORNOW: event recorded is the default
        auto set_dxf_selection_mask = [&] (uint32 i, bool32 value_to_write) {
            // Only remember global_world_state.dxf.entities selection operations that actually change the mask
            // NOTE: we could instead do a memcmp at the end, but let's stick with the simple bool32 result = false; ... ret result; approach fornow
            if (global_world_state.dxf.entities.array[i].is_selected != value_to_write) {
                _standard_event->record_me = true;
                _standard_event->checkpoint_me = (!effective_event.mouse_held);
                global_world_state.dxf.entities.array[i].is_selected = value_to_write;
            }
        };


        bool32 value_to_write_to_selection_mask = (global_world_state.modes.click_mode == CLICK_MODE_SELECT);
        if (/////
                (
                 (global_world_state.modes.click_mode == CLICK_MODE_SET_AXIS) ||
                 (global_world_state.modes.click_mode == CLICK_MODE_MEASURE) ||
                 (global_world_state.modes.click_mode == CLICK_MODE_CREATE_LINE) ||
                 (global_world_state.modes.click_mode == CLICK_MODE_CREATE_BOX) ||
                 (global_world_state.modes.click_mode == CLICK_MODE_CREATE_CIRCLE) ||
                 (global_world_state.modes.click_mode == CLICK_MODE_CREATE_FILLET) ||
                 (global_world_state.modes.click_mode == CLICK_MODE_MOVE_DXF_ENTITIES) ||
                 (((global_world_state.modes.click_mode == CLICK_MODE_SELECT) || (global_world_state.modes.click_mode == CLICK_MODE_DESELECT)) && (global_world_state.modes.click_modifier == CLICK_MODIFIER_WINDOW)))
                && (!global_world_state.two_click_command.awaiting_second_click)
           ) { //
            if (global_world_state.modes.click_mode == CLICK_MODE_MEASURE) _standard_event->record_me = false;
            if (!(((global_world_state.modes.click_mode == CLICK_MODE_SELECT) || (global_world_state.modes.click_mode == CLICK_MODE_DESELECT)) && (global_world_state.modes.click_modifier == CLICK_MODIFIER_WINDOW))) global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
            global_world_state.two_click_command.awaiting_second_click = true;
            global_world_state.two_click_command.first_click.x = effective_event.mouse.x;
            global_world_state.two_click_command.first_click.y = effective_event.mouse.y;
        } else if (global_world_state.modes.click_mode == CLICK_MODE_MEASURE) {
            ASSERT(global_world_state.two_click_command.awaiting_second_click);
            global_world_state.two_click_command.awaiting_second_click = false;
            global_world_state.modes.click_mode = CLICK_MODE_NONE;
            global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
            real32 angle = DEG(atan2(effective_event.mouse.y - global_world_state.two_click_command.first_click.y, effective_event.mouse.x - global_world_state.two_click_command.first_click.x));
            if (angle < 0.0f) angle += 360.0f;
            conversation_messagef("%gmm %gdeg", sqrt(squared_distance_point_point(global_world_state.two_click_command.first_click.x, global_world_state.two_click_command.first_click.y, effective_event.mouse.x, effective_event.mouse.y)), angle);
        } else if (global_world_state.modes.click_mode == CLICK_MODE_CREATE_LINE) {
            ASSERT(global_world_state.two_click_command.awaiting_second_click);
            global_world_state.two_click_command.awaiting_second_click = false;
            _standard_event->checkpoint_me = true;
            global_world_state.modes.click_mode = CLICK_MODE_NONE;
            global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
            DXF_ADD({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, global_world_state.two_click_command.first_click.x, global_world_state.two_click_command.first_click.y, effective_event.mouse.x, effective_event.mouse.y });
        } else if (global_world_state.modes.click_mode == CLICK_MODE_CREATE_BOX) {
            ASSERT(global_world_state.two_click_command.awaiting_second_click);
            global_world_state.two_click_command.awaiting_second_click = false;
            _standard_event->checkpoint_me = true;
            global_world_state.modes.click_mode = CLICK_MODE_NONE;
            global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
            DXF_ADD({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, global_world_state.two_click_command.first_click.x, global_world_state.two_click_command.first_click.y, effective_event.mouse.x, global_world_state.two_click_command.first_click.y });
            DXF_ADD({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, global_world_state.two_click_command.first_click.x, global_world_state.two_click_command.first_click.y, global_world_state.two_click_command.first_click.x, effective_event.mouse.y });
            DXF_ADD({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, effective_event.mouse.x, effective_event.mouse.y, effective_event.mouse.x, global_world_state.two_click_command.first_click.y });
            DXF_ADD({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, effective_event.mouse.x, effective_event.mouse.y, global_world_state.two_click_command.first_click.x, effective_event.mouse.y });
        } else if (global_world_state.modes.click_mode == CLICK_MODE_MOVE_DXF_ENTITIES) {
            ASSERT(global_world_state.two_click_command.awaiting_second_click);
            global_world_state.two_click_command.awaiting_second_click = false;
            _standard_event->checkpoint_me = true;
            global_world_state.modes.click_mode = CLICK_MODE_NONE;
            global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
            real32 dx = effective_event.mouse.x - global_world_state.two_click_command.first_click.x;
            real32 dy = effective_event.mouse.y - global_world_state.two_click_command.first_click.y;
            for (uint32 i = 0; i < global_world_state.dxf.entities.length; ++i) {
                DXFEntity *entity = &global_world_state.dxf.entities.array[i];
                if (entity->is_selected) {
                    if (entity->type == DXF_ENTITY_TYPE_LINE) {
                        DXFLine *line = &entity->line;
                        line->start.x += dx;
                        line->start.y += dy;
                        line->end.x   += dx;
                        line->end.y   += dy;
                    } else { ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                        DXFArc *arc = &entity->arc;
                        arc->center.x += dx;
                        arc->center.y += dy;
                    }
                }
            }
        } else if (global_world_state.modes.click_mode == CLICK_MODE_CREATE_CIRCLE) {
            ASSERT(global_world_state.two_click_command.awaiting_second_click);
            global_world_state.two_click_command.awaiting_second_click = false;
            _standard_event->checkpoint_me = true;
            global_world_state.modes.click_mode = CLICK_MODE_NONE;
            global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
            real32 theta_a_in_degrees = DEG(atan2(effective_event.mouse.y - global_world_state.two_click_command.first_click.y, effective_event.mouse.x - global_world_state.two_click_command.first_click.x));
            real32 theta_b_in_degrees = theta_a_in_degrees + 180.0f;
            real32 r = SQRT(squared_distance_point_point(global_world_state.two_click_command.first_click.x, global_world_state.two_click_command.first_click.y, effective_event.mouse.x, effective_event.mouse.y));
            DXF_ADD({ DXF_ENTITY_TYPE_ARC, DXF_COLOR_TRAVERSE, global_world_state.two_click_command.first_click.x, global_world_state.two_click_command.first_click.y, r, theta_a_in_degrees, theta_b_in_degrees });
            DXF_ADD({ DXF_ENTITY_TYPE_ARC, DXF_COLOR_TRAVERSE, global_world_state.two_click_command.first_click.x, global_world_state.two_click_command.first_click.y, r, theta_b_in_degrees, theta_a_in_degrees });
        } else if (global_world_state.modes.click_mode == CLICK_MODE_SET_ORIGIN) {
            _standard_event->checkpoint_me = true;
            global_world_state.dxf.origin = mouse;
            global_world_state.modes = {};
        } else if (global_world_state.modes.click_mode == CLICK_MODE_SET_AXIS) {
            ASSERT(global_world_state.two_click_command.awaiting_second_click);
            global_world_state.two_click_command.awaiting_second_click = false;
            _standard_event->checkpoint_me = true;
            global_world_state.dxf.axis_base_point = first_click;
            global_world_state.dxf.axis_angle_from_y = (-PI / 2) + atan2(second_click - first_click);
            global_world_state.modes.click_mode = CLICK_MODE_NONE;
            global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
        } else if (global_world_state.modes.click_mode == CLICK_MODE_CREATE_FILLET) {
            ASSERT(global_world_state.two_click_command.awaiting_second_click);
            global_world_state.two_click_command.awaiting_second_click = false;
            _standard_event->checkpoint_me = true;
            global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
            int i = dxf_find_closest_entity(&global_world_state.dxf.entities, global_world_state.two_click_command.first_click.x, global_world_state.two_click_command.first_click.y);
            int j = dxf_find_closest_entity(&global_world_state.dxf.entities, effective_event.mouse.x, effective_event.mouse.y);
            if ((i != j) && (i != -1) && (j != -1)) {
                real32 radius = global_world_state.popup.fillet_radius;
                DXFEntity *E_i = &global_world_state.dxf.entities.array[i];
                DXFEntity *E_j = &global_world_state.dxf.entities.array[j];
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
                            vec2 m1 = { global_world_state.two_click_command.first_click.x, global_world_state.two_click_command.first_click.y };
                            vec2 m2 = { effective_event.mouse.x, effective_event.mouse.y };
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
                            DXF_DELETE(MAX(i, j));
                            DXF_DELETE(MIN(i, j));

                            DXF_ADD({ DXF_ENTITY_TYPE_LINE, color_i, s_ab.x, s_ab.y, t_ab.x, t_ab.y });
                            DXF_ADD({ DXF_ENTITY_TYPE_LINE, color_j, s_cd.x, s_cd.y, t_cd.x, t_cd.y });

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

                                DXF_ADD({ DXF_ENTITY_TYPE_ARC, DXF_COLOR_TRAVERSE, center.x, center.y, radius, theta_ab_in_degrees, theta_cd_in_degrees });
                            }
                        }
                    }
                } else {
                    conversation_messagef("TODO: line-arc fillet; arc-arc fillet");
                }
            }
        } else if ((global_world_state.modes.click_mode == CLICK_MODE_SELECT) || (global_world_state.modes.click_mode == CLICK_MODE_DESELECT)) {
            _standard_event->record_me = false;
            if (global_world_state.modes.click_modifier != CLICK_MODIFIER_WINDOW) {
                int hot_entity_index = dxf_find_closest_entity(&global_world_state.dxf.entities, effective_event.mouse.x, effective_event.mouse.y);
                if (hot_entity_index != -1) {
                    if (global_world_state.modes.click_modifier == CLICK_MODIFIER_CONNECTED) {
                        #if 1 // TODO: consider just using the O(n*m) algorithm here instead

                        #define GRID_CELL_WIDTH 0.001f

                        auto scalar_bucket = [&](real32 a) -> real32 {
                            return roundf(a / GRID_CELL_WIDTH) * GRID_CELL_WIDTH;
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

                            for (uint32 entity_index = 0; entity_index < global_world_state.dxf.entities.length; ++entity_index) {
                                DXFEntity *entity = &global_world_state.dxf.entities.array[entity_index];

                                real32 start_x, start_y, end_x, end_y;
                                entity_get_start_and_end_points(entity, &start_x, &start_y, &end_x, &end_y);
                                push_into_grid_unless_cell_full__make_cell_if_none_exists(start_x, start_y, entity_index, false);
                                push_into_grid_unless_cell_full__make_cell_if_none_exists(end_x, end_y, entity_index, true);
                            }
                        }

                        bool32 *edge_marked = (bool32 *) calloc(global_world_state.dxf.entities.length, sizeof(bool32));

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
                                DXFEntity *entity = &global_world_state.dxf.entities.array[point->entity_index];
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
                                    entity_get_start_point(&global_world_state.dxf.entities.array[hot_entity_index], &x, &y);
                                } else {
                                    entity_get_end_point(&global_world_state.dxf.entities.array[hot_entity_index], &x, &y);
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
            } else { ASSERT(global_world_state.modes.click_modifier == CLICK_MODIFIER_WINDOW);
                ASSERT(global_world_state.two_click_command.awaiting_second_click);
                global_world_state.two_click_command.awaiting_second_click = false;
                BoundingBox window = {
                    MIN(global_world_state.two_click_command.first_click.x, effective_event.mouse.x),
                    MIN(global_world_state.two_click_command.first_click.y, effective_event.mouse.y),
                    MAX(global_world_state.two_click_command.first_click.x, effective_event.mouse.x),
                    MAX(global_world_state.two_click_command.first_click.y, effective_event.mouse.y)
                };
                for (uint32 i = 0; i < global_world_state.dxf.entities.length; ++i) {
                    if (bounding_box_contains(window, dxf_entity_get_bounding_box(&global_world_state.dxf.entities.array[i]))) {
                        set_dxf_selection_mask(i, value_to_write_to_selection_mask);
                    }
                }
            }
        }
    } else { ASSERT(effective_event.type == USER_EVENT_TYPE_MOUSE_3D_PRESS);
        _standard_event->record_me = false;
        int32 index_of_first_triangle_hit_by_ray = -1;
        {
            real32 min_distance = HUGE_VAL;
            for (uint32 i = 0; i < global_world_state.mesh.num_triangles; ++i) {
                vec3 p[3]; {
                    for (uint32 j = 0; j < 3; ++j) p[j] = get(global_world_state.mesh.vertex_positions, global_world_state.mesh.triangle_indices[3 * i + j]);
                }
                RayTriangleIntersectionResult ray_triangle_intersection_result = ray_triangle_intersection(effective_event.o, effective_event.dir, p[0], p[1], p[2]);
                if (ray_triangle_intersection_result.hit) {
                    if (ray_triangle_intersection_result.distance < min_distance) {
                        min_distance = ray_triangle_intersection_result.distance;
                        index_of_first_triangle_hit_by_ray = i; // FORNOW
                    }
                }
            }
        }

        if (index_of_first_triangle_hit_by_ray != -1) { // something hit
            _standard_event->checkpoint_me = _standard_event->record_me = true;
            global_world_state.feature_plane.is_active = true;
            {
                global_world_state.feature_plane.normal = get(global_world_state.mesh.triangle_normals, index_of_first_triangle_hit_by_ray);
                { // FORNOW (gross) calculateion of global_world_state.feature_plane.signed_distance_to_world_origin
                    vec3 a_selected = get(global_world_state.mesh.vertex_positions, global_world_state.mesh.triangle_indices[3 * index_of_first_triangle_hit_by_ray + 0]);
                    global_world_state.feature_plane.signed_distance_to_world_origin = dot(global_world_state.feature_plane.normal, a_selected);
                }
            }
        }
    }
}


//////////////////////////////////////////////////
// HISTORY ///////////////////////////////////////
//////////////////////////////////////////////////

#ifdef DEBUG_DISABLE_HISTORY
//
void history_process_and_potentially_checkpoint_and_or_snapshot_standard_fresh_user_event(UserEvent standard_event) { standard_event_process(&standard_event); }
void history_undo() { conversation_messagef("[DEBUG] history disabled"); }
void history_redo() { conversation_messagef("[DEBUG] history disabled"); }
void history_debug_draw() { gui_printf("[DEBUG] history disabled"); }
//
#else

// TODO: consider different ElephantStack internals

struct {
    ElephantStack<UserEvent> recorded_user_events;
    ElephantStack<WorldState> snapshotted_world_states;
} history;

struct StackPointers {
    UserEvent *user_event;
    WorldState *world_state;
};

StackPointers POP_UNDO_ONTO_REDO() {
    StackPointers result = {};
    result.user_event = elephant_pop_undo_onto_redo(&history.recorded_user_events);
    if (result.user_event->snapshot_me) result.world_state = elephant_pop_undo_onto_redo(&history.snapshotted_world_states);
    return result;
};

StackPointers POP_REDO_ONTO_UNDO() {
    StackPointers result = {};
    result.user_event = elephant_pop_redo_onto_undo(&history.recorded_user_events);
    if (result.user_event->snapshot_me) result.world_state = elephant_pop_redo_onto_undo(&history.snapshotted_world_states);
    return result;
};

StackPointers PEEK_UNDO() {
    UserEvent *user_event = elephant_peek_undo(&history.recorded_user_events);
    WorldState *world_state = elephant_is_empty_undo(&history.snapshotted_world_states) ? NULL : elephant_peek_undo(&history.snapshotted_world_states);
    return { user_event, world_state };
}

bool32 EVENT_UNDO_NONEMPTY() {
    return !elephant_is_empty_undo(&history.recorded_user_events);
}

bool32 EVENT_REDO_NONEMPTY() {
    return !elephant_is_empty_redo(&history.recorded_user_events);
}

void PUSH_UNDO_CLEAR_REDO(UserEvent standard_event) {
    elephant_push_undo_clear_redo(&history.recorded_user_events, standard_event);
    { // clear the world_state redo stack
        for (////
                WorldState *world_state = history.snapshotted_world_states._redo_stack.array;
                world_state < history.snapshotted_world_states._redo_stack.array + history.snapshotted_world_states._redo_stack.length;
                ++world_state
            ) {//
            world_state_free_AND_zero(world_state);
        }
        elephant_clear_redo(&history.snapshotted_world_states); // TODO ?
    }
    if (standard_event.snapshot_me) {
        WorldState snapshot;
        world_state_deep_copy(&snapshot, &global_world_state);
        elephant_push_undo_clear_redo(&history.snapshotted_world_states, snapshot); // TODO: clear is unnecessary here
    }
}

void history_process_and_potentially_checkpoint_and_or_snapshot_standard_fresh_user_event(UserEvent standard_event) {
    standard_event_process(&standard_event);
    if (standard_event.record_me) PUSH_UNDO_CLEAR_REDO(standard_event);
}

void history_undo() {
    if (elephant_is_empty_undo(&history.recorded_user_events)) {
        conversation_messagef("[undo] nothing to undo");
        return;
    }

    { // // manipulate stacks (undo -> redo)
        // 1) pop through a first checkpoint 
        // 2) pop up to a second checkpoint
        while (EVENT_UNDO_NONEMPTY()) { if (POP_UNDO_ONTO_REDO().user_event->checkpoint_me) break; }
        while (EVENT_UNDO_NONEMPTY()) {
            if (PEEK_UNDO().user_event->checkpoint_me) break;
            POP_UNDO_ONTO_REDO();
        }
    }
    UserEvent *one_past_end = elephant_undo_ptr_one_past_end(&history.recorded_user_events);
    UserEvent *begin;
    { // // find beginning
        // 1) walk back to snapshot event (or end of stack)
        // TODO: this feels kind of sloppy still
        begin = one_past_end - 1; // ?
        world_state_free_AND_zero(&global_world_state);
        while (true) {
            if (begin <= elephant_undo_ptr_begin(&history.recorded_user_events)) {
                begin =  elephant_undo_ptr_begin(&history.recorded_user_events); // !
                global_world_state = {};
                break;
            }
            if (begin->snapshot_me) {
                world_state_deep_copy(&global_world_state, PEEK_UNDO().world_state);
                break;
            }
            --begin;
        }
    }
    for (UserEvent *event = begin; event < one_past_end; ++event) standard_event_process(event);
    conversation_messagef("[undo] success");
}

void history_redo() {
    if (elephant_is_empty_redo(&history.recorded_user_events)) {
        conversation_messagef("[redo] nothing to redo");
        return;
    }

    while (EVENT_REDO_NONEMPTY()) { // // manipulate stacks (undo <- redo)
        StackPointers popped = POP_REDO_ONTO_UNDO();
        UserEvent *user_event = popped.user_event;
        WorldState *world_state = popped.world_state;

        // process or partial-process-and-copy-from-snapshots
        standard_event_process(user_event, (world_state != NULL));
        if (world_state) {
            world_state_free_AND_zero(&global_world_state);
            world_state_deep_copy(&global_world_state, world_state);
        }

        if (user_event->checkpoint_me) break;
    }
    conversation_messagef("[redo] success");
}


void _history_user_event_draw_helper(UserEvent *event) {
    char message[256]; {
        if (event->type == USER_EVENT_TYPE_KEY_PRESS) {
            if (event->key == GLFW_KEY_ENTER) {
                sprintf(message, "[KEY] ENTER");
            } else if (event->key == GLFW_KEY_TAB) {
                sprintf(message, "[KEY] TAB");
            } else if (event->key == GLFW_KEY_ESCAPE) {
                sprintf(message, "[KEY] ESCAPE");
            } else if (event->key == GLFW_KEY_BACKSPACE) {
                sprintf(message, "[KEY] BACKSPACE");
            } else if (event->key == GLFW_KEY_DELETE) {
                sprintf(message, "[KEY] DELETE");
            } else {
                sprintf(message, "[KEY] %s%s%c", (event->super) ? "CTRL+" : "", (event->shift) ? "SHIFT+" : "", (char) (event->key));
            }
        } else if (event->type == USER_EVENT_TYPE_MOUSE_2D_PRESS) {
            sprintf(message, "[MOUSE_2D] %g %g", event->mouse.x, event->mouse.y);
        } else { ASSERT(event->type == USER_EVENT_TYPE_MOUSE_3D_PRESS);
            sprintf(message, "[MOUSE_3D] %g %g %g %g %g %g", event->o.x, event->o.y, event->o.z, event->dir.x, event->dir.y, event->dir.z);
        }
    }
    gui_printf("%c%c %s",
            (event->checkpoint_me)   ? 'C' : ' ',
            (event->snapshot_me)     ? 'S' : ' ',
            message);
}

void _history_world_state_draw_helper(WorldState *world_state) {
    gui_printf("%d dxf elements   %d stl triangles", world_state->dxf.entities.length, world_state->mesh.num_triangles);
}

void history_debug_draw() {
    gui_printf("");
    if (history.snapshotted_world_states._redo_stack.length) {
        for (////
                WorldState *world_state = history.snapshotted_world_states._redo_stack.array;
                world_state < history.snapshotted_world_states._redo_stack.array + history.snapshotted_world_states._redo_stack.length;
                ++world_state
            ) {//
            _history_world_state_draw_helper(world_state);
        }
        gui_printf("`^ redo (%d)", elephant_length_redo(&history.snapshotted_world_states));
    }
    if ((history.snapshotted_world_states._redo_stack.length) || (history.snapshotted_world_states._undo_stack.length)) gui_printf("`  SNAPSHOTTED_WORLD_STATES");
    if (history.snapshotted_world_states._undo_stack.length) {
        gui_printf("`v undo (%d)", elephant_length_undo(&history.snapshotted_world_states));
        for (////
                WorldState *world_state = history.snapshotted_world_states._undo_stack.array + (history.snapshotted_world_states._undo_stack.length - 1);
                world_state >= history.snapshotted_world_states._undo_stack.array;
                --world_state
            ) {//
            _history_world_state_draw_helper(world_state);
        }
    }

    gui_printf("");

    if (history.recorded_user_events._redo_stack.length) {
        for (////
                UserEvent *event = history.recorded_user_events._redo_stack.array;
                event < history.recorded_user_events._redo_stack.array + history.recorded_user_events._redo_stack.length;
                ++event
            ) {//
            _history_user_event_draw_helper(event);
        }
        gui_printf("`^ redo (%d)", elephant_length_redo(&history.recorded_user_events));
    }
    if ((history.recorded_user_events._redo_stack.length) || (history.recorded_user_events._undo_stack.length)) gui_printf("`  RECORDED_USER_EVENTS");
    if (history.recorded_user_events._undo_stack.length) {
        gui_printf("`v undo (%d)", elephant_length_undo(&history.recorded_user_events));
        for (////
                UserEvent *event = history.recorded_user_events._undo_stack.array + (history.recorded_user_events._undo_stack.length - 1);
                event >= history.recorded_user_events._undo_stack.array;
                --event
            ) {//
            _history_user_event_draw_helper(event);
        }
    }
}

#endif


//////////////////////////////////////////////////
// PROCESS A FRESH (potentially special) EVENT ///
//////////////////////////////////////////////////
// TODOLATER: new document

void fresh_event_from_user_process(UserEvent fresh_event_from_user) {
    auto key_lambda = [fresh_event_from_user](uint32 key, bool super = false, bool shift = false) -> bool {
        if (fresh_event_from_user.type != USER_EVENT_TYPE_KEY_PRESS) return false;
        return _key_lambda(fresh_event_from_user, key, super, shift);
    };

    { // translation of one event into another: special cases that modify fresh_event_from_user
        {// space bar repeats previous command (TODO NOTE: This approach won't actually work; it needs to be in standard_event_process)
            // (otherwise we can't differentiate between (c)enter and (c)icle)
            if (key_lambda(' ')) {
                fresh_event_from_user = {};
                fresh_event_from_user.type = USER_EVENT_TYPE_KEY_PRESS;
                fresh_event_from_user.key = _global_screen_state.space_bar_event_key;
            }
        }

        { // we handle the user pressing z by spoofing a key press at the origin
            if (key_lambda('Z')) {
                fresh_event_from_user = {};
                fresh_event_from_user.type = USER_EVENT_TYPE_MOUSE_2D_PRESS;
                fresh_event_from_user.mouse = {};
            }
        }
    }

    if (key_lambda('Z', true) || key_lambda('U')) {
        HACK_DONT_DRAW_IMGUI_UNDO_REDO_OTHERWISE_POPUPS_WEIRDNESS = true;
        history_undo();
        HACK_DONT_DRAW_IMGUI_UNDO_REDO_OTHERWISE_POPUPS_WEIRDNESS = false;
    } else if (key_lambda('Y', true) || key_lambda('Z', true, true) || key_lambda('U', false, true)) {
        HACK_DONT_DRAW_IMGUI_UNDO_REDO_OTHERWISE_POPUPS_WEIRDNESS = true;
        history_redo();
        HACK_DONT_DRAW_IMGUI_UNDO_REDO_OTHERWISE_POPUPS_WEIRDNESS = false;
    } else {
        history_process_and_potentially_checkpoint_and_or_snapshot_standard_fresh_user_event(fresh_event_from_user);
    }
}


//////////////////////////////////////////////////
// DRAW() ////////////////////////////////////////
//////////////////////////////////////////////////

void conversation_draw() {
    mat4 P_2D = camera_get_P(&_global_screen_state.camera_2D);
    mat4 V_2D = camera_get_V(&_global_screen_state.camera_2D);
    mat4 PV_2D = P_2D * V_2D;
    mat4 M_3D_from_2D = get_M_3D_from_2D();

    // FORNOW: sloppy--these change mid-frame
    real32 console_param_1, console_param_2;
    get_console_params(&console_param_1, &console_param_2);

    bool32 extrude = ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT));
    bool32 revolve = ((global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT));
    bool32 add     = ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD));
    bool32 cut     = ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT));

    { // _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(extrude);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(revolve);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(add);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(cut);
    }

    // FORNOW: repeated computation; TODO function
    bool32 dxf_anything_selected;
    {
        dxf_anything_selected = false;
        for (uint32 i = 0; i < global_world_state.dxf.entities.length; ++i) {
            if (global_world_state.dxf.entities.array[i].is_selected) {
                dxf_anything_selected = true;
                break;
            }
        }
    }



    // aliases
    vec2 first_click = global_world_state.two_click_command.first_click;

    // preview
    vec2 preview_mouse = magic_snap(mouse_get_position(PV_2D));
    vec2 preview_dxf_origin; {
        if (global_world_state.modes.click_mode != CLICK_MODE_SET_ORIGIN) {
            preview_dxf_origin = global_world_state.dxf.origin;
        } else {
            preview_dxf_origin = preview_mouse;
        }
    }
    vec2 preview_dxf_axis_base_point;
    real32 preview_dxf_axis_angle_from_y;
    {
        if (global_world_state.modes.click_mode != CLICK_MODE_SET_AXIS) {
            preview_dxf_axis_base_point = global_world_state.dxf.axis_base_point;
            preview_dxf_axis_angle_from_y = global_world_state.dxf.axis_angle_from_y;
        } else if (!global_world_state.two_click_command.awaiting_second_click) {
            preview_dxf_axis_base_point = preview_mouse;
            preview_dxf_axis_angle_from_y = global_world_state.dxf.axis_angle_from_y;
        } else {
            preview_dxf_axis_base_point = first_click;
            preview_dxf_axis_angle_from_y = atan2(preview_mouse - preview_dxf_axis_base_point) - PI / 2;
        }
    }


    mat4 P_3D = camera_get_P(&_global_screen_state.camera_3D);
    mat4 V_3D = camera_get_V(&_global_screen_state.camera_3D);
    mat4 PV_3D = P_3D * V_3D;

    uint32 window_width, window_height; {
        real32 _window_width, _window_height; // FORNOW
        _window_get_size(&_window_width, &_window_height);
        window_width = (uint32) _window_width;
        window_height = (uint32) _window_height;
    }

    { // panes
        eso_begin(globals.Identity, SOUP_LINES, 5.0f, true);
        eso_color(monokai.gray);
        eso_vertex(0.0f,  1.0f);
        eso_vertex(0.0f, -1.0f);
        eso_end();
    }

    { // draw 2D draw 2d draw
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(0, 0, window_width / 2, window_height);
        }

        {
            #if 1
            // TODO: section view
            // mesh_draw(P_2D, V_2D, inverse(get_M_3D_from_2D()));
            #endif
        }

        {
            if (!_global_screen_state.hide_grid) { // grid 2D grid 2d grid
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
                for (uint32 i = 0; i < global_world_state.dxf.entities.length; ++i) {
                    DXFEntity *entity = &global_world_state.dxf.entities.array[i];
                    int32 color = (global_world_state.dxf.entities.array[i].is_selected) ? DXF_COLOR_SELECTION : DXF_COLOR_DONT_OVERRIDE;
                    real32 dx = 0.0f;
                    real32 dy = 0.0f;
                    if ((global_world_state.modes.click_mode == CLICK_MODE_MOVE_DXF_ENTITIES) && (global_world_state.two_click_command.awaiting_second_click)) {
                        if (global_world_state.dxf.entities.array[i].is_selected) {
                            dx = preview_mouse.x - first_click.x;
                            dy = preview_mouse.y - first_click.y;
                            color = DXF_COLOR_WATER_ONLY;
                        }
                    }
                    eso_dxf_entity__SOUP_LINES(entity, color, dx, dy);
                }
                eso_end();
            }
            { // dots
                if (_global_screen_state.show_details) {
                    eso_begin(camera_get_PV(&_global_screen_state.camera_2D), SOUP_POINTS, 8.0f);
                    eso_color(monokai.white);
                    for (DXFEntity *entity = global_world_state.dxf.entities.array; entity < &global_world_state.dxf.entities.array[global_world_state.dxf.entities.length]; ++entity) {
                        real32 start_x, start_y, end_x, end_y;
                        entity_get_start_and_end_points(entity, &start_x, &start_y, &end_x, &end_y);
                        eso_vertex(start_x, start_y);
                        eso_vertex(end_x, end_y);
                    }
                    eso_end();
                }
            }
            { // axes 2D axes 2d axes axis 2D axis 2d axes crosshairs cross hairs origin 2d origin 2D origin
                real32 funky_NDC_factor = _global_screen_state.camera_2D.screen_height_World / 120.0f;
                real32 LL = 1000 * funky_NDC_factor;

                eso_color(monokai.white);
                eso_begin(PV_2D, SOUP_LINES, 3.0f); {
                    // axis
                    vec2 v = LL * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                    eso_vertex(preview_dxf_axis_base_point + v);
                    eso_vertex(preview_dxf_axis_base_point - v);

                    // origin
                    real32 r = funky_NDC_factor;
                    eso_vertex(preview_dxf_origin - V2(r, 0));
                    eso_vertex(preview_dxf_origin + V2(r, 0));
                    eso_vertex(preview_dxf_origin - V2(0, r));
                    eso_vertex(preview_dxf_origin + V2(0, r));
                } eso_end();
            }

            if (global_world_state.two_click_command.awaiting_second_click) {
                if (
                        0
                        || (global_world_state.modes.click_modifier == CLICK_MODIFIER_WINDOW)
                        ||(global_world_state.modes.click_mode == CLICK_MODE_CREATE_BOX )
                   ) {
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(omax.cyan);
                    eso_vertex(first_click.x, first_click.y);
                    eso_vertex(preview_mouse.x, first_click.y);
                    eso_vertex(preview_mouse.x, preview_mouse.y);
                    eso_vertex(first_click.x, preview_mouse.y);
                    eso_end();
                }
                if (global_world_state.modes.click_mode == CLICK_MODE_MEASURE) { // measure line
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(omax.cyan);
                    eso_vertex(first_click);
                    eso_vertex(preview_mouse);
                    eso_end();
                }
                if (global_world_state.modes.click_mode == CLICK_MODE_CREATE_LINE) { // measure line
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(omax.cyan);
                    eso_vertex(first_click);
                    eso_vertex(preview_mouse);
                    eso_end();
                }
                if (global_world_state.modes.click_mode == CLICK_MODE_CREATE_CIRCLE) {
                    vec2 c = { global_world_state.two_click_command.first_click.x, global_world_state.two_click_command.first_click.y };
                    vec2 p = preview_mouse;
                    real32 r = norm(c - p);
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(omax.cyan);
                    for (uint32 i = 0; i < NUM_SEGMENTS_PER_CIRCLE; ++i) eso_vertex(c + r * e_theta(NUM_DEN(i, NUM_SEGMENTS_PER_CIRCLE) * TAU));
                    eso_end();
                }
                if (global_world_state.modes.click_mode == CLICK_MODE_CREATE_FILLET) {
                    // FORNOW
                    int i = dxf_find_closest_entity(&global_world_state.dxf.entities, global_world_state.two_click_command.first_click.x, global_world_state.two_click_command.first_click.y);
                    if (i != -1) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(omax.cyan);
                        eso_dxf_entity__SOUP_LINES(&global_world_state.dxf.entities.array[i], DXF_COLOR_WATER_ONLY);
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

        { // selection 2d selection 2D selection tube tubes slice slices stack stacks wire wireframe wires frame (FORNOW: ew)
            uint32 color = ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD)) ? DXF_COLOR_TRAVERSE : ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT)) ? DXF_COLOR_QUALITY_1 : ((global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN) || (global_world_state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) ? DXF_COLOR_WATER_ONLY : DXF_COLOR_SELECTION;

            uint32 NUM_TUBE_STACKS_INCLUSIVE;
            mat4 M;
            mat4 M_incr;
            {
                mat4 T_o = M4_Translation(preview_dxf_origin);
                mat4 inv_T_o = M4_Translation(-preview_dxf_origin);
                if (extrude) {
                    // NOTE: some repetition with wrapper
                    real32 a = MIN(0.0f, MIN(console_param_1, console_param_2));
                    real32 b = MAX(0.0f, MAX(console_param_1, console_param_2));
                    real32 length = b - a;
                    NUM_TUBE_STACKS_INCLUSIVE = MIN(64, uint32(roundf(length / 2.5f)) + 2);
                    M = M_3D_from_2D * M4_Translation(-global_world_state.dxf.origin.x, -global_world_state.dxf.origin.y, a + Z_FIGHT_EPS);
                    M_incr = M4_Translation(0.0f, 0.0f, (b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                } else if (revolve) {
                    NUM_TUBE_STACKS_INCLUSIVE = 64;
                    M = M_3D_from_2D * inv_T_o;
                    { // M_incr
                        real32 a = 0.0f;
                        real32 b = TAU;
                        mat4 R_a = M4_RotationAbout(V3(e_theta(PI / 2 + preview_dxf_axis_angle_from_y), 0.0f), (b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                        mat4 T_a = M4_Translation(V3(preview_dxf_axis_base_point, 0.0f));
                        mat4 inv_T_a = inverse(T_a);
                        M_incr = T_o * T_a * R_a * inv_T_a * inv_T_o;
                    }
                } else if (global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D;
                    M_incr = M4_Identity();
                } else if (global_world_state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0.0f, 0.0f, console_param_1 + Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else { // default
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0, 0, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                }

                for (uint32 tube_stack_index = 0; tube_stack_index < NUM_TUBE_STACKS_INCLUSIVE; ++tube_stack_index) {
                    eso_begin(PV_3D * M, SOUP_LINES, 5.0f); {
                        for (uint32 i = 0; i < global_world_state.dxf.entities.length; ++i) {
                            DXFEntity *entity = &global_world_state.dxf.entities.array[i];
                            if (global_world_state.dxf.entities.array[i].is_selected) {
                                eso_dxf_entity__SOUP_LINES(entity, color);
                            }
                        }
                    } eso_end();
                    M *= M_incr;
                }
            }
        }

        BoundingBox draw_bounding_box; {
            BoundingBox _selection_bounding_box = dxf_entities_get_bounding_box(&global_world_state.dxf.entities, true);
            if (dxf_anything_selected) {
                draw_bounding_box = _selection_bounding_box;
                real32 eps = 10.0f;
                draw_bounding_box.min[0] -= eps;
                draw_bounding_box.max[0] += eps;
                draw_bounding_box.min[1] -= eps;
                draw_bounding_box.max[1] += eps;
            } else {
                real32 r = 30.0f;
                draw_bounding_box = { -r, -r, r, r };
            }
        }

        #if 0
        if (dxf_anything_selected) { // arrow
            if ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT)) {

                vec3 color = ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD)) ? V3(83.0f / 255, 255.0f / 255, 83.0f / 255.0f) : V3(1.0f, 0.0f, 0.0f);

                real32 arrow_x = 0.0f;
                real32 arrow_y = 0.0f;
                real32 H[2] = { console_param_1, console_param_2 };
                bool32 toggle[2] = { global_world_state.console.flip_flag, !global_world_state.console.flip_flag };
                mat4 A = M_3D_from_2D;
                if ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                    bounding_box_center(_selection_bounding_box, &arrow_x, &arrow_y);
                    if (dxf_anything_selected) {
                        arrow_x -= global_world_state.dxf.origin.x;
                        arrow_y -= global_world_state.dxf.origin.y;
                    }
                } else { ASSERT((global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT));
                    H[0] = 10.0f + _selection_bounding_box.max[(!global_world_state.console.flip_flag) ? 1 : 0];
                    H[1] = 0.0f;
                    toggle[0] = false;
                    toggle[1] = false;
                    { // A
                        A *= M4_RotationAboutXAxis(RAD(-90.0f));
                        if (global_world_state.console.flip_flag) {
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
        #endif

        if (global_world_state.feature_plane.is_active) { // axes 3D axes 3d axes axis 3D axis 3d axis
            real32 r = _global_screen_state.camera_3D.ortho_screen_height_World / 120.0f;
            real32 LL = 100.0f;
            eso_color(monokai.white);
            eso_begin(PV_3D * M_3D_from_2D * M4_Translation(0.0f, 0.0f, Z_FIGHT_EPS), SOUP_LINES, 4.0f);
            eso_vertex(-r, 0.0f);
            eso_vertex( r, 0.0f);
            eso_vertex(0.0f, -r);
            eso_vertex(0.0f,  r);
            // TODO: clip this on the feature_plane
            vec2 v = LL * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
            vec2 a = preview_dxf_axis_base_point + v;
            vec2 b = preview_dxf_axis_base_point - v;
            // vec2 a = bounding_box_clamp(preview_dxf_axis_base_point + v, draw_bounding_box);
            eso_vertex(-preview_dxf_origin + a);
            eso_vertex(-preview_dxf_origin + b); // FORNOW
            eso_end();
        }

        mesh_draw(P_3D, V_3D, M4_Identity());

        if (!_global_screen_state.hide_grid) { // grid 3D grid 3d grid
            conversation_draw_3D_grid_box(P_3D, V_3D);
        }

        { // floating sketch plane; selection plane NOTE: transparent
            bool draw = true;
            mat4 PVM = PV_3D * M_3D_from_2D;
            vec3 color = monokai.yellow;
            real32 sign = -1.0f;
            if (global_world_state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                PVM *= M4_Translation(-global_world_state.dxf.origin.x, -global_world_state.dxf.origin.y, console_param_1);
                color = { 0.0f, 1.0f, 1.0f };
                sign = 1.0f;
                draw = true;
            } else if (global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN) {
                color = { 0.0f, 1.0f, 1.0f };
                sign = 1.0f;
                draw = true;
            } else {
                if (dxf_anything_selected) PVM *= M4_Translation(-global_world_state.dxf.origin.x, -global_world_state.dxf.origin.y, 0.0f); // FORNOW
            }
            if (draw) {
                eso_begin(PVM, SOUP_QUADS);
                eso_color(color, 0.35f);
                eso_vertex(draw_bounding_box.min[0], draw_bounding_box.min[1], sign * Z_FIGHT_EPS);
                eso_vertex(draw_bounding_box.min[0], draw_bounding_box.max[1], sign * Z_FIGHT_EPS);
                eso_vertex(draw_bounding_box.max[0], draw_bounding_box.max[1], sign * Z_FIGHT_EPS);
                eso_vertex(draw_bounding_box.max[0], draw_bounding_box.min[1], sign * Z_FIGHT_EPS);
                eso_end();
            }
        }

        glDisable(GL_SCISSOR_TEST);
    }

    if (!_global_screen_state.hide_gui) { // gui
        {
            char click_message[256] = {};
            gui_printf("[Click] %s %s %s",
                    (global_world_state.modes.click_mode == CLICK_MODE_NONE) ? "NONE" :
                    (global_world_state.modes.click_mode == CLICK_MODE_SET_ORIGIN) ? "SET_ORIGIN" :
                    (global_world_state.modes.click_mode == CLICK_MODE_SET_AXIS) ? "SET_AXIS" :
                    (global_world_state.modes.click_mode == CLICK_MODE_SELECT) ? "SELECT" :
                    (global_world_state.modes.click_mode == CLICK_MODE_DESELECT) ? "DESELECT" :
                    (global_world_state.modes.click_mode == CLICK_MODE_MEASURE) ? "MEASURE" :
                    (global_world_state.modes.click_mode == CLICK_MODE_CREATE_LINE) ? "CREATE_LINE" :
                    (global_world_state.modes.click_mode == CLICK_MODE_CREATE_BOX) ? "CREATE_BOX" :
                    (global_world_state.modes.click_mode == CLICK_MODE_CREATE_CIRCLE) ? "CREATE_CIRCLE" :
                    (global_world_state.modes.click_mode == CLICK_MODE_CREATE_FILLET) ? "CREATE_FILLET" :
                    (global_world_state.modes.click_mode == CLICK_MODE_MOVE_DXF_ENTITIES) ? "MOVE_DXF_TO" :
                    "???",
                    (global_world_state.modes.click_modifier == CLICK_MODE_NONE) ? "" :
                    (global_world_state.modes.click_modifier == CLICK_MODIFIER_CONNECTED) ? "CONNECTED" :
                    (global_world_state.modes.click_modifier == CLICK_MODIFIER_WINDOW) ? "WINDOW" :
                    (global_world_state.modes.click_modifier == CLICK_MODIFIER_SNAP_TO_CENTER_OF) ? "CENTER_OF" :
                    (global_world_state.modes.click_modifier == CLICK_MODIFIER_SNAP_TO_END_OF) ? "END_OF" :
                    (global_world_state.modes.click_modifier == CLICK_MODIFIER_SNAP_TO_MIDDLE_OF) ? "MIDDLE_OF" :
                    (global_world_state.modes.click_modifier == CLICK_MODIFIER_QUALITY) ? "QUALITY" :
                    "???",
                    click_message);
        }

        {
            char enter_message[256] = {};
            if ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) || (global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN)) {
                real32 p, p2;
                char glyph, glyph2;
                if ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                    p      =  console_param_1;
                    p2     = -console_param_2;
                    if (global_world_state.console.flip_flag) { // ??
                        p *= -1;
                        p2 *= -1;
                    }
                    if (IS_ZERO(p)) p = 0.0f; // FORNOW makes minus sign go away in hud (not a big deal)
                    if (IS_ZERO(p2)) p2 = 0.0f; // FORNOW makes minus sign go away in hud (not a big deal)
                    glyph  = (!global_world_state.console.flip_flag) ? '^' : 'v';
                    glyph2 = (!global_world_state.console.flip_flag) ? 'v' : '^';
                } else {
                    ASSERT(global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN);
                    p      = console_param_1;
                    p2     = console_param_2;
                    glyph  = 'x';
                    glyph2 = 'y';
                }
                sprintf(enter_message, "%c:%gmm %c:%gmm", glyph, p, glyph2, p2);
                if (((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) && IS_ZERO(console_param_2)) sprintf(enter_message, "%c:%gmm", glyph, p);
            } else if ((global_world_state.modes.enter_mode == ENTER_MODE_OPEN) || (global_world_state.modes.enter_mode == ENTER_MODE_SAVE)) {
                sprintf(enter_message, "%s%s", _global_screen_state.drop_path, global_world_state.console.buffer);
            }

            gui_printf("[Enter] %s %s",
                    (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) ? "EXTRUDE_ADD" :
                    (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) ? "EXTRUDE_CUT" :
                    (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) ? "REVOLVE_ADD" :
                    (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT) ? "REVOLVE_CUT" :
                    (global_world_state.modes.enter_mode == ENTER_MODE_OPEN) ? "OPEN" :
                    (global_world_state.modes.enter_mode == ENTER_MODE_SAVE) ? "SAVE" :
                    (global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN) ? "SET_ORIGIN" :
                    (global_world_state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY) ? "OFFSET_PLANE_TO" :
                    (global_world_state.modes.enter_mode == ENTER_MODE_NONE) ? "NONE" :
                    "???",
                    enter_message);
        }

        if ((global_world_state.modes.enter_mode == ENTER_MODE_NONE) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT)) {
            gui_printf("> %s", global_world_state.console.buffer);
        } else {
            gui_printf("> %s", global_world_state.console.buffer);
        }

        conversation_message_buffer_update_and_draw();

        if (_global_screen_state.show_details) {
            gui_printf("");
            gui_printf("%d dxf elements   %d stl triangles", global_world_state.dxf.entities.length, global_world_state.mesh.num_triangles);
        }
        if (_global_screen_state.show_help) {
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
            gui_printf("(Escape)-from-current-enter_and_modes.click_modes");
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
            gui_printf("EXPERIMENTAL: (Ctrl + n)ew-session");
            gui_printf("");
            gui_printf("you can drag and drop *.dxf and *.stl into Conversation");
        }
    }
}


//////////////////////////////////////////////////
// MAIN() ////////////////////////////////////////
//////////////////////////////////////////////////

// TODO: load is no longer special; we should be able to undo / redo across it just like anything else
// TODO: don't worry about user experience yet (it will grow to include panels toolbox etc)

void _spoof_KEY_event(uint32 key, bool32 super = false, bool32 shift = false) {
    queue_enqueue(&queue_of_fresh_events_from_user, KEY_event(key, super, shift));
}

void _spoof_MOUSE_2D_event(real32 mouse_x, real32 mouse_y) {
    queue_enqueue(&queue_of_fresh_events_from_user, MOUSE_2D_event(mouse_x, mouse_y));
}

void _spoof_MOUSE_3D_event(real32 o_x, real32 o_y, real32 o_z, real32 dir_x, real32 dir_y, real32 dir_z) {
    queue_enqueue(&queue_of_fresh_events_from_user, MOUSE_3D_event(o_x, o_y, o_z, dir_x, dir_y, dir_z));
}

void _spoof_string(char *string) {
    uint32 n = (uint32) strlen(string);
    for (uint32 i = 0; i < n; ++i) {
        char key = string[i];
        if (('a' <= key) && (key <= 'z')) key = 'A' + (key - 'a');
        _spoof_KEY_event(key);
    }
}

void conversation_init__NOTE_use_spoof_api_in_here() {
    if (1) {
        if (0) {
            _spoof_KEY_event('O', true);
            _spoof_string("splash.dxf");
            _spoof_KEY_event(GLFW_KEY_ENTER);
            if (1) {
                _spoof_KEY_event('Y');
                _spoof_KEY_event('S');
                _spoof_KEY_event('C');
                _spoof_MOUSE_2D_event( 20.0,  20.0);
                _spoof_MOUSE_2D_event( 16.0,  16.0);
                _spoof_MOUSE_2D_event( 16.0, -16.0);
                _spoof_MOUSE_2D_event(-16.0, -16.0);
                _spoof_MOUSE_2D_event(-16.0,  16.0);
                _spoof_KEY_event('E');
                _spoof_KEY_event('5');
                _spoof_KEY_event('0');
                _spoof_KEY_event(GLFW_KEY_ENTER); // FORNOW
                _spoof_MOUSE_3D_event(50.0f, 93.0f, 76.0f, -0.47f, -0.43f, -0.77f);
                _spoof_MOUSE_2D_event(16.4f, -9.5f);
                _spoof_KEY_event('E', false, true);
                _spoof_KEY_event('4');
                _spoof_KEY_event('7');
                _spoof_KEY_event(GLFW_KEY_ENTER); // FORNOW
                _spoof_KEY_event('C');
                _spoof_MOUSE_2D_event(16.0f, -16.0f);
                _spoof_MOUSE_2D_event(32.0f, -16.0f);
                _spoof_KEY_event('S');
                _spoof_KEY_event('C');
                _spoof_MOUSE_2D_event(32.0f, -16.0f);
                _spoof_MOUSE_3D_event(74.0f, 132.0f, 113.0f, -0.4f, -0.6f, -0.7f);
                _spoof_KEY_event('E', false, true);
                _spoof_KEY_event('6');
                _spoof_KEY_event('0');
                _spoof_KEY_event(GLFW_KEY_ENTER);
                _spoof_KEY_event('O', true);
                _spoof_string("omax.dxf");
                _spoof_KEY_event(GLFW_KEY_ENTER);
                _spoof_KEY_event('S');
                _spoof_KEY_event('Q');
                _spoof_KEY_event('1');
                _spoof_KEY_event('Y');
                _spoof_KEY_event('E');
                _spoof_KEY_event('3');
                _spoof_KEY_event(GLFW_KEY_ENTER);
                _spoof_KEY_event('S');
                _spoof_KEY_event('Q');
                _spoof_KEY_event('0');
                _spoof_KEY_event(GLFW_KEY_DELETE);
                _spoof_KEY_event(GLFW_KEY_ESCAPE);
                _spoof_KEY_event('C');
                _spoof_KEY_event('Z');
                _spoof_KEY_event('0');
                _spoof_KEY_event('1');
                _spoof_KEY_event('2');
                _spoof_KEY_event('3');
                _spoof_KEY_event('4');
                _spoof_KEY_event('5');
                _spoof_KEY_event('6');
                _spoof_KEY_event('7');
                _spoof_KEY_event('8');
                _spoof_KEY_event('9');


                //spoof_KEY_event('U');
                //spoof_KEY_event('U');
                //spoof_KEY_event('U');
                //spoof_KEY_event('U');
                //spoof_KEY_event('U', false, true);
                //spoof_KEY_event('U', false, true);
                //spoof_KEY_event('U', false, true);
                //spoof_KEY_event('U', false, true);
            }
        } else {
            _spoof_KEY_event('O', true);
            _spoof_string("fidget.dxf");
            _spoof_KEY_event(GLFW_KEY_ENTER);
            _spoof_KEY_event('L');
            _spoof_KEY_event('Z');
        }
        #if 0 // TODO: update to use spoof api
        else if (1) {
            conversation_dxf_load("omax.dxf");
            if (1) {
                UserEvent event = {};
                event.type = USER_EVENT_TYPE_KEY_PRESS;
                for (int i = 0; i < 5; ++i) {
                    spoof_KEY_event('Y');
                    spoof_KEY_event('S');
                    spoof_KEY_event('Q');
                    spoof_KEY_event('0' + i);
                    spoof_KEY_event('E');
                    spoof_KEY_event('1' + i);
                    spoof_KEY_event(GLFW_KEY_ENTER);
                }
            }
        } else if (0) {
            conversation_dxf_load("ik.dxf");
        } else if (0) {
            conversation_dxf_load("debug.dxf");
        }
        #endif
    }
    init_cameras(); // FORNOW
    conversation_messagef("type h for help `// pre-alpha " __DATE__ " " __TIME__);
}



int main() {
    // _window_set_size(1.5 * 640.0, 1.5 * 360.0); // TODO the first frame crap gets buggered

    conversation_init__NOTE_use_spoof_api_in_here();

    {
        // FORNOW: first frame position
        double xpos, ypos;
        glfwGetCursorPos(COW0._window_glfw_window, &xpos, &ypos);
        callback_cursor_position(NULL, xpos, ypos);
    }

    while (cow_begin_frame()) {
        // Sleep(100);

        { // camera_move, hot_pane, conversation_draw
            { // camera_move (using shimmed globals.* global_world_state)
                if (_global_screen_state.hot_pane == HOT_PANE_2D) {
                    camera_move(&_global_screen_state.camera_2D);
                } else if (_global_screen_state.hot_pane == HOT_PANE_3D) {
                    camera_move(&_global_screen_state.camera_3D);
                }
            }

            if ((!globals.mouse_left_held && !globals.mouse_right_held) || globals.mouse_left_pressed || globals.mouse_right_pressed) {
                _global_screen_state.hot_pane = (callback_xpos <= window_get_width() / 2) ? HOT_PANE_2D : HOT_PANE_3D;
                if ((global_world_state.modes.click_modifier == CLICK_MODIFIER_WINDOW) && (global_world_state.two_click_command.awaiting_second_click)) _global_screen_state.hot_pane = HOT_PANE_2D;// FORNOW
            }

            conversation_draw();
        }

        _global_screen_state.popup_blinker_time += 0.0167f;

        { // queue_of_fresh_events_from_user
            warn_once("FORNOW if not while (only one event per frame)--cause FORNOW drawing popup gui in fresh_event_from_user_process\n");
            // TODO: upgrade to handle multiple events per frame while only drawing gui once
            {
                if (queue_of_fresh_events_from_user.length) {
                    while (queue_of_fresh_events_from_user.length) {
                        UserEvent fresh_event = queue_dequeue(&queue_of_fresh_events_from_user);
                        fresh_event_from_user_process(fresh_event);
                    }
                } else {
                    // NOTE: this is so we draw the popups
                    UserEvent null_event = {};
                    fresh_event_from_user_process(null_event);
                }
            }
        }

        if (_global_screen_state.show_event_stack) history_debug_draw();

    }
}


