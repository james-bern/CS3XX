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

    if (event.type == EventType::Key) {
        KeyEvent *key_event = &event.key_event;
        if (key_event->subtype == KeyEventSubtype::Hotkey) {
            result.record_me = true;

            auto key_lambda = [key_event](uint key, bool control = false, bool shift = false) -> bool {
                return _key_lambda(key_event, key, control, shift);
            };

            bool digit_lambda;
            uint digit = 0;
            {
                digit_lambda = false;
                for_(color, 10) {
                    if (key_lambda('0' + color)) {
                        digit_lambda = true;
                        digit = color;
                        break;
                    }
                }
            }

            ClickMode prev_click_mode = state.click_mode;
            EnterMode prev_enter_mode = state.enter_mode;
            { // key_lambda
                if (digit_lambda) {
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
                } else if (key_lambda('A')) {
                    if (click_mode_SELECT_OR_DESELECT()) {
                        result.checkpoint_me = true;
                        cookbook.set_is_selected_for_all_entities(state.click_mode == ClickMode::Select);
                        state.click_mode = ClickMode::None;
                        state.click_modifier = ClickModifier::None;
                    }
                } else if (key_lambda('A', false, true)) {
                    state.click_mode = ClickMode::Axis;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;
                    do_once { messagef(omax.red, "TODO: implement magical LAYOUT thing that bumps your cursor along current axis"); };
                } else if (key_lambda('B')) {
                    state.click_mode = ClickMode::BoundingBox;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;
                } else if (key_lambda('C')) {
                    if (((state.click_mode == ClickMode::Select) || (state.click_mode == ClickMode::Deselect)) && (state.click_modifier != ClickModifier::Connected)) {
                        state.click_modifier = ClickModifier::Connected;
                    } else if (click_mode_SNAP_ELIGIBLE()) {
                        result.record_me = false;
                        state.click_modifier = ClickModifier::Center;
                    } else {
                        state.click_mode = ClickMode::Circle;
                        state.click_modifier = ClickModifier::None;
                        two_click_command->awaiting_second_click = false;
                    }
                } else if (key_lambda('C', true, false)) {
                    state.click_mode = ClickMode::TwoEdgeCircle;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;
                } else if (key_lambda('D')) {
                    state.click_mode = ClickMode::Deselect;
                    state.click_modifier = ClickModifier::None;
                } else if (key_lambda('D', false, true)) {
                    state.click_mode = ClickMode::DivideNearest;
                    state.click_modifier = ClickModifier::None;
                } else if (key_lambda('E')) {
                    if (click_mode_SNAP_ELIGIBLE()) {
                        result.record_me = false;
                        state.click_modifier = ClickModifier::End;
                    }
                } else if (key_lambda('F')) {
                    state.click_mode = ClickMode::Fillet;
                    state.click_modifier = ClickModifier::None;
                    state.enter_mode = EnterMode::None;
                    two_click_command->awaiting_second_click = false;
                } else if (key_lambda('G')) {
                    result.record_me = false;
                    other.hide_grid = !other.hide_grid;
                } else if (key_lambda('H')) {
                    result.record_me = false;
                    history_printf_script();
                } else if (key_lambda('I')) {
                    state.click_mode = ClickMode::TwoClickDivide;
                    state.click_modifier = ClickModifier::None;
                    state.enter_mode = EnterMode::None;
                    two_click_command->awaiting_second_click = false;
                    two_click_command->stored_entity = NULL;
                } else if (key_lambda('K')) { 
                    result.record_me = false;
                    other.show_event_stack = !other.show_event_stack;
                    result.record_me = false;
                } else if (key_lambda('L')) {
                    state.click_mode = ClickMode::Line;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;
                } else if (key_lambda('M')) {
                    if (click_mode_SNAP_ELIGIBLE()) {
                        result.record_me = false;
                        state.click_modifier = ClickModifier::Middle;
                    } else {
                        state.click_mode = ClickMode::Move;
                        state.click_modifier = ClickModifier::None;
                        two_click_command->awaiting_second_click = false;
                    }
                } else if (key_lambda('M', false, true)) {
                    result.record_me = false;
                    state.click_mode = ClickMode::Measure;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;
                } else if (key_lambda('M', true, true)) {
                    state.click_mode = ClickMode::MirrorLine;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;
                } else if (key_lambda('N')) {
                    if (feature_plane->is_active) {
                        state.enter_mode = EnterMode::NudgeFeaturePlane;
                        preview->feature_plane_offset = 0.0f; // FORNOW
                    } else {
                        messagef(omax.orange, "NudgeFeaturePlane: no feature plane selected");
                    }
                } else if (key_lambda('N', true, false)) {
                    result.checkpoint_me = true;
                    result.snapshot_me = true;
                    list_free_AND_zero(&drawing->entities);
                    *drawing = {};
                    messagef(omax.green, "ResetDXF");
                } else if (key_lambda('N', true, true)) {
                    result.checkpoint_me = true;
                    result.snapshot_me = true;
                    mesh_free_AND_zero(mesh);
                    *feature_plane = {};
                    messagef(omax.green, "ResetSTL");
                } else if (key_lambda('O')) {
                    state.click_mode = ClickMode::LinearCopy;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;
                } else if (key_lambda('O', true)) {
                    state.enter_mode = EnterMode::Load;
                } else if (key_lambda('P')) {
                    if (click_mode_SNAP_ELIGIBLE()) {
                        result.record_me = false;
                        state.click_modifier = ClickModifier::Perpendicular;
                    } else {
                        state.click_mode = ClickMode::Polygon;
                        state.click_modifier = ClickModifier::None;
                        two_click_command->awaiting_second_click = false;
                    }
                } else if (key_lambda('Q')) {
                    if (click_mode_SNAP_ELIGIBLE()) {
                        result.record_me = false;
                        state.click_modifier = ClickModifier::Quad;
                    } else if (click_mode_SELECT_OR_DESELECT() && (state.click_modifier == ClickModifier::None)) {
                        state.click_modifier = ClickModifier::Color;
                    } else {
                        state.click_mode = ClickMode::Color;
                        state.click_modifier = ClickModifier::None;
                    }
                } else if (key_lambda('R')) {
                    state.click_mode = ClickMode::Rotate;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;
                    do_once { messagef(omax.red, "TODO: implement magical LAYOUT thing that bumps your cursor over to the right"); };
                } else if (key_lambda('R', false, true)) {
                    state.click_mode = ClickMode::RotateCopy;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;
                    popup->have_fields_been_edited = 0; // to store some state for autofilling fields
                } else if (key_lambda('S')) {
                    if (state.click_mode != ClickMode::Color) {
                        state.click_mode = ClickMode::Select;
                        state.click_modifier = ClickModifier::None;
                    } else {
                        state.click_modifier = ClickModifier::Selected;
                    }
                } else if (key_lambda('S', true)) {
                    result.record_me = false;
                    state.enter_mode = EnterMode::Save;
                } else if (key_lambda('S', false, true)) {  // this is also doing precalculations because otherwise
                                                            // it would be done every frame
                    state.enter_mode = EnterMode::Size;
                    //vec2 center;
                    //vec2 corner;
                    //  float minX = HUGE_VAL, minY = HUGE_VAL, maxX = -HUGE_VAL, maxY = -HUGE_VAL;
                    _for_each_selected_entity_ {

                    }
                } else if (key_lambda('W')) {
                    // click_mode_SELECT_OR_DESELECT() ??
                    if ((state.click_mode == ClickMode::Select) || (state.click_mode == ClickMode::Deselect)) {
                        state.click_modifier = ClickModifier::Window;
                        two_click_command->awaiting_second_click = false;
                    }
                } else if (key_lambda('X')) {
                    if (state.click_mode != ClickMode::None) {
                        state.click_modifier = ClickModifier::XY;
                    }
                } else if (key_lambda('X', false, true)) {
                    state.click_mode = ClickMode::MirrorX;
                    state.click_modifier = ClickModifier::None;
                } else if (key_lambda('X', true, true)) {
                    result.record_me = false;
                    init_camera_drawing();
                    init_camera_mesh();
                } else if (key_lambda('Y')) {
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
                } else if (key_lambda('Y', false, true)) {
                    state.click_mode = ClickMode::MirrorY;
                    state.click_modifier = ClickModifier::None;
                } else if (key_lambda('Z')) {
                    Event equivalent = {};
                    equivalent.type = EventType::Mouse;
                    equivalent.mouse_event.subtype = MouseEventSubtype::Drawing;
                    // .mouse_position = {};
                    return _standard_event_process_NOTE_RECURSIVE(equivalent);
                } else if (key_lambda('Z', false, true)) {
                    state.click_mode = ClickMode::Origin;
                    state.click_modifier = ClickModifier::XY;
                } else if (key_lambda(' ')) {
                    state.click_mode = ClickMode::None; // FORNOW: patching space space doing CIRCLE CENTER
                    return _standard_event_process_NOTE_RECURSIVE(state.space_bar_event);
                } else if (key_lambda(' ', false, true)) {
                    return _standard_event_process_NOTE_RECURSIVE(state.shift_space_bar_event);
                } else if (key_lambda('[')) {
                    state.enter_mode = EnterMode::ExtrudeAdd;
                    preview->extrude_in_length = 0; // FORNOW
                    preview->extrude_out_length = 0; // FORNOW
                } else if (key_lambda('[', false, true)) {
                    state.enter_mode = EnterMode::ExtrudeCut;
                    preview->extrude_in_length = 0; // FORNOW
                    preview->extrude_out_length = 0; // FORNOW
                } else if (key_lambda(']')) {
                    state.enter_mode = EnterMode::RevolveAdd;
                } else if (key_lambda(']', false, true)) {
                    state.enter_mode = EnterMode::RevolveCut;
                } else if (key_lambda('.')) { 
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
                } else if (key_lambda(';')) {
                    result.checkpoint_me = true;
                    feature_plane->is_active = false;
                } else if (key_lambda('\'')) {
                    result.record_me = false;
                    other.camera_mesh.angle_of_view = CAMERA_3D_PERSPECTIVE_ANGLE_OF_VIEW - other.camera_mesh.angle_of_view;
                } else if (key_lambda(GLFW_KEY_BACKSPACE) || key_lambda(GLFW_KEY_DELETE)) {
                    for (int i = drawing->entities.length - 1; i >= 0; --i) {
                        if (drawing->entities.array[i].is_selected) {
                            cookbook._delete_entity(i);
                        }
                    }
                } else if (key_lambda('/', false, true)) {
                    result.record_me = false;
                    other.show_help = !other.show_help;
                } else if (key_lambda(GLFW_KEY_ESCAPE)) {
                    state.enter_mode = EnterMode::None;
                    state.click_mode = ClickMode::None;
                    state.click_modifier = ClickModifier::None;
                    state.click_color_code = ColorCode::Traverse;
                } else if (key_lambda(GLFW_KEY_TAB)) { // FORNOW
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
                } else if (key_lambda(GLFW_KEY_ENTER)) { // FORNOW
                                                         // messagef(omax.orange, "EnterMode is None.");
                    result.record_me = false;
                } else {
                    messagef(omax.orange, "Hotkey: %s not recognized", key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(key_event), key_event->control, key_event->shift, key_event->key);
                    result.record_me = false;
                    ;
                }
            }
            bool changed_click_mode = (prev_click_mode != state.click_mode);
            bool changed_enter_mode = (prev_enter_mode != state.enter_mode);
            if (changed_click_mode && click_mode_SPACE_BAR_REPEAT_ELIGIBLE()) state.space_bar_event = event;
            if (changed_enter_mode && enter_mode_SHIFT_SPACE_BAR_REPEAT_ELIGIBLE()) state.shift_space_bar_event = event;
        } else { ASSERT(key_event->subtype == KeyEventSubtype::Popup);
            result.record_me = true;

            other.time_since_cursor_start = 0.0; // FORNOW

            uint key = key_event->key;
            bool shift = key_event->shift;
            bool control = key_event->control;

            bool _tab_hack_so_aliases_not_introduced_too_far_up = false;
            if (key == GLFW_KEY_TAB) {
                _tab_hack_so_aliases_not_introduced_too_far_up = true;
                uint new_active_cell_index; {
                    // FORNOW
                    if (!shift) {
                        new_active_cell_index = (popup->active_cell_index + 1) % popup->num_cells;
                    } else {
                        if (popup->active_cell_index != 0) {
                            new_active_cell_index = popup->active_cell_index - 1;
                        } else {
                            new_active_cell_index = popup->num_cells - 1;
                        }
                    }
                }
                POPUP_SET_ACTIVE_CELL_INDEX(new_active_cell_index);
            }

            uint left_cursor = MIN(popup->cursor, popup->selection_cursor);
            uint right_cursor = MAX(popup->cursor, popup->selection_cursor);

            if (_tab_hack_so_aliases_not_introduced_too_far_up) {
            } else if (control && (key == 'A')) {
                popup->cursor = popup->active_cell_buffer.length;
                popup->selection_cursor = 0;
            } else if (key == GLFW_KEY_LEFT) {
                if (!shift && !control) {
                    if (POPUP_SELECTION_NOT_ACTIVE()) {
                        if (popup->cursor > 0) --popup->cursor;
                    } else {
                        popup->cursor = left_cursor;
                    }
                    popup->selection_cursor = popup->cursor;
                } else if (shift && !control) {
                    if (POPUP_SELECTION_NOT_ACTIVE()) popup->selection_cursor = popup->cursor;
                    if (popup->cursor > 0) --popup->cursor;
                } else if (control && !shift) {
                    popup->selection_cursor = popup->cursor = 0;
                } else { ASSERT(shift && control);
                    popup->selection_cursor = 0;
                }
            } else if (key == GLFW_KEY_RIGHT) {
                if (!shift && !control) {
                    if (POPUP_SELECTION_NOT_ACTIVE()) {
                        if (popup->cursor < popup->active_cell_buffer.length) ++popup->cursor;
                    } else {
                        popup->cursor = MAX(popup->cursor, popup->selection_cursor);
                    }
                    popup->selection_cursor = popup->cursor;
                } else if (shift && !control) {
                    if (POPUP_SELECTION_NOT_ACTIVE()) popup->selection_cursor = popup->cursor;
                    if (popup->cursor < popup->active_cell_buffer.length) ++popup->cursor;
                } else if (control && !shift) {
                    popup->selection_cursor = popup->cursor = popup->active_cell_buffer.length;
                } else { ASSERT(shift && control);
                    popup->selection_cursor = popup->active_cell_buffer.length;
                }
            } else if (key == GLFW_KEY_BACKSPACE) {
                // * * * *|* * * * 
                if (POPUP_SELECTION_NOT_ACTIVE()) {
                    if (popup->cursor > 0) {
                        memmove(&popup->active_cell_buffer.data[popup->cursor - 1], &popup->active_cell_buffer.data[popup->cursor], POPUP_CELL_LENGTH - popup->cursor);
                        --popup->active_cell_buffer.length;
                        --popup->cursor;
                    }
                } else {
                    // * * * * * * * * * * * * * * * *
                    // * * * * * * * - - - - - - - - -
                    //    L                 R 

                    // * * * * * * * * * * * * * * * *
                    // * * * * * * * * * * * * - - - -
                    //    L       R                   
                    memmove(&popup->active_cell_buffer.data[left_cursor], &popup->active_cell_buffer.data[right_cursor], POPUP_CELL_LENGTH - right_cursor);
                    popup->active_cell_buffer.length -= (right_cursor - left_cursor);
                    popup->cursor = left_cursor;
                }
                popup->selection_cursor = popup->cursor;
            } else if (key == GLFW_KEY_ENTER) {
                ;
            } else {
                // TODO: strip char_equivalent into function

                bool key_is_alpha = ('A' <= key) && (key <= 'Z');

                char char_equivalent; {
                    char_equivalent = (char) key;
                    if (!shift && key_is_alpha) {
                        char_equivalent = 'a' + (char_equivalent - 'A');
                    }
                }
                if (POPUP_SELECTION_NOT_ACTIVE()) {
                    if (popup->cursor < POPUP_CELL_LENGTH) {
                        memmove(&popup->active_cell_buffer.data[popup->cursor + 1], &popup->active_cell_buffer.data[popup->cursor], POPUP_CELL_LENGTH - popup->cursor - 1);
                        popup->active_cell_buffer.data[popup->cursor] = char_equivalent;
                        ++popup->cursor;
                        ++popup->active_cell_buffer.length;
                    }
                } else {
                    memmove(&popup->active_cell_buffer.data[left_cursor + 1], &popup->active_cell_buffer.data[right_cursor], POPUP_CELL_LENGTH - right_cursor);
                    popup->active_cell_buffer.length -= (right_cursor - left_cursor);
                    popup->cursor = left_cursor;
                    popup->active_cell_buffer.data[popup->cursor] = char_equivalent;
                    ++popup->cursor;
                    ++popup->active_cell_buffer.length;
                }
                popup->selection_cursor = popup->cursor;
            }

            // FORNOW: keeping null-termination around for messagef?
            popup->active_cell_buffer.data[popup->active_cell_buffer.length] = '\0';
        }
    } else if (event.type == EventType::Mouse) {
        MouseEvent *mouse_event = &event.mouse_event;
        if (mouse_event->subtype == MouseEventSubtype::Drawing) {
            MouseEventDrawing *mouse_event_drawing = &mouse_event->mouse_event_drawing;

            result.record_me = true;
            if (state.click_mode == ClickMode::Measure) result.record_me = false;
            if (mouse_event->mouse_held) result.record_me = false;

            vec2 *mouse = &mouse_event_drawing->mouse_position;
            vec2 *second_click = &mouse_event_drawing->mouse_position;

            bool click_mode_WINDOW_SELECT_OR_WINDOW_DESELECT = (click_mode_SELECT_OR_DESELECT() && (state.click_modifier == ClickModifier::Window));

            // TODO: move to misc.cpp; TODO: rename misc.cpp -> bool.cpp?
            bool click_mode_TWO_CLICK_COMMAND = 0 ||
                (state.click_mode == ClickMode::Axis) ||
                (state.click_mode == ClickMode::BoundingBox) ||
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
                DXFFindClosestEntityResult dxf_find_closest_entity_result = dxf_find_closest_entity(&drawing->entities, mouse_event_drawing->mouse_position);
                if (dxf_find_closest_entity_result.success) {
                    uint hot_entity_index = dxf_find_closest_entity_result.index;
                    if (state.click_modifier != ClickModifier::Connected) {
                        if (click_mode_SELECT_OR_DESELECT()) {
                            cookbook.entity_set_is_selected(&drawing->entities.array[hot_entity_index], value_to_write_to_selection_mask);
                        } else {
                            cookbook.entity_set_color(&drawing->entities.array[hot_entity_index], state.click_color_code);
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
                    vec2 *first_click = &two_click_command->first_click;
                    vec2 click_vector = (*second_click - *first_click);

                    if (!two_click_command->awaiting_second_click) {
                        two_click_command->awaiting_second_click = true;
                        *first_click = mouse_event_drawing->mouse_position;
                        if (state.click_modifier != ClickModifier::Window) state.click_modifier = ClickModifier::None;
                    } else {
                        if (0) {
                        } else if (state.click_mode == ClickMode::Axis) {
                            two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            drawing->axis_base_point = *first_click;
                            drawing->axis_angle_from_y = (-PI / 2) + ATAN2(*second_click - *first_click);
                        } else if (state.click_mode == ClickMode::BoundingBox) {
                            if (IS_ZERO(ABS(first_click->x - second_click->x))) {
                                messagef(omax.orange, "Box: must have non-zero width ");
                            } else if (IS_ZERO(ABS(first_click->y - second_click->y))) {
                                messagef(omax.orange, "Box: must have non-zero height");
                            } else {
                                two_click_command->awaiting_second_click = false;
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
                        } else if (state.click_mode == ClickMode::Fillet) {
                            two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_modifier = ClickModifier::None;
                            DXFFindClosestEntityResult result_j = dxf_find_closest_entity(&drawing->entities, mouse_event_drawing->mouse_position);
                            if ((two_click_command->stored_entity != NULL) && (result_j.success) && (two_click_command->entity_index != result_j.index)) {
                                uint i = two_click_command->entity_index;
                                uint j = result_j.index;
                                real radius = popup->fillet_radius;
                                Entity *E_i = two_click_command->stored_entity;
                                Entity *E_j = &drawing->entities.array[j];
                                if ((E_i->type == EntityType::Line) && (E_j->type == EntityType::Line)) {
                                    vec2 a, b, c, d;
                                    entity_get_start_and_end_points(E_i, &a, &b);
                                    entity_get_start_and_end_points(E_j, &c, &d);

                                    LineLineXResult _p = burkardt_line_line_intersection(a, b, c, d);
                                    if (!_p.lines_are_parallel) {
                                        vec2 p = _p.point;

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

                                        vec2 m = AVG(*first_click, *second_click);

                                        vec2 e_ab = normalized(b - a);
                                        vec2 e_cd = normalized(d - c);

                                        bool keep_a, keep_c; {
                                            vec2 vector_p_m_in_edge_basis = inverse(hstack(e_ab, e_cd)) * (m - p);
                                            keep_a = (vector_p_m_in_edge_basis.x < 0.0f);
                                            keep_c = (vector_p_m_in_edge_basis.y < 0.0f);
                                        }

                                        // TODO: in general, just use burkardt's angle stuff

                                        vec2 s_ab = (keep_a) ? a : b;
                                        vec2 s_cd = (keep_c) ? c : d;
                                        real half_angle; {
                                            real angle = burkardt_three_point_angle(s_ab, p, s_cd); // FORNOW TODO consider using burkardt's special interior version
                                            if (angle > PI) angle = TAU - angle;
                                            half_angle = angle / 2;
                                        }
                                        real length = radius / TAN(half_angle);
                                        vec2 t_ab = p + (keep_a ? -1 : 1) * length * e_ab;
                                        vec2 t_cd = p + (keep_c ? -1 : 1) * length * e_cd;

                                        LineLineXResult _center = burkardt_line_line_intersection(t_ab, t_ab + perpendicularTo(e_ab), t_cd, t_cd + perpendicularTo(e_cd));
                                        if (!_center.lines_are_parallel) {
                                            vec2 center = _center.point;

                                            ColorCode color_i = E_i->color_code;
                                            ColorCode color_j = E_j->color_code;
                                            cookbook.buffer_delete_entity(i);
                                            cookbook.buffer_delete_entity(j);

                                            cookbook.buffer_add_line(s_ab, t_ab, false, color_i);
                                            cookbook.buffer_add_line(s_cd, t_cd, false, color_j);

                                            real theta_ab_in_degrees = DEG(ATAN2(t_ab - center));
                                            real theta_cd_in_degrees = DEG(ATAN2(t_cd - center));

                                            if (!IS_ZERO(radius)) {
                                                if (burkardt_three_point_angle(t_ab, center, t_cd) < PI) {
                                                    // FORNOW TODO consider swap
                                                    real tmp = theta_ab_in_degrees;
                                                    theta_ab_in_degrees = theta_cd_in_degrees;
                                                    theta_cd_in_degrees = tmp;
                                                }

                                                // TODO: consider tabbing to create chamfer

                                                cookbook.buffer_add_arc(center, radius, theta_ab_in_degrees, theta_cd_in_degrees);
                                            }
                                        }
                                    }
                                } else {
                                    messagef(omax.red, "TODO: line-arc fillet; arc-arc fillet");
                                }
                            }
                        } else if (state.click_mode == ClickMode::Circle) {
                            if (IS_ZERO(norm(*first_click - *second_click))) {
                                messagef(omax.orange, "Circle: must have non-zero diameter");
                            } else {
                                two_click_command->awaiting_second_click = false;
                                result.checkpoint_me = true;
                                state.click_mode = ClickMode::None;
                                state.click_modifier = ClickModifier::None;
                                real theta_a_in_degrees = DEG(ATAN2(*second_click - *first_click));
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
                                two_click_command->awaiting_second_click = false;
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

                            // i want to store the first click closest somewhere because otherwise it has to search
                            Entity *closest_entity_one = two_click_command->stored_entity; 
                            DXFFindClosestEntityResult closest_result_two = dxf_find_closest_entity(&drawing->entities, *second_click);
                            if (closest_entity_one != NULL && closest_result_two.success) {
                                Entity *closest_entity_two = &drawing->entities.array[closest_result_two.index];
                                if (closest_entity_two->type == EntityType::Line && closest_entity_two->type == EntityType::Line) {
                                    LineEntity segment_one = closest_entity_one->line;
                                    LineEntity segment_two = closest_entity_two->line;
                                    vec2 a = segment_one.start;
                                    vec2 b = segment_one.end;
                                    vec2 c = segment_two.start;
                                    vec2 d = segment_two.end;
                                    LineLineXResult X_result = burkardt_line_line_intersection(a, b, c, d);
                                    bool neither_line_extension_hits_the_other_segment = ((!X_result.point_is_on_segment_ab) && (!X_result.point_is_on_segment_cd));
                                    if (neither_line_extension_hits_the_other_segment) {
                                        messagef(omax.orange, "TwoClickDivide: no intersection found");
                                    } else {
                                        if (X_result.point_is_on_segment_ab) {
                                            cookbook.buffer_add_line(X_result.point, a);
                                            cookbook.buffer_add_line(X_result.point, b);
                                            cookbook.buffer_delete_entity(two_click_command->entity_index);
                                        }
                                        if (X_result.point_is_on_segment_cd) {
                                            cookbook.buffer_add_line(X_result.point, c);
                                            cookbook.buffer_add_line(X_result.point, d);
                                            cookbook.buffer_delete_entity(closest_result_two.index);
                                        } 
                                    }
                                } else if (closest_entity_one->type == EntityType::Arc && closest_entity_two->type == EntityType::Arc){
                                    // math for this is here https://paulbourke.net/geometry/circlesphere/

                                    ArcEntity arcA = closest_entity_one->arc;
                                    ArcEntity arcB = closest_entity_two->arc;
                                    
                                    float d = distance(arcA.center, arcB.center);
                                    //TODO: find fucntion that checks to see if they are close enough for floats
                                    if (d > arcA.radius + arcB.radius) {                // non intersecting
                                    } else if (d < abs(arcA.radius-arcB.radius)) {      // One circle within other
                                    } else if (d == 0 && arcA.radius == arcB.radius) {  // coincident circles
                                    } else {
                                        real a = (POW(arcA.radius, 2) - POW(arcB.radius, 2) + POW(d, 2)) / (2 * d);
                                        real h = SQRT(POW(arcA.radius, 2) - POW(a, 2));

                                        vec2 v = arcA.center + a * (arcB.center - arcA.center) / d; 

                                        vec2 p1 = { v.x + h * (arcB.center.y - arcA.center.y) / d, v.y - h * (arcB.center.x - arcA.center.x) / d };
                                        vec2 p2 = { v.x - h * (arcB.center.y - arcA.center.y) / d, v.y + h * (arcB.center.x - arcA.center.x) / d };

                                        real theta1a = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(p1 - arcA.center)));
                                        real theta2a = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(p2 - arcA.center)));
                                        real theta1b = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(p1 - arcB.center)));
                                        real theta2b = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(p2 - arcB.center)));

                                        bool p1Good = ANGLE_IS_BETWEEN_CCW_DEGREES(theta1a, arcA.start_angle_in_degrees, arcA.end_angle_in_degrees)
                                                   && ANGLE_IS_BETWEEN_CCW_DEGREES(theta1b, arcB.start_angle_in_degrees, arcB.end_angle_in_degrees);
                                        bool p2Good = ANGLE_IS_BETWEEN_CCW_DEGREES(theta2a, arcA.start_angle_in_degrees, arcA.end_angle_in_degrees)
                                                   && ANGLE_IS_BETWEEN_CCW_DEGREES(theta2b, arcB.start_angle_in_degrees, arcB.end_angle_in_degrees);

                                        if (p1Good || p2Good) {
                                            bool choosing_p1 = p1Good;
                                            if (p1Good && p2Good) {
                                                choosing_p1 = distance(p1, *second_click) < distance(p2, *second_click);
                                            } 
                                            if (!choosing_p1) { 
                                                theta1a = theta2a;
                                                theta1b = theta2b; 
                                            }
                                            cookbook.buffer_add_arc(arcA.center, arcA.radius, arcA.start_angle_in_degrees, theta1a);
                                            cookbook.buffer_add_arc(arcA.center, arcA.radius, theta1a, arcA.end_angle_in_degrees);
                                            cookbook.buffer_add_arc(arcB.center, arcB.radius, arcB.start_angle_in_degrees, theta1b);
                                            cookbook.buffer_add_arc(arcB.center, arcB.radius, theta1b, arcB.end_angle_in_degrees);
                                            cookbook.buffer_delete_entity(two_click_command->entity_index);
                                            cookbook.buffer_delete_entity(closest_result_two.index);
                                        }
                                    }
                                } else { // TODO: ASSERT(...); //ASSERT((closest_entity_two->type == EntityType::Line && closest_entity_two->type == EntityType::Arc) // kinda nasty but only way 
                                         //       || (closest_entity_two->type == EntityType::Arc && closest_entity_two->type == EntityType::Line));
                                    ArcEntity arc;
                                    LineEntity line;
                                    if (closest_entity_one->type == EntityType::Arc) {
                                        arc = closest_entity_one->arc;
                                        line = closest_entity_two->line;
                                    } else {
                                        line = closest_entity_one->line;
                                        arc = closest_entity_two->arc;
                                    }

                                    // using determinant to find num intersects https://www.nagwa.com/en/explainers/987161873194/#:~:text=The%20discriminant%20%CE%94%20%3D%20%F0%9D%90%B5%20%E2%88%92%204,and%20the%20circle%20are%20disjoint.
                                    vec2 v1 = line.end - line.start;
                                    vec2 v2 = line.start - arc.center;

                                    float a = dot(v1, v1);
                                    float b = 2 * dot(v1, v2);
                                    float c = dot(v2, v2) - POW(arc.radius, 2);
                                    float d = POW(b, 2) - 4 * a * c;

                                    vec2 intersect = {}; // because compiler was complaining
                                    real theta = 0;
                                    bool does_intersect;

                                    if (d < 0) {                // no intersect
                                        does_intersect = false; // can we exit early???
                                    } else {                    // two intersects
                                        float t1 = (-b + SQRT(d)) / (2 * a); 
                                        float t2 = (-b - SQRT(d)) / (2 * a); 

                                        vec2 p1 = line.start + t1 * v1;
                                        vec2 p2 = line.start + t2 * v1;

                                        float theta1 = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(p1 - arc.center)));
                                        float theta2 = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(p2 - arc.center)));
                                        bool does_p1_arc_intersect = ANGLE_IS_BETWEEN_CCW_DEGREES(theta1, arc.start_angle_in_degrees, arc.end_angle_in_degrees);
                                        bool does_p2_arc_intersect = ANGLE_IS_BETWEEN_CCW_DEGREES(theta2, arc.start_angle_in_degrees, arc.end_angle_in_degrees);
                                        
                                        bool does_p1_line_intersect = MIN(line.start.x, line.end.x) < p1.x && p1.x < MAX(line.start.x, line.end.x)
                                                                   && MIN(line.start.y, line.end.y) < p1.y && p1.y < MAX(line.start.y, line.end.y);
                                        bool does_p2_line_intersect = MIN(line.start.x, line.end.x) < p2.x && p2.x < MAX(line.start.x, line.end.x)
                                                                   && MIN(line.start.y, line.end.y) < p2.y && p2.y < MAX(line.start.y, line.end.y);

                                        bool p1_works = does_p1_arc_intersect && does_p1_line_intersect;
                                        bool p2_works = does_p2_arc_intersect && does_p2_line_intersect;

                                        does_intersect = p1_works || p2_works;

                                        if (p1_works && p2_works) {
                                            if (distance(p1, *second_click) < distance(p2, *second_click)) {
                                                intersect = p1;
                                                theta = theta1;
                                            } else {
                                                intersect = p2;
                                                theta = theta2;
                                            }
                                        } else if (p1_works) {
                                            intersect = p1;
                                            theta = theta1;
                                        } else if (p2_works) {
                                            intersect = p2;
                                            theta = theta2;
                                        }

                                    }
                                    if (does_intersect) {
                                        cookbook.buffer_add_line(intersect, line.start);
                                        cookbook.buffer_add_line(intersect, line.end);
                                        cookbook.buffer_add_arc(arc.center, arc.radius, arc.start_angle_in_degrees, theta);
                                        cookbook.buffer_add_arc(arc.center, arc.radius, theta, arc.end_angle_in_degrees);
                                        cookbook.buffer_delete_entity(two_click_command->entity_index);
                                        cookbook.buffer_delete_entity(closest_result_two.index);
                                    }
                                }
                            }
                        } else if (state.click_mode == ClickMode::Line) {
                            two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            cookbook.buffer_add_line(*first_click, *second_click);
                        } else if (state.click_mode == ClickMode::Measure) {
                            two_click_command->awaiting_second_click = false;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            real angle = DEG(ATAN2(*second_click - *first_click));
                            if (angle < 0.0f) angle += 360.0f;
                            real length = norm(*second_click - *first_click);
                            messagef(omax.cyan, "Angle is %gdeg.", angle);
                            messagef(omax.cyan, "Length is %gmm.", length);
                        } else if (state.click_mode == ClickMode::MirrorLine) {
                            two_click_command->awaiting_second_click = false;
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
                            two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            real theta = ATAN2(click_vector);
                            _for_each_selected_entity_ {
                                if (entity->type == EntityType::Line) {
                                    LineEntity *line = &entity->line;
                                    line->start = rotated_about(line->start, *first_click, theta);
                                    line->end = rotated_about(line->end, *first_click, theta);
                                } else { ASSERT(entity->type == EntityType::Arc);
                                    ArcEntity *arc = &entity->arc;
                                    arc->center = rotated_about(arc->center, *first_click, theta);
                                    arc->start_angle_in_degrees = DEG(theta) + arc->start_angle_in_degrees;
                                    arc->end_angle_in_degrees = DEG(theta) + arc->end_angle_in_degrees;
                                }
                            }
                        } else if (state.click_mode == ClickMode::RotateCopy) {
                            two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            real theta = IS_ZERO(popup->angle_of_rotation_in_radians) ? PI : MAX(0.0f, popup->angle_of_rotation_in_radians);
                            popup->num_copies = MAX(2U, popup->num_copies);

                            _for_each_selected_entity_ {
                                Entity oldEntity = *entity;
                                for_(j, popup->num_copies - 1) { // layout has copies as + 1 of what you want so i copied it but it seems strange
                                    Entity new_entity = oldEntity;
                                    if (entity->type == EntityType::Line) {
                                        LineEntity *line = &new_entity.line;
                                        line->start = rotated_about(line->start, *first_click, theta);
                                        line->end = rotated_about(line->end, *first_click, theta);
                                    } else { ASSERT(entity->type == EntityType::Arc);
                                        ArcEntity *arc = &new_entity.arc;
                                        arc->center = rotated_about(arc->center, *first_click, theta);
                                        arc->start_angle_in_degrees = DEG(theta) + arc->start_angle_in_degrees;
                                        arc->end_angle_in_degrees = DEG(theta) + arc->end_angle_in_degrees;
                                    }
                                    cookbook._buffer_add_entity(new_entity);
                                    oldEntity = new_entity;
                                }
                            }
                        } else if (state.click_mode == ClickMode::Move) {
                            two_click_command->awaiting_second_click = false;
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
                            two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            uint num_copies = MAX(1U, popup->num_copies);
                            for_(i, num_copies) {
                                vec2 displacement = real(i + 1) * click_vector;
                                bool is_selected = (i == num_copies - 1);
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
                                two_click_command->awaiting_second_click = false;
                                result.checkpoint_me = true;
                                state.click_mode = ClickMode::None;
                                state.click_modifier = ClickModifier::None;
                                real delta_theta = TAU / polygon_num_sides;
                                vec2 center = *first_click;
                                vec2 vertex_0 = *second_click;
                                real radius = distance(center, vertex_0);
                                real theta_0 = ATAN2(vertex_0 - center);
                                cookbook.buffer_add_line(center, vertex_0); // center line (so sayeth LAYOUT)
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
                                                         entity->is_selected, entity->color_code);
                                cookbook.buffer_add_line(closest_results.line_nearest_point, line->end, entity->is_selected, entity->color_code);
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
            MouseEventPopup *mouse_event_popup = &mouse_event->mouse_event_popup;

            other.time_since_cursor_start = 0.0f;
            result.record_me = true; // FORNOW (don't bother implementing the old manual tagging method; we're going to upgrade to the memcmp approach soon)

            if (!mouse_event->mouse_held) { // press
                if (popup->active_cell_index != mouse_event_popup->cell_index) { // switch cell
                    POPUP_SET_ACTIVE_CELL_INDEX(mouse_event_popup->cell_index);
                    popup->cursor = mouse_event_popup->cursor;
                    popup->selection_cursor = popup->cursor;
                } else {
                    bool double_click = (POPUP_SELECTION_NOT_ACTIVE()) && (popup->cursor == mouse_event_popup->cursor);
                    if (double_click) {
                        popup->cursor = popup->active_cell_buffer.length;
                        popup->selection_cursor = 0;
                    } else { // move
                        popup->cursor = mouse_event_popup->cursor;
                        popup->selection_cursor = popup->cursor;
                    }
                }
            } else { // drag
                popup->selection_cursor = mouse_event_popup->cursor;
            }
        }
    } else { ASSERT(event.type == EventType::None);
        result.record_me = false;
    }

    { // sanity checks
        ASSERT(popup->active_cell_index <= popup->num_cells);
        ASSERT(popup->cursor <= POPUP_CELL_LENGTH);
        ASSERT(popup->selection_cursor <= POPUP_CELL_LENGTH);
    }

    { // popup_popup
        EnterMode _enter_mode_prev__NOTE_used_to_determine_when_to_close_popup_on_enter = state.enter_mode;
        popup->_popup_actually_called_this_event = false;

        { // popup_popup
            vec2 *first_click = &two_click_command->first_click;

            bool gui_key_enter; {
                gui_key_enter = false;
                if (event.type == EventType::Key) {
                    KeyEvent *key_event = &event.key_event;
                    if (key_event->subtype == KeyEventSubtype::Popup) {
                        gui_key_enter = (key_event->key == GLFW_KEY_ENTER);
                    }
                }
            }

            if (state.enter_mode == EnterMode::Load) {
                popup_popup(false,
                        CellType::String, STRING("load_filename"), &popup->load_filename);
                if (gui_key_enter) {
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
                        } else {
                            messagef(omax.orange, "Load: \"%s\" must be *.dxf or *.stl", popup->load_filename.data);
                        }
                    } else {
                        messagef(omax.orange, "Load: \"%s\" not found", popup->load_filename.data);
                    }
                }
            } else if (state.enter_mode == EnterMode::Save) {
                result.record_me = false;
                popup_popup(false,
                        CellType::String, STRING("save_filename"), &popup->save_filename);
                if (gui_key_enter) {
                    if (FILE_EXISTS(popup->save_filename)) {
                        messagef(omax.pink, "FORNOW Save: overwriting \"%s\" without asking", popup->save_filename.data);
                    }

                    if (string_matches_suffix(popup->save_filename, STRING(".stl"))) {
                        { // conversation_stl_save
                            bool success = mesh_save_stl(mesh, popup->save_filename);
                            ASSERT(success);
                        }
                        state.enter_mode = EnterMode::None;
                        messagef(omax.green, "SaveSTL \"%s\"", popup->save_filename.data);
                    } else if (string_matches_suffix(popup->save_filename, STRING(".dxf"))) {
                        messagef(omax.pink, "TODO: SaveDXF");
                    } else {
                        messagef(omax.orange, "Save: \"%s\" must be *.stl (TODO: .dxf)", popup->save_filename.data);
                    }
                }
            } else if (state.enter_mode == EnterMode::Size) {
                result.record_me = false;
                popup_popup(false,
                        CellType::Real, STRING("scale factor"), &popup->scale_factor);
                if (gui_key_enter) {
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
                popup_popup(true,
                        CellType::Real, STRING("extrude_add_out_length"), &popup->extrude_add_out_length,
                        CellType::Real, STRING("extrude_add_in_length"),  &popup->extrude_add_in_length); if (gui_key_enter) {
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
                popup_popup(true,
                        CellType::Real, STRING("extrude_cut_in_length"), &popup->extrude_cut_in_length,
                        CellType::Real, STRING("extrude_cut_out_length"), &popup->extrude_cut_out_length);
                if (gui_key_enter) {
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
                popup_popup(true, CellType::Real, STRING("revolve_add_dummy"), &popup->revolve_add_dummy);
                if (gui_key_enter) {
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
                popup_popup(true, CellType::Real, STRING("revolve_cut_dummy"), &popup->revolve_cut_dummy);
                if (gui_key_enter) {
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
            } else if (state.enter_mode == EnterMode::NudgeFeaturePlane) {
                popup_popup(true,
                        CellType::Real, STRING("feature_plane_nudge"), &popup->feature_plane_nudge);
                if (gui_key_enter) {
                    result.record_me = true;
                    result.checkpoint_me = true;
                    feature_plane->signed_distance_to_world_origin += popup->feature_plane_nudge;
                    state.enter_mode = EnterMode::None;
                    messagef(omax.green, "NudgeFeaturePlane %gmm", popup->feature_plane_nudge);
                }
            } else if (state.click_modifier == ClickModifier::XY) {
                // sus calling this a modifier but okay; make sure it's first or else bad bad
                popup_popup(true,
                        CellType::Real, STRING("x_coordinate"), &popup->x_coordinate,
                        CellType::Real, STRING("y_coordinate"), &popup->y_coordinate);
                if (gui_key_enter) {
                    // popup->_FORNOW_active_popup_unique_ID__FORNOW_name0 = NULL; // FORNOW when making box using 'X' 'X', we want the popup to trigger a reload
                    state.click_modifier = ClickModifier::None;
                    return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(popup->x_coordinate, popup->y_coordinate));
                }
            } else if (state.click_mode == ClickMode::Circle) {
                if (two_click_command->awaiting_second_click) {
                    real prev_circle_diameter = popup->circle_diameter;
                    real prev_circle_radius = popup->circle_radius;
                    real prev_circle_circumference = popup->circle_circumference;
                    popup_popup(false,
                            CellType::Real, STRING("circle_diameter"), &popup->circle_diameter,
                            CellType::Real, STRING("circle_radius"), &popup->circle_radius,
                            CellType::Real, STRING("circle_circumference"), &popup->circle_circumference);
                    if (gui_key_enter) {
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
                    popup_popup(true,
                            CellType::Real, STRING("line_length"), &popup->line_length,
                            CellType::Real, STRING("line_angle"),  &popup->line_angle,
                            CellType::Real, STRING("line_run"),    &popup->line_run,
                            CellType::Real, STRING("line_rise"),   &popup->line_rise
                            );
                    if (gui_key_enter) {
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
            } else if (state.click_mode == ClickMode::BoundingBox) {
                if (two_click_command->awaiting_second_click) {
                    popup_popup(true,
                            CellType::Real, STRING("box_width"), &popup->box_width,
                            CellType::Real, STRING("box_height"), &popup->box_height);
                    if (gui_key_enter) {
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->box_width, first_click->y + popup->box_height));
                    }
                }
            } else if (state.click_mode == ClickMode::Move) {
                // FORNOW: this is repeated from LINE
                if (two_click_command->awaiting_second_click) {
                    real prev_move_length = popup->move_length;
                    real prev_move_angle = popup->move_angle;
                    real prev_move_run = popup->move_run;
                    real prev_move_rise = popup->move_rise;
                    popup_popup(true,
                            CellType::Real, STRING("move_length"), &popup->move_length,
                            CellType::Real, STRING("move_angle"), &popup->move_angle,
                            CellType::Real, STRING("move_run"), &popup->move_run,
                            CellType::Real, STRING("move_rise"), &popup->move_rise);
                    if (gui_key_enter) {
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
                // FORNOW: this is repeated from LINE
                if (two_click_command->awaiting_second_click) {
                    real prev_degree = popup->angle_of_rotation_in_degrees;
                    real prev_rad = popup->angle_of_rotation_in_radians;
                    popup_popup(true,
                            CellType::Real, STRING("angle_of_roation_in_degrees"), &popup->angle_of_rotation_in_degrees,
                            CellType::Real, STRING("angle_of_roation_in_radians"), &popup->angle_of_rotation_in_radians);
                    if (gui_key_enter) {
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(*first_click + V2(10.0f * COS(popup->angle_of_rotation_in_radians), 10.0f * SIN(popup->angle_of_rotation_in_radians))));
                    } else {
                        if (prev_degree != popup->angle_of_rotation_in_degrees) {
                            popup->angle_of_rotation_in_radians = RAD(popup->angle_of_rotation_in_degrees);
                        } else if (prev_rad != popup->angle_of_rotation_in_radians) {
                            popup->angle_of_rotation_in_degrees = DEG(popup->angle_of_rotation_in_radians);
                        }
                    }
                }
            } else if (state.click_mode == ClickMode::RotateCopy) {
                // FORNOW: this is repeated from LINE
                if (two_click_command->awaiting_second_click) {
                    real prev_degree = popup->angle_of_rotation_in_degrees;
                    real prev_rad = popup->angle_of_rotation_in_radians;
                    popup_popup(true,
                            CellType::Uint, STRING("num_copies"), &popup->num_copies,
                            CellType::Real, STRING("angle_of_roation_in_degrees"), &popup->angle_of_rotation_in_degrees,
                            CellType::Real, STRING("angle_of_roation_in_radians"), &popup->angle_of_rotation_in_radians);
                    if (gui_key_enter) {
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(V2(0, 0)));
                    } else {
                        if (prev_degree != popup->angle_of_rotation_in_degrees) {
                            popup->have_fields_been_edited++;
                            popup->angle_of_rotation_in_radians = RAD(popup->angle_of_rotation_in_degrees);
                        } else if (prev_rad != popup->angle_of_rotation_in_radians) {
                            popup->have_fields_been_edited++;
                            popup->angle_of_rotation_in_degrees = DEG(popup->angle_of_rotation_in_radians);
                        } else if (popup->num_copies > 0 && !popup->have_fields_been_edited) {
                            popup->angle_of_rotation_in_degrees = 360.0f / popup->num_copies;
                            popup->angle_of_rotation_in_radians = RAD(popup->angle_of_rotation_in_degrees);
                        }
                    }
                }
            } else if (state.click_mode == ClickMode::LinearCopy) {
                // FORNOW: this is repeated from LINE
                if (two_click_command->awaiting_second_click) {
                    real prev_move_length = popup->move_length;
                    real prev_move_angle = popup->move_angle;
                    real prev_move_run = popup->move_run;
                    real prev_move_rise = popup->move_rise;
                    popup_popup(true,
                            CellType::Uint, STRING("num_copies"), &popup->num_copies,
                            CellType::Real, STRING("move_length"), &popup->move_length,
                            CellType::Real, STRING("move_angle"), &popup->move_angle,
                            CellType::Real, STRING("move_run"), &popup->move_run,
                            CellType::Real, STRING("move_rise"), &popup->move_rise);
                    if (gui_key_enter) {
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
            } else if (state.click_mode == ClickMode::Polygon) {
                if (two_click_command->awaiting_second_click) {
                    real prev_polygon_distance_to_corner = popup->polygon_distance_to_corner;
                    real prev_polygon_distance_to_side = popup->polygon_distance_to_side;
                    real prev_polygon_side_length = popup->polygon_side_length;
                    real theta = PI / popup->polygon_num_sides;
                    popup_popup(false,
                            CellType::Uint, STRING("polygon_num_sides"), &popup->polygon_num_sides, 
                            CellType::Real, STRING("polygon_distance_to_corner"), &popup->polygon_distance_to_corner,
                            CellType::Real, STRING("polygon_distance_to_side"), &popup->polygon_distance_to_side,
                            CellType::Real, STRING("polygon_side_length"), &popup->polygon_side_length);
                    if (gui_key_enter) {
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
            } else if (state.click_mode == ClickMode::Fillet) {
                popup_popup(false,
                        CellType::Real, STRING("fillet_radius"), &popup->fillet_radius);
            }
        }
        { // popup_close (FORNOW: just doing off of enter transitions)
          // NOTE: we need to do this so that the next key/mouse event doesn't get eaten by a dead popup
            bool enter_mode_transitioned_to_ENTER_MODE_NONE = ((_enter_mode_prev__NOTE_used_to_determine_when_to_close_popup_on_enter != EnterMode::None) && (state.enter_mode == EnterMode::None));
            if (0
                    || (!popup->_popup_actually_called_this_event)
                    || enter_mode_transitioned_to_ENTER_MODE_NONE
               ) {
                popup->_FORNOW_active_popup_unique_ID__FORNOW_name0 = NULL;
            }
        }
    }

    return result;
}

void freshly_baked_event_process(Event freshly_baked_event) {
    bool undo;
    bool redo;
    {
        undo = false;
        redo = false;
        if (freshly_baked_event.type == EventType::Key) {
            KeyEvent *key_event = &freshly_baked_event.key_event;
            auto key_lambda = [key_event](uint key, bool control = false, bool shift = false) -> bool {
                return _key_lambda(key_event, key, control, shift);
            };
            if (!((popup->_FORNOW_active_popup_unique_ID__FORNOW_name0) && (popup->cell_type[popup->active_cell_index] == CellType::String))) { // FORNOW
                undo = (key_lambda('Z', true) || key_lambda('U'));
                redo = (key_lambda('Y', true) || key_lambda('Z', true, true) || key_lambda('U', false, true));
            }
        }
    }

    if (undo) {
        other._please_suppress_drawing_popup_popup = true;
        history_undo();
    } else if (redo) {
        _standard_event_process_NOTE_RECURSIVE({}); // FORNOW (prevent flicker on redo with nothing left to redo)
        other._please_suppress_drawing_popup_popup = true;
        history_redo();
    } else {
        history_process_and_potentially_record_checkpoint_and_or_snapshot_standard_fresh_user_event(freshly_baked_event);
    }
}
