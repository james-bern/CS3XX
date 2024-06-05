// TODO ??? the stuff that is determined while drawing should really be stored in other, not state

void POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER() {
    uint d = popup->cell_index;
    memset(popup->active_cell_buffer, 0, POPUP_CELL_LENGTH);
    if (popup->cell_type[d] == CellType::Real32) {
        sprintf(popup->active_cell_buffer, "%g", *((real *) popup->value[d]));
    } else { ASSERT(popup->cell_type[d] == CellType::String);
        strcpy(popup->active_cell_buffer, (char *) popup->value[d]);
    }
};

void POPUP_WRITE_ACTIVE_CELL_BUFFER_INTO_CORRESPONDING_VALUE() {
    uint d = popup->cell_index;
    if (popup->cell_type[d] == CellType::Real32) {
        *((real *) popup->value[d]) = strtof(popup->active_cell_buffer, NULL);
    } else { ASSERT(popup->cell_type[d] == CellType::String);
        strcpy((char *) popup->value[d], popup->active_cell_buffer);
    }
};

void POPUP_CLEAR_ALL_VALUES_TO_ZERO() {
    for (uint d = 0; d < popup->num_cells; ++d) {
        if (!popup->name[d]) continue;
        if (popup->cell_type[d] == CellType::Real32) {
            *((real *) popup->value[d]) = 0.0f;
        } else { ASSERT(popup->cell_type[d] == CellType::String);
            memset(popup->value[d], 0, POPUP_CELL_LENGTH);
        }
    }
};

bool POPUP_SELECTION_NOT_ACTIVE() { return (popup->selection_cursor == popup->cursor); };

void POPUP_SET_ACTIVE_CELL_INDEX(uint new_cell_index) {
    popup->cell_index = new_cell_index;
    POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER();
    popup->cursor = (uint) strlen(popup->active_cell_buffer);
    popup->selection_cursor = 0; // select whole cell
    popup->_type_of_active_cell = popup->cell_type[popup->cell_index];
};

void popup_popup(
        bool zero_on_load_up,
        CellType _cell_type0,     char *_name0,        void *_value0,
        CellType _cell_type1 = CellType::None, char *_name1 = NULL, void *_value1 = NULL,
        CellType _cell_type2 = CellType::None, char *_name2 = NULL, void *_value2 = NULL,
        CellType _cell_type3 = CellType::None, char *_name3 = NULL, void *_value3 = NULL
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
            for (uint d = 0; d < POPUP_MAX_NUM_CELLS; ++d) if (popup->name[d]) ++popup->num_cells;
            ASSERT(popup->num_cells);
        }
    }


    popup->_popup_actually_called_this_event = true;

    // LOADING UP ///////////////////////////////////////////////////

    if (popup->_active_popup_unique_ID__FORNOW_name0 != _name0) {
        popup->_active_popup_unique_ID__FORNOW_name0 = _name0;
        if (zero_on_load_up) POPUP_CLEAR_ALL_VALUES_TO_ZERO();
        popup->cell_index = 0;
        POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER();
        popup->cursor = strlen(popup->active_cell_buffer);
        popup->selection_cursor = 0;
        popup->_type_of_active_cell = popup->cell_type[popup->cell_index];
    }

    POPUP_WRITE_ACTIVE_CELL_BUFFER_INTO_CORRESPONDING_VALUE(); // FORNOW: do every frame

    /////////////////////////////////////////////
    // drawing (and stuff computed while drawing)
    /////////////////////////////////////////////


    bool FORNOW_mouse_left_held = (other.mouse_left_drag_pane == Pane::Popup);
    popup->mouse_is_hovering = false;
    popup->hover_cell_index = 0;
    popup->hover_cell_index = 0;
    popup->hover_cursor = 0;
    {
        _FOR_(d, popup->num_cells) {
            if (!popup->name[d]) continue;

            uint _strlen_name;
            uint _strlen_extra;
            uint strlen_other;
            uint strlen_cell;
            static char buffer[512];
            {
                // FORNOW gross;
                if (d == popup->cell_index) {
                    sprintf(buffer, "%s %s", popup->name[d], popup->active_cell_buffer);
                } else {
                    if (popup->cell_type[d] == CellType::Real32) {
                        sprintf(buffer,  "%s %g", popup->name[d], *((real *) popup->value[d]));
                    } else { ASSERT(popup->cell_type[d] == CellType::String); 
                        sprintf(buffer,  "%s %s", popup->name[d], ((char *) popup->value[d]));
                    }
                }

                _strlen_name = strlen(popup->name[d]);
                _strlen_extra = 1;
                strlen_other = _strlen_name + _strlen_extra;
                strlen_cell = strlen(buffer) - strlen_other;
            }

            real X_MARGIN_OFFSET = COW1._gui_x_curr;

            real x_field_left;
            real x_field_right;
            real x_selection_left;
            real x_selection_right;
            real y_top;
            real y_bottom;
            {
                static char tmp[4096]; // FORNOW
                strcpy(tmp, &buffer[strlen(popup->name[d]) + 1]); // + 1 for ' '

                x_field_left = X_MARGIN_OFFSET + (stb_easy_font_width(popup->name[d]) + stb_easy_font_width(" ")) - 1.25f;
                x_field_right = x_field_left + stb_easy_font_width(tmp);

                tmp[MAX(popup->cursor, popup->selection_cursor)] = '\0';
                x_selection_right = x_field_left + stb_easy_font_width(tmp);
                tmp[MIN(popup->cursor, popup->selection_cursor)] = '\0';
                x_selection_left = x_field_left + stb_easy_font_width(tmp);
                y_top = COW1._gui_y_curr;
                y_bottom = y_top + 8;
            }

            box2 field_box = { x_field_left, y_top, x_field_right, y_bottom };
            if (box_contains(field_box, other.mouse_Pixel)) {
                popup->mouse_is_hovering = true;
                popup->hover_cell_index = d;
                popup->hover_cursor = 0; // FORNOW_UNUSED
                { // popup->hover_cursor
                    char _2char[2] = {};
                    real x_char_middle = x_field_left;
                    real half_char_width_prev = 0.0f;
                    for (uint i = 0; i < strlen_cell; ++i) {
                        x_char_middle += half_char_width_prev;
                        {
                            _2char[0] = buffer[strlen_other + i];
                            half_char_width_prev = stb_easy_font_width(_2char) / 2.0f;
                        }
                        x_char_middle += half_char_width_prev;

                        real x_mouse = other.mouse_Pixel.x;
                        if (x_mouse > x_char_middle) popup->hover_cursor = i + 1;

                        #if 1
                        eso_begin(globals.NDC_from_Screen, SOUP_LINES, 2.0f, true);
                        eso_color(omax.magenta);
                        eso_vertex(x_char_middle, y_top - 10);
                        eso_vertex(x_char_middle, y_bottom + 10);
                        eso_end();
                        #endif
                    }
                }
            }

            if (!other.please_suppress_drawing_popup_popup) {
                { // debug draw
                    eso_begin(globals.NDC_from_Screen, SOUP_LINE_LOOP, 1.0f);
                    eso_color(FORNOW_mouse_left_held ? omax.blue : omax.orange);
                    eso_vertex(x_field_left, y_top);
                    eso_vertex(x_field_left, y_bottom);
                    eso_vertex(x_field_right, y_bottom);
                    eso_vertex(x_field_right, y_top);
                    eso_end();
                    eso_begin(globals.NDC_from_Screen, SOUP_POINTS, 10.0f);
                    eso_color(1.0f, 0.0f, 1.0f);
                    eso_vertex(other.mouse_Pixel);
                    eso_end();
                }

                if (popup->name[d]) {
                    if (popup->cell_index != d) {
                        // TODO: don't use gui_printf here
                        gui_printf(buffer);
                    } else {
                        if (!POPUP_SELECTION_NOT_ACTIVE()) {
                            eso_begin(globals.NDC_from_Screen, SOUP_QUADS);
                            eso_color(0.4f, 0.4f, 0.0f);
                            eso_vertex(x_selection_left, y_top);
                            eso_vertex(x_selection_right, y_top);
                            eso_vertex(x_selection_right, y_bottom);
                            eso_vertex(x_selection_left, y_bottom);
                            eso_end();
                        }

                        real x = COW1._gui_x_curr;
                        real y = COW1._gui_y_curr;
                        FORNOW_gui_printf_red_component = 0.0f;
                        gui_printf(buffer);
                        FORNOW_gui_printf_red_component = 1.0f;
                        if (POPUP_SELECTION_NOT_ACTIVE()) { // cursor
                                                            // if (((int) (other.time_since_cursor_start * 5)) % 10 < 5)
                            {
                                real a = 0.5f + 0.5f * SIN(other.time_since_cursor_start * 7);
                                real b = CLAMPED_LINEAR_REMAP(other.time_since_cursor_start, 0.0f, 1.0f, 1.0f, 0.0f);
                                char tmp[4096]; // FORNOW
                                strcpy(tmp, popup->active_cell_buffer);
                                tmp[popup->cursor] = '\0';
                                x += (stb_easy_font_width(popup->name[d]) + stb_easy_font_width(" ") + stb_easy_font_width(tmp)); // (FORNOW 2 *)
                                x -= 1.25f;
                                // FORNOW: silly way of getting longer |
                                _text_draw((real *) &globals.NDC_from_Screen, "|", x, y - 3.0, 0.0, 1.0, 1.0, b, a, 0, 0.0, 0.0, true);
                                _text_draw((real *) &globals.NDC_from_Screen, "|", x, y + 3.0, 0.0, 1.0, 1.0, b, a, 0, 0.0, 0.0, true);
                            }
                        }
                    }
                }
            }
        }
    }

    // FORNOW
    other.please_suppress_drawing_popup_popup = true;
};
