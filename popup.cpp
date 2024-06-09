// TODO ??? the stuff that is determined while drawing should really be stored in other, not state

void POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER() {
    uint d = popup->active_cell_index;
    if (popup->cell_type[d] == CellType::Real32) {
        real *value_d_as_real_ptr = (real *)(popup->value[d]);
        popup->active_cell_buffer.length = sprintf(popup->active_cell_buffer.data, "%g", *value_d_as_real_ptr);
    } else { ASSERT(popup->cell_type[d] == CellType::String);
        String *value_d_as_String_ptr = (String *)(popup->value[d]);
        memset(popup->active_cell_buffer.data, 0, POPUP_CELL_LENGTH); // NOTE: unnecessary (FORNOW: for ease of reading in debugger)
        memcpy(popup->active_cell_buffer.data, value_d_as_String_ptr->data, value_d_as_String_ptr->length);
        popup->active_cell_buffer.length = value_d_as_String_ptr->length;
    }
};

void POPUP_WRITE_ACTIVE_CELL_BUFFER_INTO_CORRESPONDING_VALUE() {
    uint d = popup->active_cell_index;
    if (popup->cell_type[d] == CellType::Real32) {
        real *value_d_as_real_ptr = (real *)(popup->value[d]);
        *value_d_as_real_ptr = strtof(popup->active_cell_buffer);
    } else { ASSERT(popup->cell_type[d] == CellType::String);
        String *value_d_as_String_ptr = (String *)(popup->value[d]);
        memcpy(value_d_as_String_ptr->data, popup->active_cell_buffer.data, popup->active_cell_buffer.length);
        value_d_as_String_ptr->length = popup->active_cell_buffer.length;
    }
};

void POPUP_CLEAR_ALL_VALUES_TO_ZERO() {
    for_(d, popup->num_cells) {
        if (!popup->name[d].data) continue;
        if (popup->cell_type[d] == CellType::Real32) {
            real *value_d_as_real_ptr = (real *)(popup->value[d]);
            *value_d_as_real_ptr = 0.0f;
        } else { ASSERT(popup->cell_type[d] == CellType::String);
            String *value_d_as_String_ptr = (String *)(popup->value[d]);
            memset(value_d_as_String_ptr->data, 0, POPUP_CELL_LENGTH); // NOTE: unnecessary (FORNOW: for ease of reading in debugger)
            value_d_as_String_ptr->length = 0;
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

void popup_popup(
        bool zero_on_load_up,
        CellType _cell_type0,                  String _name0,      void *_value0,
        CellType _cell_type1 = CellType::None, String _name1 = {}, void *_value1 = NULL,
        CellType _cell_type2 = CellType::None, String _name2 = {}, void *_value2 = NULL,
        CellType _cell_type3 = CellType::None, String _name3 = {}, void *_value3 = NULL
        ) {

    { // args info
        popup->cell_type[0] = _cell_type0;
        popup->cell_type[1] = _cell_type1;
        popup->cell_type[2] = _cell_type2;
        popup->cell_type[3] = _cell_type3;
        popup->name[0] = _name0;
        popup->name[1] = _name1;
        popup->name[2] = _name2;
        popup->name[3] = _name3;
        popup->value[0] = _value0;
        popup->value[1] = _value1;
        popup->value[2] = _value2;
        popup->value[3] = _value3;

        { // popup->num_cells
            popup->num_cells = 0;
            for_(d, POPUP_MAX_NUM_CELLS) if (popup->name[d].data) ++popup->num_cells;
            ASSERT(popup->num_cells);
        }
    }


    popup->_popup_actually_called_this_event = true;

    // LOADING UP ///////////////////////////////////////////////////

    if (popup->_active_popup_unique_ID__FORNOW_name0 != _name0.data) {
        popup->_active_popup_unique_ID__FORNOW_name0 = _name0.data;
        if (zero_on_load_up) POPUP_CLEAR_ALL_VALUES_TO_ZERO();
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

    // TODO: print as you go (don't call stb_easy_font_width/height)

    EasyTextPen pen = { V2(12.0f), 12.0f, omax.white };

    popup->FORNOW_info_mouse_is_hovering = false;
    popup->info_hover_cell_index = -1;
    popup->info_hover_cell_cursor = -1;
    popup->info_active_cell_cursor = -1;
    {
        for_(d, popup->num_cells) {
            bool d_is_active_cell_index = (popup->active_cell_index == d);

            pen.color = (d_is_active_cell_index) ? omax.cyan : omax.white;

            real y_top;
            real y_bottom;
            real x_field_left;
            real x_field_right;
            bbox2 field_bbox;
            {
                y_top = pen.get_y_Pixel();
                y_bottom = y_top + pen.font_height_Pixel;

                {
                    easy_text_draw(&pen, popup->name[d]);
                    easy_text_draw(&pen, STRING(" "));
                }

                x_field_left = pen.get_x_Pixel();


                x_field_right = pen.get_x_Pixel();
                field_bbox = { x_field_left, y_top, x_field_right, y_bottom };
            }

            String field; {
                if (d == popup->active_cell_index) {
                    field = popup->active_cell_buffer;
                } else {
                    if (popup->cell_type[d] == CellType::Real32) {
                        real *value_d_as_real_ptr = (real *)(popup->value[d]);
                        static _STRING_CALLOC(scratch, POPUP_CELL_LENGTH);
                        scratch.length = snprintf(scratch.data, POPUP_CELL_LENGTH, "%g", *value_d_as_real_ptr);
                        field = scratch;
                    } else { ASSERT(popup->cell_type[d] == CellType::String); 
                        String *value_d_as_String_ptr = (String *)(popup->value[d]);
                        field = *value_d_as_String_ptr;
                    }
                }
                easy_text_draw(&pen, field);
            }

            { // *_cell_cursor (where the cursor is / _will be_)
                uint d_cell_cursor; { 
                    d_cell_cursor = 0;
                    String slice = field;
                    slice.length = 0;
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
                        popup->FORNOW_info_mouse_is_hovering = true;
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

            if (d_is_active_cell_index) {
                if (POPUP_SELECTION_NOT_ACTIVE()) { // draw cursor
                    real x_cursor; {
                        String slice = field;
                        slice.length = popup->cursor;
                        x_cursor = x_field_left + _easy_text_dx(&pen, slice);
                    }
                    real alpha = 0.5f + 0.5f * SIN(other.time_since_cursor_start * 7);
                    text_draw(other.OpenGL_from_Pixel, STRING("|"), V2(x_cursor, y_top - 3.0), V4(omax.yellow, alpha));
                    text_draw(other.OpenGL_from_Pixel, STRING("|"), V2(x_cursor, y_top + 3.0), V4(omax.yellow, alpha));
                } else { // draw selection
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
            } else {
                bool draw_hover_bbox = ((d == popup->info_hover_cell_index) && (other.mouse_left_drag_pane == Pane::None));
                if (draw_hover_bbox) {
                    eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS, 1.0f);
                    eso_color(omax.cyan, 0.4f);
                    eso_bbox_SOUP_QUADS(field_bbox);
                    eso_end();
                }
            }

            // TODO: hover box

            #if 0
            bbox2 field_inflated_box;
            bbox2 field_uninflated_box;
            bbox2 selection_inflated_box = {};
            {
                // real x_selection_left;
                // real x_selection_right;
                // {
                //     static char tmp[4096]; // FORNOW
                //     strcpy(tmp, &buffer[_strlen_name + 1]); // + 1 for ' '

                //     x_field_left = pen.origin_Pixel.x + (stb_easy_font_width(popup->name[d]) + stb_easy_font_width(" ")) - 1.25f;
                //     x_field_right = x_field_left + stb_easy_font_width(tmp);

                //     tmp[MAX(popup->cursor, popup->selection_cursor)] = '\0';
                //     x_selection_right = x_field_left + stb_easy_font_width(tmp);
                //     tmp[MIN(popup->cursor, popup->selection_cursor)] = '\0';
                //     x_selection_left = x_field_left + stb_easy_font_width(tmp);
                // }

                real y_top = pen.get_y_Pixel();
                real y_bottom = y_top + pen.font_height_Pixel;

                field_inflated_box = { x_field_left - 3, y_top - 3, x_field_right + 3, y_bottom + 3 };
                field_uninflated_box = { x_field_left, y_top, x_field_right, y_bottom };
                // selection_inflated_box = { x_selection_left - 0, y_top - 1, x_selection_right + 0, y_bottom + 0 };
            }

            { // *_cell_cursor
                uint d_cell_cursor; { 
                    d_cell_cursor = 0;
                    char _2char[2] = {};
                    real x_char_middle = x_field_left;
                    real half_char_width_prev = 0.0f;
                    for_(i, popup->active_cell_buffer.length) {
                        x_char_middle += half_char_width_prev;
                        {
                            _2char[0] = buffer[strlen_other + i];
                            half_char_width_prev = stb_easy_font_width(_2char) / 2.0f;
                        }
                        x_char_middle += half_char_width_prev;

                        real x_mouse = other.mouse_Pixel.x;
                        if (x_mouse > x_char_middle) d_cell_cursor = i + 1;

                        #if 0
                        eso_begin(OpenGL_from_Pixel.data, SOUP_LINES, 2.0f, true);
                        eso_color(omax.magenta);
                        eso_vertex(x_char_middle, y_top - 10);
                        eso_vertex(x_char_middle, y_bottom + 10);
                        eso_end();
                        #endif
                    }
                }

                { // popup->info_hover_cell_*
                    if (bbox_contains(field_inflated_box, other.mouse_Pixel)) {
                        popup->FORNOW_info_mouse_is_hovering = true; // FORNOW
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

            if (!other.please_suppress_drawing_popup_popup) {
                bool hovering_over_not_active_cell; {
                    hovering_over_not_active_cell = 1
                        && (d == popup->info_hover_cell_index)
                        && (d != popup->active_cell_index)
                        && (other.mouse_left_drag_pane == Pane::None) // FORNOW
                        ;
                }
                if (hovering_over_not_active_cell) { // debug draw
                    eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS, 1.0f);
                    eso_color(omax.cyan, 0.4f);
                    eso_bbox_SOUP_QUADS(field_uninflated_box);
                    eso_end();
                }

                {
                    if (popup->active_cell_index == d) {
                        bool popup_selection_is_active = (!POPUP_SELECTION_NOT_ACTIVE());
                        if (popup_selection_is_active) {
                            eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
                            eso_color(omax.yellow, 0.4f);
                            eso_bbox_SOUP_QUADS(selection_inflated_box);
                            eso_end();
                        }

                        real x = pen.get_position().x;
                        real y = pen.get_position().y;
                        pen.color = omax.cyan; {
                            easy_text_draw(&pen, buffer);
                        } pen.color = omax.white;
                        if (POPUP_SELECTION_NOT_ACTIVE()) { // cursor
                                                            // if (((int) (other.time_since_cursor_start * 5)) % 10 < 5)
                            {
                                real a = 0.5f + 0.5f * SIN(other.time_since_cursor_start * 7);
                                real b = CLAMPED_LINEAR_REMAP(other.time_since_cursor_start, 0.0f, 1.0f, 1.0f, 0.0f);
                                char tmp[4096]; // FORNOW
                                memcpy(tmp, popup->active_cell_buffer.data, popup->active_cell_buffer.length);
                                tmp[popup->cursor] = '\0';
                                x += (stb_easy_font_width(popup->name[d]) + stb_easy_font_width(" ") + stb_easy_font_width(tmp)); // (FORNOW 2 *)
                                x -= 1.25f;
                                // FORNOW: silly way of getting longer |
                                text_draw(other.OpenGL_from_Pixel, STRING("|"), V2(x, y - 3.0), V4(1.0, 1.0, b, a));
                                text_draw(other.OpenGL_from_Pixel, STRING("|"), V2(x, y + 3.0), V4(1.0, 1.0, b, a));
                            }
                        }
                    }
                }
            }
            #endif

            easy_text_draw(&pen, "\n");
        }
    }

    if (0) {
        eso_begin(other.OpenGL_from_Pixel, SOUP_POINTS, 10.0f);
        eso_color(omax.magenta);
        eso_vertex(other.mouse_Pixel);
        eso_end();
    }
    // FORNOW
    other.please_suppress_drawing_popup_popup = true;
};
