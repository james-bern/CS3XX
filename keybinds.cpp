#define MOD_SHIFT GLFW_MOD_SHIFT   // 0x01
#define MOD_CTRL  GLFW_MOD_CONTROL // 0x02
#define MOD_ALT   GLFW_MOD_ALT     // 0x04
#define BACKSPACE GLFW_KEY_BACKSPACE
#define DELETE GLFW_KEY_DELETE
#define ESCAPE GLFW_KEY_ESCAPE
#define TAB GLFW_KEY_TAB

// TODO: switch from UPPER_SNAKE_CASE to UpperCamelCase (there is gonna a fast vim method probably)

#define OUTER \
    /*//////////////////////////////*/ \
    /*///////  SNAP COMMANDS ///////*/ \
    /*//////////////////////////////*/ \
    \
    INNER(CENTER, 'C', 0b000); \
    INNER(END, 'E', 0b000);  \
    INNER(MIDDLE, 'M', 0b000);  \
    INNER(PERPENDICULAR, 'P', 0b000);  \
    INNER(QUAD, 'Q', 0b000);  \
    INNER(XY, 'X', 0b000);  \
    INNER(ZERO, 'Z', 0b000);  \
    \
    \
    /*//////////////////////////////*/ \
    /*//////  SELECT COMMANDS //////*/ \
    /*//////////////////////////////*/ \
    \
    INNER(COLOR, 'Q', 0b000);  \
    INNER(SELECT_ALL, 'A', 0b000);  \
    INNER(SELECT_CONNECTED, 'C', 0b000);  \
    INNER(SELECT_WINDOW, 'W', 0b000);  \
    \
    \
    /*//////////////////////////////*/ \
    /*//////  OTHER COMMANDS ///////*/ \
    /*//////////////////////////////*/ \
    \
    INNER(AXIS, 'A', 0b001);  \
    INNER(BOX, 'B', 0b000);  \
    INNER(CENTERED_BOX, 'B', 0b001);  \
    INNER(CHANGE_ORIGIN, 'Z', 0b001);  \
    INNER(CIRCLE, 'C', 0b000);  \
    INNER(CLEAR_DRAWING, 'N', 0b010);  \
    INNER(CLEAR_MESH, 'N', 0b011);  \
    INNER(CYCLE_FEATURE_PLANE, 'Y', 0b000);  \
    INNER(DELETE_SELECTED, DELETE, 0b000 );  \
    INNER(DELETE_SELECTED_ALTERNATE, BACKSPACE, 0b000 );  \
    INNER(DESELECT, 'D', 0b000);  \
    INNER(DIVIDE_NEAREST, 'D', 0b001);  \
    INNER(DRAWING_FRAME, 'X', 0b011);  \
    INNER(EXTRUDE_ADD, '[', 0b000);  \
    INNER(EXTRUDE_CUT, '[', 0b001);  \
    INNER(FILLET, 'F', 0b000);  \
    INNER(HELP_MENU, '/', 0b010);  \
    INNER(LINE, 'L', 0b000);  \
    INNER(LINEAR_COPY, 'O', 0b000);  \
    INNER(MEASURE, 'M', 0b001);  \
    INNER(MIRROR_LINE, 'M', 0b011);  \
    INNER(MIRROR_X, 'X', 0b001);  \
    INNER(MIRROR_Y, 'Y', 0b001);  \
    INNER(MOVE, 'M', 0b000);  \
    INNER(NUDGE_PLANE, 'N', 0b000);  \
    INNER(OFFSET, 'H', 0b000);  \
    INNER(OPEN_DRAWING, 'O', 0b010);  \
    INNER(OPEN_MESH, 'O', 0b011);  \
    INNER(POLYGON, 'P', 0b000);  \
    INNER(POWER_FILLET, 'F', 0b001);  \
    INNER(PREVIOUS_HOT_KEY_2D, ' ', 0b000);  \
    INNER(PREVIOUS_HOT_KEY_3D, ' ', 0b001);  \
    INNER(PRINT_HISTORY, 'H', 0b010);  \
    INNER(QUALITY_0, '0', 0b00); \
    INNER(QUALITY_1, '1', 0b00); \
    INNER(QUALITY_2, '2', 0b00); \
    INNER(QUALITY_3, '3', 0b00); \
    INNER(QUALITY_4, '4', 0b00); \
    INNER(QUALITY_5, '5', 0b00); \
    INNER(QUALITY_6, '6', 0b00); \
    INNER(QUALITY_7, '7', 0b00); \
    INNER(QUALITY_8, '8', 0b00); \
    INNER(QUALITY_9, '9', 0b00); \
    INNER(REDO, 'U', 0b001);  \
    INNER(REDO_ALTERNATE, 'Y', 0b010);  \
    /* INNER(REDO_ALTERNATE_ALTERNATE, 'Z', 0b011);  */ \
    INNER(RESIZE, 'S', 0b001);  \
    INNER(REVOLVE_ADD, ']', 0b000);  \
    INNER(REVOLVE_CUT, ']', 0b001);  \
    INNER(ROTATE, 'R', 0b000);  \
    INNER(ROTATE_COPY, 'R', 0b001);  \
    INNER(SAVE_DRAWING, 'S', 0b010);  \
    INNER(SAVE_MESH, 'S', 0b011);  \
    INNER(SELECT, 'S', 0b000);  \
    INNER(TOGGLE_BUTTONS, TAB, 0b001);  \
    INNER(TOGGLE_DRAWING_DETAILS, '.', 0b000);  \
    INNER(TOGGLE_EVENT_STACK, 'K', 0b000);  \
    INNER(TOGGLE_FEATURE_PLANE, ';', 0b000);  \
    INNER(TOGGLE_GRID, 'G', 0b000);  \
    INNER(TOGGLE_LIGHT_MODE, 'L', 0b011);  \
    INNER(TWO_CLICK_DIVIDE, 'I', 0b000);  \
    INNER(TWO_EDGE_CIRCLE, 'C', 0b001);  \
    INNER(UNDO, 'U', 0b000);  \
    INNER(UNDO_ALTERNATE, 'Z', 0b010);  \
    INNER(ZOOM_3D_CAMERA, '\'', 0b000);  \
    \
    /*//////////////////////////////*/ \
    /*/////  NOT SUPPORTED  ////////*/ \
    /*//////////////////////////////*/ \
    \
    INNER(NEXT_POPUP_BAR, TAB, 0b000 );/* secretly actually supported but scary */ \
    INNER(EXECUTE_COMMAND, '\n', 0b000 );/* should be glfw_key_enter */ \
    INNER(EXIT_COMMAND, ESCAPE, 0b000 );/* TODO */


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct Keybind {
    uint key;
    unsigned char mods;
};

struct Keybinds {
    #define INNER(NAME, CHAR, CODE) \
    Keybind NAME = { CHAR, CODE };

    OUTER;

    #undef INNER
};

Keybinds keybinds;

uint parse_key(String str) {
    if (0) {
    } else if (string_matches_prefix(str, "ESCAPE")) {
        return ESCAPE;
    } else if (string_matches_prefix(str, "TAB")) {
        return TAB;
    } else if (string_matches_prefix(str, "DELETE")) {
        return DELETE;
    } else if (string_matches_prefix(str, "BACKSPACE")) {
        return BACKSPACE;
    } else {
        return (uint)str.data[0];
    }
}

Keybind parse_keybind(String str) {
    char *start = str.data;
    Keybind keybind = {};
    while (str.data - start < str.length) {
        if (string_matches_prefix(str, "SHIFT+")) {
            keybind.mods |= MOD_SHIFT;
            str.data += 6;
        } else if (string_matches_prefix(str, "CTRL+")) { 
            keybind.mods |= MOD_CTRL;
            str.data += 5;
        } else if (string_matches_prefix(str, "ALT+")) {
            keybind.mods |= MOD_ALT;
            str.data += 4;
        } else {
            keybind.key = parse_key(STRING(str.data));
            break;
        }
    }
    return keybind;
}

Keybind KEYBIND(uint command_name, unsigned char modifier) {
    return { command_name, modifier };
}

run_before_main {

    FILE *file = fopen("conversation.cfg", "r");
    if (!file) {
        messagef(omax.red, "Failed to open keybinds file");
        return;
    }
    defer { fclose(file); };

    #define LINE_MAX_LENGTH 256
    static _STRING_CALLOC(line, LINE_MAX_LENGTH);
    while (FGETS(&line, LINE_MAX_LENGTH, file)) {
        if (line.length == 0) {
        } else if (line.data[0] == '#') {
        } else {
            bool is_valid;
            String command_name;
            String keybind_string;
            {
                uint index_of_equals_sign; {
                    index_of_equals_sign = 0;
                    while ((line.data[index_of_equals_sign] != '=') && (line.data[index_of_equals_sign])) ++index_of_equals_sign; 
                }
                is_valid = (line.data[index_of_equals_sign] != '\0' || index_of_equals_sign < line.length);
                keybind_string = { &line.data[index_of_equals_sign + 1], line.length - index_of_equals_sign - 1 - 1 };
                command_name = { line.data, index_of_equals_sign };
                //DEBUGBREAK();
            }
            if (is_valid) {
                if (string_matches_prefix(command_name, "LINE")) keybinds.LINE = parse_keybind(keybind_string);
                #define INNER(NAME, _CHAR, _CODE) \
                else if (string_matches_prefix(command_name, STR(NAME))) keybinds.NAME = parse_keybind(keybind_string)

                if (0);
                OUTER;

                #undef INNER
            }
        }
    }
};
#undef OUTER
