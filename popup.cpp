
// other.time_since_cursor_start = 0.0; // FORNOW
// other.time_since_cursor_start = 0.0f;
// the way the popup mouse handling works is very contrived and scary (and i think the/a source of the segfaults)
// currently: popup_popup broadcasts its most recent state elsewhere (is_hovering, PLUS--and this the problem--all sorts of hover varaibles); i think these can get dirty
// TODO: just broadcast is_hovering so we don't have to rely on scary synchronization
// (MouseEventPopup should just have a position in pixel coordinates)

// TODO: could allow user to supply non-zero starting values (this is probably a good idea -- unless it's really just used for revolveadd)
// TODO: consider adding type-checking (NOTE: maybe hard?)
// FORNOW: returns whether it just loaded up
void popup_popup(
        String title, ToolboxGroup group,
        bool zero_on_load_up,
        CellType _cell_type0,                  String _name0,      void *_value0,
        CellType _cell_type1 = CellType::None, String _name1 = {}, void *_value1 = NULL,
        CellType _cell_type2 = CellType::None, String _name2 = {}, void *_value2 = NULL,
        CellType _cell_type3 = CellType::None, String _name3 = {}, void *_value3 = NULL,
        CellType _cell_type4 = CellType::None, String _name4 = {}, void *_value4 = NULL
        ) {

    popup->manager.register_call_to_popup_popup(group);

    CellType popup_cell_type[POPUP_MAX_NUM_CELLS];
    String popup_name[POPUP_MAX_NUM_CELLS];
    void *popup_value[POPUP_MAX_NUM_CELLS];
    uint popup_num_cells;
    {
        popup_cell_type[0] = _cell_type0;
        popup_cell_type[1] = _cell_type1;
        popup_cell_type[2] = _cell_type2;
        popup_cell_type[3] = _cell_type3;
        popup_cell_type[4] = _cell_type4;
        popup_name[0] = _name0;
        popup_name[1] = _name1;
        popup_name[2] = _name2;
        popup_name[3] = _name3;
        popup_name[4] = _name4;
        popup_value[0] = _value0;
        popup_value[1] = _value1;
        popup_value[2] = _value2;
        popup_value[3] = _value3;
        popup_value[4] = _value4;

        { // popup_num_cells
            popup_num_cells = 0;
            for_(d, POPUP_MAX_NUM_CELLS) if (popup_name[d].data) ++popup_num_cells;
            ASSERT(popup_num_cells);
        }
    }


    auto POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER = [&]() -> void {
        uint d = popup->active_cell_index;
        if (popup_cell_type[d] == CellType::Real) {
            real *value_d_as_real_ptr = (real *)(popup_value[d]);
            popup->active_cell_buffer.length = sprintf(popup->active_cell_buffer.data, "%g", *value_d_as_real_ptr);
        } else if (popup_cell_type[d] == CellType::Uint) {
            uint *value_d_as_uint_ptr = (uint *)(popup_value[d]);
            popup->active_cell_buffer.length = sprintf(popup->active_cell_buffer.data, "%u", *value_d_as_uint_ptr);
        } else { ASSERT(popup_cell_type[d] == CellType::String);
            String *value_d_as_String_ptr = (String *)(popup_value[d]);
            memcpy(popup->active_cell_buffer.data, value_d_as_String_ptr->data, value_d_as_String_ptr->length);
            popup->active_cell_buffer.length = value_d_as_String_ptr->length;
            popup->active_cell_buffer.data[popup->active_cell_buffer.length] = '\0'; // FORNOW: ease of reading in debugger
        }
    };
    auto POPUP_CLEAR_ALL_VALUES_TO_ZERO = [&]() -> void {
        for_(d, popup_num_cells) {
            if (!popup_name[d].data) continue;
            if (popup_cell_type[d] == CellType::Real) {
                real *value_d_as_real_ptr = (real *)(popup_value[d]);
                *value_d_as_real_ptr = 0.0f;
            } else if (popup_cell_type[d] == CellType::Uint) {
                uint *value_d_as_uint_ptr = (uint *)(popup_value[d]);
                *value_d_as_uint_ptr = 0;
            } else { ASSERT(popup_cell_type[d] == CellType::String);
                String *value_d_as_String_ptr = (String *)(popup_value[d]);
                value_d_as_String_ptr->length = 0;
                value_d_as_String_ptr->data[value_d_as_String_ptr->length] = '\0'; // FORNOW: ease of reading in debugger
            }
        }
    };


    auto POPUP_SYNC_ACTIVE_CELL_BUFFER = [&]() -> void {
        uint d = popup->active_cell_index;
        if (popup_cell_type[d] == CellType::Real) {
            real *value_d_as_real_ptr = (real *)(popup_value[d]);
            // FORNOW: null-terminating and calling strto*
            popup->active_cell_buffer.data[popup->active_cell_buffer.length] = '\0';
            *value_d_as_real_ptr = strtof(popup->active_cell_buffer);
        } else if (popup_cell_type[d] == CellType::Uint) {
            uint *value_d_as_uint_ptr = (uint *)(popup_value[d]);
            // FORNOW: null-terminating and calling strto*
            popup->active_cell_buffer.data[popup->active_cell_buffer.length] = '\0';
            *value_d_as_uint_ptr = uint(strtol(popup->active_cell_buffer.data, NULL, 10));
        } else { ASSERT(popup_cell_type[d] == CellType::String);
            String *value_d_as_String_ptr = (String *)(popup_value[d]);
            memcpy(value_d_as_String_ptr->data, popup->active_cell_buffer.data, popup->active_cell_buffer.length);
            value_d_as_String_ptr->length = popup->active_cell_buffer.length;

            // FORNOW: keeping null-termination around for messagef?
            value_d_as_String_ptr->data[value_d_as_String_ptr->length] = '\0';
        }
    };


    auto POPUP_SELECTION_NOT_ACTIVE = [&]() -> bool {
        return (popup->selection_cursor == popup->cursor);
    };

    auto POPUP_SET_ACTIVE_CELL_INDEX = [&](int new_active_cell_index) -> void {
        popup->active_cell_index = new_active_cell_index;
        POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER();
        popup->cursor = popup->active_cell_buffer.length;
        popup->selection_cursor = 0; // select whole cell
        popup->_type_of_active_cell = popup_cell_type[popup->active_cell_index];
    };


    bool dont_draw_because_already_called = popup->a_popup_from_this_group_was_already_called_this_frame[uint(group)]; // this is for dragging the mouse and not having the transparent rectangles flicker
    popup->a_popup_from_this_group_was_already_called_this_frame[uint(group)] = true;
    bool dont_draw = (dont_draw_because_already_called || other._please_suppress_drawing_popup_popup); // NOTE: _please_suppress_drawing_popup_popup is for undo / redo
    FORNOW_UNUSED(dont_draw);
    bool is_focused = (group == popup->manager.focus_group);

    vec3 raw_accent_color;
    vec3 accent_color;
    vec3 lighter_gray;
    vec3 darker_gray;
    EasyTextPen pen; 
    {
        {
            raw_accent_color = get_accent_color(group);
            if (is_focused) {
                accent_color = raw_accent_color;
                lighter_gray = omax.light_gray;
                darker_gray = omax.gray;
                lighter_gray = LERP(0.2f, lighter_gray, raw_accent_color);
                darker_gray = LERP(0.1f, darker_gray, raw_accent_color);
            } else {
                accent_color = LERP(0.1f, omax.gray, raw_accent_color);
                lighter_gray = omax.dark_gray;
                darker_gray = omax.dark_gray;
                lighter_gray = LERP(0.1f, lighter_gray, raw_accent_color);
                darker_gray = LERP(0.05f, darker_gray, raw_accent_color);
            }
        }
        {
            pen = { V2(120.0f, 12.0f), 22.0f, AVG(lighter_gray, accent_color) };
            if (group == ToolboxGroup::Mesh) {
                pen.origin.x += get_x_divider_drawing_mesh_Pixel() - 50.0f;
            } else if (group == ToolboxGroup::Snap) {
                // pen.origin.x = get_x_divider_drawing_mesh_Pixel() - 128.0f
                pen.origin.y += 128.0f;
            }
            easy_text_draw(&pen, title);
            pen.origin.x += pen.offset_Pixel.x + 12.0f;
            pen.offset_Pixel.x = 0.0f;
            pen.origin.y += 2.5f; // FORNOW
            pen.font_height_Pixel = 18.0f;
        }
    }

    // FORNOW: HACK: i'm computing all of these based on the current other.mouse_Pixel
    //               (i am NOT actually looking at where the mouse was when the event happened)
    //               ((this is probably usually fine, and can be switched over to the "proper" approach without too much trouble"))
    bool popup_info_is_hovering = false;
    uint popup_info_hover_cell_index;
    uint popup_info_hover_cell_cursor;
    uint popup_info_active_cell_cursor;
    for_(d, popup_num_cells) {
        bool d_is_active_cell_index; {
            d_is_active_cell_index = true;
            d_is_active_cell_index &= is_focused;
            d_is_active_cell_index &= (d == popup->active_cell_index);
        }

        real y_top;
        real y_bottom;
        real x_field_left;
        real x_field_right;
        String field; 
        bbox2 field_bbox;
        { // easy_text_draw
            pen.color = (d_is_active_cell_index) ? accent_color : lighter_gray;
            y_top = pen.get_y_Pixel();
            y_bottom = y_top + (0.8f * pen.font_height_Pixel);

            easy_text_draw(&pen, popup_name[d]);
            easy_text_drawf(&pen, ": ");

            x_field_left = pen.get_x_Pixel() - (pen.font_height_Pixel / 12.0f);

            { // field 
                if (d_is_active_cell_index) {
                    field = popup->active_cell_buffer;
                } else {
                    if (popup_cell_type[d] == CellType::Real) {
                        real *value_d_as_real_ptr = (real *)(popup_value[d]);
                        static _STRING_CALLOC(scratch, POPUP_CELL_LENGTH);
                        scratch.length = snprintf(scratch.data, POPUP_CELL_LENGTH, "%g", *value_d_as_real_ptr);
                        field = scratch;
                    } else if (popup_cell_type[d] == CellType::Uint) {
                        uint *value_d_as_uint_ptr = (uint *)(popup_value[d]);
                        static _STRING_CALLOC(scratch, POPUP_CELL_LENGTH);
                        scratch.length = snprintf(scratch.data, POPUP_CELL_LENGTH, "%u", *value_d_as_uint_ptr);
                        field = scratch;
                    } else { ASSERT(popup_cell_type[d] == CellType::String); 
                        String *value_d_as_String_ptr = (String *)(popup_value[d]);
                        field = *value_d_as_String_ptr;
                    }
                }
            }

            easy_text_draw(&pen, field);

            x_field_right = pen.get_x_Pixel();

            easy_text_drawf(&pen, "\n");

            field_bbox = { x_field_left, y_top, x_field_right, y_bottom };
        }

        {
            { // *_cell_cursor (where the cursor is / _will_ be)
                uint d_cell_cursor; {
                    // FORNOW: O(n)
                    d_cell_cursor = 0;
                    String slice = field;
                    slice.length = 1;
                    real x_char_middle = x_field_left;
                    real half_char_width_prev = 0.0f;
                    for_(i, field.length) {
                        x_char_middle += half_char_width_prev;
                        {
                            half_char_width_prev = 0.5f * _easy_text_dx(&pen, slice);
                            ++slice.data;
                        }
                        x_char_middle += half_char_width_prev;

                        real x_mouse = other.mouse_Pixel.x;
                        if (x_mouse > x_char_middle) d_cell_cursor = i + 1;
                    }
                }

                { // popup_info_hover_cell_*
                    if (bbox_contains(field_bbox, other.mouse_Pixel)) {
                        popup->_FORNOW_info_mouse_is_hovering = true; // FORNOW
                        popup_info_is_hovering = true; // FORNOW
                        popup_info_hover_cell_index = d;
                        popup_info_hover_cell_cursor = d_cell_cursor;
                    }
                }

                { // popup_info_active_cell_cursor
                    if (d == popup->active_cell_index) {
                        popup_info_active_cell_cursor = d_cell_cursor;
                    }
                }
            }

            { // draw cursor selection_bbox hover_bbox
                if (d_is_active_cell_index) { // draw cursor selection_bbox
                    if (POPUP_SELECTION_NOT_ACTIVE()) { // draw cursor
                        real x_cursor; {
                            String slice = field;
                            slice.length = popup->cursor;
                            x_cursor = x_field_left + _easy_text_dx(&pen, slice);
                        }
                        real sint = SIN(other.time_since_cursor_start * 7);
                        real alpha = 0.5f + 0.5f * sint;
                        eso_begin(other.OpenGL_from_Pixel, SOUP_LINES);
                        eso_size(1.5f + 1.5f * sint);
                        SIN(other.time_since_cursor_start * 7);
                        eso_color(accent_color, alpha);
                        eso_vertex(x_cursor, y_top);
                        eso_vertex(x_cursor, y_bottom);
                        eso_end();
                    } else { // draw selection_bbox
                        real x_selection_left;
                        real x_selection_right;
                        {
                            uint left_cursor = MIN(popup->cursor, popup->selection_cursor);
                            uint right_cursor = MAX(popup->cursor, popup->selection_cursor);

                            String slice = field;
                            slice.length = left_cursor;
                            x_selection_left = (x_field_left + _easy_text_dx(&pen, slice));
                            slice.data += left_cursor;
                            slice.length = (right_cursor - left_cursor);
                            x_selection_right = (x_selection_left + _easy_text_dx(&pen, slice));
                        }
                        bbox2 selection_bbox = { x_selection_left, y_top, x_selection_right, y_bottom };
                        eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
                        eso_color(accent_color, 0.4f);
                        eso_bbox_SOUP_QUADS(selection_bbox);
                        eso_end();
                    }
                } else if (popup_info_is_hovering) { // draw hover_bbox
                    bool d_is_hover_cell_index = (d == popup_info_hover_cell_index);
                    bool draw_hover_bbox = ((d_is_hover_cell_index) && (other.mouse_left_drag_pane == Pane::None));
                    if (draw_hover_bbox) {
                        eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
                        eso_overlay(true);
                        eso_color(raw_accent_color, 0.4f);
                        eso_bbox_SOUP_QUADS(field_bbox);
                        eso_end();
                    }
                }
            }
        }
    }

    bool special_case_click_on_inactive_popup = false;
    if (!already_processed_event_passed_to_popups) { // event handling
        Event *event = &event_passed_to_popups;

        if (event->type == EventType::Key) {
            if (is_focused) {
                KeyEvent *key_event = &event->key_event;
                if (key_event->subtype == KeyEventSubtype::Popup) {

                    uint key = key_event->key;
                    bool shift = key_event->shift;
                    bool control = key_event->control;

                    bool _tab_hack_so_aliases_not_introduced_too_far_up = false;
                    if (key == GLFW_KEY_TAB) {
                        _tab_hack_so_aliases_not_introduced_too_far_up = true;
                        if (!control) {
                            already_processed_event_passed_to_popups = true; // FORNOW; TODO: CTRL+TAB should be handled by the next popup in the series

                            uint new_active_cell_index; {
                                // FORNOW
                                if (!shift) {
                                    new_active_cell_index = (popup->active_cell_index + 1) % popup_num_cells;
                                } else {
                                    if (popup->active_cell_index != 0) {
                                        new_active_cell_index = popup->active_cell_index - 1;
                                    } else {
                                        new_active_cell_index = popup_num_cells - 1;
                                    }
                                }
                            }
                            POPUP_SET_ACTIVE_CELL_INDEX(new_active_cell_index);
                        }
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
            } else { // CTRL+TAB
                KeyEvent *key_event = &event->key_event;
                if (key_event->subtype == KeyEventSubtype::Popup) {
                    uint key = key_event->key;
                    bool control = key_event->control;
                    bool shift = key_event->shift;
                    if (control && (key == GLFW_KEY_TAB)) {
                        ToolboxGroup next_group = popup->manager.focus_group;
                        do {
                            if (!shift) {
                                next_group = ToolboxGroup(uint(next_group) - 1);
                                if (next_group == ToolboxGroup::None) {
                                    next_group = ToolboxGroup(uint(ToolboxGroup::NUMBER_OF) - 1);
                                }
                            } else {
                                next_group = ToolboxGroup(uint(next_group) + 1);
                                if (next_group == ToolboxGroup::NUMBER_OF) {
                                    next_group = ToolboxGroup(uint(ToolboxGroup::None) + 1);
                                }
                            }
                        } while (popup->manager.get_tag(next_group) == NULL);

                        if (next_group == group) {
                            already_processed_event_passed_to_popups = true;
                            popup->manager.manually_set_focus_group(group);
                        }
                    }
                }
            }
        }


        if (event->type == EventType::Mouse) {
            MouseEvent *mouse_event = &event->mouse_event;
            if (mouse_event->subtype == MouseEventSubtype::Popup) {

                MouseEventPopup *mouse_event_popup = &mouse_event->mouse_event_popup;
                FORNOW_UNUSED(mouse_event_popup); // FORNOW we're just using other.mouse_Pixel

                if (!mouse_event->mouse_held) { // press
                    if (popup_info_is_hovering) {
                        already_processed_event_passed_to_popups = true; // NOTE: does NOT require focus!

                        if (is_focused) {
                            if (popup->active_cell_index == popup_info_hover_cell_index) { // same cell
                                bool double_click = (POPUP_SELECTION_NOT_ACTIVE()) && (popup->cursor == popup_info_hover_cell_cursor);
                                if (double_click) { // select all (double click)
                                    popup->cursor = popup->active_cell_buffer.length;
                                    popup->selection_cursor = 0;
                                } else { // move
                                    popup->cursor = popup_info_hover_cell_cursor;
                                    popup->selection_cursor = popup->cursor;
                                }
                            } else { // switch cell
                                POPUP_SET_ACTIVE_CELL_INDEX(popup_info_hover_cell_index);
                                popup->cursor = popup_info_hover_cell_cursor;
                                popup->selection_cursor = popup->cursor;
                            }
                        } else {
                            special_case_click_on_inactive_popup = true;
                            popup->manager.manually_set_focus_group(group);
                        }
                    }
                } else { // drag
                    if (is_focused) {
                        already_processed_event_passed_to_popups = true;
                        popup->selection_cursor = popup_info_active_cell_cursor;
                    }
                }
            }
        }
    }

    { // load up
        bool tag_corresponding_to_this_group_was_changed = (popup->manager.get_tag(group) != _name0.data);
        bool tag_corresponding_to_focus_group_became_NULL = (popup->manager.focus_group != ToolboxGroup::None) && (popup->manager.get_tag(popup->manager.focus_group) == NULL);
        bool focus_group_was_manually_set_to_this_group = (popup->manager.focus_group_was_set_manually && (group == popup->manager.focus_group));
        bool common = (focus_group_was_manually_set_to_this_group || tag_corresponding_to_this_group_was_changed || tag_corresponding_to_focus_group_became_NULL);

        if (tag_corresponding_to_this_group_was_changed) {
            popup->manager.set_tag(group, _name0.data);
            if (zero_on_load_up) POPUP_CLEAR_ALL_VALUES_TO_ZERO();
        }

        if (common) {
            popup->manager.focus_group = group;
            popup->active_cell_index = 0;
            if (special_case_click_on_inactive_popup) popup->active_cell_index = popup_info_hover_cell_index;
            POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER();
            popup->cursor = popup->active_cell_buffer.length;
            popup->selection_cursor = 0;
            if (special_case_click_on_inactive_popup) { popup->cursor = popup_info_hover_cell_cursor; popup->selection_cursor = popup->cursor; }
            popup->_type_of_active_cell = popup_cell_type[popup->active_cell_index];
        }
    }

    if (is_focused) POPUP_SYNC_ACTIVE_CELL_BUFFER();
};
