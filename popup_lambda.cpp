// cool text animations when tabbing

auto popup_popup = [&] (
        char *_name0, real32 *_value0,
        char *_name1 = NULL, real32 *_value1 = NULL,
        char *_name2 = NULL, real32 *_value2 = NULL,
        char *_name3 = NULL, real32 *_value3 = NULL
        ) -> bool32 {

    ///

    PopupState *popup = &global_world_state.popup;

    ///

    char *name[]    = {  _name0,  _name1,  _name2,  _name3 };
    real32 *value[] = { _value0, _value1, _value2, _value3 };
    uint32 num_cells; {
        num_cells = 0;
        for (uint32 d = 0; d < ARRAY_LENGTH(name); ++d) if (name[d]) ++num_cells;
    }

    ASSERT(num_cells);

    ///

    popup_popup_actually_called_this_event = true;

    // LOADING UP ///////////////////////////////////////////////////

    if (popup->_active_popup_unique_ID__FORNOW_name0 != _name0) {
        popup->_active_popup_unique_ID__FORNOW_name0 = _name0;
        popup->index_of_active_cell = 0;
        for (uint32 d = 0; d < num_cells; ++d) {
            if (!name[d]) continue;
            memset(popup->cells[d], 0, POPUP_CELL_LENGTH);
            sprintf(popup->cells[d], "%g", *value[d]);
        }
        popup->cursor = strlen(popup->cells[popup->index_of_active_cell]);
        popup->selection_cursor = 0;
    }

    // KEY PRESS ////////////////////////////////////////////////////

    // TODO: port extrude to use this
    // TODO: 'X'
    // TODO: mouse click
    // TODO: mouse drag
    // TODO: paste from os clipboard
    // TODO: parsing math formulas

    ASSERT(popup->index_of_active_cell >= 0);
    ASSERT(popup->index_of_active_cell <= num_cells);
    ASSERT(popup->cursor >= 0);
    ASSERT(popup->cursor <= POPUP_CELL_LENGTH);
    ASSERT(popup->selection_cursor >= 0);
    ASSERT(popup->selection_cursor <= POPUP_CELL_LENGTH);

    auto SELECTION_NOT_ACTIVE = [&]() -> bool { return (popup->selection_cursor == popup->cursor); };

    if (_standard_event->type == USER_EVENT_TYPE_KEY_PRESS) {
        _global_screen_state.popup_blinker_time = 0.0; // FORNOW

        uint32 key = _standard_event->key;
        bool32 shift = _standard_event->shift;
        bool32 super = _standard_event->super;

        bool32 key_is_digit = ('0' <= key) && (key <= '9');
        bool32 key_is_numerical_punctuation = (key == '.') || (key == '-');


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
            memset(popup->cells[popup->index_of_active_cell], 0, POPUP_CELL_LENGTH);
            sprintf(popup->cells[popup->index_of_active_cell], "%g", *value[popup->index_of_active_cell]);
            popup->cursor = strlen(popup->cells[popup->index_of_active_cell]);
            popup->selection_cursor = 0;
        }

        char *active_cell = popup->cells[popup->index_of_active_cell];
        uint32 len = strlen(active_cell);
        uint32 left_cursor = MIN(popup->cursor, popup->selection_cursor);
        uint32 right_cursor = MAX(popup->cursor, popup->selection_cursor);

        popup_popup_ate_this_event = true; // default
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
                    if (popup->cursor < POPUP_CELL_LENGTH) ++popup->cursor;
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
        } else if (key_is_digit || key_is_numerical_punctuation) {
            if (SELECTION_NOT_ACTIVE()) {
                if (popup->cursor < POPUP_CELL_LENGTH) {
                    memmove(&active_cell[popup->cursor + 1], &active_cell[popup->cursor], POPUP_CELL_LENGTH - popup->cursor - 1);
                    active_cell[popup->cursor++] = key;
                }
            } else {
                memmove(&active_cell[left_cursor + 1], &active_cell[right_cursor], POPUP_CELL_LENGTH - right_cursor);
                memset(&active_cell[POPUP_CELL_LENGTH - (right_cursor - (left_cursor + 1))], 0, right_cursor - (left_cursor + 1));
                popup->cursor = left_cursor;
                active_cell[popup->cursor++] = key;
            }
            popup->selection_cursor = popup->cursor;
        } else {
            popup_popup_ate_this_event = false;
        }
    }

    for (uint32 d = 0; d < num_cells; ++d) if (popup->index_of_active_cell == d) *value[d] = strtof(popup->cells[d], NULL);
    if (!HACK_DONT_DRAW_IMGUI_UNDO_REDO_OTHERWISE_POPUPS_WEIRDNESS) {
        for (uint32 d = 0; d < num_cells; ++d) { // gui_printf
            if (!name[d]) continue;
            char buffer[256];
            // if (popup->index_of_active_cell == d) sprintf(buffer, "`%s %s (%g)", name[d], popup->cells[d], *value[d]);
            // else                  sprintf(buffer,  "%s (%s) %g", name[d], popup->cells[d], *value[d]);
            if (popup->index_of_active_cell == d) sprintf(buffer, "`%s %s", name[d], popup->cells[d]);
            else                  sprintf(buffer,  "%s %g", name[d], *value[d]);
            if (name[d]) {
                if (popup->index_of_active_cell != d) {
                    gui_printf(buffer);
                } else { // FORNOW: horrifying; needs at least a variant of stb_easy_font_width that takes an offset; should also do the 2 * for us
                    if (!SELECTION_NOT_ACTIVE()) {
                        uint32 o = COW1._gui_x_curr + 2 * (stb_easy_font_width(name[d]) + stb_easy_font_width(" "));
                        char tmp[4096]; // FORNOW
                        strcpy(tmp, popup->cells[d]);
                        tmp[MAX(popup->cursor, popup->selection_cursor)] = '\0';
                        uint32 R = o + 2 * stb_easy_font_width(tmp) - 2.5;
                        tmp[MIN(popup->cursor, popup->selection_cursor)] = '\0';
                        uint32 left_cursor = o + 2 * stb_easy_font_width(tmp);
                        uint32 y = COW1._gui_y_curr;
                        eso_begin(globals._gui_NDC_from_Screen, SOUP_QUADS);
                        eso_color(0.6f, 0.6f, 0.0f);
                        eso_vertex(left_cursor, y);
                        eso_vertex(R, y);
                        eso_vertex(R, y + 20);
                        eso_vertex(left_cursor, y + 20);
                        eso_end();
                    }
                    uint32 x = COW1._gui_x_curr;
                    uint32 y = COW1._gui_y_curr;
                    gui_printf(buffer);
                    if (((int) (_global_screen_state.popup_blinker_time * 5)) % 10 < 5) {
                        char tmp[4096]; // FORNOW
                        strcpy(tmp, popup->cells[d]);
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

    bool32 close_popup_and_execute; {
        close_popup_and_execute = ((_standard_event->type == USER_EVENT_TYPE_KEY_PRESS) && (_standard_event->key == GLFW_KEY_ENTER));
        for (uint32 d = 0; d < num_cells; ++d) close_popup_and_execute &= (!IS_ZERO(*value[d])); // FORNOW
    }

    return close_popup_and_execute;
};
