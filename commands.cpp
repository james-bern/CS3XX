#define MOD_SHIFT GLFW_MOD_SHIFT   // 0x01
#define MOD_CTRL  GLFW_MOD_CONTROL // 0x02
#define MOD_ALT   GLFW_MOD_ALT     // 0x04
#define BACKSPACE GLFW_KEY_BACKSPACE
#define DELETE GLFW_KEY_DELETE
#define ESCAPE GLFW_KEY_ESCAPE
#define TAB GLFW_KEY_TAB

// TODO: switch from UPPER_SNAKE_CASE to UpperCamelCase (there is gonna a fast vim method probably)

struct Command {
    uint key;
    u8 mods;

    String name;
};

#define COMMANDS_OUTER \
    COMMANDS_INNER(Center,      'C', 0b000); \
    COMMANDS_INNER(End,         'E', 0b000); \
    COMMANDS_INNER(Intersect,   'I', 0b000); \
    COMMANDS_INNER(Middle,      'M', 0b000); \
    COMMANDS_INNER(Perp,        'P', 0b000); \
    COMMANDS_INNER(Quad,        'Q', 0b000); \
    COMMANDS_INNER(Tangent,     'T', 0b000); \
    COMMANDS_INNER(XY,          'X', 0b000); \
    COMMANDS_INNER(Zero,        'Z', 0b000); \
    \
    COMMANDS_INNER(All,         'A', 0b000); \
    COMMANDS_INNER(Color,       'Q', 0b000); \
    COMMANDS_INNER(Connected,   'C', 0b000); \
    COMMANDS_INNER(Window,      'W', 0b000); \
    \
    COMMANDS_INNER(Axis,        'A', 0b001); \
    COMMANDS_INNER(Box,         'B', 0b000); \
    COMMANDS_INNER(CenteredBox, 'B', 0b001); \
    COMMANDS_INNER(Circle,      'C', 0b000); \
    COMMANDS_INNER(ClearDrawing,'N', 0b010); \
    COMMANDS_INNER(ClearMesh,   'N', 0b011); \
    COMMANDS_INNER(Color0,      '0', 0b000); \
    COMMANDS_INNER(Color1,      '1', 0b000); \
    COMMANDS_INNER(Color2,      '2', 0b000); \
    COMMANDS_INNER(Color3,      '3', 0b000); \
    COMMANDS_INNER(Color4,      '4', 0b000); \
    COMMANDS_INNER(Color5,      '5', 0b000); \
    COMMANDS_INNER(Color6,      '6', 0b000); \
    COMMANDS_INNER(Color7,      '7', 0b000); \
    COMMANDS_INNER(Color8,      '8', 0b000); \
    COMMANDS_INNER(Color9,      '9', 0b000); \
    COMMANDS_INNER(Copy,        'O', 0b000); \
    COMMANDS_INNER(Deselect,    'D', 0b000); \
    COMMANDS_INNER(DiamCircle,  'C', 0b010); \
    COMMANDS_INNER(Divide2,     'I', 0b000); \
    COMMANDS_INNER(ExtrudeAdd,  '[', 0b000); \
    COMMANDS_INNER(ExtrudeCut,  '[', 0b001); \
    COMMANDS_INNER(Fillet,      'F', 0b000); \
    COMMANDS_INNER(Line,        'L', 0b000); \
    COMMANDS_INNER(Measure,     'M', 0b001); \
    COMMANDS_INNER(Mirror2,     'M', 0b011); \
    COMMANDS_INNER(Move,        'M', 0b000); \
    COMMANDS_INNER(NudgePlane,  'N', 0b000); \
    COMMANDS_INNER(Offset,      'H', 0b000); \
    COMMANDS_INNER(OpenDXF,     'O', 0b010); \
    COMMANDS_INNER(OpenSTL,     'O', 0b011); \
    COMMANDS_INNER(Origin,      'Z', 0b001); \
    COMMANDS_INNER(Plane,       'Y', 0b000); \
    COMMANDS_INNER(Polygon,     'P', 0b000); \
    COMMANDS_INNER(RCopy,       'R', 0b001); \
    COMMANDS_INNER(Redo,        'U', 0b001); \
    COMMANDS_INNER(RevolveAdd,  ']', 0b000); \
    COMMANDS_INNER(RevolveCut,  ']', 0b001); \
    COMMANDS_INNER(Rotate,      'R', 0b000); \
    COMMANDS_INNER(SaveDXF,     'S', 0b010); \
    COMMANDS_INNER(SaveSTL,     'S', 0b011); \
    COMMANDS_INNER(Scale,       'S', 0b001); \
    COMMANDS_INNER(Select,      'S', 0b000); \
    COMMANDS_INNER(Undo,        'U', 0b000); \
    COMMANDS_INNER(XMirror,     'X', 0b001); \
    COMMANDS_INNER(YMirror,     'Y', 0b001); \
    COMMANDS_INNER(ZoomDrawing, 'X', 0b011); \
    COMMANDS_INNER(ZoomMesh,      0, 0b000); \
    \
    \
    \
    \
    \
    \
    \
    \
    \
    \
    \
    \
    \
    \
    \
    COMMANDS_INNER(UNDO_ALTERNATE, 'Z', 0b010);  \
    COMMANDS_INNER(TOGGLE_BUTTONS, TAB, 0b001);  \
    COMMANDS_INNER(TOGGLE_DRAWING_DETAILS, '.', 0b000);  \
    COMMANDS_INNER(TOGGLE_EVENT_STACK, 'K', 0b000);  \
    COMMANDS_INNER(TOGGLE_FEATURE_PLANE, ';', 0b000);  \
    COMMANDS_INNER(TOGGLE_GRID, 'G', 0b000);  \
    COMMANDS_INNER(TOGGLE_LIGHT_MODE, 'L', 0b011);  \
    COMMANDS_INNER(REDO_ALTERNATE, 'Y', 0b010);  \
    COMMANDS_INNER(REDO_ALTERNATE_ALTERNATE, 'Z', 0b011); \
    COMMANDS_INNER(PREVIOUS_HOT_KEY_2D, ' ', 0b000);  \
    COMMANDS_INNER(PREVIOUS_HOT_KEY_3D, ' ', 0b001);  \
    COMMANDS_INNER(PRINT_HISTORY, 'H', 0b010);  \
    COMMANDS_INNER(POWER_FILLET, 'F', 0b001);  \
    COMMANDS_INNER(HELP_MENU, '/', 0b010);  \
    COMMANDS_INNER(DIVIDE_NEAREST, 'X', 0b000);  \
    COMMANDS_INNER(DELETE_SELECTED, DELETE, 0b000 );  \
    COMMANDS_INNER(DELETE_SELECTED_ALTERNATE, BACKSPACE, 0b000 );  \
    COMMANDS_INNER(NEXT_POPUP_BAR, TAB, 0b000 );/* secretly actually supported but scary */ \
    COMMANDS_INNER(EXECUTE_COMMAND, '\n', 0b000 );/* should be glfw_key_enter */ \
    COMMANDS_INNER(EXIT_COMMAND, ESCAPE, 0b000 );/* TODO */

struct {
    #define COMMANDS_INNER(NAME, CHAR, CODE) \
    Command NAME = { CHAR, CODE };

    COMMANDS_OUTER;

    #undef COMMANDS_INNER
} commands;





#define CONFIG_OUTER \
    CONFIG_INNER(HIDE_GUI, false);  \
    CONFIG_INNER(usingInches, false);


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct {
    #define CONFIG_INNER(NAME, VALUE) \
    real NAME = VALUE

    CONFIG_OUTER;

    #undef CONFIG_INNER
} config;





real parse_config(String str) {
    if (0) {
    } else if (string_matches_prefix(str, "F")) {
        return 0;
    } else if (string_matches_prefix(str, "T")) {
        return 1;
    } else {
        return strtol(str.data, NULL, 0); 
    }
}

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

Command parse_command(String str) {
    char *start = str.data;
    Command command = {};
    while (str.data - start < str.length) {
        if (string_matches_prefix(str, "SHIFT+")) {
            command.mods |= MOD_SHIFT;
            str.data += 6;
        } else if (string_matches_prefix(str, "CTRL+")) { 
            command.mods |= MOD_CTRL;
            str.data += 5;
        } else if (string_matches_prefix(str, "ALT+")) {
            command.mods |= MOD_ALT;
            str.data += 4;
        } else {
            command.key = parse_key(STRING(str.data));
            break;
        }
    }
    return command;
}

Command COMMAND(uint command_name, unsigned char modifier) {
    return { command_name, modifier };
}

run_before_main {

    FILE *file = fopen("conversation.cfg", "r");
    if (!file) {
        messagef(omax.red, "Failed to open commands file");
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
            String command_string;
            {
                uint index_of_equals_sign; {
                    index_of_equals_sign = 0;
                    while ((line.data[index_of_equals_sign] != '=') && (line.data[index_of_equals_sign])) ++index_of_equals_sign; 
                }
                is_valid = (line.data[index_of_equals_sign] != '\0' || index_of_equals_sign < line.length);
                command_string = { &line.data[index_of_equals_sign + 1], line.length - index_of_equals_sign - 1 - 1 };
                command_name = { line.data, index_of_equals_sign };
                //DEBUGBREAK();
            }
            if (is_valid) {
                #define COMMANDS_INNER(NAME, _CHAR, _CODE) \
                else if (string_matches_prefix(command_name, STR(NAME))) commands.NAME = parse_command(command_string)

                if (0);
                COMMANDS_OUTER;

                #undef INNER

                
                #define CONFIG_INNER(NAME, _VALUE) \
                else if (string_matches_prefix(command_name, STR(NAME))) config.NAME = parse_config(command_string)

                if (0);
                CONFIG_OUTER;

                #undef CONFIG_OUTER
            }
        }
    }
    

    // config stuff
    {
        other.hide_toolbox = config.HIDE_GUI;
    }
};
#undef OUTER