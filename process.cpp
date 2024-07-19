// TODO: rz needs work

// TODO: beautiful buttons; should indicate what's selected in green (persistent)
// FORNOW: rotate copy's usage of second click is wonky

// XXXX: popups should have titles

// TODO: pressing F when Fillet already active should highlight popup field

// TODO: linear copy like layout
// TODO: ???linear copy shouldn't be able to snap to the thing that 

// TODO: mouse still pops on undo/redo
// ~~~~: snaps flicker when typing in the popup 

// XXXX: click modifier belongs in other

#if 0
// TODO
StandardEventProcessResult standard_event_process(Event event) {
    bool global_world_state_changed;
    StandardEventProcessResult result = _standard_event_process_NOTE_RECURSIVE(event);
    if (global_world_state_changed) {
        result.record_me = true;
    }
    return result;
}
#endif

StandardEventProcessResult _standard_event_process_NOTE_RECURSIVE(Event event) {
    popup->manager.begin_process();
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
    bool value_to_write_to_selection_mask = (state.click_mode == ClickMode::Select);


    StandardEventProcessResult result = {};

    Cookbook cookbook = make_Cookbook(event, &result, skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack);
    defer {
        cookbook.end_frame();
        cookbook_free(&cookbook);
    };


    // NOTE: we can put other buttons in there at will (select all, etc.)
    //       we are NOT going to use a general-purpose retained-mode gui lib like layout
    // NEW IDEA (the old one won't work because of how we record history):
    //          ((we have to bake the correct event type--really can't depend on window, etc., very fragile))
    //          let's let the dummy event pass through the key events and draw everything
    //          then, like with popup, we'll bake based on which bkey_lambda is hot hot hot
    // IDEA (hacky but may work): if buttons are enabled, we'll attempt to steal any Mouse
    //                            event and press buttons with its mouse_Pixel field
    //                            if this works, we consume the event (regardless of pane)
    // TODO: should popup have worked this same way?
    //       it probably could have been (though i maybe recall the graphics get messed up
    //       if you reorder things) so popup hits first
    //       (as is, we bake MouseEventPopup based on popups from last frame)
    bool is_toolbox_button_mouse_event =
        (event.type == EventType::Mouse)
        && (event.mouse_event.subtype == MouseEventSubtype::ToolboxButton);

    if ((event.type == EventType::Key) || is_toolbox_button_mouse_event) {
        KeyEvent *key_event = &event.key_event;
        if ((key_event->subtype == KeyEventSubtype::Hotkey) || is_toolbox_button_mouse_event) {
            result.record_me = true;

            *toolbox = {};

            real padding = 12.0f;

            EasyTextPen drawing_pen = { V2(padding, padding), 12.0f, omax.white, true };
            EasyTextPen drawing_pen2 = drawing_pen;
            drawing_pen2.font_height_Pixel = 9.0f;
            drawing_pen2.color = omax.light_gray;

            real h = drawing_pen.font_height_Pixel + drawing_pen2.font_height_Pixel;

            EasyTextPen snap_pen = drawing_pen;
            EasyTextPen snap_pen2 = drawing_pen2;
            snap_pen.origin_Pixel.y = window_get_height_Pixel() - h - padding;
            snap_pen2.origin_Pixel.y = snap_pen.origin_Pixel.y;

            EasyTextPen mesh_pen = drawing_pen;
            EasyTextPen mesh_pen2 = drawing_pen2;
            mesh_pen.origin_Pixel.x = get_x_divider_drawing_mesh_Pixel() + drawing_pen.origin_Pixel.x;
            mesh_pen2.origin_Pixel.x = mesh_pen.origin_Pixel.x;

            auto SEPERATOR = [&](ToolboxGroup group) {
                real eps = 8;
                if (group == ToolboxGroup::Drawing) {
                    drawing_pen.offset_Pixel.y += eps;
                } else if (group == ToolboxGroup::Snap) {
                    messagef(omax.red, "horrifying stuff with Snap pen origin/offset");
                } else { ASSERT(group == ToolboxGroup::Mesh);
                    mesh_pen.offset_Pixel.y += eps;
                }
            };

            bool special_case_started_frame_with_snaps_enabled_NOTE_fixes_partial_snap_toolbox_graphical_glitch = click_mode_SNAP_ELIGIBLE();

            bool hotkey_recognized = false;
            auto magic_magic = [&](
                    Keybind keybind,
                    char *name = NULL,
                    bool hotkey_label_only = false,
                    ToolboxGroup group = ToolboxGroup::Drawing,
                    ClickMode click_mode = ClickMode::None,
                    ClickModifier click_modifier = ClickModifier::None,
                    EnterMode enter_mode = EnterMode::None
                    ) -> bool {

                bool control = keybind.modifiers & MOD_CTRL;
                bool shift = keybind.modifiers & MOD_SHIFT;
                bool alt = keybind.modifiers & MOD_ALT;
                uint key = keybind.key;

                FORNOW_UNUSED(alt);

                bool special_case_dont_draw_toolbox_NOTE_fixes_undo_graphical_glitch = (other._please_suppress_drawing_popup_popup && (group == ToolboxGroup::Snap));
                bool draw_tool = name;

                if (!other.hide_toolbox
                        && draw_tool
                        && !special_case_dont_draw_toolbox_NOTE_fixes_undo_graphical_glitch
                        && ((group != ToolboxGroup::Snap) || special_case_started_frame_with_snaps_enabled_NOTE_fixes_partial_snap_toolbox_graphical_glitch)
                   ) {
                    EasyTextPen *pen;
                    EasyTextPen *pen2;
                    bool horz = false;
                    if (group == ToolboxGroup::Drawing) {
                        pen = &drawing_pen;
                        pen2 = &drawing_pen2;
                    } else if (group == ToolboxGroup::Snap) {
                        pen = &snap_pen;
                        pen2 = &snap_pen2;
                        horz = true;
                    } else { ASSERT(group == ToolboxGroup::Mesh);
                        pen = &mesh_pen;
                        pen2 = &mesh_pen2;
                    }
                    real w = 64.0f;

                    real y = pen->get_y_Pixel();
                    bbox2 bbox = { pen->origin_Pixel.x, y - 2, pen->origin_Pixel.x + w, y + h };

                    bool hovering = bbox_contains(bbox, other.mouse_Pixel);

                    if (hovering) {
                        *toolbox = {};
                        toolbox->hot_name = name;
                    }

                    {

                        vec3 accent_color = get_accent_color(group); 

                        bool can_toggle;
                        bool toggled;
                        {
                            can_toggle = false;
                            toggled = false;
                            if (click_mode != ClickMode::None) {
                                can_toggle = true;
                                toggled = (click_mode == state.click_mode);
                            } else if (click_modifier != ClickModifier::None) {
                                can_toggle = true;
                                toggled = (click_modifier == state.click_modifier);
                            } else if (enter_mode != EnterMode::None) {
                                can_toggle = true;
                                toggled = (enter_mode == state.enter_mode);
                            }
                        }

                        vec3 base_color = (can_toggle) ? omax.dark_gray : AVG(omax.dark_gray, omax.gray);

                        eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
                        eso_overlay(true);
                        eso_color(
                                (hovering)
                                ? ((other.mouse_left_drag_pane == Pane::Toolbox) ? AVG(omax.white, accent_color) : accent_color)
                                : ((toggled) ? AVG(omax.black, accent_color)
                                    : base_color)
                                );
                        eso_bbox_SOUP_QUADS(bbox);
                        eso_end();
                    }

                    KeyEvent tmp = { {}, key, control, shift, alt };
                    pen->offset_Pixel.x = 0.5f * (w - _easy_text_dx(pen, name));
                    easy_text_drawf(pen, name);
                    pen2->offset_Pixel.y = pen->offset_Pixel.y;
                    pen2->offset_Pixel.x = 0.5f * (w - _easy_text_dx(pen2, key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(&tmp)));
                    easy_text_drawf(pen2, key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(&tmp));
                    pen->offset_Pixel.y = pen2->offset_Pixel.y + 4;
                    if (horz) {
                        pen->offset_Pixel = {};
                        pen2->offset_Pixel = {};
                        pen->origin_Pixel.x += w + 2;
                        pen2->origin_Pixel.x += w + 2;
                    }
                }

                // TODO: ScriptEvent
                // TODO: this should store whether we're hovering in toolbox as well as the event that will be generated if we click

                if (!hotkey_label_only) {
                    hotkey_recognized |= _key_lambda(key_event, key, control, shift, alt);
                }

                bool result; {
                    result = false;
                    if (is_toolbox_button_mouse_event) {
                        result |= (name == event.mouse_event.mouse_event_toolbox_button.name);
                    } else {
                        if (!hotkey_label_only) result |= _key_lambda(key_event, key, control, shift);
                    }
                }

                #if 0
                // canned logic 
                if (result) {
                    if (click_mode != ClickMode::None) { ASSERT(group == ToolboxGroup::Drawing); state.click_mode = click_mode; }
                    if (click_mode_SNAP_ELIGIBLE() && (click_modifier != ClickModifier::None)) { ASSERT(group == ToolboxGroup::Snap); state.click_modifier = click_modifier; }
                    if (enter_mode != EnterMode::None) { ASSERT(group == ToolboxGroup::Mesh); state.enter_mode = enter_mode; }
                }
                #endif

                return result;
            };


            ClickMode prev_click_mode = state.click_mode;
            EnterMode prev_enter_mode = state.enter_mode;
            { // magic_magic

                if (click_mode_SNAP_ELIGIBLE()) {
                    if (magic_magic(keybinds.END,"End",false,ToolboxGroup::Snap,ClickMode::None,ClickModifier::End)) {
                        result.record_me = false;
                        state.click_modifier = ClickModifier::End;

                    }

                    if (magic_magic(keybinds.XY,"XY",false,ToolboxGroup::Snap,ClickMode::None,ClickModifier::XY)) {
                        popup->manager.manually_set_focus_group(ToolboxGroup::Snap);
                        state.click_modifier = ClickModifier::XY;
                    }

                    if (magic_magic(keybinds.CENTER,"Center",false,ToolboxGroup::Snap,ClickMode::None, ClickModifier::Center)) {
                        result.record_me = false;
                        state.click_modifier = ClickModifier::Center;

                    }

                    if (magic_magic(keybinds.MIDDLE,"Middle",false,ToolboxGroup::Snap,ClickMode::None,ClickModifier::Middle)) {
                        result.record_me = false;
                        state.click_modifier = ClickModifier::Middle;

                    }

                    if (magic_magic(keybinds.PERPENDICULAR,"Perpendicular",false,ToolboxGroup::Snap,ClickMode::None,ClickModifier::Perpendicular)) {
                        result.record_me = false;
                        state.click_modifier = ClickModifier::Perpendicular;

                    }

                    if (magic_magic(keybinds.QUAD,"Quad",false,ToolboxGroup::Snap,ClickMode::None,ClickModifier::Quad)) {
                        result.record_me = false;
                        state.click_modifier = ClickModifier::Quad;

                    }

                    if (magic_magic(keybinds.ZERO,"Zero",false,ToolboxGroup::Snap)) {
                        Event equivalent = {};
                        equivalent.type = EventType::Mouse;
                        equivalent.mouse_event.subtype = MouseEventSubtype::Drawing;

                        // .mouse_position = {};
                        return _standard_event_process_NOTE_RECURSIVE(equivalent);
                    }
                }
                SEPERATOR(ToolboxGroup::Drawing);


                if (click_mode_SELECT_OR_DESELECT()) {
                    if (magic_magic(keybinds.SELECT_ALL)) { 
                        result.checkpoint_me = true;
                        cookbook.set_is_selected_for_all_entities(state.click_mode == ClickMode::Select);
                        state.click_mode = ClickMode::None;
                        state.click_modifier = ClickModifier::None;

                    }

                    if (magic_magic(keybinds.SELECT_CONNECTED)) {
                        state.click_modifier = ClickModifier::Connected;

                    }

                    if (magic_magic(keybinds.COLOR)) {
                        state.click_mode = ClickMode::Color;
                        state.click_modifier = ClickModifier::None;

                    }

                    if (magic_magic(keybinds.SELECT_WINDOW)) {
                        state.click_modifier = ClickModifier::Window;
                        two_click_command->awaiting_second_click = false;

                    }
                }

                { // undo
                    bool hotkey_undo_alternate = magic_magic(keybinds.UNDO_ALTERNATE);
                    bool button_undo = magic_magic(keybinds.UNDO,"Undo");
                    if ((hotkey_undo_alternate || button_undo)) {
                        result.record_me = false;
                        other._please_suppress_drawing_popup_popup = true;
                        history_undo();

                    }
                }

                { // redo
                    bool hotkey_redo_alternate = magic_magic(keybinds.REDO_ALTERNATE);
                    bool button_redo = magic_magic(keybinds.REDO, "Redo");
                    if ((hotkey_redo_alternate || button_redo)) {
                        result.record_me = false;
                        // _standard_event_process_NOTE_RECURSIVE({}); // FORNOW (prevent flicker on redo with nothing left to redo)
                        other._please_suppress_drawing_popup_popup = true;
                        history_redo();

                    }
                }

                if (magic_magic(keybinds.SELECT,"Select",false,ToolboxGroup::Drawing,ClickMode::Select)) { // TODO
                    if (state.click_mode != ClickMode::Color) {
                        state.click_mode = ClickMode::Select;
                        state.click_modifier = ClickModifier::None;

                    } else {
                        state.click_modifier = ClickModifier::Selected;

                    }
                }

                if (magic_magic(keybinds.DESELECT,"Deselect",false,ToolboxGroup::Drawing,ClickMode::Deselect)) {
                    state.click_mode = ClickMode::Deselect;
                    state.click_modifier = ClickModifier::None;

                }

                SEPERATOR(ToolboxGroup::Drawing);

                if (magic_magic(keybinds.LINE,"Line",false,ToolboxGroup::Drawing,ClickMode::Line)) {
                    state.click_mode = ClickMode::Line;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;

                }

                if (magic_magic(keybinds.CIRCLE, "Circle",false,ToolboxGroup::Drawing,ClickMode::Circle)) {
                    state.click_mode = ClickMode::Circle;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;


                }

                if (magic_magic(keybinds.BOX,"CornerBox",false,ToolboxGroup::Drawing,ClickMode::Box)) {
                    state.click_mode = ClickMode::Box;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;

                }

                if (magic_magic(keybinds.CENTERED_BOX,"CenterBox",false,ToolboxGroup::Drawing,ClickMode::CenteredBox)) {
                    state.click_mode = ClickMode::CenteredBox;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;

                }


                if (magic_magic(keybinds.POLYGON,"Polygon",false,ToolboxGroup::Drawing,ClickMode::Polygon)) {
                    state.click_mode = ClickMode::Polygon;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;

                }

                SEPERATOR(ToolboxGroup::Drawing);


                if (magic_magic(keybinds.MOVE,"Move",false,ToolboxGroup::Drawing,ClickMode::Move)) {
                    state.click_mode = ClickMode::Move;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;

                }
                if (magic_magic(keybinds.ROTATE,"Rotate",false,ToolboxGroup::Drawing,ClickMode::Rotate)) {
                    state.click_mode = ClickMode::Rotate;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;

                }

                SEPERATOR(ToolboxGroup::Drawing);

                if (magic_magic(keybinds.LINEAR_COPY,"LinearCopy",false,ToolboxGroup::Drawing,ClickMode::LinearCopy)) {
                    state.click_mode = ClickMode::LinearCopy;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;

                }

                if (magic_magic(keybinds.ROTATE_COPY,"RotateCopy",false,ToolboxGroup::Drawing,ClickMode::RotateCopy)) {
                    state.click_mode = ClickMode::RotateCopy;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;

                }

                SEPERATOR(ToolboxGroup::Drawing);


                if (magic_magic(keybinds.FILLET,"Fillet",false,ToolboxGroup::Drawing,ClickMode::Fillet)) {
                    popup->manager.manually_set_focus_group(ToolboxGroup::Drawing);
                    state.click_mode = ClickMode::Fillet;
                    state.click_modifier = ClickModifier::None;
                    state.enter_mode = EnterMode::None;
                    two_click_command->awaiting_second_click = false;

                }

                if (magic_magic(keybinds.POWER_FILLET,"PowerFillet",false,ToolboxGroup::Drawing,ClickMode::PowerFillet)) {
                    state.click_mode = ClickMode::PowerFillet;
                    state.click_modifier = ClickModifier::None;
                    state.enter_mode = EnterMode::None;

                }

                if (magic_magic(keybinds.TWO_CLICK_DIVIDE,"Divide2",false,ToolboxGroup::Drawing,ClickMode::TwoClickDivide)) {
                    state.click_mode = ClickMode::TwoClickDivide;
                    state.click_modifier = ClickModifier::None;
                    state.enter_mode = EnterMode::None;
                    two_click_command->awaiting_second_click = false;

                }

                SEPERATOR(ToolboxGroup::Drawing);

                if (magic_magic(keybinds.CHANGE_ORIGIN,"Origin",false,ToolboxGroup::Drawing,ClickMode::Origin)) {
                    state.click_mode = ClickMode::Origin;
                    state.click_modifier = ClickModifier::None;

                }

                if (magic_magic(keybinds.AXIS,"Axis",false,ToolboxGroup::Drawing,ClickMode::Axis)) {
                    state.click_mode = ClickMode::Axis;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;

                } 

                SEPERATOR(ToolboxGroup::Drawing);
                SEPERATOR(ToolboxGroup::Drawing);
                SEPERATOR(ToolboxGroup::Drawing);

                ////////////////////////////////////////////////////////////////////////////////


                ////////////////////////////////////////////////////////////////////////////////

                if (magic_magic(keybinds.CYCLE_FEATURE_PLANE,"Plane",false,ToolboxGroup::Mesh)) {
                    // TODO: 'Y' remembers last terminal choice of plane for next time
                    result.checkpoint_me = true;
                    other.time_since_plane_selected = 0.0f;

                    // already one of the three primary planes
                    if ((feature_plane->is_active) && ARE_EQUAL(feature_plane->signed_distance_to_world_origin, 0.0f) && ARE_EQUAL(squaredNorm(feature_plane->normal), 1.0f) && ARE_EQUAL(maxComponent(feature_plane->normal), 1.0f)) {
                        feature_plane->normal = { feature_plane->normal[2], feature_plane->normal[0], feature_plane->normal[1] };
                    } else {
                        feature_plane->is_active = true;
                        feature_plane->signed_distance_to_world_origin = 0.0f;
                        feature_plane->normal = { 0.0f, 1.0f, 0.0f };
                    }

                }

                SEPERATOR(ToolboxGroup::Mesh);

                if (magic_magic(keybinds.EXTRUDE_ADD,"ExtrudeAdd",false,ToolboxGroup::Mesh,ClickMode::None,ClickModifier::None,EnterMode::ExtrudeAdd)) {
                    popup->manager.manually_set_focus_group(ToolboxGroup::Mesh);
                    state.enter_mode = EnterMode::ExtrudeAdd;
                    preview->extrude_in_length = 0; // FORNOW
                    preview->extrude_out_length = 0; // FORNOW

                }

                if (magic_magic(keybinds.EXTRUDE_CUT, "ExtrudeCut",false,ToolboxGroup::Mesh,ClickMode::None,ClickModifier::None,EnterMode::ExtrudeCut)) {
                    popup->manager.manually_set_focus_group(ToolboxGroup::Mesh);
                    state.enter_mode = EnterMode::ExtrudeCut;
                    preview->extrude_in_length = 0; // FORNOW
                    preview->extrude_out_length = 0; // FORNOW

                }

                if (magic_magic(keybinds.REVOLVE_ADD, "RevolveAdd",false,ToolboxGroup::Mesh,ClickMode::None,ClickModifier::None,EnterMode::RevolveAdd)) {
                    popup->manager.manually_set_focus_group(ToolboxGroup::Mesh);
                    state.enter_mode = EnterMode::RevolveAdd;
                    preview->revolve_in_angle = 0; // FORNOW
                    preview->revolve_out_angle = 0; // FORNOW

                }

                if (magic_magic(keybinds.REVOLVE_CUT, "RevolveCut",false,ToolboxGroup::Mesh,ClickMode::None,ClickModifier::None,EnterMode::RevolveCut)) {
                    popup->manager.manually_set_focus_group(ToolboxGroup::Mesh);
                    state.enter_mode = EnterMode::RevolveCut;
                    preview->revolve_in_angle = 0; // FORNOW
                    preview->revolve_out_angle = 0; // FORNOW

                }

                SEPERATOR(ToolboxGroup::Mesh);

                if (magic_magic(keybinds.NUDGE_PLANE, "NudgePlane",false,ToolboxGroup::Mesh,ClickMode::None,ClickModifier::None,EnterMode::NudgePlane)) {
                    popup->manager.manually_set_focus_group(ToolboxGroup::Mesh);
                    if (feature_plane->is_active) {
                        state.enter_mode = EnterMode::NudgePlane;
                        preview->feature_plane_offset = 0.0f; // FORNOW
                    } else {
                        messagef(omax.orange, "NudgePlane: no feature plane selected");
                    }

                }

                ////////////////////////////////////////////////////////////////////////////////


                bool hotkey_quality;
                uint digit = 0;
                Keybind *qualities[10] = { &keybinds.QUALITY_0, &keybinds.QUALITY_1, &keybinds.QUALITY_2, &keybinds.QUALITY_3, &keybinds.QUALITY_4, &keybinds.QUALITY_5, &keybinds.QUALITY_6, &keybinds.QUALITY_7, &keybinds.QUALITY_8, &keybinds.QUALITY_9 };
                {
                    hotkey_quality = false;
                    for_(color, 10) {
                        if (magic_magic(*qualities[color])) {
                            hotkey_quality = true;
                            digit = color;
                            break;
                        }
                    }
                }
                #undef CHECK_COLOR

                if (hotkey_quality) {
                    if (click_mode_SELECT_OR_DESELECT() && (state.click_modifier == ClickModifier::Color)) { // [sd]q0
                        _for_each_entity_ {
                            uint i = uint(entity->color_code);
                            if (i != digit) continue;
                            cookbook.entity_set_is_selected(entity, value_to_write_to_selection_mask);
                        }
                        state.click_mode = ClickMode::None;
                        state.click_modifier = ClickModifier::None;
                    } else if ((state.click_mode == ClickMode::Color) && (state.click_modifier == ClickModifier::Selected)) { // qs0
                        _for_each_selected_entity_ cookbook.entity_set_color(entity, ColorCode(digit));
                        state.click_mode = ClickMode::None;
                        state.click_modifier = ClickModifier::None;
                        _for_each_entity_ entity->is_selected = false;
                    } else { // 0
                        result.record_me = true;
                        state.click_mode = ClickMode::Color;
                        state.click_modifier = ClickModifier::None;
                        state.click_color_code = ColorCode(digit);
                    }
                }

                if (magic_magic(keybinds.TWO_EDGE_CIRCLE)) {
                    state.click_mode = ClickMode::TwoEdgeCircle;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;
                }


                if (magic_magic(keybinds.DIVIDE_NEAREST)) {
                    state.click_mode = ClickMode::DivideNearest;
                    state.click_modifier = ClickModifier::None;

                }



                if (magic_magic(keybinds.TOGGLE_GRID)) {
                    result.record_me = false;
                    other.hide_grid = !other.hide_grid;

                }

                if (magic_magic(keybinds.PRINT_HISTORY)) {
                    result.record_me = false;
                    history_printf_script();

                }

                if (magic_magic(keybinds.OFFSET)) {
                    state.click_mode = ClickMode::Offset;
                    state.click_modifier = ClickModifier::None;
                }

                if (magic_magic(keybinds.TOGGLE_EVENT_STACK)) { 
                    result.record_me = false;
                    other.show_event_stack = !other.show_event_stack;
                    result.record_me = false;

                }

                if (magic_magic(keybinds.MEASURE)) {
                    result.record_me = false;
                    state.click_mode = ClickMode::Measure;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;

                }
                if (magic_magic(keybinds.MIRROR_LINE)) {
                    state.click_mode = ClickMode::MirrorLine;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;

                }


                if (magic_magic(keybinds.CLEAR_DRAWING)) {
                    result.checkpoint_me = true;
                    result.snapshot_me = true;
                    list_free_AND_zero(&drawing->entities);
                    *drawing = {};
                    messagef(omax.green, "ResetDXF");

                }

                if (magic_magic(keybinds.CLEAR_MESH)) {
                    result.checkpoint_me = true;
                    result.snapshot_me = true;
                    mesh_free_AND_zero(mesh);
                    *feature_plane = {};
                    messagef(omax.green, "ResetSTL");

                }


                if (magic_magic(keybinds.LOAD_FILE)) {
                    state.enter_mode = EnterMode::Load;

                }








                if (magic_magic(keybinds.SAVE)) {
                    result.record_me = false;
                    state.enter_mode = EnterMode::Save;
                    other.awaiting_confirmation = false;
                }

                if (magic_magic(keybinds.SAVE_AS)) {
                    result.record_me = false;
                    state.enter_mode = EnterMode::SaveAs;
                    other.awaiting_confirmation = false;
                }

                if (magic_magic(keybinds.RESIZE)) { 
                    state.enter_mode = EnterMode::Size;

                }

                if (magic_magic(keybinds.MIRROR_X)) {
                    state.click_mode = ClickMode::MirrorX;
                    state.click_modifier = ClickModifier::None;

                }

                if (magic_magic(keybinds.DRAWING_FRAME)) {
                    result.record_me = false;
                    init_camera_drawing();
                    init_camera_mesh();

                }


                if (magic_magic(keybinds.MIRROR_Y)) {
                    state.click_mode = ClickMode::MirrorY;
                    state.click_modifier = ClickModifier::None;

                }

                if (magic_magic(keybinds.PREVIOUS_HOT_KEY_2D)) {
                    state.click_mode = ClickMode::None; // FORNOW: patching space space doing CIRCLE CENTER

                    return _standard_event_process_NOTE_RECURSIVE(state.space_bar_event);
                }

                if (magic_magic(keybinds.PREVIOUS_HOT_KEY_3D)) {

                    return _standard_event_process_NOTE_RECURSIVE(state.shift_space_bar_event);
                }



                if (magic_magic(keybinds.TOGGLE_DRAWING_DETAILS)) { 
                    result.record_me = false;
                    other.show_details = !other.show_details;
                    { // messagef
                        uint num_lines;
                        uint num_arcs;
                        {
                            num_lines = 0;
                            num_arcs = 0;
                            _for_each_entity_ {
                                if (entity->type == EntityType::Line) {
                                    ++num_lines;
                                } else { ASSERT(entity->type == EntityType::Arc);
                                    ++num_arcs;
                                }
                            }
                        }
                        messagef(omax.cyan,"Mesh has %d triangles", mesh->num_triangles);
                        messagef(omax.cyan,"Drawing has %d elements = %d lines + %d arcs", drawing->entities.length, num_lines, num_arcs);
                    }

                }

                if (magic_magic(keybinds.TOGGLE_FEATURE_PLANE)) {
                    result.checkpoint_me = true;
                    if (feature_plane->is_active) other.time_since_plane_deselected = 0.0f;
                    feature_plane->is_active = false;

                }

                if (magic_magic(keybinds.ZOOM_3D_CAMERA)) {
                    result.record_me = false;
                    other.camera_mesh.angle_of_view = CAMERA_3D_PERSPECTIVE_ANGLE_OF_VIEW - other.camera_mesh.angle_of_view;

                }

                if ((magic_magic(keybinds.DELETE_SELECTED) || magic_magic(keybinds.DELETE_SELECTED_ALTERNATE))) {
                    // trust me you want this code (imagine deleting stuff while in the middle of a two click command)
                    state.click_mode = ClickMode::None;
                    state.click_modifier = ClickModifier::None;

                    for (int i = drawing->entities.length - 1; i >= 0; --i) {
                        if (drawing->entities.array[i].is_selected) {
                            cookbook._delete_entity(i);
                        }
                    }

                }

                if (magic_magic(keybinds.HELP_MENU)) {
                    result.record_me = false;
                    other.show_help = !other.show_help;

                }

                if (magic_magic(keybinds.EXIT_COMMAND)) {
                    state.enter_mode = EnterMode::None;
                    state.click_mode = ClickMode::None;
                    state.click_modifier = ClickModifier::None;
                    state.click_color_code = ColorCode::Traverse;

                }

                if (magic_magic(keybinds.TOGGLE_LIGHT_MODE)) { // FORNOW
                    result.record_me = false;
                    {
                        vec3 tmp = omax.light_gray;
                        omax.light_gray = omax.dark_gray;
                        omax.dark_gray = tmp;
                    }
                    {
                        vec3 tmp = omax.white;
                        omax.white = omax.black;
                        omax.black = tmp;
                    }
                    {
                        vec3 tmp = omax.yellow;
                        omax.yellow = omax.dark_yellow;
                        omax.dark_yellow = tmp;
                    }

                }

                if (magic_magic(keybinds.TOGGLE_BUTTONS)) { // FORNOW
                    result.record_me = false;
                    other.hide_toolbox = !other.hide_toolbox;

                }



                if (magic_magic(keybinds.EXECUTE_COMMAND)) { // FORNOW
                    result.record_me = false;

                }

                if (magic_magic(KEYBIND(DUMMY_HOTKEY, 0))) { // FORNOW
                    result.record_me = false;

                }

                if (magic_magic(KEYBIND(0, 0))) { // FORNOW
                    ; 
                }

                if (!hotkey_recognized) {
                    messagef(omax.orange, "Hotkey: %s not recognized", key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(key_event), key_event->control, key_event->shift, key_event->alt, key_event->key);
                    result.record_me = false;
                }


            }
            bool changed_click_mode = (prev_click_mode != state.click_mode);
            bool changed_enter_mode = (prev_enter_mode != state.enter_mode);
            if (changed_click_mode && click_mode_SPACE_BAR_REPEAT_ELIGIBLE()) state.space_bar_event = event;
            if (changed_enter_mode && enter_mode_SHIFT_SPACE_BAR_REPEAT_ELIGIBLE()) state.shift_space_bar_event = event;
        } else { ASSERT(key_event->subtype == KeyEventSubtype::Popup);
            // NOTE: this case has been moved into popup_popup();
            result.record_me = true; // FORNOW
        }
    } else if (event.type == EventType::Mouse) {
        MouseEvent *mouse_event = &event.mouse_event;
        if (mouse_event->subtype == MouseEventSubtype::Drawing) {
            MouseEventDrawing *mouse_event_drawing = &mouse_event->mouse_event_drawing;

            result.record_me = true;
            if (state.click_mode == ClickMode::Measure) result.record_me = false;
            if (mouse_event->mouse_held) result.record_me = false;

            MagicSnapResult snap_result = mouse_event_drawing->snap_result;
            vec2 *mouse = &snap_result.mouse_position;

            if (snap_result.snapped) {
                //messagef(omax.red, "SNAPPEEDD");
                //snap_result.entity_snapped_to->color_code = ColorCode::Quality4;
                //cookbook.buffer_add_line(V2(0, 0), *mouse);
                cookbook.divide_entity_at_point(snap_result.entity_snapped_to, *mouse);
            }

            bool click_mode_WINDOW_SELECT_OR_WINDOW_DESELECT = (click_mode_SELECT_OR_DESELECT() && (state.click_modifier == ClickModifier::Window));

            // TODO: move to misc.cpp; TODO: rename misc.cpp -> bool.cpp?
            bool click_mode_TWO_CLICK_COMMAND = 0 ||
                (state.click_mode == ClickMode::Axis) ||
                (state.click_mode == ClickMode::Box) ||
                (state.click_mode == ClickMode::CenteredBox) ||
                (state.click_mode == ClickMode::Circle) ||
                (state.click_mode == ClickMode::Fillet) ||
                (state.click_mode == ClickMode::Line) ||
                (state.click_mode == ClickMode::LinearCopy) ||
                (state.click_mode == ClickMode::Measure) ||
                (state.click_mode == ClickMode::MirrorLine) ||
                (state.click_mode == ClickMode::Move) ||
                (state.click_mode == ClickMode::Polygon) ||
                (state.click_mode == ClickMode::Rotate) ||
                (state.click_mode == ClickMode::RotateCopy) ||
                (state.click_mode == ClickMode::TwoEdgeCircle) ||
                (state.click_mode == ClickMode::TwoClickDivide) ||
                click_mode_WINDOW_SELECT_OR_WINDOW_DESELECT; // fornow wonky case

            // fornow window wonky case
            if (_non_WINDOW__SELECT_DESELECT___OR___SET_COLOR()) { // NOTES: includes scand qc
                result.record_me = false;
                DXFFindClosestEntityResult dxf_find_closest_entity_result = dxf_find_closest_entity(&drawing->entities, mouse_event_drawing->snap_result.mouse_position);
                if (dxf_find_closest_entity_result.success) {
                    Entity *hot_entity = dxf_find_closest_entity_result.closest_entity;
                    if (state.click_modifier != ClickModifier::Connected) {
                        if (click_mode_SELECT_OR_DESELECT()) {
                            cookbook.entity_set_is_selected(hot_entity, value_to_write_to_selection_mask);
                        } else {
                            cookbook.entity_set_color(hot_entity, state.click_color_code);
                        }
                    } else {
                        #if 1 // TODO: consider just using the O(n*m) algorithm here instead

                        #define GRID_CELL_WIDTH 0.001f

                        auto scalar_bucket = [&](real a) -> real {
                            return roundf(a / GRID_CELL_WIDTH) * GRID_CELL_WIDTH;
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
                            GridPointSlot slots[2];
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
                                entity_get_start_and_end_points(entity, &start, &end);
                                push_into_grid_unless_cell_full__make_cell_if_none_exists(start, entity_index, false);
                                push_into_grid_unless_cell_full__make_cell_if_none_exists(end, entity_index, true);
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
                                if (end_NOT_start) {
                                    p = entity_get_end_point(entity);
                                } else {
                                    p = entity_get_start_point(entity);
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


                        uint hot_entity_index = hot_entity - drawing->entities.array;

                        // NOTE: we will mark the hot entity, and then shoot off from both its endpoints
                        edge_marked[hot_entity_index] = true;
                        cookbook.entity_set_is_selected(&drawing->entities.array[hot_entity_index], value_to_write_to_selection_mask);

                        for_(pass, 2) {

                            vec2 seed; {
                                vec2 p;
                                if (pass == 0) {
                                    p = entity_get_start_point(&drawing->entities.array[hot_entity_index]);
                                } else {
                                    p = entity_get_end_point(&drawing->entities.array[hot_entity_index]);
                                }
                                seed = make_key(p);
                            }

                            GridPointSlot *curr = get_any_point_not_part_of_an_marked_entity(seed);
                            while (true) {
                                if (curr == NULL) break;
                                cookbook.entity_set_is_selected(&drawing->entities.array[curr->entity_index], value_to_write_to_selection_mask);
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
                        uint loop_index = dxf_pick_loops.loop_index_from_entity_index[hot_entity_index];
                        DXFEntityIndexAndFlipFlag *loop = dxf_pick_loops.loops[loop_index];
                        uint num_entities = dxf_pick_loops.num_entities_in_loops[loop_index];
                        for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < &loop[num_entities]; ++entity_index_and_flip_flag) {
                            cookbook.entity_set_is_selected(&drawing->entities[entity_index_and_flip_flag->entity_index], value_to_write_to_selection_mask);
                        }
                        #endif
                    }
                }
            } else if (!mouse_event->mouse_held) {
                if (click_mode_TWO_CLICK_COMMAND) {

                    if (!two_click_command->awaiting_second_click) {
                        DXFFindClosestEntityResult find_nearest_result = dxf_find_closest_entity(&drawing->entities, mouse_event_drawing->snap_result.mouse_position);
                        bool first_click_accepted; {
                            bool first_click_must_acquire_entity = (
                                    0 ||
                                    (state.click_mode == ClickMode::Fillet) ||
                                    (state.click_mode == ClickMode::TwoClickDivide)
                                    );
                            if (!first_click_must_acquire_entity) {
                                first_click_accepted = true;
                            } else {
                                first_click_accepted = find_nearest_result.success;
                            }
                        }
                        if (first_click_accepted) {
                            two_click_command->awaiting_second_click = true;
                            two_click_command->first_click = mouse_event_drawing->snap_result.mouse_position;
                            two_click_command->entity_closest_to_first_click = find_nearest_result.closest_entity;
                            if (state.click_modifier != ClickModifier::Window) state.click_modifier = ClickModifier::None;
                            { // bump bumps cursor bump cursor bumps
                                if (state.click_mode == ClickMode::Rotate) {
                                    double xpos, ypos;
                                    glfwGetCursorPos(glfw_window, &xpos, &ypos);
                                    real x_new = xpos + 64;
                                    real y_new = ypos;
                                    glfwSetCursorPos(glfw_window, x_new, y_new);
                                    callback_cursor_position(glfw_window, x_new, y_new);
                                }
                                if (state.click_mode == ClickMode::Axis) {
                                    double xpos, ypos;
                                    glfwGetCursorPos(glfw_window, &xpos, &ypos);
                                    real theta = (PI / 2) + drawing->axis_angle_from_y;
                                    real r = 64;
                                    real x_new = xpos + r * COS(theta);
                                    real y_new = ypos - r * SIN(theta);
                                    glfwSetCursorPos(glfw_window, x_new, y_new);
                                    callback_cursor_position(glfw_window, x_new, y_new);
                                }
                            }
                        }
                    } else {
                        vec2 *first_click = &two_click_command->first_click;
                        vec2 *second_click = mouse;
                        vec2 click_vector = (*second_click - *first_click);
                        real click_theta = angle_from_0_TAU(*first_click, *second_click);

                        if (0) {
                        } else if (state.click_mode == ClickMode::Axis) {
                            // two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            drawing->axis_base_point = *first_click;
                            drawing->axis_angle_from_y = (-PI / 2) + click_theta;
                        } else if (state.click_mode == ClickMode::Box) {
                            if (IS_ZERO(ABS(first_click->x - second_click->x))) {
                                messagef(omax.orange, "Box: must have non-zero width ");
                            } else if (IS_ZERO(ABS(first_click->y - second_click->y))) {
                                messagef(omax.orange, "Box: must have non-zero height");
                            } else {
                                // two_click_command->awaiting_second_click = false;
                                result.checkpoint_me = true;
                                state.click_mode = ClickMode::None;
                                state.click_modifier = ClickModifier::None;
                                vec2 other_corner_A = { first_click->x, second_click->y };
                                vec2 other_corner_B = { second_click->x, first_click->y };
                                cookbook.buffer_add_line(*first_click,  other_corner_A);
                                cookbook.buffer_add_line(*first_click,  other_corner_B);
                                cookbook.buffer_add_line(*second_click, other_corner_A);
                                cookbook.buffer_add_line(*second_click, other_corner_B);
                            }
                        } else if (state.click_mode == ClickMode::CenteredBox) {
                            if (IS_ZERO(ABS(first_click->x - second_click->x))) {
                                messagef(omax.orange, "Box: must have non-zero width ");
                            } else if (IS_ZERO(ABS(first_click->y - second_click->y))) {
                                messagef(omax.orange, "Box: must have non-zero height");
                            } else {
                                // two_click_command->awaiting_second_click = false;
                                result.checkpoint_me = true;
                                state.click_mode = ClickMode::None;
                                state.click_modifier = ClickModifier::None;
                                vec2 one_corner = *second_click;
                                vec2 center = *first_click;
                                real other_y = 2 * center.y - one_corner.y;
                                real other_x = 2 * center.x - one_corner.x;
                                cookbook.buffer_add_line(one_corner, V2(one_corner.x, other_y));
                                cookbook.buffer_add_line(V2(one_corner.x, other_y),  V2(other_x, other_y));
                                cookbook.buffer_add_line(V2(other_x, other_y), V2(other_x, one_corner.y));
                                cookbook.buffer_add_line(V2(other_x, one_corner.y), one_corner);
                            }
                        } else if (state.click_mode == ClickMode::Fillet) {
                            result.checkpoint_me = true;

                            state.click_modifier = ClickModifier::None;
                            two_click_command->awaiting_second_click = false;
                            DXFFindClosestEntityResult _F = dxf_find_closest_entity(&drawing->entities, *second_click);
                            if (_F.success) {
                                Entity *E = two_click_command->entity_closest_to_first_click;
                                Entity *F = _F.closest_entity;
                                cookbook.fillet_two_entities_from_point(E, F, second_click, popup->fillet_radius);
                            }
                        } else if (state.click_mode == ClickMode::Circle) {
                            if (IS_ZERO(norm(*first_click - *second_click))) {
                                messagef(omax.orange, "Circle: must have non-zero diameter");
                            } else {
                                two_click_command->awaiting_second_click = false;
                                result.checkpoint_me = true;
                                state.click_mode = ClickMode::None;
                                state.click_modifier = ClickModifier::None;
                                real theta_a_in_degrees = DEG(click_theta);
                                real theta_b_in_degrees = theta_a_in_degrees + 180.0f;
                                real r = norm(*second_click - *first_click);
                                cookbook.buffer_add_arc(*first_click, r, theta_a_in_degrees, theta_b_in_degrees);
                                cookbook.buffer_add_arc(*first_click, r, theta_b_in_degrees, theta_a_in_degrees);
                                // messagef(omax.green, "Circle");
                            }
                        } else if (state.click_mode == ClickMode::TwoEdgeCircle) {
                            if (IS_ZERO(norm(*first_click - *second_click))) {
                                messagef(omax.orange, "TwoEdgeCircle: must have non-zero diameter");
                            } else {
                                // two_click_command->awaiting_second_click = false;
                                result.checkpoint_me = true;
                                state.click_mode = ClickMode::None;
                                state.click_modifier = ClickModifier::None;
                                vec2 center = (*second_click + *first_click) / 2;
                                real theta_a_in_degrees = DEG(ATAN2(*second_click - center));
                                real theta_b_in_degrees = theta_a_in_degrees + 180.0f;
                                real radius = norm(*second_click - *first_click) / 2;
                                cookbook.buffer_add_arc(center, radius, theta_a_in_degrees, theta_b_in_degrees);
                                cookbook.buffer_add_arc(center, radius, theta_b_in_degrees, theta_a_in_degrees);
                                // messagef(omax.green, "Circle");
                            }
                        } else if (state.click_mode == ClickMode::TwoClickDivide) { // TODO: make sure no 0 length shenanigans
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            // two_click_command->awaiting_second_click = false;

                            Entity *closest_entity_one = two_click_command->entity_closest_to_first_click; 
                            DXFFindClosestEntityResult closest_result_two = dxf_find_closest_entity(&drawing->entities, *second_click);
                            if (closest_result_two.success) {
                                Entity *closest_entity_two = closest_result_two.closest_entity;
                                if (closest_entity_one == closest_entity_two) {
                                    messagef(omax.orange, "TwoClickDivide: clicked same entity twice");
                                } else {
                                    if (closest_entity_one->type == EntityType::Line && closest_entity_two->type == EntityType::Line) {
                                        LineEntity segment_one = closest_entity_one->line;
                                        LineEntity segment_two = closest_entity_two->line;
                                        vec2 a = segment_one.start;
                                        vec2 b = segment_one.end;
                                        vec2 c = segment_two.start;
                                        vec2 d = segment_two.end;
                                        LineLineXResult X_result = line_line_intersection(a, b, c, d);
                                        bool neither_line_extension_hits_the_other_segment = ((!X_result.point_is_on_segment_ab) && (!X_result.point_is_on_segment_cd));
                                        if (neither_line_extension_hits_the_other_segment) {
                                            messagef(omax.orange, "TwoClickDivide: no intersection found");
                                        } else {
                                            if (X_result.point_is_on_segment_ab) {
                                                cookbook.buffer_add_line(X_result.point, a, false, closest_entity_one->color_code);
                                                cookbook.buffer_add_line(X_result.point, b, false, closest_entity_one->color_code);
                                                cookbook.buffer_delete_entity(closest_entity_one);
                                            }
                                            if (X_result.point_is_on_segment_cd) {
                                                cookbook.buffer_add_line(X_result.point, c, false, closest_entity_two->color_code);
                                                cookbook.buffer_add_line(X_result.point, d, false, closest_entity_two->color_code);
                                                cookbook.buffer_delete_entity(closest_entity_two);
                                            } 
                                        }
                                    } else if (closest_entity_one->type == EntityType::Arc && closest_entity_two->type == EntityType::Arc){
                                        // math for this is here https://paulbourke.net/geometry/circlesphere/

                                        ArcEntity arcA = closest_entity_one->arc;
                                        ArcEntity arcB = closest_entity_two->arc;

                                        ArcArcXResult arc_x_arc_result = arc_arc_intersection(&arcA, &arcB);

                                        bool p1Works = arc_x_arc_result.point1_is_on_arc_a || arc_x_arc_result.point1_is_on_arc_b; 
                                        bool p2Works = arc_x_arc_result.point2_is_on_arc_a || arc_x_arc_result.point2_is_on_arc_b; 
                                        bool cut_arc_a = false;
                                        bool cut_arc_b = false;
                                        real theta_a = 0;
                                        real theta_b = 0;

                                        if (p1Works) {
                                            real click_to_p1 = distance(arc_x_arc_result.point1, *second_click);
                                            real click_to_p2 = distance(arc_x_arc_result.point2, *second_click);
                                            if (p2Works && click_to_p2 < click_to_p1) { 
                                                theta_a = arc_x_arc_result.theta_2a;
                                                theta_b = arc_x_arc_result.theta_2b;
                                                cut_arc_a = arc_x_arc_result.point2_is_on_arc_a;
                                                cut_arc_b = arc_x_arc_result.point2_is_on_arc_b;
                                            } else {
                                                theta_a = arc_x_arc_result.theta_1a;
                                                theta_b = arc_x_arc_result.theta_1b;
                                                cut_arc_a = arc_x_arc_result.point1_is_on_arc_a;
                                                cut_arc_b = arc_x_arc_result.point1_is_on_arc_b;
                                            }
                                        } else if (p2Works) {
                                            theta_a = arc_x_arc_result.theta_2a;
                                            theta_b = arc_x_arc_result.theta_2b;
                                            cut_arc_a = arc_x_arc_result.point2_is_on_arc_a;
                                            cut_arc_b = arc_x_arc_result.point2_is_on_arc_b;
                                        }
                                        if (cut_arc_a) {
                                            cookbook.buffer_add_arc(arcA.center, arcA.radius, arcA.start_angle_in_degrees, theta_a, false, closest_entity_one->color_code);
                                            cookbook.buffer_add_arc(arcA.center, arcA.radius, theta_a, arcA.end_angle_in_degrees, false, closest_entity_one->color_code);
                                            cookbook.buffer_delete_entity(closest_entity_one);
                                        }
                                        if (cut_arc_b) {
                                            cookbook.buffer_add_arc(arcB.center, arcB.radius, arcB.start_angle_in_degrees, theta_b, false, closest_entity_two->color_code);
                                            cookbook.buffer_add_arc(arcB.center, arcB.radius, theta_b, arcB.end_angle_in_degrees, false, closest_entity_two->color_code);
                                            cookbook.buffer_delete_entity(closest_entity_two);
                                        }
                                        if (!cut_arc_a && !cut_arc_b) {
                                            messagef(omax.orange, "TwoClickDivide: no intersection found");
                                        }
                                    } else { // TODO: ASSERT(...); //ASSERT((closest_entity_two->type == EntityType::Line && closest_entity_two->type == EntityType::Arc) // kinda nasty but only way 
                                             //       || (closest_entity_two->type == EntityType::Arc && closest_entity_two->type == EntityType::Line));
                                        Entity *entity_arc;
                                        Entity *entity_line;
                                        if (closest_entity_one->type == EntityType::Arc) {
                                            entity_arc = closest_entity_one;
                                            entity_line = closest_entity_two;
                                        } else {
                                            entity_arc = closest_entity_two;
                                            entity_line = closest_entity_one;
                                        }
                                        ArcEntity *arc = &entity_arc->arc;
                                        LineEntity *line = &entity_line->line;

                                        LineArcXResult line_x_arc_result = line_arc_intersection(line, arc);
                                        bool p1Works = line_x_arc_result.point1_is_on_arc || line_x_arc_result.point1_is_on_line_segment;
                                        bool p2Works = line_x_arc_result.point2_is_on_arc || line_x_arc_result.point2_is_on_line_segment;

                                        vec2 intersect = {};
                                        real theta = 0;
                                        bool cutLine = false;
                                        bool cutArc = false;

                                        if (p1Works) {
                                            real click_to_p1 = distance(line_x_arc_result.point1, *second_click);
                                            real click_to_p2 = distance(line_x_arc_result.point2, *second_click);
                                            if (p2Works && click_to_p2 < click_to_p1) { 
                                                intersect = line_x_arc_result.point2;
                                                theta = line_x_arc_result.theta_2;
                                                cutLine = line_x_arc_result.point2_is_on_line_segment;
                                                cutArc = line_x_arc_result.point2_is_on_arc;
                                            } else {
                                                intersect = line_x_arc_result.point1;
                                                theta = line_x_arc_result.theta_1;
                                                cutLine = line_x_arc_result.point1_is_on_line_segment;
                                                cutArc = line_x_arc_result.point1_is_on_arc;
                                            }
                                        } else if (p2Works) {
                                            intersect = line_x_arc_result.point2;
                                            theta = line_x_arc_result.theta_2;
                                            cutLine = line_x_arc_result.point2_is_on_line_segment;
                                            cutArc = line_x_arc_result.point2_is_on_arc;
                                        }

                                        if (p1Works || p2Works) {
                                            if (cutLine) {
                                                cookbook.buffer_add_line(intersect, line->start, false, entity_line->color_code);
                                                cookbook.buffer_add_line(intersect, line->end, false, entity_line->color_code);
                                                cookbook.buffer_delete_entity(entity_line);
                                            }
                                            if (cutArc) {
                                                cookbook.buffer_add_arc(arc->center, arc->radius, arc->start_angle_in_degrees, theta, false, entity_arc->color_code);
                                                cookbook.buffer_add_arc(arc->center, arc->radius, theta, arc->end_angle_in_degrees, false, entity_arc->color_code);
                                                cookbook.buffer_delete_entity(entity_arc);
                                            }
                                        }
                                        if (!cutArc && !cutLine) {
                                            messagef(omax.orange, "TwoClickDivide: no intersection found");
                                        }
                                    }
                                }
                            }
                        } else if (state.click_mode == ClickMode::Line) {
                            // two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            cookbook.buffer_add_line(*first_click, *second_click);
                        } else if (state.click_mode == ClickMode::Measure) {
                            // two_click_command->awaiting_second_click = false;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            real angle = DEG(click_theta);
                            real length = norm(*second_click - *first_click);
                            messagef(omax.cyan, "Angle is %gdeg.", angle);
                            messagef(omax.cyan, "Length is %gmm.", length);
                        } else if (state.click_mode == ClickMode::MirrorLine) {
                            // two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;

                            real theta = ATAN2(click_vector);
                            real theta_in_degrees = DEG(theta);

                            auto Q = [theta, first_click](vec2 p) {
                                p -= *first_click;
                                p = rotated(p, -theta);
                                p = cwiseProduct(V2(1, -1), p);
                                p = rotated(p, theta);
                                p += *first_click;
                                return p;
                            };

                            auto R = [theta_in_degrees](real angle_in_degrees) {
                                return -(angle_in_degrees - theta_in_degrees) + theta_in_degrees;
                            };

                            _for_each_selected_entity_ {
                                if (entity->type == EntityType::Line) {
                                    LineEntity *line = &entity->line;
                                    cookbook.buffer_add_line(Q(line->start), Q(line->end), true, entity->color_code);
                                } else { ASSERT(entity->type == EntityType::Arc);
                                    ArcEntity *arc = &entity->arc;
                                    cookbook.buffer_add_arc(Q(arc->center), arc->radius, R(arc->end_angle_in_degrees), R(arc->start_angle_in_degrees), true, entity->color_code);
                                }
                                entity->is_selected = false;
                            }
                        } else if (state.click_mode == ClickMode::Rotate) {
                            // two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            _for_each_selected_entity_ {
                                if (entity->type == EntityType::Line) {
                                    LineEntity *line = &entity->line;
                                    line->start = rotated_about(line->start, *first_click, click_theta);
                                    line->end = rotated_about(line->end, *first_click, click_theta);
                                } else { ASSERT(entity->type == EntityType::Arc);
                                    ArcEntity *arc = &entity->arc;
                                    arc->center = rotated_about(arc->center, *first_click, click_theta);
                                    arc->start_angle_in_degrees = DEG(click_theta) + arc->start_angle_in_degrees;
                                    arc->end_angle_in_degrees = DEG(click_theta) + arc->end_angle_in_degrees;
                                }
                            }
                        } else if (state.click_mode == ClickMode::RotateCopy) {
                            // two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;

                            uint num_total_copies = popup->rotate_copy_num_total_copies;
                            real theta_deg = popup->rotate_copy_angle;
                            if (IS_ZERO(theta_deg)) theta_deg = 180.0f;
                            real theta_rad = RAD(theta_deg);

                            _for_each_selected_entity_ {
                                Entity oldEntity = *entity;
                                for_(j, num_total_copies - 1) {
                                    Entity new_entity = oldEntity;
                                    if (entity->type == EntityType::Line) {
                                        LineEntity *line = &new_entity.line;
                                        line->start = rotated_about(line->start, *first_click, theta_rad);
                                        line->end = rotated_about(line->end, *first_click, theta_rad);
                                    } else { ASSERT(entity->type == EntityType::Arc);
                                        ArcEntity *arc = &new_entity.arc;
                                        arc->center = rotated_about(arc->center, *first_click, theta_rad);
                                        arc->start_angle_in_degrees = theta_deg + arc->start_angle_in_degrees;
                                        arc->end_angle_in_degrees = theta_deg + arc->end_angle_in_degrees;
                                    }
                                    cookbook._buffer_add_entity(new_entity);
                                    oldEntity = new_entity;
                                }
                            }
                        } else if (state.click_mode == ClickMode::Move) {
                            // two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            _for_each_selected_entity_ {
                                if (entity->type == EntityType::Line) {
                                    LineEntity *line = &entity->line;
                                    line->start += click_vector;
                                    line->end   += click_vector;
                                } else { ASSERT(entity->type == EntityType::Arc);
                                    ArcEntity *arc = &entity->arc;
                                    arc->center += click_vector;
                                }
                            }
                        } else if (state.click_mode == ClickMode::LinearCopy) {
                            // two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            uint num_additional_copies = MAX(1U, popup->linear_copy_num_additional_copies);
                            for_(i, num_additional_copies) {
                                vec2 displacement = real(i + 1) * click_vector;
                                bool is_selected = (i == num_additional_copies - 1);
                                _for_each_selected_entity_ {
                                    Entity new_entity; {
                                        new_entity = *entity;
                                        new_entity.is_selected = is_selected;
                                        if (entity->type == EntityType::Line) {
                                            LineEntity *line = &new_entity.line;
                                            line->start += displacement;
                                            line->end   += displacement;
                                        } else { ASSERT(entity->type == EntityType::Arc);
                                            ArcEntity *arc = &new_entity.arc;
                                            arc->center += displacement;
                                        }
                                    }
                                    cookbook._buffer_add_entity(new_entity);
                                }
                            }
                            _for_each_selected_entity_ entity->is_selected = false;
                        } else if (state.click_mode == ClickMode::Polygon) {
                            uint polygon_num_sides = MAX(3U, popup->polygon_num_sides);
                            if (IS_ZERO(norm(*first_click - *second_click))) {
                                messagef(omax.orange, "Polygon: must have non-zero size");
                            } else {
                                // two_click_command->awaiting_second_click = false;
                                result.checkpoint_me = true;
                                state.click_mode = ClickMode::None;
                                state.click_modifier = ClickModifier::None;
                                real delta_theta = TAU / polygon_num_sides;
                                vec2 center = *first_click;
                                vec2 vertex_0 = *second_click;
                                real radius = distance(center, vertex_0);
                                real theta_0 = ATAN2(vertex_0 - center);
                                // cookbook.buffer_add_line(center, vertex_0); // center line (so sayeth LAYOUT)
                                for_(i, polygon_num_sides) {
                                    real theta_i = theta_0 + (i * delta_theta);
                                    real theta_ip1 = theta_i + delta_theta;
                                    cookbook.buffer_add_line(
                                            get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta_i),
                                            get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta_ip1)
                                            );
                                }
                            }
                        } else if (click_mode_WINDOW_SELECT_OR_WINDOW_DESELECT) {
                            two_click_command->awaiting_second_click = false;
                            bbox2 window = {
                                MIN(first_click->x, second_click->x),
                                MIN(first_click->y, second_click->y),
                                MAX(first_click->x, second_click->x),
                                MAX(first_click->y, second_click->y)
                            };
                            _for_each_entity_ {
                                if (bbox_contains(window, entity_get_bbox(entity))) {
                                    cookbook.entity_set_is_selected(entity, value_to_write_to_selection_mask);
                                }
                            }
                        }
                    }
                } else {
                    if (state.click_mode == ClickMode::Origin) {
                        result.checkpoint_me = true;
                        state.click_mode = ClickMode::None;
                        state.click_modifier = ClickModifier::None;
                        drawing->origin = *mouse;
                    } else if (state.click_mode == ClickMode::DivideNearest) { 
                        DXFFindClosestEntityResult closest_results = dxf_find_closest_entity(&drawing->entities, *mouse); // TODO *closest* -> *nearest*
                        if (closest_results.success) {
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
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
                    } else if (state.click_mode == ClickMode::PowerFillet) {
                        result.checkpoint_me = true;
                        state.click_mode = ClickMode::None;
                        state.click_modifier = ClickModifier::None;

                        List<Entity*> selected_entities = {};
                        selected_entities._capacity = 0;

                        _for_each_selected_entity_ {
                            if (entity->type == EntityType::Line) {
                                entity->line.start -= *mouse;
                                entity->line.end -= *mouse;
                            } else {
                                entity->arc.center -= *mouse;
                            }
                            list_push_back(&selected_entities, entity);
                        }

                        qsort(selected_entities.array, selected_entities.length, sizeof(Entity*), [](const void *a, const void *b) -> int {
                                real a_angle = ATAN2(entity_get_middle(*(Entity **)a)); // why not Enity *? idk but it crashes otherwise
                                real b_angle = ATAN2(entity_get_middle(*(Entity **)b)); 
                                if (a_angle < b_angle) {
                                return 1;
                                } else if (b_angle > a_angle) {
                                return -1;
                                }
                                return 0;
                                }); // no this is not evil 
                        for_(i, selected_entities.length) {
                            Entity *entity = selected_entities.array[i];
                            if (entity->type == EntityType::Line) {
                                entity->line.start += *mouse;
                                entity->line.end += *mouse;
                            } else { ASSERT(entity->type == EntityType::Arc);
                                entity->arc.center += *mouse;
                            }
                        }

                        for_(i, selected_entities.length) {
                            cookbook.fillet_two_entities_from_point(selected_entities.array[i], selected_entities.array[(i+1) % selected_entities.length], mouse, popup->fillet_radius);
                        }

                    } else if (state.click_mode == ClickMode::MirrorX) {
                        result.checkpoint_me = true;
                        state.click_mode = ClickMode::None;
                        state.click_modifier = ClickModifier::None;
                        _for_each_selected_entity_ {
                            if (entity->type == EntityType::Line) {
                                LineEntity *line = &entity->line;
                                cookbook.buffer_add_line(
                                        V2(-(line->start.x - mouse->x) + mouse->x, line->start.y),
                                        V2(-(line->end.x - mouse->x) + mouse->x, line->end.y),
                                        true,
                                        entity->color_code
                                        );
                            } else { ASSERT(entity->type == EntityType::Arc);
                                ArcEntity *arc = &entity->arc;
                                cookbook.buffer_add_arc(
                                        V2(-(arc->center.x - mouse->x) + mouse->x, arc->center.y),
                                        arc->radius,
                                        180 - arc->end_angle_in_degrees,
                                        180 - arc->start_angle_in_degrees,
                                        true,
                                        entity->color_code);
                            }
                            entity->is_selected = false;
                        }
                    } else if (state.click_mode == ClickMode::MirrorY) {
                        result.checkpoint_me = true;
                        state.click_mode = ClickMode::None;
                        state.click_modifier = ClickModifier::None;
                        _for_each_selected_entity_ {
                            if (entity->type == EntityType::Line) {
                                LineEntity *line = &entity->line;
                                cookbook.buffer_add_line(
                                        V2(line->start.x, -(line->start.y - mouse->y) + mouse->y),
                                        V2(line->end.x, -(line->end.y - mouse->y) + mouse->y),
                                        true,
                                        entity->color_code
                                        );
                            } else { ASSERT(entity->type == EntityType::Arc);
                                ArcEntity *arc = &entity->arc;
                                cookbook.buffer_add_arc(
                                        V2(arc->center.x, -(arc->center.y - mouse->y) + mouse->y),
                                        arc->radius,
                                        -arc->end_angle_in_degrees,
                                        -arc->start_angle_in_degrees,
                                        true,
                                        entity->color_code);
                            }
                            entity->is_selected = false;
                        }
                    } else if (state.click_mode == ClickMode::Offset) {
                        DXFFindClosestEntityResult closest_results = dxf_find_closest_entity(&drawing->entities, *mouse);
                        if (closest_results.success) {
                            result.checkpoint_me = true;
                            state.click_modifier = ClickModifier::None;
                            Entity *entity = closest_results.closest_entity;
                            real input_offset = popup->offset_size;
                            if (entity->type == EntityType::Line) {
                                LineEntity *line = &entity->line;
                                vec2 dir = *mouse - closest_results.line_nearest_point; // is there an easier way to find offset??
                                vec2 offset = (input_offset * (dir/norm(dir)));
                                cookbook.buffer_add_line(line->start + offset, line->end + offset, entity->is_selected, entity->color_code);
                            } else { ASSERT(entity->type == EntityType::Arc);
                                ArcEntity *arc = &entity->arc;
                                bool in_circle = distance(arc->center, *mouse) < arc->radius;
                                bool in_sector = false;
                                if (!in_circle) {
                                    vec2 start_point = entity_get_start_point(entity);
                                    vec2 end_point = entity_get_end_point(entity);
                                    vec2 perp_end = perpendicularTo(end_point - arc->center);
                                    vec2 perp_start = perpendicularTo(start_point - arc->center);
                                    vec2 end_to_mouse = *mouse - end_point;
                                    vec2 start_to_mouse = *mouse - start_point;
                                    real end_cross_p = cross(end_to_mouse, perp_end);
                                    real start_cross_p = cross(start_to_mouse, perp_start);
                                    real diam_cross_p = cross(end_to_mouse, start_point - end_point);
                                    in_sector = (end_cross_p > 0) && (start_cross_p > 0) && (diam_cross_p > 0);
                                }
                                real radius = arc->radius + input_offset;
                                if (in_circle || in_sector) {
                                    radius = arc->radius - input_offset;
                                } 
                                cookbook.buffer_add_arc(arc->center, radius, arc->start_angle_in_degrees, arc->end_angle_in_degrees, entity->is_selected, entity->color_code); 
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
                int index_of_first_triangle_hit_by_ray = -1;
                {
                    real min_distance = HUGE_VAL;
                    for_(i, mesh->num_triangles) {
                        vec3 p[3]; {
                            for_(j, 3) p[j] = mesh->vertex_positions[mesh->triangle_indices[i][j]];
                        }
                        RayTriangleIntersectionResult ray_triangle_intersection_result = ray_triangle_intersection(mouse_event_mesh->mouse_ray_origin, mouse_event_mesh->mouse_ray_direction, p[0], p[1], p[2]);
                        if (ray_triangle_intersection_result.hit) {
                            if (ray_triangle_intersection_result.distance < min_distance) {
                                min_distance = ray_triangle_intersection_result.distance;
                                index_of_first_triangle_hit_by_ray = i; // FORNOW
                            }
                        }
                    }
                }

                if (index_of_first_triangle_hit_by_ray != -1) { // something hit
                    result.checkpoint_me = result.record_me = true;
                    feature_plane->is_active = true;
                    other.time_since_plane_selected = 0.0f;
                    {
                        feature_plane->normal = mesh->triangle_normals[index_of_first_triangle_hit_by_ray];
                        { // feature_plane->signed_distance_to_world_origin
                            vec3 a_selected = mesh->vertex_positions[mesh->triangle_indices[index_of_first_triangle_hit_by_ray][0]];
                            feature_plane->signed_distance_to_world_origin = dot(feature_plane->normal, a_selected);
                        }

                    }
                }
            }
        } else { ASSERT(mouse_event->subtype == MouseEventSubtype::Popup);
            // NOTE: this case has been moved into popup_popup();
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

    { // popup_popup
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

        { // popup_popup
            { // Snap
                if (state.click_modifier == ClickModifier::XY) {
                    // sus calling this a modifier but okay; make sure it's first or else bad bad
                    popup_popup(STRING("XY"), ToolboxGroup::Snap,
                            true,
                            CellType::Real, STRING("x"), &popup->x_coordinate,
                            CellType::Real, STRING("y"), &popup->y_coordinate);
                    if (gui_key_enter(ToolboxGroup::Snap)) {
                        // popup->_FORNOW_active_popup_unique_ID__FORNOW_name0 = NULL; // FORNOW when making box using 'X' 'X', we want the popup to trigger a reload
                        state.click_modifier = ClickModifier::None;
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(popup->x_coordinate, popup->y_coordinate));
                    }
                }
            }

            { // Drawing
                vec2 *first_click = &two_click_command->first_click;
                if (state.click_mode == ClickMode::Circle) {
                    if (two_click_command->awaiting_second_click) {
                        real prev_circle_diameter = popup->circle_diameter;
                        real prev_circle_radius = popup->circle_radius;
                        real prev_circle_circumference = popup->circle_circumference;
                        popup_popup(STRING("Circle"), ToolboxGroup::Drawing,
                                false,
                                CellType::Real, STRING("diameter"), &popup->circle_diameter,
                                CellType::Real, STRING("radius"), &popup->circle_radius,
                                CellType::Real, STRING("circumference"), &popup->circle_circumference);
                        if (gui_key_enter(ToolboxGroup::Drawing)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->circle_radius, first_click->y));
                        } else {
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
                        }
                    }
                } else if (state.click_mode == ClickMode::TwoEdgeCircle) {
                    ;
                } else if (state.click_mode == ClickMode::Line) {
                    if (two_click_command->awaiting_second_click) {
                        real prev_line_length = popup->line_length;
                        real prev_line_angle  = popup->line_angle;
                        real prev_line_run    = popup->line_run;
                        real prev_line_rise   = popup->line_rise;
                        popup_popup(STRING("Line"), ToolboxGroup::Drawing,
                                true,
                                CellType::Real, STRING("length"), &popup->line_length,
                                CellType::Real, STRING("angle"),  &popup->line_angle,
                                CellType::Real, STRING("run"),    &popup->line_run,
                                CellType::Real, STRING("rise"),   &popup->line_rise
                                );
                        if (gui_key_enter(ToolboxGroup::Drawing)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->line_run, first_click->y + popup->line_rise));
                        } else {
                            if ((prev_line_length != popup->line_length) || (prev_line_angle != popup->line_angle)) {
                                popup->line_run  = popup->line_length * COS(RAD(popup->line_angle));
                                popup->line_rise = popup->line_length * SIN(RAD(popup->line_angle));
                            } else if ((prev_line_run != popup->line_run) || (prev_line_rise != popup->line_rise)) {
                                popup->line_length = SQRT(popup->line_run * popup->line_run + popup->line_rise * popup->line_rise);
                                popup->line_angle = DEG(ATAN2(popup->line_rise, popup->line_run));
                            }
                        }
                    }
                } else if (state.click_mode == ClickMode::Box) {
                    if (two_click_command->awaiting_second_click) {
                        popup_popup(STRING("Box"), ToolboxGroup::Drawing,
                                true,
                                CellType::Real, STRING("width"), &popup->box_width,
                                CellType::Real, STRING("height"), &popup->box_height);
                        if (gui_key_enter(ToolboxGroup::Drawing)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->box_width, first_click->y + popup->box_height));
                        }
                    }
                } else if (state.click_mode == ClickMode::CenteredBox) {
                    if (two_click_command->awaiting_second_click) {
                        popup_popup(STRING("CenteredBox"), ToolboxGroup::Drawing,
                                true,
                                CellType::Real, STRING("width"), &popup->box_width,
                                CellType::Real, STRING("height"), &popup->box_height);
                        if (gui_key_enter(ToolboxGroup::Drawing)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->box_width / 2.0f, first_click->y + popup->box_height / 2.0f));
                        }
                    }
                } else if (state.click_mode == ClickMode::Move) {
                    // FORNOW: this is repeated from LINE
                    if (two_click_command->awaiting_second_click) {
                        real prev_move_length = popup->move_length;
                        real prev_move_angle = popup->move_angle;
                        real prev_move_run = popup->move_run;
                        real prev_move_rise = popup->move_rise;
                        popup_popup(STRING("Move"), ToolboxGroup::Drawing,
                                true,
                                CellType::Real, STRING("length"), &popup->move_length,
                                CellType::Real, STRING("angle"), &popup->move_angle,
                                CellType::Real, STRING("run"), &popup->move_run,
                                CellType::Real, STRING("rise"), &popup->move_rise);
                        if (gui_key_enter(ToolboxGroup::Drawing)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->move_run, first_click->y + popup->move_rise));
                        } else {
                            if ((prev_move_length != popup->move_length) || (prev_move_angle != popup->move_angle)) {
                                popup->move_run = popup->move_length * COS(RAD(popup->move_angle));
                                popup->move_rise = popup->move_length * SIN(RAD(popup->move_angle));
                            } else if ((prev_move_run != popup->move_run) || (prev_move_rise != popup->move_rise)) {
                                popup->move_length = SQRT(popup->move_run * popup->move_run + popup->move_rise * popup->move_rise);
                                popup->move_angle = DEG(ATAN2(popup->move_rise, popup->move_run));
                            }
                        }
                    }
                } else if (state.click_mode == ClickMode::Rotate) {
                    if (two_click_command->awaiting_second_click) {
                        popup_popup(STRING("Rotate"), ToolboxGroup::Drawing,
                                true,
                                CellType::Real, STRING("angle"), &popup->rotate_angle
                                );
                        if (gui_key_enter(ToolboxGroup::Drawing)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(*first_click + e_theta(RAD(popup->rotate_angle))));
                        }
                    }
                } else if (state.click_mode == ClickMode::RotateCopy) {
                    if (two_click_command->awaiting_second_click) {
                        real prev_rotate_copy_angle_in_degrees = popup->rotate_copy_angle;
                        uint prev_rotate_copy_num_copies = popup->rotate_copy_num_total_copies;
                        popup_popup(STRING("RotateCopy"), ToolboxGroup::Drawing,
                                true,
                                CellType::Uint, STRING("num_total_copies"), &popup->rotate_copy_num_total_copies,
                                CellType::Real, STRING("angle"), &popup->rotate_copy_angle
                                );
                        if (gui_key_enter(ToolboxGroup::Drawing)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D({})); // FORNOW
                        } else {
                            if (prev_rotate_copy_angle_in_degrees != popup->rotate_copy_angle) {
                                popup->rotate_copy_num_total_copies = MAX(2U, uint(360 / popup->rotate_copy_angle));
                            } else if (prev_rotate_copy_num_copies != popup->rotate_copy_num_total_copies) {
                                popup->rotate_copy_angle = 360 / popup->rotate_copy_num_total_copies;
                            }
                        }
                    }
                } else if (state.click_mode == ClickMode::LinearCopy) {
                    if (two_click_command->awaiting_second_click) {
                        real prev_linear_copy_length = popup->linear_copy_length;
                        real prev_linear_copy_angle = popup->linear_copy_angle;
                        real prev_linear_copy_run = popup->linear_copy_run;
                        real prev_linear_copy_rise = popup->linear_copy_rise;
                        popup_popup(STRING("LinearCopy"), ToolboxGroup::Drawing,
                                true,
                                CellType::Uint, STRING("num_additional_copies"), &popup->linear_copy_num_additional_copies,
                                CellType::Real, STRING("length"), &popup->linear_copy_length,
                                CellType::Real, STRING("angle"), &popup->linear_copy_angle,
                                CellType::Real, STRING("run"), &popup->linear_copy_run,
                                CellType::Real, STRING("rise"), &popup->linear_copy_rise);
                        if (gui_key_enter(ToolboxGroup::Drawing)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->linear_copy_run, first_click->y + popup->linear_copy_rise));
                        } else {
                            if ((prev_linear_copy_length != popup->linear_copy_length) || (prev_linear_copy_angle != popup->linear_copy_angle)) {
                                popup->linear_copy_run = popup->linear_copy_length * COS(RAD(popup->linear_copy_angle));
                                popup->linear_copy_rise = popup->linear_copy_length * SIN(RAD(popup->linear_copy_angle));
                            } else if ((prev_linear_copy_run != popup->linear_copy_run) || (prev_linear_copy_rise != popup->linear_copy_rise)) {
                                popup->linear_copy_length = SQRT(popup->linear_copy_run * popup->linear_copy_run + popup->linear_copy_rise * popup->linear_copy_rise);
                                popup->linear_copy_angle = DEG(ATAN2(popup->linear_copy_rise, popup->linear_copy_run));
                            }
                        }
                    }
                } else if (state.click_mode == ClickMode::Polygon) {
                    if (two_click_command->awaiting_second_click) {
                        real prev_polygon_distance_to_corner = popup->polygon_distance_to_corner;
                        real prev_polygon_distance_to_side = popup->polygon_distance_to_side;
                        real prev_polygon_side_length = popup->polygon_side_length;
                        real theta = PI / popup->polygon_num_sides;
                        popup_popup(STRING("Polygon"), ToolboxGroup::Drawing,
                                false,
                                CellType::Uint, STRING("num_sides"), &popup->polygon_num_sides, 
                                CellType::Real, STRING("distance_to_corner"), &popup->polygon_distance_to_corner,
                                CellType::Real, STRING("distance_to_side"), &popup->polygon_distance_to_side,
                                CellType::Real, STRING("side_length"), &popup->polygon_side_length);
                        if (gui_key_enter(ToolboxGroup::Drawing)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->polygon_distance_to_corner, first_click->y));
                        } else {
                            if (prev_polygon_distance_to_corner != popup->polygon_distance_to_corner) {
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
                    }
                } else if (state.click_mode == ClickMode::Offset) {
                    popup_popup(STRING("Offset"), ToolboxGroup::Drawing,
                            false,
                            CellType::Real, STRING("distance"), &popup->offset_size);
                } else if (state.click_mode == ClickMode::Fillet) {
                    popup_popup(STRING("Fillet"), ToolboxGroup::Drawing,
                            false,
                            CellType::Real, STRING("radius"), &popup->fillet_radius);
                } else if (state.click_mode == ClickMode::PowerFillet) {
                    popup_popup(STRING("PowerFillet"), ToolboxGroup::Drawing,
                            false,
                            CellType::Real, STRING("radius"), &popup->fillet_radius);
                }
            }

            { // Mesh
                if (state.enter_mode == EnterMode::Load) {
                    popup_popup(STRING("Load"), ToolboxGroup::Drawing,
                            false,
                            CellType::String, STRING("filename"), &popup->load_filename);
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (FILE_EXISTS(popup->load_filename)) {
                            if (string_matches_suffix(popup->load_filename, STRING(".dxf"))) {
                                result.record_me = true;
                                result.checkpoint_me = true;
                                result.snapshot_me = true;

                                { // conversation_dxf_load
                                    ASSERT(FILE_EXISTS(popup->load_filename));

                                    list_free_AND_zero(&drawing->entities);

                                    entities_load(popup->load_filename, &drawing->entities);

                                    if (!skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack) {
                                        init_camera_drawing();
                                        drawing->origin = {};
                                    }
                                }
                                state.enter_mode = EnterMode::None;
                                messagef(omax.green, "LoadDXF \"%s\"", popup->load_filename.data);
                                other.currently_open_dxf = popup->load_filename;
                            } else if (string_matches_suffix(popup->load_filename, STRING(".stl"))) {
                                result.record_me = true;
                                result.checkpoint_me = true;
                                result.snapshot_me = true;
                                { // conversation_stl_load(...)
                                    ASSERT(FILE_EXISTS(popup->load_filename));
                                    // ?
                                    stl_load(popup->load_filename, mesh);
                                    init_camera_mesh();
                                }
                                state.enter_mode = EnterMode::None;
                                messagef(omax.green, "LoadSTL \"%s\"", popup->load_filename.data);
                                other.currently_open_stl = popup->load_filename;
                            } else {
                                messagef(omax.orange, "Load: \"%s\" must be *.dxf or *.stl", popup->load_filename.data);
                            }
                        } else {
                            messagef(omax.orange, "Load: \"%s\" not found", popup->load_filename.data);
                        }
                    }
                } else if (state.enter_mode == EnterMode::Save || state.enter_mode == EnterMode::SaveAs) {
                    if (state.enter_mode == EnterMode::Save) {
                        if (true) {}
                    }
                    result.record_me = false;
                    if (other.awaiting_confirmation) {
                        popup_popup(STRING("Confirm overwrite"), ToolboxGroup::Drawing, false, CellType::String, STRING("(y/n)"), &popup->save_confirmation);
                    } else {
                        popup_popup(STRING("Save"), ToolboxGroup::Drawing,
                                false,
                                CellType::String, STRING("filename"), &popup->save_filename);
                    }
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (FILE_EXISTS(popup->save_filename)) {
                            if (string_equal(popup->save_filename, other.currently_open_stl) || string_equal(popup->save_filename, other.currently_open_dxf)) {
                            } else if (other.awaiting_confirmation) {
                                messagef(omax.pink, "Overwrote \"%s\"", popup->save_filename.data);
                                if (popup->save_confirmation.data[0] == 'y') {
                                    other.awaiting_confirmation = false;
                                } else {
                                    state.enter_mode = EnterMode::None;
                                    messagef(omax.red, "SAVE ABORTED");
                                }
                            } else {
                                messagef(omax.pink, "WARNING \"%s\" already exists", popup->save_filename.data);
                                other.awaiting_confirmation = true;
                            }
                        }


                        if (!other.awaiting_confirmation) {
                            if (string_matches_suffix(popup->save_filename, STRING(".stl"))) {
                                { // conversation_stl_save
                                    bool success = mesh_save_stl(mesh, popup->save_filename);
                                    ASSERT(success);
                                }
                                state.enter_mode = EnterMode::None;
                                messagef(omax.green, "SaveSTL \"%s\"", popup->save_filename.data);
                            } else if (string_matches_suffix(popup->save_filename, STRING(".dxf"))) {
                                {
                                    bool success = drawing_save_dxf(drawing, popup->save_filename);
                                    ASSERT(success);
                                }
                                state.enter_mode = EnterMode::None;
                                messagef(omax.green, "SaveDXF \"%s\"", popup->save_filename.data);
                            } else {
                                messagef(omax.orange, "Save: \"%s\" must be *.stl (TODO: .dxf)", popup->save_filename.data);
                            }
                        }
                    }
                } else if (state.enter_mode == EnterMode::Size) {
                    result.record_me = false;
                    popup_popup(STRING("Size"), ToolboxGroup::Drawing,
                            false,
                            CellType::Real, STRING("scale factor"), &popup->scale_factor);
                    if (gui_key_enter(ToolboxGroup::Drawing)) {
                        if (!IS_ZERO(popup->scale_factor)) {
                            bbox2 bbox = entities_get_bbox(&drawing->entities, true);
                            vec2 bbox_center = AVG(bbox.min, bbox.max);
                            _for_each_selected_entity_ {
                                if (entity->type == EntityType::Line) {
                                    LineEntity *line = &entity->line;
                                    line->start = scaled_about(line->start, bbox_center, popup->scale_factor);
                                    line->end = scaled_about(line->end, bbox_center, popup->scale_factor);
                                } else { ASSERT(entity->type == EntityType::Arc);
                                    ArcEntity *arc = &entity->arc;
                                    arc->center = scaled_about(arc->center, bbox_center, popup->scale_factor);
                                    arc->radius *= popup->scale_factor;
                                }
                            }
                        }
                        state.enter_mode = EnterMode::None;
                    }
                } else if (state.enter_mode == EnterMode::ExtrudeAdd) {
                    popup_popup(STRING("ExtrudeAdd"), ToolboxGroup::Mesh,
                            true,
                            CellType::Real, STRING("out_length"), &popup->extrude_add_out_length,
                            CellType::Real, STRING("in_length"),  &popup->extrude_add_in_length);
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (!dxf_anything_selected) {
                            messagef(omax.orange, "ExtrudeAdd: selection empty");
                        } else if (!feature_plane->is_active) {
                            messagef(omax.orange, "ExtrudeAdd: no feature plane selected");
                        } else if (IS_ZERO(popup->extrude_add_in_length) && IS_ZERO(popup->extrude_add_out_length)) {
                            messagef(omax.orange, "ExtrudeAdd: total extrusion length zero");
                        } else {
                            cookbook.manifold_wrapper();
                            if (IS_ZERO(popup->extrude_add_in_length)) {
                                messagef(omax.green, "ExtrudeAdd %gmm", popup->extrude_add_out_length);
                            } else {
                                messagef(omax.green, "ExtrudeAdd %gmm %gmm", popup->extrude_add_out_length, popup->extrude_add_in_length);
                            }
                        }
                    }
                } else if (state.enter_mode == EnterMode::ExtrudeCut) {
                    popup_popup(STRING("ExtrudeCut"), ToolboxGroup::Mesh,
                            true,
                            CellType::Real, STRING("in_length"), &popup->extrude_cut_in_length,
                            CellType::Real, STRING("out_length"), &popup->extrude_cut_out_length);
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (!dxf_anything_selected) {
                            messagef(omax.orange, "ExtrudeCut: selection empty");
                        } else if (!feature_plane->is_active) {
                            messagef(omax.orange, "ExtrudeCut: no feature plane selected");
                        } else if (IS_ZERO(popup->extrude_cut_in_length) && IS_ZERO(popup->extrude_cut_out_length)) {
                            messagef(omax.orange, "ExtrudeCut: total extrusion length zero");
                        } else if (mesh->num_triangles == 0) {
                            messagef(omax.orange, "ExtrudeCut: current mesh empty");
                        } else {
                            cookbook.manifold_wrapper();
                            if (IS_ZERO(popup->extrude_cut_out_length)) {
                                messagef(omax.green, "ExtrudeCut %gmm", popup->extrude_cut_in_length);
                            } else {
                                messagef(omax.green, "ExtrudeCut %gmm %gmm", popup->extrude_cut_in_length, popup->extrude_cut_out_length);
                            }
                        }
                    }
                } else if (state.enter_mode == EnterMode::RevolveAdd) {
                    popup_popup(STRING("RevolveAdd"), ToolboxGroup::Mesh,
                            true,
                            CellType::Real, STRING("out_angle"), &popup->revolve_add_out_angle,
                            CellType::Real, STRING("in_angle"), &popup->revolve_add_in_angle
                            );
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (!dxf_anything_selected) {
                            messagef(omax.orange, "RevolveAdd: selection empty");
                        } else if (!feature_plane->is_active) {
                            messagef(omax.orange, "RevolveAdd: no feature plane selected");
                        } else {
                            cookbook.manifold_wrapper();
                            messagef(omax.green, "RevolveAdd");
                        }
                    }
                } else if (state.enter_mode == EnterMode::RevolveCut) {
                    popup_popup(STRING("RevolveCut"), ToolboxGroup::Mesh,
                            true,
                            CellType::Real, STRING("in_angle"), &popup->revolve_cut_in_angle,
                            CellType::Real, STRING("out_angle"), &popup->revolve_cut_out_angle
                            );
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (!dxf_anything_selected) {
                            messagef(omax.orange, "RevolveCut: selection empty");
                        } else if (!feature_plane->is_active) {
                            messagef(omax.orange, "RevolveCut: no feature plane selected");
                        } else if (mesh->num_triangles == 0) {
                            messagef(omax.orange, "RevolveCut: current mesh empty");
                        } else {
                            cookbook.manifold_wrapper();
                            messagef(omax.green, "RevolveCut");
                        }
                    }
                } else if (state.enter_mode == EnterMode::NudgePlane) {
                    popup_popup(STRING("NudgePlane"), ToolboxGroup::Mesh,
                            true,
                            CellType::Real, STRING("feature_plane_nudge"), &popup->feature_plane_nudge);
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        result.record_me = true;
                        result.checkpoint_me = true;
                        feature_plane->signed_distance_to_world_origin += popup->feature_plane_nudge;
                        state.enter_mode = EnterMode::None;
                        messagef(omax.green, "NudgePlane %gmm", popup->feature_plane_nudge);
                    }
                }
            }
        }
    }

    event_passed_to_popups = {}; // FORNOW: probably unnecessary
    already_processed_event_passed_to_popups = false; // FORNOW: probably unnecessary
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
