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
            if (*value[d] != 0) sprintf(popup->cells[d], "%g", *value[d]);
        }
        popup->cursor = 0;
        popup->selection_left = 0;
        popup->selection_right = strlen(popup->cells[popup->index_of_active_cell]);
    }

    // KEY PRESS ////////////////////////////////////////////////////

    // TODO: paste, copy, etc.

    ASSERT(popup->index_of_active_cell >= 0);
    ASSERT(popup->index_of_active_cell <= num_cells);
    ASSERT(popup->selection_left <= popup->selection_right);
    ASSERT(popup->selection_right <= POPUP_CELL_LENGTH);

    auto SELECTION_CLEAR = [&]() -> void { popup->selection_left = popup->selection_right = 0; };
    auto SELECTION_NOT_ACTIVE = [&]() -> bool { return (popup->selection_left == popup->selection_right); };

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
            popup->cursor = 0;
            memset(popup->cells[popup->index_of_active_cell], 0, POPUP_CELL_LENGTH);
            sprintf(popup->cells[popup->index_of_active_cell], "%g", *value[popup->index_of_active_cell]);
            { // active_cell_selection_*
                popup->selection_left = 0;
                popup->selection_right = strlen(popup->cells[popup->index_of_active_cell]); // FORNOW
            }
        }

        char *active_cell = popup->cells[popup->index_of_active_cell];
        uint32 len = strlen(active_cell);


        // TODO: consider selection_left_right -> selection cursor (the other)


        popup_popup_ate_this_event = true; // default
        if (_tab_hack_so_aliases_not_introduced_too_far_up) {
        } else if (key == GLFW_KEY_LEFT) {
            if (!shift && !super) {
                if (SELECTION_NOT_ACTIVE()) {
                    if (popup->cursor > 0) --popup->cursor;
                } else {
                    popup->cursor = popup->selection_left;
                    SELECTION_CLEAR();
                }
            } else if (shift && !super) {
                if (SELECTION_NOT_ACTIVE()) {
                    popup->selection_right = popup->cursor;
                    popup->selection_left = (popup->cursor > 0) ? popup->cursor - 1 : 0;
                    popup->cursor = popup->selection_left;
                } else {
                    bool32 move_left = (popup->cursor == popup->selection_left);
                    if (popup->cursor > 0) --popup->cursor;
                    if (move_left) popup->selection_left = popup->cursor; else popup->selection_right = popup->cursor;
                }
            } else if (super && !shift) {
                popup->cursor = 0;
                SELECTION_CLEAR();
            } else { ASSERT(shift && super);
            }
        } else if (key == GLFW_KEY_RIGHT) {
            if (!shift && !super) {
                if (SELECTION_NOT_ACTIVE()) {
                    if (popup->cursor < POPUP_CELL_LENGTH) ++popup->cursor;
                    popup->cursor = MIN(popup->cursor, strlen(active_cell)); // FORNOW
                } else {
                    popup->cursor = popup->selection_right;
                    SELECTION_CLEAR();
                }
            } else if (shift && !super) {
                if (SELECTION_NOT_ACTIVE()) {
                    popup->selection_left = popup->cursor;
                    popup->selection_right = (popup->cursor < len) ? popup->cursor + 1 : len;
                    popup->cursor = popup->selection_right;
                } else {
                    bool32 move_left = (popup->cursor == popup->selection_left);
                    if (popup->cursor < len) ++popup->cursor;
                    if (move_left) popup->selection_left = popup->cursor; else popup->selection_right = popup->cursor;
                }
            } else if (super && !shift) {
                popup->cursor = len;
                SELECTION_CLEAR();
            } else { ASSERT(shift && super);
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
                popup->cursor = popup->selection_left;
                memmove(&active_cell[popup->selection_left], &active_cell[popup->selection_right], POPUP_CELL_LENGTH - popup->selection_right);
                memset(&active_cell[POPUP_CELL_LENGTH - (popup->selection_right - popup->selection_left)], 0, popup->selection_right - popup->selection_left);
                SELECTION_CLEAR();
            }
        } else if (key_is_digit || key_is_numerical_punctuation) {
            if (SELECTION_NOT_ACTIVE()) {
                if (popup->cursor < POPUP_CELL_LENGTH) {
                    memmove(&active_cell[popup->cursor + 1], &active_cell[popup->cursor], POPUP_CELL_LENGTH - popup->cursor - 1);
                    active_cell[popup->cursor++] = key;
                }
            } else {
                popup->cursor = popup->selection_left;
                active_cell[popup->cursor++] = key;
                memmove(&active_cell[popup->selection_left + 1], &active_cell[popup->selection_right], POPUP_CELL_LENGTH - popup->selection_right);
                memset(&active_cell[POPUP_CELL_LENGTH - (popup->selection_right - (popup->selection_left + 1))], 0, popup->selection_right - (popup->selection_left + 1));
                SELECTION_CLEAR();
            }
        } else {
            popup_popup_ate_this_event = false;
        }
    }

#define _CELL_ACTIVE_(d) (popup->index_of_active_cell == d)

    for (uint32 d = 0; d < num_cells; ++d) if (_CELL_ACTIVE_(d)) *value[d] = strtof(popup->cells[d], NULL);
    if (!HACK_DONT_DRAW_IMGUI_UNDO_REDO_OTHERWISE_POPUPS_WEIRDNESS) {
        for (uint32 d = 0; d < num_cells; ++d) { // gui_printf
            if (!name[d]) continue;
            char buffer[256];
            // if (_CELL_ACTIVE_(d)) sprintf(buffer, "`%s %s (%g)", name[d], popup->cells[d], *value[d]);
            // else                  sprintf(buffer,  "%s (%s) %g", name[d], popup->cells[d], *value[d]);
            if (_CELL_ACTIVE_(d)) sprintf(buffer, "`%s %s", name[d], popup->cells[d]);
            else                  sprintf(buffer,  "%s %g", name[d], *value[d]);
            if (name[d]) {
                if (!_CELL_ACTIVE_(d)) {
                    gui_printf(buffer);
                } else { // FORNOW: horrifying; needs at least a variant of stb_easy_font_width that takes an offset; should also do the 2 * for us
                    if (!SELECTION_NOT_ACTIVE()) {
                        uint32 o = COW1._gui_x_curr + 2 * (stb_easy_font_width(name[d]) + stb_easy_font_width(" "));
                        char tmp[4096]; // FORNOW
                        strcpy(tmp, popup->cells[d]);
                        tmp[popup->selection_right] = '\0';
                        uint32 R = o + 2 * stb_easy_font_width(tmp) - 2.5;
                        tmp[popup->selection_left] = '\0';
                        uint32 L = o + 2 * stb_easy_font_width(tmp);
                        uint32 y = COW1._gui_y_curr;
                        eso_begin(globals._gui_NDC_from_Screen, SOUP_QUADS);
                        eso_color(0.6f, 0.6f, 0.0f);
                        eso_vertex(L, y);
                        eso_vertex(R, y);
                        eso_vertex(R, y + 20);
                        eso_vertex(L, y + 20);
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
