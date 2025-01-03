// NOTE: FORNOW: FOCUS_THIEF is really the same thing as having a popup (except for two-click commands like Line where the popup doesn't show initially)
#define TWO_CLICK     (1 << 0)
#define FOCUS_THIEF   (1 << 1)
#define SNAPPER       (1 << 2)
#define SHIFT_15      (1 << 3)
#define NO_RECORD     (1 << 4)
#define HIDE_FEATURE_PLANE (1 << 6)
#define _UNSUED_FLAG7 (1 << 7)
#define _UNSUED_FLAG8 (1 << 8)
#define _UNSUED_FLAG9 (1 << 9)

//      alt/option      
//      |               
//      | control/super 
//      |/              
//      || shift        
//      ||/             
//      vvv             
//    0b000             


#define COMMANDS_OUTER \
    COMMANDS_INNER(None, 0, 0, None, 0, 0); \
    COMMANDS_INNER(Escape, GLFW_KEY_ESCAPE, 0b000, Both, 0, 0); \
    \
    COMMANDS_INNER(Undo,            'U', 0b000, Both, 0, 0 | NO_RECORD, 'Z', 0b010); \
    COMMANDS_INNER(Redo,            'U', 0b001, Both, 0, 0 | NO_RECORD, 'Y', 0b010, 'Z', 0b011 ); \
    \
    COMMANDS_INNER(ToggleConsole,   ',', 0b000, Both, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(ToggleDetails2D, '.', 0b000, Both, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(ToggleDetails3D,   0, 0b000, Both, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(ToggleGrid,          'G', 0b011, Both, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(ToggleGUI,       '.', 0b010, Both, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(ToggleFPS,       '/', 0b000, None, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(ToggleHistory,   'H', 0b011, None, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(ToggleLightMode2D, 0, 0b000, Both, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(ToggleLightMode3D, 0, 0b000, Both, 0, 0 | NO_RECORD);  \
    \
    \
    COMMANDS_INNER(Center,          'C', 0b000, Snap, 1, 0); \
    COMMANDS_INNER(End,             'E', 0b000, Snap, 1, 0); \
    COMMANDS_INNER(Intersect,       'I', 0b000, Snap, 1, 0); \
    COMMANDS_INNER(Middle,          'M', 0b000, Snap, 1, 0); \
    COMMANDS_INNER(Perp,            'P', 0b000, Snap, 1, 0); \
    COMMANDS_INNER(Quad,            'Q', 0b000, Snap, 1, 0); \
    COMMANDS_INNER(XY,              'X', 0b000, Snap, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(Zero,            'Z', 0b000, Snap, 0, 0); \
    COMMANDS_INNER(ClearSnap,         0, 0b000, Snap, 0, 0); \
    \
    \
    COMMANDS_INNER(Box,             'B', 0b000, Draw, 1, 0 | TWO_CLICK | SNAPPER); \
    COMMANDS_INNER(Circle,          'C', 0b000, Draw, 1, 0 | TWO_CLICK | SNAPPER); \
    COMMANDS_INNER(Line,            'L', 0b000, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    COMMANDS_INNER(Polygon,         'P', 0b000, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    \
    COMMANDS_INNER(Translate,       'M', 0b000, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15, 'T'); \
    COMMANDS_INNER(Rotate,          'R', 0b000, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    COMMANDS_INNER(Scale,           'S', 0b001, Draw, 1, 0 | TWO_CLICK | SNAPPER); \
    \
    COMMANDS_INNER(XMirror,         'X', 0b001, Draw, 1, 0 | SNAPPER); \
    COMMANDS_INNER(YMirror,         'Y', 0b001, Draw, 1, 0 | SNAPPER); \
    \
    COMMANDS_INNER(SetAxis,         'A', 0b001, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    COMMANDS_INNER(SetColor,        'Q', 0b000, Draw, 1, 0); \
    COMMANDS_INNER(LCopy,           'O', 0b000, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15, 'L', 0b001); \
    COMMANDS_INNER(Deselect,        'D', 0b000, Draw, 1, 0); \
    COMMANDS_INNER(Divide2,         'I', 0b000, Draw, 1, 0 | TWO_CLICK); \
    COMMANDS_INNER(Fillet,          'F', 0b000, Draw, 1, 0 | TWO_CLICK | FOCUS_THIEF); \
    COMMANDS_INNER(ElfHat,          'E', 0b000, Draw, 1, 0 | TWO_CLICK | FOCUS_THIEF); \
    COMMANDS_INNER(DogEar,          'G', 0b000, Draw, 1, 0 | TWO_CLICK | FOCUS_THIEF); \
    COMMANDS_INNER(Join2,           'J', 0b000, Draw, 1, 0 | TWO_CLICK); \
    COMMANDS_INNER(Measure,         'M', 0b001, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    COMMANDS_INNER(Drag,            'D', 0b001, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    COMMANDS_INNER(Offset,          'H', 0b000, Draw, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(OpenDXF,         'O', 0b010, Draw, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(SetOrigin,       'Z', 0b001, Draw, 1, 0 | SNAPPER); \
    COMMANDS_INNER(RCopy,           'R', 0b001, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    COMMANDS_INNER(SaveDXF,         'S', 0b010, Draw, 1, 0 | FOCUS_THIEF | NO_RECORD); \
    COMMANDS_INNER(Select,          'S', 0b000, Draw, 1, 0); \
    COMMANDS_INNER(Clear2D,    'N', 0b010, Draw, 0, 0); \
    COMMANDS_INNER(Zoom2D,       0, 0b000, Draw, 0, 0 | NO_RECORD); \
    \
    COMMANDS_INNER(Delete,               GLFW_KEY_DELETE, 0b000, Draw, 0, 0, GLFW_KEY_BACKSPACE, 0b000);  \
    COMMANDS_INNER(OverwriteDXF,                       0, 0b000, Draw, 0, 0); \
    COMMANDS_INNER(OverwriteSTL,                       0, 0b000, Mesh, 0, 0); \
    \
    COMMANDS_INNER(ExtrudeAdd,      '[', 0b000, Mesh, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(ExtrudeCut,      '[', 0b001, Mesh, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(NudgePlane,      'N', 0b000, Mesh, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(OpenSTL,         'O', 0b100, Mesh, 1, 0 | FOCUS_THIEF, 'O', 0b011); \
    COMMANDS_INNER(CyclePlane,      'Y', 0b000, Mesh, 0, 0); \
    COMMANDS_INNER(TogglePlane,     ';', 0b000, Mesh, 0, 0);  \
    COMMANDS_INNER(MirrorPlaneX,    'X', 0b100, Mesh, 0, 0);  \
    COMMANDS_INNER(MirrorPlaneY,    'Y', 0b100, Mesh, 0, 0);  \
    COMMANDS_INNER(RotatePlane,     'R', 0b100, Mesh, 1, 0 | FOCUS_THIEF);  \
    \
    COMMANDS_INNER(RevolveAdd,      ']', 0b000, Mesh, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(RevolveCut,      ']', 0b001, Mesh, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(SaveSTL,         'S', 0b100, Mesh, 1, 0 | FOCUS_THIEF | NO_RECORD, 'S', 0b011); \
    COMMANDS_INNER(Clear3D,         'N', 0b100, Mesh, 0, 0, 'N', 0b011); \
    COMMANDS_INNER(ZoomMesh,          0, 0b000, Mesh, 0, 0 | NO_RECORD); \
    COMMANDS_INNER(ZoomPlane,         0, 0b000, Mesh, 0, 0 | NO_RECORD); \
    \
    COMMANDS_INNER(Measure3D,       'M', 0b100, Mesh, 1, 0 | TWO_CLICK | SNAPPER | HIDE_FEATURE_PLANE); \
    COMMANDS_INNER(SetOrigin3D,     'Z', 0b100, Mesh, 1, 0 | SNAPPER | HIDE_FEATURE_PLANE); \
    \
    \
    COMMANDS_INNER(All,             'A', 0b000, Xsel, 0, 0); \
    COMMANDS_INNER(Connected,       'C', 0b000, Xsel, 1, 0); \
    COMMANDS_INNER(Window,          'W', 0b000, Xsel, 1, 0 | TWO_CLICK); \
    COMMANDS_INNER(ByColor,         'Q', 0b000, Xsel, 1, 0); \
    \
    \
    \
    \
    COMMANDS_INNER(OfSelection,        'S', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color0,          '0', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color1,          '1', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color2,          '2', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color3,          '3', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color4,          '4', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color5,          '5', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color6,          '6', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color7,          '7', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color8,          '8', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color9,          '9', 0b000, Colo, 1, 0); \
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
    COMMANDS_INNER(ConfirmClose,                     0,   0b000, Draw, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(TOGGLE_BUTTONS,          GLFW_KEY_TAB, 0b001, None, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(TOGGLE_LIGHT_MODE,                'L', 0b011, None, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(PREVIOUS_HOT_KEY_2D,              ' ', 0b000, None, 0, 0);  \
    COMMANDS_INNER(PREVIOUS_HOT_KEY_3D,              ' ', 0b001, None, 0, 0);  \
    COMMANDS_INNER(PowerFillet,                      'F', 0b001, None, 0, 0);  \
    COMMANDS_INNER(DivideNearest,                    'X', 0b000, None, 0, 0);  \
    // COMMANDS_INNER(NEXT_POPUP_BAR,                  TAB,  0b000, None, 0, 0);/* secretly supported but scary */ \




    // TODO: tag each command with an ID?--have them part of an enum?
    //       (checking equality based on name.data feels dangerous)


struct {
    #define COMMANDS_INNER(NAME, CHAR, CODE, GROUP, IS_MODE, FLAGS, ...) \
    Command NAME = { ToolboxGroup::GROUP, IS_MODE, FLAGS, STRING(STR(NAME)), { CHAR, CODE, __VA_ARGS__ } };

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
        return 0.0f;
    } else if (string_matches_prefix(str, "T")) {
        return 1.0f;
    } else {
        return real(strtol(str.data, NULL, 0)); 
    }
}

uint parse_key(String str) {
    if (0) {
    } else if (string_matches_prefix(str, "ESCAPE")) {
        return GLFW_KEY_ESCAPE;
    } else if (string_matches_prefix(str, "TAB")) {
        return GLFW_KEY_TAB;
    } else if (string_matches_prefix(str, "DELETE")) {
        return GLFW_KEY_DELETE;
    } else if (string_matches_prefix(str, "BACKSPACE")) {
        return GLFW_KEY_BACKSPACE;
    } else {
        return (uint)str.data[0];
    }
}

#if 0
Command parse_command(String str) {
    char *start = str.data;
    Command command = {};
    while (str.data - start < str.length) {
        if (string_matches_prefix(str, "SHIFT+")) {
            command.shortcut.mods |= MOD_SHIFT;
            str.data += 6;
        } else if (string_matches_prefix(str, "CTRL+")) { 
            command.shortcut.mods |= MOD_CTRL;
            str.data += 5;
        } else if (string_matches_prefix(str, "ALT+")) {
            command.shortcut.mods |= MOD_ALT;
            str.data += 4;
        } else {
            command.shortcut.key = parse_key(STRING(str.data));
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
        // messagef(pallete.red, "Failed to open commands file");
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
                #define COMMANDS_INNER(NAME, _CHAR, _CODE, _GROUP, _FLAGS, _IS_MODE) \
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
        // other.hide_toolbox = config.HIDE_GUI;
    }
};
#undef OUTER
#endif
