// TODO: make good map implementation
// TODO: don't store FancyMesh as a bunch of alloc'd pointers

#include "cs345.cpp"
#include "manifoldc.h"
#include "poe.cpp"
#undef real // ??
#define u32 DO_NOT_USE_u32_USE_uint32_INSTEAD
#include "burkardt.cpp"
#include "conversation.h"


//////////////////////////////////////////////////
// GLOBAL STATE //////////////////////////////////
//////////////////////////////////////////////////

WorldState state;
ScreenState ui;


//////////////////////////////////////////////////
// NON-ZERO INITIALIZERS /////////////////////////
//////////////////////////////////////////////////

void init_cameras() {
    ui.camera_2D = { 100.0f, 0.0, 0.0f, -0.5f, -0.125f };
    camera2D_zoom_to_bounding_box(&ui.camera_2D, dxf_entities_get_bounding_box(&state.dxf.entities));
    ui.camera_3D = { 2.0f * ui.camera_2D.screen_height_World, CAMERA_3D_DEFAULT_ANGLE_OF_VIEW, RAD(33.0f), RAD(-44.0f), 0.0f, 0.0f, 0.5f, -0.125f };
}


//////////////////////////////////////////////////
// GETTERS (STATE NOT WORTH TROUBLE OF STORING) //
//////////////////////////////////////////////////

mat4 get_M_3D_from_2D() {
    vec3 up = { 0.0f, 1.0f, 0.0f };
    real32 dot_product = dot(state.feature_plane.normal, up);
    vec3 y = (ARE_EQUAL(ABS(dot_product), 1.0f)) ? V3(0.0f,  0.0f, -1.0f * SGN(dot_product)) : up;
    vec3 x = normalized(cross(y, state.feature_plane.normal));
    vec3 z = cross(x, y);

    // FORNOW
    if (ARE_EQUAL(ABS(dot_product), 1.0f) && SGN(dot_product) < 0.0f) {
        y *= -1;
    }

    return M4_xyzo(x, y, z, (state.feature_plane.signed_distance_to_world_origin) * state.feature_plane.normal);
}

void get_console_params(real32 *console_param_1, real32 *console_param_2) {
    char buffs[2][64] = {};
    uint32 buff_i = 0;
    uint32 i = 0;
    for (char *c = state.console.buffer; (*c) != '\0'; ++c) {
        if (*c == ',') {
            if (++buff_i == 2) break;
            i = 0;
            continue;
        }
        buffs[buff_i][i++] = (*c);
    }
    real32 sign = 1.0f;
    if ((state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
        sign = (!state.console.flip_flag) ? 1.0f : -1.0f;
    }
    *console_param_1 = sign * strtof(buffs[0], NULL);
    *console_param_2 = sign * strtof(buffs[1], NULL);
    if ((state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
        *console_param_2 *= -1;
    }
}


//////////////////////////////////////////////////
// STATE-DEPENDENT UTILITY FUNCTIONS /////////////
//////////////////////////////////////////////////

void snap_map(real32 before_x, real32 before_y, real32 *after_x, real32 *after_y) {
    real32 tmp_x = before_x;
    real32 tmp_y = before_y;
    {
        if (state.modes.click_modifier == CLICK_MODIFIER_SNAP_TO_CENTER_OF) {
            real32 min_squared_distance = HUGE_VAL;
            for (DXFEntity *entity = state.dxf.entities.array; entity < &state.dxf.entities.array[state.dxf.entities.length]; ++entity) {
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
        } else if (state.modes.click_modifier == CLICK_MODIFIER_SNAP_TO_MIDDLE_OF) {
            real32 min_squared_distance = HUGE_VAL;
            real32 middle_x;
            real32 middle_y;
            for (DXFEntity *entity = state.dxf.entities.array; entity < &state.dxf.entities.array[state.dxf.entities.length]; ++entity) {
                real32 squared_distance = squared_distance_point_dxf_entity(before_x, before_y, entity);
                if (squared_distance < min_squared_distance) {
                    min_squared_distance = squared_distance;
                    entity_get_middle(entity, &middle_x, &middle_y);
                    tmp_x = middle_x;
                    tmp_y = middle_y;
                }
            }
        } else if (state.modes.click_modifier == CLICK_MODIFIER_SNAP_TO_END_OF) {
            real32 min_squared_distance = HUGE_VAL;
            for (DXFEntity *entity = state.dxf.entities.array; entity < &state.dxf.entities.array[state.dxf.entities.length]; ++entity) {
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


//////////////////////////////////////////////////
// LOADING AND SAVING STATE TO DISK //////////////
//////////////////////////////////////////////////

void conversation_dxf_load(char *filename, bool preserve_cameras_and_dxf_origin = false) {
    if (!poe_file_exists(filename)) {
        conversation_messagef("[load] \"%s\" not found", filename);
        return;
    }

    list_free_AND_zero(&state.dxf.entities);
    list_free_AND_zero(&state.dxf.is_selected);

    dxf_entities_load(filename, &state.dxf.entities);
    list_calloc_NOT_reserve(&state.dxf.is_selected, state.dxf.entities.length, sizeof(bool32));

    if (!preserve_cameras_and_dxf_origin) {
        camera2D_zoom_to_bounding_box(&ui.camera_2D, dxf_entities_get_bounding_box(&state.dxf.entities));
        state.dxf.feature_reference_point.x = 0.0f;
        state.dxf.feature_reference_point.y = 0.0f;
    }

    strcpy(ui.dxf_filename_for_reload, filename);

    conversation_messagef("[load] loaded %s", filename);
}

void conversation_stl_load(char *filename, bool preserve_cameras = false) {
    if (!poe_file_exists(filename)) {
        conversation_messagef("[load] \"%s\" not found", filename);
        return;
    }
    {
        stl_load(filename, &state.mesh);
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
    if (fancy_mesh_save_stl(&state.mesh, filename) ) {
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
                ui.drop_path[i] = filename[i];
                if (filename[i] == '.') {
                    while (
                            (i != 0) &&
                            (ui.drop_path[i - 1] != '\\') &&
                            (ui.drop_path[i - 1] != '/')
                          ) --i;
                    ui.drop_path[i] = '\0';
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
    // _callback_key(NULL, key, 0, action, mods); // FORNOW TODO TODO TODO SHIM
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_LEFT_CONTROL) return;
        if (key == GLFW_KEY_RIGHT_CONTROL) return;
        if (key == GLFW_KEY_LEFT_SUPER) return;
        if (key == GLFW_KEY_RIGHT_SUPER) return;
        UserEvent event = {};
        event.type = UI_EVENT_TYPE_KEY_PRESS;
        event.key = key;
        event.super = (mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER));
        event.shift = (mods & GLFW_MOD_SHIFT);
        queue_enqueue(&queue_of_fresh_events_from_user, event);
    }
}

void callback_cursor_position(GLFWwindow *, double xpos, double ypos) {
    _callback_cursor_position(NULL, xpos, ypos); // FORNOW TODO TODO TODO SHIM

    callback_xpos = (real32) (xpos * COW0._window_macbook_retina_scale_ONLY_USED_FOR_FIXING_CURSOR_POS);
    callback_ypos = (real32) (ypos * COW0._window_macbook_retina_scale_ONLY_USED_FOR_FIXING_CURSOR_POS);

    // // mouse held generates mouse presses
    // FORNOW repeated from callback_mouse_button
    if ((ui.hot_pane == HOT_PANE_2D) && (callback_mouse_left_held) && (((state.modes.click_mode == CLICK_MODE_SELECT) || (state.modes.click_mode == CLICK_MODE_DESELECT)) && (state.modes.click_modifier != CLICK_MODIFIER_WINDOW))) {
        vec2 mouse_s_NDC = transformPoint(_window_get_NDC_from_Screen(), V2(callback_xpos, callback_ypos));
        UserEvent event = {};
        event.type = UI_EVENT_TYPE_MOUSE_2D_PRESS;
        {
            vec2 s_2D = transformPoint(inverse(camera_get_PV(&ui.camera_2D)), mouse_s_NDC);
            snap_map(s_2D.x, s_2D.y, &event.mouse_x, &event.mouse_y);
        }
        event.checkpoint_ineligible = true;
        queue_enqueue(&queue_of_fresh_events_from_user, event);
    }
}

void callback_mouse_button(GLFWwindow *, int button, int action, int) {
    _callback_mouse_button(NULL, button, action, 0); // FORNOW TODO TODO TODO SHIM

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) { 
            callback_mouse_left_held = true;

            vec2 mouse_s_NDC = transformPoint(_window_get_NDC_from_Screen(), V2(callback_xpos, callback_ypos));
            if (ui.hot_pane == HOT_PANE_2D) {
                UserEvent event = {};
                event.type = UI_EVENT_TYPE_MOUSE_2D_PRESS;
                {
                    vec2 s_2D = transformPoint(inverse(camera_get_PV(&ui.camera_2D)), mouse_s_NDC);
                    snap_map(s_2D.x, s_2D.y, &event.mouse_x, &event.mouse_y);
                }
                queue_enqueue(&queue_of_fresh_events_from_user, event);
            } else if (ui.hot_pane == HOT_PANE_3D) {
                UserEvent event = {};
                event.type = UI_EVENT_TYPE_MOUSE_3D_PRESS;
                {
                    mat4 inverse_PV_3D = inverse(camera_get_PV(&ui.camera_3D));
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

void spoof_KEY_event(uint32 key, bool32 super = false, bool32 shift = false) {
    UserEvent event = {};
    event.type = UI_EVENT_TYPE_KEY_PRESS;
    event.key = key;
    event.super = super;
    event.shift = shift;
    queue_enqueue(&queue_of_fresh_events_from_user, event);
}

void spoof_MOUSE_2D_event(real32 mouse_x, real32 mouse_y) {
    UserEvent event = {};
    event.type = UI_EVENT_TYPE_MOUSE_2D_PRESS;
    event.mouse_x = mouse_x;
    event.mouse_y = mouse_y;
    queue_enqueue(&queue_of_fresh_events_from_user, event);
}

void spoof_MOUSE_3D_event(real32 o_x, real32 o_y, real32 o_z, real32 dir_x, real32 dir_y, real32 dir_z) {
    UserEvent event = {};
    event.type = UI_EVENT_TYPE_MOUSE_3D_PRESS;
    event.o = { o_x, o_y, o_z };
    event.dir = { dir_x, dir_y, dir_z };
    queue_enqueue(&queue_of_fresh_events_from_user, event);
}


//////////////////////////////////////////////////
// PROCESSING A SINGLE (NOT-special) EVENT  //////
// (special events include undo, redo, ...) //////
//////////////////////////////////////////////////

uint32 standard_event_process(UserEvent event, bool32 skip_mesh_generation_because_we_are_loading_from_the_redo_stack = false) {
    // // grug variables 
    bool32 extrude = ((state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT));
    bool32 revolve = ((state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) || (state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT));
    bool32 cut = ((state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) || (state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT));
    // // computed variables
    bool32 dxf_anything_selected; {
        dxf_anything_selected = false;
        for (uint32 i = 0; i < state.dxf.entities.length; ++i) {
            if (state.dxf.is_selected.array[i]) {
                dxf_anything_selected = true;
                break;
            }
        }
    }
    real32 console_param_1, console_param_2; {
        get_console_params(&console_param_1, &console_param_2);
    }


    bool32 result = PROCESSED_EVENT_CATEGORY_RECORD; {
        auto push_back_dxf_entity_and_mask_bool_lambda = [&](DXFEntity entity) {
            list_push_back(&state.dxf.entities, entity);
            list_push_back(&state.dxf.is_selected, (bool32) false);
        };
        auto delete_dxf_entity_and_mask_bool_lambda_NOTE_if_iterating_must_be_going_backwards = [&](uint32 i) {
            list_delete_at(&state.dxf.entities, i);
            list_delete_at(&state.dxf.is_selected, i);
        };

        if ((state.modes.enter_mode == ENTER_MODE_OPEN) || (state.modes.enter_mode == ENTER_MODE_SAVE)) {
            result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
        }


        if (event.type == UI_EVENT_TYPE_KEY_PRESS) {
            auto key_lambda = [event](uint32 key, bool super = false, bool shift = false) -> bool {
                if (event.type != UI_EVENT_TYPE_KEY_PRESS) return false; // TODO: ASSERT
                return _key_lambda(event, key, super, shift);
            };

            bool32 click_mode_SNAP_ELIGIBLE_ =
                0
                || (state.modes.click_mode == CLICK_MODE_ORIGIN_MOVE)
                || (state.modes.click_mode == CLICK_MODE_MEASURE)
                || (state.modes.click_mode == CLICK_MODE_CREATE_LINE)
                || (state.modes.click_mode == CLICK_MODE_CREATE_BOX)
                || (state.modes.click_mode == CLICK_MODE_CREATE_CIRCLE)
                || (state.modes.click_mode == CLICK_MODE_DXF_MOVE)
                ;

            bool32 click_mode_NEEDS_CONSOLE = 0
                || (state.modes.click_mode == CLICK_MODE_CREATE_FILLET);

            // TODO: _modes.enter_mode_NEEDS_CONSOLE
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
                    if (state.modes.click_modifier == CLICK_MODIFIER_QUALITY) {
                        for (uint32 color = 0; color <= 9; ++color) {
                            if (key_lambda('0' + color)) {
                                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                                key_eaten_by_special__NOTE_dealt_with_up_top = true;
                                for (uint32 i = 0; i < state.dxf.entities.length; ++i) {
                                    if (state.dxf.entities.array[i].color == color) {
                                        bool32 value_to_write_to_selection_mask = (state.modes.click_mode == CLICK_MODE_SELECT);
                                        state.dxf.is_selected.array[i] = value_to_write_to_selection_mask;
                                    }
                                }
                                state.modes.click_modifier = CLICK_MODIFIER_NONE;
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
                                || (state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD)
                                || (state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)
                                || (state.modes.enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO)
                                || (state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY)
                                || (state.modes.click_mode == CLICK_MODE_CREATE_FILLET)
                           ) {
                            send_key_to_console |= key_lambda(COW_KEY_BACKSPACE);
                            send_key_to_console |= key_lambda('.');
                            send_key_to_console |= key_lambda('-');
                            for (uint32 i = 0; i < 10; ++i) send_key_to_console |= key_lambda('0' + i);
                            if ((state.modes.enter_mode != ENTER_MODE_OFFSET_PLANE_BY)) {
                                send_key_to_console |= key_lambda(',');
                            }
                        } else if ((state.modes.enter_mode == ENTER_MODE_OPEN) || (state.modes.enter_mode == ENTER_MODE_SAVE)) {
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
                            if (state.modes.enter_mode == ENTER_MODE_NONE) {
                                conversation_messagef("[enter] enter mode is none");
                                send_key_to_console = false;
                            } else if (extrude || revolve) {
                                if (!dxf_anything_selected) {
                                    conversation_messagef("[enter] no state.dxf.entities elements is_selected");
                                    send_key_to_console = false;
                                } else if (!state.feature_plane.is_active) {
                                    conversation_messagef("[enter] no plane is_selected");
                                    send_key_to_console = false;
                                } else if (cut && (state.mesh.num_triangles == 0)) { // FORNOW
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
                            } else if (state.modes.enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO) {
                                ;
                            } else if (state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
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
                            if (state.console.num_bytes_written != 0) state.console.buffer[--state.console.num_bytes_written] = '\0';
                        } else {
                            state.console.buffer[state.console.num_bytes_written++] = character_equivalent;
                        }
                    } else { // wrapper
                        if (extrude || revolve) {
                            result = PROCESSED_EVENT_CATEGORY_SUPER_CHECKPOINT;

                            if (!skip_mesh_generation_because_we_are_loading_from_the_redo_stack) {
                                FancyMesh resulting_fancy_mesh; {
                                    CrossSectionEvenOdd cross_section = cross_section_create_FORNOW_QUADRATIC(&state.dxf.entities, state.dxf.is_selected.array);
                                    resulting_fancy_mesh = wrapper_manifold(
                                            &state.mesh,
                                            cross_section.num_polygonal_loops,
                                            cross_section.num_vertices_in_polygonal_loops,
                                            cross_section.polygonal_loops,
                                            get_M_3D_from_2D(),
                                            state.modes.enter_mode,
                                            console_param_1,
                                            console_param_2,
                                            state.dxf.feature_reference_point.x,
                                            state.dxf.feature_reference_point.y,
                                            state.console.flip_flag // FORNOW:revolve_use_x_instead
                                            );
                                    cross_section_free(&cross_section);
                                }

                                fancy_mesh_free(&state.mesh);
                                state.mesh = resulting_fancy_mesh;
                            }

                            { // reset some stuff
                                state.feature_plane = {};
                                list_memset(&state.dxf.is_selected, 0, state.dxf.entities.length * sizeof(bool32));
                            }

                            if (state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) conversation_messagef("[extrude-add] success");
                            if (state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) conversation_messagef("[extrude-cut] success");
                            if (state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) conversation_messagef("[revolve-add] success");
                            if (state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT) conversation_messagef("[revolve-cut] success");
                            if (revolve) conversation_messagef("[revolve] success");
                        } else { ASSERT((state.modes.enter_mode == ENTER_MODE_OPEN) || (state.modes.enter_mode == ENTER_MODE_SAVE));
                            static char full_filename_including_path[512];
                            sprintf(full_filename_including_path, "%s%s", ui.drop_path, state.console.buffer);
                            if (state.modes.enter_mode == ENTER_MODE_OPEN) {
                                result = PROCESSED_EVENT_CATEGORY_KILL_HISTORY;
                                if (poe_suffix_match(full_filename_including_path, ".dxf")) {
                                    conversation_dxf_load(full_filename_including_path, (strcmp(full_filename_including_path, ui.dxf_filename_for_reload) == 0));
                                } else if (poe_suffix_match(full_filename_including_path, ".stl")) {
                                    conversation_stl_load(full_filename_including_path);
                                } else {
                                    conversation_messagef("[open] %s not found", full_filename_including_path);
                                }
                            } else { ASSERT(state.modes.enter_mode == ENTER_MODE_SAVE);
                                conversation_save(full_filename_including_path);
                            }
                        }
                        state.modes.enter_mode = {};
                        state.console = {};
                    }
                } else if (key_lambda('Q', true)) {
                    exit(1);
                } else if (key_lambda('0')) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    ui.camera_3D.angle_of_view = CAMERA_3D_DEFAULT_ANGLE_OF_VIEW - ui.camera_3D.angle_of_view;
                } else if (key_lambda('X', false, true)) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    camera2D_zoom_to_bounding_box(&ui.camera_2D, dxf_entities_get_bounding_box(&state.dxf.entities));
                } else if (key_lambda('G')) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    ui.hide_grid = !ui.hide_grid;
                } else if (key_lambda('H')) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    ui.show_help = !ui.show_help;
                } else if (key_lambda('.')) { 
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    ui.show_details = !ui.show_details;
                } else if (key_lambda('K')) { 
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    ui.show_command_stack = !ui.show_command_stack;
                } else if (key_lambda('K', false, true)) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    ui.hide_gui = !ui.hide_gui;
                } else if (key_lambda('O', true)) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    state.modes.enter_mode = ENTER_MODE_OPEN;
                } else if (key_lambda('S', true)) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    state.modes.enter_mode = ENTER_MODE_SAVE;
                } else if (key_lambda('R', true, true)) {
                    result = PROCESSED_EVENT_CATEGORY_KILL_HISTORY;
                    conversation_dxf_load(ui.dxf_filename_for_reload, true);
                } else if (key_lambda(COW_KEY_ESCAPE)) {
                    state.modes = {};
                    state.console = {};
                    state.feature_plane = {};
                } else if (key_lambda(COW_KEY_TAB)) {
                    result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                    state.console.flip_flag = !state.console.flip_flag;
                } else if (key_lambda('N')) {
                    if (state.feature_plane.is_active) {
                        state.modes.enter_mode = ENTER_MODE_OFFSET_PLANE_BY;
                        state.console = {};
                    } else {
                        conversation_messagef("[n] no plane is_selected");
                    }
                } else if (key_lambda('Z', false, true)) {
                    state.modes = {};
                    state.modes.enter_mode = ENTER_MODE_MOVE_DXF_ORIGIN_TO;
                    state.console = {};
                } else if (key_lambda('S')) {
                    state.modes.click_mode = CLICK_MODE_SELECT;
                    state.modes.click_modifier = CLICK_MODIFIER_NONE;
                } else if (key_lambda('D')) {
                    state.modes.click_mode = CLICK_MODE_DESELECT;
                    state.modes.click_modifier = CLICK_MODIFIER_NONE;
                } else if (key_lambda('C')) {
                    if (((state.modes.click_mode == CLICK_MODE_SELECT) || (state.modes.click_mode == CLICK_MODE_DESELECT)) && (state.modes.click_modifier != CLICK_MODIFIER_CONNECTED)) {
                        state.modes.click_modifier = CLICK_MODIFIER_CONNECTED;
                    } else if (click_mode_SNAP_ELIGIBLE_) {
                        result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                        state.modes.click_modifier = CLICK_MODIFIER_SNAP_TO_CENTER_OF;
                    } else {
                        result = PROCESSED_EVENT_CATEGORY_RECORD;
                        state.modes.click_mode = CLICK_MODE_CREATE_CIRCLE;
                        state.modes.click_modifier = CLICK_MODIFIER_NONE;
                        state.two_click_command.awaiting_second_click = false;
                        ui.space_bar_event_key = 'C'; // FORNOW; TODO perhaps key_lambda can store the entirety of the event
                                                      // update_space_bar_event_off_of_most_recent_key_lambda
                    }
                } else if (key_lambda('Q')) {
                    if ((state.modes.click_mode == CLICK_MODE_SELECT) || (state.modes.click_mode == CLICK_MODE_DESELECT)) {
                        state.modes.click_modifier = CLICK_MODIFIER_QUALITY;
                    }
                } else if (key_lambda('A')) {
                    if ((state.modes.click_mode == CLICK_MODE_SELECT) || (state.modes.click_mode == CLICK_MODE_DESELECT)) {
                        result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                        bool32 value_to_write_to_selection_mask = (state.modes.click_mode == CLICK_MODE_SELECT);
                        for (uint32 i = 0; i < state.dxf.entities.length; ++i) state.dxf.is_selected.array[i] = value_to_write_to_selection_mask;
                    }
                } else if (key_lambda('Y')) {
                    result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;

                    // already one of the three primary planes
                    if ((state.feature_plane.is_active) && ARE_EQUAL(state.feature_plane.signed_distance_to_world_origin, 0.0f) && ARE_EQUAL(squaredNorm(state.feature_plane.normal), 1.0f) && ARE_EQUAL(max(state.feature_plane.normal), 1.0f)) {
                        state.feature_plane.normal = { state.feature_plane.normal[2], state.feature_plane.normal[0], state.feature_plane.normal[1] };
                    } else {
                        state.feature_plane.is_active = true;
                        state.feature_plane.signed_distance_to_world_origin = 0.0f;
                        state.feature_plane.normal = { 0.0f, 1.0f, 0.0f };
                    }
                } else if (key_lambda('E')) {
                    if (click_mode_SNAP_ELIGIBLE_) {
                        result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                        state.modes.click_modifier = CLICK_MODIFIER_SNAP_TO_END_OF;
                    } else {
                        // result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                        state.modes.enter_mode = ENTER_MODE_EXTRUDE_ADD;
                        if (click_mode_NEEDS_CONSOLE) state.modes.click_mode = CLICK_MODE_NONE;
                        state.console = {};
                    }
                } else if (key_lambda('M')) {
                    if (click_mode_SNAP_ELIGIBLE_) {
                        result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                        state.modes.click_modifier = CLICK_MODIFIER_SNAP_TO_MIDDLE_OF;
                        state.two_click_command.awaiting_second_click = false;
                    } else {
                        result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                        state.modes.click_mode = CLICK_MODE_DXF_MOVE;
                        state.modes.click_modifier = CLICK_MODIFIER_NONE;
                        state.two_click_command.awaiting_second_click = false;
                    }
                } else if (key_lambda('E', false, true)) {
                    // result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                    state.modes.enter_mode = ENTER_MODE_EXTRUDE_CUT;
                    if (click_mode_NEEDS_CONSOLE) state.modes.click_mode = CLICK_MODE_NONE;
                    state.console = {};
                    state.console.flip_flag = true;
                } else if (key_lambda('R')) {
                    state.modes.enter_mode = ENTER_MODE_REVOLVE_ADD;
                } else if (key_lambda('R', false, true)) {
                    state.modes.enter_mode = ENTER_MODE_REVOLVE_CUT;
                } else if (key_lambda('W')) {
                    if ((state.modes.click_mode == CLICK_MODE_SELECT) || (state.modes.click_mode == CLICK_MODE_DESELECT)) {
                        state.modes.click_modifier = CLICK_MODIFIER_WINDOW;
                        state.two_click_command.awaiting_second_click = false;
                    }
                } else if (key_lambda('M')) {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    state.modes.click_mode = CLICK_MODE_MEASURE;
                    state.modes.click_modifier = CLICK_MODIFIER_NONE;
                    state.two_click_command.awaiting_second_click = false;
                } else if (key_lambda('L')) {
                    result = PROCESSED_EVENT_CATEGORY_RECORD;
                    state.modes.click_mode = CLICK_MODE_CREATE_LINE;
                    state.modes.click_modifier = CLICK_MODIFIER_NONE;
                    state.two_click_command.awaiting_second_click = false;
                    ui.space_bar_event_key = 'L'; // FORNOW 
                } else if (key_lambda('F')) {
                    result = PROCESSED_EVENT_CATEGORY_RECORD;
                    state.modes.click_mode = CLICK_MODE_CREATE_FILLET;
                    state.modes.click_modifier = CLICK_MODIFIER_NONE;
                    state.modes.enter_mode = ENTER_MODE_NONE;
                    state.two_click_command.awaiting_second_click = false;
                } else if (key_lambda('B')) {
                    result = PROCESSED_EVENT_CATEGORY_RECORD;
                    state.modes.click_mode = CLICK_MODE_CREATE_BOX;
                    state.modes.click_modifier = CLICK_MODIFIER_NONE;
                    state.two_click_command.awaiting_second_click = false;
                    ui.space_bar_event_key = 'B'; // FORNOW
                } else if (key_lambda(COW_KEY_BACKSPACE)) {
                    for (int32 i = state.dxf.entities.length - 1; i >= 0; --i) {
                        if (state.dxf.is_selected.array[i]) {
                            delete_dxf_entity_and_mask_bool_lambda_NOTE_if_iterating_must_be_going_backwards(i);
                        }
                    }
                    list_memset(&state.dxf.is_selected, 0, state.dxf.entities.length * sizeof(bool32));
                } else {
                    result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;
                    ;
                }
            }
        } else if (event.type == UI_EVENT_TYPE_MOUSE_2D_PRESS) {
            result = PROCESSED_EVENT_CATEGORY_DONT_RECORD;

            auto set_dxf_selection_mask = [&result] (uint32 i, bool32 value_to_write) {
                // Only remember state.dxf.entities selection operations that actually change the mask
                // NOTE: we could instead do a memcmp at the end, but let's stick with the simple bool32 result = false; ... ret result; approach fornow
                if (state.dxf.is_selected.array[i] != value_to_write) {
                    result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                    state.dxf.is_selected.array[i] = value_to_write;
                }
            };


            bool32 value_to_write_to_selection_mask = (state.modes.click_mode == CLICK_MODE_SELECT);
            if ((
                        (state.modes.click_mode == CLICK_MODE_MEASURE) ||
                        (state.modes.click_mode == CLICK_MODE_CREATE_LINE) ||
                        (state.modes.click_mode == CLICK_MODE_CREATE_BOX) ||
                        (state.modes.click_mode == CLICK_MODE_CREATE_CIRCLE) ||
                        (state.modes.click_mode == CLICK_MODE_CREATE_FILLET) ||
                        (state.modes.click_mode == CLICK_MODE_DXF_MOVE) ||
                        (((state.modes.click_mode == CLICK_MODE_SELECT) || (state.modes.click_mode == CLICK_MODE_DESELECT)) && (state.modes.click_modifier == CLICK_MODIFIER_WINDOW)))
                    && (!state.two_click_command.awaiting_second_click)) { 
                result = (state.modes.click_mode == CLICK_MODE_MEASURE) ? PROCESSED_EVENT_CATEGORY_DONT_RECORD : PROCESSED_EVENT_CATEGORY_RECORD; 
                if (!(((state.modes.click_mode == CLICK_MODE_SELECT) || (state.modes.click_mode == CLICK_MODE_DESELECT)) && (state.modes.click_modifier == CLICK_MODIFIER_WINDOW))) state.modes.click_modifier = CLICK_MODIFIER_NONE;
                state.two_click_command.awaiting_second_click = true;
                state.two_click_command.first_click.x = event.mouse_x;
                state.two_click_command.first_click.y = event.mouse_y;
            } else if (state.modes.click_mode == CLICK_MODE_MEASURE) {
                ASSERT(state.two_click_command.awaiting_second_click);
                state.two_click_command.awaiting_second_click = false;
                state.modes.click_mode = CLICK_MODE_NONE;
                state.modes.click_modifier = CLICK_MODIFIER_NONE;
                real32 angle = DEG(atan2(event.mouse_y - state.two_click_command.first_click.y, event.mouse_x - state.two_click_command.first_click.x));
                if (angle < 0.0f) angle += 360.0f;
                conversation_messagef("%gmm %gdeg", sqrt(squared_distance_point_point(state.two_click_command.first_click.x, state.two_click_command.first_click.y, event.mouse_x, event.mouse_y)), angle);
            } else if (state.modes.click_mode == CLICK_MODE_CREATE_LINE) {
                ASSERT(state.two_click_command.awaiting_second_click);
                state.two_click_command.awaiting_second_click = false;
                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                state.modes.click_mode = CLICK_MODE_NONE;
                state.modes.click_modifier = CLICK_MODIFIER_NONE;
                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, state.two_click_command.first_click.x, state.two_click_command.first_click.y, event.mouse_x, event.mouse_y });
            } else if (state.modes.click_mode == CLICK_MODE_CREATE_BOX) {
                ASSERT(state.two_click_command.awaiting_second_click);
                state.two_click_command.awaiting_second_click = false;
                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                state.modes.click_mode = CLICK_MODE_NONE;
                state.modes.click_modifier = CLICK_MODIFIER_NONE;
                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, state.two_click_command.first_click.x, state.two_click_command.first_click.y, event.mouse_x, state.two_click_command.first_click.y });
                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, state.two_click_command.first_click.x, state.two_click_command.first_click.y, state.two_click_command.first_click.x, event.mouse_y });
                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, event.mouse_x, event.mouse_y, event.mouse_x, state.two_click_command.first_click.y });
                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_LINE, DXF_COLOR_TRAVERSE, event.mouse_x, event.mouse_y, state.two_click_command.first_click.x, event.mouse_y });
            } else if (state.modes.click_mode == CLICK_MODE_DXF_MOVE) {
                ASSERT(state.two_click_command.awaiting_second_click);
                state.two_click_command.awaiting_second_click = false;
                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                state.modes.click_mode = CLICK_MODE_NONE;
                state.modes.click_modifier = CLICK_MODIFIER_NONE;
                real32 dx = event.mouse_x - state.two_click_command.first_click.x;
                real32 dy = event.mouse_y - state.two_click_command.first_click.y;
                for (uint32 i = 0; i < state.dxf.entities.length; ++i) {
                    DXFEntity *entity = &state.dxf.entities.array[i];
                    if (state.dxf.is_selected.array[i]) {
                        entity->line.start_x += dx;
                        entity->line.start_y += dy;
                        entity->line.end_x   += dx;
                        entity->line.end_y   += dy;
                        entity->arc.center_x += dx;
                        entity->arc.center_y += dy;
                    }
                }
            } else if (state.modes.click_mode == CLICK_MODE_CREATE_CIRCLE) {
                ASSERT(state.two_click_command.awaiting_second_click);
                state.two_click_command.awaiting_second_click = false;
                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                state.modes.click_mode = CLICK_MODE_NONE;
                state.modes.click_modifier = CLICK_MODIFIER_NONE;
                real32 theta_a_in_degrees = DEG(atan2(event.mouse_y - state.two_click_command.first_click.y, event.mouse_x - state.two_click_command.first_click.x));
                real32 theta_b_in_degrees = theta_a_in_degrees + 180.0f;
                real32 r = SQRT(squared_distance_point_point(state.two_click_command.first_click.x, state.two_click_command.first_click.y, event.mouse_x, event.mouse_y));
                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_ARC, DXF_COLOR_TRAVERSE, state.two_click_command.first_click.x, state.two_click_command.first_click.y, r, theta_a_in_degrees, theta_b_in_degrees });
                push_back_dxf_entity_and_mask_bool_lambda({ DXF_ENTITY_TYPE_ARC, DXF_COLOR_TRAVERSE, state.two_click_command.first_click.x, state.two_click_command.first_click.y, r, theta_b_in_degrees, theta_a_in_degrees });
            } else if (state.modes.click_mode == CLICK_MODE_ORIGIN_MOVE) {
                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                state.dxf.feature_reference_point.x = event.mouse_x;
                state.dxf.feature_reference_point.y = event.mouse_y;
                state.modes.click_mode = CLICK_MODE_NONE;
                state.modes.click_modifier = CLICK_MODIFIER_NONE;
                state.modes.enter_mode = ENTER_MODE_NONE;
            } else if (state.modes.click_mode == CLICK_MODE_CREATE_FILLET) {
                ASSERT(state.two_click_command.awaiting_second_click);
                state.two_click_command.awaiting_second_click = false;
                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                state.modes.click_modifier = CLICK_MODIFIER_NONE;
                int i = dxf_find_closest_entity(&state.dxf.entities, state.two_click_command.first_click.x, state.two_click_command.first_click.y);
                int j = dxf_find_closest_entity(&state.dxf.entities, event.mouse_x, event.mouse_y);
                if ((i != j) && (i != -1) && (j != -1)) {
                    real32 radius = console_param_1;
                    DXFEntity *E_i = &state.dxf.entities.array[i];
                    DXFEntity *E_j = &state.dxf.entities.array[j];
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
                                vec2 m1 = { state.two_click_command.first_click.x, state.two_click_command.first_click.y };
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
            } else if ((state.modes.click_mode == CLICK_MODE_SELECT) || (state.modes.click_mode == CLICK_MODE_DESELECT)) {
                if (state.modes.click_modifier != CLICK_MODIFIER_WINDOW) {
                    int hot_entity_index = dxf_find_closest_entity(&state.dxf.entities, event.mouse_x, event.mouse_y);
                    if (hot_entity_index != -1) {
                        if (state.modes.click_modifier == CLICK_MODIFIER_CONNECTED) {
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

                                for (uint32 entity_index = 0; entity_index < state.dxf.entities.length; ++entity_index) {
                                    DXFEntity *entity = &state.dxf.entities.array[entity_index];

                                    real32 start_x, start_y, end_x, end_y;
                                    entity_get_start_and_end_points(entity, &start_x, &start_y, &end_x, &end_y);
                                    push_into_grid_unless_cell_full__make_cell_if_none_exists(start_x, start_y, entity_index, false);
                                    push_into_grid_unless_cell_full__make_cell_if_none_exists(end_x, end_y, entity_index, true);
                                }
                            }

                            bool32 *edge_marked = (bool32 *) calloc(state.dxf.entities.length, sizeof(bool32));

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
                                    DXFEntity *entity = &state.dxf.entities.array[point->entity_index];
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
                                        entity_get_start_point(&state.dxf.entities.array[hot_entity_index], &x, &y);
                                    } else {
                                        entity_get_end_point(&state.dxf.entities.array[hot_entity_index], &x, &y);
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
                } else { ASSERT(state.modes.click_modifier == CLICK_MODIFIER_WINDOW);
                    ASSERT(state.two_click_command.awaiting_second_click);
                    state.two_click_command.awaiting_second_click = false;
                    result = PROCESSED_EVENT_CATEGORY_RECORD;
                    BoundingBox window = {
                        MIN(state.two_click_command.first_click.x, event.mouse_x),
                        MIN(state.two_click_command.first_click.y, event.mouse_y),
                        MAX(state.two_click_command.first_click.x, event.mouse_x),
                        MAX(state.two_click_command.first_click.y, event.mouse_y)
                    };
                    for (uint32 i = 0; i < state.dxf.entities.length; ++i) {
                        if (bounding_box_contains(window, dxf_entity_get_bounding_box(&state.dxf.entities.array[i]))) {
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
                for (uint32 i = 0; i < state.mesh.num_triangles; ++i) {
                    vec3 p[3]; {
                        for (uint32 j = 0; j < 3; ++j) p[j] = get(state.mesh.vertex_positions, state.mesh.triangle_indices[3 * i + j]);
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
                                                            // TODO don't record double click on same triangle
                result = PROCESSED_EVENT_CATEGORY_CHECKPOINT;
                state.feature_plane.is_active = true;
                {
                    state.feature_plane.normal = get(state.mesh.triangle_normals, index_of_first_triangle_hit_by_ray);
                    { // FORNOW (gross) calculateion of state.feature_plane.signed_distance_to_world_origin
                        vec3 a_selected = get(state.mesh.vertex_positions, state.mesh.triangle_indices[3 * index_of_first_triangle_hit_by_ray + 0]);
                        state.feature_plane.signed_distance_to_world_origin = dot(state.feature_plane.normal, a_selected);
                    }
                }
            }
        }
    }
    // result = PROCESSED_EVENT_CATEGORY_CHECKPOINT; // FORNOW: testing
    // result = PROCESSED_EVENT_CATEGORY_SUPER_CHECKPOINT; // FORNOW: testing
    return result;
}


//////////////////////////////////////////////////
// PROCESS A FRESH (potentially special EVENT) ///
// (HISTORY)                                   ///
// (undo, redo, space, ...)                    ///
//////////////////////////////////////////////////

// TODO: HistoryStack
Stack<WorldState> super_undo_stack;
Stack<WorldState> super_redo_stack;

void super_stacks_do__NOTE_clears_redo_stack() {
    for (uint32 i = 0; i < super_redo_stack.length; ++i) {
        world_state_free(&super_redo_stack.array[i]);
    }

    WorldState deep_copy;
    world_state_deep_copy(&deep_copy, &state);
    stack_push(&super_undo_stack, deep_copy);
}

void _super_stacks_deepcopy_peek_undo_stack_NOTE_frees() {
    world_state_free(&state);

    ASSERT(super_undo_stack.length);
    WorldState other = stack_peek(&super_undo_stack); // FORNOW; TODO: pointer version?
    world_state_deep_copy(&state, &other);
}

void super_stacks_undo_and_load() {
    ASSERT(super_undo_stack.length);
    stack_push(&super_redo_stack, stack_pop(&super_undo_stack));
    _super_stacks_deepcopy_peek_undo_stack_NOTE_frees();
}

void super_stacks_redo_and_load() {
    ASSERT(super_redo_stack.length);
    stack_push(&super_undo_stack, stack_pop(&super_redo_stack));
    _super_stacks_deepcopy_peek_undo_stack_NOTE_frees();
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

UserEvent history_A_undo[999999];
UserEvent *history_B_redo = history_A_undo;
UserEvent *history_C_one_past_end_of_redo = history_A_undo;

void fresh_event_from_user_process(UserEvent fresh_event_from_user) {
    auto key_lambda = [fresh_event_from_user](uint32 key, bool super = false, bool shift = false) -> bool {
        if (fresh_event_from_user.type != UI_EVENT_TYPE_KEY_PRESS) return false;
        return _key_lambda(fresh_event_from_user, key, super, shift);
    };

    // space bar repeats previous command (TODO NOTE: This approach won't actually work; it needs to be in standard_event_process)
    // (otherwise we can't differentiate between (c)enter and (c)icle)
    if (key_lambda(' ')) {
        fresh_event_from_user = {};
        fresh_event_from_user.type = UI_EVENT_TYPE_KEY_PRESS;
        fresh_event_from_user.key = ui.space_bar_event_key;
    }

    if (key_lambda('Z')) { // we handle the user pressing z by spoofing a key press at the feature_reference_point
        fresh_event_from_user = { UI_EVENT_TYPE_MOUSE_2D_PRESS, 0.0f, 0.0f };
    } else if (key_lambda('Z', true) || key_lambda('U')) { // undo
        #define _UNDO_STACK_NONEMPTY_ (history_A_undo != history_B_redo)

        if (_UNDO_STACK_NONEMPTY_) {
            bool32 just_undid_super_checkpoint = false;
            // // move history_B_redo
            // pop back _through_ a first checkpoint__NOTE_set_in_new_event_process
            do {
                --history_B_redo;
                just_undid_super_checkpoint |= (history_B_redo->checkpoint_type == CHECKPOINT_TYPE_SUPER_CHECKPOINT);
            } while ((_UNDO_STACK_NONEMPTY_) && (history_B_redo->checkpoint_type == CHECKPOINT_TYPE_NONE));
            // * short-circuit pop back _up to_ a second checkpoint
            while ((_UNDO_STACK_NONEMPTY_) && ((history_B_redo - 1)->checkpoint_type == CHECKPOINT_TYPE_NONE)) {
                --history_B_redo;
            }

            state = {};
            {
                if (!just_undid_super_checkpoint) {
                    _super_stacks_deepcopy_peek_undo_stack_NOTE_frees();
                } else {
                    super_stacks_undo_and_load();
                }
            }

            UserEvent *history_D_one_after_last_super_checkpoint; {
                history_D_one_after_last_super_checkpoint = history_B_redo;
                while ((history_A_undo != history_D_one_after_last_super_checkpoint) && (history_D_one_after_last_super_checkpoint - 1)->checkpoint_type != CHECKPOINT_TYPE_SUPER_CHECKPOINT) --history_D_one_after_last_super_checkpoint;
            }

            for (UserEvent *event = history_D_one_after_last_super_checkpoint; event < history_B_redo; ++event) standard_event_process(*event);

            conversation_messagef("[undo] success");
        } else {
            conversation_messagef("[undo] nothing to undo");
        }
    } else if (key_lambda('Y', true) || key_lambda('Z', true, true) || key_lambda('U', false, true)) { // redo
        if (history_B_redo != history_C_one_past_end_of_redo) {
            do {

                if (history_B_redo->checkpoint_type != CHECKPOINT_TYPE_SUPER_CHECKPOINT) {
                    standard_event_process(*history_B_redo);
                } else {
                    standard_event_process(*history_B_redo, true);
                    super_stacks_redo_and_load();
                }

                ++history_B_redo;

            } while ((history_B_redo != history_C_one_past_end_of_redo) && ((history_B_redo - 1)->checkpoint_type == CHECKPOINT_TYPE_NONE));
            conversation_messagef("[redo] success");
        } else {
            conversation_messagef("[redo] nothing to redo");
        }
    } else { // standard event
        uint32 category = standard_event_process(fresh_event_from_user);

        if (category == PROCESSED_EVENT_CATEGORY_DONT_RECORD) {
        } else if (category == PROCESSED_EVENT_CATEGORY_KILL_HISTORY) {
            history_B_redo = history_C_one_past_end_of_redo = history_A_undo;
            queue_free_AND_zero(&queue_of_fresh_events_from_user);
            super_stacks_do__NOTE_clears_redo_stack();
            ASSERT(false); // TODO
        } else {
            { // FORNOW (sloppy): assign checkpoint_type
                fresh_event_from_user.checkpoint_type = CHECKPOINT_TYPE_NONE; // FORNOW (not necessary)
                if ((
                            0
                            || (category == PROCESSED_EVENT_CATEGORY_CHECKPOINT)
                            || (category == PROCESSED_EVENT_CATEGORY_SUPER_CHECKPOINT)
                    )
                        && (!fresh_event_from_user.checkpoint_ineligible)
                   ) {
                    if (category == PROCESSED_EVENT_CATEGORY_CHECKPOINT) {
                        fresh_event_from_user.checkpoint_type = CHECKPOINT_TYPE_CHECKPOINT;
                    } else if (category == PROCESSED_EVENT_CATEGORY_SUPER_CHECKPOINT) {
                        fresh_event_from_user.checkpoint_type = CHECKPOINT_TYPE_SUPER_CHECKPOINT;
                    }
                } 
            }

            { // history_push_back(...)
                *history_B_redo++ = fresh_event_from_user;
                history_C_one_past_end_of_redo = history_B_redo; // kill redo "stack"
            }

            if (category == PROCESSED_EVENT_CATEGORY_SUPER_CHECKPOINT) { // push super checkpoint if necessary
                super_stacks_do__NOTE_clears_redo_stack();
            }

        }
    }
}


//////////////////////////////////////////////////
// DRAW() ////////////////////////////////////////
//////////////////////////////////////////////////

void history_debug_draw__NOTE_guiprintf() {
    gui_printf("----------------");
    gui_printf("super_undo_stack.length = %d", super_undo_stack.length);
    gui_printf("note: top of the undo stack in blue");
    gui_printf("----------------");
    uint32 i = 0;
    for (UserEvent *event = history_A_undo; event < history_C_one_past_end_of_redo; ++event) {
        {
            char leader_leader = (event->checkpoint_type == CHECKPOINT_TYPE_SUPER_CHECKPOINT) ? '+' : ' ';
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

void conversation_draw() {
    mat4 M_3D_from_2D = get_M_3D_from_2D();

    // FORNOW: repeated computation
    bool32 dxf_anything_selected;
    {
        dxf_anything_selected = false;
        for (uint32 i = 0; i < state.dxf.entities.length; ++i) {
            if (state.dxf.is_selected.array[i]) {
                dxf_anything_selected = true;
                break;
            }
        }
    }
    // FORNOW: sloppy--these change mid-frame
    real32 console_param_1, console_param_2;
    get_console_params(&console_param_1, &console_param_2);



    // FORNOW: repeated computation
    mat4 PV_2D = camera_get_PV(&ui.camera_2D);
    real32 mouse_x, mouse_y; { _input_get_mouse_position_and_change_in_position_in_world_coordinates(PV_2D.data, &mouse_x, &mouse_y, NULL, NULL); }
    mat4 P_3D = camera_get_P(&ui.camera_3D);
    mat4 V_3D = camera_get_V(&ui.camera_3D);
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
                if (ui.hot_pane == HOT_PANE_2D) {
                    eso_begin(globals.Identity, SOUP_QUADS);
                    eso_color(0.1f, 0.1f, 0.0f);
                    eso_vertex(-1.0f,  1.0f);
                    eso_vertex(-1.0f, -1.0f);
                    eso_vertex( 0.0f, -1.0f);
                    eso_vertex( 0.0f,  1.0f);
                    eso_end();
                } else if (ui.hot_pane == HOT_PANE_3D) {
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
            if (!ui.hide_grid) { // grid 2D grid 2d grid
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
                for (uint32 i = 0; i < state.dxf.entities.length; ++i) {
                    DXFEntity *entity = &state.dxf.entities.array[i];
                    int32 color = (state.dxf.is_selected.array[i]) ? DXF_COLOR_SELECTION : DXF_COLOR_DONT_OVERRIDE;
                    real32 dx = 0.0f;
                    real32 dy = 0.0f;
                    if ((state.modes.click_mode == CLICK_MODE_DXF_MOVE) && (state.two_click_command.awaiting_second_click)) {
                        if (state.dxf.is_selected.array[i]) {
                            dx = mouse_x - state.two_click_command.first_click.x;
                            dy = mouse_y - state.two_click_command.first_click.y;
                            color = DXF_COLOR_WATER_ONLY;
                        }
                    }
                    eso_dxf_entity__SOUP_LINES(entity, color, dx, dy);
                }
                eso_end();
            }
            { // dots
                if (ui.show_details) {
                    eso_begin(camera_get_PV(&ui.camera_2D), SOUP_POINTS, 8.0f);
                    eso_color(monokai.white);
                    for (DXFEntity *entity = state.dxf.entities.array; entity < &state.dxf.entities.array[state.dxf.entities.length]; ++entity) {
                        real32 start_x, start_y, end_x, end_y;
                        entity_get_start_and_end_points(entity, &start_x, &start_y, &end_x, &end_y);
                        eso_vertex(start_x, start_y);
                        eso_vertex(end_x, end_y);
                    }
                    eso_end();
                }
            }
            { // axes 2D axes 2d axes axis 2D axis 2d axes crosshairs cross hairs feature_reference_point 2d feature_reference_point 2D feature_reference_point
                real32 r = ui.camera_2D.screen_height_World / 120.0f;
                mat4 M = M4_Translation(state.dxf.feature_reference_point.x, state.dxf.feature_reference_point.y);
                vec3 color = V3(0.8f, 0.8f, 1.0f);
                eso_begin(PV_2D * M, SOUP_LINES);
                eso_color(color);
                eso_vertex(-r*.7f, 0.0f);
                eso_vertex( r, 0.0f);
                eso_vertex(0.0f, -r);
                eso_vertex(0.0f,  r*.7f);
                eso_end();
            }

            if (state.two_click_command.awaiting_second_click) {
                if (state.modes.click_modifier == CLICK_MODIFIER_WINDOW) {
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(0.0f, 1.0f, 1.0f);
                    real32 x0 = state.two_click_command.first_click.x;
                    real32 y0 = state.two_click_command.first_click.y;
                    real32 x1 = mouse_x;
                    real32 y1 = mouse_y;
                    eso_vertex(x0, y0);
                    eso_vertex(x1, y0);
                    eso_vertex(x1, y1);
                    eso_vertex(x0, y1);
                    eso_end();
                }
                if (state.modes.click_mode == CLICK_MODE_MEASURE) { // measure line
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(0.0f, 1.0f, 1.0f);
                    eso_vertex(state.two_click_command.first_click.x, state.two_click_command.first_click.y);
                    eso_vertex(mouse_x, mouse_y);
                    eso_end();
                }
                if (state.modes.click_mode == CLICK_MODE_CREATE_LINE) { // measure line
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(0.0f, 1.0f, 1.0f);
                    eso_vertex(state.two_click_command.first_click.x, state.two_click_command.first_click.y);
                    eso_vertex(mouse_x, mouse_y);
                    eso_end();
                }
                if (state.modes.click_mode == CLICK_MODE_CREATE_BOX) {
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(0.0f, 1.0f, 1.0f);
                    eso_vertex(state.two_click_command.first_click.x, state.two_click_command.first_click.y);
                    eso_vertex(mouse_x,               state.two_click_command.first_click.y);
                    eso_vertex(mouse_x,               mouse_y              );
                    eso_vertex(state.two_click_command.first_click.x, mouse_y              );
                    eso_end();
                }
                if (state.modes.click_mode == CLICK_MODE_CREATE_CIRCLE) {
                    vec2 c = { state.two_click_command.first_click.x, state.two_click_command.first_click.y };
                    vec2 p = { mouse_x, mouse_y };
                    real32 r = norm(c - p);
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(0.0f, 1.0f, 1.0f);
                    for (uint32 i = 0; i < NUM_SEGMENTS_PER_CIRCLE; ++i) eso_vertex(c + r * e_theta(NUM_DEN(i, NUM_SEGMENTS_PER_CIRCLE) * TAU));
                    eso_end();
                }
                if (state.modes.click_mode == CLICK_MODE_CREATE_FILLET) {
                    // FORNOW
                    int i = dxf_find_closest_entity(&state.dxf.entities, state.two_click_command.first_click.x, state.two_click_command.first_click.y);
                    if (i != -1) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(0.0f, 1.0f, 1.0f);
                        eso_dxf_entity__SOUP_LINES(&state.dxf.entities.array[i], DXF_COLOR_WATER_ONLY);
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
            uint32 color = ((state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD)) ? DXF_COLOR_TRAVERSE : ((state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) || (state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT)) ? DXF_COLOR_QUALITY_1 : ((state.modes.enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO) || (state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) ? DXF_COLOR_WATER_ONLY : DXF_COLOR_SELECTION;

            uint32 NUM_TUBE_STACKS_INCLUSIVE;
            mat4 M;
            mat4 M_incr;
            if (true) {
                if ((state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                    // NOTE: some repetition with wrapper
                    real32 a = MIN(0.0f, MIN(console_param_1, console_param_2));
                    real32 b = MAX(0.0f, MAX(console_param_1, console_param_2));
                    real32 length = b - a;
                    NUM_TUBE_STACKS_INCLUSIVE = MIN(64, uint32(roundf(length / 2.5f)) + 2);
                    M = M_3D_from_2D * M4_Translation(-state.dxf.feature_reference_point.x, -state.dxf.feature_reference_point.y, a + Z_FIGHT_EPS);
                    M_incr = M4_Translation(0.0f, 0.0f, (b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                } else if ((state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) || (state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT)) {
                    NUM_TUBE_STACKS_INCLUSIVE = 64;
                    M = M_3D_from_2D * M4_Translation(-state.dxf.feature_reference_point.x, -state.dxf.feature_reference_point.y, 0.0f);
                    real32 a = 0.0f;
                    real32 b = TAU;
                    real32 argument  = (b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1);
                    mat4 R = ((state.console.flip_flag) ? M4_RotationAboutXAxis(argument) : M4_RotationAboutYAxis(argument));
                    M_incr = M4_Translation(state.dxf.feature_reference_point.x, state.dxf.feature_reference_point.y, 0.0f) * R * M4_Translation(-state.dxf.feature_reference_point.x, -state.dxf.feature_reference_point.y, 0.0f);
                } else if (state.modes.enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO) {
                    // FORNOW
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D;
                    M_incr = M4_Identity();
                } else if (state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * M4_Translation(-state.dxf.feature_reference_point.x, -state.dxf.feature_reference_point.y, console_param_1 + Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * M4_Translation(-state.dxf.feature_reference_point.x, -state.dxf.feature_reference_point.y, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                }
                for (uint32 tube_stack_index = 0; tube_stack_index < NUM_TUBE_STACKS_INCLUSIVE; ++tube_stack_index) {
                    {
                        eso_begin(PV_3D * M, SOUP_LINES, 5.0f);
                        for (uint32 i = 0; i < state.dxf.entities.length; ++i) {
                            DXFEntity *entity = &state.dxf.entities.array[i];
                            if (state.dxf.is_selected.array[i]) {
                                eso_dxf_entity__SOUP_LINES(entity, color);
                            }
                        }
                        eso_end();
                    }
                    M *= M_incr;
                }
            }
        }

        BoundingBox selection_bounding_box = dxf_entities_get_bounding_box(&state.dxf.entities, state.dxf.is_selected.array);

        if (dxf_anything_selected) { // arrow
            if ((state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) || (state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) || (state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT)) {

                vec3 color = ((state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD)) ? V3(83.0f / 255, 255.0f / 255, 83.0f / 255.0f) : V3(1.0f, 0.0f, 0.0f);

                real32 arrow_x = 0.0f;
                real32 arrow_y = 0.0f;
                real32 H[2] = { console_param_1, console_param_2 };
                bool32 toggle[2] = { state.console.flip_flag, !state.console.flip_flag };
                mat4 A = M_3D_from_2D;
                if ((state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                    bounding_box_center(selection_bounding_box, &arrow_x, &arrow_y);
                    if (dxf_anything_selected) {
                        arrow_x -= state.dxf.feature_reference_point.x;
                        arrow_y -= state.dxf.feature_reference_point.y;
                    }
                } else { ASSERT((state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) || (state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT));
                    H[0] = 10.0f + selection_bounding_box.max[(!state.console.flip_flag) ? 1 : 0];
                    H[1] = 0.0f;
                    toggle[0] = false;
                    toggle[1] = false;
                    { // A
                        A *= M4_RotationAboutXAxis(RAD(-90.0f));
                        if (state.console.flip_flag) {
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

        if (state.feature_plane.is_active) { // axes 3D axes 3d axes axis 3D axis 3d axis
            real32 r = ui.camera_3D.ortho_screen_height_World / 120.0f;
            eso_begin(PV_3D * M_3D_from_2D * M4_Translation(0.0f, 0.0f, Z_FIGHT_EPS), SOUP_LINES, 4.0f);
            eso_color(0.7f, 0.7f, 1.0f);
            // eso_color(0.0f, 0.0f, 0.0f);
            eso_vertex(-r*0.6f, 0.0f);
            eso_vertex(r, 0.0f);
            eso_vertex(0.0f, -r);
            eso_vertex(0.0f, r*0.6f);
            eso_end();
        }

        { // mesh; NOTE: includes transparency 3d mesh 3D mesh 3d
            if (state.mesh.cosmetic_edges) {
                eso_begin(PV_3D, SOUP_LINES); 
                eso_color(monokai.black);
                // 3 * num_triangles * 2 / 2
                for (uint32 k = 0; k < 2 * state.mesh.num_cosmetic_edges; ++k) eso_vertex(state.mesh.vertex_positions, state.mesh.cosmetic_edges[k]);
                eso_end();
            }
            for (uint32 pass = 0; pass <= 1; ++pass) {
                eso_begin(PV_3D, (!ui.show_details) ? SOUP_TRIANGLES : SOUP_OUTLINED_TRIANGLES);

                mat3 inv_transpose_V_3D = inverse(transpose(M3(V_3D(0, 0), V_3D(0, 1), V_3D(0, 2), V_3D(1, 0), V_3D(1, 1), V_3D(1, 2), V_3D(2, 0), V_3D(2, 1), V_3D(2, 2))));

                for (uint32 i = 0; i < state.mesh.num_triangles; ++i) {
                    vec3 n = get(state.mesh.triangle_normals, i);
                    vec3 p[3];
                    real32 x_n;
                    {
                        for (uint32 j = 0; j < 3; ++j) p[j] = get(state.mesh.vertex_positions, state.mesh.triangle_indices[3 * i + j]);
                        x_n = dot(n, p[0]);
                    }
                    vec3 color; 
                    real32 alpha;
                    {
                        vec3 n_camera = inv_transpose_V_3D * n;
                        vec3 color_n = V3(0.5f + 0.5f * n_camera.x, 0.5f + 0.5f * n_camera.y, 1.0f);
                        if ((state.feature_plane.is_active) && (dot(n, state.feature_plane.normal) > 0.999f) && (ABS(x_n - state.feature_plane.signed_distance_to_world_origin) < 0.001f)) {
                            if (pass == 0) continue;
                            color = V3(0.85f, 0.87f, 0.30f);
                            alpha = ((state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD || (state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) && ((state.console.flip_flag) || (console_param_2 != 0.0f))) ? 0.7f : 1.0f;
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

        if (!ui.hide_grid) { // grid 3D grid 3d grid
            conversation_draw_3D_grid_box(P_3D, V_3D);
        }

        { // floating sketch plane; NOTE: transparent
            bool draw = true;
            mat4 PVM = PV_3D * M_3D_from_2D;
            vec3 color = monokai.yellow;
            real32 sign = -1.0f;
            if (state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                PVM *= M4_Translation(-state.dxf.feature_reference_point.x, -state.dxf.feature_reference_point.y, console_param_1);
                color = { 0.0f, 1.0f, 1.0f };
                sign = 1.0f;
                draw = true;
            } else if (state.modes.enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO) {
                color = { 0.0f, 1.0f, 1.0f };
                sign = 1.0f;
                draw = true;
            } else {
                if (dxf_anything_selected) PVM *= M4_Translation(-state.dxf.feature_reference_point.x, -state.dxf.feature_reference_point.y, 0.0f); // FORNOW
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

    if (!ui.hide_gui) { // gui
        {
            char click_message[256] = {};
            if (state.modes.click_mode == CLICK_MODE_CREATE_FILLET) {
                sprintf(click_message, "radius:%gmm", console_param_1);
            }
            gui_printf("[Click] %s %s %s",
                    (state.modes.click_mode == CLICK_MODE_NONE) ? "NONE" :
                    (state.modes.click_mode == CLICK_MODE_ORIGIN_MOVE) ? "MOVE_ORIGIN_TO" :
                    (state.modes.click_mode == CLICK_MODE_SELECT) ? "SELECT" :
                    (state.modes.click_mode == CLICK_MODE_DESELECT) ? "DESELECT" :
                    (state.modes.click_mode == CLICK_MODE_MEASURE) ? "MEASURE" :
                    (state.modes.click_mode == CLICK_MODE_CREATE_LINE) ? "CREATE_LINE" :
                    (state.modes.click_mode == CLICK_MODE_CREATE_BOX) ? "CREATE_BOX" :
                    (state.modes.click_mode == CLICK_MODE_CREATE_CIRCLE) ? "CREATE_CIRCLE" :
                    (state.modes.click_mode == CLICK_MODE_CREATE_FILLET) ? "CREATE_FILLET" :
                    (state.modes.click_mode == CLICK_MODE_DXF_MOVE) ? "MOVE_DXF_TO" :
                    "???",
                    (state.modes.click_modifier == CLICK_MODE_NONE) ? "" :
                    (state.modes.click_modifier == CLICK_MODIFIER_CONNECTED) ? "CONNECTED" :
                    (state.modes.click_modifier == CLICK_MODIFIER_WINDOW) ? "WINDOW" :
                    (state.modes.click_modifier == CLICK_MODIFIER_SNAP_TO_CENTER_OF) ? "CENTER_OF" :
                    (state.modes.click_modifier == CLICK_MODIFIER_SNAP_TO_END_OF) ? "END_OF" :
                    (state.modes.click_modifier == CLICK_MODIFIER_SNAP_TO_MIDDLE_OF) ? "MIDDLE_OF" :
                    (state.modes.click_modifier == CLICK_MODIFIER_QUALITY) ? "QUALITY" :
                    "???",
                    click_message);
        }

        {
            char enter_message[256] = {};
            if ((state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) || (state.modes.enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO)) {
                real32 p, p2;
                char glyph, glyph2;
                if ((state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                    p      =  console_param_1;
                    p2     = -console_param_2;
                    if (state.console.flip_flag) { // ??
                        p *= -1;
                        p2 *= -1;
                    }
                    if (IS_ZERO(p)) p = 0.0f; // FORNOW makes minus sign go away in hud (not a big deal)
                    if (IS_ZERO(p2)) p2 = 0.0f; // FORNOW makes minus sign go away in hud (not a big deal)
                    glyph  = (!state.console.flip_flag) ? '^' : 'v';
                    glyph2 = (!state.console.flip_flag) ? 'v' : '^';
                } else {
                    ASSERT(state.modes.enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO);
                    p      = console_param_1;
                    p2     = console_param_2;
                    glyph  = 'x';
                    glyph2 = 'y';
                }
                sprintf(enter_message, "%c:%gmm %c:%gmm", glyph, p, glyph2, p2);
                if (((state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) && IS_ZERO(console_param_2)) sprintf(enter_message, "%c:%gmm", glyph, p);
            } else if ((state.modes.enter_mode == ENTER_MODE_OPEN) || (state.modes.enter_mode == ENTER_MODE_SAVE)) {
                sprintf(enter_message, "%s%s", ui.drop_path, state.console.buffer);
            }

            gui_printf("[Enter] %s %s",
                    (state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) ? "EXTRUDE_ADD" :
                    (state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) ? "EXTRUDE_CUT" :
                    (state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) ? "REVOLVE_ADD" :
                    (state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT) ? "REVOLVE_CUT" :
                    (state.modes.enter_mode == ENTER_MODE_OPEN) ? "OPEN" :
                    (state.modes.enter_mode == ENTER_MODE_SAVE) ? "SAVE" :
                    (state.modes.enter_mode == ENTER_MODE_MOVE_DXF_ORIGIN_TO) ? "MOVE_DXF_ORIGIN_TO" :
                    (state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY) ? "OFFSET_PLANE_TO" :
                    (state.modes.enter_mode == ENTER_MODE_NONE) ? "NONE" :
                    "???",
                    enter_message);
        }

        if ((state.modes.enter_mode == ENTER_MODE_NONE) || (state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) || (state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT)) {
            gui_printf("> %s", state.console.buffer);
        } else {
            gui_printf("> %s", state.console.buffer);
        }

        conversation_message_buffer_update_and_draw();

        if (ui.show_details) {
            gui_printf("%d state.dxf.entities elements", state.dxf.entities.length);
            gui_printf("%d stl triangles", state.mesh.num_triangles);
        }
        if (ui.show_command_stack) history_debug_draw__NOTE_guiprintf();
        if (ui.show_help) {
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
            gui_printf("(Z)-move-feature_reference_point + (c)enter-of (e)nd-of (m)iddle-of [Click] / (-0123456789.,) (f)lip-direction [Enter]");
            gui_printf("(M)easure + (c)enter-of (e)nd-of (m)iddle-of (z)-feature_reference_point [Click]");
            gui_printf("");
            gui_printf("EXPERIMENTAL: (r)evolve-add (R)evolve-cut");
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

void conversation_init() {
    if (1) {
        // FORNOW: TODO: figure out super_stacks_do__NOTE_clears_redo_stack
        if (1) {
            conversation_dxf_load("splash.dxf");
            super_stacks_do__NOTE_clears_redo_stack(); // FORNOW: ?? here???
            if (1) {
                spoof_KEY_event('Y');
                spoof_KEY_event('S');
                spoof_KEY_event('C');
                spoof_MOUSE_2D_event( 20.0,  20.0);
                spoof_MOUSE_2D_event( 16.0,  16.0);
                spoof_MOUSE_2D_event( 16.0, -16.0);
                spoof_MOUSE_2D_event(-16.0, -16.0);
                spoof_MOUSE_2D_event(-16.0,  16.0);
                spoof_KEY_event('E');
                spoof_KEY_event('5');
                spoof_KEY_event('0');
                spoof_KEY_event(COW_KEY_ENTER); // FORNOW
                spoof_MOUSE_3D_event(50.0, 93.0, 76.0, -0.47, -0.43, -0.77);
                spoof_MOUSE_2D_event(16.4, -9.5);
                spoof_KEY_event('E', false, true);
                spoof_KEY_event('4');
                spoof_KEY_event('7');
                spoof_KEY_event(COW_KEY_ENTER); // FORNOW
                spoof_KEY_event('U');
                spoof_KEY_event('U');
                spoof_KEY_event('U');
                spoof_KEY_event('U');
                spoof_KEY_event('U', false, true);
                spoof_KEY_event('U', false, true);
                spoof_KEY_event('U', false, true);
                spoof_KEY_event('U', false, true);
            }
        } else if (1) {
            conversation_dxf_load("omax.dxf");
            super_stacks_do__NOTE_clears_redo_stack(); // FORNOW: ?? here???
            if (1) {
                UserEvent event = {};
                event.type = UI_EVENT_TYPE_KEY_PRESS;
                for (int i = 0; i < 5; ++i) {
                    spoof_KEY_event('Y');
                    spoof_KEY_event('S');
                    spoof_KEY_event('Q');
                    spoof_KEY_event('0' + i);
                    spoof_KEY_event('E');
                    spoof_KEY_event('1' + i);
                    spoof_KEY_event(COW_KEY_ENTER);
                }
            }
        } else if (0) {
            conversation_dxf_load("ik.dxf");
            super_stacks_do__NOTE_clears_redo_stack(); // FORNOW: ?? here???
        } else if (0) {
            conversation_dxf_load("debug.dxf");
            super_stacks_do__NOTE_clears_redo_stack(); // FORNOW: ?? here???
        }
    }
    init_cameras(); // FORNOW
    conversation_messagef("type h for help `// pre-alpha " __DATE__ " " __TIME__);
}

int main() {
    // _window_set_size(1.5 * 640.0, 1.5 * 360.0); // TODO the first frame crap gets buggered

    conversation_init();

    {
        // FORNOW: first frame position
        double xpos, ypos;
        glfwGetCursorPos(COW0._window_glfw_window, &xpos, &ypos);
        callback_cursor_position(NULL, xpos, ypos);
    }

    while (cow_begin_frame()) {

        // invariants
        ASSERT(state.dxf.entities.length == state.dxf.is_selected.length);

        while (queue_of_fresh_events_from_user.length) {
            fresh_event_from_user_process(queue_dequeue(&queue_of_fresh_events_from_user));
        }

        { // stuff that still shims globals.*
            if ((!globals.mouse_left_held && !globals.mouse_right_held) || globals.mouse_left_pressed || globals.mouse_right_pressed) {
                ui.hot_pane = (callback_xpos <= window_get_width() / 2) ? HOT_PANE_2D : HOT_PANE_3D;
                if ((state.modes.click_modifier == CLICK_MODIFIER_WINDOW) && (state.two_click_command.awaiting_second_click)) ui.hot_pane = HOT_PANE_2D;// FORNOW
            }
            { // camera_move (using shimmed globals.* state)
                if (ui.hot_pane == HOT_PANE_2D) {
                    camera_move(&ui.camera_2D);
                } else if (ui.hot_pane == HOT_PANE_3D) {
                    camera_move(&ui.camera_3D);
                }
            }
        }
        conversation_draw();
    }
}


