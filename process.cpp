// TODO: beautiful buttons; should indicate what's selected in green (persistent)
// FORNOW: rotate copy's usage of second click is wonky
// TODO: mouse_transformed_position still pops on undo/redo
// ~~~~: snaps flicker when typing in the popup 
// XXXX: click modifier belongs in other

StandardEventProcessResult _standard_event_process_NOTE_RECURSIVE(Event event);
StandardEventProcessResult standard_event_process(Event event) {
    // bool global_world_state_changed;
    popup->manager.begin_process(); // NOTE: this is actually why we *need* this helper
                                    //       (the auto-recording is just a bonus)
    StandardEventProcessResult result = _standard_event_process_NOTE_RECURSIVE(event);
    // if (global_world_state_changed) {
    //     result.record_me = true;
    // }

    // // popup->manager.end_process();
    // FORNOW: horrifying workaround
    if (popup->manager.focus_group != ToolboxGroup::None) {
        Command check; {
            check = {};
            // FORNOW only checking the groups that make popups (Aug 14, 2024)
            if (popup->manager.focus_group == ToolboxGroup::Draw) {
                check = state.Draw_command;
            } else if (popup->manager.focus_group == ToolboxGroup::Mesh) {
                check = state.Mesh_command;
            } else if (popup->manager.focus_group == ToolboxGroup::Snap) {
                check = state.Snap_command;
            }
        }
        if (check.group == ToolboxGroup::None) {
            popup->manager.focus_group = ToolboxGroup::None;
        }
    }
    return result;
}

StandardEventProcessResult _standard_event_process_NOTE_RECURSIVE(Event event) {
    event_passed_to_popups = event;
    already_processed_event_passed_to_popups = false;


    void history_printf_script(); // FORNOW forward declaration

    bool skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack = event.snapshot_me;

    bool dxf_anything_selected; {
        dxf_anything_selected = false;
        _for_each_selected_entity_ {
            dxf_anything_selected = true;
            break;
        }
    }
    bool value_to_write_to_selection_mask = (state_Draw_command_is_(Select));


    StandardEventProcessResult result = {};

    Cookbook cookbook = make_Cookbook(event, &result, skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack);
    defer {
        cookbook.end_frame();
        cookbook_free(&cookbook);
    };


    bool is_toolbox_button_mouse_event =
        (event.type == EventType::Mouse)
        && (event.mouse_event.subtype == MouseEventSubtype::ToolboxButton);

    if ((event.type == EventType::Key) || is_toolbox_button_mouse_event) {
        KeyEvent *key_event = &event.key_event;
        if ((key_event->subtype == KeyEventSubtype::Hotkey) || is_toolbox_button_mouse_event) {
            result.record_me = true;

            *toolbox = {};

            real _w = 54.0f;
            real h = 12.0f;
            EasyTextPen Draw_pen,  Xsel_pen,  Snap_pen,  Colo_pen,  Both_pen,  Mesh_pen;
            EasyTextPen Draw_pen2, Xsel_pen2, Snap_pen2, Colo_pen2, Both_pen2, Mesh_pen2;
            {
                real padding = 4.0f;

                Draw_pen = { V2(padding, padding + 6), h, basic.white, true };
                Draw_pen2 = Draw_pen;
                Draw_pen2.color = basic.light_gray;

                Xsel_pen = Draw_pen;
                Xsel_pen2 = Draw_pen2;
                Xsel_pen.origin.x += (_w + padding);
                Xsel_pen2.origin = Xsel_pen.origin;

                Snap_pen = Xsel_pen;
                Snap_pen2 = Xsel_pen2;

                Colo_pen = Snap_pen;
                Colo_pen2 = Snap_pen2;
                if (command_equals(state.Xsel_command, commands.ByColor)) {
                    Colo_pen.origin.y += 92.0f;
                }
                Colo_pen2.origin = Colo_pen.origin;


                Both_pen = Draw_pen;
                Both_pen2 = Draw_pen2;
                Both_pen.origin.y += h + padding + h;
                Both_pen.origin.x = get_x_divider_drawing_mesh_Pixel() - (_w / 2);
                Both_pen2.origin = Both_pen.origin;

                Mesh_pen = Draw_pen;
                Mesh_pen2 = Draw_pen2;
                Mesh_pen.origin.x = window_get_width_Pixel() - (64 + padding);
                // Mesh_pen.origin.y += 64.0f;
                Mesh_pen2.origin = Mesh_pen.origin;
            }

            bool special_case_started_frame_with_snaps_enabled_NOTE_fixes_partial_snap_toolbox_graphical_glitch = (state.Draw_command.flags & SNAPPER);

            // TODO: GUIBUTTON should be able to take a hardcoded bbox
            //       (we still want the hot_name logic)
            // TODO: have the non-toggle-able buttons a different color (like before)
            Map<Shortcut, bool> shortcut_already_checked = {}; // TODO: arena
            defer { map_free_and_zero(&shortcut_already_checked); };
            bool hotkey_consumed_by_GUIBUTTON = false;
            ToolboxGroup most_recent_group_for_SEPERATOR = ToolboxGroup::None;
            auto GUIBUTTON = [&](
                    Command command,
                    bool hide_button = false,
                    bool deactivate_hotkey = false,
                    bool use_custom_button = false,
                    uint custom_button_id = 0
                    ) -> bool {
                most_recent_group_for_SEPERATOR = command.group;
                bool gray_out_shortcut;
                gray_out_shortcut = false; // SUPPRESS COMPILER WARNING
                if (!other._please_suppress_drawing_popup_popup) {
                    if (!deactivate_hotkey && !map_get(&shortcut_already_checked, command.shortcuts[0], false)) {
                        map_put(&shortcut_already_checked, command.shortcuts[0], true);
                        gray_out_shortcut = false;
                    } else {
                        gray_out_shortcut = true;
                    }
                }

                real w = _w;
                ToolboxGroup group = command.group;
                bool is_mode = command.is_mode;
                u64 flags = command.flags;
                String name = command.name;

                bool control = command.shortcuts[0].mods & GLFW_MOD_CONTROL;
                bool shift = command.shortcuts[0].mods & GLFW_MOD_SHIFT;
                bool alt = command.shortcuts[0].mods & GLFW_MOD_ALT;
                uint key = command.shortcuts[0].key;

                bool special_case_dont_draw_toolbox_NOTE_fixes_undo_graphical_glitch = (other._please_suppress_drawing_popup_popup && (group == ToolboxGroup::Snap));
                bool draw_tool = name.data;

                if (1
                        && (!other.hide_toolbox || (command_equals(command, commands.ToggleGUI)))
                        && (!other._please_suppress_drawing_popup_popup)
                        && (!hide_button)
                        && (group != ToolboxGroup::None)
                        && draw_tool
                        && (!special_case_dont_draw_toolbox_NOTE_fixes_undo_graphical_glitch)
                        && ((group != ToolboxGroup::Snap) || special_case_started_frame_with_snaps_enabled_NOTE_fixes_partial_snap_toolbox_graphical_glitch)
                   ) {
                    bbox2 bbox;
                    {
                        if (!use_custom_button) {
                            EasyTextPen *pen;
                            EasyTextPen *pen2;
                            bool horz = false;
                            if (group == ToolboxGroup::Draw) {
                                pen = &Draw_pen;
                                pen2 = &Draw_pen2;
                            } else if (group == ToolboxGroup::Xsel) {
                                pen = &Xsel_pen;
                                pen2 = &Xsel_pen2;
                            } else if (group == ToolboxGroup::Snap) {
                                pen = &Snap_pen;
                                pen2 = &Snap_pen2;
                                w = 64.0f;
                            } else if (group == ToolboxGroup::Colo) {
                                pen = &Colo_pen;
                                pen2 = &Colo_pen2;
                                w = 64.0f;
                            } else if (group == ToolboxGroup::Both) {
                                pen = &Both_pen;
                                pen2 = &Both_pen2;
                            } else { ASSERT(group == ToolboxGroup::Mesh);
                                pen = &Mesh_pen;
                                pen2 = &Mesh_pen2;
                                w = 64.0f;
                            }

                            real eps = 6.0;

                            real y = pen->get_y_Pixel();
                            bbox = { pen->origin.x, y - 2 - eps / 2, pen->origin.x + w, y + h + eps / 2 + 1 };


                            vec3 button_background = (group != ToolboxGroup::Mesh) ? pallete_2D->button_background : pallete_3D->button_background;
                            vec3 button_foreground = (group != ToolboxGroup::Mesh) ? pallete_2D->button_foreground : pallete_3D->button_foreground;
                            if (group == ToolboxGroup::Colo) {
                                for_(i, 10) if (command_equals(command, commands_Color[i])) {
                                    button_background = LERP(0.1f, button_background, get_color_from_color_code(i)); break;
                                }
                            }

                            vec3 color;
                            {
                                vec3 hover_color = LERP(0.1f, button_background, button_foreground);

                                bool can_toggle = is_mode;
                                bool toggled;
                                {
                                    toggled = false;
                                    if (can_toggle) {
                                        {
                                            toggled = false;
                                            if (0) {
                                            } else if (group == ToolboxGroup::Draw) {
                                                toggled = command_equals(state.Draw_command, command);
                                            } else if (group == ToolboxGroup::Snap) {
                                                toggled = command_equals(state.Snap_command, command);
                                            } else if (group == ToolboxGroup::Mesh) {
                                                toggled = command_equals(state.Mesh_command, command);
                                            } else if (group == ToolboxGroup::Colo) {
                                                toggled = command_equals(state.Colo_command, command);
                                            } else if (group == ToolboxGroup::Xsel) {
                                                toggled = command_equals(state.Xsel_command, command);
                                            }
                                        }
                                    }
                                }


                                if (group == ToolboxGroup::Colo) {
                                    if (group == ToolboxGroup::Colo) {
                                        for_(i, 10) if (command_equals(command, commands_Color[i])) {
                                            button_background = hover_color;
                                            hover_color = get_color_from_color_code(i);
                                            break;
                                        }
                                    }
                                }

                                color = (bbox_contains(bbox, other.mouse_Pixel)) ? ((other.mouse_left_drag_pane == Pane::Toolbox) ? LERP(0.2f, hover_color, button_foreground) : hover_color) : ((toggled) ? hover_color : button_background);

                                if (can_toggle) {
                                    eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
                                    eso_overlay(true);
                                    eso_color(color);
                                    eso_bbox_SOUP_QUADS(bbox);
                                    eso_end();
                                } else {
                                    eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
                                    eso_overlay(true);
                                    eso_color(color);
                                    real r = h / 2 + eps / 2;
                                    eso_bbox_SOUP_QUADS(bbox_inflate(bbox, { -r, 0.0f }));
                                    eso_bbox_SOUP_QUADS(bbox_inflate(bbox, { 0.0f, -r }));
                                    eso_end();
                                    eso_begin(other.OpenGL_from_Pixel, SOUP_POINTS);
                                    eso_size(2 * r);
                                    eso_color(color);
                                    eso_vertex(bbox.min + V2(r));
                                    eso_vertex(bbox.max - V2(r));
                                    eso_vertex(bbox.min.x + r, bbox.max.y - r);
                                    eso_vertex(bbox.max.x - r, bbox.min.y + r);
                                    eso_end();
                                }
                            }




                            vec3 tmp_pen_color = pen->color;
                            vec3 tmp_pen2_color = pen2->color;
                            {
                                pen->color = button_foreground;
                            }
                            {
                                if (gray_out_shortcut) {
                                    pen2->color = LERP(0.8f, pen->color, color);
                                } else {
                                    pen2->color = LERP(0.2f, pen->color, color);
                                }
                            }

                            KeyEvent tmp = { {}, key, control, shift, alt };
                            pen->offset_Pixel.x = 0.5f * (w - _easy_text_dx(pen, name));
                            // pen->offset_Pixel.x = 4;
                            pen->offset_Pixel.x = ROUND(pen->offset_Pixel.x);
                            {
                                // String fornow_hack = name;
                                // if (string_matches_prefix(name, "Clear")) fornow_hack.length = 5;
                                // if (string_matches_prefix(name, "Zoom")) fornow_hack.length = 4;
                                // if (!hovering) {
                                // easy_text_draw(pen, fornow_hack);

                                easy_text_draw(pen, name);
                                // } else {
                                // easy_text_drawf(pen, key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(&tmp));
                                // }
                            }
                            pen2->offset_Pixel.y = pen->offset_Pixel.y + eps;
                            pen2->offset_Pixel.x = 0.5f * (w - _easy_text_dx(pen2, key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(&tmp)));
                            pen2->offset_Pixel.x = ROUND(pen2->offset_Pixel.x);
                            // if (!gray_out_shortcut) {
                            //     easy_text_drawf(pen2, key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(&tmp));
                            // } else {
                            //     easy_text_drawf(pen2, "");
                            // }
                            //easy_text_drawf(pen2, key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(&tmp));
                            pen->color = tmp_pen_color;
                            pen2->color = tmp_pen2_color;

                            pen->offset_Pixel.y = pen2->offset_Pixel.y + 4;
                            if (horz) {
                                pen->offset_Pixel = {};
                                pen2->offset_Pixel = {};
                                pen->origin.x += w + 2;
                                pen2->origin.x += w + 2;
                            }
                        } else {
                            if (custom_button_id == 0 || custom_button_id == 1) { // sun moon light mode dark mode toggle light mode
                                int sign = (custom_button_id == 0) ? -1 : 1;
                                real tween = (custom_button_id == 0) ? pallete_2D->dark_light_tween : pallete_3D->dark_light_tween;
                                vec3 background = (custom_button_id == 0) ? pallete_2D->background : pallete_3D->background;
                                real x = get_x_divider_drawing_mesh_Pixel() + sign * 16.0f;
                                real y = 16.0f;
                                real half_bbox_side_length = 8.0f;
                                bbox = { x - half_bbox_side_length, y - half_bbox_side_length, x + half_bbox_side_length, y + half_bbox_side_length };
                                eso_begin(other.OpenGL_from_Pixel, SOUP_TRIANGLES);
                                eso_overlay(true);
                                for_(pass, 2) {
                                    eso_color((pass == 0) ?
                                            LERP(
                                                ((!bbox_contains(bbox, other.mouse_Pixel)) ? 0.0f : 0.3f),
                                                LERP(tween, basic.gray, 0.9f * basic.yellow),
                                                basic.white)
                                            : background);
                                    // eso_color(basic.yellow);
                                    uint N = 16;

                                    real radius = LERP(tween, 1.0f, 1.2f) *  half_bbox_side_length;

                                    vec2 center = { x, y };
                                    if (pass == 1) center.x = LERP(tween, x + sign * 0.5 * radius, x + sign * (radius + radius));
                                    for_(i, N) {
                                        real theta_i = TAU * i / N;
                                        real theta_ip1 = TAU * (i + 1) / N;
                                        // TODOLATER: check handedness of culling
                                        eso_vertex(center);
                                        eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta_ip1));
                                        eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta_i));
                                    }
                                }
                                eso_end();
                            } else if ((custom_button_id == 2) || (custom_button_id == 3)) { // details_2D details 2D dots

                                int sign = (custom_button_id == 2) ? -1 : 1;
                                bool show_details = (custom_button_id == 2) ? other.show_details_2D : other.show_details_3D;

                                real x = get_x_divider_drawing_mesh_Pixel() + sign * 2 * 16.0f;
                                real y = 16.0f;
                                real half_bbox_side_length = 8.0f;
                                bbox = { x - half_bbox_side_length, y - half_bbox_side_length, x + half_bbox_side_length, y + half_bbox_side_length };
                                eso_begin(other.OpenGL_from_Pixel, SOUP_POINTS);
                                eso_overlay(true);
                                eso_size((!show_details) ? 2.0f : 4.0f);
                                eso_color(basic.red);//(!other.show_details_2D) ? pallete_2D->button_background : pallete_2D->button_foreground);
                                eso_vertex(x, y);
                                eso_end();
                            } else {
                                ASSERT(false);
                            }
                        }
                    }
                    if (bbox_contains(bbox, other.mouse_Pixel)) {
                        *toolbox = {};
                        toolbox->hot_name = name.data;
                    }
                }

                // TODO: ScriptEvent
                // TODO: this should store whether we're hovering in toolbox as well as the event that will be generated if we click

                if (hotkey_consumed_by_GUIBUTTON) return false;

                bool inner_result; {
                    inner_result = false;
                    if (is_toolbox_button_mouse_event) {
                        inner_result |= (name.data == event.mouse_event.mouse_event_toolbox_button.name);
                    } else if (!deactivate_hotkey) {
                        for (Shortcut *shortcut = command.shortcuts; shortcut < command.shortcuts + COMMAND_MAX_NUM_SHORTCUTS; ++shortcut) {
                            bool tmp = _key_lambda(key_event, shortcut->key, (shortcut->mods & GLFW_MOD_CONTROL), (shortcut->mods & GLFW_MOD_SHIFT), (shortcut->mods & GLFW_MOD_ALT));
                            inner_result |= tmp;
                            hotkey_consumed_by_GUIBUTTON |= tmp;
                        }
                    }
                }

                // canned logic
                if (inner_result) {
                    if (flags & TWO_CLICK) {
                        ASSERT(is_mode);
                        two_click_command->awaiting_second_click = false;
                        two_click_command->tangent_first_click = false;

                        mesh_two_click_command->awaiting_second_click = false;

                        preview->mouse_from_Draw_Enter__BLUE_position = {}; // ohno NOTE:SEE001
                    }
                    if (flags & FOCUS_THIEF) {
                        popup->manager.manually_set_focus_group(group);
                    }
                    if (flags & NO_RECORD) {
                        result.record_me = false;
                    }
                    if (flags & HIDE_FEATURE_PLANE) {
                        if (feature_plane->is_active) other.time_since_plane_deselected = 0.0f;
                        other.should_feature_plane_be_active = feature_plane->is_active;
                        feature_plane->is_active = false;
                    }
                    if (is_mode) {
                        if (0) {
                        } else if (group == ToolboxGroup::Draw) {
                            state.Draw_command = command;
                            state.Snap_command = commands.None;
                            state.Xsel_command = commands.None;
                            state.Colo_command = commands.None;
                        } else if (group == ToolboxGroup::Snap) {
                            state.Snap_command = command;
                        } else if (group == ToolboxGroup::Mesh) {
                            state.Mesh_command = command;
                        } else if (group == ToolboxGroup::Xsel) {
                            state.Xsel_command = command;
                        } else if (group == ToolboxGroup::Colo) {
                        } else { ASSERT(0);
                        }
                    }
                }

                return inner_result;
            };

            auto SEPERATOR = [&]() {
                if (other.hide_toolbox) return;

                ToolboxGroup group = most_recent_group_for_SEPERATOR;
                ASSERT(group != ToolboxGroup::None);

                real eps = 4;
                if (group == ToolboxGroup::Draw) {
                    Draw_pen.offset_Pixel.y += eps;
                } else if (group == ToolboxGroup::Snap) {
                    MESSAGE_FAILURE("horrifying stuff with Snap pen origin/offset");
                } else if (group == ToolboxGroup::Mesh) {
                    Mesh_pen.offset_Pixel.y += eps;
                } else { ASSERT(group == ToolboxGroup::Both);
                    Both_pen.offset_Pixel.y += eps;
                }
            };

            Command prev_Draw_command = state.Draw_command;
            Command prev_Mesh_command = state.Mesh_command;
            { // GUIBUTTON
              // NOTE: ordered by priority

                { // Both
                    if (GUIBUTTON(commands.ToggleGUI)) { 
                        other.hide_toolbox = !other.hide_toolbox;
                    }

                    SEPERATOR();

                    { // Escape
                        if (GUIBUTTON(commands.Escape)) {
                            if (popup->manager.focus_group == ToolboxGroup::Draw) {
                                if (!state_Draw_command_is_(None)) {
                                    set_state_Draw_command(None);
                                    set_state_Snap_command(None);
                                    set_state_Colo_command(None);
                                } else {
                                    // Size, Load, Save...
                                    set_state_Mesh_command(None);
                                }
                            } else if (popup->manager.focus_group == ToolboxGroup::Mesh) {
                                set_state_Mesh_command(None);
                            } else if (popup->manager.focus_group == ToolboxGroup::Snap) {
                                set_state_Snap_command(None);
                            } else {
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);
                                set_state_Colo_command(None);
                                set_state_Mesh_command(None);
                                set_state_Xsel_command(None);
                            }
                        }
                    }

                    SEPERATOR();

                    { // undo
                        if (GUIBUTTON(commands.Undo)) {
                            other._please_suppress_drawing_popup_popup = true;
                            history_undo();
                        }
                    }

                    { // redo
                        bool button_redo = GUIBUTTON(commands.Redo);
                        // bool hotkey_redo_alternate = GUIBUTTON(commands.REDO_ALTERNATE);
                        // bool hotkey_redo_alternate_alternate = GUIBUTTON(commands.REDO_ALTERNATE_ALTERNATE);
                        if (button_redo) {
                            // _standard_event_process_NOTE_RECURSIVE({}); // FORNOW (prevent flicker on redo with nothing left to redo)
                            other._please_suppress_drawing_popup_popup = true;
                            history_redo();

                        }
                    }

                    SEPERATOR();

                    if (GUIBUTTON(commands.ToggleConsole)) other.show_console = !other.show_console;
                    if (GUIBUTTON(commands.ToggleFPS)) other.show_debug = !other.show_debug;
                    if (GUIBUTTON(commands.ToggleHistory)) other.show_history = !other.show_history;
                    if (GUIBUTTON(commands.ToggleLightMode2D, false, false, true, 0)) target_pallete->_2D = (target_pallete->_2D.id == PALLETE_2D_DARK) ?  _pallete_2D_light : _pallete_2D_dark;
                    if (GUIBUTTON(commands.ToggleLightMode3D, false, false, true, 1)) target_pallete->_3D = (target_pallete->_3D.id == PALLETE_3D_DARK) ?  _pallete_3D_light : _pallete_3D_dark;
                    if (GUIBUTTON(commands.ToggleDetails2D, false, false, true, 2)) other.show_details_2D = !other.show_details_2D;
                    if (GUIBUTTON(commands.ToggleDetails3D, false, false, true, 3)) other.show_details_3D = !other.show_details_3D;
                }

                { // Colo
                    { // OfSelection
                        if (state_Draw_command_is_(SetColor)) {
                            if (GUIBUTTON(commands.OfSelection)) { 
                                set_state_Colo_command(OfSelection);
                            }
                        }
                    }
                    { // ColorX
                        bool hide_buttons = !(0
                                || state_Draw_command_is_(SetColor)
                                || ((SELECT_OR_DESELECT()) && (state_Xsel_command_is_(ByColor)))
                                );
                        bool spoof_is_mode_false = 0
                            || (state_Draw_command_is_(SetColor) && state_Colo_command_is_(OfSelection))
                            || (SELECT_OR_DESELECT() && state_Xsel_command_is_(ByColor))
                            ;
                        if (true) {
                            bool hotkey_quality;
                            u8 digit = 0;
                            {
                                hotkey_quality = false;
                                for (u8 color = 0; color < 10; ++color) {
                                    Command command = commands_Color[color];
                                    if (spoof_is_mode_false) command.is_mode = false;
                                    if (GUIBUTTON(command, hide_buttons)) {
                                        hotkey_quality = true;
                                        digit = color;
                                        break;
                                    }
                                }
                            }

                            if (hotkey_quality) {
                                if (SELECT_OR_DESELECT() && (state_Xsel_command_is_(ByColor))) { // [sd]q0
                                    _for_each_entity_ {
                                        uint i = uint(entity->color_code);
                                        if (i != digit) continue;
                                        cookbook.entity_set_is_selected(entity, value_to_write_to_selection_mask);
                                    }
                                    set_state_Draw_command(None);
                                    set_state_Xsel_command(None);
                                } else if ((state_Draw_command_is_(SetColor)) && (state_Colo_command_is_(OfSelection))) { // qs0
                                    _for_each_selected_entity_ cookbook.entity_set_color(entity, digit);
                                    set_state_Draw_command(None);
                                    set_state_Colo_command(None);
                                    _for_each_entity_ entity->is_selected = false;
                                } else { // 0
                                    set_state_Draw_command(SetColor);
                                    state.Colo_command = commands_Color[digit];
                                    set_state_Snap_command(None);
                                }
                            }
                        }
                    }
                }

                { // Xsel
                    if (SELECT_OR_DESELECT()) {
                        if (GUIBUTTON(commands.All)) { 
                            result.checkpoint_me = true;
                            cookbook.set_is_selected_for_all_entities(state_Draw_command_is_(Select));
                            set_state_Draw_command(None);
                            set_state_Xsel_command(None);
                        }
                        GUIBUTTON(commands.Connected);
                        GUIBUTTON(commands.Window);
                        bool deactive_sq = !command_equals(state.Xsel_command, commands.None);
                        GUIBUTTON(commands.ByColor, false, deactive_sq);
                    }
                }

                { // Snap 
                    if (state.Draw_command.flags & SNAPPER) {

                        bool snap_button_other_than_XY_or_Zero_pressed = 0
                            | GUIBUTTON(commands.Center)
                            | GUIBUTTON(commands.End)
                            | GUIBUTTON(commands.Intersect)
                            | GUIBUTTON(commands.Middle)
                            | GUIBUTTON(commands.Perp)
                            | GUIBUTTON(commands.Quad);

                        if (snap_button_other_than_XY_or_Zero_pressed) {
                            preview->mouse_transformed__PINK_position = preview->mouse_no_snap_potentially_15_deg__WHITE_position; // FORNOW
                        }

                        if (GUIBUTTON(commands.XY)) {
                            // this is the only good code in the codebase

                            // ohno NOTE:SEE001
                            preview->xy_xy = (!two_click_command->awaiting_second_click) ? V2(0) : preview->mouse_no_snap_potentially_15_deg__WHITE_position;
                        }
                        if (GUIBUTTON(commands.Zero)) {
                            state.Snap_command = commands.None;

                            Event equivalent = {};
                            equivalent.type = EventType::Mouse;
                            equivalent.mouse_event.subtype = MouseEventSubtype::Drawing;
                            return _standard_event_process_NOTE_RECURSIVE(equivalent);
                        }
                        if (!command_equals(state.Snap_command, commands.None)) {
                            if (GUIBUTTON(commands.ClearSnap)) {
                                state.Snap_command = commands.None;
                            }
                        }
                    }
                }

                { // Draw
                    GUIBUTTON(commands.Select);
                    GUIBUTTON(commands.Deselect);
                    SEPERATOR();
                    if (GUIBUTTON(commands.Delete)) {
                        // trust me you want this code (imagine deleting stuff while in the middle of a two click command)
                        set_state_Draw_command(None);
                        set_state_Snap_command(None);
                        set_state_Xsel_command(None);
                        set_state_Colo_command(None);

                        bool checkpoint = false;
                        for (int i = drawing->entities.length - 1; i >= 0; --i) {
                            if (drawing->entities.array[i].is_selected) {
                                cookbook._delete_entity(i);
                                checkpoint = true;
                            }
                        }
                        result.checkpoint_me = checkpoint;
                    }
                    SEPERATOR();
                    if (GUIBUTTON(commands.SetColor)) set_state_Colo_command(Color0);
                    SEPERATOR();
                    GUIBUTTON(commands.Line);
                    GUIBUTTON(commands.Circle);
                    GUIBUTTON(commands.Box);
                    if (GUIBUTTON(commands.Polygon)) preview->polygon_num_sides = popup->polygon_num_sides;
                    SEPERATOR();
                    GUIBUTTON(commands.Measure);
                    SEPERATOR();
                    GUIBUTTON(commands.Translate);
                    // GUIBUTTON(commands.Drag);
                    GUIBUTTON(commands.Rotate);
                    if (GUIBUTTON(commands.Scale)) {
                        preview->scale_factor = 1.0f;
                    }
                    SEPERATOR();
                    GUIBUTTON(commands.LCopy);
                    if (GUIBUTTON(commands.RCopy)) {
                        preview->rcopy_last_angle = 0;
                    }
                    SEPERATOR();
                    GUIBUTTON(commands.XMirror);
                    GUIBUTTON(commands.YMirror);
                    // GUIBUTTON(commands.Mirror2);
                    SEPERATOR();
                    GUIBUTTON(commands.Divide2);
                    GUIBUTTON(commands.Join2);
                    SEPERATOR();
                    // GUIBUTTON(commands.ElfHat);
                    GUIBUTTON(commands.Fillet);
                    GUIBUTTON(commands.DogEar);
                    GUIBUTTON(commands.Offset);
                    SEPERATOR();
                    GUIBUTTON(commands.SetOrigin);
                    GUIBUTTON(commands.SetAxis);
                    SEPERATOR();
                    if (GUIBUTTON(commands.Clear2D)) {
                        result.checkpoint_me = true;
                        // result.snapshot_me = true;
                        list_free_AND_zero(&drawing->entities);
                        *drawing = {};
                        MESSAGE_SUCCESS("Clear2D");
                    }
                    if (GUIBUTTON(commands.Zoom2D)) {
                        init_camera_drawing();
                    }
                    SEPERATOR();
                    GUIBUTTON(commands.OpenDXF);
                    GUIBUTTON(commands.SaveDXF);
                    SEPERATOR();
                    SEPERATOR();
                    SEPERATOR();
                    if (GUIBUTTON(commands.PowerFillet)) {
                        set_state_Draw_command(PowerFillet);
                        set_state_Snap_command(None);
                        set_state_Mesh_command(None);
                    }
                }

                { // Mesh
                    if (GUIBUTTON(commands.ExtrudeAdd)) {
                        preview->extrude_in_length = 0; // FORNOW
                        preview->extrude_out_length = 0; // FORNOW
                    }
                    if (GUIBUTTON(commands.ExtrudeCut)) {
                        preview->extrude_in_length = 0; // FORNOW
                        preview->extrude_out_length = 0; // FORNOW
                    }
                    SEPERATOR();
                    if (GUIBUTTON(commands.RevolveAdd)) {
                        preview->revolve_in_angle = 0; // FORNOW
                        preview->revolve_out_angle = 0; // FORNOW
                    }
                    if (GUIBUTTON(commands.RevolveCut)) {
                        preview->revolve_in_angle = 0; // FORNOW
                        preview->revolve_out_angle = 0; // FORNOW
                    }
                    SEPERATOR();
                    if (GUIBUTTON(commands.CyclePlane)) {
                        // TODO: 'Y' remembers last terminal choice of plane for next time
                        result.checkpoint_me = true;
                        other.time_since_plane_selected = 0.0f;

                        // already one of the three primary planes
                        if ((feature_plane->is_active) && ARE_EQUAL(feature_plane->signed_distance_to_world_origin, 0.0f) && ARE_EQUAL(squaredNorm(feature_plane->normal), 1.0f) && ARE_EQUAL(maxComponent(feature_plane->normal), 1.0f)) {
                            feature_plane->normal = { feature_plane->normal[2], feature_plane->normal[0], feature_plane->normal[1] };
                        } else {
                            feature_plane->is_active = true;
                            feature_plane->signed_distance_to_world_origin = 0.0f;
                            preview->feature_plane_signed_distance_to_world_origin = 0.0f;
                            feature_plane->normal = { 0.0f, 1.0f, 0.0f };
                            feature_plane->rotation_angle = 0.0f;
                        }
                    }
                    if (GUIBUTTON(commands.NudgePlane)) {
                        if (!feature_plane->is_active) {
                            MESSAGE_FAILURE("NudgePlane: no feature plane selected");
                            set_state_Mesh_command(None); // FORNOW
                        }
                    }
                    if (GUIBUTTON(commands.TogglePlane)) {
                        feature_plane->is_active = !feature_plane->is_active;
                        if (feature_plane->is_active) other.time_since_plane_selected = 0.0f;
                        else other.time_since_plane_deselected = 0.0f;
                    }
                    if (GUIBUTTON(commands.MirrorPlaneX)) {
                        if (feature_plane->is_active) {
                            result.checkpoint_me = true;
                            feature_plane->mirror_x = !feature_plane->mirror_x;
                        } else {
                            MESSAGE_FAILURE("MirrorPlaneX: no feature plane selected");
                        }
                    }
                    if (GUIBUTTON(commands.MirrorPlaneY)) {
                        if (feature_plane->is_active) {
                            result.checkpoint_me = true;
                            feature_plane->mirror_y = !feature_plane->mirror_y;
                        } else {
                            MESSAGE_FAILURE("MirrorPlaneY: no feature plane selected");
                        }
                    }
                    if (GUIBUTTON(commands.RotatePlane)) {
                        if (!feature_plane->is_active) {
                            MESSAGE_FAILURE("RotatePlane: no feature plane selected");
                            set_state_Mesh_command(None); // FORNOW
                        }
                    }
                    SEPERATOR();
                    if (GUIBUTTON(commands.Clear3D)) {
                        result.checkpoint_me = true;
                        result.snapshot_me = true;
                        meshes_free_AND_zero(meshes);
                        *feature_plane = {};
                        MESSAGE_SUCCESS("Clear3D");
                    }
                    if (GUIBUTTON(commands.ZoomMesh)) {
                        init_camera_mesh();
                    }
                    if (GUIBUTTON(commands.ZoomPlane)) {
                        init_camera_mesh();
                        real x = feature_plane->normal.x;
                        real y = feature_plane->normal.y;
                        real z = feature_plane->normal.z;
                        camera_mesh->euler_angles.y = ATAN2({z, x});
                        camera_mesh->euler_angles.x = -ATAN2({norm(V2(z, x)), y});
                    }
                    SEPERATOR();
                    GUIBUTTON(commands.Measure3D);
                    GUIBUTTON(commands.SetOrigin3D);
                    SEPERATOR();
                    SEPERATOR();
                    GUIBUTTON(commands.OpenSTL);
                    GUIBUTTON(commands.SaveSTL);
                }



                ////////////////////////////////////////////////////////////////////////////////

                #ifndef SHIP
                { // Secret Commands
                    if (GUIBUTTON(commands.PREVIOUS_HOT_KEY_2D)) {
                        set_state_Draw_command(None); // FORNOW: patching space space doing Circle Center
                        return _standard_event_process_NOTE_RECURSIVE(state.space_bar_event);
                    }
                    // if (GUIBUTTON(commands.OrthoCamera)) {
                    //     other.camera_mesh.angle_of_view = CAMERA_3D_PERSPECTIVE_ANGLE_OF_VIEW - other.camera_mesh.angle_of_view;
                    // }

                    // if (GUIBUTTON(commands.DivideNearest)) {
                    //     set_state_Draw_command(DivideNearest);
                    //     set_state_Snap_command(None);

                    // }



                    if (GUIBUTTON(commands.ToggleGrid)) {
                        other.hide_grid = !other.hide_grid;

                    }





                    if (GUIBUTTON(commands.PREVIOUS_HOT_KEY_3D)) {

                        return _standard_event_process_NOTE_RECURSIVE(state.shift_space_bar_event);
                    }


                }
                #endif

                if (key_event->subtype == KeyEventSubtype::Hotkey) { // (not button)
                    if (!hotkey_consumed_by_GUIBUTTON) {
                        result.record_me = false;
                    }
                    if (1
                            && !hotkey_consumed_by_GUIBUTTON
                            && (key_event->key != DUMMY_HOTKEY)
                            && (key_event->key != 0)
                            && (key_event->key != 0)
                       ) {
                        MESSAGE_FAILURE("Hotkey: %s not recognized", key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(key_event), key_event->control, key_event->shift, key_event->alt, key_event->key);
                    }
                }
            }
            bool changed_click_mode = (!command_equals(prev_Draw_command, state.Draw_command));
            bool changed_enter_mode = (!command_equals(prev_Mesh_command, state.Mesh_command));
            if (changed_click_mode && click_mode_SPACE_BAR_REPEAT_ELIGIBLE()) state.space_bar_event = event;
            if (changed_enter_mode && enter_mode_SHIFT_SPACE_BAR_REPEAT_ELIGIBLE()) state.shift_space_bar_event = event;
        } else { ASSERT(key_event->subtype == KeyEventSubtype::Popup);
            // NOTE: this case has been moved into POPUP();
            result.record_me = true; // FORNOW
        }
    } else if (event.type == EventType::Mouse) {
        MouseEvent *mouse_event = &event.mouse_event;
        if (mouse_event->subtype == MouseEventSubtype::Drawing) {
            MouseEventDrawing *mouse_event_drawing = &mouse_event->mouse_event_drawing;

            result.record_me = true;
            if (state_Draw_command_is_(Measure)) result.record_me = false;
            if (mouse_event->mouse_held) result.record_me = false;

            TransformMouseDrawingPositionResult transform_mouse_drawing_position_result = transform_mouse_drawing_position(mouse_event_drawing->unsnapped_position, mouse_event_drawing->shift_held, false);
            vec2 *mouse_transformed_position = &transform_mouse_drawing_position_result.mouse_position;


            // TODO: commands.cpp flag
            if (transform_mouse_drawing_position_result.snapped
                    && ( 
                        0
                        || (state_Draw_command_is_(Box))
                        || (state_Draw_command_is_(Circle))
                        || (state_Draw_command_is_(Line))
                        || (state_Draw_command_is_(Polygon))
                       )
               ) {
                ASSERT(transform_mouse_drawing_position_result.entity_index_snapped_to >= 0);
                ASSERT(transform_mouse_drawing_position_result.entity_index_snapped_to < drawing->entities.length);
                cookbook.attempt_divide_entity_at_point(transform_mouse_drawing_position_result.entity_index_snapped_to, *mouse_transformed_position);
                other.snap_divide_dot = *mouse_transformed_position;
                other.size_snap_divide_dot = 7.0f;
            }


            // fornow window wonky case
            if (_non_WINDOW__SELECT_DESELECT___OR___SET_COLOR()) { // NOTES: includes sc and qc
                result.record_me = false;
                DXFFindClosestEntityResult dxf_find_closest_entity_result = dxf_find_closest_entity(&drawing->entities, transform_mouse_drawing_position_result.mouse_position);
                if (dxf_find_closest_entity_result.success) {
                    Entity *hot_entity = dxf_find_closest_entity_result.closest_entity;
                    if (!state_Xsel_command_is_(Connected) && !state_Colo_command_is_(OfSelection)) {
                        if (SELECT_OR_DESELECT()) {
                            cookbook.entity_set_is_selected(hot_entity, value_to_write_to_selection_mask);
                        } else {
                            bool found = false;
                            for_(digit, 10) {
                                if (command_equals(state.Colo_command, commands_Color[digit])) {
                                    cookbook.entity_set_color(hot_entity, digit);
                                    found = true;
                                    break;
                                }
                            }
                            ASSERT(found);
                        }
                    } else {
                        #define GRID_CELL_WIDTH 0.003f

                        auto scalar_bucket = [&](real a) -> real {
                            real ret = roundf(a / GRID_CELL_WIDTH) * GRID_CELL_WIDTH;
                            return ret == -0 ? 0 : ret; // what a fun bug
                        };

                        auto make_key = [&](vec2 p) -> vec2 {
                            return { scalar_bucket(p.x), scalar_bucket(p.y) };
                        };

                        auto nudge_key = [&](vec2 key, int dx, int dy) -> vec2 {
                            return make_key(V2(key.x + dx * GRID_CELL_WIDTH, key.y + dy * GRID_CELL_WIDTH));
                        };

                        struct GridPointSlot {
                            bool populated;
                            int entity_index;
                            bool end_NOT_start;
                        };

                        struct GridCell {
                            GridPointSlot slots[5];
                        };

                        Map<vec2, GridCell> grid; { // TODO: build grid
                            grid = {};

                            auto push_into_grid_unless_cell_full__make_cell_if_none_exists = [&](vec2 p, uint entity_index, bool end_NOT_start) {
                                vec2 key = make_key(p);
                                GridCell *cell = _map_get_pointer(&grid, key);
                                if (cell == NULL) {
                                    map_put(&grid, key, {});
                                    cell = _map_get_pointer(&grid, key);
                                }
                                for_(i, ARRAY_LENGTH(cell->slots)) {
                                    GridPointSlot *slot = &cell->slots[i];
                                    if (slot->populated) continue;
                                    slot->populated = true;
                                    slot->entity_index = entity_index;
                                    slot->end_NOT_start = end_NOT_start;
                                    // printf("%f %f [%d]\n", key.x, key.y, i);
                                    break;
                                }
                            };

                            for_(entity_index, drawing->entities.length) {
                                Entity *entity = &drawing->entities.array[entity_index];

                                vec2 start;
                                vec2 end;
                                bool poosh = false;
                                if (entity->type == EntityType::Circle) {
                                    CircleEntity *circle = &entity->circle;
                                    if (circle->has_pseudo_point) {
                                        poosh = true;
                                        start = end = circle->get_pseudo_point();
                                    }
                                } else {
                                    poosh = true;
                                    entity_get_start_and_end_points(entity, &start, &end);
                                }
                                if (poosh) {
                                    push_into_grid_unless_cell_full__make_cell_if_none_exists(start, entity_index, false);
                                    push_into_grid_unless_cell_full__make_cell_if_none_exists(end, entity_index, true);
                                }
                            }
                        }

                        bool *edge_marked = (bool *) calloc(drawing->entities.length, sizeof(bool));

                        ////////////////////////////////////////////////////////////////////////////////
                        // NOTE: We are now done adding to the grid, so we can now operate directly on GridCell *'s
                        //       We will use _map_get_pointer(...)
                        ////////////////////////////////////////////////////////////////////////////////


                        auto get_key = [&](GridPointSlot *point, bool other_endpoint) {
                            bool end_NOT_start; {
                                end_NOT_start = point->end_NOT_start;
                                if (other_endpoint) end_NOT_start = !end_NOT_start;
                            }
                            vec2 p; {
                                Entity *entity = &drawing->entities.array[point->entity_index];
                                if (entity->type == EntityType::Circle) {
                                    CircleEntity *circle = &entity->circle;
                                    ASSERT(circle->has_pseudo_point);
                                    p = circle->get_pseudo_point();
                                } else {
                                    if (end_NOT_start) {
                                        p = entity_get_end_point(entity);
                                    } else {
                                        p = entity_get_start_point(entity);
                                    }
                                }
                            }
                            return make_key(p);
                        };

                        auto get_any_point_not_part_of_an_marked_entity = [&](vec2 key) -> GridPointSlot * {
                            GridCell *cell = _map_get_pointer(&grid, key);
                            if (!cell) return NULL;

                            for_(i, ARRAY_LENGTH(cell->slots)) {
                                GridPointSlot *slot = &cell->slots[i];
                                if (!slot->populated) continue;
                                if (edge_marked[slot->entity_index]) continue;
                                return slot;
                            }
                            return NULL;
                        };


                        uint hot_entity_index = uint(hot_entity - drawing->entities.array);

                        // NOTE: we will mark the hot entity, and then shoot off from both its endpoints
                        edge_marked[hot_entity_index] = true;
                        cookbook.entity_set_is_selected(&drawing->entities.array[hot_entity_index], value_to_write_to_selection_mask);

                        Entity *entity = &drawing->entities.array[hot_entity_index]; // FORNOW: this is a scary name to give (just doing it out of laziness atm Jim Sep 27 2024)

                        // this should be moved earlier in the code especially once this crap is turned into a function/its own file
                        bool special_case_circle_no_pseudo_point = ((entity->type == EntityType::Circle) && (!entity->circle.has_pseudo_point));
                        if (special_case_circle_no_pseudo_point) {
                            entity->is_selected = value_to_write_to_selection_mask;
                        } else {
                            for_(pass, 2) {
                                vec2 seed; {
                                    vec2 p;
                                    if (entity->type == EntityType::Circle) {
                                        CircleEntity *circle = &entity->circle;
                                        ASSERT(circle->has_pseudo_point);
                                        p = circle->get_pseudo_point();
                                    } else {
                                        if (pass == 0) {
                                            p = entity_get_start_point(entity);
                                        } else {
                                            p = entity_get_end_point(entity);
                                        }
                                    }
                                    seed = make_key(p);
                                }

                                Queue<vec2> queue = {};
                                queue_enqueue(&queue, seed);

                                while (queue.length) {
                                    seed = queue_dequeue(&queue);
                                    for (int dx = -1; dx <= 1; ++dx) {
                                        for (int dy = -1; dy <= 1; ++dy) {
                                            while (1) {
                                                GridPointSlot *tmp = get_any_point_not_part_of_an_marked_entity(nudge_key(seed, dx, dy));

                                                if (!tmp) break;

                                                cookbook.entity_set_is_selected(&drawing->entities.array[tmp->entity_index], value_to_write_to_selection_mask);
                                                GridPointSlot *nullCheck = get_any_point_not_part_of_an_marked_entity(get_key(tmp, true));
                                                if (nullCheck) queue_enqueue(&queue, get_key(nullCheck, false)); // get other end);
                                                edge_marked[tmp->entity_index] = true;
                                            } 
                                        }
                                    }
                                }

                                queue_free_AND_zero(&queue);
                            }
                        }




                        map_free_and_zero(&grid);
                        free(edge_marked);

                        /*uint loop_index = dxf_pick_loops.loop_index_from_entity_index[hot_entity_index];
                          DXFEntityIndexAndFlipFlag *loop = dxf_pick_loops.loops[loop_index];
                          uint num_entities = dxf_pick_loops.num_entities_in_loops[loop_index];
                          for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < &loop[num_entities]; ++entity_index_and_flip_flag) {
                          cookbook.entity_set_is_selected(&drawing->entities[entity_index_and_flip_flag->entity_index], value_to_write_to_selection_mask);
                          }*/
                    }
                }
            } else if (!mouse_event->mouse_held) {
                if ((state.Draw_command.flags | state.Xsel_command.flags) & TWO_CLICK) { // FORNOW
                    if (!two_click_command->awaiting_second_click) {
                        DXFFindClosestEntityResult find_nearest_result = dxf_find_closest_entity(&drawing->entities, transform_mouse_drawing_position_result.mouse_position);
                        bool first_click_accepted; {
                            if (!first_click_must_acquire_entity()) {
                                first_click_accepted = true;
                            } else {
                                first_click_accepted = find_nearest_result.success;
                            }
                        }
                        if (first_click_accepted) {
                            two_click_command->awaiting_second_click = true;
                            two_click_command->first_click = transform_mouse_drawing_position_result.mouse_position;
                            // if (!two_click_command->tangent_first_click) { // ???
                            two_click_command->entity_closest_to_first_click = find_nearest_result.closest_entity;
                            // }
                            set_state_Snap_command(None);
                            if (!other._please_suppress_drawing_popup_popup) { // bump bumps cursor bump cursor bumps
                                if (state_Draw_command_is_(Rotate)) {
                                    double xpos, ypos;
                                    glfwGetCursorPos(glfw_window, &xpos, &ypos);
                                    real x_new = real(xpos) + 64.0f;
                                    real y_new = real(ypos);
                                    glfwSetCursorPos(glfw_window, x_new, y_new);
                                    callback_cursor_position(glfw_window, x_new, y_new);
                                }
                                if (state_Draw_command_is_(SetAxis)) {
                                    double xpos, ypos;
                                    glfwGetCursorPos(glfw_window, &xpos, &ypos);
                                    real theta = (PI / 2) + drawing->axis_angle_from_y;
                                    real r = 64.0f;
                                    real x_new = real(xpos) + r * COS(theta);
                                    real y_new = real(ypos) - r * SIN(theta);
                                    glfwSetCursorPos(glfw_window, x_new, y_new);
                                    callback_cursor_position(glfw_window, x_new, y_new);
                                }
                            }

                            // FORNOW FORNOW FORNOW DUPLICATNG THIS CODE FROM PROCESS BAD BAD VERY BAD
                            vec2 Draw_Enter; {
                                do_once { MESSAGE_FAILURE("FORNOW FORNOW FORNOW DUPLICATNG THIS CODE FROM PROCESS BAD BAD VERY BAD"); };
                                Draw_Enter = (!two_click_command->awaiting_second_click) ? V2(0, 0) : two_click_command->first_click;
                                // if (popup->manager.focus_group == ToolboxGroup::Draw) {
                                // if (state_Draw_command_is_(Box)) Draw_Enter += V2(popup->box_width, popup->box_height);
                                // if (state_Draw_command_is_(Circle)) Draw_Enter += V2(popup->circle_radius, 0.0f);
                                // if (state_Draw_command_is_(Polygon)) Draw_Enter += V2(popup->polygon_distance_to_corner, 0.0f);
                                // if (state_Draw_command_is_(Line)) Draw_Enter += V2(popup->line_run, popup->line_rise);
                                // if (state_Draw_command_is_(Translate)) Draw_Enter += V2(popup->move_run, popup->move_rise);
                                // if (state_Draw_command_is_(LCopy)) Draw_Enter += V2(popup->lcopy_run, popup->lcopy_rise);
                                // if (state_Draw_command_is_(Rotate)) Draw_Enter += 10.0f * e_theta(RAD(popup->rotate_angle));
                                // }
                            }

                            preview->mouse_from_Draw_Enter__BLUE_position = Draw_Enter;
                            preview->xy_xy = two_click_command->first_click;
                        }
                    } else { // (two_click_command->awaiting_second_click)
                        vec2 first_click = two_click_command->first_click;
                        vec2 second_click = *mouse_transformed_position;
                        vec2 click_vector = (second_click - first_click);
                        vec2 average_click = AVG(first_click, second_click);
                        real click_theta = angle_from_0_TAU(first_click, second_click);
                        real clicks_are_same = IS_ZERO(click_vector);
                        real length_click_vector = norm(click_vector);

                        DXFFindClosestEntityResult second_ent_search = dxf_find_closest_entity(&drawing->entities, second_click);
                        two_click_command->entity_closest_to_second_click = second_ent_search.success ? second_ent_search.closest_entity : NULL;

                        if (0) {
                        } else if (state_Draw_command_is_(SetAxis)) {
                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);
                            drawing->axis_base_point = first_click;
                            drawing->axis_angle_from_y = (-PI / 2) + click_theta;
                        } else if (state_Draw_command_is_(Box)) {
                            if (IS_ZERO(ABS(first_click.x - second_click.x))) {
                                MESSAGE_FAILURE("Box: must have non-zero width");
                            } else if (IS_ZERO(ABS(first_click.y - second_click.y))) {
                                MESSAGE_FAILURE("Box: must have non-zero height");
                            } else {
                                result.checkpoint_me = true;
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);
                                vec2 other_corner_A = { first_click.x, second_click.y };
                                vec2 other_corner_B = { second_click.x, first_click.y };
                                cookbook.buffer_add_line(first_click,  other_corner_A);
                                cookbook.buffer_add_line(first_click,  other_corner_B);
                                cookbook.buffer_add_line(second_click, other_corner_A);
                                cookbook.buffer_add_line(second_click, other_corner_B);
                            }
                        } else if (state_Draw_command_is_(Fillet)) {
                            if (two_click_command->entity_closest_to_second_click) {
                                Entity *E = two_click_command->entity_closest_to_first_click;
                                Entity *F = two_click_command->entity_closest_to_second_click;
                                FilletResult fillet_result = preview_fillet(E, F, average_click, popup->fillet_radius);
                                if (fillet_result.fillet_success) {

                                    result.checkpoint_me = true;
                                    set_state_Snap_command(None);
                                    two_click_command->awaiting_second_click = false;

                                    cookbook._buffer_add_entity(fillet_result.ent_one);
                                    cookbook._buffer_add_entity(fillet_result.ent_two);
                                    if (!IS_ZERO(popup->fillet_radius)) {
                                        cookbook._buffer_add_entity(fillet_result.fillet_arc);
                                    }
                                    cookbook.buffer_delete_entity(E);
                                    cookbook.buffer_delete_entity(F);
                                }
                            }
                        } else if (state_Draw_command_is_(DogEar)) {
                            result.checkpoint_me = true;

                            set_state_Snap_command(None);
                            if (two_click_command->entity_closest_to_second_click) {
                                DogEarResult dogear_result = preview_dogear(
                                        two_click_command->entity_closest_to_first_click, 
                                        two_click_command->entity_closest_to_second_click, 
                                        average_click, 
                                        popup->dogear_radius);
                                two_click_command->awaiting_second_click = false;

                                cookbook._buffer_add_entity(dogear_result.ent_one);
                                cookbook._buffer_add_entity(dogear_result.ent_two);
                                cookbook._buffer_add_entity(dogear_result.fillet_arc_one);
                                cookbook._buffer_add_entity(dogear_result.fillet_arc_two);
                                cookbook._buffer_add_entity(dogear_result.dogear_arc);

                                cookbook.buffer_delete_entity(two_click_command->entity_closest_to_first_click);
                                cookbook.buffer_delete_entity(two_click_command->entity_closest_to_second_click);
                            }
                        } else if (state_Draw_command_is_(Circle)) {
                            if (clicks_are_same) {
                                MESSAGE_FAILURE("Circle: must have non-zero diameter");
                            } else {
                                result.checkpoint_me = true;
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);
                                real r = length_click_vector;

                                #if 0
                                real theta_a_in_degrees = DEG(click_theta);
                                real theta_b_in_degrees = theta_a_in_degrees + 180.0f;
                                cookbook.buffer_add_arc(first_click, r, theta_a_in_degrees, theta_b_in_degrees);
                                cookbook.buffer_add_arc(first_click, r, theta_b_in_degrees, theta_a_in_degrees);
                                #else
                                cookbook.buffer_add_circle(first_click, r, false, {});
                                #endif
                            }
                        } else if (state_Draw_command_is_(Divide2)) { 

                            Entity *closest_entity_one = two_click_command->entity_closest_to_first_click; 
                            if (two_click_command->entity_closest_to_second_click) {
                                Entity *closest_entity_two = two_click_command->entity_closest_to_second_click;
                                if (closest_entity_one == closest_entity_two) {
                                    MESSAGE_FAILURE("TwoClickDivide: clicked same entity twice");
                                } else {
                                    ClosestIntersectionResult intersection_result = closest_intersection(closest_entity_one, closest_entity_two, second_click);

                                    bool other_condition = (
                                            intersection_result.point_is_on_entity_b
                                            // || intersection_result.point_is_on_entity_a
                                            );

                                    bool success = false;
                                    if (!intersection_result.no_possible_intersection && other_condition) {
                                        bool divided_first = cookbook.attempt_divide_entity_at_point(closest_entity_one, intersection_result.point);
                                        bool divided_second = cookbook.attempt_divide_entity_at_point(closest_entity_two, intersection_result.point);
                                        if (divided_first || divided_second) {
                                            result.checkpoint_me = true;
                                            set_state_Draw_command(None);
                                            set_state_Snap_command(None);
                                        }
                                    }
                                    if (!success) {
                                        MESSAGE_FAILURE("TwoClickDivide: no intersection found");
                                    } else {
                                        MESSAGE_SUCCESS("TwoClickDivide");
                                    }
                                }
                            }
                        } else if (state_Draw_command_is_(Join2)) {
                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);

                            // Entity *closest_entity_one = two_click_command->entity_closest_to_first_click; 
                        } else if (state_Draw_command_is_(Line)) {
                            if (clicks_are_same) {
                                MESSAGE_FAILURE("Line: must have non-zero length");
                            } else {
                                result.checkpoint_me = true;
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);
                                cookbook.buffer_add_line(first_click, second_click);
                            }
                        } else if (state_Draw_command_is_(Measure)) {
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);
                            real angle = DEG(click_theta);
                            real length = length_click_vector;
                            MESSAGE_FAILURE("Angle is %gdeg.", angle);
                            MESSAGE_FAILURE("Length is %gmm.", length);

                            MESSAGE_INFO("EXPERIMENTAL: Measure copies into field.");
                            _POPUP_MEASURE_HOOK(length);
                        } else if (state_Draw_command_is_(Translate)) {
                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);
                            _for_each_selected_entity_ {
                                *entity = entity_translated(entity, click_vector);
                            }
                        } else if (state_Draw_command_is_(Rotate)) {
                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);
                            _for_each_selected_entity_ {
                                *entity = entity_rotated(entity, first_click, click_theta);
                            }
                        } else if (state_Draw_command_is_(Scale)) {
                            if (IS_ZERO(popup->scale_factor)) {
                                MESSAGE_FAILURE("Scale: must have non-zero factor");
                            } else {
                                result.checkpoint_me = true;
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);

                                _for_each_selected_entity_ {
                                    *entity = entity_scaled(entity, first_click, popup->scale_factor);
                                }
                            }
                        } else if (state_Draw_command_is_(LCopy)) {
                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);
                            uint num_additional_copies = MAX(1U, popup->lcopy_num_additional_copies);
                            for_(i, num_additional_copies) {
                                vec2 translation_vector = real(i + 1) * click_vector;
                                bool is_selected = (i == num_additional_copies - 1);
                                _for_each_selected_entity_ {
                                    Entity new_entity = entity_translated(entity, translation_vector);
                                    new_entity.is_selected = is_selected;
                                    cookbook._buffer_add_entity(new_entity);
                                }
                            }
                            _for_each_selected_entity_ entity->is_selected = false;
                        } else if (state_Draw_command_is_(RCopy)) {
                            if (popup->rcopy_num_total_copies < 2) {
                                MESSAGE_FAILURE("RCopy: must have at least 2 total copies");
                            } else {
                                result.checkpoint_me = true;
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);

                                real dtheta = TAU / popup->rcopy_num_total_copies;

                                _for_each_selected_entity_ {
                                    for_(j, popup->rcopy_num_total_copies - 1) {
                                        real theta = (j + 1) * dtheta;
                                        cookbook._buffer_add_entity(entity_rotated(entity, first_click, theta));
                                    }
                                }
                            }
                        } else if (state_Draw_command_is_(Drag)) {
                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);


                            #define GRID_CELL_WIDTH 0.003f

                            auto scalar_bucket = [&](real a) -> real {
                                real ret = roundf(a / GRID_CELL_WIDTH) * GRID_CELL_WIDTH;
                                return ret == -0 ? 0 : ret; // what a fun bug
                            };

                            auto make_key = [&](vec2 p) -> vec2 {
                                return { scalar_bucket(p.x), scalar_bucket(p.y) };
                            };

                            auto nudge_key = [&](vec2 key, int dx, int dy) -> vec2 {
                                return make_key(V2(key.x + dx * GRID_CELL_WIDTH, key.y + dy * GRID_CELL_WIDTH));
                            };

                            struct GridPointSlot {
                                bool populated;
                                int entity_index;
                                bool end_NOT_start;
                            };

                            struct GridCell {
                                GridPointSlot slots[5];
                            };

                            Map<vec2, GridCell> grid; {
                                grid = {};

                                auto push_into_grid_unless_cell_full__make_cell_if_none_exists = [&](vec2 p, uint entity_index, bool end_NOT_start) {
                                    vec2 key = make_key(p);
                                    GridCell *cell = _map_get_pointer(&grid, key);
                                    if (cell == NULL) {
                                        map_put(&grid, key, {});
                                        cell = _map_get_pointer(&grid, key);
                                    }
                                    for_(i, ARRAY_LENGTH(cell->slots)) {
                                        GridPointSlot *slot = &cell->slots[i];
                                        if (slot->populated) continue;
                                        slot->populated = true;
                                        slot->entity_index = entity_index;
                                        slot->end_NOT_start = end_NOT_start;
                                        // printf("%f %f [%d]\n", key.x, key.y, i);
                                        break;
                                    }
                                };

                                for_(entity_index, drawing->entities.length) {
                                    Entity *entity = &drawing->entities.array[entity_index];

                                    vec2 start;
                                    vec2 end;
                                    bool poosh = false;

                                    if (entity_length(entity) < 0.05f) continue; // TODO: TODO: VERY SCARY 0 LENGTH ENTITIES 
                                    if (entity->type == EntityType::Circle) {
                                        CircleEntity *circle = &entity->circle;
                                        if (circle->has_pseudo_point) {
                                            poosh = true;
                                            start = end = circle->get_pseudo_point();
                                        }
                                    } else {
                                        poosh = true;
                                        entity_get_start_and_end_points(entity, &start, &end);
                                    }
                                    if (poosh) {
                                        push_into_grid_unless_cell_full__make_cell_if_none_exists(start, entity_index, false);
                                        push_into_grid_unless_cell_full__make_cell_if_none_exists(end, entity_index, true);
                                    }
                                }
                            }

                            struct EndpointMark {
                                bool start_marked;
                                bool end_marked;
                            };
                            EndpointMark *endpoint_marks = (EndpointMark *)calloc(drawing->entities.length, sizeof(EndpointMark));

                            char *to_move = (char *) calloc(drawing->entities.length, sizeof(char));

                            ////////////////////////////////////////////////////////////////////////////////
                            // NOTE: We are now done adding to the grid, so we can now operate directly on GridCell *'s
                            //       We will use _map_get_pointer(...)
                            ////////////////////////////////////////////////////////////////////////////////


                            auto get_key = [&](GridPointSlot *point, bool other_endpoint) {
                                bool end_NOT_start; {
                                    end_NOT_start = point->end_NOT_start;
                                    if (other_endpoint) end_NOT_start = !end_NOT_start;
                                }
                                vec2 p; {
                                    Entity *entity = &drawing->entities.array[point->entity_index];

                                    if (entity->type == EntityType::Circle) {
                                        CircleEntity *circle = &entity->circle;
                                        ASSERT(circle->has_pseudo_point);
                                        p = circle->get_pseudo_point();
                                    } else {
                                        if (end_NOT_start) {
                                            p = entity_get_end_point(entity);
                                        } else {
                                            p = entity_get_start_point(entity);
                                        }
                                    }
                                }
                                return make_key(p);
                            };

                            auto get_any_point_not_part_of_an_marked_entity = [&](vec2 key) -> GridPointSlot * {
                                GridCell *cell = _map_get_pointer(&grid, key);
                                if (!cell) return NULL;

                                for_(i, ARRAY_LENGTH(cell->slots)) {
                                    GridPointSlot *slot = &cell->slots[i];
                                    if (!slot->populated) continue;
                                    EndpointMark *mark = &endpoint_marks[slot->entity_index];
                                    if (slot->end_NOT_start) {
                                        if (mark->end_marked) continue;
                                    } else {
                                        if (mark->start_marked) continue;
                                    }
                                    return slot;
                                }
                                return NULL;
                            };

                            typedef struct EntVecMapping {
                                vec2 p;
                                int parentIndex;
                                bool start;
                            } EntVecMapping;

                            typedef struct EntEntEndMapping {
                                int entityToConnectToIndex;
                                bool connectToStart;

                                int entityToBeMovedIndex;
                                bool moveStart;
                            } EntEntEndMapping;

                            Queue<EntEntEndMapping> movePairs = {};

                            _for_each_selected_entity_ {

                                int hot_entity_index = entity - drawing->entities.array;

                                endpoint_marks[hot_entity_index].start_marked = true;
                                endpoint_marks[hot_entity_index].end_marked = true;

                                bool special_case_circle_no_pseudo_point = ((entity->type == EntityType::Circle) && (!entity->circle.has_pseudo_point));
                                if (special_case_circle_no_pseudo_point) {
                                    entity->is_selected = value_to_write_to_selection_mask;
                                    to_move[hot_entity_index] = true;
                                    continue;
                                }
                                for_(pass, 2) {
                                    vec2 seed; {
                                        vec2 p;
                                        if (entity->type == EntityType::Circle) {
                                            CircleEntity *circle = &entity->circle;
                                            ASSERT(circle->has_pseudo_point);
                                            p = circle->get_pseudo_point();

                                        } else {
                                            if (pass == 0) {
                                                p = entity_get_start_point(&drawing->entities.array[hot_entity_index]);
                                            } else {
                                                p = entity_get_end_point(&drawing->entities.array[hot_entity_index]);
                                            }
                                        }
                                        seed = make_key(p);
                                    }



                                    Queue<EntVecMapping> queue = {};
                                    queue_enqueue(&queue, { seed, hot_entity_index, pass == 0 });

                                    while (queue.length) {
                                        EntVecMapping curParent = queue_dequeue(&queue);
                                        seed = curParent.p;

                                        for (int dx = -1; dx <= 1; ++dx) {
                                            for (int dy = -1; dy <= 1; ++dy) {
                                                while (1) {
                                                    vec2 curPos = nudge_key(seed, dx, dy);
                                                    GridPointSlot *tmp = get_any_point_not_part_of_an_marked_entity(curPos);

                                                    if (!tmp) break;

                                                    Entity ent = drawing->entities.array[tmp->entity_index];

                                                    if (ent.type != EntityType::Line) {
                                                        GridPointSlot *nullCheck = get_any_point_not_part_of_an_marked_entity(get_key(tmp, true));
                                                        to_move[tmp->entity_index] = true;
                                                        endpoint_marks[tmp->entity_index].start_marked = true;
                                                        endpoint_marks[tmp->entity_index].end_marked = true;

                                                        if (ent.type == EntityType::Arc && nullCheck)  {
                                                            ASSERT(ent.type == EntityType::Arc);
                                                            vec2 startPoint = entity_get_start_point(&ent);
                                                            vec2 endPoint = entity_get_end_point(&ent);
                                                            bool start = distance(startPoint, curPos) < distance(endPoint, curPos);


                                                            queue_enqueue(&queue, { get_key(nullCheck, false), tmp->entity_index, !start }); // not start because this is the other end that we are adding
                                                        }
                                                    } else { // what to do if it is a line
                                                        vec2 startPoint = entity_get_start_point(&ent);
                                                        vec2 endPoint = entity_get_end_point(&ent);
                                                        bool start = distance(startPoint, curPos) < distance(endPoint, curPos);

                                                        if (start) {
                                                            endpoint_marks[tmp->entity_index].start_marked = true;
                                                        } else {
                                                            endpoint_marks[tmp->entity_index].end_marked = true;
                                                        }

                                                        queue_enqueue(&movePairs, { curParent.parentIndex, curParent.start, tmp->entity_index, start });
                                                    }


                                                } 
                                            }
                                        }
                                    }

                                    queue_free_AND_zero(&queue);
                                }
                            }

                            _for_each_entity_ { // TODO: dont actually need to go over each but im lazy 
                                if (to_move[entity - drawing->entities.array] || entity->is_selected) {
                                    *entity = entity_translated(entity, click_vector);
                                }
                            }


                            while (movePairs.length) {
                                EntEntEndMapping curMapping = queue_dequeue(&movePairs);

                                Entity *entToConnectTo = &drawing->entities.array[curMapping.entityToConnectToIndex];
                                Entity *entToMove = &drawing->entities.array[curMapping.entityToBeMovedIndex];

                                vec2 pointToConnectTo = entToConnectTo->type == EntityType::Circle ? entToConnectTo->circle.get_pseudo_point() :
                                    curMapping.connectToStart ? entity_get_start_point(entToConnectTo) : entity_get_end_point(entToConnectTo);
                                if (entToMove->type == EntityType::Line) {
                                    if (popup->drag_extend_line == 0) {
                                        if (curMapping.moveStart) {
                                            entToMove->line.start = pointToConnectTo;
                                        } else {
                                            entToMove->line.end = pointToConnectTo;
                                        }
                                    } else {
                                        // TODO: dont make duplicate lines 
                                        // TODO: make more similar to Jim's idea
                                        cookbook.buffer_add_line(pointToConnectTo, curMapping.moveStart ? entToMove->line.start : entToMove->line.end);
                                    }
                                }
                            }

                            free(endpoint_marks);
                            queue_free_AND_zero(&movePairs);
                            map_free_and_zero(&grid);
                        } else if (state_Draw_command_is_(Polygon)) {
                            uint polygon_num_sides = popup->polygon_num_sides;
                            if (clicks_are_same) {
                                MESSAGE_FAILURE("Polygon: must have non-zero size");
                            } else {
                                result.checkpoint_me = true;
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);
                                real delta_theta = TAU / polygon_num_sides;
                                vec2 center = first_click;
                                vec2 vertex_0 = second_click;
                                real radius = distance(center, vertex_0);
                                real theta_0 = ATAN2(vertex_0 - center);
                                // cookbook.buffer_add_line(center, vertex_0); // center line (so sayeth LAYOUT)
                                for_(i, polygon_num_sides) {
                                    real theta_i = theta_0 + (i * delta_theta);
                                    real theta_ip1 = theta_0 + ((i + 1) * delta_theta);
                                    cookbook.buffer_add_line(
                                            get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta_i),
                                            get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta_ip1)
                                            );
                                }
                            }
                        } else if (WINDOW_SELECT_OR_WINDOW_DESELECT()) {
                            set_state_Draw_command(None);
                            set_state_Xsel_command(None);
                            bbox2 window = {
                                MIN(first_click.x, second_click.x),
                                MIN(first_click.y, second_click.y),
                                MAX(first_click.x, second_click.x),
                                MAX(first_click.y, second_click.y)
                            };
                            _for_each_entity_ {
                                if (bbox_contains(window, entity_get_bbox(entity))) {
                                    cookbook.entity_set_is_selected(entity, value_to_write_to_selection_mask);
                                }
                            }
                        }
                    }
                } else {
                    if (state_Draw_command_is_(SetOrigin)) {
                        result.checkpoint_me = true;
                        set_state_Draw_command(None);
                        set_state_Snap_command(None);
                        drawing->origin = *mouse_transformed_position;
                    } else if (state_Draw_command_is_(DivideNearest)) { 
                        DXFFindClosestEntityResult closest_results = dxf_find_closest_entity(&drawing->entities, *mouse_transformed_position); // TODO *closest* -> *nearest*
                        if (closest_results.success) {
                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);
                            Entity *entity = closest_results.closest_entity;
                            if (entity->type == EntityType::Line) {
                                LineEntity *line = &entity->line;
                                cookbook.buffer_add_line(line->start, 
                                        closest_results.line_nearest_point, 
                                        entity->is_selected, 
                                        entity->color_code);
                                cookbook.buffer_add_line(closest_results.line_nearest_point, 
                                        line->end, 
                                        entity->is_selected, 
                                        entity->color_code);
                                cookbook.buffer_delete_entity(entity);
                            } else { ASSERT(entity->type == EntityType::Arc);
                                ArcEntity *arc = &entity->arc;
                                if (ANGLE_IS_BETWEEN_CCW(RAD(closest_results.arc_nearest_angle_in_degrees), 
                                            RAD(arc->start_angle_in_degrees), 
                                            RAD(arc->end_angle_in_degrees))) {
                                    cookbook.buffer_add_arc(arc->center, 
                                            arc->radius, 
                                            arc->start_angle_in_degrees, 
                                            closest_results.arc_nearest_angle_in_degrees, 
                                            entity->is_selected, 
                                            entity->color_code);
                                    cookbook.buffer_add_arc(arc->center, 
                                            arc->radius, 
                                            closest_results.arc_nearest_angle_in_degrees, 
                                            arc->end_angle_in_degrees, 
                                            entity->is_selected, 
                                            entity->color_code);
                                    cookbook.buffer_delete_entity(entity);
                                }
                            }          
                        }
                    } else if (state_Draw_command_is_(XMirror)) {
                        result.checkpoint_me = true;
                        set_state_Draw_command(None);
                        set_state_Snap_command(None);
                        _for_each_selected_entity_ {
                            cookbook._buffer_add_entity(entity_mirrored(entity, *mouse_transformed_position, *mouse_transformed_position + V2(0.0f, 1.0f)));
                            entity->is_selected = false;
                        }
                    } else if (state_Draw_command_is_(YMirror)) {
                        result.checkpoint_me = true;
                        set_state_Draw_command(None);
                        set_state_Snap_command(None);
                        _for_each_selected_entity_ {
                            cookbook._buffer_add_entity(entity_mirrored(entity, *mouse_transformed_position, *mouse_transformed_position + V2(1.0f, 0.0f)));
                            entity->is_selected = false;
                        }
                    } else if (state_Draw_command_is_(Offset)) {
                        // TODO: entity_offseted (and preview drawing)
                        if (IS_ZERO(popup->offset_distance)) {
                            MESSAGE_FAILURE("Offset: must have non-zero distance");
                        } else {
                            DXFFindClosestEntityResult closest_results = dxf_find_closest_entity(&drawing->entities, *mouse_transformed_position);
                            if (closest_results.success) {
                                result.checkpoint_me = true;
                                set_state_Snap_command(None);
                                cookbook._buffer_add_entity(entity_offsetted(closest_results.closest_entity, popup->offset_distance, *mouse_transformed_position));
                            }
                        }
                    } else {
                        result.record_me = false;
                    }
                }
            }
        } else if (mouse_event->subtype == MouseEventSubtype::Mesh) {
            MouseEventMesh *mouse_event_mesh = &mouse_event->mouse_event_mesh;
            result.record_me = false;
            if (!mouse_event->mouse_held) {
                MagicSnapResult3D snap_result = magic_snap_raycast(mouse_event_mesh->mouse_ray_origin, mouse_event_mesh->mouse_ray_direction);
                WorkMesh *mesh = &meshes->work;

                if (snap_result.hit_mesh) { // something hit
                    if (!(state.Mesh_command.flags & HIDE_FEATURE_PLANE)) {
                        result.checkpoint_me = result.record_me = true;
                        // if (!feature_plane->is_active)
                        other.time_since_plane_selected = 0.0f;
                        feature_plane->is_active = true;

                        // The default tolerance on this is too tight to prevent clicks on same plane from resetting things
                        // real new_distance = dot(mesh->triangle_normals[snap_result.triangle_index], snap_result.mouse_position);
                        // bool plane_changed = !ARE_EQUAL(feature_plane->normal, mesh->triangle_normals[snap_result.triangle_index])
                        //     || !ARE_EQUAL(feature_plane->signed_distance_to_world_origin, new_distance);
                        feature_plane->normal = mesh->triangle_normals[snap_result.triangle_index];
                        feature_plane->signed_distance_to_world_origin = dot(feature_plane->normal, snap_result.mouse_position);
                        preview->feature_plane_signed_distance_to_world_origin = feature_plane->signed_distance_to_world_origin;
                        feature_plane->mirror_x = false;
                        feature_plane->mirror_y = false;
                        feature_plane->rotation_angle = 0.0f;

                        // TODO Decide whether to skip animation for these
                        // preview->feature_plane_mirror_x_angle = 0.0f;
                        // preview->feature_plane_mirror_y_angle = 0.0f;
                        // preview->feature_plane_mirror_XXX_bump = 0.0f;
                        // preview->feature_plane_rotation_angle = 0.0f;
                    } 

                    if (state.Mesh_command.flags & TWO_CLICK) {
                        if (!mesh_two_click_command->awaiting_second_click) {
                            mesh_two_click_command->first_click = snap_result.mouse_position;
                            mesh_two_click_command->awaiting_second_click = true;
                        } else {
                            vec3 first_click = mesh_two_click_command->first_click;
                            vec3 second_click = snap_result.mouse_position;

                            if (0) {
                            } else if (state_Mesh_command_is_(Measure3D)) {
                                MESSAGE_INFO("Length is %gmm.", norm(second_click - first_click));
                                set_state_Mesh_command(None);
                                feature_plane->is_active = other.should_feature_plane_be_active;
                                if (feature_plane->is_active)
                                    other.time_since_plane_selected = 0.0f;
                            }
                        }
                    }
                }
            }
        } else { ASSERT(mouse_event->subtype == MouseEventSubtype::Popup);
            // NOTE: this case has been moved into POPUP();
            result.record_me = true; // FORNOW
        }
    } else if (event.type == EventType::None) {
        result.record_me = false;
    }

    { // sanity checks
      // ASSERT(popup->active_cell_index <= popup->num_cells);
        ASSERT(popup->cursor <= POPUP_CELL_LENGTH);
        ASSERT(popup->selection_cursor <= POPUP_CELL_LENGTH);
    }

    popup->_FORNOW_info_mouse_is_hovering = false; // FORNOW

    { // POPUP
        bool _gui_key_enter; {
            _gui_key_enter = false;
            if (event.type == EventType::Key) {
                KeyEvent *key_event = &event.key_event;
                if (key_event->subtype == KeyEventSubtype::Popup) {
                    _gui_key_enter = (key_event->key == GLFW_KEY_ENTER);
                }
            }
        }

        auto gui_key_enter = [&](ToolboxGroup group) {
            return (_gui_key_enter && (group == popup->manager.focus_group));
        };

        { // POPUP
            { // Snap
                if (state_Snap_command_is_(XY)) {
                    POPUP(state.Snap_command, true
                            , CellType::Real, STRING("x"), &popup->xy_x_coordinate
                            , CellType::Real, STRING("y"), &popup->xy_y_coordinate
                         );

                    if (gui_key_enter(ToolboxGroup::Snap)) {
                        set_state_Snap_command(None);
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(popup->xy_x_coordinate, popup->xy_y_coordinate));
                    }
                }
            }

            { // Draw
                vec2 *first_click = &two_click_command->first_click;
                if (0) {
                } else if (state_Draw_command_is_(Box)) {
                    if (two_click_command->awaiting_second_click) {
                        POPUP(state.Draw_command, true
                                , CellType::Real, STRING("width"),  &popup->box_width
                                , CellType::Real, STRING("height"), &popup->box_height
                             );

                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->box_width, first_click->y + popup->box_height));
                        }
                    }
                } else if (state_Draw_command_is_(Circle)) {
                    if (two_click_command->awaiting_second_click) {
                        real prev_circle_diameter = popup->circle_diameter;
                        real prev_circle_radius = popup->circle_radius;
                        real prev_circle_circumference = popup->circle_circumference;
                        {
                            POPUP(state.Draw_command, true 
                                    , CellType::Real, STRING("diameter"), &popup->circle_diameter
                                    , CellType::Real, STRING("radius"), &popup->circle_radius
                                    , CellType::Real, STRING("circumference"), &popup->circle_circumference
                                 );
                        }
                        if (prev_circle_diameter != popup->circle_diameter) {
                            popup->circle_radius = popup->circle_diameter / 2;
                            popup->circle_circumference = PI * popup->circle_diameter;
                        } else if (prev_circle_radius != popup->circle_radius) {
                            popup->circle_diameter = 2 * popup->circle_radius;
                            popup->circle_circumference = PI * popup->circle_diameter;
                        } else if (prev_circle_circumference != popup->circle_circumference) {
                            popup->circle_diameter = popup->circle_circumference / PI;
                            popup->circle_radius = popup->circle_diameter / 2;
                        }

                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->circle_radius, first_click->y));
                        }
                    }
                } else if (state_Draw_command_is_(Line)) {
                    if (two_click_command->awaiting_second_click) {
                        real prev_line_length = popup->line_length;
                        real prev_line_angle  = popup->line_angle;
                        real prev_line_run    = popup->line_run;
                        real prev_line_rise   = popup->line_rise;
                        {
                            POPUP(state.Draw_command, true
                                    , CellType::Real, STRING("run (dx)"),  &popup->line_run
                                    , CellType::Real, STRING("rise (dy)"), &popup->line_rise
                                    , CellType::Real, STRING("length"),    &popup->line_length
                                    , CellType::Real, STRING("angle"),     &popup->line_angle
                                 );
                        }
                        if ((prev_line_length != popup->line_length) || (prev_line_angle != popup->line_angle)) {
                            popup->line_run  = popup->line_length * COS(RAD(popup->line_angle));
                            popup->line_rise = popup->line_length * SIN(RAD(popup->line_angle));
                        } else if ((prev_line_run != popup->line_run) || (prev_line_rise != popup->line_rise)) {
                            popup->line_length = SQRT(popup->line_run * popup->line_run + popup->line_rise * popup->line_rise);
                            popup->line_angle = DEG(ATAN2(popup->line_rise, popup->line_run));
                        }

                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->line_run, first_click->y + popup->line_rise));
                        }
                    }
                } else if (state_Draw_command_is_(Polygon)) {
                    if (two_click_command->awaiting_second_click) {
                        uint prev_polygon_num_sides = popup->polygon_num_sides;
                        real prev_polygon_distance_to_corner = popup->polygon_distance_to_corner;
                        real prev_polygon_distance_to_side = popup->polygon_distance_to_side;
                        real prev_polygon_side_length = popup->polygon_side_length;
                        {
                            POPUP(state.Draw_command, false
                                    , CellType::Uint, STRING("num_sides"), &popup->polygon_num_sides
                                    , CellType::Real, STRING("distance_to_corner"), &popup->polygon_distance_to_corner
                                    , CellType::Real, STRING("distance_to_side"), &popup->polygon_distance_to_side
                                    , CellType::Real, STRING("side_length"), &popup->polygon_side_length
                                 );
                            popup->polygon_num_sides = MIN(MAX(3U, popup->polygon_num_sides), 256U);
                        }
                        {
                            real theta = PI / popup->polygon_num_sides;
                            if (prev_polygon_num_sides != popup->polygon_num_sides) {
                                popup->polygon_distance_to_side = popup->polygon_distance_to_corner * COS(theta);
                                popup->polygon_side_length = 2 * popup->polygon_distance_to_corner * SIN(theta);
                            } else if (prev_polygon_distance_to_corner != popup->polygon_distance_to_corner) {
                                popup->polygon_distance_to_side = popup->polygon_distance_to_corner * COS(theta);
                                popup->polygon_side_length = 2 * popup->polygon_distance_to_corner * SIN(theta);
                            } else if (prev_polygon_distance_to_side != popup->polygon_distance_to_side) {
                                popup->polygon_distance_to_corner = popup->polygon_distance_to_side / COS(theta); 
                                popup->polygon_side_length = 2 * popup->polygon_distance_to_side * TAN(theta);
                            } else if (prev_polygon_side_length != popup->polygon_side_length) {
                                popup->polygon_distance_to_corner = popup->polygon_side_length / (2 * SIN(theta));
                                popup->polygon_distance_to_side = popup->polygon_side_length / (2 * TAN(theta));
                            }
                        }

                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->polygon_distance_to_corner, first_click->y));
                        }
                    }
                } else if (state_Draw_command_is_(Translate)) {
                    // FORNOW: this is repeated from Line
                    real prev_move_length = popup->move_length;
                    real prev_move_angle = popup->move_angle;
                    real prev_move_run = popup->move_run;
                    real prev_move_rise = popup->move_rise;
                    {
                        POPUP(state.Draw_command, true
                                , CellType::Real, STRING("run (dx)"),  &popup->move_run
                                , CellType::Real, STRING("rise (dy)"), &popup->move_rise
                                , CellType::Real, STRING("length"),    &popup->move_length
                                , CellType::Real, STRING("angle"),     &popup->move_angle 
                             );
                    }
                    if ((prev_move_length != popup->move_length) || (prev_move_angle != popup->move_angle)) {
                        popup->move_run = popup->move_length * COS(RAD(popup->move_angle));
                        popup->move_rise = popup->move_length * SIN(RAD(popup->move_angle));
                    } else if ((prev_move_run != popup->move_run) || (prev_move_rise != popup->move_rise)) {
                        popup->move_length = SQRT(popup->move_run * popup->move_run + popup->move_rise * popup->move_rise);
                        popup->move_angle = DEG(ATAN2(popup->move_rise, popup->move_run));
                    }

                    if (gui_key_enter(ToolboxGroup::Draw)) {
                        two_click_command->awaiting_second_click = true;
                        two_click_command->first_click = {};
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(popup->move_run, popup->move_rise));
                    }
                } else if (state_Draw_command_is_(Rotate)) {
                    POPUP(state.Draw_command, true
                            , CellType::Real, STRING("angle"), &popup->rotate_angle
                         );

                    if (gui_key_enter(ToolboxGroup::Draw)) {
                        if (!two_click_command->awaiting_second_click) {
                            two_click_command->first_click = {};
                        }
                        two_click_command->awaiting_second_click = true;
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(two_click_command->first_click + e_theta(RAD(popup->rotate_angle))));
                    }
                } else if (state_Draw_command_is_(Scale)) {
                    POPUP(state.Draw_command, true
                            , CellType::Real, STRING("scale factor"), &popup->scale_factor
                         );

                    if (gui_key_enter(ToolboxGroup::Draw)) {
                        if (!two_click_command->awaiting_second_click) {
                            bbox2 bbox = entities_get_bbox(&drawing->entities, true);
                            vec2 bbox_center = AVG(bbox.min, bbox.max);
                            two_click_command->first_click = bbox_center;
                        }
                        two_click_command->awaiting_second_click = true;
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D({})); // FORNOW
                    }
                } else if (state_Draw_command_is_(Drag)) {
                    // FORNOW: this is repeated from Line
                    real prev_drag_length = popup->drag_length;
                    real prev_drag_angle = popup->drag_angle;
                    real prev_drag_run = popup->drag_run;
                    real prev_drag_rise = popup->drag_rise;
                    {
                        POPUP(state.Draw_command, true
                                , CellType::Real, STRING("run (dx)"),  &popup->drag_run
                                , CellType::Real, STRING("rise (dy)"), &popup->drag_rise
                                , CellType::Real, STRING("length"),    &popup->drag_length
                                , CellType::Real, STRING("angle"),     &popup->drag_angle 
                                // , CellType::Uint, STRING("1 for extend line"), &popup->drag_extend_line
                             );
                    }
                    if ((prev_drag_length != popup->drag_length) || (prev_drag_angle != popup->drag_angle)) {
                        popup->drag_run = popup->drag_length * COS(RAD(popup->drag_angle));
                        popup->drag_rise = popup->drag_length * SIN(RAD(popup->drag_angle));
                    } else if ((prev_drag_run != popup->drag_run) || (prev_drag_rise != popup->drag_rise)) {
                        popup->drag_length = SQRT(popup->drag_run * popup->drag_run + popup->drag_rise * popup->drag_rise);
                        popup->drag_angle = DEG(ATAN2(popup->drag_rise, popup->drag_run));
                    }

                    if (gui_key_enter(ToolboxGroup::Draw)) {
                        *first_click = V2(0, 0);
                        two_click_command->awaiting_second_click = true;
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(popup->drag_run, popup->drag_rise));
                    }
                } else if (state_Draw_command_is_(LCopy)) {
                    real prev_lcopy_length = popup->lcopy_length;
                    real prev_lcopy_angle = popup->lcopy_angle;
                    real prev_lcopy_run = popup->lcopy_run;
                    real prev_lcopy_rise = popup->lcopy_rise;
                    {
                        POPUP(state.Draw_command, true
                                , CellType::Real, STRING("run (dx)"),              &popup->lcopy_run
                                , CellType::Real, STRING("rise (dy)"),             &popup->lcopy_rise
                                , CellType::Real, STRING("length"),                &popup->lcopy_length
                                , CellType::Real, STRING("angle"),                 &popup->lcopy_angle
                                , CellType::Uint, STRING("num_additional_copies"), &popup->lcopy_num_additional_copies
                             );

                        // TODO
                        popup->lcopy_num_additional_copies = CLAMP(popup->lcopy_num_additional_copies, 1U, 1024U);
                    }
                    if ((prev_lcopy_length != popup->lcopy_length) || (prev_lcopy_angle != popup->lcopy_angle)) {
                        popup->lcopy_run = popup->lcopy_length * COS(RAD(popup->lcopy_angle));
                        popup->lcopy_rise = popup->lcopy_length * SIN(RAD(popup->lcopy_angle));
                    } else if ((prev_lcopy_run != popup->lcopy_run) || (prev_lcopy_rise != popup->lcopy_rise)) {
                        popup->lcopy_length = SQRT(popup->lcopy_run * popup->lcopy_run + popup->lcopy_rise * popup->lcopy_rise);
                        popup->lcopy_angle = DEG(ATAN2(popup->lcopy_rise, popup->lcopy_run));
                    }

                    if (gui_key_enter(ToolboxGroup::Draw)) {
                        two_click_command->first_click = {};
                        two_click_command->awaiting_second_click = true;
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(popup->lcopy_run, popup->lcopy_rise));
                    }
                } else if (state_Draw_command_is_(RCopy)) {
                    {
                        POPUP(state.Draw_command,
                                true
                                , CellType::Uint, STRING("num_total_copies"), &popup->rcopy_num_total_copies
                             );

                        // TODO
                        popup->rcopy_num_total_copies = CLAMP(popup->rcopy_num_total_copies, 1U, 128U);
                    }

                    if (gui_key_enter(ToolboxGroup::Draw)) {
                        if (!two_click_command->awaiting_second_click) {
                            two_click_command->first_click = {};
                        }
                        two_click_command->awaiting_second_click = true;
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D({})); // FORNOW
                    }
                } else if (state_Draw_command_is_(ElfHat)) {
                    // TODO
                } else if (state_Draw_command_is_(Fillet)) {
                    POPUP(state.Draw_command,
                            false
                            , CellType::Real, STRING("radius"), &popup->fillet_radius
                         );
                } else if (state_Draw_command_is_(DogEar)) {
                    POPUP(state.Draw_command,
                            false
                            , CellType::Real, STRING("radius"), &popup->dogear_radius
                         );
                } else if (state_Draw_command_is_(Offset)) {
                    POPUP(state.Draw_command,
                            false
                            , CellType::Real, STRING("distance"), &popup->offset_distance
                         );
                } else if (state_Draw_command_is_(OpenDXF)) {
                    POPUP(state.Draw_command,
                            false
                            , CellType::String, STRING("filename"), &popup->open_dxf_filename
                         );
                    if (gui_key_enter(ToolboxGroup::Draw)) {
                        if (FILE_EXISTS(popup->open_dxf_filename)) {
                            if (string_matches_suffix(popup->open_dxf_filename, STRING(".dxf"))) {
                                result.record_me = true;
                                result.checkpoint_me = true;
                                // result.snapshot_me = true;

                                { // conversation_dxf_load
                                    ASSERT(FILE_EXISTS(popup->open_dxf_filename));

                                    list_free_AND_zero(&drawing->entities);

                                    entities_load(popup->open_dxf_filename, &drawing->entities);

                                    { // TODO: guard this properly so it doesn't happen on undo/(redo)??
                                        init_camera_drawing();
                                        drawing->origin = {};
                                    }
                                }
                                set_state_Draw_command(None);
                                MESSAGE_SUCCESS("OpenDXF \"%s\"", popup->open_dxf_filename.data);
                            } else {
                                MESSAGE_FAILURE("OpenDXF: \"%s\" must be *.dxf", popup->open_dxf_filename.data);
                            }
                        } else {
                            MESSAGE_FAILURE("Load: \"%s\" not found", popup->open_dxf_filename.data);
                        }
                    }
                } else if (state_Draw_command_is_(SaveDXF)) {
                    result.record_me = false;
                    POPUP(state.Draw_command,
                            false
                            , CellType::String, STRING("filename"), &popup->save_dxf_filename
                         );
                    if (gui_key_enter(ToolboxGroup::Draw)) {
                        if (!FILE_EXISTS(popup->save_dxf_filename)) {
                            if (string_matches_suffix(popup->save_dxf_filename, STRING(".dxf"))) {
                                set_state_Draw_command(None);
                                {
                                    bool success = drawing_save_dxf(drawing, popup->save_dxf_filename);
                                    ASSERT(success);
                                }
                                MESSAGE_SUCCESS("SaveDXF \"%s\"", popup->save_dxf_filename.data);
                            } else {
                                MESSAGE_FAILURE("SaveDXF \"%s\" must be *.dxf", popup->save_dxf_filename.data);
                            }
                        } else {
                            MESSAGE_FAILURE("SaveDXF \"%s\" already exists", popup->save_dxf_filename.data);
                            set_state_Draw_command(OverwriteDXF);
                        }
                    }
                } else if (state_Draw_command_is_(OverwriteDXF)) {
                    result.record_me = false;
                    POPUP(state.Draw_command,
                            true
                            , CellType::String, STRING("confirm (y/n)"), &popup->overwrite_dxf_yn_buffer
                         );
                    if (gui_key_enter(ToolboxGroup::Draw)) {
                        if (popup->overwrite_dxf_yn_buffer.data[0] == 'y') {
                            if (string_matches_suffix(popup->save_dxf_filename, STRING(".dxf"))) {
                                {
                                    bool success = drawing_save_dxf(drawing, popup->save_dxf_filename);
                                    ASSERT(success);
                                }
                                set_state_Draw_command(None);
                                MESSAGE_SUCCESS("OverwriteDXF \"%s\"", popup->save_dxf_filename.data);
                            } else {
                                MESSAGE_FAILURE("OverwriteDXF \"%s\" must be *.dxf", popup->save_dxf_filename.data);
                            }
                        } else if (popup->overwrite_dxf_yn_buffer.data[0] == 'n') {
                            set_state_Draw_command(None);
                            MESSAGE_FAILURE("OverwriteDXF declined");
                        } else {
                            MESSAGE_INFO("OverwriteDXF confirm with y or n");
                        }
                    }
                }
            }

            { // Mesh
                if (0) {
                } else if (state_Mesh_command_is_(NudgePlane)) {
                    POPUP(state.Mesh_command,
                            true
                            , CellType::Real, STRING("rise"), &popup->feature_plane_nudge
                         );
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        result.record_me = true;
                        result.checkpoint_me = true;
                        feature_plane->signed_distance_to_world_origin += popup->feature_plane_nudge;
                        set_state_Mesh_command(None);
                        MESSAGE_SUCCESS("NudgePlane %gmm", popup->feature_plane_nudge);
                    }
                } else if (state_Mesh_command_is_(RotatePlane)) {
                    POPUP(state.Mesh_command,
                            true
                            , CellType::Real, STRING("angle"), &popup->feature_plane_rotate_plane_angle
                         );
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        result.record_me = true;
                        result.checkpoint_me = true;
                        feature_plane->rotation_angle += RAD(popup->feature_plane_rotate_plane_angle);
                        set_state_Mesh_command(None);
                        MESSAGE_SUCCESS("RotatePlane %gdegrees", popup->feature_plane_rotate_plane_angle);
                    }
                } else if (state_Mesh_command_is_(ExtrudeAdd)) {
                    POPUP(state.Mesh_command, true
                            , CellType::Real, STRING("out_length"), &popup->extrude_add_out_length
                            , CellType::Real, STRING("in_length"),  &popup->extrude_add_in_length
                         );
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (!dxf_anything_selected) {
                            MESSAGE_FAILURE("ExtrudeAdd: selection empty");
                        } else if (!feature_plane->is_active) {
                            MESSAGE_FAILURE("ExtrudeAdd: no feature plane selected");
                        } else if (IS_ZERO(popup->extrude_add_in_length) && IS_ZERO(popup->extrude_add_out_length)) {
                            MESSAGE_FAILURE("ExtrudeAdd: total extrusion length zero");
                        } else {
                            cookbook.manifold_wrapper_wrapper();

                            if (IS_ZERO(popup->extrude_add_in_length)) {
                                MESSAGE_SUCCESS("ExtrudeAdd %gmm", popup->extrude_add_out_length);
                            } else {
                                MESSAGE_SUCCESS("ExtrudeAdd %gmm %gmm", popup->extrude_add_out_length, popup->extrude_add_in_length);
                            }

                            other.tween_extrude_add_feature_plane = *feature_plane;
                            preview->tween_extrude_add_scale      = 0.0f;
                        }
                    }
                } else if (state_Mesh_command_is_(ExtrudeCut)) {
                    POPUP(state.Mesh_command,
                            true
                            , CellType::Real, STRING("in_length"),  &popup->extrude_cut_in_length
                            , CellType::Real, STRING("out_length"), &popup->extrude_cut_out_length
                         );
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (!dxf_anything_selected) {
                            MESSAGE_FAILURE("ExtrudeCut: selection empty");
                        } else if (!feature_plane->is_active) {
                            MESSAGE_FAILURE("ExtrudeCut: no feature plane selected");
                        } else if (IS_ZERO(popup->extrude_cut_in_length) && IS_ZERO(popup->extrude_cut_out_length)) {
                            MESSAGE_FAILURE("ExtrudeCut: total extrusion length zero");
                        } else if (meshes->work.num_triangles == 0) {
                            MESSAGE_FAILURE("ExtrudeCut: current mesh empty");
                        } else {
                            cookbook.manifold_wrapper_wrapper();
                            if (IS_ZERO(popup->extrude_cut_out_length)) {
                                MESSAGE_SUCCESS("ExtrudeCut %gmm", popup->extrude_cut_in_length);
                            } else {
                                MESSAGE_SUCCESS("ExtrudeCut %gmm %gmm", popup->extrude_cut_in_length, popup->extrude_cut_out_length);
                            }
                        }
                    }
                } else if (state_Mesh_command_is_(RevolveAdd)) {
                    POPUP(state.Mesh_command,
                            true
                            , CellType::Real, STRING("out_angle"), &popup->revolve_add_out_angle
                            , CellType::Real, STRING("in_angle"),  &popup->revolve_add_in_angle
                         );
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (!dxf_anything_selected) {
                            MESSAGE_FAILURE("RevolveAdd: selection empty");
                        } else if (!feature_plane->is_active) {
                            MESSAGE_FAILURE("RevolveAdd: no feature plane selected");
                        } else {
                            cookbook.manifold_wrapper_wrapper();
                            MESSAGE_SUCCESS("RevolveAdd");
                        }
                    }
                } else if (state_Mesh_command_is_(RevolveCut)) {
                    POPUP(state.Mesh_command,
                            true
                            , CellType::Real, STRING("in_angle"),  &popup->revolve_cut_in_angle
                            , CellType::Real, STRING("out_angle"), &popup->revolve_cut_out_angle
                         );
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (!dxf_anything_selected) {
                            MESSAGE_FAILURE("RevolveCut: selection empty");
                        } else if (!feature_plane->is_active) {
                            MESSAGE_FAILURE("RevolveCut: no feature plane selected");
                        } else if (meshes->work.num_triangles == 0) {
                            MESSAGE_FAILURE("RevolveCut: current mesh empty");
                        } else {
                            cookbook.manifold_wrapper_wrapper();
                            MESSAGE_SUCCESS("RevolveCut");
                        }
                    }
                } else if (state_Mesh_command_is_(OpenSTL)) {
                    POPUP(state.Mesh_command,
                            false
                            , CellType::String, STRING("filename"), &popup->open_stl_filename
                         );
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (FILE_EXISTS(popup->open_stl_filename)) {
                            if (string_matches_suffix(popup->open_stl_filename, STRING(".stl"))) {
                                result.record_me = true;
                                result.checkpoint_me = true;
                                result.snapshot_me = true;
                                { // conversation_stl_load(...)
                                    WARN_ONCE("TODO: we should be freeing here");
                                    *meshes = stl_load(popup->open_stl_filename);
                                    init_camera_mesh();
                                }
                                MESSAGE_SUCCESS("OpenSTL \"%s\"", popup->open_stl_filename.data);

                                set_state_Mesh_command(None);

                                preview->tween_extrude_add_scale      = 0.0f;
                            } else {
                                MESSAGE_FAILURE("OpenSTL: \"%s\" must be *.stl", popup->open_stl_filename.data);
                            }
                        } else {
                            MESSAGE_FAILURE("Load: \"%s\" not found", popup->open_stl_filename.data);
                        }
                    }
                } else if (state_Mesh_command_is_(SaveSTL)) {
                    result.record_me = false;
                    POPUP(state.Mesh_command,
                            false
                            , CellType::String, STRING("filename"), &popup->save_stl_filename
                         );
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (!FILE_EXISTS(popup->save_stl_filename)) {
                            if (string_matches_suffix(popup->save_stl_filename, STRING(".stl"))) {
                                set_state_Mesh_command(None);
                                {
                                    bool success = mesh_save_stl(&meshes->work, popup->save_stl_filename);
                                    ASSERT(success);
                                }
                                MESSAGE_SUCCESS("SaveSTL \"%s\"", popup->save_stl_filename.data);
                            } else {
                                MESSAGE_FAILURE("SaveSTL \"%s\" must be *.stl", popup->save_stl_filename.data);
                            }
                        } else {
                            MESSAGE_FAILURE("SaveSTL \"%s\" already exists", popup->save_stl_filename.data);
                            set_state_Mesh_command(OverwriteSTL);
                        }
                    }
                } else if (state_Mesh_command_is_(OverwriteSTL)) {
                    result.record_me = false;
                    POPUP(state.Mesh_command, true
                            , CellType::String, STRING("confirm (y/n)"), &popup->overwrite_stl_yn_buffer
                         );
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (popup->overwrite_stl_yn_buffer.data[0] == 'y') {
                            if (string_matches_suffix(popup->save_stl_filename, STRING(".stl"))) {
                                {
                                    bool success = mesh_save_stl(&meshes->work, popup->save_stl_filename);
                                    ASSERT(success);
                                }
                                set_state_Mesh_command(None);
                                MESSAGE_SUCCESS("OverwriteSTL \"%s\"", popup->save_stl_filename.data);
                            } else {
                                MESSAGE_FAILURE("OverwriteSTL \"%s\" must be *.stl", popup->save_stl_filename.data);
                            }
                        } else if (popup->overwrite_stl_yn_buffer.data[0] == 'n') {
                            set_state_Mesh_command(None);
                            MESSAGE_FAILURE("OverwriteSTL declined");
                        } else {
                            MESSAGE_INFO("OverwriteSTL confirm with y or n");
                        }
                    }
                }
            }
        }
    }


    // FORNOW: remove zero length at end of loop TODO: don't allow their creation
    _for_each_entity_ {
        if (entity_length(entity) < GRID_CELL_WIDTH) {
            MESSAGE_FAILURE("!!! zero length entity detected and deleted");
            cookbook.buffer_delete_entity(entity);
        }
        if (entity->type == EntityType::Circle) {
            if (entity->circle.radius < GRID_CELL_WIDTH) {
                MESSAGE_FAILURE("!!! zero length entity detected and deleted");
                cookbook.buffer_delete_entity(entity);
            }
        }
        if (entity->type == EntityType::Arc) {
            if (entity->arc.radius < GRID_CELL_WIDTH) {
                MESSAGE_FAILURE("!!! zero length entity detected and deleted");
                cookbook.buffer_delete_entity(entity);
            }
        }
    }

    // event_passed_to_popups = {}; // FORNOW: probably unnecessary
    // already_processed_event_passed_to_popups = false; // FORNOW: probably unnecessary
    return result;
}

#if 0
void history_process_event(Event freshly_baked_event) {
    // bool undo;
    // bool redo;
    // {
    //     undo = false;
    //     redo = false;
    //     if (freshly_baked_event.type == EventType::Key) {
    //         KeyEvent *key_event = &freshly_baked_event.key_event;
    //         auto key_lambda = [key_event](uint key, bool control = false, bool shift = false) -> bool {
    //             return _key_lambda(key_event, key, control, shift);
    //         };
    //         if (!((popup->_FORNOW_active_popup_unique_ID__FORNOW_name0) && (popup->cell_type[popup->active_cell_index] == CellType::String))) { // FORNOW
    //             undo = (key_lambda('Z', true) || key_lambda('U'));
    //             redo = (key_lambda('Y', true) || key_lambda('Z', true, true) || key_lambda('U', false, true));
    //         }
    //     }
    // }

    // if (undo) {
    //     other._please_suppress_drawing_popup_popup = true;
    //     history_undo();
    // } else if (redo) {
    //     _standard_event_process_NOTE_RECURSIVE({}); // FORNOW (prevent flicker on redo with nothing left to redo)
    //     other._please_suppress_drawing_popup_popup = true;
    //     history_redo();
    // } else {
    history_process_event(freshly_baked_event);
    // }
}
#endif
