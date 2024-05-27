// TODO: the flow here is a bit tortuous
// on the previous frame we store information about the hover state (is the user hovering?--over which cell?--what new cursor position?)
// then...when the user clicks (in the click callback) we can generate a USER_EVENT_TYPE_GUI_MOUSE_PRESS of the appropraite type, which gets processed by the next pass through the popup
// this seems generally fine, i guess what feels weird is that we need to retain some state / store some data (?? perhaps this is morally speaking ScreenState data) in order to classify the event--but i suppose this makes sense

// there is a parallel with deciding what to do with key input
// NOTE: this could be split into two event types

// cool text animations when tabbing
// TODO: 'X'
// TODO: mouse click
// TODO: mouse drag
// TODO: paste from os clipboard
// TODO: parsing math formulas
// TODO: field type

auto popup_popup = [&] (
        bool32 zero_on_load_up,
        uint8 _cell_type0,     char *_name0,        void *_value0,
        uint8 _cell_type1 = 0, char *_name1 = NULL, void *_value1 = NULL,
        uint8 _cell_type2 = 0, char *_name2 = NULL, void *_value2 = NULL,
        uint8 _cell_type3 = 0, char *_name3 = NULL, void *_value3 = NULL
        ) -> void {

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
            for (uint32 d = 0; d < POPUP_MAX_NUM_CELLS; ++d) if (popup->name[d]) ++popup->num_cells;
            ASSERT(popup->num_cells);
        }
    }


    popup_popup_actually_called_this_event = true;

    // LOADING UP ///////////////////////////////////////////////////

    if (popup->_active_popup_unique_ID__FORNOW_name0 != _name0) {
        popup->_active_popup_unique_ID__FORNOW_name0 = _name0;
        if (zero_on_load_up) POPUP_CLEAR_ALL_VALUES_TO_ZERO();
        popup->active_cell_index = 0;
        POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER();
        popup->cursor = (uint32) strlen(popup->active_cell_buffer);
        popup->selection_cursor = 0;
        popup->_type_of_active_cell = popup->cell_type[popup->active_cell_index];
    }

    POPUP_WRITE_ACTIVE_CELL_BUFFER_INTO_CORRESPONDING_VALUE(); // FORNOW: do every frame

    if (! _global_screen_state.DONT_DRAW_ANY_MORE_POPUPS_THIS_FRAME) {
        _global_screen_state.DONT_DRAW_ANY_MORE_POPUPS_THIS_FRAME = true;
    } else {
        return;
    }

    // drawing (and stuff computed while drawing)

    popup->mouse_is_hovering = false;
    popup->hover_cell_index = -1;
    popup->hover_cursor = -1;
    {
        for (uint32 d = 0; d < popup->num_cells; ++d) { // gui_printf
            if (!popup->name[d]) continue;

            uint32 _strlen_name;
            uint32 _strlen_extra;
            uint32 strlen_other;
            uint32 strlen_cell;
            static char buffer[512];
            {
                // FORNOW gross;
                if (d == popup->active_cell_index) {
                    sprintf(buffer, "%s %s", popup->name[d], popup->active_cell_buffer);
                } else {
                    if (popup->cell_type[d] == POPUP_CELL_TYPE_REAL32) {
                        sprintf(buffer,  "%s %g", popup->name[d], *((real32 *) popup->value[d]));
                    } else { ASSERT(popup->cell_type[d] == POPUP_CELL_TYPE_CSTRING); 
                        sprintf(buffer,  "%s %s", popup->name[d], ((char *) popup->value[d]));
                    }
                }

                _strlen_name = strlen(popup->name[d]);
                _strlen_extra = 1;
                strlen_other = _strlen_name + _strlen_extra;
                strlen_cell = strlen(buffer) - strlen_other;
            }


            real32 X_MARGIN_OFFSET = COW1._gui_x_curr;

            real32 x_mouse = _global_screen_state.mouse_Pixel.x;
            // real32 y_mouse = _global_screen_state.mouse_Pixel.y;

            real32 x_field_left;
            real32 x_field_right;
            real32 x_selection_left;
            real32 x_selection_right;
            real32 y_top;
            real32 y_bottom;
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

            bbox2 field_box = { x_field_left, y_top, x_field_right, y_bottom };
            if (bounding_box_contains(field_box, _global_screen_state.mouse_Pixel)) {
                popup->mouse_is_hovering = true;
                popup->hover_cell_index = d;
                popup->hover_cursor = 0; // _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE
                { // popup->hover_cursor
                    char _2char[2] = {};
                    // conversation_messagef("---%f\n", x_mouse);
                    real32 x_char_middle = x_field_left - 1.25f;
                    real32 half_char_width_prev = 0;
                    for (uint32 i = 0; i < strlen_cell; ++i) {
                        x_char_middle += half_char_width_prev;
                        {
                            _2char[0] = buffer[strlen_other + i];
                            half_char_width_prev = stb_easy_font_width(_2char) / 2;
                        }
                        x_char_middle += half_char_width_prev;

                        if (x_mouse > x_char_middle) popup->hover_cursor = i + 1;

                        #if 0
                        eso_begin(globals.NDC_from_Screen, SOUP_LINES, 5.0f);
                        eso_color(color_kelly(i));
                        eso_vertex(x_char_middle, y_top);
                        eso_vertex(x_char_middle, y_bottom);
                        eso_end();
                        #endif
                    }
                }
            }

            // eso_begin(globals.NDC_from_Screen, SOUP_LINE_LOOP, 5.0f);
            // eso_color(0.0f, 0.0f, 1.0f);
            // eso_vertex(x_field_left, y_top);
            // eso_vertex(x_field_left, y_bottom);
            // eso_vertex(x_field_right, y_bottom);
            // eso_vertex(x_field_right, y_top);
            // eso_end();
            // eso_begin(globals.NDC_from_Screen, SOUP_POINTS, 10.0f);
            // eso_color(1.0f, 0.0f, 1.0f);
            // eso_vertex(_global_screen_state.mouse_in_pixel_coordinates);
            // eso_end();

            #if 1
            {
                eso_begin(globals.NDC_from_Screen, SOUP_QUADS);
                eso_color(0.0f, 0.4f, 0.4f);
                eso_vertex(x_field_left, y_top);
                eso_vertex(x_field_right, y_top);
                eso_vertex(x_field_right, y_bottom);
                eso_vertex(x_field_left, y_bottom);
                eso_end();
            }
            #endif

            if (popup->name[d]) {
                if (popup->active_cell_index != d) {
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

                    real32 x = COW1._gui_x_curr;
                    real32 y = COW1._gui_y_curr;
                    FORNOW_gui_printf_red_component = 0.0f;
                    gui_printf(buffer);
                    FORNOW_gui_printf_red_component = 1.0f;
                    if (POPUP_SELECTION_NOT_ACTIVE()) { // cursor
                        // if (((int) (time_since->cursor_start * 5)) % 10 < 5)
                        {
                            real32 a = 0.5f + 0.5f * SIN(time_since->cursor_start * 7);
                            real32 b = CLAMPED_LINEAR_REMAP(time_since->cursor_start, 0.0f, 1.0f, 1.0f, 0.0f);
                            char tmp[4096]; // FORNOW
                            strcpy(tmp, popup->active_cell_buffer);
                            tmp[popup->cursor] = '\0';
                            x += (stb_easy_font_width(popup->name[d]) + stb_easy_font_width(" ") + stb_easy_font_width(tmp)); // (FORNOW 2 *)
                            x -= 1.25f;
                            // FORNOW: silly way of getting longer |
                            _text_draw((cow_real *) &globals.NDC_from_Screen, "|", x, y - 3.0, 0.0, 1.0, 1.0, b, a, 0, 0.0, 0.0, true);
                            _text_draw((cow_real *) &globals.NDC_from_Screen, "|", x, y + 3.0, 0.0, 1.0, 1.0, b, a, 0, 0.0, 0.0, true);
                        }
                    }
                }
            }
        }
    }
};
