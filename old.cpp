
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

{
    char enter_message[256] = {};
    if ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) || (global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN)) {
        real32 p, p2;
        char glyph, glyph2;
        if ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
            p      =  global_world_state.popup.param0;
            p2     = -global_world_state.popup.param1;
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
            p      = global_world_state.popup.param0;
            p2     = global_world_state.popup.param1;
            glyph  = 'x';
            glyph2 = 'y';
        }
        sprintf(enter_message, "%c:%gmm %c:%gmm", glyph, p, glyph2, p2);
        if (((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) && IS_ZERO(global_world_state.popup.param1)) sprintf(enter_message, "%c:%gmm", glyph, p);
    } else if ((global_world_state.modes.enter_mode == ENTER_MODE_OPEN) || (global_world_state.modes.enter_mode == ENTER_MODE_SAVE)) {
        sprintf(enter_message, "%s%s", _global_screen_state.drop_path, global_world_state.console.buffer);
    }

}


            } else if (send_key_to_console) {
                if (!key_lambda(GLFW_KEY_ENTER)) {
                    if (key_lambda(GLFW_KEY_BACKSPACE)) {
                        if (global_world_state.console.num_bytes_written != 0) global_world_state.console.buffer[--global_world_state.console.num_bytes_written] = '\0';
                    } else {
                        global_world_state.console.buffer[global_world_state.console.num_bytes_written++] = character_equivalent;
                    }
                } else
                { // wrapper
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
                                        global_world_state.popup.param0,
                                        global_world_state.popup.param1,
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
                        // } else if (global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN) {
                        //     global_world_state.dxf.origin.x = global_world_state.popup.param0;
                        //     global_world_state.dxf.origin.y = global_world_state.popup.param1;
                } else { ASSERT((global_world_state.modes.enter_mode == ENTER_MODE_OPEN) || (global_world_state.modes.enter_mode == ENTER_MODE_SAVE));
                    static char full_filename_including_path[512];
                    sprintf(full_filename_including_path, "%s%s", _global_screen_state.drop_path, global_world_state.popup.buffer0);
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
                                warn_once("check for 0 extrude total height disabled");
                                // if (IS_ZERO(global_world_state.popup.param0) && IS_ZERO(global_world_state.popup.param1)) {
                                //     conversation_messagef("[enter] extrude height is zero");
                                //     send_key_to_console = false;
                                // }
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
        char character_equivalent; {
            character_equivalent = (char) effective_event.key;
            if (effective_event.shift && (effective_event.key == '-')) character_equivalent = '_';
            if ((!effective_event.shift) && ('A' <= effective_event.key) && (effective_event.key <= 'Z')) character_equivalent = (char) ('a' + (effective_event.key - 'A'));
        }
