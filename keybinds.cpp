#define MOD_SHIFT 0x02
#define MOD_CTRL  0x01
#define MOD_ALT   0x04
#define BACKSPACE GLFW_KEY_BACKSPACE
#define DELETE GLFW_KEY_DELETE
#define ESCAPE GLFW_KEY_ESCAPE
#define TAB GLFW_KEY_TAB

struct Keybind {
    int key;
    unsigned char modifiers;    // 00000    0    0    0
};                              //         ALT SHIFT CTRL

struct Keybinds {
    Keybind PREVIOUS_HOT_KEY_2D;
    Keybind PREVIOUS_HOT_KEY_3D;
    Keybind DELETE_SELECTED; // TODO
    Keybind DELETE_SELECTED_ALTERNATE; //TODO
    Keybind EXIT_COMMAND; // TODO
    Keybind TOGGLE_LIGHT_MODE;
    Keybind TOGGLE_BUTTONS;
    Keybind NEXT_POPUP_BAR;
    Keybind EXECUTE_COMMAND;
    Keybind EXTRUDE_ADD;
    Keybind EXTRUDE_CUT;
    Keybind REVOLVE_ADD;
    Keybind REVOLVE_CUT;
    Keybind TOGGLE_DRAWING_DETAILS;
    Keybind TOGGLE_FEATURE_PLANE;
    Keybind ZOOM_3D_CAMERA;
    Keybind HELP_MENU;
    Keybind AXIS;
    Keybind BOX;
    Keybind CIRCLE;
    Keybind CENTER;
    Keybind CONNECTED;
    Keybind TWO_EDGE_CIRCLE;
    Keybind DESELECT;
    Keybind DIVIDE_NEAREST;
    Keybind END;
    Keybind FILLET;
    Keybind POWER_FILLET;
    Keybind TOGGLE_GRID;
    Keybind PRINT_HISTORY;
    Keybind TWO_CLICK_DIVIDE;
    Keybind TOGGLE_EVENT_STACK;
    Keybind LINE;
    Keybind MOVE;
    Keybind MIDDLE;
    Keybind MEASURE;
    Keybind NUDGE_PLANE;
    Keybind CLEAR_DRAWING;
    Keybind CLEAR_MESH;
    Keybind LINEAR_COPY;
    Keybind LOAD_FILE;
    Keybind POLYGON;
    Keybind PERPENDICULAR;
    Keybind COLOR;
    Keybind QUAD;
    Keybind ROTATE;
    Keybind ROTATE_COPY;
    Keybind SELECT;
    Keybind RESIZE;
    Keybind SAVE;
    Keybind UNDO;
    Keybind UNDO_ALTERNATE;
    Keybind REDO;
    Keybind REDO_ALTERNATE;
    Keybind WINDOW_SELECT;
    Keybind XY;
    Keybind DRAWING_FRAME;
    Keybind MIRROR_X;
    Keybind MIRROR_Y;
    Keybind MIRROR_LINE;
    Keybind CYCLE_FEATURE_PLANE;
    Keybind ORIGIN;
    Keybind CHANGE_ORIGIN;
    Keybind SELECT_ALL;
};

Keybinds keybinds;

Keybinds default_values() {
    Keybinds default_keybinds;

    default_keybinds.PREVIOUS_HOT_KEY_2D = { ' ', 0b000 };
    default_keybinds.PREVIOUS_HOT_KEY_3D = { ' ', 0b010 };
    default_keybinds.DELETE_SELECTED = { DELETE, 0b000 }; // TODO
    default_keybinds.DELETE_SELECTED_ALTERNATE = { BACKSPACE, 0b000 }; //TODO
    default_keybinds.EXIT_COMMAND = { ESCAPE, 0b000 }; // TODO
    default_keybinds.TOGGLE_LIGHT_MODE = { TAB, 0b000 };
    default_keybinds.TOGGLE_BUTTONS = { TAB, 0b010 };
    default_keybinds.NEXT_POPUP_BAR = { TAB, 0b000 };
    default_keybinds.EXECUTE_COMMAND = { '\n', 0b000 };
    default_keybinds.EXTRUDE_ADD = { '[', 0b000 };
    default_keybinds.EXTRUDE_CUT = { '[', 0b010 };
    default_keybinds.REVOLVE_ADD = { ']', 0b000 };
    default_keybinds.REVOLVE_CUT = { ']', 0b010 };
    default_keybinds.TOGGLE_DRAWING_DETAILS = { '.', 0b000 };
    default_keybinds.TOGGLE_FEATURE_PLANE = { ';', 0b000 };
    default_keybinds.ZOOM_3D_CAMERA = { '\'', 0b000 };
    default_keybinds.HELP_MENU = { '/', 0b010 };
    default_keybinds.AXIS = { 'A', 0b000 };
    default_keybinds.BOX = { 'B', 0b000 };
    default_keybinds.CIRCLE = { 'C', 0b000 };
    default_keybinds.CENTER = { 'C', 0b000 };
    default_keybinds.CONNECTED = { 'C', 0b000 };
    default_keybinds.TWO_EDGE_CIRCLE = { 'C', 0b010 };
    default_keybinds.DESELECT = { 'D', 0b000 };
    default_keybinds.DIVIDE_NEAREST = { 'D', 0b010 };
    default_keybinds.END = { 'E', 0b000 };
    default_keybinds.FILLET = { 'F', 0b000 };
    default_keybinds.POWER_FILLET = { 'F', 0b010 };
    default_keybinds.TOGGLE_GRID = { 'G', 0b000 };
    default_keybinds.PRINT_HISTORY = { 'H', 0b000 };
    default_keybinds.TWO_CLICK_DIVIDE = { 'I', 0b000 };
    default_keybinds.TOGGLE_EVENT_STACK = { 'K', 0b000 };
    default_keybinds.LINE = { 'L', 0b000 };
    default_keybinds.MOVE = { 'M', 0b000 };
    default_keybinds.MIDDLE = { 'M', 0b000 };
    default_keybinds.MEASURE = { 'M', 0b010 };
    default_keybinds.NUDGE_PLANE = { 'N', 0b000 };
    default_keybinds.CLEAR_DRAWING = { 'N', 0b001 };
    default_keybinds.CLEAR_MESH = { 'N', 0b011 };
    default_keybinds.LINEAR_COPY = { 'O', 0b000 };
    default_keybinds.LOAD_FILE = { 'O', 0b001 };
    default_keybinds.POLYGON = { 'P', 0b000 };
    default_keybinds.PERPENDICULAR = { 'P', 0b000 };
    default_keybinds.COLOR = { 'Q', 0b000 };
    default_keybinds.QUAD = { 'Q', 0b000 };
    default_keybinds.ROTATE = { 'R', 0b000 };
    default_keybinds.ROTATE_COPY = { 'R', 0b010 };
    default_keybinds.SELECT = { 'S', 0b000 };
    default_keybinds.RESIZE = { 'S', 0b010 };
    default_keybinds.SAVE = { 'S', 0b001 };
    default_keybinds.UNDO = { 'U', 0b000 };
    default_keybinds.UNDO_ALTERNATE = { 'Z', 0b001 };
    default_keybinds.REDO = { 'U', 0b010 };
    default_keybinds.REDO_ALTERNATE = { 'Y', 0b011 };
    default_keybinds.WINDOW_SELECT = { 'W', 0b000 };
    default_keybinds.XY = { 'X', 0b000 };
    default_keybinds.DRAWING_FRAME = { 'X', 0b011 };
    default_keybinds.MIRROR_X = { 'X', 0b010 };
    default_keybinds.MIRROR_Y = { 'Y', 0b010 };
    default_keybinds.MIRROR_LINE = { 'M', 0b011 };
    default_keybinds.CYCLE_FEATURE_PLANE = { 'Y', 0b000 };
    default_keybinds.ORIGIN = { 'Z', 0b000 };
    default_keybinds.CHANGE_ORIGIN = { 'Z', 0b010 };
    default_keybinds.SELECT_ALL = { 'A', 0b000 };

    return default_keybinds;
}

Keybind parse_keybind(const char *str) {
    Keybind keybind = {};
    while (*str) {
        if (strncmp(str, "shift+", 6) == 0 || strncmp(str, "SHIFT+", 6) == 0) {
            keybind.modifiers |= MOD_SHIFT;
            str += 6;
        } else if (strncmp(str, "ctrl+", 5) == 0 || strncmp(str, "CTRL+", 5) == 0) { 
            keybind.modifiers |= MOD_CTRL;
            str += 5;
        } else if (strncmp(str, "alt+", 4) == 0 || strncmp(str, "ALT+", 4) == 0) {
            keybind.modifiers |= MOD_ALT;
            str += 4;
        } else {
            keybind.key = *str;
            break;
        }
    }
    return keybind;
}

Keybind KEYBIND(int key, unsigned char modifier) {
    return { key, modifier };
}

Keybinds init_keybinds() {
    Keybinds init_keybinds = default_values();
    FILE *file = fopen("conversation.cfg", "r");
    if (!file) {
        messagef(omax.red, "Failed to open keybinds file");
        return init_keybinds;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char key[32];
        char value[32];
        if (sscanf(line, "%31[^=]=%31s", key, value) == 2) {
            if (strcmp(key, "PREVIOUS_HOT_KEY_2D") == 0) init_keybinds.PREVIOUS_HOT_KEY_2D = parse_keybind(value);
            else if (strcmp(key, "PREVIOUS_HOT_KEY_3D") == 0) init_keybinds.PREVIOUS_HOT_KEY_3D = parse_keybind(value);
            else if (strcmp(key, "DELETE_SELECTED") == 0) init_keybinds.DELETE_SELECTED = parse_keybind(value); // TODO
            else if (strcmp(key, "DELETE_SELECTED_ALTERNATE") == 0) init_keybinds.DELETE_SELECTED_ALTERNATE = parse_keybind(value); //TODO
            else if (strcmp(key, "EXIT_COMMAND") == 0) init_keybinds.EXIT_COMMAND = parse_keybind(value); // TODO
            else if (strcmp(key, "TOGGLE_LIGHT_MODE") == 0) init_keybinds.TOGGLE_LIGHT_MODE = parse_keybind(value);
            else if (strcmp(key, "TOGGLE_BUTTONS") == 0) init_keybinds.TOGGLE_BUTTONS = parse_keybind(value);
            else if (strcmp(key, "NEXT_POPUP_BAR") == 0) init_keybinds.NEXT_POPUP_BAR = parse_keybind(value);
            else if (strcmp(key, "EXECUTE_COMMAND") == 0) init_keybinds.EXECUTE_COMMAND= parse_keybind(value);
            else if (strcmp(key, "EXTRUDE_ADD") == 0) init_keybinds.EXTRUDE_ADD = parse_keybind(value);
            else if (strcmp(key, "EXTRUDE_CUT") == 0) init_keybinds.EXTRUDE_CUT = parse_keybind(value);
            else if (strcmp(key, "REVOLVE_ADD") == 0) init_keybinds.REVOLVE_ADD = parse_keybind(value);
            else if (strcmp(key, "REVOLVE_CUT") == 0) init_keybinds.REVOLVE_CUT = parse_keybind(value);
            else if (strcmp(key, "TOGGLE_DRAWING_DETAILS") == 0) init_keybinds.TOGGLE_DRAWING_DETAILS = parse_keybind(value);
            else if (strcmp(key, "TOGGLE_FEATURE_PLANE") == 0) init_keybinds.TOGGLE_FEATURE_PLANE = parse_keybind(value);
            else if (strcmp(key, "ZOOM_3D_CAMERA") == 0) init_keybinds.ZOOM_3D_CAMERA = parse_keybind(value);
            else if (strcmp(key, "HELP_MENU") == 0) init_keybinds.HELP_MENU = parse_keybind(value);
            else if (strcmp(key, "AXIS") == 0) init_keybinds.AXIS = parse_keybind(value);
            else if (strcmp(key, "BOX") == 0) init_keybinds.BOX = parse_keybind(value);
            else if (strcmp(key, "CIRCLE") == 0) init_keybinds.CIRCLE = parse_keybind(value);
            else if (strcmp(key, "CENTER") == 0) init_keybinds.CENTER = parse_keybind(value);
            else if (strcmp(key, "CONNECTED") == 0) init_keybinds.CONNECTED = parse_keybind(value);
            else if (strcmp(key, "TWO_EDGE_CIRCLE") == 0) init_keybinds.TWO_EDGE_CIRCLE = parse_keybind(value);
            else if (strcmp(key, "DESELECT") == 0) init_keybinds.DESELECT = parse_keybind(value);
            else if (strcmp(key, "DIVIDE_NEAREST") == 0) init_keybinds.DIVIDE_NEAREST = parse_keybind(value);
            else if (strcmp(key, "END") == 0) init_keybinds.END = parse_keybind(value);
            else if (strcmp(key, "FILLET") == 0) init_keybinds.FILLET = parse_keybind(value);
            else if (strcmp(key, "POWER_FILLET") == 0) init_keybinds.POWER_FILLET = parse_keybind(value);
            else if (strcmp(key, "TOGGLE_GRID") == 0) init_keybinds.TOGGLE_GRID = parse_keybind(value);
            else if (strcmp(key, "PRINT_HISTORY") == 0) init_keybinds.PRINT_HISTORY = parse_keybind(value);
            else if (strcmp(key, "TWO_CLICK_DIVIDE") == 0) init_keybinds.TWO_CLICK_DIVIDE = parse_keybind(value);
            else if (strcmp(key, "TOGGLE_EVENT_STACK") == 0) init_keybinds.TOGGLE_EVENT_STACK = parse_keybind(value);
            else if (strcmp(key, "LINE") == 0) init_keybinds.LINE = parse_keybind(value);
            else if (strcmp(key, "MOVE") == 0) init_keybinds.MOVE = parse_keybind(value);
            else if (strcmp(key, "MIDDLE") == 0) init_keybinds.MIDDLE = parse_keybind(value);
            else if (strcmp(key, "MEASURE") == 0) init_keybinds.MEASURE = parse_keybind(value);
            else if (strcmp(key, "NUDGE_PLANE") == 0) init_keybinds.NUDGE_PLANE = parse_keybind(value);
            else if (strcmp(key, "CLEAR_DRAWING") == 0) init_keybinds.CLEAR_DRAWING = parse_keybind(value);
            else if (strcmp(key, "CLEAR_MESH") == 0) init_keybinds.CLEAR_MESH = parse_keybind(value);
            else if (strcmp(key, "LINEAR_COPY") == 0) init_keybinds.LINEAR_COPY = parse_keybind(value);
            else if (strcmp(key, "LOAD_FILE") == 0) init_keybinds.LOAD_FILE = parse_keybind(value);
            else if (strcmp(key, "POLYGON") == 0) init_keybinds.POLYGON = parse_keybind(value);
            else if (strcmp(key, "PERPENDICULAR") == 0) init_keybinds.PERPENDICULAR = parse_keybind(value);
            else if (strcmp(key, "COLOR") == 0) init_keybinds.COLOR = parse_keybind(value);
            else if (strcmp(key, "QUAD") == 0) init_keybinds.QUAD = parse_keybind(value);
            else if (strcmp(key, "ROTATE") == 0) init_keybinds.ROTATE = parse_keybind(value);
            else if (strcmp(key, "ROTATE_COPY") == 0) init_keybinds.ROTATE_COPY = parse_keybind(value);
            else if (strcmp(key, "SELECT") == 0) init_keybinds.SELECT = parse_keybind(value);
            else if (strcmp(key, "RESIZE") == 0) init_keybinds.RESIZE = parse_keybind(value);
            else if (strcmp(key, "SAVE") == 0) init_keybinds.SAVE = parse_keybind(value);
            else if (strcmp(key, "UNDO") == 0) init_keybinds.UNDO = parse_keybind(value);
            else if (strcmp(key, "UNDO_ALTERNATE") == 0) init_keybinds.UNDO_ALTERNATE = parse_keybind(value);
            else if (strcmp(key, "REDO") == 0) init_keybinds.REDO = parse_keybind(value);
            else if (strcmp(key, "REDO_ALTERNATE") == 0) init_keybinds.REDO_ALTERNATE = parse_keybind(value);
            else if (strcmp(key, "WINDOW_SELECT") == 0) init_keybinds.WINDOW_SELECT = parse_keybind(value);
            else if (strcmp(key, "XY") == 0) init_keybinds.XY = parse_keybind(value);
            else if (strcmp(key, "DRAWING_FRAME") == 0) init_keybinds.DRAWING_FRAME = parse_keybind(value);
            else if (strcmp(key, "MIRROR_X") == 0) init_keybinds.MIRROR_X = parse_keybind(value);
            else if (strcmp(key, "MIRROR_Y") == 0) init_keybinds.MIRROR_Y = parse_keybind(value);
            else if (strcmp(key, "MIRROR_LINE") == 0) init_keybinds.MIRROR_LINE = parse_keybind(value);
            else if (strcmp(key, "CYCLE_FEATURE_PLANE") == 0) init_keybinds.CYCLE_FEATURE_PLANE = parse_keybind(value);
            else if (strcmp(key, "ORIGIN") == 0) init_keybinds.ORIGIN = parse_keybind(value);
            else if (strcmp(key, "CHANGE_ORIGIN") == 0) init_keybinds.CHANGE_ORIGIN = parse_keybind(value);
            else if (strcmp(key, "SELECT_ALL") == 0) init_keybinds.SELECT_ALL = parse_keybind(value);
        }
    }

    fclose(file);
    return init_keybinds;
}
