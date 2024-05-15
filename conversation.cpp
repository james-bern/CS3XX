// // things we are unwilling to compromise on
// frame-perfect UI / graphics (except first frame FORNOW)
// ability to churn through a bunch of events in a single frame (either by user interacting really fast, spoofing, or undo/redo-ing)

// TODO: consuming a bunch of events in one frame broken again
// until ctrl+o is processed, we can't know that the next event should be a GUI event
// TODO: we need actually a whole nother layer and intermediate rep that happens preclassification
// TODO: RAW_EVENT_TYPE_KEY_PRESS
// TODO: RAW_EVENT_TYPE_MOUSE_PRESS // TODO FIRST TODO FIRST TODO bake these

// XXX: restore back undo/redo

// TODO: restore ability to move cameras

// TODO: restore hot pane persistence when dragging (so 3D camera doesn't move all weird)

// TODO: restore click and drag selection functionality

// TODO: 0 (ability to just click / click and drag to set quality)

// TODO: restore shift held 15deg snap functionality

// TODO: restore spoofing api

// TODO: muratori type "language" for scripts

// TODO: restore quality clicking

// TODO: restore SQ[0-9] functionality

// // TODO: figuring out what kind of an event an interaction is needs to happen in the callback layer
// TODO: undo redo

// TODO: GUI_MOUSE getting triggered when it shouldn't

// TODO: dragging hot pane divider

// TODO: move aesthetics to an AestheticsState

// TODO: click and drag in the text boxes




// TODO: switch entities over to taking vec2's (compression too important)

// consider a setter like SET_ECC_RCS(uint32 enter_mode, uint32 click_mode, uint32 click_modifier, bool32 record_me, bool32 checkpoint_me, bool32 snapshot_me)
// of ECC(...), RCS(...)

// void SET_ECC_RCS(int32 enter_mode, int32 click_mode, int32 click_modifier, bool32 record_me, bool32 checkpoint_me, bool32 snapshot_me) {
// }


// // TODO: good undergrad tasks
// TODO: add LAYOUT's two click mirror

// if (awaiting_second_click) // BAD VERY BAD NOW

// TODO: the USER_EVENT_TYPE_GUI_MOUSE_PRESS should be exactly what is ready to be consumed by the popup (it's just popup food) -- dragging comes later 

// NOTE multiple popups is a really easy situation to find self in (start drawing a line then extrude; this seems like a job for much later

// TODO: what would a demo app look like that proves you've actually gotten rid of all the /2 business (would need scissoring)
// NOTE: it's possible you've just broken everything on windows. whatevrer

// TODO: what about when popup becomes empty?--need some easy way of saying is zero or whatever, num_cells is worrisome

// TODO: move away from gui_printf for popups (bring cow into the app)

// NOTE: this has to happen just working on cow
// TODO: any changes to cow before the summer starts (size per vertex)
// TODO: weird factor of 2-ness with mouse
// (this is stupid macbook retina stuff)
// TODO: get rid of all factors of 2
// TODO: ? get rid of gui_NDC_from_screen versus NDC_from_screen

// TODO: colors for messagef (warning, error, info, success)

// cirle diameter goes back to zero for some reason after making a couple circles and pressing enter
// recursive call is sus


// TODO: maybe we should just like...somehow *open* the popup when the user does the thing
// TODO: this would be a big refactor

// TODO: really formalize char_equivalent

// TODO: cursor in popup
// TODO: fleury ops

// TODO: bring offset plane back
// TODO: figure out system for space bar
// TODO: hotpanegui (mouse stuff)
// TODO: being able to type equations into boxes
/////////////////////////////////////////////
// // TODO (soon): feature freeeze
// ---------------------------------
// TODO: various intersection commands
// ---------------------------------
// TODO: rewrite cow/snail in C

// TODO: merge a copy of cow into the app (split off into own repo with a cow inside it)

#include "cs345.cpp"
#include "manifoldc.h"
#include "poe.cpp"
#define u32 DO_NOT_USE_u32_USE_uint32_INSTEAD
#include "burkardt.cpp"
#include "conversation.h"
#include "elephant.cpp"

// TODO: ghostly section view on 2D side
// TODO: gui printf that doesn't automatically add a newline
// TODOLATER repeated keys shouldn't be recorded (unless enter mode changes) -- enter mode lambda in process
// TODOLATER: don't record saving

//////////////////////////////////////////////////
// SCARY MACROS //////////////////////////////////
//////////////////////////////////////////////////

#define _for_each_entity_ for (\
        DXFEntity *entity = global_world_state.dxf.entities.array;\
        entity < &global_world_state.dxf.entities.array[global_world_state.dxf.entities.length];\
        ++entity)

#define _for_each_selected_entity_ _for_each_entity_ if (entity->is_selected)

//////////////////////////////////////////////////
// DEBUG /////////////////////////////////////////
//////////////////////////////////////////////////

// #define DEBUG_DISABLE_HISTORY


//////////////////////////////////////////////////
// GLOBAL STATE //////////////////////////////////
//////////////////////////////////////////////////

WorldState global_world_state;
ScreenState _global_screen_state;



// short-name pointers
DXFState *dxf = &global_world_state.dxf;
vec2 *first_click             = &global_world_state.two_click_command.first_click;
uint32 *enter_mode            = &global_world_state.modes.enter_mode;
uint32 *click_mode            = &global_world_state.modes.click_mode;
uint32 *click_modifier        = &global_world_state.modes.click_modifier;
uint32 *click_color           = &global_world_state.modes.click_color;
bool32 *awaiting_second_click = &global_world_state.two_click_command.awaiting_second_click;
PopupState *popup = &global_world_state.popup;
real32 *extrude_add_out_length   = &popup->extrude_add_out_length;
real32 *extrude_add_in_length    = &popup->extrude_add_in_length;
real32 *extrude_cut_in_length    = &popup->extrude_cut_in_length;
real32 *extrude_cut_out_length   = &popup->extrude_cut_out_length;
real32 *x_coordinate             = &popup->x_coordinate;
real32 *y_coordinate             = &popup->y_coordinate;
real32 *circle_diameter          = &popup->circle_diameter;
real32 *circle_radius            = &popup->circle_radius;
real32 *circle_circumference     = &popup->circle_circumference;
real32 *fillet_radius            = &popup->fillet_radius;
real32 *box_width                = &popup->box_width;
real32 *box_height               = &popup->box_height;
real32 *line_length              = &popup->line_length;
real32 *line_angle               = &popup->line_angle;
real32 *line_run                 = &popup->line_run;
real32 *line_rise                = &popup->line_rise;
real32 *revolve_add_dummy        = &popup->revolve_add_dummy;
real32 *revolve_cut_dummy        = &popup->revolve_cut_dummy;
char *open_filename = popup->open_filename;
char *save_filename = popup->open_filename;
Mesh *mesh = &global_world_state.mesh;
FeaturePlaneState *feature_plane = &global_world_state.feature_plane;

//////////////////////////////////////////////////
// NON-ZERO INITIALIZERS /////////////////////////
//////////////////////////////////////////////////

void init_cameras() {
    _global_screen_state.camera_2D = { 100.0f, 0.0, 0.0f, -0.5f, -0.125f };
    // FORNOW
    if (dxf->entities.length) camera2D_zoom_to_bounding_box(&_global_screen_state.camera_2D, dxf_entities_get_bounding_box(&global_world_state.dxf.entities));
    if (!global_world_state.mesh.num_vertices) {
        _global_screen_state.camera_3D = { 2.0f * MIN(150.0f, _global_screen_state.camera_2D.screen_height_World), CAMERA_3D_DEFAULT_ANGLE_OF_VIEW, RAD(33.0f), RAD(-44.0f), 0.0f, 0.0f, 0.5f, -0.125f };
    }
}


//////////////////////////////////////////////////
// GETTERS (STATE NOT WORTH TROUBLE OF STORING) //
//////////////////////////////////////////////////
// getters

mat4 get_M_3D_from_2D() {
    vec3 up = { 0.0f, 1.0f, 0.0f };
    real32 dot_product = dot(feature_plane->normal, up);
    vec3 y = (ARE_EQUAL(ABS(dot_product), 1.0f)) ? V3(0.0f,  0.0f, -1.0f * SGN(dot_product)) : up;
    vec3 x = normalized(cross(y, feature_plane->normal));
    vec3 z = cross(x, y);

    // FORNOW
    if (ARE_EQUAL(ABS(dot_product), 1.0f) && SGN(dot_product) < 0.0f) {
        y *= -1;
    }

    return M4_xyzo(x, y, z, (feature_plane->signed_distance_to_world_origin) * feature_plane->normal);
}


bool32 click_mode_SELECT_OR_DESELECT() {
    return ((*click_mode == CLICK_MODE_SELECT) || (*click_mode == CLICK_MODE_DESELECT));
}

bool32 select_or_deselct_quality_poised_to_eat_digit() {
    bool32 A = click_mode_SELECT_OR_DESELECT();
    bool32 B = (global_world_state.modes.click_modifier == CLICK_MODIFIER_QUALITY);
    return A && B;
}

//////////////////////////////////////////////////
// STATE-DEPENDENT UTILITY FUNCTIONS /////////////
//////////////////////////////////////////////////


vec2 magic_snap(vec2 before) {
    vec2 result = before;
    {
        if (*click_modifier == CLICK_MODIFIER_SNAP_TO_CENTER_OF) {
            real32 min_squared_distance = HUGE_VAL;
            _for_each_entity_ {
                if (entity->type == DXF_ENTITY_TYPE_LINE) {
                    continue;
                } else { ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                    DXFArc *arc = &entity->arc;
                    real32 squared_distance = squared_distance_point_dxf_arc(before.x, before.y, arc);
                    if (squared_distance < min_squared_distance) {
                        min_squared_distance = squared_distance;
                        result = arc->center;
                    }
                }
            }
        } else if (*click_modifier == CLICK_MODIFIER_SNAP_TO_MIDDLE_OF) {
            real32 min_squared_distance = HUGE_VAL;
            _for_each_entity_ {
                real32 squared_distance = squared_distance_point_dxf_entity(before.x, before.y, entity);
                if (squared_distance < min_squared_distance) {
                    min_squared_distance = squared_distance;
                    entity_get_middle(entity, &result.x, &result.y);
                }
            }
        } else if (*click_modifier == CLICK_MODIFIER_SNAP_TO_END_OF) {
            real32 min_squared_distance = HUGE_VAL;
            _for_each_entity_ {
                real32 x[2], y[2];
                entity_get_start_and_end_points(entity, &x[0], &y[0], &x[1], &y[1]);
                for (uint32 d = 0; d < 2; ++d) {
                    real32 squared_distance = squared_distance_point_point(before.x, before.y, x[d], y[d]);
                    if (squared_distance < min_squared_distance) {
                        min_squared_distance = squared_distance;
                        result.x = x[d];
                        result.y = y[d];
                    }
                }
            }
        } else if (
                ( 0 
                  || (*click_mode == CLICK_MODE_CREATE_LINE)
                  || (*click_mode == CLICK_MODE_SET_AXIS)
                )
                && (*awaiting_second_click)
                && (_global_screen_state.mouse_shift_held)) {
            vec2 a = global_world_state.two_click_command.first_click;
            vec2 b = before;
            vec2 r = b - a; 
            real32 norm_r = norm(r);
            real32 factor = 360 / 15 / TAU;
            real32 theta = roundf(atan2(r) * factor) / factor;
            result = a + norm_r * e_theta(theta);
        } else if (
                (*click_mode == CLICK_MODE_CREATE_BOX)
                && (*awaiting_second_click)
                && (_global_screen_state.mouse_shift_held)) {
            // TODO (Felipe): snap square
            result = before;
        }
    }
    return result;
}

// TODO: angle_map

void mesh_draw(mat4 P_3D, mat4 V_3D, mat4 M_3D) {
    mat4 PVM_3D = P_3D * V_3D * M_3D;

    if (mesh->cosmetic_edges) {
        eso_begin(PVM_3D, SOUP_LINES); 
        // eso_color(CLAMPED_LERP(2 * _global_screen_state.successful_feature_time, monokai.white, monokai.black));
        eso_color(monokai.black);
        // 3 * num_triangles * 2 / 2
        for (uint32 k = 0; k < 2 * mesh->num_cosmetic_edges; ++k) eso_vertex(mesh->vertex_positions, mesh->cosmetic_edges[k]);
        eso_end();
    }
    for (uint32 pass = 0; pass < 2; ++pass) {
        eso_begin(PVM_3D, (!_global_screen_state.show_details) ? SOUP_TRIANGLES : SOUP_OUTLINED_TRIANGLES);

        mat3 inv_transpose_V_3D = inverse(transpose(M3(V_3D(0, 0), V_3D(0, 1), V_3D(0, 2), V_3D(1, 0), V_3D(1, 1), V_3D(1, 2), V_3D(2, 0), V_3D(2, 1), V_3D(2, 2))));

        for (uint32 i = 0; i < mesh->num_triangles; ++i) {
            vec3 n = get(mesh->triangle_normals, i);
            vec3 p[3];
            real32 x_n;
            {
                for (uint32 j = 0; j < 3; ++j) p[j] = get(mesh->vertex_positions, mesh->triangle_indices[3 * i + j]);
                x_n = dot(n, p[0]);
            }
            vec3 color; 
            real32 alpha;
            {
                vec3 n_camera = inv_transpose_V_3D * n;
                vec3 color_n = V3(0.5f + 0.5f * n_camera.x, 0.5f + 0.5f * n_camera.y, 1.0f);
                if ((feature_plane->is_active) && (dot(n, feature_plane->normal) > 0.99f) && (ABS(x_n - feature_plane->signed_distance_to_world_origin) < 0.01f)) {
                    if (pass == 0) continue;
                    color = CLAMPED_LERP(_global_screen_state.plane_selection_time, monokai.yellow, V3(0.85f, 0.87f, 0.30f));
                    alpha = CLAMPED_LERP(_global_screen_state.plane_selection_time, 1.0f, _global_screen_state.going_inside ? 
                            CLAMPED_LERP(_global_screen_state.going_inside_time, 1.0f, 0.7f)
                            : 1.0f);// ? 0.7f : 1.0f;
                } else {
                    if (pass == 1) continue;
                    color = color_n;
                    alpha = 1.0f;
                }
            }
            real32 mask = CLAMP(.6 * _global_screen_state.successful_feature_time, 0.0f, 2.0f);
            eso_color(CLAMPED_LERP(mask + sin(CLAMPED_INVERSE_LERP(p[0].y, mesh->max.y, mesh->min.y) + 0.5f * _global_screen_state.successful_feature_time), monokai.white, color), alpha);
            eso_vertex(p[0]);
            eso_color(CLAMPED_LERP(mask + sin(CLAMPED_INVERSE_LERP(p[1].y, mesh->max.y, mesh->min.y) + 0.5f * _global_screen_state.successful_feature_time), monokai.white, color), alpha);
            eso_vertex(p[1]);
            eso_color(CLAMPED_LERP(mask + sin(CLAMPED_INVERSE_LERP(p[2].y, mesh->max.y, mesh->min.y) + 0.5f * _global_screen_state.successful_feature_time), monokai.white, color), alpha);
            eso_vertex(p[2]);
        }
        eso_end();
    }
}

//////////////////////////////////////////////////
// LOADING AND SAVING STATE TO/FROM DISK /////////
//////////////////////////////////////////////////

void conversation_dxf_load(char *filename, bool preserve_cameras_and_dxf_origin = false) {
    if (!poe_file_exists(filename)) {
        conversation_messagef("[load] \"%s\" not found", filename);
        return;
    }

    list_free_AND_zero(&global_world_state.dxf.entities);

    dxf_entities_load(filename, &global_world_state.dxf.entities);

    if (!preserve_cameras_and_dxf_origin) {
        // camera2D_zoom_to_bounding_box(&_global_screen_state.camera_2D, dxf_entities_get_bounding_box(&global_world_state.dxf.entities));
        init_cameras();
        global_world_state.dxf.origin.x = 0.0f;
        global_world_state.dxf.origin.y = 0.0f;
    }

    conversation_messagef("[load] loaded %s", filename);
}

void conversation_stl_load(char *filename, bool preserve_cameras = false) {
    if (!poe_file_exists(filename)) {
        conversation_messagef("[load] \"%s\" not found", filename);
        return;
    }
    {
        stl_load(filename, &global_world_state.mesh);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(preserve_cameras);
        conversation_messagef("[load] loaded %s", filename);
    }
}

void conversation_load(char *filename, bool preserve_cameras = false) {
    if (poe_suffix_match(filename, ".dxf")) {
        conversation_dxf_load(filename, preserve_cameras);
    } else if (poe_suffix_match(filename, ".stl")) {
        conversation_stl_load(filename, preserve_cameras);
    } else {
        conversation_messagef("[load] \"%s\" fileype not supported; must be *.dxf or *.stl", filename);
    }
}

void conversation_stl_save(char *filename) {
    // TODO: prompt for overwriting
    if (mesh_save_stl(&global_world_state.mesh, filename) ) {
        conversation_messagef("[save] saved %s", filename);
    } else {
        conversation_messagef("[save] could not save open %s for writing.", filename);
    }
}

void conversation_save(char *filename) {
    if (poe_suffix_match(filename, ".stl")) {
        conversation_stl_save(filename);
    } else {
        conversation_messagef("[save] \"%s\" filetype not supported; must be *.stl", filename);
    }
}

void drop_callback(GLFWwindow *, int count, const char **paths) {
    if (count > 0) {
        char *filename = (char *) paths[0];
        conversation_load(filename);
        { // conversation_set_drop_path(filename)
            for (uint32 i = 0; filename[i]; ++i) {
                _global_screen_state.drop_path[i] = filename[i];
                if (filename[i] == '.') {
                    while (
                            (i != 0) &&
                            (_global_screen_state.drop_path[i - 1] != '\\') &&
                            (_global_screen_state.drop_path[i - 1] != '/')
                          ) --i;
                    _global_screen_state.drop_path[i] = '\0';
                    break;
                }
            }
        }
    }
}
BEGIN_PRE_MAIN { glfwSetDropCallback(COW0._window_glfw_window, drop_callback); } END_PRE_MAIN;


//////////////////////////////////////////////////
// GATHERING NEW EVENTS FROM USER ////////////////
//////////////////////////////////////////////////

Queue<RawUserEvent> raw_user_event_queue;

void callback_key(GLFWwindow *, int key, int, int action, int mods) {
    if (action == GLFW_PRESS || (action == GLFW_REPEAT)) {
        RawUserEvent raw_event = {}; {
            raw_event.type = RAW_USER_EVENT_TYPE_KEY_PRESS;
            raw_event.key = key;
            raw_event.super = (mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER));
            raw_event.shift = (mods & GLFW_MOD_SHIFT);
        }
        queue_enqueue(&raw_user_event_queue, raw_event);
    }
}

void callback_cursor_position(GLFWwindow *, double xpos, double ypos) {
    _callback_cursor_position(NULL, xpos, ypos); // FORNOW TODO TODO TODO SHIM

    _global_screen_state.mouse_in_pixel_coordinates = { real32(xpos), real32(ypos) };

    #if 0
    // // mouse held generates mouse presses
    // FORNOW repeated from callback_mouse_button
    if ((_global_screen_state.hot_pane == HOT_PANE_2D) && (_global_screen_state.mouse_left_held) && (((global_world_state.modes.click_mode == CLICK_MODE_SELECT) || (global_world_state.modes.click_mode == CLICK_MODE_DESELECT)) && (global_world_state.modes.click_modifier != CLICK_MODIFIER_WINDOW))) {
        UserEvent event; {
            event = callback_mouse_event_helper();
            event.mouse_held = true;
        }
        queue_enqueue(&queue_of_fresh_events_from_user, event);
    }
    #endif
}

void callback_mouse_button(GLFWwindow *, int button, int action, int) {
    _callback_mouse_button(NULL, button, action, 0); // FORNOW TODO TODO TODO SHIM for cow Camera's

    if ((button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_PRESS)) {
        RawUserEvent raw_event; {
            raw_event = {};
            raw_event.type = RAW_USER_EVENT_TYPE_MOUSE_PRESS;
            raw_event.mouse_in_pixel_coordinates = _global_screen_state.mouse_in_pixel_coordinates; // FORNOW
        }
        queue_enqueue(&raw_user_event_queue, raw_event);
    }
}

bool32 get_baked_type_of_raw_key_event(RawUserEvent raw_event) {
    if (!popup->_active_popup_unique_ID__FORNOW_name0) return USER_EVENT_TYPE_HOTKEY_PRESS;

    uint32 key = raw_event.key;
    // bool32 shift = event.shift;
    // bool32 super = event.super;
    bool32 key_is_digit = ('0' <= key) && (key <= '9');
    bool32 key_is_punc  = (key == '.') || (key == '-');
    bool32 key_is_alpha = ('A' <= key) && (key <= 'Z');
    bool32 key_is_delete = (key == GLFW_KEY_BACKSPACE) || (key == GLFW_KEY_DELETE);
    bool32 key_is_enter = (key == GLFW_KEY_ENTER);
    bool32 key_is_nav = (key == GLFW_KEY_TAB) || (key == GLFW_KEY_LEFT) || (key == GLFW_KEY_RIGHT);

    bool32 is_consumable_by_popup; {
        is_consumable_by_popup = false;
        if (!select_or_deselct_quality_poised_to_eat_digit()) is_consumable_by_popup |= key_is_digit;
        is_consumable_by_popup |= key_is_punc;
        is_consumable_by_popup |= key_is_delete;
        is_consumable_by_popup |= key_is_enter;
        is_consumable_by_popup |= key_is_nav;
        if (popup->_type_of_active_cell == POPUP_CELL_TYPE_REAL32) {
            ;
        } else if (popup->_type_of_active_cell == POPUP_CELL_TYPE_CSTRING) {
            is_consumable_by_popup |= key_is_alpha;
        } else {
            ASSERT(false);
        }
    }
    if (is_consumable_by_popup) return USER_EVENT_TYPE_GUI_KEY_PRESS;
    return USER_EVENT_TYPE_HOTKEY_PRESS;
}

UserEvent bake_user_event(RawUserEvent raw_event) {
    _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(raw_event);
    UserEvent result = {};
    if (raw_event.type == RAW_USER_EVENT_TYPE_KEY_PRESS) {
        result.key = raw_event.key;
        result.super = raw_event.super;
        result.shift = raw_event.shift;
        result.type = get_baked_type_of_raw_key_event(raw_event);
    } else { ASSERT(raw_event.type == RAW_USER_EVENT_TYPE_MOUSE_PRESS);
        vec2 mouse_in_NDC = transformPoint(_window_get_NDC_from_Screen(), _global_screen_state.mouse_in_pixel_coordinates);
        bool32 left_pane = _global_screen_state.mouse_in_pixel_coordinates.x < window_get_width() / 2;
        if (left_pane) {
            if (!popup->mouse_is_hovering) {
                mat4 inv_PV_2D = inverse(camera_get_PV(&_global_screen_state.camera_2D));
                result.type = USER_EVENT_TYPE_MOUSE_2D_PRESS;
                result.mouse = magic_snap(transformPoint(inv_PV_2D, mouse_in_NDC));
            } else {
                result.type = USER_EVENT_TYPE_GUI_MOUSE_PRESS;
                result.hover_cell_index = popup->hover_cell_index; 
                result.hover_cursor = popup->hover_cursor; 
            }
        } else { // right pane
            mat4 inv_PV_3D = inverse(camera_get_PV(&_global_screen_state.camera_3D));
            result.type = USER_EVENT_TYPE_MOUSE_3D_PRESS;
            result.o = transformPoint(inv_PV_3D, V3(mouse_in_NDC, -1.0f));
            result.dir = normalized(transformPoint(inv_PV_3D, V3(mouse_in_NDC, 1.0f)) - result.o);
        }
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////


void callback_scroll(GLFWwindow *, double, double yoffset) {
    _callback_scroll(NULL, 0, yoffset); // FORNOW TODO TODO TODO SHIM for cameras
}

BEGIN_PRE_MAIN {
    glfwSetKeyCallback(        COW0._window_glfw_window, callback_key);
    glfwSetCursorPosCallback(  COW0._window_glfw_window, callback_cursor_position);
    glfwSetMouseButtonCallback(COW0._window_glfw_window, callback_mouse_button);
    glfwSetScrollCallback(     COW0._window_glfw_window, callback_scroll);
} END_PRE_MAIN;

// TODO: this API should match what is printed to the terminal in verbose output mode
//       (so we can copy and paste a session for later use as an end to end test)

UserEvent KEY_event(uint32 key, bool32 super = false, bool32 shift = false) {
    ASSERT(!(('a' <= key) && (key <= 'z')));
    UserEvent event = {};
    event.type = USER_EVENT_TYPE_HOTKEY_PRESS;
    event.key = key;
    event.super = super;
    event.shift = shift;
    return event;
}

UserEvent MOUSE_2D_event(real32 mouse_x, real32 mouse_y) {
    UserEvent event = {};
    event.type = USER_EVENT_TYPE_MOUSE_2D_PRESS;
    event.mouse.x = mouse_x;
    event.mouse.y = mouse_y;
    return event;
}

UserEvent MOUSE_3D_event(real32 o_x, real32 o_y, real32 o_z, real32 dir_x, real32 dir_y, real32 dir_z) {
    UserEvent event = {};
    event.type = USER_EVENT_TYPE_MOUSE_3D_PRESS;
    event.o = { o_x, o_y, o_z };
    event.dir = { dir_x, dir_y, dir_z };
    return event;
}

/////////////////////////////////////////////////////////
// PROCESSING A SINGLE STANDARD EVENT ///////////////////
/////////////////////////////////////////////////////////
// NOTE: this function is (a teeny bit) recursive D;
// NOTE: this sometimes modifies global_world_state.dxf
// NOTE: this sometimes modifies global_world_state.mesh
// NOTE (May 5, 2024): 
// - returns flags for how the fresh event processor should deal with this event
// - is the event is marked as a snapshotted event (by construction, only possible for non-fresh events), then skips the expensive stuff

struct StandardEventProcessResult {
    bool32 record_me;
    bool32 checkpoint_me;
    bool32 snapshot_me;
};

StandardEventProcessResult standard_event_process(const UserEvent event) {
    bool32 skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack = event.snapshot_me;

    bool32 dxf_anything_selected; {
        dxf_anything_selected = false;
        _for_each_selected_entity_ {
            dxf_anything_selected = true;
            break;
        }
    }
    bool32 value_to_write_to_selection_mask = (*click_mode == CLICK_MODE_SELECT);

    StandardEventProcessResult result = {};

    #include "cookbook.cpp"

    if (event.type == USER_EVENT_TYPE_HOTKEY_PRESS) {
        result.record_me = true;

        auto key_lambda = [event](uint32 key, bool super = false, bool shift = false) -> bool {
            if (event.type != USER_EVENT_TYPE_HOTKEY_PRESS) return false; // TODO: ASSERT
            return _key_lambda(event, key, super, shift);
        };

        bool32 click_mode_SNAP_ELIGIBLE_ =
            0
            || (*click_mode == CLICK_MODE_SET_ORIGIN)
            || (*click_mode == CLICK_MODE_SET_AXIS)
            || (*click_mode == CLICK_MODE_MEASURE)
            || (*click_mode == CLICK_MODE_CREATE_LINE)
            || (*click_mode == CLICK_MODE_CREATE_BOX)
            || (*click_mode == CLICK_MODE_CREATE_CIRCLE)
            || (*click_mode == CLICK_MODE_MOVE_DXF_ENTITIES)
            ;

        bool32 digit_lambda;
        uint32 digit;
        {
            digit_lambda = false;
            for (uint32 color = 0; color <= 9; ++color) {
                if (key_lambda('0' + color)) {
                    digit_lambda = true;
                    digit = color;
                    break;
                }
            }
        }

        if (digit_lambda) {
            if (click_mode_SELECT_OR_DESELECT() && (*click_modifier == CLICK_MODIFIER_QUALITY)) { // [sd]q0
                _for_each_entity_ {
                    if (entity->color != digit) continue;
                    DXF_ENTITY_SET_IS_SELECTED(entity, value_to_write_to_selection_mask);
                }
                *click_mode = CLICK_MODE_NONE;
                *click_modifier = CLICK_MODIFIER_NONE;
            } else if ((*click_mode == CLICK_MODE_SET_QUALITY) && (*click_modifier == CLICK_MODIFIER_SELECTED)) { // qs0
                _for_each_selected_entity_ entity->color = digit;
                *click_mode = CLICK_MODE_NONE;
                *click_modifier = CLICK_MODIFIER_NONE;
                _for_each_entity_ entity->is_selected = false;
            } else { // 0
                *click_mode = CLICK_MODE_SET_QUALITY;
                *click_modifier = CLICK_MODIFIER_NONE;
                *click_color = digit;
            }
        } if (key_lambda('A')) {
            if (click_mode_SELECT_OR_DESELECT()) {
                result.checkpoint_me = true;
                DXF_CLEAR_SELECTION_MASK_TO(*click_mode == CLICK_MODE_SELECT);
                *click_mode = CLICK_MODE_NONE;
                *click_modifier = CLICK_MODIFIER_NONE;
            }
        } else if (key_lambda('A', false, true)) {
            *click_mode = CLICK_MODE_SET_AXIS;
            *click_modifier = CLICK_MODIFIER_NONE;
            *awaiting_second_click = false;
        } else if (key_lambda('B')) {
            *click_mode = CLICK_MODE_CREATE_BOX;
            *click_modifier = CLICK_MODIFIER_NONE;
            *awaiting_second_click = false;
        } else if (key_lambda('C')) {
            if (((global_world_state.modes.click_mode == CLICK_MODE_SELECT) || (global_world_state.modes.click_mode == CLICK_MODE_DESELECT)) && (global_world_state.modes.click_modifier != CLICK_MODIFIER_CONNECTED)) {
                *click_modifier = CLICK_MODIFIER_CONNECTED;
            } else if (click_mode_SNAP_ELIGIBLE_) {
                result.record_me = false;
                *click_modifier = CLICK_MODIFIER_SNAP_TO_CENTER_OF;
            } else {
                *click_mode = CLICK_MODE_CREATE_CIRCLE;
                *click_modifier = CLICK_MODIFIER_NONE;
                *awaiting_second_click = false;
            }
        } else if (key_lambda('D')) {
            *click_mode = CLICK_MODE_DESELECT;
            *click_modifier = CLICK_MODIFIER_NONE;
        } else if (key_lambda('E')) {
            if (click_mode_SNAP_ELIGIBLE_) {
                result.record_me = false;
                *click_modifier = CLICK_MODIFIER_SNAP_TO_END_OF;
            }
        } else if (key_lambda('F')) {
            *click_mode = CLICK_MODE_CREATE_FILLET;
            *click_modifier = CLICK_MODIFIER_NONE;
            *enter_mode = ENTER_MODE_NONE;
            *awaiting_second_click = false;
        } else if (key_lambda('G')) {
            result.record_me = false;
            _global_screen_state.hide_grid = !_global_screen_state.hide_grid;
        } else if (key_lambda('H')) {
            result.record_me = false;
            _global_screen_state.show_help = !_global_screen_state.show_help;
        } else if (key_lambda('K')) { 
            result.record_me = false;
            _global_screen_state.show_event_stack = !_global_screen_state.show_event_stack;
        } else if (key_lambda('K', false, true)) {
            result.record_me = false;
            _global_screen_state.hide_gui = !_global_screen_state.hide_gui;
        } else if (key_lambda('L')) {
            *click_mode = CLICK_MODE_CREATE_LINE;
            *click_modifier = CLICK_MODIFIER_NONE;
            *awaiting_second_click = false;
        } else if (key_lambda('M')) {
            if (click_mode_SNAP_ELIGIBLE_) {
                result.record_me = false;
                *click_modifier = CLICK_MODIFIER_SNAP_TO_MIDDLE_OF;
            } else {
                result.checkpoint_me = true;
                *click_mode = CLICK_MODE_MOVE_DXF_ENTITIES;
                *click_modifier = CLICK_MODIFIER_NONE;
                *awaiting_second_click = false;
            }
        } else if (key_lambda('M', false, true)) {
            result.record_me = false;
            *click_mode = CLICK_MODE_MEASURE;
            *click_modifier = CLICK_MODIFIER_NONE;
            *awaiting_second_click = false;
        } else if (key_lambda('N')) {
            if (feature_plane->is_active) {
                *enter_mode = ENTER_MODE_OFFSET_PLANE_BY;
            } else {
                conversation_messagef("[n] no plane is_selected");
            }
        } else if (key_lambda('O', true)) {
            *enter_mode = ENTER_MODE_OPEN;
        } else if (key_lambda('Q')) {
            if (!click_mode_SELECT_OR_DESELECT()) {
                *click_mode = CLICK_MODE_SET_QUALITY;
                *click_modifier = CLICK_MODIFIER_NONE;
            } else {
                *click_modifier = CLICK_MODIFIER_QUALITY;
            }
        } else if (key_lambda('Q', true)) {
            exit(1);
        } else if (key_lambda('S')) {
            if (*click_mode != CLICK_MODE_SET_QUALITY) {
                *click_mode = CLICK_MODE_SELECT;
                *click_modifier = CLICK_MODIFIER_NONE;
            } else {
                *click_modifier = CLICK_MODIFIER_SELECTED;
            }
        } else if (key_lambda('S', true)) {
            result.record_me = false;
            *enter_mode = ENTER_MODE_SAVE;
        } else if (key_lambda('W')) {
            if ((*click_mode == CLICK_MODE_SELECT) || (*click_mode == CLICK_MODE_DESELECT)) {
                *click_modifier = CLICK_MODIFIER_WINDOW;
                *awaiting_second_click = false;
            }
        } else if (key_lambda('X')) {
            if (*click_mode != CLICK_MODE_NONE) {
                *click_modifier = CLICK_MODIFIER_EXACT_X_Y_COORDINATES;
            }
        } else if (key_lambda('X', false, true)) {
            *click_mode = CLICK_MODE_X_MIRROR;
            *click_modifier = CLICK_MODIFIER_NONE;
        } else if (key_lambda('X', true, true)) {
            result.record_me = false;
            camera2D_zoom_to_bounding_box(&_global_screen_state.camera_2D, dxf_entities_get_bounding_box(&dxf->entities));
        } else if (key_lambda('Y')) {
            // TODO: 'Y' remembers last terminal choice of plane for next time
            result.checkpoint_me = true;

            // already one of the three primary planes
            if ((feature_plane->is_active) && ARE_EQUAL(feature_plane->signed_distance_to_world_origin, 0.0f) && ARE_EQUAL(squaredNorm(feature_plane->normal), 1.0f) && ARE_EQUAL(maxComponent(feature_plane->normal), 1.0f)) {
                feature_plane->normal = { feature_plane->normal[2], feature_plane->normal[0], feature_plane->normal[1] };
            } else {
                feature_plane->is_active = true;
                feature_plane->signed_distance_to_world_origin = 0.0f;
                feature_plane->normal = { 0.0f, 1.0f, 0.0f };
            }
        } else if (key_lambda('Y', false, true)) {
            *click_mode = CLICK_MODE_Y_MIRROR;
            *click_modifier = CLICK_MODIFIER_NONE;
        } else if (key_lambda('Z')) {
            UserEvent equivalent = {};
            equivalent.type = USER_EVENT_TYPE_MOUSE_2D_PRESS;
            equivalent.mouse = {};
            return standard_event_process(equivalent);
        } else if (key_lambda('Z', false, true)) {
            *click_mode = CLICK_MODE_SET_ORIGIN;
            *click_modifier = CLICK_MODIFIER_NONE;
        } else if (key_lambda(';')) {
            *enter_mode = ENTER_MODE_EXTRUDE_ADD;
        } else if (key_lambda(';', false, true)) {
            *enter_mode = ENTER_MODE_EXTRUDE_CUT;
        } else if (key_lambda('\'')) {
            *enter_mode = ENTER_MODE_REVOLVE_ADD;
        } else if (key_lambda('\'', false, true)) {
            *enter_mode = ENTER_MODE_REVOLVE_CUT;
        } else if (key_lambda('.')) { 
            result.record_me = false;
            _global_screen_state.show_details = !_global_screen_state.show_details;
        } else if (key_lambda('-')) {
            result.record_me = false;
            _global_screen_state.camera_3D.angle_of_view = CAMERA_3D_DEFAULT_ANGLE_OF_VIEW - _global_screen_state.camera_3D.angle_of_view;
        } else if (key_lambda('=')) {
            result.record_me = false;
            feature_plane->is_active = false;
        } else if (key_lambda(GLFW_KEY_BACKSPACE) || key_lambda(COW_KEY_DELETE)) {
            for (int32 i = dxf->entities.length - 1; i >= 0; --i) {
                if (dxf->entities.array[i].is_selected) {
                    _DXF_REMOVE_ENTITY(i);
                }
            }
        } else if (key_lambda(COW_KEY_ESCAPE)) {
            global_world_state.modes = {};
        } else {
            result.record_me = false;
            ;
        }
    } else if (event.type == USER_EVENT_TYPE_MOUSE_2D_PRESS) {
        result.record_me = (*click_mode != CLICK_MODE_MEASURE);

        const vec2 *mouse = &event.mouse;
        const vec2 *second_click = &event.mouse;

        bool32 click_mode_WINDOW_SELECT_OR_WINDOW_DESELECT = (click_mode_SELECT_OR_DESELECT() && (*click_modifier == CLICK_MODIFIER_WINDOW));

        bool32 click_mode_TWO_CLICK_COMMAND = 0 ||
            (*click_mode == CLICK_MODE_SET_AXIS) ||
            (*click_mode == CLICK_MODE_MEASURE) ||
            (*click_mode == CLICK_MODE_CREATE_LINE) ||
            (*click_mode == CLICK_MODE_CREATE_BOX) ||
            (*click_mode == CLICK_MODE_CREATE_CIRCLE) ||
            (*click_mode == CLICK_MODE_CREATE_FILLET) ||
            (*click_mode == CLICK_MODE_MOVE_DXF_ENTITIES) ||
            click_mode_WINDOW_SELECT_OR_WINDOW_DESELECT;

        if (click_mode_TWO_CLICK_COMMAND) {
            if (!*awaiting_second_click) {
                *awaiting_second_click = true;
                *first_click = event.mouse;
            } else {
                if (*click_mode == CLICK_MODE_MEASURE) {
                    *awaiting_second_click = false;
                    *click_mode = CLICK_MODE_NONE;
                    *click_modifier = CLICK_MODIFIER_NONE;
                    real32 angle = DEG(atan2(*second_click - *first_click));
                    if (angle < 0.0f) angle += 360.0f;
                    conversation_messagef("[measure] %gmm %gdeg", norm(*second_click - *first_click), angle);
                } else if (*click_mode == CLICK_MODE_CREATE_LINE) {
                    *awaiting_second_click = false;
                    result.checkpoint_me = true;
                    *click_mode = CLICK_MODE_NONE;
                    *click_modifier = CLICK_MODIFIER_NONE;
                    DXF_ADD_LINE(*first_click, *second_click);
                } else if (*click_mode == CLICK_MODE_CREATE_BOX) {
                    if (IS_ZERO(ABS(first_click->x - second_click->x))) {
                        conversation_messagef("[box] must have non-zero width ");
                    } else if (IS_ZERO(ABS(first_click->y - second_click->y))) {
                        conversation_messagef("[box] must have non-zero height");
                    } else {
                        *awaiting_second_click = false;
                        result.checkpoint_me = true;
                        *click_mode = CLICK_MODE_NONE;
                        *click_modifier = CLICK_MODIFIER_NONE;
                        vec2 other_corner_A = { first_click->x, second_click->y };
                        vec2 other_corner_B = { second_click->x, first_click->y };
                        DXF_ADD_LINE(*first_click,  other_corner_A);
                        DXF_ADD_LINE(*first_click,  other_corner_B);
                        DXF_ADD_LINE(*second_click, other_corner_A);
                        DXF_ADD_LINE(*second_click, other_corner_B);
                    }
                } else if (*click_mode == CLICK_MODE_MOVE_DXF_ENTITIES) {
                    *awaiting_second_click = false;
                    result.checkpoint_me = true;
                    *click_mode = CLICK_MODE_NONE;
                    *click_modifier = CLICK_MODIFIER_NONE;
                    vec2 ds = *second_click - *first_click;
                    _for_each_selected_entity_ {
                        if (entity->type == DXF_ENTITY_TYPE_LINE) {
                            DXFLine *line = &entity->line;
                            line->start += ds;
                            line->end   += ds;
                        } else { ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                            DXFArc *arc = &entity->arc;
                            arc->center += ds;
                        }
                    }
                } else if (*click_mode == CLICK_MODE_CREATE_CIRCLE) {
                    if (IS_ZERO(norm(*first_click - *second_click))) {
                        conversation_messagef("[circle] must have non-zero diameter");
                    } else {
                        *awaiting_second_click = false;
                        result.checkpoint_me = true;
                        *click_mode = CLICK_MODE_NONE;
                        *click_modifier = CLICK_MODIFIER_NONE;
                        real32 theta_a_in_degrees = DEG(atan2(*second_click - *first_click));
                        real32 theta_b_in_degrees = theta_a_in_degrees + 180.0f;
                        real32 r = norm(*second_click - *first_click);
                        DXF_ADD_ARC(*first_click, r, theta_a_in_degrees, theta_b_in_degrees);
                        DXF_ADD_ARC(*first_click, r, theta_b_in_degrees, theta_a_in_degrees);
                    }
                } else if (*click_mode == CLICK_MODE_SET_AXIS) {
                    *awaiting_second_click = false;
                    result.checkpoint_me = true;
                    dxf->axis_base_point = *first_click;
                    dxf->axis_angle_from_y = (-PI / 2) + atan2(*second_click - *first_click);
                    *click_mode = CLICK_MODE_NONE;
                    *click_modifier = CLICK_MODIFIER_NONE;
                } else if (*click_mode == CLICK_MODE_CREATE_FILLET) {
                    *awaiting_second_click = false;
                    result.checkpoint_me = true;
                    *click_modifier = CLICK_MODIFIER_NONE;
                    int i = dxf_find_closest_entity(&dxf->entities, first_click->x, first_click->y);
                    int j = dxf_find_closest_entity(&global_world_state.dxf.entities, event.mouse.x, event.mouse.y);
                    if ((i != j) && (i != -1) && (j != -1)) {
                        real32 radius = *fillet_radius;
                        DXFEntity *E_i = &dxf->entities.array[i];
                        DXFEntity *E_j = &dxf->entities.array[j];
                        if ((E_i->type == DXF_ENTITY_TYPE_LINE) && (E_j->type == DXF_ENTITY_TYPE_LINE)) {
                            vec2 a, b, c, d;
                            entity_get_start_and_end_points(E_i, &a.x, &a.y, &b.x, &b.y);
                            entity_get_start_and_end_points(E_j, &c.x, &c.y, &d.x, &d.y);

                            LineLineIntersectionResult _p = burkardt_line_line_intersection(a, b, c, d);
                            if (_p.is_valid) {
                                vec2 p = _p.position;

                                //  a -- b   p          s -- t-.  
                                //                              - 
                                //           d    =>             t
                                //     m     |             m     |
                                //           c                   s

                                //         d                              
                                //         |                              
                                //         |                              
                                //  a ---- p ---- b   =>   s - t.         
                                //         |                     -t       
                                //    m    |                 m    |       
                                //         c                      s       

                                vec2 m; {
                                    vec2 m1 = *first_click;
                                    vec2 m2 = { event.mouse.x, event.mouse.y };
                                    m = AVG(m1, m2);
                                }

                                vec2 e_ab = normalized(b - a);
                                vec2 e_cd = normalized(d - c);

                                bool32 keep_a, keep_c; {
                                    vec2 vector_p_m_in_edge_basis = inverse(hstack(e_ab, e_cd)) * (m - p);
                                    keep_a = (vector_p_m_in_edge_basis.x < 0.0f);
                                    keep_c = (vector_p_m_in_edge_basis.y < 0.0f);
                                }

                                // TODO: in general, just use burkardt's angle stuff

                                vec2 s_ab = (keep_a) ? a : b;
                                vec2 s_cd = (keep_c) ? c : d;
                                real32 half_angle; {
                                    real32 angle = burkardt_three_point_angle(s_ab, p, s_cd); // FORNOW TODO consider using burkardt's special interior version
                                    if (angle > PI) angle = TAU - angle;
                                    half_angle = angle / 2;
                                }
                                real32 length = radius / tan(half_angle);
                                vec2 t_ab = p + (keep_a ? -1 : 1) * length * e_ab;
                                vec2 t_cd = p + (keep_c ? -1 : 1) * length * e_cd;

                                LineLineIntersectionResult _center = burkardt_line_line_intersection(t_ab, t_ab + perpendicularTo(e_ab), t_cd, t_cd + perpendicularTo(e_cd));
                                if (_center.is_valid) {
                                    vec2 center = _center.position;

                                    uint32 color_i = E_i->color;
                                    uint32 color_j = E_j->color;
                                    _DXF_REMOVE_ENTITY(MAX(i, j));
                                    _DXF_REMOVE_ENTITY(MIN(i, j));

                                    DXF_ADD_LINE(s_ab, t_ab, false, color_i);
                                    DXF_ADD_LINE(s_cd, t_cd, false, color_j);

                                    real32 theta_ab_in_degrees = DEG(atan2(t_ab - center));
                                    real32 theta_cd_in_degrees = DEG(atan2(t_cd - center));

                                    if (!IS_ZERO(radius)) {
                                        if (burkardt_three_point_angle(t_ab, center, t_cd) < PI) {
                                            // FORNOW TODO consider swap
                                            real32 tmp = theta_ab_in_degrees;
                                            theta_ab_in_degrees = theta_cd_in_degrees;
                                            theta_cd_in_degrees = tmp;
                                        }

                                        // TODO: consider tabbing to create chamfer

                                        DXF_ADD_ARC(center, radius, theta_ab_in_degrees, theta_cd_in_degrees);
                                    }
                                }
                            }
                        } else {
                            conversation_messagef("TODO: line-arc fillet; arc-arc fillet");
                        }
                    }
                } else if (click_mode_WINDOW_SELECT_OR_WINDOW_DESELECT) {
                    *awaiting_second_click = false;
                    BoundingBox window = {
                        MIN(first_click->x, second_click->x),
                        MIN(first_click->y, second_click->y),
                        MAX(first_click->x, second_click->x),
                        MAX(first_click->y, second_click->y)
                    };
                    _for_each_entity_ {
                        if (bounding_box_contains(window, dxf_entity_get_bounding_box(entity))) {
                            DXF_ENTITY_SET_IS_SELECTED(entity, value_to_write_to_selection_mask);
                        }
                    }
                }
            }
        } else {
            if (*click_mode == CLICK_MODE_SET_ORIGIN) {
                result.checkpoint_me = true;
                dxf->origin = *mouse;
                global_world_state.modes = {};
            } else if (*click_mode == CLICK_MODE_X_MIRROR) {
                result.checkpoint_me = true;
                _for_each_selected_entity_ {
                    if (entity->type == DXF_ENTITY_TYPE_LINE) {
                        DXFLine *line = &entity->line;
                        DXF_BUFFER_LINE(
                                V2(-(line->start.x - mouse->x) + mouse->x, line->start.y),
                                V2(-(line->end.x - mouse->x) + mouse->x, line->end.y),
                                true,
                                entity->color
                                );
                    } else { ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                        DXFArc *arc = &entity->arc;
                        DXF_BUFFER_ARC(
                                V2(-(arc->center.x - mouse->x) + mouse->x, arc->center.y),
                                arc->radius,
                                arc->end_angle_in_degrees, // TODO
                                arc->start_angle_in_degrees, // TODO
                                true,
                                entity->color); // FORNOW + 180
                    }
                    entity->is_selected = false;
                }
                DXF_ADD_BUFFERED_ENTITIES();
            } else if (*click_mode == CLICK_MODE_Y_MIRROR) {
                result.checkpoint_me = true;
                _for_each_selected_entity_ {
                    if (entity->type == DXF_ENTITY_TYPE_LINE) {
                        DXFLine *line = &entity->line;
                        DXF_BUFFER_LINE(
                                V2(line->start.x, -(line->start.y - mouse->y) + mouse->y),
                                V2(line->end.x, -(line->end.y - mouse->y) + mouse->y),
                                true,
                                entity->color
                                );
                    } else { ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                        DXFArc *arc = &entity->arc;
                        DXF_BUFFER_ARC(
                                V2(arc->center.x, -(arc->center.y - mouse->y) + mouse->y),
                                arc->radius,
                                arc->end_angle_in_degrees, // TODO
                                arc->start_angle_in_degrees, // TODO
                                true,
                                entity->color); // FORNOW + 180
                    }
                    entity->is_selected = false;
                }
                DXF_ADD_BUFFERED_ENTITIES();
            } else if (click_mode_SELECT_OR_DESELECT()) { // includes dealing with connected modifier
                result.record_me = false;
                int hot_entity_index = dxf_find_closest_entity(&global_world_state.dxf.entities, event.mouse.x, event.mouse.y);
                if (hot_entity_index != -1) {
                    if (global_world_state.modes.click_modifier != CLICK_MODIFIER_CONNECTED) {
                        DXF_ENTITY_SET_IS_SELECTED(&dxf->entities.array[hot_entity_index], value_to_write_to_selection_mask);
                    } else {
                        #if 1 // TODO: consider just using the O(n*m) algorithm here instead

                        #define GRID_CELL_WIDTH 0.001f

                        auto scalar_bucket = [&](real32 a) -> real32 {
                            return roundf(a / GRID_CELL_WIDTH) * GRID_CELL_WIDTH;
                        };

                        auto make_key = [&](real32 x, real32 y) -> vec2 {
                            return { scalar_bucket(x), scalar_bucket(y) };

                        };

                        auto nudge_key = [&](vec2 key, int dx, int dy) -> vec2 {
                            return make_key(key.x + dx * GRID_CELL_WIDTH, key.y + dy * GRID_CELL_WIDTH);
                        };

                        struct GridPointSlot {
                            bool32 populated;
                            int32 entity_index;
                            bool32 end_NOT_start;
                        };

                        struct GridCell {
                            GridPointSlot slots[2];
                        };

                        Map<vec2, GridCell> grid; { // TODO: build grid
                            grid = {};

                            auto push_into_grid_unless_cell_full__make_cell_if_none_exists = [&](real32 x, real32 y, uint32 entity_index, bool32 end_NOT_start) {
                                vec2 key = make_key(x, y);
                                GridCell *cell = _map_get_pointer(&grid, key);
                                if (cell == NULL) {
                                    map_put(&grid, key, {});
                                    cell = _map_get_pointer(&grid, key);
                                }
                                for (uint32 i = 0; i < ARRAY_LENGTH(cell->slots); ++i) {
                                    GridPointSlot *slot = &cell->slots[i];
                                    if (slot->populated) continue;
                                    slot->populated = true;
                                    slot->entity_index = entity_index;
                                    slot->end_NOT_start = end_NOT_start;
                                    // printf("%f %f [%d]\n", key.x, key.y, i);
                                    break;
                                }
                            };

                            for (uint32 entity_index = 0; entity_index < global_world_state.dxf.entities.length; ++entity_index) {
                                DXFEntity *entity = &global_world_state.dxf.entities.array[entity_index];

                                real32 start_x, start_y, end_x, end_y;
                                entity_get_start_and_end_points(entity, &start_x, &start_y, &end_x, &end_y);
                                push_into_grid_unless_cell_full__make_cell_if_none_exists(start_x, start_y, entity_index, false);
                                push_into_grid_unless_cell_full__make_cell_if_none_exists(end_x, end_y, entity_index, true);
                            }
                        }

                        bool32 *edge_marked = (bool32 *) calloc(global_world_state.dxf.entities.length, sizeof(bool32));

                        ////////////////////////////////////////////////////////////////////////////////
                        // NOTE: We are now done adding to the grid, so we can now operate directly on GridCell *'s
                        //       We will use _map_get_pointer(...)
                        ////////////////////////////////////////////////////////////////////////////////


                        auto get_key = [&](GridPointSlot *point, bool32 other_endpoint) {
                            bool32 end_NOT_start; {
                                end_NOT_start = point->end_NOT_start;
                                if (other_endpoint) end_NOT_start = !end_NOT_start;
                            }
                            real32 x, y; {
                                DXFEntity *entity = &global_world_state.dxf.entities.array[point->entity_index];
                                if (end_NOT_start) {
                                    entity_get_end_point(entity, &x, &y);
                                } else {
                                    entity_get_start_point(entity, &x, &y);
                                }
                            }
                            return make_key(x, y);
                        };

                        auto get_any_point_not_part_of_an_marked_entity = [&](vec2 key) -> GridPointSlot * {
                            GridCell *cell = _map_get_pointer(&grid, key);
                            if (!cell) return NULL;

                            for (uint32 i = 0; i < ARRAY_LENGTH(cell->slots); ++i) {
                                GridPointSlot *slot = &cell->slots[i];
                                if (!slot->populated) continue;
                                if (edge_marked[slot->entity_index]) continue;
                                return slot;
                            }
                            return NULL;
                        };



                        // NOTE: we will mark the hot entity, and then shoot off from both its endpoints
                        edge_marked[hot_entity_index] = true;
                        DXF_ENTITY_SET_IS_SELECTED(&dxf->entities.array[hot_entity_index], value_to_write_to_selection_mask);

                        for (uint32 pass = 0; pass <= 1; ++pass) {

                            vec2 seed; {
                                real32 x, y;
                                if (pass == 0) {
                                    entity_get_start_point(&global_world_state.dxf.entities.array[hot_entity_index], &x, &y);
                                } else {
                                    entity_get_end_point(&global_world_state.dxf.entities.array[hot_entity_index], &x, &y);
                                }
                                seed = make_key(x, y);
                            }

                            GridPointSlot *curr = get_any_point_not_part_of_an_marked_entity(seed);
                            while (true) {
                                if (curr == NULL) break;
                                DXF_ENTITY_SET_IS_SELECTED(&dxf->entities.array[curr->entity_index], value_to_write_to_selection_mask);
                                edge_marked[curr->entity_index] = true;
                                curr = get_any_point_not_part_of_an_marked_entity(get_key(curr, true)); // get other end
                                if (curr == NULL) break;
                                { // curr <- next (9-cell)
                                    vec2 key = get_key(curr, false);
                                    {
                                        curr = NULL;
                                        for (int dx = -1; dx <= 1; ++dx) {
                                            for (int dy = -1; dy <= 1; ++dy) {
                                                GridPointSlot *tmp = get_any_point_not_part_of_an_marked_entity(nudge_key(key, dx, dy));
                                                if (tmp) curr = tmp;
                                            }
                                        }
                                    }
                                }
                            }
                        }




                        map_free_and_zero(&grid);
                        free(edge_marked);

                        #else // old O(n^2) version
                        uint32 loop_index = dxf_pick_loops.loop_index_from_entity_index[hot_entity_index];
                        DXFEntityIndexAndFlipFlag *loop = dxf_pick_loops.loops[loop_index];
                        uint32 num_entities = dxf_pick_loops.num_entities_in_loops[loop_index];
                        for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < &loop[num_entities]; ++entity_index_and_flip_flag) {
                            DXF_ENTITY_SET_IS_SELECTED(&dxf->entities[entity_index_and_flip_flag->entity_index], value_to_write_to_selection_mask);
                        }
                        #endif
                    }
                }
            }
        }
    } else if (event.type == USER_EVENT_TYPE_MOUSE_3D_PRESS) {
        result.record_me = false;
        int32 index_of_first_triangle_hit_by_ray = -1;
        {
            real32 min_distance = HUGE_VAL;
            for (uint32 i = 0; i < global_world_state.mesh.num_triangles; ++i) {
                vec3 p[3]; {
                    for (uint32 j = 0; j < 3; ++j) p[j] = get(global_world_state.mesh.vertex_positions, global_world_state.mesh.triangle_indices[3 * i + j]);
                }
                RayTriangleIntersectionResult ray_triangle_intersection_result = ray_triangle_intersection(event.o, event.dir, p[0], p[1], p[2]);
                if (ray_triangle_intersection_result.hit) {
                    if (ray_triangle_intersection_result.distance < min_distance) {
                        min_distance = ray_triangle_intersection_result.distance;
                        index_of_first_triangle_hit_by_ray = i; // FORNOW
                    }
                }
            }
        }

        if (index_of_first_triangle_hit_by_ray != -1) { // something hit
            result.checkpoint_me = result.record_me = true;
            feature_plane->is_active = true;
            {
                vec3 n_prev = feature_plane->normal;
                real32 d_prev = feature_plane->signed_distance_to_world_origin;

                feature_plane->normal = get(global_world_state.mesh.triangle_normals, index_of_first_triangle_hit_by_ray);
                { // FORNOW (gross) calculateion of feature_plane->signed_distance_to_world_origin
                    vec3 a_selected = get(global_world_state.mesh.vertex_positions, global_world_state.mesh.triangle_indices[3 * index_of_first_triangle_hit_by_ray + 0]);
                    feature_plane->signed_distance_to_world_origin = dot(feature_plane->normal, a_selected);
                }

                if (!IS_ZERO(norm(n_prev - feature_plane->normal)) || !ARE_EQUAL(d_prev, feature_plane->signed_distance_to_world_origin)) {
                    _global_screen_state.plane_selection_time = 0.0f;
                }
            }
        }
    } else if (event.type == USER_EVENT_TYPE_GUI_KEY_PRESS) {
        result.record_me = true;

        _global_screen_state.popup_blinker_time = 0.0; // FORNOW

        uint32 key = event.key;
        bool32 shift = event.shift;
        bool32 super = event.super;

        bool32 _tab_hack_so_aliases_not_introduced_too_far_up = false;
        if (key == GLFW_KEY_TAB) {
            _tab_hack_so_aliases_not_introduced_too_far_up = true;
            uint32 new_active_cell_index; {
                // FORNOW
                if (!shift) {
                    new_active_cell_index = (popup->active_cell_index + 1) % popup->num_cells;
                } else {
                    if (popup->active_cell_index != 0) {
                        new_active_cell_index = popup->active_cell_index - 1;
                    } else {
                        new_active_cell_index = popup->num_cells - 1;
                    }
                }
            }
            POPUP_SET_ACTIVE_CELL_INDEX(new_active_cell_index);
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
                if (POPUP_SELECTION_NOT_ACTIVE()) {
                    if (popup->cursor > 0) --popup->cursor;
                } else {
                    popup->cursor = left_cursor;
                }
                popup->selection_cursor = popup->cursor;
            } else if (shift && !super) {
                if (POPUP_SELECTION_NOT_ACTIVE()) popup->selection_cursor = popup->cursor;
                if (popup->cursor > 0) --popup->cursor;
            } else if (super && !shift) {
                popup->selection_cursor = popup->cursor = 0;
            } else { ASSERT(shift && super);
                popup->selection_cursor = 0;
            }
        } else if (key == GLFW_KEY_RIGHT) {
            if (!shift && !super) {
                if (POPUP_SELECTION_NOT_ACTIVE()) {
                    if (popup->cursor < len) ++popup->cursor;
                } else {
                    popup->cursor = MAX(popup->cursor, popup->selection_cursor);
                }
                popup->selection_cursor = popup->cursor;
            } else if (shift && !super) {
                if (POPUP_SELECTION_NOT_ACTIVE()) popup->selection_cursor = popup->cursor;
                if (popup->cursor < len) ++popup->cursor;
            } else if (super && !shift) {
                popup->selection_cursor = popup->cursor = len;
            } else { ASSERT(shift && super);
                popup->selection_cursor = len;
            }
        } else if (key == GLFW_KEY_BACKSPACE) {
            // * * * *|* * * * 
            if (POPUP_SELECTION_NOT_ACTIVE()) {
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
        } else if (key == GLFW_KEY_ENTER) {
            ;
        } else {
            // TODO: strip char_equivalent into function

            bool32 key_is_alpha = ('A' <= key) && (key <= 'Z');

            char char_equivalent; {
                char_equivalent = (char) key;
                if (!shift && key_is_alpha) {
                    char_equivalent = 'a' + (char_equivalent - 'A');
                }
            }
            if (POPUP_SELECTION_NOT_ACTIVE()) {
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
    } else if (event.type == USER_EVENT_TYPE_GUI_MOUSE_PRESS) {
        result.record_me = true;

        _global_screen_state.popup_blinker_time = 0.0f;
        if (event.hover_cell_index == popup->active_cell_index) {
            ;
        } else {
            POPUP_SET_ACTIVE_CELL_INDEX(event.hover_cell_index);
        }

        if (event.hover_cursor == popup->cursor) { // FORNOW TODO: time limit
            ;
        } else {
            popup->cursor = event.hover_cursor;
            popup->selection_cursor = popup->cursor;
        }
    } else { ASSERT(event.type == USER_EVENT_TYPE_NONE);
        result.record_me = false;
    }

    { // sanity checks
        ASSERT(popup->active_cell_index <= popup->num_cells);
        ASSERT(popup->cursor <= POPUP_CELL_LENGTH);
        ASSERT(popup->selection_cursor <= POPUP_CELL_LENGTH);
    }

    { // popup_popup
        static char full_filename_scratch_buffer[512];

        uint32 _enter_mode_prev__NOTE_used_to_determine_when_to_close_popup_on_enter = *enter_mode;
        bool32 popup_popup_actually_called_this_event = false;
        #include "popup_lambda.cpp"
        { // popup_popup
            bool32 enter = ((event.type == USER_EVENT_TYPE_GUI_KEY_PRESS) && (event.key == GLFW_KEY_ENTER));
            if (*enter_mode == ENTER_MODE_OPEN) {
                sprintf(full_filename_scratch_buffer, "%s%s", _global_screen_state.drop_path, open_filename);
                popup_popup(false,
                        POPUP_CELL_TYPE_CSTRING, "open_filename", open_filename);
                if (enter) {
                    if (poe_suffix_match(full_filename_scratch_buffer, ".dxf")) {
                        result.record_me = true;
                        result.checkpoint_me = true;
                        result.snapshot_me = true;
                        conversation_dxf_load(full_filename_scratch_buffer,
                                skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack);
                        global_world_state.modes.enter_mode = ENTER_MODE_NONE;
                    } else if (poe_suffix_match(full_filename_scratch_buffer, ".stl")) {
                        result.record_me = true;
                        result.checkpoint_me = true;
                        result.snapshot_me = true;
                        conversation_stl_load(full_filename_scratch_buffer);
                        global_world_state.modes.enter_mode = ENTER_MODE_NONE;
                    } else {
                        conversation_messagef("[open] \"%s\" not found", full_filename_scratch_buffer);
                    }
                }
            } else if (*enter_mode == ENTER_MODE_SAVE) {
                result.record_me = false;
                sprintf(full_filename_scratch_buffer, "%s%s", _global_screen_state.drop_path, save_filename);
                popup_popup(false,
                        POPUP_CELL_TYPE_CSTRING, "save_filename", save_filename);
                if (enter) {
                    conversation_save(full_filename_scratch_buffer);
                    global_world_state.modes.enter_mode = ENTER_MODE_NONE;
                }
            } else if (*enter_mode == ENTER_MODE_EXTRUDE_ADD) {
                popup_popup(true,
                        POPUP_CELL_TYPE_REAL32, "extrude_add_out_length", extrude_add_out_length,
                        POPUP_CELL_TYPE_REAL32, "extrude_add_in_length",  extrude_add_in_length);
                if (enter) {
                    if (!dxf_anything_selected) {
                        conversation_messagef("[extrude-add] no dxf elements selected");
                    } else if (!feature_plane->is_active) {
                        conversation_messagef("[extrude-add] no plane selected");
                    } else if (IS_ZERO(global_world_state.popup.extrude_add_in_length) && IS_ZERO(global_world_state.popup.extrude_add_out_length)) {
                        conversation_messagef("[extrude-add] must have non-zero total height");
                    } else {
                        GENERAL_PURPOSE_MANIFOLD_WRAPPER();
                        conversation_messagef("[extrude-add] success");
                    }
                }
            } else if (*enter_mode == ENTER_MODE_EXTRUDE_CUT) {
                popup_popup(true,
                        POPUP_CELL_TYPE_REAL32, "extrude_cut_in_length",  extrude_cut_in_length,
                        POPUP_CELL_TYPE_REAL32, "extrude_cut_out_length", extrude_cut_out_length);
                if (enter) {
                    if (!dxf_anything_selected) {
                        conversation_messagef("[extrude-cut] no dxf elements selected");
                    } else if (!feature_plane->is_active) {
                        conversation_messagef("[extrude-cut] no plane selected");
                    } else if (IS_ZERO(global_world_state.popup.extrude_cut_in_length) && IS_ZERO(global_world_state.popup.extrude_cut_out_length)) {
                        conversation_messagef("[extrude-cut] must have non-zero total height");
                    } else if (global_world_state.mesh.num_triangles == 0) {
                        conversation_messagef("[extrude-cut] no mesh to cut from");
                    } else {
                        GENERAL_PURPOSE_MANIFOLD_WRAPPER();
                        conversation_messagef("[extrude-cut] success");
                    }
                }
            } else if (*enter_mode == ENTER_MODE_REVOLVE_ADD) {
                popup_popup(true, POPUP_CELL_TYPE_REAL32, "revolve_add_dummy", revolve_add_dummy);
                if (enter) {
                    if (!dxf_anything_selected) {
                        conversation_messagef("[revolve-add] no dxf elements selected");
                    } else if (!feature_plane->is_active) {
                        conversation_messagef("[revolve-add] no plane selected");
                    } else {
                        GENERAL_PURPOSE_MANIFOLD_WRAPPER();
                        conversation_messagef("[revolve-add] success");
                    }
                }
            } else if (*enter_mode == ENTER_MODE_REVOLVE_CUT) {
                popup_popup(true, POPUP_CELL_TYPE_REAL32, "revolve_cut_dummy", revolve_cut_dummy);
                if (enter) {
                    if (!dxf_anything_selected) {
                        conversation_messagef("[revolve-cut] no dxf elements selected");
                    } else if (!feature_plane->is_active) {
                        conversation_messagef("[revolve-cut] no plane selected");
                    } else if (global_world_state.mesh.num_triangles == 0) {
                        conversation_messagef("[revolve-cut] no mesh to cut from");
                    } else {
                        GENERAL_PURPOSE_MANIFOLD_WRAPPER();
                        conversation_messagef("[revolve-cut] success");
                    }
                }
            } else if (*click_modifier == CLICK_MODIFIER_EXACT_X_Y_COORDINATES) {
                // sus calling this a modifier but okay; make sure it's first or else bad bad
                popup_popup(true,
                        POPUP_CELL_TYPE_REAL32, "x coordinate", x_coordinate,
                        POPUP_CELL_TYPE_REAL32, "y coordinate", y_coordinate);
                if (enter) {
                    // popup->_active_popup_unique_ID__FORNOW_name0 = NULL; // FORNOW when making box using 'X' 'X', we want the popup to trigger a reload
                    global_world_state.modes.click_modifier = CLICK_MODIFIER_NONE;
                    return standard_event_process(MOUSE_2D_event(*x_coordinate, *y_coordinate));
                }
            } else if (*click_mode == CLICK_MODE_CREATE_CIRCLE) {
                if (*awaiting_second_click) {
                    real32 prev_circle_diameter = *circle_diameter;
                    real32 prev_circle_radius = *circle_radius;
                    real32 prev_circle_circumference = *circle_circumference;
                    popup_popup(false,
                            POPUP_CELL_TYPE_REAL32, "circle_diameter", circle_diameter,
                            POPUP_CELL_TYPE_REAL32, "circle_radius", circle_radius,
                            POPUP_CELL_TYPE_REAL32, "circle_circumference", circle_circumference);
                    if (enter) {
                        return standard_event_process(MOUSE_2D_event(first_click->x + *circle_radius, first_click->y));
                    } else {
                        if (prev_circle_diameter != *circle_diameter) {
                            *circle_radius = *circle_diameter / 2;
                            *circle_circumference = PI * *circle_diameter;
                        } else if (prev_circle_radius != *circle_radius) {
                            *circle_diameter = 2 * *circle_radius;
                            *circle_circumference = PI * *circle_diameter;
                        } else if (prev_circle_circumference != *circle_circumference) {
                            *circle_diameter = *circle_circumference / PI;
                            *circle_radius = *circle_diameter / 2;
                        }
                    }
                }
            } else if (*click_mode == CLICK_MODE_CREATE_LINE) {
                if (*awaiting_second_click) {
                    real32 prev_line_length = *line_length;
                    real32 prev_line_angle = *line_angle;
                    real32 prev_line_run = *line_run;
                    real32 prev_line_rise = *line_rise;
                    popup_popup(false,
                            POPUP_CELL_TYPE_REAL32, "line_length", line_length,
                            POPUP_CELL_TYPE_REAL32, "line_angle", line_angle,
                            POPUP_CELL_TYPE_REAL32, "line_run", line_run,
                            POPUP_CELL_TYPE_REAL32, "line_rise", line_rise
                            );
                    if (enter) {
                        return standard_event_process(MOUSE_2D_event(first_click->x + *line_run, first_click->y + *line_rise));
                    } else {
                        if ((prev_line_length != *line_length) || (prev_line_angle != *line_angle)) {
                            *line_run = *line_length * COS(RAD(*line_angle));
                            *line_rise = *line_length * SIN(RAD(*line_angle));
                        } else if ((prev_line_run != *line_run) || (prev_line_rise != *line_rise)) {
                            *line_length = SQRT(*line_run * *line_run + *line_rise * *line_rise);
                            *line_angle = DEG(ATAN2(*line_rise, *line_run));
                        }
                    }
                }
            } else if (*click_mode == CLICK_MODE_CREATE_BOX) {
                if (*awaiting_second_click) {
                    popup_popup(true,
                            POPUP_CELL_TYPE_REAL32, "box_width", box_width,
                            POPUP_CELL_TYPE_REAL32, "box_height", box_height);
                    if (enter) {
                        return standard_event_process(MOUSE_2D_event(first_click->x + *box_width, first_click->y + *box_height));
                    }
                }
            } else if (*click_mode == CLICK_MODE_CREATE_FILLET) {
                popup_popup(false,
                        POPUP_CELL_TYPE_REAL32, "fillet radius", fillet_radius);
            }
        }
        { // popup_close (FORNOW: just doing off of enter transitions)
          // NOTE: we need to do this so that the next key event doesn't get eaten by a dead popup
            bool32 enter_mode_transitioned_to_ENTER_MODE_NONE = ((_enter_mode_prev__NOTE_used_to_determine_when_to_close_popup_on_enter != ENTER_MODE_NONE) && (*enter_mode == ENTER_MODE_NONE));
            if (0
                    || (!popup_popup_actually_called_this_event)
                    || enter_mode_transitioned_to_ENTER_MODE_NONE
               ) {
                global_world_state.popup._active_popup_unique_ID__FORNOW_name0 = NULL;
            }
        }
    }

    return result;
}

//////////////////////////////////////////////////
// HISTORY ///////////////////////////////////////
//////////////////////////////////////////////////

#ifdef DEBUG_DISABLE_HISTORY
//
void history_process_and_potentially_record_checkpoint_and_or_snapshot_standard_fresh_user_event(UserEvent standard_event) { standard_event_process(standard_event); }
void history_undo() { conversation_messagef("[DEBUG] history disabled"); }
void history_redo() { conversation_messagef("[DEBUG] history disabled"); }
void history_debug_draw() { gui_printf("[DEBUG] history disabled"); }
//
#else

struct {
    ElephantStack<UserEvent> recorded_user_events;
    ElephantStack<WorldState> snapshotted_world_states;
} history;

struct StackPointers {
    UserEvent *user_event;
    WorldState *world_state;
};

StackPointers POP_UNDO_ONTO_REDO() {
    StackPointers result = {};
    result.user_event = elephant_pop_undo_onto_redo(&history.recorded_user_events);
    if (result.user_event->snapshot_me) result.world_state = elephant_pop_undo_onto_redo(&history.snapshotted_world_states);
    return result;
};

StackPointers POP_REDO_ONTO_UNDO() {
    StackPointers result = {};
    result.user_event = elephant_pop_redo_onto_undo(&history.recorded_user_events);
    if (result.user_event->snapshot_me) result.world_state = elephant_pop_redo_onto_undo(&history.snapshotted_world_states);
    return result;
};

StackPointers PEEK_UNDO() {
    UserEvent *user_event = elephant_peek_undo(&history.recorded_user_events);
    WorldState *world_state = elephant_is_empty_undo(&history.snapshotted_world_states) ? NULL : elephant_peek_undo(&history.snapshotted_world_states);
    return { user_event, world_state };
}

bool32 EVENT_UNDO_NONEMPTY() {
    return !elephant_is_empty_undo(&history.recorded_user_events);
}

bool32 EVENT_REDO_NONEMPTY() {
    return !elephant_is_empty_redo(&history.recorded_user_events);
}

void PUSH_UNDO_CLEAR_REDO(UserEvent standard_event) {
    elephant_push_undo_clear_redo(&history.recorded_user_events, standard_event);
    { // clear the world_state redo stack
        for (////
                WorldState *world_state = history.snapshotted_world_states._redo_stack.array;
                world_state < history.snapshotted_world_states._redo_stack.array + history.snapshotted_world_states._redo_stack.length;
                ++world_state
            ) {//
            world_state_free_AND_zero(world_state);
        }
        elephant_clear_redo(&history.snapshotted_world_states); // TODO ?
    }
    if (standard_event.snapshot_me) {
        WorldState snapshot;
        world_state_deep_copy(&snapshot, &global_world_state);
        elephant_push_undo_clear_redo(&history.snapshotted_world_states, snapshot); // TODO: clear is unnecessary here
    }
}

void history_process_and_potentially_record_checkpoint_and_or_snapshot_standard_fresh_user_event(UserEvent standard_event) {
    StandardEventProcessResult tmp = standard_event_process(standard_event);
    standard_event.record_me = tmp.record_me;
    standard_event.checkpoint_me = tmp.checkpoint_me;
    standard_event.snapshot_me = tmp.snapshot_me;
    if (standard_event.record_me) PUSH_UNDO_CLEAR_REDO(standard_event);
}

void history_undo() {
    if (elephant_is_empty_undo(&history.recorded_user_events)) {
        conversation_messagef("[undo] nothing to undo");
        return;
    }

    { // // manipulate stacks (undo -> redo)
      // 1) pop through a first checkpoint 
      // 2) pop up to a second checkpoint
        while (EVENT_UNDO_NONEMPTY()) { if (POP_UNDO_ONTO_REDO().user_event->checkpoint_me) break; }
        while (EVENT_UNDO_NONEMPTY()) {
            if (PEEK_UNDO().user_event->checkpoint_me) break;
            POP_UNDO_ONTO_REDO();
        }
    }
    UserEvent *one_past_end = elephant_undo_ptr_one_past_end(&history.recorded_user_events);
    UserEvent *begin;
    { // // find beginning
      // 1) walk back to snapshot event (or end of stack)
      // TODO: this feels kind of sloppy still
        begin = one_past_end - 1; // ?
        world_state_free_AND_zero(&global_world_state);
        while (true) {
            if (begin <= elephant_undo_ptr_begin(&history.recorded_user_events)) {
                begin =  elephant_undo_ptr_begin(&history.recorded_user_events); // !
                global_world_state = {};
                break;
            }
            if (begin->snapshot_me) {
                world_state_deep_copy(&global_world_state, PEEK_UNDO().world_state);
                break;
            }
            --begin;
        }
    }
    IGNORE_NEW_MESSAGEFS = true; // TODO: why does this seem unnecessary in practice??
    for (UserEvent *event = begin; event < one_past_end; ++event) standard_event_process(*event);
    IGNORE_NEW_MESSAGEFS = false;
    conversation_messagef("[undo] success");
}

void history_redo() {
    if (elephant_is_empty_redo(&history.recorded_user_events)) {
        conversation_messagef("[redo] nothing to redo");
        return;
    }

    IGNORE_NEW_MESSAGEFS = true;
    while (EVENT_REDO_NONEMPTY()) { // // manipulate stacks (undo <- redo)
        StackPointers popped = POP_REDO_ONTO_UNDO();
        UserEvent *user_event = popped.user_event;
        WorldState *world_state = popped.world_state;

        standard_event_process(*user_event);
        if (world_state) {
            world_state_free_AND_zero(&global_world_state);
            world_state_deep_copy(&global_world_state, world_state);
        }

        if (user_event->checkpoint_me) break;
    }
    IGNORE_NEW_MESSAGEFS = false;
    conversation_messagef("[redo] success");
}

void _history_user_event_draw_helper(UserEvent *event) {
    char message[256]; {
        // TODO: Left and right arrow
        // TODO: handle shift and super with the special characters
        if ((event->type == USER_EVENT_TYPE_HOTKEY_PRESS) || (event->type == USER_EVENT_TYPE_GUI_KEY_PRESS)) {
            char *boxed; {
                if (event->type == USER_EVENT_TYPE_HOTKEY_PRESS) {
                    boxed = "[HOTKEY]";
                } else { ASSERT(event->type == USER_EVENT_TYPE_GUI_KEY_PRESS);
                    boxed = "[GUI_KEY]";
                }
            }

            char *_ctrl_plus; {
                if (!event->super) {
                    _ctrl_plus = "";
                } else {
                    _ctrl_plus = "CTRL+";
                }
            }

            char *_shift_plus; {
                if (!event->shift) {
                    _shift_plus = "";
                } else {
                    _shift_plus = "SHIFT+";
                }
            }

            char _key_buffer[2];
            char *_key; {
                if (event->key == GLFW_KEY_ENTER) {
                    _key = "ENTER";
                } else if (event->key == GLFW_KEY_TAB) {
                    _key = "TAB";
                } else if (event->key == GLFW_KEY_ESCAPE) {
                    _key = "ESCAPE";
                } else if (event->key == GLFW_KEY_BACKSPACE) {
                    _key = "BACKSPACE";
                } else if (event->key == GLFW_KEY_DELETE) {
                    _key = "DELETE";
                } else if (event->key == GLFW_KEY_LEFT) {
                    _key = "LEFT";
                } else if (event->key == GLFW_KEY_RIGHT) {
                    _key = "RIGHT";
                } else {
                    _key_buffer[0] = (char) event->key;
                    _key_buffer[1] = '\0';
                    _key = _key_buffer;
                }
            }

            sprintf(message, "%s %s%s%s", boxed, _ctrl_plus, _shift_plus, _key);
        } else if (event->type == USER_EVENT_TYPE_MOUSE_2D_PRESS) {
            sprintf(message, "[MOUSE-2D] %g %g", event->mouse.x, event->mouse.y);
        } else if (event->type == USER_EVENT_TYPE_MOUSE_3D_PRESS) {
            sprintf(message, "[MOUSE-3D] %g %g %g %g %g %g", event->o.x, event->o.y, event->o.z, event->dir.x, event->dir.y, event->dir.z);
        } else { ASSERT(event->type == USER_EVENT_TYPE_GUI_MOUSE_PRESS);
            sprintf(message, "[GUI-MOUSE-2D] %d %d", event->hover_cell_index, event->hover_cursor);
        }
    }
    gui_printf("%c%c %s",
            (event->checkpoint_me)   ? 'C' : ' ',
            (event->snapshot_me)     ? 'S' : ' ',
            message);
}

void _history_world_state_draw_helper(WorldState *world_state) {
    gui_printf("%d dxf elements   %d stl triangles", world_state->dxf.entities.length, world_state->mesh.num_triangles);
}

void history_debug_draw() {
    gui_printf("");
    if (history.snapshotted_world_states._redo_stack.length) {
        for (////
                WorldState *world_state = history.snapshotted_world_states._redo_stack.array;
                world_state < history.snapshotted_world_states._redo_stack.array + history.snapshotted_world_states._redo_stack.length;
                ++world_state
            ) {//
            _history_world_state_draw_helper(world_state);
        }
        gui_printf("`^ redo (%d)", elephant_length_redo(&history.snapshotted_world_states));
    }
    if ((history.snapshotted_world_states._redo_stack.length) || (history.snapshotted_world_states._undo_stack.length)) gui_printf("`  SNAPSHOTTED_WORLD_STATES");
    if (history.snapshotted_world_states._undo_stack.length) {
        gui_printf("`v undo (%d)", elephant_length_undo(&history.snapshotted_world_states));
        for (////
                WorldState *world_state = history.snapshotted_world_states._undo_stack.array + (history.snapshotted_world_states._undo_stack.length - 1);
                world_state >= history.snapshotted_world_states._undo_stack.array;
                --world_state
            ) {//
            _history_world_state_draw_helper(world_state);
        }
    }

    gui_printf("");

    if (history.recorded_user_events._redo_stack.length) {
        for (////
                UserEvent *event = history.recorded_user_events._redo_stack.array;
                event < history.recorded_user_events._redo_stack.array + history.recorded_user_events._redo_stack.length;
                ++event
            ) {//
            _history_user_event_draw_helper(event);
        }
        gui_printf("`^ redo (%d)", elephant_length_redo(&history.recorded_user_events));
    }
    if ((history.recorded_user_events._redo_stack.length) || (history.recorded_user_events._undo_stack.length)) {
        gui_printf("`  RECORDED_USER_EVENTS");
    } else {
        gui_printf("`--- no history ---");
    }
    if (history.recorded_user_events._undo_stack.length) {
        gui_printf("`v undo (%d)", elephant_length_undo(&history.recorded_user_events));
        for (////
                UserEvent *event = history.recorded_user_events._undo_stack.array + (history.recorded_user_events._undo_stack.length - 1);
                event >= history.recorded_user_events._undo_stack.array;
                --event
            ) {//
            _history_user_event_draw_helper(event);
        }
    }
}

#endif


//////////////////////////////////////////////////
// PROCESS A FRESH (potentially special) EVENT ///
//////////////////////////////////////////////////

void freshly_baked_event_process(UserEvent freshly_baked_event) {
    auto key_lambda = [freshly_baked_event](uint32 key, bool super = false, bool shift = false) -> bool {
        if (freshly_baked_event.type != USER_EVENT_TYPE_HOTKEY_PRESS) return false;
        return _key_lambda(freshly_baked_event, key, super, shift);
    };

    bool32 undo = (key_lambda('Z', true) || key_lambda('U'));
    bool32 redo = (key_lambda('Y', true) || key_lambda('Z', true, true) || key_lambda('U', false, true));
    if (undo) {
        _global_screen_state.DONT_DRAW_ANY_MORE_POPUPS_THIS_FRAME = true;
        history_undo();
    } else if (redo) {
        _global_screen_state.DONT_DRAW_ANY_MORE_POPUPS_THIS_FRAME = true;
        history_redo();
    } else {
        history_process_and_potentially_record_checkpoint_and_or_snapshot_standard_fresh_user_event(freshly_baked_event);
    }
}

//////////////////////////////////////////////////
// DRAW() ////////////////////////////////////////
//////////////////////////////////////////////////

void conversation_draw() {
    mat4 P_2D = camera_get_P(&_global_screen_state.camera_2D);
    mat4 V_2D = camera_get_V(&_global_screen_state.camera_2D);
    mat4 PV_2D = P_2D * V_2D;
    mat4 M_3D_from_2D = get_M_3D_from_2D();

    bool32 extrude = ((*enter_mode == ENTER_MODE_EXTRUDE_ADD) || (*enter_mode == ENTER_MODE_EXTRUDE_CUT));
    bool32 revolve = ((*enter_mode == ENTER_MODE_REVOLVE_ADD) || (*enter_mode == ENTER_MODE_REVOLVE_CUT));
    bool32 add     = ((*enter_mode == ENTER_MODE_EXTRUDE_ADD) || (*enter_mode == ENTER_MODE_REVOLVE_ADD));
    // bool32 cut     = ((*enter_mode == ENTER_MODE_EXTRUDE_CUT) || (*enter_mode == ENTER_MODE_REVOLVE_CUT));

    // FORNOW: repeated computation; TODO function
    bool32 dxf_anything_selected;
    {
        dxf_anything_selected = false;
        _for_each_selected_entity_ {
            dxf_anything_selected = true;
            break;
        }
    }



    // aliases
    real32 preview_extrude_in_length = (add) ? *extrude_add_in_length : *extrude_cut_in_length;
    real32 preview_extrude_out_length = (add) ? *extrude_add_out_length : *extrude_cut_out_length;
    real32 preview_plane_offset_distance = global_world_state.popup.plane_offset_distance;

    // preview
    vec2 preview_mouse = /*magic_snap*/(mouse_get_position(PV_2D));
    vec2 preview_dxf_origin; {
        if (global_world_state.modes.click_mode != CLICK_MODE_SET_ORIGIN) {
            preview_dxf_origin = global_world_state.dxf.origin;
        } else {
            preview_dxf_origin = preview_mouse;
        }
    }
    vec2 preview_dxf_axis_base_point;
    real32 preview_dxf_axis_angle_from_y;
    {
        if (global_world_state.modes.click_mode != CLICK_MODE_SET_AXIS) {
            preview_dxf_axis_base_point = global_world_state.dxf.axis_base_point;
            preview_dxf_axis_angle_from_y = global_world_state.dxf.axis_angle_from_y;
        } else if (!*awaiting_second_click) {
            preview_dxf_axis_base_point = preview_mouse;
            preview_dxf_axis_angle_from_y = global_world_state.dxf.axis_angle_from_y;
        } else {
            preview_dxf_axis_base_point = *first_click;
            preview_dxf_axis_angle_from_y = atan2(preview_mouse - preview_dxf_axis_base_point) - PI / 2;
        }
    }


    mat4 P_3D = camera_get_P(&_global_screen_state.camera_3D);
    mat4 V_3D = camera_get_V(&_global_screen_state.camera_3D);
    mat4 PV_3D = P_3D * V_3D;

    uint32 window_width, window_height; {
        real32 _window_width, _window_height; // FORNOW
        _window_get_size(&_window_width, &_window_height);
        window_width = (uint32) _window_width;
        window_height = (uint32) _window_height;
    }

    { // panes
        eso_begin(globals.Identity, SOUP_LINES, 5.0f, true);
        eso_color(monokai.gray);
        // if (_global_screen_state.hot_pane == HOT_PANE_2D) {
        //     eso_color(monokai.yellow);
        // } else {
        //     eso_color(1.0f, 0.0f, 1.0f);
        // }
        eso_vertex(0.0f,  1.0f);
        eso_vertex(0.0f, -1.0f);
        eso_end();
    }

    { // draw 2D draw 2d draw
        {
            glEnable(GL_SCISSOR_TEST);
            gl_scissor_TODO_CHECK_ARGS(0, 0, window_width / 2, window_height);
        }

        {
            #if 1
            // TODO: section view
            // mesh_draw(P_2D, V_2D, inverse(get_M_3D_from_2D()));
            #endif
        }

        {
            if (!_global_screen_state.hide_grid) { // grid 2D grid 2d grid
                eso_begin(PV_2D, SOUP_LINES, 2.0f);
                eso_color(80.0f / 255, 80.0f / 255, 80.0f / 255);
                for (uint32 i = 0; i <= uint32(GRID_SIDE_LENGTH / GRID_SPACING); ++i) {
                    real32 tmp = i * GRID_SPACING;
                    eso_vertex(tmp, 0.0f);
                    eso_vertex(tmp, GRID_SIDE_LENGTH);
                    eso_vertex(0.0f, tmp);
                    eso_vertex(GRID_SIDE_LENGTH, tmp);
                }
                eso_end();
                eso_begin(PV_2D, SOUP_LINE_LOOP, 2.0f);
                eso_vertex(0.0f, 0.0f);
                eso_vertex(0.0f, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, 0.0f);
                eso_end();
            }
            { // axes 2D axes 2d axes axis 2D axis 2d axes crosshairs cross hairs origin 2d origin 2D origin
                real32 funky_NDC_factor = _global_screen_state.camera_2D.screen_height_World / 120.0f;
                real32 LL = 1000 * funky_NDC_factor;

                eso_color(monokai.white);
                eso_begin(PV_2D, SOUP_LINES, 3.0f); {
                    // axis
                    vec2 v = LL * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                    eso_vertex(preview_dxf_axis_base_point + v);
                    eso_vertex(preview_dxf_axis_base_point - v);

                    // origin
                    real32 r = funky_NDC_factor;
                    eso_vertex(preview_dxf_origin - V2(r, 0));
                    eso_vertex(preview_dxf_origin + V2(r, 0));
                    eso_vertex(preview_dxf_origin - V2(0, r));
                    eso_vertex(preview_dxf_origin + V2(0, r));
                } eso_end();
            }
            { // entities
                eso_begin(PV_2D, SOUP_LINES);
                _for_each_entity_ {
                    int32 color = (entity->is_selected) ? DXF_COLOR_SELECTION : DXF_COLOR_DONT_OVERRIDE;
                    real32 dx = 0.0f;
                    real32 dy = 0.0f;
                    if ((*click_mode == CLICK_MODE_MOVE_DXF_ENTITIES) && (*awaiting_second_click)) {
                        if (entity->is_selected) {
                            dx = preview_mouse.x - first_click->x;
                            dy = preview_mouse.y - first_click->y;
                            color = DXF_COLOR_WATER_ONLY;
                        }
                    }
                    eso_dxf_entity__SOUP_LINES(entity, color, dx, dy);
                }
                eso_end();
            }
            { // dots
                if (_global_screen_state.show_details) {
                    eso_begin(camera_get_PV(&_global_screen_state.camera_2D), SOUP_POINTS, 4.0f);
                    eso_color(monokai.white);
                    _for_each_entity_ {
                        real32 start_x, start_y, end_x, end_y;
                        entity_get_start_and_end_points(entity, &start_x, &start_y, &end_x, &end_y);
                        eso_vertex(start_x, start_y);
                        eso_vertex(end_x, end_y);
                    }
                    eso_end();
                }
            }

            if (*awaiting_second_click) {
                if (
                        0
                        || (global_world_state.modes.click_modifier == CLICK_MODIFIER_WINDOW)
                        ||(global_world_state.modes.click_mode == CLICK_MODE_CREATE_BOX )
                   ) {
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(omax.cyan);
                    eso_vertex(first_click->x, first_click->y);
                    eso_vertex(preview_mouse.x, first_click->y);
                    eso_vertex(preview_mouse.x, preview_mouse.y);
                    eso_vertex(first_click->x, preview_mouse.y);
                    eso_end();
                }
                if (*click_mode == CLICK_MODE_MEASURE) { // measure line
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(omax.cyan);
                    eso_vertex(*first_click);
                    eso_vertex(preview_mouse);
                    eso_end();
                }
                if (*click_mode == CLICK_MODE_CREATE_LINE) { // measure line
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(omax.cyan);
                    eso_vertex(*first_click);
                    eso_vertex(preview_mouse);
                    eso_end();
                }
                if (*click_mode == CLICK_MODE_CREATE_CIRCLE) {
                    vec2 c = { global_world_state.two_click_command.first_click.x, global_world_state.two_click_command.first_click.y };
                    vec2 p = preview_mouse;
                    real32 r = norm(c - p);
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(omax.cyan);
                    for (uint32 i = 0; i < NUM_SEGMENTS_PER_CIRCLE; ++i) eso_vertex(c + r * e_theta(NUM_DEN(i, NUM_SEGMENTS_PER_CIRCLE) * TAU));
                    eso_end();
                }
                if (*click_mode == CLICK_MODE_CREATE_FILLET) {
                    // FORNOW
                    int i = dxf_find_closest_entity(&global_world_state.dxf.entities, global_world_state.two_click_command.first_click.x, global_world_state.two_click_command.first_click.y);
                    if (i != -1) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(omax.cyan);
                        eso_dxf_entity__SOUP_LINES(&global_world_state.dxf.entities.array[i], DXF_COLOR_WATER_ONLY);
                        eso_end();
                    }
                }
            }
        }
        glDisable(GL_SCISSOR_TEST);
    }

    { // 3D draw 3D 3d draw 3d
        glEnable(GL_SCISSOR_TEST);
        gl_scissor_TODO_CHECK_ARGS(window_width / 2, 0, window_width / 2, window_height);

        { // selection 2d selection 2D selection tube tubes slice slices stack stacks wire wireframe wires frame (FORNOW: ew)
            uint32 color = ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD)) ? DXF_COLOR_TRAVERSE : ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT)) ? DXF_COLOR_QUALITY_1 : ((global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN) || (global_world_state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) ? DXF_COLOR_WATER_ONLY : DXF_COLOR_SELECTION;

            uint32 NUM_TUBE_STACKS_INCLUSIVE;
            mat4 M;
            mat4 M_incr;
            {
                mat4 T_o = M4_Translation(preview_dxf_origin);
                mat4 inv_T_o = M4_Translation(-preview_dxf_origin);
                if (extrude) {
                    real32 a = -preview_extrude_in_length;
                    real32 L = preview_extrude_out_length + preview_extrude_in_length;
                    NUM_TUBE_STACKS_INCLUSIVE = MIN(64, uint32(roundf(L / 2.5f)) + 2);
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0.0f, 0.0f, a + Z_FIGHT_EPS);
                    M_incr = M4_Translation(0.0f, 0.0f, L / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                } else if (revolve) {
                    NUM_TUBE_STACKS_INCLUSIVE = 64;
                    M = M_3D_from_2D * inv_T_o;
                    { // M_incr
                        real32 a = 0.0f;
                        real32 b = TAU;
                        mat4 R_a = M4_RotationAbout(V3(e_theta(PI / 2 + preview_dxf_axis_angle_from_y), 0.0f), (b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                        mat4 T_a = M4_Translation(V3(preview_dxf_axis_base_point, 0.0f));
                        mat4 inv_T_a = inverse(T_a);
                        M_incr = T_o * T_a * R_a * inv_T_a * inv_T_o;
                    }
                } else if (global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D;
                    M_incr = M4_Identity();
                } else if (global_world_state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0.0f, 0.0f, preview_plane_offset_distance + Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else { // default
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0, 0, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                }

                for (uint32 tube_stack_index = 0; tube_stack_index < NUM_TUBE_STACKS_INCLUSIVE; ++tube_stack_index) {
                    eso_begin(PV_3D * M, SOUP_LINES, 5.0f); {
                        _for_each_selected_entity_ {
                            eso_dxf_entity__SOUP_LINES(entity, color);
                        }
                    } eso_end();
                    M *= M_incr;
                }
            }
        }

        BoundingBox draw_bounding_box; {
            BoundingBox _selection_bounding_box = dxf_entities_get_bounding_box(&global_world_state.dxf.entities, true);
            if (dxf_anything_selected) {
                draw_bounding_box = _selection_bounding_box;
                real32 eps = 10.0f;
                draw_bounding_box.min[0] -= eps;
                draw_bounding_box.max[0] += eps;
                draw_bounding_box.min[1] -= eps;
                draw_bounding_box.max[1] += eps;
            } else {
                real32 r = 30.0f;
                draw_bounding_box = { -r, -r, r, r };
            }
        }

        #if 0
        if (dxf_anything_selected) { // arrow
            if ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT)) {

                vec3 color = ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD)) ? V3(83.0f / 255, 255.0f / 255, 83.0f / 255.0f) : V3(1.0f, 0.0f, 0.0f);

                real32 arrow_x = 0.0f;
                real32 arrow_y = 0.0f;
                real32 H[2] = { global_world_state.popup.param0, global_world_state.popup.param1 };
                bool32 toggle[2] = { global_world_state.console.flip_flag, !global_world_state.console.flip_flag };
                mat4 A = M_3D_from_2D;
                if ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                    bounding_box_center(_selection_bounding_box, &arrow_x, &arrow_y);
                    if (dxf_anything_selected) {
                        arrow_x -= global_world_state.dxf.origin.x;
                        arrow_y -= global_world_state.dxf.origin.y;
                    }
                } else { ASSERT((global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT));
                    H[0] = 10.0f + _selection_bounding_box.max[(!global_world_state.console.flip_flag) ? 1 : 0];
                    H[1] = 0.0f;
                    toggle[0] = false;
                    toggle[1] = false;
                    { // A
                        A *= M4_RotationAboutXAxis(RAD(-90.0f));
                        if (global_world_state.console.flip_flag) {
                            A *= M4_RotationAboutYAxis(RAD(90.0f));
                        }
                    }
                }

                for (uint32 i = 0; i < 2; ++i) {
                    if (!IS_ZERO(H[i])) {
                        real32 total_height = ABS(H[i]);
                        real32 cap_height = (total_height > 10.0f) ? 5.0f : (0.5f * total_height);
                        real32 shaft_height = total_height - cap_height;
                        real32 s = 1.5f;
                        mat4 N = M4_Translation(0.0, 0.0, -Z_FIGHT_EPS);
                        if (toggle[i]) N = M4_Scaling(1.0f, 1.0f, -1.0f) * N;
                        mat4 M_cyl  = A * M4_Translation(arrow_x, arrow_y) * N * M4_Scaling(s * 1.0f, s * 1.0f, shaft_height) * M4_RotationAboutXAxis(RAD(90.0f));
                        mat4 M_cone = A * M4_Translation(arrow_x, arrow_y) * N * M4_Translation(0.0f, 0.0f, shaft_height) * M4_Scaling(s * 2.0f, s * 2.0f, cap_height) * M4_RotationAboutXAxis(RAD(90.0f));
                        library.meshes.cylinder.draw(P_3D, V_3D, M_cyl, color);
                        library.meshes.cone.draw(P_3D, V_3D, M_cone, color);
                    }
                }
            }
        }
        #endif

        if (feature_plane->is_active) { // axes 3D axes 3d axes axis 3D axis 3d axis
            real32 r = _global_screen_state.camera_3D.ortho_screen_height_World / 120.0f;
            real32 LL = 100.0f;
            eso_color(monokai.white);
            eso_begin(PV_3D * M_3D_from_2D * M4_Translation(0.0f, 0.0f, Z_FIGHT_EPS), SOUP_LINES, 4.0f);
            eso_vertex(-r, 0.0f);
            eso_vertex( r, 0.0f);
            eso_vertex(0.0f, -r);
            eso_vertex(0.0f,  r);
            // TODO: clip this on the feature_plane
            vec2 v = LL * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
            vec2 a = preview_dxf_axis_base_point + v;
            vec2 b = preview_dxf_axis_base_point - v;
            // vec2 a = bounding_box_clamp(preview_dxf_axis_base_point + v, draw_bounding_box);
            eso_vertex(-preview_dxf_origin + a);
            eso_vertex(-preview_dxf_origin + b); // FORNOW
            eso_end();
        }

        mesh_draw(P_3D, V_3D, M4_Identity());

        if (!_global_screen_state.hide_grid) { // grid 3D grid 3d grid
            conversation_draw_3D_grid_box(P_3D, V_3D);
        }

        if (feature_plane->is_active) { // floating sketch plane; selection plane NOTE: transparent
            bool draw = true;
            mat4 PVM = PV_3D * M_3D_from_2D;
            vec3 color = monokai.yellow;
            real32 sign = -1.0f;
            if (global_world_state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                PVM *= M4_Translation(-global_world_state.dxf.origin.x, -global_world_state.dxf.origin.y, preview_plane_offset_distance);
                color = { 0.0f, 1.0f, 1.0f };
                sign = 1.0f;
                draw = true;
            } else if (global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN) {
                color = { 0.0f, 1.0f, 1.0f };
                sign = 1.0f;
                draw = true;
            } else {
                if (dxf_anything_selected) PVM *= M4_Translation(-global_world_state.dxf.origin.x, -global_world_state.dxf.origin.y, 0.0f); // FORNOW
            }
            if (draw) {
                eso_begin(PVM, SOUP_QUADS);
                eso_color(color, 0.35f);
                eso_vertex(draw_bounding_box.min[0], draw_bounding_box.min[1], sign * Z_FIGHT_EPS);
                eso_vertex(draw_bounding_box.min[0], draw_bounding_box.max[1], sign * Z_FIGHT_EPS);
                eso_vertex(draw_bounding_box.max[0], draw_bounding_box.max[1], sign * Z_FIGHT_EPS);
                eso_vertex(draw_bounding_box.max[0], draw_bounding_box.min[1], sign * Z_FIGHT_EPS);
                eso_end();
            }
        }

        glDisable(GL_SCISSOR_TEST);
    }

    if (!_global_screen_state.hide_gui) { // gui
        gui_printf("[Click] %s %s",
                (*click_mode == CLICK_MODE_NONE) ? "NONE" :
                (*click_mode == CLICK_MODE_SET_ORIGIN) ? "SET_ORIGIN" :
                (*click_mode == CLICK_MODE_SET_AXIS) ? "SET_AXIS" :
                (*click_mode == CLICK_MODE_SELECT) ? "SELECT" :
                (*click_mode == CLICK_MODE_DESELECT) ? "DESELECT" :
                (*click_mode == CLICK_MODE_MEASURE) ? "MEASURE" :
                (*click_mode == CLICK_MODE_CREATE_LINE) ? "CREATE_LINE" :
                (*click_mode == CLICK_MODE_CREATE_BOX) ? "CREATE_BOX" :
                (*click_mode == CLICK_MODE_CREATE_CIRCLE) ? "CREATE_CIRCLE" :
                (*click_mode == CLICK_MODE_CREATE_FILLET) ? "CREATE_FILLET" :
                (*click_mode == CLICK_MODE_MOVE_DXF_ENTITIES) ? "MOVE_DXF_TO" :
                (*click_mode == CLICK_MODE_X_MIRROR) ? "X_MIRROR" :
                (*click_mode == CLICK_MODE_Y_MIRROR) ? "Y_MIRROR" :
                (*click_mode == CLICK_MODE_SET_QUALITY) ? "SET_QUALITY" :
                "???MODE???",
                (*click_modifier == CLICK_MODE_NONE) ? "" :
                (*click_modifier == CLICK_MODIFIER_CONNECTED) ? "CONNECTED" :
                (*click_modifier == CLICK_MODIFIER_WINDOW) ? "WINDOW" :
                (*click_modifier == CLICK_MODIFIER_SNAP_TO_CENTER_OF) ? "CENTER_OF" :
                (*click_modifier == CLICK_MODIFIER_SNAP_TO_END_OF) ? "END_OF" :
                (*click_modifier == CLICK_MODIFIER_SNAP_TO_MIDDLE_OF) ? "MIDDLE_OF" :
                (*click_modifier == CLICK_MODIFIER_QUALITY) ? "QUALITY" :
                (*click_modifier == CLICK_MODIFIER_EXACT_X_Y_COORDINATES) ? "EXACT_X_Y_COORDINATES" :
                (*click_modifier == CLICK_MODIFIER_SELECTED) ? "SELECTED" :
                "???MODIFIER???");

        gui_printf("[Enter] %s",
                (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) ? "EXTRUDE_ADD" :
                (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT) ? "EXTRUDE_CUT" :
                (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD) ? "REVOLVE_ADD" :
                (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_CUT) ? "REVOLVE_CUT" :
                (global_world_state.modes.enter_mode == ENTER_MODE_OPEN) ? "OPEN" :
                (global_world_state.modes.enter_mode == ENTER_MODE_SAVE) ? "SAVE" :
                (global_world_state.modes.enter_mode == ENTER_MODE_SET_ORIGIN) ? "SET_ORIGIN" :
                (global_world_state.modes.enter_mode == ENTER_MODE_OFFSET_PLANE_BY) ? "OFFSET_PLANE_TO" :
                (global_world_state.modes.enter_mode == ENTER_MODE_NONE) ? "NONE" :
                "???ENTER???");

        conversation_message_buffer_update_and_draw();

        if (_global_screen_state.show_details) {
            gui_printf("");
            gui_printf("%d dxf elements   %d stl triangles", global_world_state.dxf.entities.length, global_world_state.mesh.num_triangles);
        }
        if (_global_screen_state.show_help) {
            { // overlay
                eso_begin(M4_Identity(), SOUP_QUADS, 0.0f, true);
                eso_color(0.0f, 0.0f, 0.0f, 0.7f);
                eso_vertex(-1.0f, -1.0f);
                eso_vertex(-1.0f,  1.0f);
                eso_vertex( 1.0f,  1.0f);
                eso_vertex( 1.0f, -1.0f);
                eso_end();
            }
            gui_printf("show/hide-(h)elp");
            gui_printf("(Escape)-from-current-enter_and_modes.click_modes");
            gui_printf("(s)elect (d)eselect + (c)onected (a)ll [Click] / (q)uality + (012345)");
            gui_printf("(y)-cycle-through-top-front-right-planes");
            gui_printf("(e)trude-add (E)xtrude-cut + (0123456789.,) (Tab)-flip-direction [Enter]");
            gui_printf("(Ctrl + z)-undo (Ctrl+Z)-redo (Ctrl+y)-redo");
            gui_printf("(Ctrl + o)pen (Ctrl+s)ave + ... + [Enter]");
            gui_printf("(Ctrl + r)eload-dxf-if-edited-elsewhere (Ctrl + R)-clear-stl");
            gui_printf("show-(g)rid (.)-show-details show-event-stac(k)-for-debugging");
            gui_printf("zoom-to-e(X)tents");
            gui_printf("(0)-toggle-camera-perspective-orthographic");
            gui_printf("(Z)-move-origin + (c)enter-of (e)nd-of (m)iddle-of [Click] / (-0123456789.,) (f)lip-direction [Enter]");
            gui_printf("(M)easure + (c)enter-of (e)nd-of (m)iddle-of (z)-origin [Click]");
            gui_printf("");
            gui_printf("EXPERIMENTAL: (r)evolve-add (R)evolve-cut");
            gui_printf("EXPERIMENTAL: (Ctrl + n)ew-session");
            gui_printf("");
            gui_printf("you can drag and drop *.dxf and *.stl into Conversation");
        }
    }
}

//////////////////////////////////////////////////
// TODO: SPOOF ///////////////////////////////////
//////////////////////////////////////////////////
void todo_spoof() {
    // m2d
    // m3d
    // gui
    // TODOLATER (weird 'X' version): char *string = "^osplash.dxf\nyscx2020\ne\t50";
    #define TAG_LENGTH 3

    // char *string = "^osplash.dxf\nysc{m2d 20 20}{m2d 16 16}{m2d 16 -16}{m2d -16 -16}{m2d -16 16};50\n{m3d 0 100 0 0 -1 0}{m2d 0 17.5}:47\n";
    // char *string = "cz0123456789";
    char *string = "^osplash.dxf\nysc{m2d 20 20}{m2d 16 16}{m2d 16 -16}{m2d -16 -16}{m2d -16 16};50\n{m3d 0 100 0 0 -1 0}{m2d 0 17.5}:47\nc{m2d 16 -16}\t\t100\nsc{m2d 32 -16}{m3d 74 132 113 -0.4 -0.6 -0.7}:60\n^oomax.dxf\nsq0sq1y;3\n";
    #if 0
    _spoof_KEY_event("SQ0");
    _spoof_KEY_event(GLFW_KEY_DELETE);
    _spoof_KEY_event(GLFW_KEY_ESCAPE);
    _spoof_KEY_event('A', false, true);
    _spoof_KEY_event('Z');
    _spoof_MOUSE_2D_event(-5.0f, 10.0f);
    _spoof_KEY_event('S');
    _spoof_KEY_event('C');
    _spoof_MOUSE_2D_event(70.0f, 4.0f);
    _spoof_KEY_event('R');
    _spoof_KEY_event(GLFW_KEY_ENTER);
    // for (uint32 i = 0; i < 1024; ++i) _spoof_KEY_event('U');
    // for (uint32 i = 0; i < 1024; ++i) _spoof_KEY_event('U', false, true);
    #endif
    bool32 super = false;
    for (uint32 i = 0; string[i]; ++i) {
        char c = string[i];
        if (c == '^') {
            super = true;
        } else if (c == '{') {
            UserEvent instabaked_event = {}; {
                uint32 next_i; {
                    next_i = i;
                    while (string[++next_i] != '}') {}
                }
                {
                    char *tag = &string[i + 1];
                    char *params = &string[i + 1 + TAG_LENGTH];
                    if (strncmp(tag, "m2d", TAG_LENGTH) == 0) {
                        instabaked_event.type = USER_EVENT_TYPE_MOUSE_2D_PRESS;
                        sscanf(params, "%f %f", &instabaked_event.mouse.x, &instabaked_event.mouse.y);
                    } else if (strncmp(tag, "m3d", TAG_LENGTH) == 0) {
                        instabaked_event.type = USER_EVENT_TYPE_MOUSE_3D_PRESS;
                        sscanf(params, "%f %f %f %f %f %f", &instabaked_event.o.x, &instabaked_event.o.y, &instabaked_event.o.z, &instabaked_event.dir.x, &instabaked_event.dir.y, &instabaked_event.dir.z);
                    }
                }
                i = next_i;
            }
            freshly_baked_event_process(instabaked_event);
        } else {
            RawUserEvent raw_event = {};
            raw_event.type = RAW_USER_EVENT_TYPE_KEY_PRESS;
            raw_event.super = super;
            {
                if ('a' <= c && c <= 'z') {
                    raw_event.key = 'A' + (c - 'a');
                } else if ('A' <= c && c <= 'Z') {
                    raw_event.shift = true;
                    raw_event.key = c;
                } else if (c == ':') {
                    raw_event.shift = true;
                    raw_event.key = ';';
                } else if (c == '\"') {
                    raw_event.shift = true;
                    raw_event.key = '\'';
                } else if (c == '\n') {
                    raw_event.key = GLFW_KEY_ENTER;
                } else if (c == '\t') {
                    raw_event.key = GLFW_KEY_TAB;
                } else {
                    raw_event.key = c;
                }
            }
            super = false;
            UserEvent freshly_baked_event = bake_user_event(raw_event);
            freshly_baked_event_process(freshly_baked_event);
        }
    }
}

//////////////////////////////////////////////////
// MAIN() ////////////////////////////////////////
//////////////////////////////////////////////////

// TODO: load is no longer special; we should be able to undo / redo across it just like anything else
// TODO: don't worry about user experience yet (it will grow to include panels toolbox etc)

#if 0

void _spoof_KEY_event(uint32 key, bool32 super = false, bool32 shift = false) {
    queue_enqueue(&queue_of_fresh_events_from_user, KEY_event(key, super, shift));
}

void _spoof_MOUSE_2D_event(real32 mouse_x, real32 mouse_y) {
    queue_enqueue(&queue_of_fresh_events_from_user, MOUSE_2D_event(mouse_x, mouse_y));
}

void _spoof_MOUSE_3D_event(real32 o_x, real32 o_y, real32 o_z, real32 dir_x, real32 dir_y, real32 dir_z) {
    queue_enqueue(&queue_of_fresh_events_from_user, MOUSE_3D_event(o_x, o_y, o_z, dir_x, dir_y, dir_z));
}

void _spoof_KEY_event(char *string) {
    for (uint32 i = 0; i < string[i]; ++i) {
        char key = string[i];
        if (('a' <= key) && (key <= 'z')) key = 'A' + (key - 'a');
        _spoof_KEY_event(key);
    }
}

void INIT_SPOOF_CHAIN_END_TO_END_TEST() {
    // TODO: key event string that can handle shift and super ~^
    if (1) {
        if (1) {
            // _spoof_KEY_event("CZ100"); return;
            _spoof_KEY_event('O', true);
            _spoof_KEY_event("splash.dxf");
            return;
            _spoof_KEY_event(GLFW_KEY_ENTER);
            if (1) {
                _spoof_KEY_event('Y');
                _spoof_KEY_event('S');
                _spoof_KEY_event('C');
                _spoof_MOUSE_2D_event( 20.0,  20.0);
                _spoof_MOUSE_2D_event( 16.0,  16.0);
                _spoof_MOUSE_2D_event( 16.0, -16.0);
                _spoof_MOUSE_2D_event(-16.0, -16.0);
                _spoof_MOUSE_2D_event(-16.0,  16.0);
                _spoof_KEY_event('E');
                _spoof_KEY_event('5');
                _spoof_KEY_event('0');
                _spoof_KEY_event(GLFW_KEY_ENTER); // FORNOW
                _spoof_MOUSE_3D_event(50.0f, 93.0f, 76.0f, -0.47f, -0.43f, -0.77f);
                _spoof_MOUSE_2D_event(16.4f, -9.5f);
                _spoof_KEY_event('E', false, true);
                _spoof_KEY_event('4');
                _spoof_KEY_event('7');
                _spoof_KEY_event(GLFW_KEY_ENTER); // FORNOW
                _spoof_KEY_event('C');
                _spoof_MOUSE_2D_event(16.0f, -16.0f);
                _spoof_KEY_event(GLFW_KEY_TAB);
                _spoof_KEY_event(GLFW_KEY_TAB);
                _spoof_KEY_event("100");
                _spoof_KEY_event(GLFW_KEY_ENTER);
                _spoof_KEY_event('S');
                _spoof_KEY_event('C');
                _spoof_MOUSE_2D_event(32.0f, -16.0f);
                _spoof_MOUSE_3D_event(74.0f, 132.0f, 113.0f, -0.4f, -0.6f, -0.7f);
                _spoof_KEY_event('E', false, true);
                _spoof_KEY_event('6');
                _spoof_KEY_event('0');
                _spoof_KEY_event(GLFW_KEY_ENTER);
                _spoof_KEY_event('O', true);
                _spoof_KEY_event("omax.dxf");
                _spoof_KEY_event(GLFW_KEY_ENTER);
                _spoof_KEY_event("SQ1YE3");
                _spoof_KEY_event(GLFW_KEY_ENTER);
                _spoof_KEY_event("SQ0");
                _spoof_KEY_event(GLFW_KEY_DELETE);
                _spoof_KEY_event(GLFW_KEY_ESCAPE);
                //_spoof_KEY_event('C');
                //_spoof_KEY_event('Z');
                //_spoof_KEY_event('0');
                //_spoof_KEY_event('1');
                //_spoof_KEY_event('2');
                //_spoof_KEY_event('3');
                //_spoof_KEY_event('4');
                //_spoof_KEY_event('5');
                //_spoof_KEY_event('6');
                //_spoof_KEY_event('7');
                //_spoof_KEY_event('8');
                //_spoof_KEY_event('9');
                _spoof_KEY_event('A', false, true);
                _spoof_KEY_event('Z');
                _spoof_MOUSE_2D_event(-5.0f, 10.0f);
                _spoof_KEY_event('S');
                _spoof_KEY_event('C');
                _spoof_MOUSE_2D_event(70.0f, 4.0f);
                _spoof_KEY_event('R');
                _spoof_KEY_event(GLFW_KEY_ENTER);
                // for (uint32 i = 0; i < 1024; ++i) _spoof_KEY_event('U');
                // for (uint32 i = 0; i < 1024; ++i) _spoof_KEY_event('U', false, true);

                //spoof_KEY_event('U');
                //spoof_KEY_event('U');
                //spoof_KEY_event('U');
                //spoof_KEY_event('U');
                //spoof_KEY_event('U', false, true);
                //spoof_KEY_event('U', false, true);
                //spoof_KEY_event('U', false, true);
                //spoof_KEY_event('U', false, true);
            }
        } else {
            _spoof_KEY_event('O', true);
            _spoof_KEY_event("fidget.dxf");
            _spoof_KEY_event(GLFW_KEY_ENTER);
            _spoof_KEY_event('L');
            _spoof_KEY_event('Z');
        }
        #if 0 // TODO: update to use spoof api
        else if (1) {
            conversation_dxf_load("omax.dxf");
            if (1) {
                UserEvent event = {};
                event.type = USER_EVENT_TYPE_HOTKEY_PRESS;
                for (int i = 0; i < 5; ++i) {
                    spoof_KEY_event('Y');
                    spoof_KEY_event('S');
                    spoof_KEY_event('Q');
                    spoof_KEY_event('0' + i);
                    spoof_KEY_event('E');
                    spoof_KEY_event('1' + i);
                    spoof_KEY_event(GLFW_KEY_ENTER);
                }
            }
        } else if (0) {
            conversation_dxf_load("ik.dxf");
        } else if (0) {
            conversation_dxf_load("debug.dxf");
        }
        #endif
    }
    // conversation_messagef("type h for help `// pre-alpha " __DATE__ " " __TIME__);
}

#endif

int main() {
    // _window_set_size(1.5 * 640.0, 1.5 * 360.0); // TODO the first frame crap gets buggered

    glfwHideWindow(COW0._window_glfw_window);

    // INIT_SPOOF_CHAIN_END_TO_END_TEST();
    todo_spoof();
    init_cameras(); // FORNOW

    {
        // FORNOW: first frame position
        double xpos, ypos;
        glfwGetCursorPos(COW0._window_glfw_window, &xpos, &ypos);
        callback_cursor_position(NULL, xpos, ypos);
    }

    bool32 frame_0 = true;
    bool32 frame_1 = false;
    while (cow_begin_frame()) {
        _global_screen_state.DONT_DRAW_ANY_MORE_POPUPS_THIS_FRAME = false;

        // conversation_messagef("%lf", global_world_state.popup.circle_diameter);
        // Sleep(100);

        { // animations
            _global_screen_state.popup_blinker_time += 0.0167f;
            _global_screen_state.successful_feature_time += 0.03;
            _global_screen_state.plane_selection_time += 0.03;
            // _global_screen_state.successful_feature_time = 1.0f;

            _global_screen_state.going_inside_time += 0.0167;
            bool32 going_inside_next = 
                ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) && (global_world_state.popup.extrude_add_in_length > 0.0f))
                ||
                (global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_CUT);
            if (going_inside_next && !_global_screen_state.going_inside) {
                _global_screen_state.going_inside_time = 0.0f;
            }
            _global_screen_state.going_inside= going_inside_next;
        }

        #if 0
        { // camera_move, hot_pane
            { // camera_move (using shimmed globals.* global_world_state)
                if (_global_screen_state.hot_pane == HOT_PANE_2D) {
                    camera_move(&_global_screen_state.camera_2D);
                } else if (_global_screen_state.hot_pane == HOT_PANE_3D) {
                    camera_move(&_global_screen_state.camera_3D);
                }
            }

            if ((!globals.mouse_left_held && !globals.mouse_right_held) || globals.mouse_left_pressed || globals.mouse_right_pressed) {
                _global_screen_state.hot_pane = (_global_screen_state.mouse_in_pixel_coordinates.x <= window_get_width() / 2) ? HOT_PANE_2D : HOT_PANE_3D;

                if ((global_world_state.modes.click_modifier == CLICK_MODIFIER_WINDOW) && (*awaiting_second_click)) _global_screen_state.hot_pane = HOT_PANE_2D;// FORNOW
            }
        }
        #endif

        { // queue_of_fresh_events_from_user
          // TODO: upgrade to handle multiple events per frame while only drawing gui once (simple simple with a boolean here -- reusing boolean is sus)
            if (raw_user_event_queue.length) {
                while (raw_user_event_queue.length) {
                    RawUserEvent raw_event = queue_dequeue(&raw_user_event_queue);
                    UserEvent freshly_baked_event = bake_user_event(raw_event);
                    freshly_baked_event_process(freshly_baked_event);
                }
            } else {
                // NOTE: this is so we draw the popups
                UserEvent null_event = {};
                null_event.type = USER_EVENT_TYPE_NONE;
                freshly_baked_event_process(null_event);
            }
        }

        conversation_draw(); // FORNOW: moving this down here (be wary of frame imperfections)

        { // frame_0, frame_1
            if (frame_1) glfwShowWindow(COW0._window_glfw_window);
            if (frame_0) {
                frame_0 = false;
                frame_1 = true;
            } else if (frame_1) {
                frame_1 = false;
            }
        }

        //

        if (_global_screen_state.show_event_stack) history_debug_draw();
    }
}


