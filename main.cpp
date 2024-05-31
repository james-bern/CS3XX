// BUG: SEGFAULT when messing around with pausing (TODO go back and check all the vec3 int3 stuff on mesh)

// TODO: saving dxf
// TODO: confirm overwrite saving stl / dxf

// TODO: formalize timestep (support variable display refresh rate)
// XXX: 'P' to pause
// TODO: rename WORLD, SCREEN
// XXX: rename NDC_from_Screen -> NDC_from_Pixel
// XXX: rename Identity
// XXX: only use C++-style casts 
// XXX: switch to regular old bool
// XXX: mesh -> vec3's

// XXX: enums
// XXX: color for messagef
// TODO: simplify selected triangles color (just use JUICEIT_EASYTWEEN)

// TODO: select connected broken on 10mm filleted 50mm box
// TODO: restore checkpoint one before on extrudes?

// TODO: something is slow now (3d picking

// TODO: overwrite the tween (bool?) when clicking the mouse

// TODO: fix up the dragging of the mouse while grabbing the popup

// XXX: color -> color_code
// TODO: programmatic toggle on Sleep


// TODO: update help

// XXX: selection timer on dxf entities so they can flash white
// TODOLATER: entities transparency appear and disappear better

// TODO: printing program to console (storing program as a string)
// TODO: drag and drop of program
// TODO: how are we going to distribute files

// TODO: cow upgrades

// TODO: follow 2-3 hours of autocad tutorials


// TODO: floating sketch plane should tween in size
// TODO: initial plane should be scaled to match the size of the selected face 

//////////////////////////////////////////////////
// DEBUG /////////////////////////////////////////
//////////////////////////////////////////////////

// #define DEBUG_HISTORY_DISABLE_HISTORY_ENTIRELY
// #define DEBUG_HISTORY_DISABLE_SNAPSHOTTING
// #define DEBUG_DISABLE_EASY_TWEEN

//////////////////////////////////////////////////
// NOTES /////////////////////////////////////////
//////////////////////////////////////////////////

// // things we are always unwilling to compromise on
// frame-perfect UI / graphics (except first frame FORNOW)
// ability to churn through a bunch of events in a single frame (either by user interacting really fast, spoofing, or undo/redo-ing)

// // things we are almost always unwilling to compromise on
// zero is initialization
// keep the tool (line_entity, circle, box, move, ...) code simple, formulaic, and not-clever
// - repetition is fine
// we write things ourselves
// - the primary reason to do this is "we can do better"
// - also it's the only way to really understand the system

// // things i feel weird/uncomfy about
// i have a type variable and substructs in Entity and Event so i don't get confused; ryan fleury doesn't like this but i sure seem to (shrug)
// i have some repetition (grug say hmmm...) HotkeyUserEvent and GUIKeyUserEvent are the same thing... popup MOVE and popup LINE are the same thing...

// // misc
// if order doesn't matter, go with alphabetical

// // thing we aspire for Conversation to be
// "juicy"
// - watch this: https://www.youtube.com/watch?v=Fy0aCDmgnxg
// - watch this: https://www.youtube.com/watch?v=AJdEqssNZ-U

// // vocab
// an entity is a line_entity or arc_entity in a dxf


// BUG: clicking with sc doesn't do anything sometimes
// BUG: sw broken for very short arcs of huge circles


// // TODO: jim tasks
// cow
// TODO: better text_draw
// TODO: Consolas font
// TODO: upgrade soup_draw and eso_*; redocument
// TODO: upgrade basic_draw
// TODO: gui printf that doesn't automatically add a newline
// TODO: check whether everything broken on windows
// other
// XXX: " " that just replicates LAYOUT's funcionality
// XXX: "SHIFT+ " could be for enter mode
// TODO#define DEBUG_HISTORY_RECORD_EVERYTHING
// TODO#define DEBUG_HISTORY_CHECKPOINT_EVERYTHING
// TODO: remove all usage of gui-printf (after folding in cow)
// NOTE multiple popups is a really easy situation to find self in (start drawing a line_entity then extrude; this seems like a job for much later
// XXX: dragging hot pane divider
// TODO: bring cameras into app
// TODO: possible bug: GUI_MOUSE getting triggered when it shouldn't
// // TODO: consider recording an event based on whether anything in the state changed
//          (this would remove the need to do all the dirty checks)
//          and you would only need to do it for new events
// NOTE: this is a good idea
// the recursive crap in _standard_event_process_NOTE_RECURSIVE you're doing now could be a problem
// (another layer atop it would probably do the trick -- _original_call_standard_event_process)
// TODO: ghostly section view on 2D side
// TODOLATER repeated keys shouldn't be recorded (unless enter mode changes) -- enter mode lambda in process
// TODOLATER: don't record saving
// NONO: switch key_lambda over to strings like the scripts "a" "A" "^a" "^A"
// TODO if (awaiting_second_click) // BAD VERY BAD NOW
// TODO: really formalize char_equivalent
// XXX: figure out system for space bar
// TODO: merge a copy of cow into the app (split off into own repo with a cow inside it)



// // TODO: good undergrad tasks
// TODO: fast picking (something like a BVH)--need arena (mesh should have its own arena)
// TODO: popup_popup real popup automatically parse formulas
// TODO: ROTATE
// TODO: COPY
// TODO: SCALE
// TODO: OFFSET
// TODO: shell (OFFSET CONNECTED)
// TODO: FILLET CONNECTED (super fillet 'F')
// TODO: perpendicular snap
// TODO: quad snap
// TODO: LAYOUT polygon
// TODO: add LAYOUT's two click mirror
// TODO: being able to type equations into boxes
// TODO: holding shift to make box a square
// TODO: ERASE (including drawing the eraser circle in pixel coordinates
// TODO: three click circle
// TODO: elipse
// TODO: fillet line_entity-arc_entity
// TODO: fillet arc_entity-arc_entity
// TODO: power intersect
// TODO: 1 click intersect
// TODO: 2 click intersect
// TODO: power intersect
// NONO: colors for messagef (warning, error, info, success)
// // TODO: toolbox you can click on that copies over a sketch (like the counter sunk nuts)

// // TODO: big tassks
// TODO: somehow, this should be parsed out of the DXF
// TODO: ability to scroll through toolbox with mouse

#include "include.cpp"
#include "manifoldc.h"
#include "jim.cpp"
#include "box.cpp"
#include "burkardt.cpp"
#include "header.cpp"
#include "elephant.cpp"

//////////////////////////////////////////////////
// SCARY MACROS //////////////////////////////////
//////////////////////////////////////////////////

#define _for_each_entity_ for (\
        Entity *entity = WORLD.dxf.entities.array;\
        entity < &WORLD.dxf.entities.array[WORLD.dxf.entities.length];\
        ++entity)

#define _for_each_selected_entity_ _for_each_entity_ if (entity->is_selected)

//////////////////////////////////////////////////
// GLOBAL STATE //////////////////////////////////
//////////////////////////////////////////////////

WorldState WORLD;
ScreenState SCREEN;
AestheticsState AESTHETICS;

// short-name pointers
Camera2D *camera_2D = &SCREEN.camera_2D;
Camera3D *camera_3D = &SCREEN.camera_3D;
ClickMode *click_mode = &WORLD.click_mode;
ClickModifier *click_modifier = &WORLD.click_modifier;
ColorCode *click_color_code = &WORLD.click_color_code;
DXFState *dxf = &WORLD.dxf;
EnterMode *enter_mode = &WORLD.enter_mode;
Event *shift_space_bar_event = &WORLD.shift_space_bar_event;
Event *space_bar_event = &WORLD.space_bar_event;
FeaturePlaneState *feature_plane = &WORLD.feature_plane;
Mesh *mesh = &WORLD.mesh;
Pane *hot_pane = &SCREEN.hot_pane;
Pane *mouse_left_drag_pane = &SCREEN.mouse_left_drag_pane;
Pane *mouse_right_drag_pane = &SCREEN.mouse_right_drag_pane;
PopupState *popup = &WORLD.popup;
TimeSince *time_since = &AESTHETICS.time_since;
bool *awaiting_second_click = &WORLD.two_click_command.awaiting_second_click;
bool *paused = &SCREEN.paused;
box2 *preview_feature_plane = &AESTHETICS.preview_feature_plane;
char *open_filename = popup->open_filename;
char *save_filename = popup->open_filename;
mat4 *transform_NDC_from_Pixel = &SCREEN.transform_NDC_from_Pixel;
real *box_height = &popup->box_height;
real *box_width = &popup->box_width;
real *circle_circumference = &popup->circle_circumference;
real *circle_diameter = &popup->circle_diameter;
real *circle_radius = &popup->circle_radius;
real *extrude_add_in_length = &popup->extrude_add_in_length;
real *extrude_add_out_length = &popup->extrude_add_out_length;
real *extrude_cut_in_length = &popup->extrude_cut_in_length;
real *extrude_cut_out_length = &popup->extrude_cut_out_length;
real *feature_plane_nudge = &popup->feature_plane_nudge;
real *fillet_radius = &popup->fillet_radius;
real *line_angle = &popup->line_angle;
real *line_length = &popup->line_length;
real *line_rise = &popup->line_rise;
real *line_run = &popup->line_run;
real *move_angle = &popup->move_angle;
real *move_length = &popup->move_length;
real *move_rise = &popup->move_rise;
real *move_run = &popup->move_run;
real *preview_extrude_in_length = &AESTHETICS.preview_extrude_in_length;
real *preview_extrude_out_length = &AESTHETICS.preview_extrude_out_length;
real *preview_feature_plane_offset = &AESTHETICS.preview_feature_plane_offset;
real *revolve_add_dummy = &popup->revolve_add_dummy;
real *revolve_cut_dummy = &popup->revolve_cut_dummy;
real *x_coordinate = &popup->x_coordinate;
real *y_coordinate = &popup->y_coordinate;
vec2 *first_click = &WORLD.two_click_command.first_click;
vec2 *mouse_NDC = &SCREEN.mouse_NDC;
vec2 *mouse_Pixel = &SCREEN.mouse_Pixel;
vec3 *preview_feature_plane_color = &AESTHETICS.preview_feature_plane_color;
vec3 *preview_tubes_color = &AESTHETICS.preview_tubes_color;

template <typename T> void JUICEIT_EASYTWEEN(T *a, T b) {
    real32 f = 0.1f;
    if (!SCREEN.paused) *a += f * (b - *a);
}

real _JUICEIT_EASYTWEEN(real t) {
    return 0.287f * log(t) + 1.172f;
}

//////////////////////////////////////////////////
// NON-ZERO INITIALIZERS /////////////////////////
//////////////////////////////////////////////////

void init_cameras() {
    SCREEN.camera_2D = { 100.0f, 0.0, 0.0f, -0.5f, -0.125f };
    // FORNOW
    if (dxf->entities.length) camera2D_zoom_to_bounding_box(&SCREEN.camera_2D, dxf_entities_get_bounding_box(&WORLD.dxf.entities));
    if ((!SCREEN.camera_3D.persp_distance_to_origin) || (!mesh->num_vertices)) {
        SCREEN.camera_3D = { 2.0f * MIN(150.0f, SCREEN.camera_2D.height_World), CAMERA_3D_DEFAULT_ANGLE_OF_VIEW, RAD(33.0f), RAD(-44.0f), 0.0f, 0.0f, 0.5f, -0.125f };
    }
}

//////////////////////////////////////////////////
// GETTERS (STATE NOT WORTH TROUBLE OF STORING) //
//////////////////////////////////////////////////
// getters

real get_x_divider_Pixel() {
    return LINEAR_REMAP(SCREEN.x_divider_NDC, -1.0f, 1.0f, 0.0f, window_get_width());
}

mat4 get_M_3D_from_2D() {
    vec3 up = { 0.0f, 1.0f, 0.0f };
    real dot_product = dot(feature_plane->normal, up);
    vec3 y = (ARE_EQUAL(ABS(dot_product), 1.0f)) ? V3(0.0f,  0.0f, -1.0f * SGN(dot_product)) : up;
    vec3 x = normalized(cross(y, feature_plane->normal));
    vec3 z = cross(x, y);

    // FORNOW
    if (ARE_EQUAL(ABS(dot_product), 1.0f) && SGN(dot_product) < 0.0f) {
        y *= -1;
    }

    return M4_xyzo(x, y, z, (feature_plane->signed_distance_to_world_origin) * feature_plane->normal);
}

// fornow

bool click_mode_SELECT_OR_DESELECT() {
    return ((*click_mode == ClickMode::Select) || (*click_mode == ClickMode::Deselect));
}

bool _non_WINDOW__SELECT_DESELECT___OR___SET_COLOR() {
    return ((click_mode_SELECT_OR_DESELECT() && (*click_modifier != ClickModifier::Window)) || (*click_mode == ClickMode::Color));
}

bool _SELECT_OR_DESELECT_COLOR() {
    bool A = click_mode_SELECT_OR_DESELECT();
    bool B = (*click_modifier == ClickModifier::Color);
    return A && B;
}

bool click_mode_SNAP_ELIGIBLE() {
    return 0
        || (*click_mode == ClickMode::Axis)
        || (*click_mode == ClickMode::Box)
        || (*click_mode == ClickMode::Circle)
        || (*click_mode == ClickMode::Line)
        || (*click_mode == ClickMode::Measure)
        || (*click_mode == ClickMode::Move)
        || (*click_mode == ClickMode::Origin)
        || (*click_mode == ClickMode::MirrorX)
        || (*click_mode == ClickMode::MirrorY)
        ;
}

bool click_mode_SPACE_BAR_REPEAT_ELIGIBLE() {
    return 0
        || (*click_mode == ClickMode::Axis)
        || (*click_mode == ClickMode::Box)
        || (*click_mode == ClickMode::Circle)
        || (*click_mode == ClickMode::Fillet)
        || (*click_mode == ClickMode::Line)
        || (*click_mode == ClickMode::Measure)
        || (*click_mode == ClickMode::Move)
        || (*click_mode == ClickMode::Origin)
        || (*click_mode == ClickMode::MirrorX)
        || (*click_mode == ClickMode::MirrorY)
        ;
}

bool enter_mode_SHIFT_SPACE_BAR_REPEAT_ELIGIBLE() {
    return 0
        || (*enter_mode == EnterMode::ExtrudeAdd)
        || (*enter_mode == EnterMode::ExtrudeCut)
        || (*enter_mode == EnterMode::RevolveAdd)
        || (*enter_mode == EnterMode::RevolveCut)
        || (*enter_mode == EnterMode::NudgeFeaturePlane)
        ;
}
//////////////////////////////////////////////////
// STATE-DEPENDENT UTILITY FUNCTIONS /////////////
//////////////////////////////////////////////////

vec2 magic_snap(vec2 before, bool calling_this_function_for_drawing_preview = false) {
    vec2 result = before;
    {
        if (
                ( 0 
                  || (*click_mode == ClickMode::Line)
                  || (*click_mode == ClickMode::Axis)
                )
                && (*awaiting_second_click)
                && (SCREEN.shift_held)) {
            vec2 a = WORLD.two_click_command.first_click;
            vec2 b = before;
            vec2 r = b - a; 
            real norm_r = norm(r);
            real factor = 360 / 15 / TAU;
            real theta = roundf(atan2(r) * factor) / factor;
            result = a + norm_r * e_theta(theta);
        } else if (
                (*click_mode == ClickMode::Box)
                && (*awaiting_second_click)
                && (SCREEN.shift_held)) {
            // TODO (Felipe): snap square
            result = before;
        } else if (!calling_this_function_for_drawing_preview) { // NOTE: this else does, in fact, match LAYOUT's behavior
            if (*click_modifier == ClickModifier::Center) {
                real min_squared_distance = HUGE_VAL;
                _for_each_entity_ {
                    if (entity->type == EntityType::Line) {
                        continue;
                    } else { ASSERT(entity->type == EntityType::Arc);
                        ArcEntity *arc_entity = &entity->arc_entity;
                        real squared_distance = squared_distance_point_dxf_arc(before.x, before.y, arc_entity);
                        if (squared_distance < min_squared_distance) {
                            min_squared_distance = squared_distance;
                            result = arc_entity->center;
                        }
                    }
                }
            } else if (*click_modifier == ClickModifier::Middle) {
                real min_squared_distance = HUGE_VAL;
                _for_each_entity_ {
                    real squared_distance = squared_distance_point_dxf_entity(before.x, before.y, entity);
                    if (squared_distance < min_squared_distance) {
                        min_squared_distance = squared_distance;
                        entity_get_middle(entity, &result.x, &result.y);
                    }
                }
            } else if (*click_modifier == ClickModifier::End) {
                real min_squared_distance = HUGE_VAL;
                _for_each_entity_ {
                    real x[2], y[2];
                    entity_get_start_and_end_points(entity, &x[0], &y[0], &x[1], &y[1]);
                    for (uint d = 0; d < 2; ++d) {
                        real squared_distance = squared_distance_point_point(before.x, before.y, x[d], y[d]);
                        if (squared_distance < min_squared_distance) {
                            min_squared_distance = squared_distance;
                            result.x = x[d];
                            result.y = y[d];
                        }
                    }
                }
            }
        }
    }
    return result;
}

box2 mesh_draw(mat4 P_3D, mat4 V_3D, mat4 M_3D) {
    box2 face_selection_bounding_box = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
    mat4 inv_M_3D_from_2D = inverse(get_M_3D_from_2D());

    mat4 PVM_3D = P_3D * V_3D * M_3D;

    if (mesh->cosmetic_edges) {
        eso_begin(PVM_3D, SOUP_LINES); 
        // eso_color(CLAMPED_LERP(2 * time_since->successful_feature, monokai.white, monokai.black));
        eso_color(monokai.black);
        // 3 * num_triangles * 2 / 2
        for (uint i = 0; i < mesh->num_cosmetic_edges; ++i) {
            for (uint d = 0; d < 2; ++d ) {
                eso_vertex(mesh->vertex_positions[mesh->cosmetic_edges[i][d]]);
            }
        }
        eso_end();
    }
    for (uint pass = 0; pass < 2; ++pass) {
        eso_begin(PVM_3D, (!SCREEN.show_details) ? SOUP_TRIANGLES : SOUP_OUTLINED_TRIANGLES);

        mat3 inv_transpose_V_3D = inverse(transpose(M3(V_3D(0, 0), V_3D(0, 1), V_3D(0, 2), V_3D(1, 0), V_3D(1, 1), V_3D(1, 2), V_3D(2, 0), V_3D(2, 1), V_3D(2, 2))));


        for (uint i = 0; i < mesh->num_triangles; ++i) {
            vec3 n = mesh->triangle_normals[i];
            vec3 p[3];
            real x_n;
            {
                for (uint j = 0; j < 3; ++j) p[j] = mesh->vertex_positions[mesh->triangle_indices[i][j]];
                x_n = dot(n, p[0]);
            }
            vec3 color; 
            real alpha;
            {
                vec3 n_Camera = inv_transpose_V_3D * n;
                vec3 color_n = V3(0.5f + 0.5f * n_Camera.x, 0.5f + 0.5f * n_Camera.y, 1.0f);
                if ((feature_plane->is_active) && (dot(n, feature_plane->normal) > 0.99f) && (ABS(x_n - feature_plane->signed_distance_to_world_origin) < 0.01f)) {
                    if (pass == 0) continue;
                    color = CLAMPED_LERP(_JUICEIT_EASYTWEEN(time_since->plane_selected), color_n, 0.9f * omax.yellow);// CLAMPED_LERP(2.0f * time_since->plane_selected - 0.5f, monokai.yellow, V3(0.85f, 0.87f, 0.30f));
                                                                                                                      // if (2.0f * time_since->plane_selected < 0.3f) color = monokai.white;
                    alpha = CLAMPED_LERP(2.0f * time_since->plane_selected, 1.0f, time_since->_helper_going_inside ? 
                            CLAMPED_LERP(time_since->going_inside, 1.0f, 0.7f)
                            : 1.0f);// ? 0.7f : 1.0f;


                    bounding_box_add_point(&face_selection_bounding_box, _V2(transformPoint(inv_M_3D_from_2D, p[0])));
                    bounding_box_add_point(&face_selection_bounding_box, _V2(transformPoint(inv_M_3D_from_2D, p[1])));
                    bounding_box_add_point(&face_selection_bounding_box, _V2(transformPoint(inv_M_3D_from_2D, p[2])));


                } else {
                    if (pass == 1) continue;
                    color = color_n;
                    alpha = 1.0f;
                }
            }
            real mask = CLAMP(1.2f * time_since->successful_feature, 0.0f, 2.0f);
            // color = CLAMPED_LINEAR_REMAP(time_since->successful_feature, -0.5f, 0.5f, omax.white, color);
            eso_color(color, alpha);
            for (uint d = 0; d < 3; ++d ) {
                eso_color(CLAMPED_LERP(mask + sin(CLAMPED_INVERSE_LERP(p[d].y, mesh->bounding_box.max.y, mesh->bounding_box.min.y) + 0.5f * time_since->successful_feature), monokai.white, color), alpha);
                eso_vertex(p[d]);
            }
        }
        eso_end();
    }

    return face_selection_bounding_box;
}

//////////////////////////////////////////////////
// LOADING AND SAVING STATE TO/FROM DISK /////////
//////////////////////////////////////////////////

void conversation_dxf_load(char *filename, bool preserve_cameras_and_dxf_origin = false) {
    if (!poe_file_exists(filename)) {
        conversation_messagef(omax.orange, "File \"%s\" was not found.", filename);
        return;
    }

    list_free_AND_zero(&WORLD.dxf.entities);

    dxf_entities_load(filename, &WORLD.dxf.entities);

    if (!preserve_cameras_and_dxf_origin) {
        // camera2D_zoom_to_bounding_box(&SCREEN.camera_2D, dxf_entities_get_bounding_box(&WORLD.dxf.entities));
        init_cameras();
        WORLD.dxf.origin.x = 0.0f;
        WORLD.dxf.origin.y = 0.0f;
    }

    conversation_messagef(omax.green, "Loaded file \"%s.\"", filename);
}

void conversation_stl_load(char *filename, bool preserve_cameras = false) {
    if (!poe_file_exists(filename)) {
        conversation_messagef(omax.orange, "File \"%s\" was not found.", filename);
        return;
    }
    {
        stl_load(filename, &WORLD.mesh);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(preserve_cameras);
        conversation_messagef(omax.green, "Loaded file \"%s.\"", filename);
    }
}

void conversation_load(char *filename, bool preserve_cameras = false) {
    if (poe_suffix_match(filename, ".dxf")) {
        conversation_dxf_load(filename, preserve_cameras);
    } else if (poe_suffix_match(filename, ".stl")) {
        conversation_stl_load(filename, preserve_cameras);
    } else {
        conversation_messagef(omax.orange, "File \"%s\" is not supported: must be *.dxf or *.stl. ", filename);
    }
}

void conversation_stl_save(char *filename) {
    // TODO: prompt for overwriting
    if (mesh_save_stl(&WORLD.mesh, filename) ) {
        conversation_messagef(omax.green, "Saved file \"%s.\"", filename);
    } else {
        conversation_messagef(omax.orange, "Failed to open file \"%s\" for writing.", filename);
    }
}

void conversation_save(char *filename) {
    if (poe_suffix_match(filename, ".stl")) {
        conversation_stl_save(filename);
    } else {
        conversation_messagef(omax.orange, "File \"%s\" is not supported: must be *.dxf or *.stl. ", filename);
    }
}

void drop_callback(GLFWwindow *, int count, const char **paths) {
    if (count > 0) {
        char *filename = (char *)(paths[0]);
        conversation_load(filename);
        { // conversation_set_drop_path(filename)
            for (uint i = 0; filename[i]; ++i) {
                SCREEN.drop_path[i] = filename[i];
                if (filename[i] == '.') {
                    while (
                            (i != 0) &&
                            (SCREEN.drop_path[i - 1] != '\\') &&
                            (SCREEN.drop_path[i - 1] != '/')
                          ) --i;
                    SCREEN.drop_path[i] = '\0';
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

Queue<RawEvent> raw_user_event_queue;

void callback_key(GLFWwindow *, int key, int, int action, int mods) {
    bool control = (mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER));
    bool shift = (mods & GLFW_MOD_SHIFT);

    if (key == GLFW_KEY_LEFT_SHIFT) {
        if (action == GLFW_PRESS) {
            SCREEN.shift_held = true;
        } else if (action == GLFW_RELEASE) {
            SCREEN.shift_held = false;
        }
    }
    if (key == GLFW_KEY_LEFT_SHIFT) return;
    if (key == GLFW_KEY_RIGHT_SHIFT) return;
    if (key == GLFW_KEY_LEFT_CONTROL) return;
    if (key == GLFW_KEY_RIGHT_CONTROL) return;
    if (key == GLFW_KEY_LEFT_SUPER) return;
    if (key == GLFW_KEY_RIGHT_SUPER) return;
    if (action == GLFW_PRESS || (action == GLFW_REPEAT)) {
        // FORNOW: i guess okay to handle these here?
        bool toggle_pause = ((key == 'P') && (!control) && (!shift));
        bool step = (SCREEN.paused) && ((key == '.') && (!control) && (!shift));
        bool quit = ((key == 'Q') && (control) && (!shift));
        if (toggle_pause) {
            SCREEN.paused = !SCREEN.paused;
        } else if (step) {
            SCREEN.stepping_one_frame_while_paused = true;
        } else if (quit) {
            exit(1);
        } else {
            RawEvent raw_event = {}; {
                raw_event.type = EventType::Key;
                raw_event.key = key;
                raw_event.control = control;
                raw_event.shift = shift;
            }
            queue_enqueue(&raw_user_event_queue, raw_event);
        }
    }
}

// FORNOW: gui stuff that we don't record is handled here
void callback_cursor_position(GLFWwindow *, double xpos, double ypos) {
    { // hot_pane
        real x_divider_Pixel = get_x_divider_Pixel();
        real eps = 6.0f;
        real x = mouse_Pixel->x;
        if (popup->mouse_is_hovering) {
            *hot_pane = Pane::Popup;
        } else if (x < x_divider_Pixel - eps) {
            *hot_pane = Pane::DXF;
        } else if (x < x_divider_Pixel + eps) {
            *hot_pane = Pane::Divider;
        } else {
            *hot_pane = Pane::STL;
        }
    }


    // TODO: is a Pane::Popup a bad idea?--maybe just try it??

    // mouse_*
    vec2 delta_mouse_NDC;
    vec2 delta_mouse_World_2D;
    {
        vec2 prev_mouse_NDC = SCREEN.mouse_NDC;
        *mouse_Pixel = { real(xpos), real(ypos) };
        *mouse_NDC = transformPoint(*transform_NDC_from_Pixel, *mouse_Pixel);
        delta_mouse_NDC = SCREEN.mouse_NDC - prev_mouse_NDC;
        delta_mouse_World_2D = transformVector(inverse(camera_get_PV(camera_2D)), delta_mouse_NDC);
    }

    { // special draggin mouse_held EventType::Mouse
        if (0
                || (*mouse_left_drag_pane == Pane::DXF)
                || (*mouse_left_drag_pane == Pane::Popup)
           ) {
            RawEvent raw_event; {
                raw_event = {};
                raw_event.type = EventType::Mouse;
                raw_event.mouse_Pixel = *mouse_Pixel;
                raw_event.mouse_held = true;
                raw_event.pane = *mouse_left_drag_pane;
            }
            queue_enqueue(&raw_user_event_queue, raw_event);
        }
    }

    { // dragging divider
        if (*mouse_left_drag_pane == Pane::Divider) {
            real prev_x_divider_NDC = SCREEN.x_divider_NDC;
            SCREEN.x_divider_NDC = MAG_CLAMP(LINEAR_REMAP(xpos, 0.0f, window_get_width(), -1.0f, 1.0f), 0.9975f);
            real delta_NDC = 0.5f * (SCREEN.x_divider_NDC - prev_x_divider_NDC);
            camera_2D->display_nudge_NDC.x += delta_NDC;
            camera_3D->display_nudge_NDC.x += delta_NDC;
        }
    }

    { // moving cameras
        ;
        // NOTE: stolen from cow
        // NOTE: _not_ accumulating; just going for it on every event (shrug)
        if (*mouse_right_drag_pane == Pane::DXF) camera_2D->center_World -= delta_mouse_World_2D;
        if (*mouse_right_drag_pane == Pane::STL) {
            // FORNOW: bad, repetitive
            Camera2D tmp_2D = { camera_get_screen_height_World(camera_3D), camera_3D->_center_World.x, camera_3D->_center_World.y, camera_3D->display_nudge_NDC.x, camera_3D->display_nudge_NDC.y };
            tmp_2D.center_World -= transformVector(inverse(camera_get_PV(&tmp_2D)), delta_mouse_NDC);
            camera_3D->_center_World = tmp_2D.center_World;
        }
        if (*mouse_left_drag_pane == Pane::STL) {
            cow_real fac = 2;
            camera_3D->theta -= fac * delta_mouse_NDC.x;
            camera_3D->phi += fac * delta_mouse_NDC.y;
            camera_3D->phi = CLAMP(camera_3D->phi, -RAD(90), RAD(90));
        }
    }
}

// NOTE: mouse does not have GLFW_REPEAT
void callback_mouse_button(GLFWwindow *, int button, int action, int) {
    _callback_mouse_button(NULL, button, action, 0); // FORNOW TODO TODO TODO SHIM for cow Camera's

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            *mouse_left_drag_pane = *hot_pane;
            RawEvent raw_event; {
                raw_event = {};
                raw_event.type = EventType::Mouse;
                raw_event.mouse_Pixel = *mouse_Pixel;
                raw_event.pane = *hot_pane;
            }
            queue_enqueue(&raw_user_event_queue, raw_event);
        } else { ASSERT(action == GLFW_RELEASE);
            *mouse_left_drag_pane = Pane::None;
        }
    } else { ASSERT(button == GLFW_MOUSE_BUTTON_RIGHT);
        if (action == GLFW_PRESS) {
            *mouse_right_drag_pane = *hot_pane;
        } else { ASSERT(action == GLFW_RELEASE);
            *mouse_right_drag_pane = Pane::None;
        }
    }
}

void _callback_scroll_helper(Camera2D *camera, double yoffset) {
    // GOAL: mouse_World_2D_III equal-to mouse_World_2D_I
    vec2 mouse_World_2D_I  = transformPoint(inverse(camera_get_PV(camera)), SCREEN.mouse_NDC);
    camera->height_World *= (1.0f - 0.1f * yoffset);
    vec2 mouse_World_2D_II = transformPoint(inverse(camera_get_PV(camera)), SCREEN.mouse_NDC);
    camera->center_World -= (mouse_World_2D_II - mouse_World_2D_I);
}
void callback_scroll(GLFWwindow *, double, double yoffset) {
    // NOTE: stolen from cow
    // NOTE: _not_ accumulating; just going for it on every event (shrug)
    if (*hot_pane == Pane::DXF) {
        _callback_scroll_helper(camera_2D, yoffset);
    } else if (*hot_pane == Pane::STL) {
        Camera2D tmp_2D = { camera_get_screen_height_World(camera_3D), camera_3D->_center_World.x, camera_3D->_center_World.y, camera_3D->display_nudge_NDC.x, camera_3D->display_nudge_NDC.y };
        _callback_scroll_helper(&tmp_2D, yoffset);
        if (IS_ZERO(camera_3D->angle_of_view)) {
            camera_3D->ortho_screen_height_World = tmp_2D.height_World;
        } else {
            real r_y = tmp_2D.height_World / 2;
            real theta = camera_3D->angle_of_view / 2;
            camera_3D->persp_distance_to_origin = r_y / tan(theta);
        }
        camera_3D->_center_World = tmp_2D.center_World;
    }
}

KeyEventSubtype classify_baked_subtype_of_raw_key_event(KeyEvent key_event) {
    if (!popup->_active_popup_unique_ID__FORNOW_name0) return KeyEventSubtype::Hotkey;

    uint key = key_event.key;
    // bool shift = event.shift;
    // bool control = event.control;
    bool key_is_digit = ('0' <= key) && (key <= '9');
    bool key_is_punc  = (key == '.') || (key == '-');
    bool key_is_alpha = ('A' <= key) && (key <= 'Z');
    bool key_is_delete = (key == GLFW_KEY_BACKSPACE) || (key == GLFW_KEY_DELETE);
    bool key_is_enter = (key == GLFW_KEY_ENTER);
    bool key_is_nav = (key == GLFW_KEY_TAB) || (key == GLFW_KEY_LEFT) || (key == GLFW_KEY_RIGHT);

    bool is_consumable_by_popup; {
        is_consumable_by_popup = false;
        if (!_SELECT_OR_DESELECT_COLOR()) is_consumable_by_popup |= key_is_digit;
        is_consumable_by_popup |= key_is_punc;
        is_consumable_by_popup |= key_is_delete;
        is_consumable_by_popup |= key_is_enter;
        is_consumable_by_popup |= key_is_nav;
        if (popup->_type_of_active_cell == CellType::Real32) {
            ;
        } else if (popup->_type_of_active_cell == CellType::String) {
            is_consumable_by_popup |= key_is_alpha;
        } else {
            ASSERT(false);
        }
    }
    if (is_consumable_by_popup) return KeyEventSubtype::Popup;
    return KeyEventSubtype::Hotkey;
}

// NOTE: this function does WORLD-dependent stuff (magic-snapping)
// NOTE: a lot of stuff is happening at once here:
//       pixel coords -> pre-snapped world coords -> snapped world-coords
// NOTE: this function can "drop" raw_event's by returning the null event.
//       this smells a bit (should probs fail earlier, but I like the previous layer not knowing this stuff)
Event bake_event(RawEvent raw_event) {
    _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(raw_event);

    Event event = {};
    if (raw_event.type == EventType::Key) {
        event.type = EventType::Key;
        KeyEvent *key_event = &event.key_event;
        key_event->key = raw_event.key;
        key_event->control = raw_event.control;
        key_event->shift = raw_event.shift;
        key_event->subtype = classify_baked_subtype_of_raw_key_event(*key_event); // NOTE: must come last
    } else { ASSERT(raw_event.type == EventType::Mouse);
        event.type = EventType::Mouse;
        MouseEvent *mouse_event = &event.mouse_event;
        mouse_event->mouse_held = raw_event.mouse_held;
        {
            if (raw_event.pane == Pane::DXF) {
                mat4 inv_PV_2D = inverse(camera_get_PV(camera_2D));
                vec2 mouse_World_2D = transformPoint(inv_PV_2D, *mouse_NDC);

                mouse_event->subtype = MouseEventSubtype::DXF;

                MouseEvent2D *mouse_event_2D = &mouse_event->mouse_event_2D;
                mouse_event_2D->mouse_position = magic_snap(mouse_World_2D);
            } else if (raw_event.pane == Pane::STL) {
                mat4 inv_PV_3D = inverse(camera_get_PV(&SCREEN.camera_3D));
                vec3 a = transformPoint(inv_PV_3D, V3(*mouse_NDC, -1.0f));
                vec3 b = transformPoint(inv_PV_3D, V3(*mouse_NDC,  1.0f));

                mouse_event->subtype = MouseEventSubtype::STL;

                MouseEvent3D *mouse_event_3D = &mouse_event->mouse_event_3D;
                mouse_event_3D->mouse_ray_origin = a;
                mouse_event_3D->mouse_ray_direction = normalized(b - a);
            } else if (raw_event.pane == Pane::Popup) {
                // TODO: clean up (some state baad in certain cases -- dragging but leave cell)
                // TODO: gross gross gross

                mouse_event->subtype = MouseEventSubtype::Popup;

                MouseEventGUI *mouse_event_gui = &mouse_event->mouse_event_gui;
                mouse_event_gui->cell_index = popup->hover_cell_index; 
                if (!raw_event.mouse_held) {
                    mouse_event_gui->cursor = popup->hover_cursor; 
                } else {
                    // preserve old value; NOTE: could also be achieved with event.set_cursor = false;
                    mouse_event_gui->cursor = popup->cursor;
                }
                mouse_event_gui->selection_cursor = popup->hover_cursor; 
            } else { ASSERT(raw_event.pane == Pane::Divider);
                event = {};
            }
        }
    }
    return event;
}

BEGIN_PRE_MAIN {
    glfwSetKeyCallback(        COW0._window_glfw_window, callback_key);
    glfwSetCursorPosCallback(  COW0._window_glfw_window, callback_cursor_position);
    glfwSetMouseButtonCallback(COW0._window_glfw_window, callback_mouse_button);
    glfwSetScrollCallback(     COW0._window_glfw_window, callback_scroll);
} END_PRE_MAIN;

// TODO: this API should match what is printed to the terminal in verbose output mode
//       (so we can copy and paste a session for later use as an end to end test)

Event construct_mouse_event_2D(vec2 mouse_position) {
    Event event = {};
    event.type = EventType::Mouse;
    MouseEvent *mouse_event = &event.mouse_event;
    mouse_event->subtype = MouseEventSubtype::DXF;
    MouseEvent2D *mouse_event_2D = &mouse_event->mouse_event_2D;
    mouse_event_2D->mouse_position = mouse_position;
    return event;
}
Event construct_mouse_event_2D(real mouse_position_x, real mouse_position_y) { return construct_mouse_event_2D({ mouse_position_x, mouse_position_y }); }

Event construct_mouse_event_3D(vec3 mouse_ray_origin, vec3 mouse_ray_direction) {
    Event event = {};
    event.type = EventType::Mouse;
    MouseEvent *mouse_event = &event.mouse_event;
    mouse_event->subtype = MouseEventSubtype::STL;
    MouseEvent3D *mouse_event_3D = &mouse_event->mouse_event_3D;
    mouse_event_3D->mouse_ray_origin = mouse_ray_origin;
    mouse_event_3D->mouse_ray_direction = mouse_ray_direction;
    return event;
}

/////////////////////////////////////////////////////////
// PROCESSING A SINGLE STANDARD EVENT ///////////////////
/////////////////////////////////////////////////////////
// NOTE: this function is (a teeny bit) recursive D;
// NOTE: this sometimes modifies WORLD.dxf
// NOTE: this sometimes modifies WORLD.mesh
// NOTE (May 5, 2024): 
// - returns flags for how the fresh event processor should deal with this event
// - is the event is marked as a snapshotted event (by construction, only possible for non-fresh events), then skips the expensive stuff

struct StandardEventProcessResult {
    bool record_me;
    bool checkpoint_me;
    bool snapshot_me;
};

#if 0
// TODO
StandardEventProcessResult standard_event_process(Event event) {
    bool global_world_state_changed;
    StandardEventProcessResult result = _standard_event_process_NOTE_RECURSIVE(event);
    if (global_world_state_changed) {
        result.record_me = true;
    }
    return result;
}
#endif

void history_printf_script(); // FORNOW forward declaration

StandardEventProcessResult _standard_event_process_NOTE_RECURSIVE(Event event) {
    bool skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack = event.snapshot_me;

    bool dxf_anything_selected; {
        dxf_anything_selected = false;
        _for_each_selected_entity_ {
            dxf_anything_selected = true;
            break;
        }
    }
    bool value_to_write_to_selection_mask = (*click_mode == ClickMode::Select);

    StandardEventProcessResult result = {};

    #include "cookbook_lambdas.cpp"

    if (event.type == EventType::Key) {
        KeyEvent *key_event = &event.key_event;
        if (key_event->subtype == KeyEventSubtype::Hotkey) {
            result.record_me = true;

            auto key_lambda = [key_event](uint key, bool control = false, bool shift = false) -> bool {
                return _key_lambda(key_event, key, control, shift);
            };

            bool digit_lambda;
            uint digit;
            {
                digit_lambda = false;
                for (uint color = 0; color <= 9; ++color) {
                    if (key_lambda('0' + color)) {
                        digit_lambda = true;
                        digit = color;
                        break;
                    }
                }
            }

            ClickMode prev_click_mode = *click_mode;
            EnterMode prev_enter_mode = *enter_mode;
            { // key_lambda
                if (digit_lambda) {
                    if (click_mode_SELECT_OR_DESELECT() && (*click_modifier == ClickModifier::Color)) { // [sd]q0
                        _for_each_entity_ {
                            uint i = uint(entity->color_code);
                            if (i != digit) continue;
                            ENTITY_SET_IS_SELECTED(entity, value_to_write_to_selection_mask);
                        }
                        *click_mode = ClickMode::None;
                        *click_modifier = ClickModifier::None;
                    } else if ((*click_mode == ClickMode::Color) && (*click_modifier == ClickModifier::Selected)) { // qs0
                        _for_each_selected_entity_ ENTITY_SET_COLOR(entity, ColorCode(digit));
                        *click_mode = ClickMode::None;
                        *click_modifier = ClickModifier::None;
                        _for_each_entity_ entity->is_selected = false;
                    } else { // 0
                        result.record_me = true;
                        *click_mode = ClickMode::Color;
                        *click_modifier = ClickModifier::None;
                        *click_color_code = ColorCode(digit);
                    }
                } else if (key_lambda('A')) {
                    if (click_mode_SELECT_OR_DESELECT()) {
                        result.checkpoint_me = true;
                        CLEAR_SELECTION_MASK_TO(*click_mode == ClickMode::Select);
                        *click_mode = ClickMode::None;
                        *click_modifier = ClickModifier::None;
                    }
                } else if (key_lambda('A', false, true)) {
                    *click_mode = ClickMode::Axis;
                    *click_modifier = ClickModifier::None;
                    *awaiting_second_click = false;
                } else if (key_lambda('B')) {
                    *click_mode = ClickMode::Box;
                    *click_modifier = ClickModifier::None;
                    *awaiting_second_click = false;
                } else if (key_lambda('C')) {
                    if (((*click_mode == ClickMode::Select) || (*click_mode == ClickMode::Deselect)) && (*click_modifier != ClickModifier::Connected)) {
                        *click_modifier = ClickModifier::Connected;
                    } else if (click_mode_SNAP_ELIGIBLE()) {
                        result.record_me = false;
                        *click_modifier = ClickModifier::Center;
                    } else {
                        *click_mode = ClickMode::Circle;
                        *click_modifier = ClickModifier::None;
                        *awaiting_second_click = false;
                    }
                } else if (key_lambda('D')) {
                    *click_mode = ClickMode::Deselect;
                    *click_modifier = ClickModifier::None;
                } else if (key_lambda('E')) {
                    if (click_mode_SNAP_ELIGIBLE()) {
                        result.record_me = false;
                        *click_modifier = ClickModifier::End;
                    }
                } else if (key_lambda('F')) {
                    *click_mode = ClickMode::Fillet;
                    *click_modifier = ClickModifier::None;
                    *enter_mode = EnterMode::None;
                    *awaiting_second_click = false;
                } else if (key_lambda('G')) {
                    result.record_me = false;
                    SCREEN.hide_grid = !SCREEN.hide_grid;
                } else if (key_lambda('H')) {
                    result.record_me = false;
                    history_printf_script();
                } else if (key_lambda('K')) { 
                    result.record_me = false;
                    SCREEN.show_event_stack = !SCREEN.show_event_stack;
                } else if (key_lambda('K', false, true)) {
                    result.record_me = false;
                    SCREEN.hide_gui = !SCREEN.hide_gui;
                } else if (key_lambda('L')) {
                    *click_mode = ClickMode::Line;
                    *click_modifier = ClickModifier::None;
                    *awaiting_second_click = false;
                } else if (key_lambda('M')) {
                    if (click_mode_SNAP_ELIGIBLE()) {
                        result.record_me = false;
                        *click_modifier = ClickModifier::Middle;
                    } else {
                        *click_mode = ClickMode::Move;
                        *click_modifier = ClickModifier::None;
                        *awaiting_second_click = false;
                    }
                } else if (key_lambda('M', false, true)) {
                    result.record_me = false;
                    *click_mode = ClickMode::Measure;
                    *click_modifier = ClickModifier::None;
                    *awaiting_second_click = false;
                } else if (key_lambda('N')) {
                    if (feature_plane->is_active) {
                        *enter_mode = EnterMode::NudgeFeaturePlane;
                        *preview_feature_plane_offset = 0.0f; // FORNOW
                    } else {
                        conversation_messagef(omax.orange, "[n] no plane is_selected");
                    }
                } else if (key_lambda('N', true, false)) {
                    result.checkpoint_me = true;
                    result.snapshot_me = true;
                    list_free_AND_zero(&dxf->entities);
                    *dxf = {};
                    conversation_messagef(omax.green, "DXF was reset.");
                } else if (key_lambda('N', true, true)) {
                    result.checkpoint_me = true;
                    result.snapshot_me = true;
                    mesh_free_AND_zero(mesh);
                    *feature_plane = {};
                    conversation_messagef(omax.green, "STL was reset.");
                } else if (key_lambda('O', true)) {
                    *enter_mode = EnterMode::Open;
                } else if (key_lambda('Q')) {
                    if (click_mode_SELECT_OR_DESELECT() && (*click_modifier == ClickModifier::None)) {
                        *click_modifier = ClickModifier::Color;
                    } else {
                        *click_mode = ClickMode::Color;
                        *click_modifier = ClickModifier::None;
                    }
                } else if (key_lambda('S')) {
                    if (*click_mode != ClickMode::Color) {
                        *click_mode = ClickMode::Select;
                        *click_modifier = ClickModifier::None;
                    } else {
                        *click_modifier = ClickModifier::Selected;
                    }
                } else if (key_lambda('S', true)) {
                    result.record_me = false;
                    *enter_mode = EnterMode::Save;
                } else if (key_lambda('W')) {
                    if ((*click_mode == ClickMode::Select) || (*click_mode == ClickMode::Deselect)) {
                        *click_modifier = ClickModifier::Window;
                        *awaiting_second_click = false;
                    }
                } else if (key_lambda('X')) {
                    if (*click_mode != ClickMode::None) {
                        *click_modifier = ClickModifier::XYCoordinates;
                    }
                } else if (key_lambda('X', false, true)) {
                    *click_mode = ClickMode::MirrorX;
                    *click_modifier = ClickModifier::None;
                } else if (key_lambda('X', true, true)) {
                    result.record_me = false;
                    camera2D_zoom_to_bounding_box(&SCREEN.camera_2D, dxf_entities_get_bounding_box(&dxf->entities));
                } else if (key_lambda('Y')) {
                    // TODO: 'Y' remembers last terminal choice of plane for next time
                    result.checkpoint_me = true;
                    time_since->plane_selected = 0.0f;

                    // already one of the three primary planes
                    if ((feature_plane->is_active) && ARE_EQUAL(feature_plane->signed_distance_to_world_origin, 0.0f) && ARE_EQUAL(squaredNorm(feature_plane->normal), 1.0f) && ARE_EQUAL(maxComponent(feature_plane->normal), 1.0f)) {
                        feature_plane->normal = { feature_plane->normal[2], feature_plane->normal[0], feature_plane->normal[1] };
                    } else {
                        feature_plane->is_active = true;
                        feature_plane->signed_distance_to_world_origin = 0.0f;
                        feature_plane->normal = { 0.0f, 1.0f, 0.0f };
                    }
                } else if (key_lambda('Y', false, true)) {
                    *click_mode = ClickMode::MirrorY;
                    *click_modifier = ClickModifier::None;
                } else if (key_lambda('Z')) {
                    Event equivalent = {};
                    equivalent.type = EventType::Mouse;
                    equivalent.mouse_event.subtype = MouseEventSubtype::DXF;
                    // .mouse_position = {};
                    return _standard_event_process_NOTE_RECURSIVE(equivalent);
                } else if (key_lambda('Z', false, true)) {
                    *click_mode = ClickMode::Origin;
                    *click_modifier = ClickModifier::None;
                } else if (key_lambda(' ')) {
                    *click_mode = ClickMode::None; // FORNOW: patching space space doing CIRCLE CENTER
                    return _standard_event_process_NOTE_RECURSIVE(*space_bar_event);
                } else if (key_lambda(' ', false, true)) {
                    return _standard_event_process_NOTE_RECURSIVE(*shift_space_bar_event);
                } else if (key_lambda('[')) {
                    *enter_mode = EnterMode::ExtrudeAdd;
                    *preview_extrude_in_length = 0; // FORNOW
                    *preview_extrude_out_length = 0; // FORNOW
                } else if (key_lambda('[', false, true)) {
                    *enter_mode = EnterMode::ExtrudeCut;
                    *preview_extrude_in_length = 0; // FORNOW
                    *preview_extrude_out_length = 0; // FORNOW
                } else if (key_lambda(']')) {
                    *enter_mode = EnterMode::RevolveAdd;
                } else if (key_lambda(']', false, true)) {
                    *enter_mode = EnterMode::RevolveCut;
                } else if (key_lambda('.')) { 
                    result.record_me = false;
                    SCREEN.show_details = !SCREEN.show_details;
                    if (SCREEN.show_details) conversation_messagef(omax.cyan,"%d dxf-elements  %d stl-triangles", WORLD.dxf.entities.length, mesh->num_triangles);
                } else if (key_lambda(';')) {
                    result.checkpoint_me = true;
                    feature_plane->is_active = false;
                } else if (key_lambda('\'')) {
                    result.record_me = false;
                    SCREEN.camera_3D.angle_of_view = CAMERA_3D_DEFAULT_ANGLE_OF_VIEW - SCREEN.camera_3D.angle_of_view;
                } else if (key_lambda(GLFW_KEY_BACKSPACE) || key_lambda(COW_KEY_DELETE)) {
                    for (int32 i = dxf->entities.length - 1; i >= 0; --i) {
                        if (dxf->entities.array[i].is_selected) {
                            _REMOVE_ENTITY(i);
                        }
                    }
                } else if (key_lambda('/', false, true)) {
                    result.record_me = false;
                    SCREEN.show_help = !SCREEN.show_help;
                } else if (key_lambda(GLFW_KEY_ESCAPE)) {
                    *enter_mode = EnterMode::None;
                    *click_mode = ClickMode::None;
                    *click_modifier = ClickModifier::None;
                    *click_color_code = ColorCode::Traverse;
                } else if (key_lambda(GLFW_KEY_ENTER)) { // FORNOW
                    conversation_messagef(omax.orange, "EnterMode is None.");
                } else {
                    conversation_messagef(omax.orange, "Hotkey %s was not recognized.", key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(key_event), key_event->control, key_event->shift, key_event->key);
                    result.record_me = false;
                    ;
                }
            }
            bool changed_click_mode = (prev_click_mode != *click_mode);
            bool changed_enter_mode = (prev_enter_mode != *enter_mode);
            if (changed_click_mode && click_mode_SPACE_BAR_REPEAT_ELIGIBLE()) *space_bar_event = event;
            if (changed_enter_mode && enter_mode_SHIFT_SPACE_BAR_REPEAT_ELIGIBLE()) *shift_space_bar_event = event;
        } else { ASSERT(key_event->subtype == KeyEventSubtype::Popup);
            result.record_me = true;

            time_since->cursor_start = 0.0; // FORNOW

            uint key = key_event->key;
            bool shift = key_event->shift;
            bool control = key_event->control;

            bool _tab_hack_so_aliases_not_introduced_too_far_up = false;
            if (key == GLFW_KEY_TAB) {
                _tab_hack_so_aliases_not_introduced_too_far_up = true;
                uint new_active_cell_index; {
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
            uint len = uint(strlen(active_cell));
            uint left_cursor = MIN(popup->cursor, popup->selection_cursor);
            uint right_cursor = MAX(popup->cursor, popup->selection_cursor);

            if (_tab_hack_so_aliases_not_introduced_too_far_up) {
            } else if (control && (key == 'A')) {
                popup->cursor = len;
                popup->selection_cursor = 0;
            } else if (key == GLFW_KEY_LEFT) {
                if (!shift && !control) {
                    if (POPUP_SELECTION_NOT_ACTIVE()) {
                        if (popup->cursor > 0) --popup->cursor;
                    } else {
                        popup->cursor = left_cursor;
                    }
                    popup->selection_cursor = popup->cursor;
                } else if (shift && !control) {
                    if (POPUP_SELECTION_NOT_ACTIVE()) popup->selection_cursor = popup->cursor;
                    if (popup->cursor > 0) --popup->cursor;
                } else if (control && !shift) {
                    popup->selection_cursor = popup->cursor = 0;
                } else { ASSERT(shift && control);
                    popup->selection_cursor = 0;
                }
            } else if (key == GLFW_KEY_RIGHT) {
                if (!shift && !control) {
                    if (POPUP_SELECTION_NOT_ACTIVE()) {
                        if (popup->cursor < len) ++popup->cursor;
                    } else {
                        popup->cursor = MAX(popup->cursor, popup->selection_cursor);
                    }
                    popup->selection_cursor = popup->cursor;
                } else if (shift && !control) {
                    if (POPUP_SELECTION_NOT_ACTIVE()) popup->selection_cursor = popup->cursor;
                    if (popup->cursor < len) ++popup->cursor;
                } else if (control && !shift) {
                    popup->selection_cursor = popup->cursor = len;
                } else { ASSERT(shift && control);
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

                bool key_is_alpha = ('A' <= key) && (key <= 'Z');

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
        }
    } else if (event.type == EventType::Mouse) {
        MouseEvent *mouse_event = &event.mouse_event;
        if (mouse_event->subtype == MouseEventSubtype::DXF) {
            MouseEvent2D *mouse_event_2D = &mouse_event->mouse_event_2D;

            result.record_me = true;
            if (*click_mode == ClickMode::Measure) result.record_me = false;
            if (mouse_event->mouse_held) result.record_me = false;

            vec2 *mouse = &mouse_event_2D->mouse_position;
            vec2 *second_click = &mouse_event_2D->mouse_position;

            bool click_mode_WINDOW_SELECT_OR_WINDOW_DESELECT = (click_mode_SELECT_OR_DESELECT() && (*click_modifier == ClickModifier::Window));

            bool click_mode_TWO_CLICK_COMMAND = 0 ||
                (*click_mode == ClickMode::Axis) ||
                (*click_mode == ClickMode::Measure) ||
                (*click_mode == ClickMode::Line) ||
                (*click_mode == ClickMode::Box) ||
                (*click_mode == ClickMode::Circle) ||
                (*click_mode == ClickMode::Fillet) ||
                (*click_mode == ClickMode::Move) ||
                click_mode_WINDOW_SELECT_OR_WINDOW_DESELECT; // fornow wonky case

            // fornow window wonky case
            if (_non_WINDOW__SELECT_DESELECT___OR___SET_COLOR()) { // NOTES: includes scand qc
                result.record_me = false;
                int hot_entity_index = dxf_find_closest_entity(&WORLD.dxf.entities, mouse_event_2D->mouse_position.x, mouse_event_2D->mouse_position.y);
                if (hot_entity_index != -1) {
                    if (*click_modifier != ClickModifier::Connected) {
                        if (click_mode_SELECT_OR_DESELECT()) {
                            ENTITY_SET_IS_SELECTED(&dxf->entities.array[hot_entity_index], value_to_write_to_selection_mask);
                        } else {
                            ENTITY_SET_COLOR(&dxf->entities.array[hot_entity_index], *click_color_code);
                        }
                    } else {
                        #if 1 // TODO: consider just using the O(n*m) algorithm here instead

                        #define GRID_CELL_WIDTH 0.001f

                        auto scalar_bucket = [&](real a) -> real {
                            return roundf(a / GRID_CELL_WIDTH) * GRID_CELL_WIDTH;
                        };

                        auto make_key = [&](real x, real y) -> vec2 {
                            return { scalar_bucket(x), scalar_bucket(y) };

                        };

                        auto nudge_key = [&](vec2 key, int dx, int dy) -> vec2 {
                            return make_key(key.x + dx * GRID_CELL_WIDTH, key.y + dy * GRID_CELL_WIDTH);
                        };

                        struct GridPointSlot {
                            bool populated;
                            int32 entity_index;
                            bool end_NOT_start;
                        };

                        struct GridCell {
                            GridPointSlot slots[2];
                        };

                        Map<vec2, GridCell> grid; { // TODO: build grid
                            grid = {};

                            auto push_into_grid_unless_cell_full__make_cell_if_none_exists = [&](real x, real y, uint entity_index, bool end_NOT_start) {
                                vec2 key = make_key(x, y);
                                GridCell *cell = _map_get_pointer(&grid, key);
                                if (cell == NULL) {
                                    map_put(&grid, key, {});
                                    cell = _map_get_pointer(&grid, key);
                                }
                                for (uint i = 0; i < ARRAY_LENGTH(cell->slots); ++i) {
                                    GridPointSlot *slot = &cell->slots[i];
                                    if (slot->populated) continue;
                                    slot->populated = true;
                                    slot->entity_index = entity_index;
                                    slot->end_NOT_start = end_NOT_start;
                                    // printf("%f %f [%d]\n", key.x, key.y, i);
                                    break;
                                }
                            };

                            for (uint entity_index = 0; entity_index < WORLD.dxf.entities.length; ++entity_index) {
                                Entity *entity = &WORLD.dxf.entities.array[entity_index];

                                real start_x, start_y, end_x, end_y;
                                entity_get_start_and_end_points(entity, &start_x, &start_y, &end_x, &end_y);
                                push_into_grid_unless_cell_full__make_cell_if_none_exists(start_x, start_y, entity_index, false);
                                push_into_grid_unless_cell_full__make_cell_if_none_exists(end_x, end_y, entity_index, true);
                            }
                        }

                        bool *edge_marked = (bool *) calloc(WORLD.dxf.entities.length, sizeof(bool));

                        ////////////////////////////////////////////////////////////////////////////////
                        // NOTE: We are now done adding to the grid, so we can now operate directly on GridCell *'s
                        //       We will use _map_get_pointer(...)
                        ////////////////////////////////////////////////////////////////////////////////


                        auto get_key = [&](GridPointSlot *point, bool other_endpoint) {
                            bool end_NOT_start; {
                                end_NOT_start = point->end_NOT_start;
                                if (other_endpoint) end_NOT_start = !end_NOT_start;
                            }
                            real x, y; {
                                Entity *entity = &WORLD.dxf.entities.array[point->entity_index];
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

                            for (uint i = 0; i < ARRAY_LENGTH(cell->slots); ++i) {
                                GridPointSlot *slot = &cell->slots[i];
                                if (!slot->populated) continue;
                                if (edge_marked[slot->entity_index]) continue;
                                return slot;
                            }
                            return NULL;
                        };



                        // NOTE: we will mark the hot entity, and then shoot off from both its endpoints
                        edge_marked[hot_entity_index] = true;
                        ENTITY_SET_IS_SELECTED(&dxf->entities.array[hot_entity_index], value_to_write_to_selection_mask);

                        for (uint pass = 0; pass <= 1; ++pass) {

                            vec2 seed; {
                                real x, y;
                                if (pass == 0) {
                                    entity_get_start_point(&WORLD.dxf.entities.array[hot_entity_index], &x, &y);
                                } else {
                                    entity_get_end_point(&WORLD.dxf.entities.array[hot_entity_index], &x, &y);
                                }
                                seed = make_key(x, y);
                            }

                            GridPointSlot *curr = get_any_point_not_part_of_an_marked_entity(seed);
                            while (true) {
                                if (curr == NULL) break;
                                ENTITY_SET_IS_SELECTED(&dxf->entities.array[curr->entity_index], value_to_write_to_selection_mask);
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
                        uint loop_index = dxf_pick_loops.loop_index_from_entity_index[hot_entity_index];
                        DXFEntityIndexAndFlipFlag *loop = dxf_pick_loops.loops[loop_index];
                        uint num_entities = dxf_pick_loops.num_entities_in_loops[loop_index];
                        for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < &loop[num_entities]; ++entity_index_and_flip_flag) {
                            ENTITY_SET_IS_SELECTED(&dxf->entities[entity_index_and_flip_flag->entity_index], value_to_write_to_selection_mask);
                        }
                        #endif
                    }
                }
            } else if (!mouse_event->mouse_held) {
                if (click_mode_TWO_CLICK_COMMAND) {
                    if (!*awaiting_second_click) {
                        *awaiting_second_click = true;
                        *first_click = mouse_event_2D->mouse_position;
                        if (*click_modifier != ClickModifier::Window) *click_modifier = ClickModifier::None;
                    } else {
                        if (0) {
                        } else if (*click_mode == ClickMode::Axis) {
                            *awaiting_second_click = false;
                            result.checkpoint_me = true;
                            *click_mode = ClickMode::None;
                            *click_modifier = ClickModifier::None;
                            dxf->axis_base_point = *first_click;
                            dxf->axis_angle_from_y = (-PI / 2) + atan2(*second_click - *first_click);
                        } else if (*click_mode == ClickMode::Box) {
                            if (IS_ZERO(ABS(first_click->x - second_click->x))) {
                                conversation_messagef(omax.orange, "[box] must have non-zero width ");
                            } else if (IS_ZERO(ABS(first_click->y - second_click->y))) {
                                conversation_messagef(omax.orange, "[box] must have non-zero height");
                            } else {
                                *awaiting_second_click = false;
                                result.checkpoint_me = true;
                                *click_mode = ClickMode::None;
                                *click_modifier = ClickModifier::None;
                                vec2 other_corner_A = { first_click->x, second_click->y };
                                vec2 other_corner_B = { second_click->x, first_click->y };
                                ADD_LINE_ENTITY(*first_click,  other_corner_A);
                                ADD_LINE_ENTITY(*first_click,  other_corner_B);
                                ADD_LINE_ENTITY(*second_click, other_corner_A);
                                ADD_LINE_ENTITY(*second_click, other_corner_B);
                            }
                        } else if (*click_mode == ClickMode::Fillet) {
                            *awaiting_second_click = false;
                            result.checkpoint_me = true;
                            *click_modifier = ClickModifier::None;
                            int i = dxf_find_closest_entity(&dxf->entities, first_click->x, first_click->y);
                            int j = dxf_find_closest_entity(&WORLD.dxf.entities, mouse_event_2D->mouse_position.x, mouse_event_2D->mouse_position.y);
                            if ((i != j) && (i != -1) && (j != -1)) {
                                real radius = *fillet_radius;
                                Entity *E_i = &dxf->entities.array[i];
                                Entity *E_j = &dxf->entities.array[j];
                                if ((E_i->type == EntityType::Line) && (E_j->type == EntityType::Line)) {
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

                                        vec2 m = AVG(*first_click, *second_click);

                                        vec2 e_ab = normalized(b - a);
                                        vec2 e_cd = normalized(d - c);

                                        bool keep_a, keep_c; {
                                            vec2 vector_p_m_in_edge_basis = inverse(hstack(e_ab, e_cd)) * (m - p);
                                            keep_a = (vector_p_m_in_edge_basis.x < 0.0f);
                                            keep_c = (vector_p_m_in_edge_basis.y < 0.0f);
                                        }

                                        // TODO: in general, just use burkardt's angle stuff

                                        vec2 s_ab = (keep_a) ? a : b;
                                        vec2 s_cd = (keep_c) ? c : d;
                                        real half_angle; {
                                            real angle = burkardt_three_point_angle(s_ab, p, s_cd); // FORNOW TODO consider using burkardt's special interior version
                                            if (angle > PI) angle = TAU - angle;
                                            half_angle = angle / 2;
                                        }
                                        real length = radius / tan(half_angle);
                                        vec2 t_ab = p + (keep_a ? -1 : 1) * length * e_ab;
                                        vec2 t_cd = p + (keep_c ? -1 : 1) * length * e_cd;

                                        LineLineIntersectionResult _center = burkardt_line_line_intersection(t_ab, t_ab + perpendicularTo(e_ab), t_cd, t_cd + perpendicularTo(e_cd));
                                        if (_center.is_valid) {
                                            vec2 center = _center.position;

                                            ColorCode color_i = E_i->color_code;
                                            ColorCode color_j = E_j->color_code;
                                            _REMOVE_ENTITY(MAX(i, j));
                                            _REMOVE_ENTITY(MIN(i, j));

                                            ADD_LINE_ENTITY(s_ab, t_ab, false, color_i);
                                            ADD_LINE_ENTITY(s_cd, t_cd, false, color_j);

                                            real theta_ab_in_degrees = DEG(atan2(t_ab - center));
                                            real theta_cd_in_degrees = DEG(atan2(t_cd - center));

                                            if (!IS_ZERO(radius)) {
                                                if (burkardt_three_point_angle(t_ab, center, t_cd) < PI) {
                                                    // FORNOW TODO consider swap
                                                    real tmp = theta_ab_in_degrees;
                                                    theta_ab_in_degrees = theta_cd_in_degrees;
                                                    theta_cd_in_degrees = tmp;
                                                }

                                                // TODO: consider tabbing to create chamfer

                                                ADD_ARC_ENTITY(center, radius, theta_ab_in_degrees, theta_cd_in_degrees);
                                            }
                                        }
                                    }
                                } else {
                                    conversation_messagef(monokai.red, "TODO: line_entity-arc_entity fillet; arc_entity-arc_entity fillet");
                                }
                            }
                        } else if (*click_mode == ClickMode::Circle) {
                            if (IS_ZERO(norm(*first_click - *second_click))) {
                                conversation_messagef(omax.orange, "[circle] must have non-zero diameter");
                            } else {
                                *awaiting_second_click = false;
                                result.checkpoint_me = true;
                                *click_mode = ClickMode::None;
                                *click_modifier = ClickModifier::None;
                                real theta_a_in_degrees = DEG(atan2(*second_click - *first_click));
                                real theta_b_in_degrees = theta_a_in_degrees + 180.0f;
                                real r = norm(*second_click - *first_click);
                                ADD_ARC_ENTITY(*first_click, r, theta_a_in_degrees, theta_b_in_degrees);
                                ADD_ARC_ENTITY(*first_click, r, theta_b_in_degrees, theta_a_in_degrees);
                            }
                        } else if (*click_mode == ClickMode::Line) {
                            *awaiting_second_click = false;
                            result.checkpoint_me = true;
                            *click_mode = ClickMode::None;
                            *click_modifier = ClickModifier::None;
                            ADD_LINE_ENTITY(*first_click, *second_click);
                        } else if (*click_mode == ClickMode::Measure) {
                            *awaiting_second_click = false;
                            *click_mode = ClickMode::None;
                            *click_modifier = ClickModifier::None;
                            real angle = DEG(atan2(*second_click - *first_click));
                            if (angle < 0.0f) angle += 360.0f;
                            conversation_messagef(omax.cyan, "Measured %gmm at %gdeg.", norm(*second_click - *first_click), angle);
                        } else if (*click_mode == ClickMode::Move) {
                            *awaiting_second_click = false;
                            result.checkpoint_me = true;
                            *click_mode = ClickMode::None;
                            *click_modifier = ClickModifier::None;
                            vec2 ds = *second_click - *first_click;
                            _for_each_selected_entity_ {
                                if (entity->type == EntityType::Line) {
                                    LineEntity *line_entity = &entity->line_entity;
                                    line_entity->start += ds;
                                    line_entity->end   += ds;
                                } else { ASSERT(entity->type == EntityType::Arc);
                                    ArcEntity *arc_entity = &entity->arc_entity;
                                    arc_entity->center += ds;
                                }
                            }
                        } else if (click_mode_WINDOW_SELECT_OR_WINDOW_DESELECT) {
                            *awaiting_second_click = false;
                            box2 window = {
                                MIN(first_click->x, second_click->x),
                                MIN(first_click->y, second_click->y),
                                MAX(first_click->x, second_click->x),
                                MAX(first_click->y, second_click->y)
                            };
                            _for_each_entity_ {
                                if (bounding_box_contains(window, dxf_entity_get_bounding_box(entity))) {
                                    ENTITY_SET_IS_SELECTED(entity, value_to_write_to_selection_mask);
                                }
                            }
                        }
                    }
                } else {
                    if (*click_mode == ClickMode::Origin) {
                        result.checkpoint_me = true;
                        *click_mode = ClickMode::None;
                        *click_modifier = ClickModifier::None;
                        dxf->origin = *mouse;
                    } else if (*click_mode == ClickMode::MirrorX) {
                        result.checkpoint_me = true;
                        *click_mode = ClickMode::None;
                        *click_modifier = ClickModifier::None;
                        _for_each_selected_entity_ {
                            if (entity->type == EntityType::Line) {
                                LineEntity *line_entity = &entity->line_entity;
                                BUFFER_LINE_ENTITY(
                                        V2(-(line_entity->start.x - mouse->x) + mouse->x, line_entity->start.y),
                                        V2(-(line_entity->end.x - mouse->x) + mouse->x, line_entity->end.y),
                                        true,
                                        entity->color_code
                                        );
                            } else { ASSERT(entity->type == EntityType::Arc);
                                ArcEntity *arc_entity = &entity->arc_entity;
                                BUFFER_ARC_ENTITY(
                                        V2(-(arc_entity->center.x - mouse->x) + mouse->x, arc_entity->center.y),
                                        arc_entity->radius,
                                        arc_entity->end_angle_in_degrees, // TODO
                                        arc_entity->start_angle_in_degrees, // TODO
                                        true,
                                        entity->color_code); // FORNOW + 180
                            }
                            entity->is_selected = false;
                        }
                        ADD_BUFFERED_ENTITIES();
                    } else if (*click_mode == ClickMode::MirrorY) {
                        result.checkpoint_me = true;
                        *click_mode = ClickMode::None;
                        *click_modifier = ClickModifier::None;
                        _for_each_selected_entity_ {
                            if (entity->type == EntityType::Line) {
                                LineEntity *line_entity = &entity->line_entity;
                                BUFFER_LINE_ENTITY(
                                        V2(line_entity->start.x, -(line_entity->start.y - mouse->y) + mouse->y),
                                        V2(line_entity->end.x, -(line_entity->end.y - mouse->y) + mouse->y),
                                        true,
                                        entity->color_code
                                        );
                            } else { ASSERT(entity->type == EntityType::Arc);
                                ArcEntity *arc_entity = &entity->arc_entity;
                                BUFFER_ARC_ENTITY(
                                        V2(arc_entity->center.x, -(arc_entity->center.y - mouse->y) + mouse->y),
                                        arc_entity->radius,
                                        arc_entity->end_angle_in_degrees, // TODO
                                        arc_entity->start_angle_in_degrees, // TODO
                                        true,
                                        entity->color_code); // FORNOW + 180
                            }
                            entity->is_selected = false;
                        }
                        ADD_BUFFERED_ENTITIES();
                    }
                }
            }
        } else if (mouse_event->subtype == MouseEventSubtype::STL) {
            MouseEvent3D *mouse_event_3D = &mouse_event->mouse_event_3D;
            result.record_me = false;
            if (!mouse_event->mouse_held) {
                int32 index_of_first_triangle_hit_by_ray = -1;
                {
                    real min_distance = HUGE_VAL;
                    for (uint i = 0; i < mesh->num_triangles; ++i) {
                        vec3 p[3]; {
                            for (uint j = 0; j < 3; ++j) p[j] = mesh->vertex_positions[mesh->triangle_indices[i][j]];
                        }
                        RayTriangleIntersectionResult ray_triangle_intersection_result = ray_triangle_intersection(mouse_event_3D->mouse_ray_origin, mouse_event_3D->mouse_ray_direction, p[0], p[1], p[2]);
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
                    time_since->plane_selected = 0.0f;
                    {
                        feature_plane->normal = mesh->triangle_normals[index_of_first_triangle_hit_by_ray];
                        { // FORNOW (gross) calculateion of feature_plane->signed_distance_to_world_origin
                            vec3 a_selected = mesh->vertex_positions[mesh->triangle_indices[index_of_first_triangle_hit_by_ray][0]];
                            feature_plane->signed_distance_to_world_origin = dot(feature_plane->normal, a_selected);
                        }

                    }
                }
            }
        } else { ASSERT(mouse_event->subtype == MouseEventSubtype::Popup);
            MouseEventGUI *mouse_event_gui = &mouse_event->mouse_event_gui;

            result.record_me = false;
            time_since->cursor_start = 0.0f;

            // TODO: probably also gross
            if ((!mouse_event->mouse_held) && (popup->active_cell_index != mouse_event_gui->cell_index)) {
                result.record_me = true;
                POPUP_SET_ACTIVE_CELL_INDEX(mouse_event_gui->cell_index);
            }
            // NOTE: this if is really gross and patches a problem where if you drag the mouse off the cell things break
            // TODO: this should be fixed earlier in the chain; better guarantees about what the event is sending you
            // (TODO: you should be able to leave the box)
            if (popup->active_cell_index == mouse_event_gui->cell_index) {
                if (popup->cursor != mouse_event_gui->cursor) {
                    result.record_me = true;
                    popup->cursor = mouse_event_gui->cursor;
                }
                if (popup->selection_cursor != mouse_event_gui->selection_cursor) {
                    result.record_me = true;
                    popup->selection_cursor = mouse_event_gui->selection_cursor;
                }
            }
        }
    } else { ASSERT(event.type == EventType::None);
        result.record_me = false;
    }


    { // sanity checks
        ASSERT(popup->active_cell_index <= popup->num_cells);
        ASSERT(popup->cursor <= POPUP_CELL_LENGTH);
        ASSERT(popup->selection_cursor <= POPUP_CELL_LENGTH);
    }

    { // popup_popup
        static char full_filename_scratch_buffer[512];

        EnterMode _enter_mode_prev__NOTE_used_to_determine_when_to_close_popup_on_enter = *enter_mode;
        bool popup_popup_actually_called_this_event = false;

        #include "popup_lambda.cpp"

        { // popup_popup

            bool gui_key_enter; {
                gui_key_enter = false;
                if (event.type == EventType::Key) {
                    KeyEvent *key_event = &event.key_event;
                    if (key_event->subtype == KeyEventSubtype::Popup) {
                        gui_key_enter = (key_event->key == GLFW_KEY_ENTER);
                    }
                }
            }

            if (*enter_mode == EnterMode::Open) {
                sprintf(full_filename_scratch_buffer, "%s%s", SCREEN.drop_path, open_filename);
                popup_popup(false,
                        CellType::String, "open_filename", open_filename);
                if (gui_key_enter) {
                    if (poe_suffix_match(full_filename_scratch_buffer, ".dxf")) {
                        result.record_me = true;
                        result.checkpoint_me = true;
                        result.snapshot_me = true;
                        conversation_dxf_load(full_filename_scratch_buffer,
                                skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack);
                        *enter_mode = EnterMode::None;
                    } else if (poe_suffix_match(full_filename_scratch_buffer, ".stl")) {
                        result.record_me = true;
                        result.checkpoint_me = true;
                        result.snapshot_me = true;
                        conversation_stl_load(full_filename_scratch_buffer);
                        *enter_mode = EnterMode::None;
                    } else {
                        conversation_messagef(omax.orange, "[open] \"%s\" not found", full_filename_scratch_buffer);
                    }
                }
            } else if (*enter_mode == EnterMode::Save) {
                result.record_me = false;
                sprintf(full_filename_scratch_buffer, "%s%s", SCREEN.drop_path, save_filename);
                popup_popup(false,
                        CellType::String, "save_filename", save_filename);
                if (gui_key_enter) {
                    conversation_save(full_filename_scratch_buffer);
                    *enter_mode = EnterMode::None;
                }
            } else if (*enter_mode == EnterMode::ExtrudeAdd) {
                popup_popup(true,
                        CellType::Real32, "extrude_add_out_length", extrude_add_out_length,
                        CellType::Real32, "extrude_add_in_length",  extrude_add_in_length);
                if (gui_key_enter) {
                    if (!dxf_anything_selected) {
                        conversation_messagef(omax.orange, "DXF selection is empty.");
                    } else if (!feature_plane->is_active) {
                        conversation_messagef(omax.orange, "No feature plane is selected.");
                    } else if (IS_ZERO(*extrude_add_in_length) && IS_ZERO(*extrude_add_out_length)) {
                        conversation_messagef(omax.orange, "Total extrusion length is zero.");
                    } else {
                        GENERAL_PURPOSE_MANIFOLD_WRAPPER();
                        conversation_messagef(omax.green, "ExtrudeAdd was successful.");
                    }
                }
            } else if (*enter_mode == EnterMode::ExtrudeCut) {
                popup_popup(true,
                        CellType::Real32, "extrude_cut_in_length",  extrude_cut_in_length,
                        CellType::Real32, "extrude_cut_out_length", extrude_cut_out_length);
                if (gui_key_enter) {
                    if (!dxf_anything_selected) {
                        conversation_messagef(omax.orange, "DXF selection is empty.");
                    } else if (!feature_plane->is_active) {
                        conversation_messagef(omax.orange, "No feature plane is selected.");
                    } else if (IS_ZERO(*extrude_cut_in_length) && IS_ZERO(*extrude_cut_out_length)) {
                        conversation_messagef(omax.orange, "Total extrusion length is zero.");
                    } else if (mesh->num_triangles == 0) {
                        conversation_messagef(omax.orange, "Current mesh is empty.");
                    } else {
                        GENERAL_PURPOSE_MANIFOLD_WRAPPER();
                        conversation_messagef(omax.green, "ExtrudeCut was successful.");
                    }
                }
            } else if (*enter_mode == EnterMode::RevolveAdd) {
                popup_popup(true, CellType::Real32, "revolve_add_dummy", revolve_add_dummy);
                if (gui_key_enter) {
                    if (!dxf_anything_selected) {
                        conversation_messagef(omax.orange, "DXF selection is empty.");
                    } else if (!feature_plane->is_active) {
                        conversation_messagef(omax.orange, "No feature plane is selected.");
                    } else {
                        GENERAL_PURPOSE_MANIFOLD_WRAPPER();
                        conversation_messagef(omax.green, "RevolveAdd was successful.");
                    }
                }
            } else if (*enter_mode == EnterMode::RevolveCut) {
                popup_popup(true, CellType::Real32, "revolve_cut_dummy", revolve_cut_dummy);
                if (gui_key_enter) {
                    if (!dxf_anything_selected) {
                        conversation_messagef(omax.orange, "DXF selection is empty.");
                    } else if (!feature_plane->is_active) {
                        conversation_messagef(omax.orange, "No feature plane is selected.");
                    } else if (mesh->num_triangles == 0) {
                        conversation_messagef(omax.orange, "Current mesh is empty.");
                    } else {
                        GENERAL_PURPOSE_MANIFOLD_WRAPPER();
                        conversation_messagef(omax.green, "RevolveCut was successful.");
                    }
                }
            } else if (*enter_mode == EnterMode::NudgeFeaturePlane) {
                popup_popup(true,
                        CellType::Real32, "feature_plane_nudge", feature_plane_nudge);
                if (gui_key_enter) {
                    result.record_me = true;
                    result.checkpoint_me = true;
                    feature_plane->signed_distance_to_world_origin += *feature_plane_nudge;
                    *enter_mode = EnterMode::None;
                }
            } else if (*click_modifier == ClickModifier::XYCoordinates) {
                // sus calling this a modifier but okay; make sure it's first or else bad bad
                popup_popup(true,
                        CellType::Real32, "x_coordinate", x_coordinate,
                        CellType::Real32, "y_coordinate", y_coordinate);
                if (gui_key_enter) {
                    // popup->_active_popup_unique_ID__FORNOW_name0 = NULL; // FORNOW when making box using 'X' 'X', we want the popup to trigger a reload
                    *click_modifier = ClickModifier::None;
                    return _standard_event_process_NOTE_RECURSIVE(construct_mouse_event_2D(*x_coordinate, *y_coordinate));
                }
            } else if (*click_mode == ClickMode::Circle) {
                if (*awaiting_second_click) {
                    real prev_circle_diameter = *circle_diameter;
                    real prev_circle_radius = *circle_radius;
                    real prev_circle_circumference = *circle_circumference;
                    popup_popup(false,
                            CellType::Real32, "circle_diameter", circle_diameter,
                            CellType::Real32, "circle_radius", circle_radius,
                            CellType::Real32, "circle_circumference", circle_circumference);
                    if (gui_key_enter) {
                        return _standard_event_process_NOTE_RECURSIVE(construct_mouse_event_2D(first_click->x + *circle_radius, first_click->y));
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
            } else if (*click_mode == ClickMode::Line) {
                if (*awaiting_second_click) {
                    real prev_line_length = *line_length;
                    real prev_line_angle = *line_angle;
                    real prev_line_run = *line_run;
                    real prev_line_rise = *line_rise;
                    popup_popup(true,
                            CellType::Real32, "line_length", line_length,
                            CellType::Real32, "line_angle", line_angle,
                            CellType::Real32, "line_run", line_run,
                            CellType::Real32, "line_rise", line_rise
                            );
                    if (gui_key_enter) {
                        return _standard_event_process_NOTE_RECURSIVE(construct_mouse_event_2D(first_click->x + *line_run, first_click->y + *line_rise));
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
            } else if (*click_mode == ClickMode::Box) {
                if (*awaiting_second_click) {
                    popup_popup(true,
                            CellType::Real32, "box_width", box_width,
                            CellType::Real32, "box_height", box_height);
                    if (gui_key_enter) {
                        return _standard_event_process_NOTE_RECURSIVE(construct_mouse_event_2D(first_click->x + *box_width, first_click->y + *box_height));
                    }
                }
            } else if (*click_mode == ClickMode::Move) {
                // FORNOW: this is repeated from LINE
                if (*awaiting_second_click) {
                    real prev_move_length = *move_length;
                    real prev_move_angle = *move_angle;
                    real prev_move_run = *move_run;
                    real prev_move_rise = *move_rise;
                    popup_popup(true,
                            CellType::Real32, "move_length", move_length,
                            CellType::Real32, "move_angle", move_angle,
                            CellType::Real32, "move_run", move_run,
                            CellType::Real32, "move_rise", move_rise
                            );
                    if (gui_key_enter) {
                        return _standard_event_process_NOTE_RECURSIVE(construct_mouse_event_2D(first_click->x + *move_run, first_click->y + *move_rise));
                    } else {
                        if ((prev_move_length != *move_length) || (prev_move_angle != *move_angle)) {
                            *move_run = *move_length * COS(RAD(*move_angle));
                            *move_rise = *move_length * SIN(RAD(*move_angle));
                        } else if ((prev_move_run != *move_run) || (prev_move_rise != *move_rise)) {
                            *move_length = SQRT(*move_run * *move_run + *move_rise * *move_rise);
                            *move_angle = DEG(ATAN2(*move_rise, *move_run));
                        }
                    }
                }
            } else if (*click_mode == ClickMode::Fillet) {
                popup_popup(false,
                        CellType::Real32, "fillet_radius", fillet_radius);
            }
        }
        { // popup_close (FORNOW: just doing off of enter transitions)
          // NOTE: we need to do this so that the next key event doesn't get eaten by a dead popup
            bool enter_mode_transitioned_to_ENTER_MODE_NONE = ((_enter_mode_prev__NOTE_used_to_determine_when_to_close_popup_on_enter != EnterMode::None) && (*enter_mode == EnterMode::None));
            if (0
                    || (!popup_popup_actually_called_this_event)
                    || enter_mode_transitioned_to_ENTER_MODE_NONE
               ) {
                WORLD.popup._active_popup_unique_ID__FORNOW_name0 = NULL;
            }
        }
    }

    return result;
}

//////////////////////////////////////////////////
// HISTORY ///////////////////////////////////////
//////////////////////////////////////////////////

#ifdef DEBUG_HISTORY_DISABLE_HISTORY_ENTIRELY
//
void history_process_and_potentially_record_checkpoint_and_or_snapshot_standard_fresh_user_event(Event standard_event) { _standard_event_process_NOTE_RECURSIVE(standard_event); }
void history_undo() { conversation_messagef("[DEBUG] history disabled"); }
void history_redo() { conversation_messagef("[DEBUG] history disabled"); }
void history_debug_draw() { gui_printf("[DEBUG] history disabled"); }
//
#else

struct {
    ElephantStack<Event> recorded_user_events;
    ElephantStack<WorldState> snapshotted_world_states;
} history;

struct StackPointers {
    Event *user_event;
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
    Event *user_event = elephant_peek_undo(&history.recorded_user_events);
    WorldState *world_state = elephant_is_empty_undo(&history.snapshotted_world_states) ? NULL : elephant_peek_undo(&history.snapshotted_world_states);
    return { user_event, world_state };
}

bool EVENT_UNDO_NONEMPTY() {
    return !elephant_is_empty_undo(&history.recorded_user_events);
}

bool EVENT_REDO_NONEMPTY() {
    return !elephant_is_empty_redo(&history.recorded_user_events);
}

void PUSH_UNDO_CLEAR_REDO(Event standard_event) {
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
        world_state_deep_copy(&snapshot, &WORLD);
        elephant_push_undo_clear_redo(&history.snapshotted_world_states, snapshot); // TODO: clear is unnecessary here
    }
}

void history_process_and_potentially_record_checkpoint_and_or_snapshot_standard_fresh_user_event(Event standard_event) {
    StandardEventProcessResult tmp = _standard_event_process_NOTE_RECURSIVE(standard_event);
    standard_event.record_me = tmp.record_me;
    standard_event.checkpoint_me = tmp.checkpoint_me;
    #ifndef DEBUG_HISTORY_DISABLE_SNAPSHOTTING
    standard_event.snapshot_me = tmp.snapshot_me;
    #endif
    if (standard_event.record_me) PUSH_UNDO_CLEAR_REDO(standard_event);
}

void history_undo() {
    if (elephant_is_empty_undo(&history.recorded_user_events)) {
        conversation_messagef(omax.orange, "There is nothing to undo.");
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
    Event *one_past_end = elephant_undo_ptr_one_past_end(&history.recorded_user_events);
    Event *begin;
    { // // find beginning
      // 1) walk back to snapshot event (or end of stack)
      // TODO: this feels kind of sloppy still
        begin = one_past_end - 1; // ?
        world_state_free_AND_zero(&WORLD);
        while (true) {
            if (begin <= elephant_undo_ptr_begin(&history.recorded_user_events)) {
                begin =  elephant_undo_ptr_begin(&history.recorded_user_events); // !
                WORLD = {};
                break;
            }
            if (begin->snapshot_me) {
                world_state_deep_copy(&WORLD, PEEK_UNDO().world_state);
                break;
            }
            --begin;
        }
    }
    IGNORE_NEW_MESSAGEFS = true; // TODO: why does this seem unnecessary in practice??
    for (Event *event = begin; event < one_past_end; ++event) _standard_event_process_NOTE_RECURSIVE(*event);
    IGNORE_NEW_MESSAGEFS = false;
    conversation_messagef(omax.green, "Undo was successful.");
}

void history_redo() {
    if (elephant_is_empty_redo(&history.recorded_user_events)) {
        conversation_messagef(omax.orange, "There is nothing to redo.");
        return;
    }

    IGNORE_NEW_MESSAGEFS = true;
    while (EVENT_REDO_NONEMPTY()) { // // manipulate stacks (undo <- redo)
        StackPointers popped = POP_REDO_ONTO_UNDO();
        Event *user_event = popped.user_event;
        WorldState *world_state = popped.world_state;

        _standard_event_process_NOTE_RECURSIVE(*user_event);
        if (world_state) {
            world_state_free_AND_zero(&WORLD);
            world_state_deep_copy(&WORLD, world_state);
        }

        if (user_event->checkpoint_me) break;
    }
    IGNORE_NEW_MESSAGEFS = false;
    conversation_messagef(omax.green, "Redo was successful.");
}

void _history_user_event_draw_helper(Event event) {
    char message[256]; {
        // TODO: Left and right arrow
        // TODO: handle shift and control with the special characters
        if (event.type == EventType::Key) {
            KeyEvent *key_event = &event.key_event;
            char *boxed; {
                if (key_event->subtype == KeyEventSubtype::Hotkey) {
                    boxed = "[HOTKEY]";
                } else { ASSERT(key_event->subtype == KeyEventSubtype::Popup);
                    boxed = "[GUI_KEY]";
                }
            }
            sprintf(message, "%s %s", boxed, key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(key_event));
        } else { ASSERT(event.type == EventType::Mouse);
            MouseEvent *mouse_event = &event.mouse_event;
            if (mouse_event->subtype == MouseEventSubtype::DXF) {
                MouseEvent2D *mouse_event_2D = &mouse_event->mouse_event_2D;
                sprintf(message, "[MOUSE-2D] %g %g", mouse_event_2D->mouse_position.x, mouse_event_2D->mouse_position.y);
            } else if (mouse_event->subtype == MouseEventSubtype::STL) {
                MouseEvent3D *mouse_event_3D = &mouse_event->mouse_event_3D;
                sprintf(message, "[MOUSE-3D] %g %g %g %g %g %g", mouse_event_3D->mouse_ray_origin.x, mouse_event_3D->mouse_ray_origin.y, mouse_event_3D->mouse_ray_origin.z, mouse_event_3D->mouse_ray_direction.x, mouse_event_3D->mouse_ray_direction.y, mouse_event_3D->mouse_ray_direction.z);
            } else { ASSERT(mouse_event->subtype == MouseEventSubtype::Popup);
                MouseEventGUI *mouse_event_gui = &mouse_event->mouse_event_gui;
                sprintf(message, "[GUI-MOUSE] %d %d %d", mouse_event_gui->cell_index, mouse_event_gui->cursor, mouse_event_gui->selection_cursor);
            }
        }
    }
    gui_printf("%c%c %s",
            (event.checkpoint_me)   ? 'C' : ' ',
            (event.snapshot_me)     ? 'S' : ' ',
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
                Event *event = history.recorded_user_events._redo_stack.array;
                event < history.recorded_user_events._redo_stack.array + history.recorded_user_events._redo_stack.length;
                ++event
            ) {//
            _history_user_event_draw_helper(*event);
        }
        gui_printf("`^ redo (%d)", elephant_length_redo(&history.recorded_user_events));
    }
    if ((history.recorded_user_events._redo_stack.length) || (history.recorded_user_events._undo_stack.length)) {
        gui_printf("`  RECORDED_EVENTS");
    } else {
        gui_printf("`--- no history ---");
    }
    if (history.recorded_user_events._undo_stack.length) {
        gui_printf("`v undo (%d)", elephant_length_undo(&history.recorded_user_events));
        for (////
                Event *event = history.recorded_user_events._undo_stack.array + (history.recorded_user_events._undo_stack.length - 1);
                event >= history.recorded_user_events._undo_stack.array;
                --event
            ) {//
            _history_user_event_draw_helper(*event);
        }
    }
}

void history_printf_script() {
    List<char> _script = {};
    for (////
            Event *event = history.recorded_user_events._undo_stack.array;
            event < history.recorded_user_events._undo_stack.array + history.recorded_user_events._undo_stack.length;
            ++event
        ) {//
        if (event->type == EventType::Key) {
            KeyEvent key_event = event->key_event;
            if (key_event.control) list_push_back(&_script, '^');
            if (key_event.key == GLFW_KEY_ENTER) {
                list_push_back(&_script, '\\');
                list_push_back(&_script, 'n');
            } else {
                char char_equivalent = (char) key_event.key;
                if ((('A' <= key_event.key) && (key_event.key <= 'Z')) && !key_event.shift) char_equivalent = 'a' + (char_equivalent - 'A');
                list_push_back(&_script, (char) char_equivalent);
            }
        }
    }
    printf("%.*s\n", _script.length, _script.array);
    list_free_AND_zero(&_script);
}

#endif


//////////////////////////////////////////////////
// PROCESS A FRESH (potentially special) EVENT ///
//////////////////////////////////////////////////

void freshly_baked_event_process(Event freshly_baked_event) {
    bool undo;
    bool redo;
    {
        undo = false;
        redo = false;
        if (freshly_baked_event.type == EventType::Key) {
            KeyEvent *key_event = &freshly_baked_event.key_event;
            auto key_lambda = [key_event](uint key, bool control = false, bool shift = false) -> bool {
                return _key_lambda(key_event, key, control, shift);
            };
            if (!((popup->_active_popup_unique_ID__FORNOW_name0) && (popup->cell_type[popup->active_cell_index] == CellType::String))) { // FORNOW
                undo = (key_lambda('Z', true) || key_lambda('U'));
                redo = (key_lambda('Y', true) || key_lambda('Z', true, true) || key_lambda('U', false, true));
            }
        }
    }

    if (undo) {
        SCREEN.DONT_DRAW_ANY_MORE_POPUPS_THIS_FRAME = true;
        history_undo();
    } else if (redo) {
        SCREEN.DONT_DRAW_ANY_MORE_POPUPS_THIS_FRAME = true;
        history_redo();
    } else {
        history_process_and_potentially_record_checkpoint_and_or_snapshot_standard_fresh_user_event(freshly_baked_event);
    }
}

//////////////////////////////////////////////////
// DRAW() ////////////////////////////////////////
//////////////////////////////////////////////////

void conversation_draw() {
    mat4 P_2D = camera_get_P(&SCREEN.camera_2D);
    mat4 V_2D = camera_get_V(&SCREEN.camera_2D);
    mat4 PV_2D = P_2D * V_2D;
    mat4 inv_PV_2D = inverse(camera_get_PV(camera_2D));
    vec2 mouse_World_2D = transformPoint(inv_PV_2D, SCREEN.mouse_NDC);
    mat4 M_3D_from_2D = get_M_3D_from_2D();

    bool extruding = ((*enter_mode == EnterMode::ExtrudeAdd) || (*enter_mode == EnterMode::ExtrudeCut));
    bool revolving = ((*enter_mode == EnterMode::RevolveAdd) || (*enter_mode == EnterMode::RevolveCut));
    bool adding     = ((*enter_mode == EnterMode::ExtrudeAdd) || (*enter_mode == EnterMode::RevolveAdd));
    bool cutting     = ((*enter_mode == EnterMode::ExtrudeCut) || (*enter_mode == EnterMode::RevolveCut));

    // FORNOW: repeated computation; TODO function
    bool dxf_anything_selected;
    {
        dxf_anything_selected = false;
        _for_each_selected_entity_ {
            dxf_anything_selected = true;
            break;
        }
    }



    { // preview_extrude_in_length
        real target = (adding) ? *extrude_add_in_length : *extrude_cut_in_length;
        JUICEIT_EASYTWEEN(preview_extrude_in_length, target);
    }
    { // preview_extrude_out_length
        real target = (adding) ? *extrude_add_out_length : *extrude_cut_out_length;
        JUICEIT_EASYTWEEN(preview_extrude_out_length, target);
    }
    // TODO
    { // preview_feature_plane_offset
        real target = (*enter_mode == EnterMode::NudgeFeaturePlane) ? *feature_plane_nudge : 0.0f;
        JUICEIT_EASYTWEEN(preview_feature_plane_offset, target);
    }

    // preview
    vec2 preview_mouse = magic_snap(mouse_World_2D, true);
    vec2 preview_dxf_origin; {
        if (*click_mode != ClickMode::Origin) {
            preview_dxf_origin = dxf->origin;
        } else {
            preview_dxf_origin = preview_mouse;
        }
    }
    vec2 preview_dxf_axis_base_point;
    real preview_dxf_axis_angle_from_y;
    {
        if (*click_mode != ClickMode::Axis) {
            preview_dxf_axis_base_point = WORLD.dxf.axis_base_point;
            preview_dxf_axis_angle_from_y = WORLD.dxf.axis_angle_from_y;
        } else if (!*awaiting_second_click) {
            preview_dxf_axis_base_point = preview_mouse;
            preview_dxf_axis_angle_from_y = WORLD.dxf.axis_angle_from_y;
        } else {
            preview_dxf_axis_base_point = *first_click;
            preview_dxf_axis_angle_from_y = atan2(preview_mouse - preview_dxf_axis_base_point) - PI / 2;
        }
    }


    mat4 P_3D = camera_get_P(&SCREEN.camera_3D);
    mat4 V_3D = camera_get_V(&SCREEN.camera_3D);
    mat4 PV_3D = P_3D * V_3D;

    uint window_width, window_height; {
        real _window_width, _window_height; // FORNOW
        _window_get_size(&_window_width, &_window_height);
        window_width = uint(_window_width);
        window_height = uint(_window_height);
    }

    { // panes
        eso_begin(M4_Identity(), SOUP_LINES, (*mouse_left_drag_pane == Pane::Divider) ? 2.0f : (*hot_pane == Pane::Divider) ? 6.0f : 4.0f, true);
        eso_color((*mouse_left_drag_pane == Pane::Divider) ? monokai.white : (*hot_pane == Pane::Divider) ? monokai.white : monokai.gray);
        // if (SCREEN.hot_pane == Pane::DXF) {
        //     eso_color(monokai.yellow);
        // } else {
        //     eso_color(1.0f, 0.0f, 1.0f);
        // }
        eso_vertex(SCREEN.x_divider_NDC,  1.0f);
        eso_vertex(SCREEN.x_divider_NDC, -1.0f);
        eso_end();
    }

    real x_divider_Pixel = get_x_divider_Pixel();

    { // draw 2D draw 2d draw
        {
            glEnable(GL_SCISSOR_TEST);
            gl_scissor_TODO_CHECK_ARGS(0, 0, x_divider_Pixel, window_height);
        }

        {
            #if 0
            // TODO: section view
            mesh_draw(P_2D, V_2D, inverse(get_M_3D_from_2D()));
            #endif
        }

        {
            if (!SCREEN.hide_grid) { // grid 2D grid 2d grid
                eso_begin(PV_2D, SOUP_LINES, 2.0f);
                eso_color(80.0f / 255, 80.0f / 255, 80.0f / 255);
                for (uint i = 0; i <= uint(GRID_SIDE_LENGTH / GRID_SPACING); ++i) {
                    real tmp = i * GRID_SPACING;
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
            if (1) { // axes 2D axes 2d axes axis 2D axis 2d axes crosshairs cross hairs origin 2d origin 2D origin
                real funky_NDC_factor = SCREEN.camera_2D.height_World / 120.0f;
                real LL = 1000 * funky_NDC_factor;

                eso_color(monokai.white);
                if (0) {
                    eso_begin(PV_2D, SOUP_LINES, 1.5f); {
                        // axis
                        vec2 v = LL * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                        eso_vertex(preview_dxf_axis_base_point + v);
                        eso_vertex(preview_dxf_axis_base_point - v);
                    } eso_end();
                }
                eso_begin(PV_2D, SOUP_LINES, 3.0f); {
                    // origin
                    real r = funky_NDC_factor;
                    eso_vertex(preview_dxf_origin - V2(r, 0));
                    eso_vertex(preview_dxf_origin + V2(r, 0));
                    eso_vertex(preview_dxf_origin - V2(0, r));
                    eso_vertex(preview_dxf_origin + V2(0, r));
                } eso_end();
            }
            { // entities
                eso_begin(PV_2D, SOUP_LINES);
                _for_each_entity_ {
                    ColorCode color_code = (!entity->is_selected) ? entity->color_code : ColorCode::Selection;
                    real dx = 0.0f;
                    real dy = 0.0f;
                    if ((*click_mode == ClickMode::Move) && (*awaiting_second_click)) {
                        if (entity->is_selected) {
                            dx = preview_mouse.x - first_click->x;
                            dy = preview_mouse.y - first_click->y;
                            color_code = ColorCode::WaterOnly;
                        }
                    }
                    eso_color(get_color(color_code));
                    eso_dxf_entity__SOUP_LINES(entity, dx, dy);
                }
                eso_end();
            }
            { // dots
                if (SCREEN.show_details) {
                    eso_begin(camera_get_PV(&SCREEN.camera_2D), SOUP_POINTS, 4.0f);
                    eso_color(monokai.white);
                    _for_each_entity_ {
                        real start_x, start_y, end_x, end_y;
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
                        || (*click_modifier == ClickModifier::Window)
                        ||(*click_mode == ClickMode::Box )
                   ) {
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(basic.cyan);
                    eso_vertex(first_click->x, first_click->y);
                    eso_vertex(preview_mouse.x, first_click->y);
                    eso_vertex(preview_mouse.x, preview_mouse.y);
                    eso_vertex(first_click->x, preview_mouse.y);
                    eso_end();
                }
                if (*click_mode == ClickMode::Measure) { // measure line_entity
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(basic.cyan);
                    eso_vertex(*first_click);
                    eso_vertex(preview_mouse);
                    eso_end();
                }
                if (*click_mode == ClickMode::Line) { // measure line_entity
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(basic.cyan);
                    eso_vertex(*first_click);
                    eso_vertex(preview_mouse);
                    eso_end();
                }
                if (*click_mode == ClickMode::Circle) {
                    vec2 c = { WORLD.two_click_command.first_click.x, WORLD.two_click_command.first_click.y };
                    vec2 p = preview_mouse;
                    real r = norm(c - p);
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(basic.cyan);
                    for (uint i = 0; i < NUM_SEGMENTS_PER_CIRCLE; ++i) eso_vertex(c + r * e_theta(NUM_DEN(i, NUM_SEGMENTS_PER_CIRCLE) * TAU));
                    eso_end();
                }
                if (*click_mode == ClickMode::Fillet) {
                    // FORNOW
                    int i = dxf_find_closest_entity(&WORLD.dxf.entities, WORLD.two_click_command.first_click.x, WORLD.two_click_command.first_click.y);
                    if (i != -1) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(get_color(ColorCode::WaterOnly));
                        eso_dxf_entity__SOUP_LINES(&WORLD.dxf.entities.array[i]);
                        eso_end();
                    }
                }
            }
        }
        glDisable(GL_SCISSOR_TEST);
    }

    { // 3D draw 3D 3d draw 3d
        {
            glEnable(GL_SCISSOR_TEST);
            gl_scissor_TODO_CHECK_ARGS(x_divider_Pixel, 0, window_width - x_divider_Pixel, window_height);
        }

        if (!SCREEN.hide_grid) { // grid 3D grid 3d grid
            conversation_draw_3D_grid_box(P_3D, V_3D);
        }

        if (feature_plane->is_active) { // selection 2d selection 2D selection tube tubes slice slices stack stacks wire wireframe wires frame (FORNOW: ew)
            ;
            // FORNOW
            bool moving_stuff = ((*click_mode == ClickMode::Origin) || (*enter_mode == EnterMode::NudgeFeaturePlane));
            vec3 target_preview_tubes_color = (0) ? V3(0)
                : (adding) ? get_color(ColorCode::Traverse)
                : (cutting) ? get_color(ColorCode::Quality1)
                : (moving_stuff) ? get_color(ColorCode::WaterOnly)
                : monokai.yellow;
            JUICEIT_EASYTWEEN(preview_tubes_color, target_preview_tubes_color);

            uint NUM_TUBE_STACKS_INCLUSIVE;
            mat4 M;
            mat4 M_incr;
            {
                mat4 T_o = M4_Translation(preview_dxf_origin);
                mat4 inv_T_o = M4_Translation(-preview_dxf_origin);
                if (extruding) {
                    real a = -*preview_extrude_in_length;
                    real L = *preview_extrude_out_length + *preview_extrude_in_length;
                    NUM_TUBE_STACKS_INCLUSIVE = MIN(64, uint(roundf(L / 2.5f)) + 2);
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0.0f, 0.0f, a + Z_FIGHT_EPS);
                    M_incr = M4_Translation(0.0f, 0.0f, L / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                } else if (revolving) {
                    NUM_TUBE_STACKS_INCLUSIVE = 64;
                    M = M_3D_from_2D * inv_T_o;
                    { // M_incr
                        real a = 0.0f;
                        real b = TAU;
                        mat4 R_a = M4_RotationAbout(V3(e_theta(PI / 2 + preview_dxf_axis_angle_from_y), 0.0f), (b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                        mat4 T_a = M4_Translation(V3(preview_dxf_axis_base_point, 0.0f));
                        mat4 inv_T_a = inverse(T_a);
                        M_incr = T_o * T_a * R_a * inv_T_a * inv_T_o;
                    }
                } else if (*click_mode == ClickMode::Origin) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0, 0, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else if (*enter_mode == EnterMode::NudgeFeaturePlane) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0.0f, 0.0f, *preview_feature_plane_offset + Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else { // default
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0, 0, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                }

                for (uint tube_stack_index = 0; tube_stack_index < NUM_TUBE_STACKS_INCLUSIVE; ++tube_stack_index) {
                    eso_begin(PV_3D * M, SOUP_LINES, 5.0f); {
                        _for_each_selected_entity_ {
                            real alpha;
                            vec3 color;
                            // if (entity->is_selected) {
                            alpha = CLAMP(-0.2f + 3.0f * MIN(entity->time_since_is_selected_changed, time_since->plane_selected), 0.0f, 1.0f);
                            color = CLAMPED_LERP(-0.5f + SQRT(2.0f * entity->time_since_is_selected_changed), monokai.white, *preview_tubes_color);
                            // } else {
                            //     alpha = CLAMPED_LERP(5.0f * entity->time_since_is_selected_changed, 1.0f, 0.0f);
                            //     color = get_color(color);
                            // }
                            eso_color(color, alpha);
                            eso_dxf_entity__SOUP_LINES(entity);
                        }
                    } eso_end();
                    M *= M_incr;
                }
            }
        }

        #if 0
        if (dxf_anything_selected) { // arrow
            if ((*enter_mode == EnterMode::ExtrudeAdd) || (*enter_mode == ENTER_MODE_EXTRUDE_CUT) || (*enter_mode == EnterMode::RevolveAdd) || (WORLD.modes.enter_mode == ENTER_MODE_REVOLVE_CUT)) {

                vec3 color = ((*enter_mode == EnterMode::ExtrudeAdd) || (*enter_mode == EnterMode::RevolveAdd)) ? V3(83.0f / 255, 255.0f / 255, 83.0f / 255.0f) : V3(1.0f, 0.0f, 0.0f);

                real arrow_x = 0.0f;
                real arrow_y = 0.0f;
                real H[2] = { WORLD.popup.param0, WORLD.popup.param1 };
                bool toggle[2] = { WORLD.console.flip_flag, !WORLD.console.flip_flag };
                mat4 A = M_3D_from_2D;
                if ((*enter_mode == EnterMode::ExtrudeAdd) || (*enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                    bounding_box_center(_selection_bounding_box, &arrow_x, &arrow_y);
                    if (dxf_anything_selected) {
                        arrow_x -= WORLD.dxf.origin.x;
                        arrow_y -= WORLD.dxf.origin.y;
                    }
                } else { ASSERT((*enter_mode == EnterMode::RevolveAdd) || (*enter_mode == ENTER_MODE_REVOLVE_CUT));
                    H[0] = 10.0f + _selection_bounding_box.max[(!WORLD.console.flip_flag) ? 1 : 0];
                    H[1] = 0.0f;
                    toggle[0] = false;
                    toggle[1] = false;
                    { // A
                        A *= M4_RotationAboutXAxis(RAD(-90.0f));
                        if (WORLD.console.flip_flag) {
                            A *= M4_RotationAboutYAxis(RAD(90.0f));
                        }
                    }
                }

                for (uint i = 0; i < 2; ++i) {
                    if (!IS_ZERO(H[i])) {
                        real total_height = ABS(H[i]);
                        real cap_height = (total_height > 10.0f) ? 5.0f : (0.5f * total_height);
                        real shaft_height = total_height - cap_height;
                        real s = 1.5f;
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
            real r = SCREEN.camera_3D.ortho_screen_height_World / 120.0f;
            eso_color(monokai.white);
            eso_begin(PV_3D * M_3D_from_2D * M4_Translation(0.0f, 0.0f, Z_FIGHT_EPS), SOUP_LINES, 2.0f);
            eso_vertex(-r, 0.0f);
            eso_vertex( r, 0.0f);
            eso_vertex(0.0f, -r);
            eso_vertex(0.0f,  r);
            if (0) {
                // TODO: clip this to the feature_plane
                real LL = 10.0f;
                vec2 v = LL * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                vec2 a = preview_dxf_axis_base_point + v;
                vec2 b = preview_dxf_axis_base_point - v;
                eso_vertex(-preview_dxf_origin + a);
                eso_vertex(-preview_dxf_origin + b); // FORNOW
            }
            eso_end();
        }


        { // feature plane feature-plane feature_plane
            box2 face_selection_bounding_box = mesh_draw(P_3D, V_3D, M4_Identity());
            box2 dxf_selection_bounding_box = dxf_entities_get_bounding_box(&WORLD.dxf.entities, true);
            box2 target_bounding_box; {
                target_bounding_box = bounding_box_union(face_selection_bounding_box, dxf_selection_bounding_box);
                for (uint d = 0; d < 2; ++d) {
                    if (target_bounding_box.min[d] > target_bounding_box.max[d]) {
                        target_bounding_box.min[d] = 0.0f;
                        target_bounding_box.max[d] = 0.0f;
                    }
                }
                {
                    real eps = 10.0f;
                    target_bounding_box.min[0] -= eps;
                    target_bounding_box.max[0] += eps;
                    target_bounding_box.min[1] -= eps;
                    target_bounding_box.max[1] += eps;
                }
            }
            JUICEIT_EASYTWEEN(&preview_feature_plane->min, target_bounding_box.min);
            JUICEIT_EASYTWEEN(&preview_feature_plane->max, target_bounding_box.max);
            if (time_since->plane_selected == 0.0f) { // FORNOW
                *preview_feature_plane = target_bounding_box;
            }
        }

        if (feature_plane->is_active) { // floating sketch plane; selection plane NOTE: transparent
            bool draw = true;
            mat4 PVM = PV_3D * M_3D_from_2D;
            vec3 target_feature_plane_color = monokai.yellow;
            real sign = -1.0f;
            {
                if (*enter_mode == EnterMode::NudgeFeaturePlane) {
                    PVM *= M4_Translation(-WORLD.dxf.origin.x, -WORLD.dxf.origin.y, *preview_feature_plane_offset);
                    target_feature_plane_color = { 0.0f, 1.0f, 1.0f };
                    sign = 1.0f;
                    draw = true;
                } else if (*click_mode == ClickMode::Origin) {
                    target_feature_plane_color = { 0.0f, 1.0f, 1.0f };
                    sign = 1.0f;
                    draw = true;
                } else {
                    if (dxf_anything_selected) PVM *= M4_Translation(-WORLD.dxf.origin.x, -WORLD.dxf.origin.y, 0.0f); // FORNOW
                }
            }

            JUICEIT_EASYTWEEN(preview_feature_plane_color, target_feature_plane_color);

            if (draw) {
                real f = CLAMPED_LERP(SQRT(3.0f * time_since->plane_selected), 0.0f, 1.0f);
                eso_begin(PVM, SOUP_QUADS);
                eso_color(*preview_feature_plane_color, f * 0.35f);
                vec2 center = (preview_feature_plane->max + preview_feature_plane->min) / 2.0f;
                vec2 radius = LERP(f, 0.5f, 1.0f) * (preview_feature_plane->max - preview_feature_plane->min) / 2.0f;
                eso_vertex(center.x + radius.x, center.y + radius.y, sign * Z_FIGHT_EPS);
                eso_vertex(center.x + radius.x, center.y - radius.y, sign * Z_FIGHT_EPS);
                eso_vertex(center.x - radius.x, center.y - radius.y, sign * Z_FIGHT_EPS);
                eso_vertex(center.x - radius.x, center.y + radius.y, sign * Z_FIGHT_EPS);
                eso_end();
            }
        }


        glDisable(GL_SCISSOR_TEST);
    }

    if (!SCREEN.hide_gui) { // gui


        { // cursor decorations
            real a = ((*hot_pane == Pane::DXF) && ((*mouse_left_drag_pane == Pane::None) || (*mouse_left_drag_pane == Pane::DXF))) ? 1.0f : 0.5f;
            real r, g, b;
            r = g = b = 1.0f;
            char _COLOR_X[64] = {};
            if (*click_mode == ClickMode::Color) {
                if (*click_modifier == ClickModifier::Selected) {
                    sprintf(_COLOR_X, "COLOR");
                } else {
                    sprintf(_COLOR_X, "COLOR %d", *click_color_code);
                    vec3 rgb = get_color(*click_color_code);
                    r = rgb[0];
                    g = rgb[1];
                    b = rgb[2];
                }
            }
            char _X_Y[256] = {};
            if (*click_modifier == ClickModifier::XYCoordinates) {
                sprintf(_X_Y, "(%g, %g)", *x_coordinate, *y_coordinate);
            }

            _text_draw(
                    (cow_real *) transform_NDC_from_Pixel,
                    (char *) (
                        (*click_mode == ClickMode::None) ? "" :
                        (*click_mode == ClickMode::Axis)     ? "AXIS" :
                        (*click_mode == ClickMode::Box)      ? "BOX" :
                        (*click_mode == ClickMode::Circle)   ? "CIRCLE" :
                        (*click_mode == ClickMode::Color)    ? _COLOR_X :
                        (*click_mode == ClickMode::Deselect) ? "DESELECT" :
                        (*click_mode == ClickMode::Fillet)   ? "FILLET" :
                        (*click_mode == ClickMode::Line)     ? "LINE" :
                        (*click_mode == ClickMode::Measure)  ? "MEASURE" :
                        (*click_mode == ClickMode::Move)     ? "MOVE" :
                        (*click_mode == ClickMode::Origin)   ? "ORIGIN" :
                        (*click_mode == ClickMode::Select)   ? "SELECT" :
                        (*click_mode == ClickMode::MirrorX) ? "X_MIRROR" :
                        (*click_mode == ClickMode::MirrorY) ? "Y_MIRROR" :
                        "???MODE???"),
                    mouse_Pixel->x + 12,
                    mouse_Pixel->y + 14,
                    0.0,

                    r,
                    g,
                    b,
                    a,

                    12,
                    0.0,
                    0.0,
                    true);

            _text_draw(
                    (cow_real *) transform_NDC_from_Pixel,
                    (char *) (
                        (*click_modifier == ClickModifier::None)                  ? "" :
                        (*click_modifier == ClickModifier::Center)     ? "CENTER" :
                        (*click_modifier == ClickModifier::Connected)             ? "CONNECTED" :
                        (*click_modifier == ClickModifier::End)        ? "END" :
                        (*click_modifier == ClickModifier::Color)                 ? "COLOR" :
                        (*click_modifier == ClickModifier::Middle)     ? "MIDDLE" :
                        (*click_modifier == ClickModifier::Selected)              ? "SELECTED" :
                        (*click_modifier == ClickModifier::Window)                ? "WINDOW" :
                        (*click_modifier == ClickModifier::XYCoordinates) ? _X_Y :
                        "???MODIFIER???"),
                    mouse_Pixel->x + 12,
                    mouse_Pixel->y + 24,
                    0.0,

                    1.0,
                    1.0,
                    1.0,
                    a,

                    12,
                    0.0,
                    0.0,
                    true);
        }

        // gui_printf("[Enter] %s",
        //         (*enter_mode == EnterMode::ExtrudeAdd) ? "EXTRUDE_ADD" :
        //         (*enter_mode == ENTER_MODE_EXTRUDE_CUT) ? "EXTRUDE_CUT" :
        //         (*enter_mode == EnterMode::RevolveAdd) ? "REVOLVE_ADD" :
        //         (*enter_mode == ENTER_MODE_REVOLVE_CUT) ? "REVOLVE_CUT" :
        //         (*enter_mode == ENTER_MODE_OPEN) ? "OPEN" :
        //         (*enter_mode == ENTER_MODE_SAVE) ? "SAVE" :
        //         (*enter_mode == ENTER_MODE_ORIGIN) ? "SET_ORIGIN" :
        //         (*enter_mode == EnterMode::NudgeFeaturePlane) ? "OFFSET_PLANE_TO" :
        //         (*enter_mode == EnterMode::None) ? "NONE" :
        //         "???ENTER???");

        if (SCREEN.show_help) {
            { // overlay
                eso_begin(M4_Identity(), SOUP_QUADS, 0.0f, true);
                eso_color(0.0f, 0.0f, 0.0f, 0.7f);
                eso_vertex(-1.0f, -1.0f);
                eso_vertex(-1.0f,  1.0f);
                eso_vertex( 1.0f,  1.0f);
                eso_vertex( 1.0f, -1.0f);
                eso_end();
            }
            gui_printf("TODO: update help popup");
            #if 0
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
            #endif
        }
    }
}

//////////////////////////////////////////////////
// TODO: SPOOF ///////////////////////////////////
//////////////////////////////////////////////////
void script_process(char *string) {
    // TODO: gui
    // TODOLATER (weird 'X' version): char *string = "^osplash.dxf\nyscx2020\ne\t50";
    #define TAG_LENGTH 3
    bool control = false;
    for (uint i = 0; string[i]; ++i) {
        char c = string[i];
        if (c == '^') {
            control = true;
        } else if (c == '<') {
            bool is_instabaked = true;
            Event instabaked_event = {};
            RawEvent _raw_event = {};
            {
                uint next_i; {
                    next_i = i;
                    while (string[++next_i] != '>') {} // ++next_i intentional
                }
                {
                    char *tag = &string[i + 1];
                    if (strncmp(tag, "m2d", TAG_LENGTH) == 0) {
                        char *params = &string[i + 1 + TAG_LENGTH];
                        vec2 p; {
                            sscanf(params, "%f %f", &p.x, &p.y);
                            // NOTE: user of this api specified click in pre-snapped world coordinates
                            //       (as opposed to [pre-snapped] pixel coordinates, which is what bake_event takes)
                            p = magic_snap(p);
                        }
                        instabaked_event = construct_mouse_event_2D(p);
                    } else if (strncmp(tag, "m3d", TAG_LENGTH) == 0) {
                        char *params = &string[i + 1 + TAG_LENGTH];
                        vec3 mouse_ray_origin;
                        vec3 mouse_ray_direction;
                        sscanf(params, "%f %f %f %f %f %f", &mouse_ray_origin.x, &mouse_ray_origin.y, &mouse_ray_origin.z, &mouse_ray_direction.x, &mouse_ray_direction.y, &mouse_ray_direction.z);
                        instabaked_event = construct_mouse_event_3D(mouse_ray_origin, mouse_ray_direction);
                    } else if (strncmp(tag, "esc", TAG_LENGTH) == 0) {
                        is_instabaked = false;
                        _raw_event.type = EventType::Key;
                        _raw_event.key = GLFW_KEY_ESCAPE;
                    }
                }
                i = next_i;
            }
            if (is_instabaked) {
                freshly_baked_event_process(instabaked_event);
            } else {
                Event freshly_baked_event = bake_event(_raw_event);
                freshly_baked_event_process(freshly_baked_event);
            }
        } else {
            RawEvent raw_event = {};
            raw_event.type = EventType::Key;
            raw_event.control = control;
            {
                if ('a' <= c && c <= 'z') {
                    raw_event.key = 'A' + (c - 'a');
                } else if ('A' <= c && c <= 'Z') {
                    raw_event.shift = true;
                    raw_event.key = c;
                } else if (c == '{') {
                    raw_event.shift = true;
                    raw_event.key = '[';
                } else if (c == '}') {
                    raw_event.shift = true;
                    raw_event.key = ']';
                } else if (c == '\n') {
                    raw_event.key = GLFW_KEY_ENTER;
                } else if (c == '\t') {
                    raw_event.key = GLFW_KEY_TAB;
                } else {
                    raw_event.key = c;
                }
            }
            control = false;
            Event freshly_baked_event = bake_event(raw_event);
            freshly_baked_event_process(freshly_baked_event);
        }
    }
}

//////////////////////////////////////////////////
// MAIN() ////////////////////////////////////////
//////////////////////////////////////////////////

int main() {
    // _window_set_size(1.5 * 640.0, 1.5 * 360.0); // TODO the first frame crap gets buggered

    glfwHideWindow(COW0._window_glfw_window);

    char *script;
    // script = "cz0123456789";
    // script = "^osplash.dxf\nysc<m2d 20 20><m2d 16 16><m2d 16 -16><m2d -16 -16><m2d -16 16>[50\n<m3d 0 100 0 0 -1 0><m2d 0 17.5>{47\nc<m2d 16 -16>\t\t100\nsc<m2d 32 -16><m3d 74 132 113 -0.4 -0.6 -0.7>{60\n^oomax.dxf\nsq0sq1y[3\n";
    #if 1
    script = \
             "cz10\n"
             "cz\t10\n"
             "bzx30\t30\n"
             "ysadcz"
             "[5\t15\n"
             "sc<m2d 0 30>qs3"
             "1<m2d 30 15>0<esc>"
             "sq1sq3me<m2d 40 40>x15\t15\n"
             "{3\n"
             "sczZm<m2d -50 0>]\n"
             "^n"
             "cx30\t30\n3.4\n"
             "saXzYzXzsa[1\n"
             "^osplash.dxf\nsc<m2d 24 0><m2d 16 0>[\t10\n"
             "Ac<m2d 15.3 15.4>c<m2d -16.4 -16.3>sc<m2d -16 16>]\n"
             "^n"
             "l<m2d 0 0><m2d 0 10>l<m2d 0 10><m2d 10 0>l<m2d 10 0><m2d 0 0>"
             "n25\n"
             "sa[1\n"
             "n0\n"
             "^n"
             "cz8\n"
             "<m3d 1 100 -1 0 -1 0>"
             "sa{100\n"
             ";"
             "^odemo.dxf\n"
             "^sz.stl\n"
             "^oz.stl\n"
             ;
    #endif
    script_process(script);

    init_cameras(); // FORNOW

    {
        // FORNOW: first frame position
        double xpos, ypos;
        glfwGetCursorPos(COW0._window_glfw_window, &xpos, &ypos);
        callback_cursor_position(NULL, xpos, ypos);
    }

    uint frame = 0;
    while (cow_begin_frame()) {
        *transform_NDC_from_Pixel = window_get_NDC_from_Screen();

        SCREEN.DONT_DRAW_ANY_MORE_POPUPS_THIS_FRAME = false;

        if (SCREEN.stepping_one_frame_while_paused) {
            *paused = false;
        }

        if (!SCREEN.paused) { // update
            { // time_since
                real dt = 0.0167f;
                _for_each_entity_ entity->time_since_is_selected_changed += dt;
                time_since->cursor_start += dt;
                time_since->successful_feature += dt;
                time_since->plane_selected += dt;
                // time_since->successful_feature = 1.0f;

                time_since->going_inside += dt;
                bool _going_inside_next = 
                    ((*enter_mode == EnterMode::ExtrudeAdd) && (*extrude_add_in_length > 0.0f))
                    ||
                    (*enter_mode == EnterMode::ExtrudeCut);
                if (_going_inside_next && !time_since->_helper_going_inside) {
                    time_since->going_inside = 0.0f;
                }
                time_since->_helper_going_inside = _going_inside_next;
            }

            { // events
                if (raw_user_event_queue.length) {
                    while (raw_user_event_queue.length) {
                        RawEvent raw_event = queue_dequeue(&raw_user_event_queue);
                        Event freshly_baked_event = bake_event(raw_event);
                        freshly_baked_event_process(freshly_baked_event);
                    }
                } else {
                    // NOTE: this is so we draw the popups
                    freshly_baked_event_process({});
                }
            }

            conversation_message_buffer_update();
        } else {
            freshly_baked_event_process({});
        }

        { // draw
            conversation_draw(); // FORNOW: moving this down here (be wary of frame imperfections)
            if (SCREEN.show_event_stack) history_debug_draw();
            conversation_message_buffer_draw();

            if (SCREEN.paused) {
                eso_begin(SCREEN.transform_NDC_from_Pixel, SOUP_QUADS);
                eso_color(omax.white, 0.5f);
                real x = 16;
                real y = 16;
                real w = 16;
                real h = 2.5f * w;
                for (uint d = 0; d < 2; ++d) {
                    real o = d * (1.7f * w);
                    eso_vertex(o + x,     y    );
                    eso_vertex(o + x,     y + h);
                    eso_vertex(o + x + w, y + h);
                    eso_vertex(o + x + w, y    );
                }
                eso_end();
            }
        }

        if (frame++ == 1) glfwShowWindow(COW0._window_glfw_window);

        if (SCREEN.stepping_one_frame_while_paused) {
            SCREEN.stepping_one_frame_while_paused = false;
            SCREEN.paused = true;
        }
    }
}


