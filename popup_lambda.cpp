// cool text animations when tabbing
// TODO: 'X'
// TODO: mouse click
// TODO: mouse drag
// TODO: paste from os clipboard
// TODO: parsing math formulas
// TODO: field type


auto popup_popup = [&] (
        bool32 zero_on_load_up,
        uint8 cell_type0, char *_name0, void *_value0,
        uint8 cell_type1 = 0, char *_name1 = NULL, void *_value1 = NULL,
        uint8 cell_type2 = 0, char *_name2 = NULL, void *_value2 = NULL,
        uint8 cell_type3 = 0, char *_name3 = NULL, void *_value3 = NULL
        ) -> void {

    PopupState *popup = &global_world_state.popup;

    ///

    char *name[]       = {     _name0,     _name1,     _name2,     _name3 };
    void *value[]      = {    _value0,    _value1,    _value2,    _value3 };
    uint32 cell_type[] = { cell_type0, cell_type1, cell_type2, cell_type3 };
    uint32 num_cells; {
        num_cells = 0;
        for (uint32 d = 0; d < ARRAY_LENGTH(name); ++d) if (name[d]) ++num_cells;
    }

    ASSERT(num_cells);

    ///

    popup_popup_actually_called_this_event = true;

    /////////////////////////////////////////////////////////////////

    auto LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER = [&]() {
        uint32 d = popup->index_of_active_cell;
        memset(popup->active_cell_buffer, 0, POPUP_CELL_LENGTH);
        if (cell_type[d] == CELL_TYPE_REAL32) {
            sprintf(popup->active_cell_buffer, "%g", *((real32 *) value[d]));
        } else { ASSERT(cell_type[d] == CELL_TYPE_CSTRING);
            strcpy(popup->active_cell_buffer, (char *) value[d]);
        }
    };

    auto WRITE_ACTIVE_CELL_BUFFER_INTO_CORRESPONDING_VALUE = [&]() {
        uint32 d = popup->index_of_active_cell;
        if (cell_type[d] == CELL_TYPE_REAL32) {
            *((real32 *) value[d]) = strtof(popup->active_cell_buffer, NULL);
        } else { ASSERT(cell_type[d] == CELL_TYPE_CSTRING);
            strcpy((char *) value[d], popup->active_cell_buffer);
        }
    };

    auto CLEAR_ALL_VALUES_TO_ZERO = [&]() {
        for (uint32 d = 0; d < num_cells; ++d) {
            if (!name[d]) continue;
            if (cell_type[d] == CELL_TYPE_REAL32) {
                *((real32 *) value[d]) = 0.0f;
            } else { ASSERT(cell_type[d] == CELL_TYPE_CSTRING);
                memset(value[d], 0, POPUP_CELL_LENGTH);
            }
        }
    };

    // LOADING UP ///////////////////////////////////////////////////

    if (popup->_active_popup_unique_ID__FORNOW_name0 != _name0) {
        popup->_active_popup_unique_ID__FORNOW_name0 = _name0;
        if (zero_on_load_up) CLEAR_ALL_VALUES_TO_ZERO();
        popup->index_of_active_cell = 0;
        LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER();
        popup->cursor = (uint32) strlen(popup->active_cell_buffer);
        popup->selection_cursor = 0;
        popup->_type_of_active_cell = cell_type[popup->index_of_active_cell];
    }

    // KEY PRESS ////////////////////////////////////////////////////


    ASSERT(popup->index_of_active_cell >= 0);
    ASSERT(popup->index_of_active_cell <= num_cells);
    ASSERT(popup->cursor >= 0);
    ASSERT(popup->cursor <= POPUP_CELL_LENGTH);
    ASSERT(popup->selection_cursor >= 0);
    ASSERT(popup->selection_cursor <= POPUP_CELL_LENGTH);

    auto SELECTION_NOT_ACTIVE = [&]() -> bool { return (popup->selection_cursor == popup->cursor); };

    if (event.type == USER_EVENT_TYPE_KEY_PRESS) {
        _global_screen_state.popup_blinker_time = 0.0; // FORNOW

        uint32 key = event.key;
        bool32 shift = event.shift;
        bool32 super = event.super;


        bool32 _tab_hack_so_aliases_not_introduced_too_far_up = false;
        if (key == GLFW_KEY_TAB) {
            _tab_hack_so_aliases_not_introduced_too_far_up = true;
            { // change index_of_active_cell
                if (!shift) {
                    ++popup->index_of_active_cell;
                } else {
                    if (popup->index_of_active_cell != 0) {
                        --popup->index_of_active_cell;
                    } else {
                        popup->index_of_active_cell = num_cells - 1;
                    }
                }
                popup->index_of_active_cell = MODULO(popup->index_of_active_cell, num_cells);
            }
            LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER();
            popup->cursor = (uint32) strlen(popup->active_cell_buffer);
            popup->selection_cursor = 0;
            popup->_type_of_active_cell = cell_type[popup->index_of_active_cell];
        }

        char *active_cell = popup->active_cell_buffer;
        uint32 len = (uint32) strlen(active_cell);
        uint32 left_cursor = MIN(popup->cursor, popup->selection_cursor);
        uint32 right_cursor = MAX(popup->cursor, popup->selection_cursor);

        if (_tab_hack_so_aliases_not_introduced_too_far_up) {
        } else if (super && (key == 'A')) {
            popup->cursor = len;
            popup->selection_cursor = 0;
        } else if (key == GLFW_KEY_LEFT) {
            if (!shift && !super) {
                if (SELECTION_NOT_ACTIVE()) {
                    if (popup->cursor > 0) --popup->cursor;
                } else {
                    popup->cursor = left_cursor;
                }
                popup->selection_cursor = popup->cursor;
            } else if (shift && !super) {
                if (SELECTION_NOT_ACTIVE()) popup->selection_cursor = popup->cursor;
                if (popup->cursor > 0) --popup->cursor;
            } else if (super && !shift) {
                popup->selection_cursor = popup->cursor = 0;
            } else { ASSERT(shift && super);
                popup->selection_cursor = 0;
            }
        } else if (key == GLFW_KEY_RIGHT) {
            if (!shift && !super) {
                if (SELECTION_NOT_ACTIVE()) {
                    if (popup->cursor < len) ++popup->cursor;
                } else {
                    popup->cursor = MAX(popup->cursor, popup->selection_cursor);
                }
                popup->selection_cursor = popup->cursor;
            } else if (shift && !super) {
                if (SELECTION_NOT_ACTIVE()) popup->selection_cursor = popup->cursor;
                if (popup->cursor < len) ++popup->cursor;
            } else if (super && !shift) {
                popup->selection_cursor = popup->cursor = len;
            } else { ASSERT(shift && super);
                popup->selection_cursor = len;
            }
        } else if (key == GLFW_KEY_BACKSPACE) {
            // * * * *|* * * * 
            if (SELECTION_NOT_ACTIVE()) {
                if (popup->cursor > 0) {
                    memmove(&active_cell[popup->cursor - 1], &active_cell[popup->cursor], POPUP_CELL_LENGTH - popup->cursor);
                    active_cell[POPUP_CELL_LENGTH - 1] = '\0';
                    --popup->cursor;
                }
            } else {
                // * * * * * * * * * * * * * * * *
                // * * * * * * * - - - - - - - - -
                //    L                 R 

                // * * * * * * * * * * * * * * * *
                // * * * * * * * * * * * * - - - -
                //    L       R                   
                memmove(&active_cell[left_cursor], &active_cell[right_cursor], POPUP_CELL_LENGTH - right_cursor);
                memset(&active_cell[POPUP_CELL_LENGTH - (right_cursor - left_cursor)], 0, right_cursor - left_cursor);
                popup->cursor = left_cursor;
            }
            popup->selection_cursor = popup->cursor;
        } else if ((!hotkey_consumed_this_event) && event_is_consumable_by_current_popup(event)) {
            // TODO: strip char_equivalent into function

            bool32 key_is_alpha = ('A' <= key) && (key <= 'Z');

            char char_equivalent; {
                char_equivalent = (char) key;
                if (!shift && key_is_alpha) {
                    char_equivalent = 'a' + (char_equivalent - 'A');
                }
            }
            if (SELECTION_NOT_ACTIVE()) {
                if (popup->cursor < POPUP_CELL_LENGTH) {
                    memmove(&active_cell[popup->cursor + 1], &active_cell[popup->cursor], POPUP_CELL_LENGTH - popup->cursor - 1);
                    active_cell[popup->cursor++] = char_equivalent;
                }
            } else {
                memmove(&active_cell[left_cursor + 1], &active_cell[right_cursor], POPUP_CELL_LENGTH - right_cursor);
                memset(&active_cell[POPUP_CELL_LENGTH - (right_cursor - (left_cursor + 1))], 0, right_cursor - (left_cursor + 1));
                popup->cursor = left_cursor;
                active_cell[popup->cursor++] = char_equivalent;
            }
            popup->selection_cursor = popup->cursor;
        }

    }

    WRITE_ACTIVE_CELL_BUFFER_INTO_CORRESPONDING_VALUE(); // FORNOW: do every frame

    if (!HACK_DISABLE_POPUP_DRAWING) {
        for (uint32 d = 0; d < num_cells; ++d) { // gui_printf
            if (!name[d]) continue;
            static char buffer[512]; // FORNOW; ???

            // FORNOW gross;
            if (d == popup->index_of_active_cell) {
                sprintf(buffer, "`%s %s", name[d], popup->active_cell_buffer);
            } else {
                if (cell_type[d] == CELL_TYPE_REAL32) {
                    sprintf(buffer,  "%s %g", name[d], *((real32 *) value[d]));
                } else { ASSERT(cell_type[d] == CELL_TYPE_CSTRING); 
                    sprintf(buffer,  "%s %s", name[d], ((char *) value[d]));
                }
            }

            if (name[d]) {
                if (popup->index_of_active_cell != d) {
                    gui_printf(buffer);
                } else { // FORNOW: horrifying; needs at least a variant of stb_easy_font_width that takes an offset; should also do the 2 * for us
                    if (!SELECTION_NOT_ACTIVE()) {
                        real32 o = COW1._gui_x_curr + 2 * (stb_easy_font_width(name[d]) + stb_easy_font_width(" "));
                        char tmp[4096]; // FORNOW
                        strcpy(tmp, popup->active_cell_buffer);
                        tmp[MAX(popup->cursor, popup->selection_cursor)] = '\0';
                        real32 R = o + 2 * stb_easy_font_width(tmp) - 2.5f;
                        tmp[MIN(popup->cursor, popup->selection_cursor)] = '\0';
                        real32 L = o + 2 * stb_easy_font_width(tmp);
                        real32 y = COW1._gui_y_curr;
                        eso_begin(globals._gui_NDC_from_Screen, SOUP_QUADS);
                        eso_color(0.4f, 0.4f, 0.0f);
                        eso_vertex(L, y);
                        eso_vertex(R, y);
                        eso_vertex(R, y + 20);
                        eso_vertex(L, y + 20);
                        eso_end();
                    }
                    real32 x = COW1._gui_x_curr;
                    real32 y = COW1._gui_y_curr;
                    gui_printf(buffer);
                    if (((int) (_global_screen_state.popup_blinker_time * 5)) % 10 < 5) {
                        char tmp[4096]; // FORNOW
                        strcpy(tmp, popup->active_cell_buffer);
                        tmp[popup->cursor] = '\0';
                        x += 2 * (stb_easy_font_width(name[d]) + stb_easy_font_width(" ") + stb_easy_font_width(tmp)); // (FORNOW 2 *)
                        x -= 2.5;
                        // FORNOW: silly way of getting longer |
                        _text_draw((cow_real *) &globals._gui_NDC_from_Screen, "|", x, y - 5, 0.0, 1.0, 1.0, 0.0, 1.0, 0, 0.0, 0.0, true);
                        _text_draw((cow_real *) &globals._gui_NDC_from_Screen, "|", x, y + 5, 0.0, 1.0, 1.0, 0.0, 1.0, 0, 0.0, 0.0, true);
                    }
                }
            }
        }
    }

    // bool32 close_popup_and_execute; {
    //     close_popup_and_execute = ((_standard_event.type == USER_EVENT_TYPE_KEY_PRESS) && (_standard_event.key == GLFW_KEY_ENTER));
    // for (uint32 d = 0; d < num_cells; ++d) close_popup_and_execute &= (!IS_ZERO(*value[d])); // FORNOW
    // }

    // return close_popup_and_execute;
};
