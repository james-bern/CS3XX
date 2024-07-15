// TODO: could allow user to supply non-zero starting values (this is probably a good idea -- unless it's really just used for revolveadd)

void POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER() {
    uint d = popup->active_cell_index;
    if (popup->cell_type[d] == CellType::Real) {
        real *value_d_as_real_ptr = (real *)(popup->value[d]);
        popup->active_cell_buffer.length = sprintf(popup->active_cell_buffer.data, "%g", *value_d_as_real_ptr);
    } else if (popup->cell_type[d] == CellType::Uint) {
        uint *value_d_as_uint_ptr = (uint *)(popup->value[d]);
        popup->active_cell_buffer.length = sprintf(popup->active_cell_buffer.data, "%u", *value_d_as_uint_ptr);
    } else { ASSERT(popup->cell_type[d] == CellType::String);
        String *value_d_as_String_ptr = (String *)(popup->value[d]);
        memcpy(popup->active_cell_buffer.data, value_d_as_String_ptr->data, value_d_as_String_ptr->length);
        popup->active_cell_buffer.length = value_d_as_String_ptr->length;
        popup->active_cell_buffer.data[popup->active_cell_buffer.length] = '\0'; // FORNOW: ease of reading in debugger
    }
};

void POPUP_WRITE_ACTIVE_CELL_BUFFER_INTO_CORRESPONDING_VALUE() {
    uint d = popup->active_cell_index;
    if (popup->cell_type[d] == CellType::Real) {
        real *value_d_as_real_ptr = (real *)(popup->value[d]);
        // FORNOW: null-terminating and calling strto*
        popup->active_cell_buffer.data[popup->active_cell_buffer.length] = '\0';
        *value_d_as_real_ptr = strtof(popup->active_cell_buffer);
    } else if (popup->cell_type[d] == CellType::Uint) {
        uint *value_d_as_uint_ptr = (uint *)(popup->value[d]);
        // FORNOW: null-terminating and calling strto*
        popup->active_cell_buffer.data[popup->active_cell_buffer.length] = '\0';
        *value_d_as_uint_ptr = uint(strtol(popup->active_cell_buffer.data, NULL, 10));
    } else { ASSERT(popup->cell_type[d] == CellType::String);
        String *value_d_as_String_ptr = (String *)(popup->value[d]);
        memcpy(value_d_as_String_ptr->data, popup->active_cell_buffer.data, popup->active_cell_buffer.length);
        value_d_as_String_ptr->length = popup->active_cell_buffer.length;

        // FORNOW: keeping null-termination around for messagef?
        value_d_as_String_ptr->data[value_d_as_String_ptr->length] = '\0';
    }
};

void POPUP_CLEAR_ALL_VALUES_TO_ZERO() {
    for_(d, popup->num_cells) {
        if (!popup->name[d].data) continue;
        if (popup->cell_type[d] == CellType::Real) {
            real *value_d_as_real_ptr = (real *)(popup->value[d]);
            *value_d_as_real_ptr = 0.0f;
        } else if (popup->cell_type[d] == CellType::Uint) {
            uint *value_d_as_uint_ptr = (uint *)(popup->value[d]);
            *value_d_as_uint_ptr = 0;
        } else { ASSERT(popup->cell_type[d] == CellType::String);
            String *value_d_as_String_ptr = (String *)(popup->value[d]);
            value_d_as_String_ptr->length = 0;
            value_d_as_String_ptr->data[value_d_as_String_ptr->length] = '\0'; // FORNOW: ease of reading in debugger
        }
    }
};

bool POPUP_SELECTION_NOT_ACTIVE() { return (popup->selection_cursor == popup->cursor); };

void POPUP_SET_ACTIVE_CELL_INDEX(uint new_active_cell_index) {
    popup->active_cell_index = new_active_cell_index;
    POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER();
    popup->cursor = popup->active_cell_buffer.length;
    popup->selection_cursor = 0; // select whole cell
    popup->_type_of_active_cell = popup->cell_type[popup->active_cell_index];
};

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

    { // args info
        popup->cell_type[0] = _cell_type0;
        popup->cell_type[1] = _cell_type1;
        popup->cell_type[2] = _cell_type2;
        popup->cell_type[3] = _cell_type3;
        popup->cell_type[4] = _cell_type4;
        popup->name[0] = _name0;
        popup->name[1] = _name1;
        popup->name[2] = _name2;
        popup->name[3] = _name3;
        popup->name[4] = _name4;
        popup->value[0] = _value0;
        popup->value[1] = _value1;
        popup->value[2] = _value2;
        popup->value[3] = _value3;
        popup->value[4] = _value4;

        { // popup->num_cells
            popup->num_cells = 0;
            for_(d, POPUP_MAX_NUM_CELLS) if (popup->name[d].data) ++popup->num_cells;
            ASSERT(popup->num_cells);
        }
    }

    popup->_popup_actually_called_this_event = true;

    // LOADING UP ///////////////////////////////////////////////////

    if (popup->_FORNOW_active_popup_unique_ID__FORNOW_name0 != _name0.data) {
        popup->_FORNOW_active_popup_unique_ID__FORNOW_name0 = _name0.data;
        if (zero_on_load_up) {
            POPUP_CLEAR_ALL_VALUES_TO_ZERO();
        }
        popup->active_cell_index = 0;
        POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER();
        popup->cursor = popup->active_cell_buffer.length;
        popup->selection_cursor = 0;
        popup->_type_of_active_cell = popup->cell_type[popup->active_cell_index];
    }

    POPUP_WRITE_ACTIVE_CELL_BUFFER_INTO_CORRESPONDING_VALUE(); // FORNOW: do every frame

    /////////////////////////////////////////////
    // drawing (and stuff computed while drawing)
    /////////////////////////////////////////////

    EasyTextPen pen = { V2(128.0f, 12.0f), 22.0f, AVG(omax.white, get_accent_color(group)) };
    if (group == ToolboxGroup::Mesh) pen.origin_Pixel.x += get_x_divider_drawing_mesh_Pixel();
    if (!other._please_suppress_drawing_popup_popup) {
        easy_text_draw(&pen, title);
        pen.origin_Pixel.x += pen.offset_Pixel.x + 12.0f;
        pen.offset_Pixel.x = 0.0f;
        pen.origin_Pixel.y += 2.5f; // FORNOW
        pen.font_height_Pixel = 18.0f;
    }

    popup->_FORNOW_info_mouse_is_hovering = false;
    popup->info_hover_cell_index = uint(-1);
    popup->info_hover_cell_cursor = uint(-1);
    popup->info_active_cell_cursor = uint(-1);
    {
        for_(d, popup->num_cells) {
            bool d_is_active_cell_index = (popup->active_cell_index == d);

            pen.color = (d_is_active_cell_index) ? get_accent_color(group) : omax.light_gray;

            real y_top;
            real y_bottom;
            real x_field_left;
            real x_field_right;
            String field; 
            bbox2 field_bbox;
            {
                y_top = pen.get_y_Pixel();
                y_bottom = y_top + (0.8f * pen.font_height_Pixel);

                if (!other._please_suppress_drawing_popup_popup) {
                    easy_text_draw(&pen, popup->name[d]);
                    easy_text_drawf(&pen, ": ");
                }

                x_field_left = pen.get_x_Pixel() - (pen.font_height_Pixel / 12.0f);

                { // field 
                    if (d == popup->active_cell_index) {
                        field = popup->active_cell_buffer;
                    } else {
                        if (popup->cell_type[d] == CellType::Real) {
                            real *value_d_as_real_ptr = (real *)(popup->value[d]);
                            static _STRING_CALLOC(scratch, POPUP_CELL_LENGTH);
                            scratch.length = snprintf(scratch.data, POPUP_CELL_LENGTH, "%g", *value_d_as_real_ptr);
                            field = scratch;
                        } else if (popup->cell_type[d] == CellType::Uint) {
                            uint *value_d_as_uint_ptr = (uint *)(popup->value[d]);
                            static _STRING_CALLOC(scratch, POPUP_CELL_LENGTH);
                            scratch.length = snprintf(scratch.data, POPUP_CELL_LENGTH, "%u", *value_d_as_uint_ptr);
                            field = scratch;
                        } else { ASSERT(popup->cell_type[d] == CellType::String); 
                            String *value_d_as_String_ptr = (String *)(popup->value[d]);
                            field = *value_d_as_String_ptr;
                        }
                    }
                }

                if (!other._please_suppress_drawing_popup_popup) easy_text_draw(&pen, field);

                x_field_right = pen.get_x_Pixel();
                field_bbox = { x_field_left, y_top, x_field_right, y_bottom };
            }

            { // *_cell_cursor (where the cursor is / _will be_)
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

                { // popup->info_hover_cell_*
                    if (bbox_contains(field_bbox, other.mouse_Pixel)) {
                        popup->_FORNOW_info_mouse_is_hovering = true;
                        popup->info_hover_cell_index = d;
                        popup->info_hover_cell_cursor = d_cell_cursor;
                    }
                }

                { // popup->info_active_cell_cursor
                    if (d == popup->active_cell_index) {
                        popup->info_active_cell_cursor = d_cell_cursor;
                    }
                }
            }

            if (!other._please_suppress_drawing_popup_popup) { // draw cursor selection_bbox hover_bbox
                if (d_is_active_cell_index) { // draw cursor selection_bbox
                    if (POPUP_SELECTION_NOT_ACTIVE()) { // draw cursor
                        real x_cursor; {
                            String slice = field;
                            slice.length = popup->cursor;
                            x_cursor = x_field_left + _easy_text_dx(&pen, slice);
                        }
                        real alpha = 0.5f + 0.5f * SIN(other.time_since_cursor_start * 7);
                        eso_begin(other.OpenGL_from_Pixel, SOUP_LINES);
                        eso_color(AVG(omax.white, omax.yellow), alpha);
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
                        eso_color(omax.yellow, 0.4f);
                        eso_bbox_SOUP_QUADS(selection_bbox);
                        eso_end();
                    }
                } else { // draw hover_bbox
                    bool d_is_hover_cell_index = (d == popup->info_hover_cell_index);
                    bool draw_hover_bbox = ((d_is_hover_cell_index) && (other.mouse_left_drag_pane == Pane::None));
                    if (draw_hover_bbox) {
                        eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
                        eso_overlay(true);
                        eso_color(omax.cyan, 0.4f);
                        eso_bbox_SOUP_QUADS(field_bbox);
                        eso_end();
                    }
                }

                if (!other._please_suppress_drawing_popup_popup) easy_text_drawf(&pen, "\n");
            }
        }
    }

    other._please_suppress_drawing_popup_popup = true;
};
