
void get_console_params(real32 *console_param_1, real32 *console_param_2) {
    char buffs[2][64] = {};
    uint32 buff_i = 0;
    uint32 i = 0;
    for (char *c = global_world_state.console.buffer; (*c) != '\0'; ++c) {
        if (*c == ',') {
            if (++buff_i == 2) break;
            i = 0;
            continue;
        }
        buffs[buff_i][i++] = (*c);
    }
    real32 sign = 1.0f;
    if ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
        sign = (!global_world_state.console.flip_flag) ? 1.0f : -1.0f;
    }
    *console_param_1 = sign * strtof(buffs[0], NULL);
    *console_param_2 = sign * strtof(buffs[1], NULL);
    if ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
        *console_param_2 *= -1;
    }
}

{
    char enter_message[256] = {};
    if ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) || (global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN)) {
        real32 p, p2;
        char glyph, glyph2;
        if ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
            p      =  global_world_state.popup.param0;
            p2     = -global_world_state.popup.param1;
            if (global_world_state.console.flip_flag) { // ??
                p *= -1;
                p2 *= -1;
            }
            if (IS_ZERO(p)) p = 0.0f; // FORNOW makes minus sign go away in hud (not a big deal)
            if (IS_ZERO(p2)) p2 = 0.0f; // FORNOW makes minus sign go away in hud (not a big deal)
            glyph  = (!global_world_state.console.flip_flag) ? '^' : 'v';
            glyph2 = (!global_world_state.console.flip_flag) ? 'v' : '^';
        } else {
            ASSERT(global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN);
            p      = global_world_state.popup.param0;
            p2     = global_world_state.popup.param1;
            glyph  = 'x';
            glyph2 = 'y';
        }
        sprintf(enter_message, "%c:%gmm %c:%gmm", glyph, p, glyph2, p2);
        if (((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) && IS_ZERO(global_world_state.popup.param1)) sprintf(enter_message, "%c:%gmm", glyph, p);
    } else if ((global_world_state.modes.enter_mode == ENTER_MODE_OPEN) || (global_world_state.modes.enter_mode == ENTER_MODE_SAVE)) {
        sprintf(enter_message, "%s%s", _global_screen_state.drop_path, global_world_state.console.buffer);
    }

}

