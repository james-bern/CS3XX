#define MOD_SHIFT GLFW_MOD_SHIFT   // 0x01
#define MOD_CTRL  GLFW_MOD_CONTROL // 0x02
#define MOD_ALT   GLFW_MOD_ALT     // 0x04
#define BACKSPACE GLFW_KEY_BACKSPACE
#define DELETE GLFW_KEY_DELETE
#define ESCAPE GLFW_KEY_ESCAPE
#define TAB GLFW_KEY_TAB

struct Keybind {
    uint key;
    unsigned char modifiers;    // 00000    0    0    0
};                              //         ALT SHIFT CTRL


/////////// TODO TODO TODO /////////////
//                                    // 
//    PROCESS FOR ADDING A KEYBIND    //
//      1. add it in struct           //
//      2. set default value          //
//      3. set custom keybinding      //
//      4. add to help menu           //
//                                    //
////////////////////////////////////////



struct Keybinds {

    //////////////////////////////////////////
    //////  SNAP COMMANDS  ///////////////////
    //////////////////////////////////////////

    Keybind CENTER;
    Keybind END;
    Keybind MIDDLE;
    Keybind PERPENDICULAR;
    Keybind QUAD;
    Keybind XY;
    Keybind ZERO;


    //////////////////////////////////////////
    //////  SELECT COMMANDS  /////////////////
    //////////////////////////////////////////

    Keybind COLOR;
    Keybind SELECT_CONNECTED;
    Keybind SELECT_ALL;
    Keybind SELECT_WINDOW;


    //////////////////////////////////////////
    //////  OTHER COMMANDS  //////////////////
    //////////////////////////////////////////

    Keybind AXIS;
    Keybind BOX;
    Keybind CENTERED_BOX;
    Keybind CHANGE_ORIGIN;
    Keybind CIRCLE;
    Keybind CLEAR_DRAWING;
    Keybind CLEAR_MESH;
    Keybind CYCLE_FEATURE_PLANE;
    Keybind DELETE_SELECTED; // TODO
    Keybind DELETE_SELECTED_ALTERNATE; //TODO
    Keybind DESELECT;
    Keybind DIVIDE_NEAREST;
    Keybind DRAWING_FRAME;
    Keybind EXECUTE_COMMAND;
    Keybind EXIT_COMMAND; // TODO
    Keybind EXTRUDE_ADD;
    Keybind EXTRUDE_CUT;
    Keybind FILLET;
    Keybind HELP_MENU;
    Keybind LINE;
    Keybind LINEAR_COPY;
    Keybind LOAD_FILE;
    Keybind MEASURE;
    Keybind MIRROR_LINE;
    Keybind MIRROR_X;
    Keybind MIRROR_Y;
    Keybind MOVE;
    Keybind NEXT_POPUP_BAR;
    Keybind NUDGE_PLANE;
    Keybind OFFSET;
    Keybind POLYGON;
    Keybind POWER_FILLET;
    Keybind PREVIOUS_HOT_KEY_2D;
    Keybind PREVIOUS_HOT_KEY_3D;
    Keybind PRINT_HISTORY;
    Keybind QUALITY_0;
    Keybind QUALITY_1;
    Keybind QUALITY_2;
    Keybind QUALITY_3;
    Keybind QUALITY_4;
    Keybind QUALITY_5;
    Keybind QUALITY_6;
    Keybind QUALITY_7;
    Keybind QUALITY_8;
    Keybind QUALITY_9;
    Keybind REDO;
    Keybind REDO_ALTERNATE;
    Keybind RESIZE;
    Keybind REVOLVE_ADD;
    Keybind REVOLVE_CUT;
    Keybind ROTATE;
    Keybind ROTATE_COPY;
    Keybind SAVE;
    Keybind SAVE_AS;
    Keybind SELECT;
    Keybind TOGGLE_BUTTONS;
    Keybind TOGGLE_DRAWING_DETAILS;
    Keybind TOGGLE_EVENT_STACK;
    Keybind TOGGLE_FEATURE_PLANE;
    Keybind TOGGLE_GRID;
    Keybind TOGGLE_LIGHT_MODE;
    Keybind TWO_CLICK_DIVIDE;
    Keybind TWO_EDGE_CIRCLE;
    Keybind UNDO;
    Keybind UNDO_ALTERNATE;
    Keybind ZOOM_3D_CAMERA;
};

Keybinds keybinds;

Keybinds default_values() {
    Keybinds default_keybinds;

    #define SET_DEFAULT(NAME, KEY, MODIFIERS) default_keybinds.NAME = { KEY, MODIFIERS }

    //////////////////////////////////////////
    //////  SNAP COMMANDS  ///////////////////
    //////////////////////////////////////////

    SET_DEFAULT(CENTER, 'C', 0b000);
    SET_DEFAULT(END, 'E', 0b000); 
    SET_DEFAULT(MIDDLE, 'M', 0b000); 
    SET_DEFAULT(PERPENDICULAR, 'P', 0b000); 
    SET_DEFAULT(QUAD, 'Q', 0b000); 
    SET_DEFAULT(XY, 'X', 0b000); 
    SET_DEFAULT(ZERO, 'Z', 0b000); 


    //////////////////////////////////////////
    //////  SELECT COMMANDS  /////////////////
    //////////////////////////////////////////

    SET_DEFAULT(COLOR, 'Q', 0b000); 
    SET_DEFAULT(SELECT_ALL, 'A', 0b000); 
    SET_DEFAULT(SELECT_CONNECTED, 'C', 0b000); 
    SET_DEFAULT(SELECT_WINDOW, 'W', 0b000); 


    //////////////////////////////////////////
    //////  OTHER COMMANDS  //////////////////
    //////////////////////////////////////////

    SET_DEFAULT(AXIS, 'A', 0b000); 
    SET_DEFAULT(BOX, 'B', 0b000); 
    SET_DEFAULT(CENTERED_BOX, 'B', 0b001); 
    SET_DEFAULT(CHANGE_ORIGIN, 'Z', 0b001); 
    SET_DEFAULT(CIRCLE, 'C', 0b000); 
    SET_DEFAULT(CLEAR_DRAWING, 'N', 0b010); 
    SET_DEFAULT(CLEAR_MESH, 'N', 0b011); 
    SET_DEFAULT(CYCLE_FEATURE_PLANE, 'Y', 0b000); 
    SET_DEFAULT(DELETE_SELECTED, DELETE, 0b000 );// TODO
    SET_DEFAULT(DELETE_SELECTED_ALTERNATE, BACKSPACE, 0b000 );//TODO
    SET_DEFAULT(DESELECT, 'D', 0b000); 
    SET_DEFAULT(DIVIDE_NEAREST, 'D', 0b001); 
    SET_DEFAULT(DRAWING_FRAME, 'X', 0b011); 
    SET_DEFAULT(EXTRUDE_ADD, '[', 0b000); 
    SET_DEFAULT(EXTRUDE_CUT, '[', 0b001); 
    SET_DEFAULT(FILLET, 'F', 0b000); 
    SET_DEFAULT(HELP_MENU, '/', 0b010); 
    SET_DEFAULT(LINE, 'L', 0b000); 
    SET_DEFAULT(LINEAR_COPY, 'O', 0b000); 
    SET_DEFAULT(LOAD_FILE, 'O', 0b001); 
    SET_DEFAULT(MEASURE, 'M', 0b010); 
    SET_DEFAULT(MIRROR_LINE, 'M', 0b011); 
    SET_DEFAULT(MIRROR_X, 'X', 0b001); 
    SET_DEFAULT(MIRROR_Y, 'Y', 0b001); 
    SET_DEFAULT(MOVE, 'M', 0b000); 
    SET_DEFAULT(NUDGE_PLANE, 'N', 0b000); 
    SET_DEFAULT(OFFSET, 'J', 0b000); 
    SET_DEFAULT(POLYGON, 'P', 0b000); 
    SET_DEFAULT(POWER_FILLET, 'F', 0b001); 
    SET_DEFAULT(PREVIOUS_HOT_KEY_2D, ' ', 0b000); 
    SET_DEFAULT(PREVIOUS_HOT_KEY_3D, ' ', 0b001); 
    SET_DEFAULT(PRINT_HISTORY, 'H', 0b000); 
    SET_DEFAULT(QUALITY_0, '0', 0b00);
    SET_DEFAULT(QUALITY_1, '1', 0b00);
    SET_DEFAULT(QUALITY_2, '2', 0b00);
    SET_DEFAULT(QUALITY_3, '3', 0b00);
    SET_DEFAULT(QUALITY_4, '4', 0b00);
    SET_DEFAULT(QUALITY_5, '5', 0b00);
    SET_DEFAULT(QUALITY_6, '6', 0b00);
    SET_DEFAULT(QUALITY_7, '7', 0b00);
    SET_DEFAULT(QUALITY_8, '8', 0b00);
    SET_DEFAULT(QUALITY_9, '9', 0b00);
    SET_DEFAULT(REDO, 'U', 0b001); 
    SET_DEFAULT(REDO_ALTERNATE, 'Y', 0b011); 
    SET_DEFAULT(RESIZE, 'S', 0b001); 
    SET_DEFAULT(REVOLVE_ADD, ']', 0b000); 
    SET_DEFAULT(REVOLVE_CUT, ']', 0b001); 
    SET_DEFAULT(ROTATE, 'R', 0b000); 
    SET_DEFAULT(ROTATE_COPY, 'R', 0b001); 
    SET_DEFAULT(SAVE, 'S', 0b001); 
    SET_DEFAULT(SAVE_AS, 'S', 0b011); 
    SET_DEFAULT(SELECT, 'S', 0b000); 
    SET_DEFAULT(TOGGLE_BUTTONS, TAB, 0b001); 
    SET_DEFAULT(TOGGLE_DRAWING_DETAILS, '.', 0b000); 
    SET_DEFAULT(TOGGLE_EVENT_STACK, 'K', 0b000); 
    SET_DEFAULT(TOGGLE_FEATURE_PLANE, ';', 0b000); 
    SET_DEFAULT(TOGGLE_GRID, 'G', 0b000); 
    SET_DEFAULT(TOGGLE_LIGHT_MODE, TAB, 0b000); 
    SET_DEFAULT(TWO_CLICK_DIVIDE, 'I', 0b000); 
    SET_DEFAULT(TWO_EDGE_CIRCLE, 'C', 0b001); 
    SET_DEFAULT(UNDO, 'U', 0b000); 
    SET_DEFAULT(UNDO_ALTERNATE, 'Z', 0b010); 
    SET_DEFAULT(ZOOM_3D_CAMERA, '\'', 0b000); 


    //////////////////////////////////
    ///////  NOT SUPPORTED  //////////
    //////////////////////////////////

    SET_DEFAULT(NEXT_POPUP_BAR, TAB, 0b000 );// secretly actually supported but scary
    SET_DEFAULT(EXECUTE_COMMAND, '\n', 0b000 );// should be glfw_key_enter
    SET_DEFAULT(EXIT_COMMAND, ESCAPE, 0b000 );// TODO

    return default_keybinds;
}

uint parse_key(const char *str) {
    if (strncmp(str, "ESCAPE", 6) == 0) {
        return ESCAPE;
    } else if (strncmp(str, "TAB", 3) == 0) {
        return TAB;
    } else if (strncmp(str, "DELETE", 7) == 0) {
        return DELETE;
    } else if (strncmp(str, "BACKSPACE", 9) == 0) {
        return BACKSPACE;
    } else {
        return (uint)str[0];
    }
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
            keybind.key = parse_key(str);
            break;
        }
    }
    return keybind;
}

Keybind KEYBIND(uint key, unsigned char modifier) {
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
        if (line[0] == '#') { // excludes comments from parse
        } else if (sscanf(line, "%31[^=]=%31s", key, value) == 2) {

            //////////////////////////////////////////
            //////  SNAP COMMANDS  ///////////////////
            //////////////////////////////////////////

            #define ELIF(NAME) else if (strcmp(key, STR(NAME)) == 0) init_keybinds.NAME = parse_keybind(value)
            if (0) ;
            // else if (strcmp(key, "CENTER") == 0) init_keybinds.CENTER = parse_keybind(value);
            ELIF(CENTER);
            ELIF(END);
            ELIF(MIDDLE);
            ELIF(PERPENDICULAR);
            ELIF(QUAD);
            ELIF(XY);
            ELIF(ZERO);


            //////////////////////////////////////////
            //////  SELECT COMMANDS  /////////////////
            //////////////////////////////////////////

            ELIF(COLOR);
            ELIF(SELECT_ALL);
            ELIF(SELECT_CONNECTED);
            ELIF(SELECT_WINDOW);


            //////////////////////////////////////////
            //////  OTHER COMMANDS  //////////////////
            //////////////////////////////////////////

            ELIF(AXIS);
            ELIF(BOX);
            ELIF(CENTERED_BOX);
            ELIF(CHANGE_ORIGIN);
            ELIF(CIRCLE);
            ELIF(CLEAR_DRAWING);
            ELIF(CLEAR_MESH);
            ELIF(CYCLE_FEATURE_PLANE);
            ELIF(DELETE_SELECTED);
            ELIF(DELETE_SELECTED_ALTERNATE);
            ELIF(DESELECT);
            ELIF(DIVIDE_NEAREST);
            ELIF(DRAWING_FRAME);
            ELIF(EXIT_COMMAND);
            ELIF(EXTRUDE_ADD);
            ELIF(EXTRUDE_CUT);
            ELIF(FILLET);
            ELIF(HELP_MENU);
            ELIF(LINE);
            ELIF(LINEAR_COPY);
            ELIF(LOAD_FILE);
            ELIF(MEASURE);
            ELIF(MIRROR_LINE);
            ELIF(MIRROR_X);
            ELIF(MIRROR_Y);
            ELIF(MOVE);
            ELIF(NEXT_POPUP_BAR);
            ELIF(NUDGE_PLANE);
            ELIF(OFFSET);
            ELIF(POLYGON);
            ELIF(POWER_FILLET);
            ELIF(PREVIOUS_HOT_KEY_2D);
            ELIF(PREVIOUS_HOT_KEY_3D);
            ELIF(PRINT_HISTORY);
            ELIF(QUALITY_0);
            ELIF(QUALITY_1);
            ELIF(QUALITY_2);
            ELIF(QUALITY_3);
            ELIF(QUALITY_4);
            ELIF(QUALITY_5);
            ELIF(QUALITY_6);
            ELIF(QUALITY_7);
            ELIF(QUALITY_8);
            ELIF(QUALITY_9);
            ELIF(REDO);
            ELIF(REDO_ALTERNATE);
            ELIF(RESIZE);
            ELIF(REVOLVE_ADD);
            ELIF(REVOLVE_CUT);
            ELIF(ROTATE);
            ELIF(ROTATE_COPY);
            ELIF(SAVE);
            ELIF(SAVE_AS);
            ELIF(SELECT);
            ELIF(TOGGLE_BUTTONS);
            ELIF(TOGGLE_DRAWING_DETAILS);
            ELIF(TOGGLE_EVENT_STACK);
            ELIF(TOGGLE_FEATURE_PLANE);
            ELIF(TOGGLE_GRID);
            ELIF(TOGGLE_LIGHT_MODE);
            ELIF(TWO_CLICK_DIVIDE);
            ELIF(TWO_EDGE_CIRCLE);
            ELIF(UNDO);
            ELIF(UNDO_ALTERNATE);
            ELIF(ZOOM_3D_CAMERA);
            #undef ELIF
        }
    }

    fclose(file);
    return init_keybinds;
}
