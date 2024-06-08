// TODO: memcmp to see if should record
// TODO: timer to see if should snapshot
// TODO: copy over cow and snail into the working directory and delete the parts you don't need
// TODO: strip out all the uneeded functions
// TODO: replace all cow macros with teplated functions

#include "include.cpp" // (state-independent)

// (global) state
WorldState_ChangesToThisMustBeRecorded_state state;
ScreenState_ChangesToThisDo_NOT_NeedToBeRecorded_other other;

// pointers to shorten xxx.foo.bar into foo->bar
Drawing *drawing = &state.drawing;
FeaturePlaneState *feature_plane = &state.feature_plane;
Mesh *mesh = &state.mesh;
PopupState *popup = &state.popup;
TwoClickCommandState *two_click_command = &state.two_click_command;
Camera *camera_drawing = &other.camera_drawing;
Camera *camera_mesh = &other.camera_mesh;
PreviewState *preview = &other.preview;

// // macros
// entity
#define _for_each_entity_ for (\
        Entity *entity = drawing->entities.array;\
        entity < &drawing->entities.array[drawing->entities.length];\
        ++entity)
#define _for_each_selected_entity_ _for_each_entity_ if (entity->is_selected)


// // tween
template <typename T> void JUICEIT_EASYTWEEN(T *a, T b) {
    real f = 0.1f;
    if (!other.paused) *a += f * (b - *a);
}
real _JUICEIT_EASYTWEEN(real t) { return 0.287f * log(t) + 1.172f; }


#include "popup.cpp"

//////////////////////////////////////////////////
// NON-ZERO INITIALIZERS /////////////////////////
//////////////////////////////////////////////////

void init_cameras() {
    *camera_drawing = make_Camera2D(100.0f, {}, { -0.5f, -0.125f });
    if (drawing->entities.length) camera2D_zoom_to_bounding_box(&other.camera_drawing, dxf_entities_get_bounding_box(&drawing->entities));
    if ((!other.camera_mesh.persp_distance_to_origin_World) || (!mesh->num_vertices)) {
        *camera_mesh = make_OrbitCamera3D(
                CAMERA_3D_PERSPECTIVE_ANGLE_OF_VIEW,
                2.0f * MIN(150.0f, other.camera_drawing.ortho_screen_height_World),
                { RAD(-44.0f), RAD(33.0f) },
                {},
                { 0.5f, -0.125f }
                );
    }
}

//////////////////////////////////////////////////
// GETTERS (state NOT WORTH TROUBLE OF STORING) //
//////////////////////////////////////////////////
// getters


// fornow

bool click_mode_SELECT_OR_DESELECT() {
    return ((state.click_mode == ClickMode::Select) || (state.click_mode == ClickMode::Deselect));
}

bool _non_WINDOW__SELECT_DESELECT___OR___SET_COLOR() {
    return ((click_mode_SELECT_OR_DESELECT() && (state.click_modifier != ClickModifier::Window)) || (state.click_mode == ClickMode::Color));
}

bool _SELECT_OR_DESELECT_COLOR() {
    bool A = click_mode_SELECT_OR_DESELECT();
    bool B = (state.click_modifier == ClickModifier::Color);
    return A && B;
}

bool click_mode_SNAP_ELIGIBLE() {
    return 0
        || (state.click_mode == ClickMode::Axis)
        || (state.click_mode == ClickMode::Box)
        || (state.click_mode == ClickMode::Circle)
        || (state.click_mode == ClickMode::Line)
        || (state.click_mode == ClickMode::Measure)
        || (state.click_mode == ClickMode::Move)
        || (state.click_mode == ClickMode::Origin)
        || (state.click_mode == ClickMode::MirrorX)
        || (state.click_mode == ClickMode::MirrorY)
        ;
}

bool click_mode_SPACE_BAR_REPEAT_ELIGIBLE() {
    return 0
        || (state.click_mode == ClickMode::Axis)
        || (state.click_mode == ClickMode::Box)
        || (state.click_mode == ClickMode::Circle)
        || (state.click_mode == ClickMode::Fillet)
        || (state.click_mode == ClickMode::Line)
        || (state.click_mode == ClickMode::Measure)
        || (state.click_mode == ClickMode::Move)
        || (state.click_mode == ClickMode::Origin)
        || (state.click_mode == ClickMode::MirrorX)
        || (state.click_mode == ClickMode::MirrorY)
        ;
}

bool enter_mode_SHIFT_SPACE_BAR_REPEAT_ELIGIBLE() {
    return 0
        || (state.enter_mode == EnterMode::ExtrudeAdd)
        || (state.enter_mode == EnterMode::ExtrudeCut)
        || (state.enter_mode == EnterMode::RevolveAdd)
        || (state.enter_mode == EnterMode::RevolveCut)
        || (state.enter_mode == EnterMode::NudgeFeaturePlane)
        ;
}
//////////////////////////////////////////////////
// state-DEPENDENT UTILITY FUNCTIONS /////////////
//////////////////////////////////////////////////

vec2 magic_snap(vec2 before, bool calling_this_function_for_drawing_preview = false) {
    vec2 result = before;
    {
        if (
                ( 0 
                  || (state.click_mode == ClickMode::Line)
                  || (state.click_mode == ClickMode::Axis)
                )
                && (two_click_command->awaiting_second_click)
                && (other.shift_held)) {
            vec2 a = two_click_command->first_click;
            vec2 b = before;
            vec2 r = b - a; 
            real norm_r = norm(r);
            real factor = 360 / 15 / TAU;
            real theta = roundf(atan2(r) * factor) / factor;
            result = a + norm_r * e_theta(theta);
        } else if (
                (state.click_mode == ClickMode::Box)
                && (two_click_command->awaiting_second_click)
                && (other.shift_held)) {
            // TODO (Felipe): snap square
            result = before;
        } else if (!calling_this_function_for_drawing_preview) { // NOTE: this else does, in fact, match LAYOUT's behavior
            if (state.click_modifier == ClickModifier::Center) {
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
            } else if (state.click_modifier == ClickModifier::Middle) {
                real min_squared_distance = HUGE_VAL;
                _for_each_entity_ {
                    real squared_distance = squared_distance_point_dxf_entity(before.x, before.y, entity);
                    if (squared_distance < min_squared_distance) {
                        min_squared_distance = squared_distance;
                        entity_get_middle(entity, &result.x, &result.y);
                    }
                }
            } else if (state.click_modifier == ClickModifier::End) {
                real min_squared_distance = HUGE_VAL;
                _for_each_entity_ {
                    real x[2], y[2];
                    entity_get_start_and_end_points(entity, &x[0], &y[0], &x[1], &y[1]);
                    for_(d, 2) {
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

real get_x_divider_Pixel() {
    return LINEAR_REMAP(other.x_divider_NDC, -1.0f, 1.0f, 0.0f, window_get_size_Pixel().x);
}

///

#include "draw.cpp"

//////////////////////////////////////////////////
// LOADING AND SAVING state TO/FROM DISK /////////
//////////////////////////////////////////////////

void conversation_dxf_load(char *filename, bool preserve_cameras_and_dxf_origin = false) {
    if (!poe_file_exists(filename)) {
        messagef(omax.orange, "File \"%s\" was not found.", filename);
        return;
    }

    list_free_AND_zero(&drawing->entities);

    dxf_entities_load(filename, &drawing->entities);

    if (!preserve_cameras_and_dxf_origin) {
        // camera2D_zoom_to_bounding_box(&other.camera_drawing, dxf_entities_get_bounding_box(&drawing->entities));
        init_cameras();
        drawing->origin = {};
    }

    messagef(omax.green, "LoadDXF \"%s\"", filename);
}

void conversation_stl_load(char *filename, bool preserve_cameras = false) {
    if (!poe_file_exists(filename)) {
        messagef(omax.orange, "File \"%s\" was not found.", filename);
        return;
    }
    {
        stl_load(filename, mesh);
        FORNOW_UNUSED(preserve_cameras);
        messagef(omax.green, "LoadSTL \"%s\"", filename);
    }
}

void conversation_load(char *filename, bool preserve_cameras = false) {
    if (poe_suffix_match(filename, ".dxf")) {
        conversation_dxf_load(filename, preserve_cameras);
    } else if (poe_suffix_match(filename, ".stl")) {
        conversation_stl_load(filename, preserve_cameras);
    } else {
        messagef(omax.orange, "File \"%s\" is not supported: must be *.drawing or *.stl. ", filename);
    }
}

void conversation_stl_save(char *filename) {
    // TODO: prompt for overwriting
    if (mesh_save_stl(mesh, filename) ) {
        messagef(omax.green, "SaveSTL \"%s\"", filename);
    } else {
        messagef(omax.orange, "Failed to open file \"%s\" for writing.", filename);
    }
}

void conversation_save(char *filename) {
    if (poe_suffix_match(filename, ".stl")) {
        conversation_stl_save(filename);
    } else {
        messagef(omax.orange, "File \"%s\" is not supported: must be *.drawing or *.stl. ", filename);
    }
}

void callback_drop(GLFWwindow *, int count, const char **paths) {
    if (count > 0) {
        char *filename = (char *)(paths[0]);
        conversation_load(filename);
        { // conversation_set_drop_path(filename)
            for (uint i = 0; filename[i]; ++i) {
                other.drop_path[i] = filename[i];
                if (filename[i] == '.') {
                    while (
                            (i != 0) &&
                            (other.drop_path[i - 1] != '\\') &&
                            (other.drop_path[i - 1] != '/')
                          ) --i;
                    other.drop_path[i] = '\0';
                    break;
                }
            }
        }
    }
}


//////////////////////////////////////////////////
// GATHERING NEW EVENTS FROM USER ////////////////
//////////////////////////////////////////////////

Queue<RawEvent> raw_user_event_queue;

void callback_key(GLFWwindow *, int key, int, int action, int mods) {
    bool control = (mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER));
    bool shift = (mods & GLFW_MOD_SHIFT);

    if (key == GLFW_KEY_LEFT_SHIFT) {
        if (action == GLFW_PRESS) {
            other.shift_held = true;
        } else if (action == GLFW_RELEASE) {
            other.shift_held = false;
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
        bool toggle_pause; {
            toggle_pause = false;
            if (!((popup->_active_popup_unique_ID__FORNOW_name0) && (popup->cell_type[popup->active_cell_index] == CellType::String))) { // FORNOW
                toggle_pause = ((key == 'P') && (!control) && (!shift));
            }
        }
        bool step = (other.paused) && ((key == '.') && (!control) && (!shift));
        bool quit = ((key == 'Q') && (control) && (!shift));
        if (toggle_pause) {
            other.paused = !other.paused;
        } else if (step) {
            other.stepping_one_frame_while_paused = true;
        } else if (quit) {
            glfwSetWindowShouldClose(glfw_window, true);
        } else {
            RawEvent raw_event = {}; {
                raw_event.type = EventType::Key;
                RawKeyEvent *raw_key_event = &raw_event.raw_key_event;
                raw_key_event->key = key;
                raw_key_event->control = control;
                raw_key_event->shift = shift;
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
        real x = other.mouse_Pixel.x;
        if (popup->FORNOW_info_mouse_is_hovering) {
            other.hot_pane = Pane::Popup;
        } else if (x < x_divider_Pixel - eps) {
            other.hot_pane = Pane::Drawing;
        } else if (x < x_divider_Pixel + eps) {
            other.hot_pane = Pane::Separator;
        } else {
            other.hot_pane = Pane::Mesh;
        }
    }


    // TODO: is a Pane::Popup a bad idea?--maybe just try it??

    // mouse_*
    vec2 delta_mouse_NDC;
    vec2 delta_mouse_World_2D;
    {
        vec2 prev_mouse_NDC = other.mouse_NDC;
        other.mouse_Pixel = { real(xpos), real(ypos) };
        other.mouse_NDC = transformPoint(other.transform_NDC_from_Pixel, other.mouse_Pixel);
        delta_mouse_NDC = other.mouse_NDC - prev_mouse_NDC;
        delta_mouse_World_2D = transformVector(inverse(camera_get_PV(camera_drawing)), delta_mouse_NDC);
    }

    { // special draggin mouse_held EventType::Mouse
        if (0
                || (other.mouse_left_drag_pane == Pane::Drawing)
                || (other.mouse_left_drag_pane == Pane::Popup)
           ) {
            RawEvent raw_event; {
                raw_event = {};
                raw_event.type = EventType::Mouse;
                RawMouseEvent *raw_mouse_event = &raw_event.raw_mouse_event;
                raw_mouse_event->pane = other.mouse_left_drag_pane;
                raw_mouse_event->mouse_Pixel = other.mouse_Pixel;
                raw_mouse_event->mouse_held = true;
            }
            queue_enqueue(&raw_user_event_queue, raw_event);
        }
    }

    { // dragging divider
        if (other.mouse_left_drag_pane == Pane::Separator) {
            real prev_x_divider_NDC = other.x_divider_NDC;
            other.x_divider_NDC = MAG_CLAMP(LINEAR_REMAP(xpos, 0.0f, window_get_size_Pixel().x, -1.0f, 1.0f), 0.9975f);
            real delta_NDC = 0.5f * (other.x_divider_NDC - prev_x_divider_NDC);
            camera_drawing->post_nudge_NDC.x += delta_NDC;
            camera_mesh->post_nudge_NDC.x += delta_NDC;
        }
    }

    { // moving cameras
        // mouse_left_drag_pane
        if (other.mouse_left_drag_pane == Pane::Mesh) {
            real fac = 2.0f;
            camera_mesh->euler_angles.y -= fac * delta_mouse_NDC.x;
            camera_mesh->euler_angles.x += fac * delta_mouse_NDC.y;
            camera_mesh->euler_angles.x = CLAMP(camera_mesh->euler_angles.x, -RAD(90), RAD(90));
        }

        // mouse_right_drag_pane
        if (other.mouse_right_drag_pane == Pane::Drawing) {
            camera_drawing->pre_nudge_World -= delta_mouse_World_2D;
        } else if (other.mouse_right_drag_pane == Pane::Mesh) {
            Camera tmp_2D = make_EquivalentCamera2D(camera_mesh);
            tmp_2D.pre_nudge_World -= transformVector(inverse(camera_get_PV(&tmp_2D)), delta_mouse_NDC);
            camera_mesh->pre_nudge_World = tmp_2D.pre_nudge_World;
        }
    }
}

// NOTE: mouse does not have GLFW_REPEAT
void callback_mouse_button(GLFWwindow *, int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            other.mouse_left_drag_pane = other.hot_pane;
            RawEvent raw_event; {
                raw_event = {};
                raw_event.type = EventType::Mouse;
                RawMouseEvent *raw_mouse_event = &raw_event.raw_mouse_event;
                raw_mouse_event->pane = other.hot_pane;
                raw_mouse_event->mouse_Pixel = other.mouse_Pixel;
            }
            queue_enqueue(&raw_user_event_queue, raw_event);
        } else { ASSERT(action == GLFW_RELEASE);
            other.mouse_left_drag_pane = Pane::None;
        }
    } else { ASSERT(button == GLFW_MOUSE_BUTTON_RIGHT);
        if (action == GLFW_PRESS) {
            other.mouse_right_drag_pane = other.hot_pane;
        } else { ASSERT(action == GLFW_RELEASE);
            other.mouse_right_drag_pane = Pane::None;
        }
    }
}

void _callback_scroll_helper(Camera *camera_2D, double yoffset) {
    // IDEA: preserve mouse position
    ASSERT(IS_ZERO(camera_2D->angle_of_view));
    ASSERT(IS_ZERO(camera_2D->euler_angles));
    vec2 mouse_position_before  = transformPoint(inverse(camera_get_PV(camera_2D)), other.mouse_NDC);
    camera_2D->ortho_screen_height_World *= (1.0f - 0.1f * yoffset);
    vec2 mouse_position_after = transformPoint(inverse(camera_get_PV(camera_2D)), other.mouse_NDC);
    camera_2D->pre_nudge_World -= (mouse_position_after - mouse_position_before);
}
void callback_scroll(GLFWwindow *, double, double yoffset) {
    if (other.hot_pane == Pane::Drawing) {
        _callback_scroll_helper(camera_drawing, yoffset);
    } else if (other.hot_pane == Pane::Mesh) {
        // IDEA: preserve mouse position in the rasterized image
        bool is_perspective_camera = (!IS_ZERO(camera_mesh->angle_of_view));
        Camera tmp_2D = make_EquivalentCamera2D(camera_mesh);
        _callback_scroll_helper(&tmp_2D, yoffset);
        if (is_perspective_camera) {
            camera_mesh->persp_distance_to_origin_World = 0.5f * (tmp_2D.ortho_screen_height_World / TAN(camera_mesh->angle_of_view / 2));
        } else {
            camera_mesh->ortho_screen_height_World = tmp_2D.ortho_screen_height_World;
        }
        camera_mesh->pre_nudge_World = tmp_2D.pre_nudge_World;
    }
}

void callback_framebuffer_size(GLFWwindow *, int width, int height) {
    glViewport(0, 0, width, height);
}

KeyEventSubtype classify_baked_subtype_of_raw_key_event(RawKeyEvent *raw_key_event) {
    if (!popup->_active_popup_unique_ID__FORNOW_name0) return KeyEventSubtype::Hotkey;

    uint key = raw_key_event->key;
    bool control = raw_key_event->control;
    // bool shift = raw_key_event->shift;

    // TODO: these need to take control into account
    bool key_is_digit = ('0' <= key) && (key <= '9');
    bool key_is_punc  = (key == '.') || (key == '-');
    bool key_is_alpha = ('A' <= key) && (key <= 'Z');
    bool key_is_delete = (key == GLFW_KEY_BACKSPACE) || (key == GLFW_KEY_DELETE);
    bool key_is_enter = (key == GLFW_KEY_ENTER);
    bool key_is_nav = (key == GLFW_KEY_TAB) || (key == GLFW_KEY_LEFT) || (key == GLFW_KEY_RIGHT);
    bool key_is_ctrl_a = (key == 'A') && (control);

    bool is_consumable_by_popup; {
        is_consumable_by_popup = false;
        if (!_SELECT_OR_DESELECT_COLOR()) is_consumable_by_popup |= key_is_digit;
        is_consumable_by_popup |= key_is_punc;
        is_consumable_by_popup |= key_is_delete;
        is_consumable_by_popup |= key_is_enter;
        is_consumable_by_popup |= key_is_nav;
        is_consumable_by_popup |= key_is_ctrl_a;
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

// NOTE: this function does state-dependent stuff (magic-snapping)
// NOTE: a lot of stuff is happening at once here:
//       pixel coords -> pre-snapped world coords -> snapped world-coords
// NOTE: this function can "drop" raw_event's by returning the null event.
//       this smells a bit (should probs fail earlier, but I like the previous layer not knowing this stuff)
Event bake_event(RawEvent raw_event) {
    Event event = {};
    if (raw_event.type == EventType::Key) {
        RawKeyEvent *raw_key_event = &raw_event.raw_key_event;

        event.type = EventType::Key;
        KeyEvent *key_event = &event.key_event;
        key_event->key = raw_key_event->key;
        key_event->control = raw_key_event->control;
        key_event->shift = raw_key_event->shift;
        key_event->subtype = classify_baked_subtype_of_raw_key_event(raw_key_event); // NOTE: must come last
    } else { ASSERT(raw_event.type == EventType::Mouse);
        RawMouseEvent *raw_mouse_event = &raw_event.raw_mouse_event;

        event.type = EventType::Mouse;
        MouseEvent *mouse_event = &event.mouse_event;
        mouse_event->mouse_held = raw_mouse_event->mouse_held;
        {
            if (raw_mouse_event->pane == Pane::Drawing) {
                mat4 World_2D_from_NDC = inverse(camera_get_PV(camera_drawing));
                vec2 mouse_World_2D = transformPoint(World_2D_from_NDC, other.mouse_NDC);

                mouse_event->subtype = MouseEventSubtype::Drawing;

                MouseEventDrawing *mouse_event_drawing = &mouse_event->mouse_event_drawing;
                mouse_event_drawing->mouse_position = magic_snap(mouse_World_2D);
            } else if (raw_mouse_event->pane == Pane::Mesh) {
                mat4 World_3D_from_NDC = inverse(camera_get_PV(&other.camera_mesh));
                vec3 point_a = transformPoint(World_3D_from_NDC, V3(other.mouse_NDC, -1.0f));
                vec3 point_b = transformPoint(World_3D_from_NDC, V3(other.mouse_NDC,  1.0f));

                mouse_event->subtype = MouseEventSubtype::Mesh;

                MouseEventMesh *mouse_event_mesh = &mouse_event->mouse_event_mesh;
                mouse_event_mesh->mouse_ray_origin = point_a;
                mouse_event_mesh->mouse_ray_direction = normalized(point_b - point_a);
            } else if (raw_mouse_event->pane == Pane::Popup) {
                mouse_event->subtype = MouseEventSubtype::Popup;

                MouseEventPopup *mouse_event_popup = &mouse_event->mouse_event_popup;
                bool mouse_event_is_press = (!mouse_event->mouse_held);
                if (mouse_event_is_press) {
                    mouse_event_popup->cell_index = popup->info_hover_cell_index; 
                    mouse_event_popup->cursor = popup->info_hover_cell_cursor;
                } else {
                    mouse_event_popup->cell_index = popup->active_cell_index; // hmm...
                    mouse_event_popup->cursor = popup->info_active_cell_cursor;
                }
            } else { ASSERT(raw_mouse_event->pane == Pane::Separator);
                event = {};
            }
        }
    }
    return event;
}


// TODO: this API should match what is printed to the terminal in verbose output mode
//       (so we can copy and paste a session for later use as an end to end test)

Event make_mouse_event_2D(vec2 mouse_position) {
    Event event = {};
    event.type = EventType::Mouse;
    MouseEvent *mouse_event = &event.mouse_event;
    mouse_event->subtype = MouseEventSubtype::Drawing;
    MouseEventDrawing *mouse_event_drawing = &mouse_event->mouse_event_drawing;
    mouse_event_drawing->mouse_position = mouse_position;
    return event;
}
Event make_mouse_event_2D(real mouse_position_x, real mouse_position_y) { return make_mouse_event_2D({ mouse_position_x, mouse_position_y }); }

Event make_mouse_event_3D(vec3 mouse_ray_origin, vec3 mouse_ray_direction) {
    Event event = {};
    event.type = EventType::Mouse;
    MouseEvent *mouse_event = &event.mouse_event;
    mouse_event->subtype = MouseEventSubtype::Mesh;
    MouseEventMesh *mouse_event_mesh = &mouse_event->mouse_event_mesh;
    mouse_event_mesh->mouse_ray_origin = mouse_ray_origin;
    mouse_event_mesh->mouse_ray_direction = mouse_ray_direction;
    return event;
}

/////////////////////////////////////////////////////////
// PROCESSING A SINGLE STANDARD EVENT ///////////////////
/////////////////////////////////////////////////////////
// NOTE: this function is (a teeny bit) recursive D;
// NOTE: this sometimes modifies state.drawing
// NOTE: this sometimes modifies state.mesh
// NOTE (May 5, 2024): 
// - returns flags for how the fresh event processor should deal with this event
// - is the event is marked as a snapshotted event (by makeion, only possible for non-fresh events), then skips the expensive stuff

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
    bool value_to_write_to_selection_mask = (state.click_mode == ClickMode::Select);

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
                for_(color, 10) {
                    if (key_lambda('0' + color)) {
                        digit_lambda = true;
                        digit = color;
                        break;
                    }
                }
            }

            ClickMode prev_click_mode = state.click_mode;
            EnterMode prev_enter_mode = state.enter_mode;
            { // key_lambda
                if (digit_lambda) {
                    if (click_mode_SELECT_OR_DESELECT() && (state.click_modifier == ClickModifier::Color)) { // [sd]q0
                        _for_each_entity_ {
                            uint i = uint(entity->color_code);
                            if (i != digit) continue;
                            ENTITY_SET_IS_SELECTED(entity, value_to_write_to_selection_mask);
                        }
                        state.click_mode = ClickMode::None;
                        state.click_modifier = ClickModifier::None;
                    } else if ((state.click_mode == ClickMode::Color) && (state.click_modifier == ClickModifier::Selected)) { // qs0
                        _for_each_selected_entity_ ENTITY_SET_COLOR(entity, ColorCode(digit));
                        state.click_mode = ClickMode::None;
                        state.click_modifier = ClickModifier::None;
                        _for_each_entity_ entity->is_selected = false;
                    } else { // 0
                        result.record_me = true;
                        state.click_mode = ClickMode::Color;
                        state.click_modifier = ClickModifier::None;
                        state.click_color_code = ColorCode(digit);
                    }
                } else if (key_lambda('A')) {
                    if (click_mode_SELECT_OR_DESELECT()) {
                        result.checkpoint_me = true;
                        CLEAR_SELECTION_MASK_TO(state.click_mode == ClickMode::Select);
                        state.click_mode = ClickMode::None;
                        state.click_modifier = ClickModifier::None;
                    }
                } else if (key_lambda('A', false, true)) {
                    state.click_mode = ClickMode::Axis;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;
                } else if (key_lambda('B')) {
                    state.click_mode = ClickMode::Box;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;
                } else if (key_lambda('C')) {
                    if (((state.click_mode == ClickMode::Select) || (state.click_mode == ClickMode::Deselect)) && (state.click_modifier != ClickModifier::Connected)) {
                        state.click_modifier = ClickModifier::Connected;
                    } else if (click_mode_SNAP_ELIGIBLE()) {
                        result.record_me = false;
                        state.click_modifier = ClickModifier::Center;
                    } else {
                        state.click_mode = ClickMode::Circle;
                        state.click_modifier = ClickModifier::None;
                        two_click_command->awaiting_second_click = false;
                    }
                } else if (key_lambda('D')) {
                    state.click_mode = ClickMode::Deselect;
                    state.click_modifier = ClickModifier::None;
                } else if (key_lambda('E')) {
                    if (click_mode_SNAP_ELIGIBLE()) {
                        result.record_me = false;
                        state.click_modifier = ClickModifier::End;
                    }
                } else if (key_lambda('F')) {
                    state.click_mode = ClickMode::Fillet;
                    state.click_modifier = ClickModifier::None;
                    state.enter_mode = EnterMode::None;
                    two_click_command->awaiting_second_click = false;
                } else if (key_lambda('G')) {
                    result.record_me = false;
                    other.hide_grid = !other.hide_grid;
                } else if (key_lambda('H')) {
                    result.record_me = false;
                    history_printf_script();
                } else if (key_lambda('K')) { 
                    result.record_me = false;
                    other.show_event_stack = !other.show_event_stack;
                } else if (key_lambda('K', false, true)) {
                    result.record_me = false;
                    other.hide_gui = !other.hide_gui;
                } else if (key_lambda('L')) {
                    state.click_mode = ClickMode::Line;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;
                } else if (key_lambda('M')) {
                    if (click_mode_SNAP_ELIGIBLE()) {
                        result.record_me = false;
                        state.click_modifier = ClickModifier::Middle;
                    } else {
                        state.click_mode = ClickMode::Move;
                        state.click_modifier = ClickModifier::None;
                        two_click_command->awaiting_second_click = false;
                    }
                } else if (key_lambda('M', false, true)) {
                    result.record_me = false;
                    state.click_mode = ClickMode::Measure;
                    state.click_modifier = ClickModifier::None;
                    two_click_command->awaiting_second_click = false;
                } else if (key_lambda('N')) {
                    if (feature_plane->is_active) {
                        state.enter_mode = EnterMode::NudgeFeaturePlane;
                        preview->feature_plane_offset = 0.0f; // FORNOW
                    } else {
                        messagef(omax.orange, "NudgeFeaturePlane: no feature plane is selected");
                    }
                } else if (key_lambda('N', true, false)) {
                    result.checkpoint_me = true;
                    result.snapshot_me = true;
                    list_free_AND_zero(&drawing->entities);
                    *drawing = {};
                    messagef(omax.green, "ResetDXF");
                } else if (key_lambda('N', true, true)) {
                    result.checkpoint_me = true;
                    result.snapshot_me = true;
                    mesh_free_AND_zero(mesh);
                    *feature_plane = {};
                    messagef(omax.green, "ResetSTL");
                } else if (key_lambda('O', true)) {
                    state.enter_mode = EnterMode::Open;
                } else if (key_lambda('Q')) {
                    if (click_mode_SELECT_OR_DESELECT() && (state.click_modifier == ClickModifier::None)) {
                        state.click_modifier = ClickModifier::Color;
                    } else {
                        state.click_mode = ClickMode::Color;
                        state.click_modifier = ClickModifier::None;
                    }
                } else if (key_lambda('S')) {
                    if (state.click_mode != ClickMode::Color) {
                        state.click_mode = ClickMode::Select;
                        state.click_modifier = ClickModifier::None;
                    } else {
                        state.click_modifier = ClickModifier::Selected;
                    }
                } else if (key_lambda('S', true)) {
                    result.record_me = false;
                    state.enter_mode = EnterMode::Save;
                } else if (key_lambda('W')) {
                    if ((state.click_mode == ClickMode::Select) || (state.click_mode == ClickMode::Deselect)) {
                        state.click_modifier = ClickModifier::Window;
                        two_click_command->awaiting_second_click = false;
                    }
                } else if (key_lambda('X')) {
                    if (state.click_mode != ClickMode::None) {
                        state.click_modifier = ClickModifier::XYCoordinates;
                    }
                } else if (key_lambda('X', false, true)) {
                    state.click_mode = ClickMode::MirrorX;
                    state.click_modifier = ClickModifier::None;
                } else if (key_lambda('X', true, true)) {
                    result.record_me = false;
                    camera2D_zoom_to_bounding_box(&other.camera_drawing, dxf_entities_get_bounding_box(&drawing->entities));
                } else if (key_lambda('Y')) {
                    // TODO: 'Y' remembers last terminal choice of plane for next time
                    result.checkpoint_me = true;
                    other.time_since_plane_selected = 0.0f;

                    // already one of the three primary planes
                    if ((feature_plane->is_active) && ARE_EQUAL(feature_plane->signed_distance_to_world_origin, 0.0f) && ARE_EQUAL(squaredNorm(feature_plane->normal), 1.0f) && ARE_EQUAL(maxComponent(feature_plane->normal), 1.0f)) {
                        feature_plane->normal = { feature_plane->normal[2], feature_plane->normal[0], feature_plane->normal[1] };
                    } else {
                        feature_plane->is_active = true;
                        feature_plane->signed_distance_to_world_origin = 0.0f;
                        feature_plane->normal = { 0.0f, 1.0f, 0.0f };
                    }
                } else if (key_lambda('Y', false, true)) {
                    state.click_mode = ClickMode::MirrorY;
                    state.click_modifier = ClickModifier::None;
                } else if (key_lambda('Z')) {
                    Event equivalent = {};
                    equivalent.type = EventType::Mouse;
                    equivalent.mouse_event.subtype = MouseEventSubtype::Drawing;
                    // .mouse_position = {};
                    return _standard_event_process_NOTE_RECURSIVE(equivalent);
                } else if (key_lambda('Z', false, true)) {
                    state.click_mode = ClickMode::Origin;
                    state.click_modifier = ClickModifier::None;
                } else if (key_lambda(' ')) {
                    state.click_mode = ClickMode::None; // FORNOW: patching space space doing CIRCLE CENTER
                    return _standard_event_process_NOTE_RECURSIVE(state.space_bar_event);
                } else if (key_lambda(' ', false, true)) {
                    return _standard_event_process_NOTE_RECURSIVE(state.shift_space_bar_event);
                } else if (key_lambda('[')) {
                    state.enter_mode = EnterMode::ExtrudeAdd;
                    preview->extrude_in_length = 0; // FORNOW
                    preview->extrude_out_length = 0; // FORNOW
                } else if (key_lambda('[', false, true)) {
                    state.enter_mode = EnterMode::ExtrudeCut;
                    preview->extrude_in_length = 0; // FORNOW
                    preview->extrude_out_length = 0; // FORNOW
                } else if (key_lambda(']')) {
                    state.enter_mode = EnterMode::RevolveAdd;
                } else if (key_lambda(']', false, true)) {
                    state.enter_mode = EnterMode::RevolveCut;
                } else if (key_lambda('.')) { 
                    result.record_me = false;
                    other.show_details = !other.show_details;
                    { // messagef
                        uint num_lines;
                        uint num_arcs;
                        {
                            num_lines = 0;
                            num_arcs = 0;
                            _for_each_entity_ {
                                if (entity->type == EntityType::Line) {
                                    ++num_lines;
                                } else { ASSERT(entity->type == EntityType::Arc);
                                    ++num_arcs;
                                }
                            }
                        }
                        messagef(omax.cyan,"Mesh has %d triangles", mesh->num_triangles);
                        messagef(omax.cyan,"Drawing has %d elements = %d lines + %d arcs", drawing->entities.length, num_lines, num_arcs);
                    }
                } else if (key_lambda(';')) {
                    result.checkpoint_me = true;
                    feature_plane->is_active = false;
                } else if (key_lambda('\'')) {
                    result.record_me = false;
                    other.camera_mesh.angle_of_view = CAMERA_3D_PERSPECTIVE_ANGLE_OF_VIEW - other.camera_mesh.angle_of_view;
                } else if (key_lambda(GLFW_KEY_BACKSPACE) || key_lambda(GLFW_KEY_DELETE)) {
                    for (int i = drawing->entities.length - 1; i >= 0; --i) {
                        if (drawing->entities.array[i].is_selected) {
                            _REMOVE_ENTITY(i);
                        }
                    }
                } else if (key_lambda('/', false, true)) {
                    result.record_me = false;
                    other.show_help = !other.show_help;
                } else if (key_lambda(GLFW_KEY_ESCAPE)) {
                    state.enter_mode = EnterMode::None;
                    state.click_mode = ClickMode::None;
                    state.click_modifier = ClickModifier::None;
                    state.click_color_code = ColorCode::Traverse;
                } else if (key_lambda(GLFW_KEY_TAB)) { // FORNOW
                    result.record_me = false;
                    {
                        vec3 tmp = omax.light_gray;
                        omax.light_gray = omax.dark_gray;
                        omax.dark_gray = tmp;
                    }
                    {
                        vec3 tmp = omax.white;
                        omax.white = omax.black;
                        omax.black = tmp;
                    }
                } else if (key_lambda(GLFW_KEY_ENTER)) { // FORNOW
                                                         // messagef(omax.orange, "EnterMode is None.");
                    result.record_me = false;
                } else {
                    messagef(omax.orange, "Hotkey %s was not recognized.", key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(key_event), key_event->control, key_event->shift, key_event->key);
                    result.record_me = false;
                    ;
                }
            }
            bool changed_click_mode = (prev_click_mode != state.click_mode);
            bool changed_enter_mode = (prev_enter_mode != state.enter_mode);
            if (changed_click_mode && click_mode_SPACE_BAR_REPEAT_ELIGIBLE()) state.space_bar_event = event;
            if (changed_enter_mode && enter_mode_SHIFT_SPACE_BAR_REPEAT_ELIGIBLE()) state.shift_space_bar_event = event;
        } else { ASSERT(key_event->subtype == KeyEventSubtype::Popup);
            result.record_me = true;

            other.time_since_cursor_start = 0.0; // FORNOW

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

            uint len = strlen(popup->active_cell_buffer);
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
                        memmove(&popup->active_cell_buffer[popup->cursor - 1], &popup->active_cell_buffer[popup->cursor], POPUP_CELL_LENGTH - popup->cursor);
                        popup->active_cell_buffer[POPUP_CELL_LENGTH - 1] = '\0';
                        --popup->cursor;
                    }
                } else {
                    // * * * * * * * * * * * * * * * *
                    // * * * * * * * - - - - - - - - -
                    //    L                 R 

                    // * * * * * * * * * * * * * * * *
                    // * * * * * * * * * * * * - - - -
                    //    L       R                   
                    memmove(&popup->active_cell_buffer[left_cursor], &popup->active_cell_buffer[right_cursor], POPUP_CELL_LENGTH - right_cursor);
                    memset(&popup->active_cell_buffer[POPUP_CELL_LENGTH - (right_cursor - left_cursor)], 0, right_cursor - left_cursor);
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
                        memmove(&popup->active_cell_buffer[popup->cursor + 1], &popup->active_cell_buffer[popup->cursor], POPUP_CELL_LENGTH - popup->cursor - 1);
                        popup->active_cell_buffer[popup->cursor++] = char_equivalent;
                    }
                } else {
                    memmove(&popup->active_cell_buffer[left_cursor + 1], &popup->active_cell_buffer[right_cursor], POPUP_CELL_LENGTH - right_cursor);
                    memset(&popup->active_cell_buffer[POPUP_CELL_LENGTH - (right_cursor - (left_cursor + 1))], 0, right_cursor - (left_cursor + 1));
                    popup->cursor = left_cursor;
                    popup->active_cell_buffer[popup->cursor++] = char_equivalent;
                }
                popup->selection_cursor = popup->cursor;
            }
        }
    } else if (event.type == EventType::Mouse) {
        MouseEvent *mouse_event = &event.mouse_event;
        if (mouse_event->subtype == MouseEventSubtype::Drawing) {
            MouseEventDrawing *mouse_event_drawing = &mouse_event->mouse_event_drawing;

            result.record_me = true;
            if (state.click_mode == ClickMode::Measure) result.record_me = false;
            if (mouse_event->mouse_held) result.record_me = false;

            vec2 *mouse = &mouse_event_drawing->mouse_position;
            vec2 *second_click = &mouse_event_drawing->mouse_position;

            bool click_mode_WINDOW_SELECT_OR_WINDOW_DESELECT = (click_mode_SELECT_OR_DESELECT() && (state.click_modifier == ClickModifier::Window));

            bool click_mode_TWO_CLICK_COMMAND = 0 ||
                (state.click_mode == ClickMode::Axis) ||
                (state.click_mode == ClickMode::Measure) ||
                (state.click_mode == ClickMode::Line) ||
                (state.click_mode == ClickMode::Box) ||
                (state.click_mode == ClickMode::Circle) ||
                (state.click_mode == ClickMode::Fillet) ||
                (state.click_mode == ClickMode::Move) ||
                click_mode_WINDOW_SELECT_OR_WINDOW_DESELECT; // fornow wonky case

            // fornow window wonky case
            if (_non_WINDOW__SELECT_DESELECT___OR___SET_COLOR()) { // NOTES: includes scand qc
                result.record_me = false;
                DXFFindClosestEntityResult dxf_find_closest_entity_result = dxf_find_closest_entity(&drawing->entities, mouse_event_drawing->mouse_position.x, mouse_event_drawing->mouse_position.y);
                if (dxf_find_closest_entity_result.success) {
                    uint hot_entity_index = dxf_find_closest_entity_result.index;
                    if (state.click_modifier != ClickModifier::Connected) {
                        if (click_mode_SELECT_OR_DESELECT()) {
                            ENTITY_SET_IS_SELECTED(&drawing->entities.array[hot_entity_index], value_to_write_to_selection_mask);
                        } else {
                            ENTITY_SET_COLOR(&drawing->entities.array[hot_entity_index], state.click_color_code);
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
                            int entity_index;
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
                                for_(i, ARRAY_LENGTH(cell->slots)) {
                                    GridPointSlot *slot = &cell->slots[i];
                                    if (slot->populated) continue;
                                    slot->populated = true;
                                    slot->entity_index = entity_index;
                                    slot->end_NOT_start = end_NOT_start;
                                    // printf("%f %f [%d]\n", key.x, key.y, i);
                                    break;
                                }
                            };

                            for_(entity_index, drawing->entities.length) {
                                Entity *entity = &drawing->entities.array[entity_index];

                                real start_x, start_y, end_x, end_y;
                                entity_get_start_and_end_points(entity, &start_x, &start_y, &end_x, &end_y);
                                push_into_grid_unless_cell_full__make_cell_if_none_exists(start_x, start_y, entity_index, false);
                                push_into_grid_unless_cell_full__make_cell_if_none_exists(end_x, end_y, entity_index, true);
                            }
                        }

                        bool *edge_marked = (bool *) calloc(drawing->entities.length, sizeof(bool));

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
                                Entity *entity = &drawing->entities.array[point->entity_index];
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

                            for_(i, ARRAY_LENGTH(cell->slots)) {
                                GridPointSlot *slot = &cell->slots[i];
                                if (!slot->populated) continue;
                                if (edge_marked[slot->entity_index]) continue;
                                return slot;
                            }
                            return NULL;
                        };



                        // NOTE: we will mark the hot entity, and then shoot off from both its endpoints
                        edge_marked[hot_entity_index] = true;
                        ENTITY_SET_IS_SELECTED(&drawing->entities.array[hot_entity_index], value_to_write_to_selection_mask);

                        for_(pass, 2) {

                            vec2 seed; {
                                real x, y;
                                if (pass == 0) {
                                    entity_get_start_point(&drawing->entities.array[hot_entity_index], &x, &y);
                                } else {
                                    entity_get_end_point(&drawing->entities.array[hot_entity_index], &x, &y);
                                }
                                seed = make_key(x, y);
                            }

                            GridPointSlot *curr = get_any_point_not_part_of_an_marked_entity(seed);
                            while (true) {
                                if (curr == NULL) break;
                                ENTITY_SET_IS_SELECTED(&drawing->entities.array[curr->entity_index], value_to_write_to_selection_mask);
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
                            ENTITY_SET_IS_SELECTED(&drawing->entities[entity_index_and_flip_flag->entity_index], value_to_write_to_selection_mask);
                        }
                        #endif
                    }
                }
            } else if (!mouse_event->mouse_held) {
                if (click_mode_TWO_CLICK_COMMAND) {
                    vec2 *first_click = &two_click_command->first_click;

                    if (!two_click_command->awaiting_second_click) {
                        two_click_command->awaiting_second_click = true;
                        *first_click = mouse_event_drawing->mouse_position;
                        if (state.click_modifier != ClickModifier::Window) state.click_modifier = ClickModifier::None;
                    } else {
                        if (0) {
                        } else if (state.click_mode == ClickMode::Axis) {
                            two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            drawing->axis_base_point = *first_click;
                            drawing->axis_angle_from_y = (-PI / 2) + atan2(*second_click - *first_click);
                        } else if (state.click_mode == ClickMode::Box) {
                            if (IS_ZERO(ABS(first_click->x - second_click->x))) {
                                messagef(omax.orange, "[box] must have non-zero width ");
                            } else if (IS_ZERO(ABS(first_click->y - second_click->y))) {
                                messagef(omax.orange, "[box] must have non-zero height");
                            } else {
                                two_click_command->awaiting_second_click = false;
                                result.checkpoint_me = true;
                                state.click_mode = ClickMode::None;
                                state.click_modifier = ClickModifier::None;
                                vec2 other_corner_A = { first_click->x, second_click->y };
                                vec2 other_corner_B = { second_click->x, first_click->y };
                                ADD_LINE_ENTITY(*first_click,  other_corner_A);
                                ADD_LINE_ENTITY(*first_click,  other_corner_B);
                                ADD_LINE_ENTITY(*second_click, other_corner_A);
                                ADD_LINE_ENTITY(*second_click, other_corner_B);
                            }
                        } else if (state.click_mode == ClickMode::Fillet) {
                            two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_modifier = ClickModifier::None;
                            DXFFindClosestEntityResult result_i = dxf_find_closest_entity(&drawing->entities, first_click->x, first_click->y);
                            DXFFindClosestEntityResult result_j = dxf_find_closest_entity(&drawing->entities, mouse_event_drawing->mouse_position.x, mouse_event_drawing->mouse_position.y);
                            if ((result_i.success) && (result_j.success) && (result_i.index != result_j.index)) {
                                uint i = result_i.index;
                                uint j = result_j.index;
                                real radius = popup->fillet_radius;
                                Entity *E_i = &drawing->entities.array[i];
                                Entity *E_j = &drawing->entities.array[j];
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
                                        real length = radius / TAN(half_angle);
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
                                    messagef(omax.red, "TODO: line_entity-arc_entity fillet; arc_entity-arc_entity fillet");
                                }
                            }
                        } else if (state.click_mode == ClickMode::Circle) {
                            if (IS_ZERO(norm(*first_click - *second_click))) {
                                messagef(omax.orange, "[circle] must have non-zero diameter");
                            } else {
                                two_click_command->awaiting_second_click = false;
                                result.checkpoint_me = true;
                                state.click_mode = ClickMode::None;
                                state.click_modifier = ClickModifier::None;
                                real theta_a_in_degrees = DEG(atan2(*second_click - *first_click));
                                real theta_b_in_degrees = theta_a_in_degrees + 180.0f;
                                real r = norm(*second_click - *first_click);
                                ADD_ARC_ENTITY(*first_click, r, theta_a_in_degrees, theta_b_in_degrees);
                                ADD_ARC_ENTITY(*first_click, r, theta_b_in_degrees, theta_a_in_degrees);
                            }
                        } else if (state.click_mode == ClickMode::Line) {
                            two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            ADD_LINE_ENTITY(*first_click, *second_click);
                        } else if (state.click_mode == ClickMode::Measure) {
                            two_click_command->awaiting_second_click = false;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
                            real angle = DEG(atan2(*second_click - *first_click));
                            if (angle < 0.0f) angle += 360.0f;
                            real length = norm(*second_click - *first_click);
                            messagef(omax.cyan, "Angle is %gdeg.", angle);
                            messagef(omax.cyan, "Length is %gmm.", length);
                        } else if (state.click_mode == ClickMode::Move) {
                            two_click_command->awaiting_second_click = false;
                            result.checkpoint_me = true;
                            state.click_mode = ClickMode::None;
                            state.click_modifier = ClickModifier::None;
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
                            two_click_command->awaiting_second_click = false;
                            box2 window = {
                                MIN(first_click->x, second_click->x),
                                MIN(first_click->y, second_click->y),
                                MAX(first_click->x, second_click->x),
                                MAX(first_click->y, second_click->y)
                            };
                            _for_each_entity_ {
                                if (box_contains(window, dxf_entity_get_bounding_box(entity))) {
                                    ENTITY_SET_IS_SELECTED(entity, value_to_write_to_selection_mask);
                                }
                            }
                        }
                    }
                } else {
                    if (state.click_mode == ClickMode::Origin) {
                        result.checkpoint_me = true;
                        state.click_mode = ClickMode::None;
                        state.click_modifier = ClickModifier::None;
                        drawing->origin = *mouse;
                    } else if (state.click_mode == ClickMode::MirrorX) {
                        result.checkpoint_me = true;
                        state.click_mode = ClickMode::None;
                        state.click_modifier = ClickModifier::None;
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
                    } else if (state.click_mode == ClickMode::MirrorY) {
                        result.checkpoint_me = true;
                        state.click_mode = ClickMode::None;
                        state.click_modifier = ClickModifier::None;
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
                    } else {
                        result.record_me = false;
                    }
                }
            }
        } else if (mouse_event->subtype == MouseEventSubtype::Mesh) {
            MouseEventMesh *mouse_event_mesh = &mouse_event->mouse_event_mesh;
            result.record_me = false;
            if (!mouse_event->mouse_held) {
                int index_of_first_triangle_hit_by_ray = -1;
                {
                    real min_distance = HUGE_VAL;
                    for_(i, mesh->num_triangles) {
                        vec3 p[3]; {
                            for_(j, 3) p[j] = mesh->vertex_positions[mesh->triangle_indices[i][j]];
                        }
                        RayTriangleIntersectionResult ray_triangle_intersection_result = ray_triangle_intersection(mouse_event_mesh->mouse_ray_origin, mouse_event_mesh->mouse_ray_direction, p[0], p[1], p[2]);
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
                    other.time_since_plane_selected = 0.0f;
                    {
                        feature_plane->normal = mesh->triangle_normals[index_of_first_triangle_hit_by_ray];
                        { // feature_plane->signed_distance_to_world_origin
                            vec3 a_selected = mesh->vertex_positions[mesh->triangle_indices[index_of_first_triangle_hit_by_ray][0]];
                            feature_plane->signed_distance_to_world_origin = dot(feature_plane->normal, a_selected);
                        }

                    }
                }
            }
        } else { ASSERT(mouse_event->subtype == MouseEventSubtype::Popup);
            MouseEventPopup *mouse_event_popup = &mouse_event->mouse_event_popup;

            other.time_since_cursor_start = 0.0f;
            result.record_me = true; // FORNOW (don't bother implementing the old manual tagging method; we're going to upgrade to the memcmp approach soon)

            if (!mouse_event->mouse_held) { // press
                if (popup->active_cell_index != mouse_event_popup->cell_index) { // switch cell
                    POPUP_SET_ACTIVE_CELL_INDEX(mouse_event_popup->cell_index);
                    popup->cursor = mouse_event_popup->cursor;
                    popup->selection_cursor = popup->cursor;
                } else {
                    bool double_click = (POPUP_SELECTION_NOT_ACTIVE()) && (popup->cursor == mouse_event_popup->cursor);
                    if (double_click) {
                        uint len = strlen(popup->active_cell_buffer); // FORNOW
                        popup->cursor = len;
                        popup->selection_cursor = 0;
                    } else { // move
                        popup->cursor = mouse_event_popup->cursor;
                        popup->selection_cursor = popup->cursor;
                    }
                }
            } else { // drag
                popup->selection_cursor = mouse_event_popup->cursor;
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

        EnterMode _enter_mode_prev__NOTE_used_to_determine_when_to_close_popup_on_enter = state.enter_mode;
        popup->_popup_actually_called_this_event = false;

        { // popup_popup
            vec2 *first_click = &two_click_command->first_click;

            bool gui_key_enter; {
                gui_key_enter = false;
                if (event.type == EventType::Key) {
                    KeyEvent *key_event = &event.key_event;
                    if (key_event->subtype == KeyEventSubtype::Popup) {
                        gui_key_enter = (key_event->key == GLFW_KEY_ENTER);
                    }
                }
            }

            if (state.enter_mode == EnterMode::Open) {
                sprintf(full_filename_scratch_buffer, "%s%s", other.drop_path, popup->open_filename);
                popup_popup(false,
                        CellType::String, "open_filename", popup->open_filename);
                if (gui_key_enter) {
                    if (poe_suffix_match(full_filename_scratch_buffer, ".dxf")) {
                        result.record_me = true;
                        result.checkpoint_me = true;
                        result.snapshot_me = true;
                        conversation_dxf_load(full_filename_scratch_buffer,
                                skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack);
                        state.enter_mode = EnterMode::None;
                    } else if (poe_suffix_match(full_filename_scratch_buffer, ".stl")) {
                        result.record_me = true;
                        result.checkpoint_me = true;
                        result.snapshot_me = true;
                        conversation_stl_load(full_filename_scratch_buffer);
                        state.enter_mode = EnterMode::None;
                    } else {
                        messagef(omax.orange, "[open] \"%s\" not found", full_filename_scratch_buffer);
                    }
                }
            } else if (state.enter_mode == EnterMode::Save) {
                result.record_me = false;
                sprintf(full_filename_scratch_buffer, "%s%s", other.drop_path, popup->save_filename);
                popup_popup(false,
                        CellType::String, "save_filename", popup->save_filename);
                if (gui_key_enter) {
                    conversation_save(full_filename_scratch_buffer);
                    state.enter_mode = EnterMode::None;
                }
            } else if (state.enter_mode == EnterMode::ExtrudeAdd) {
                popup_popup(true,
                        CellType::Real32, "extrude_add_out_length", &popup->extrude_add_out_length,
                        CellType::Real32, "extrude_add_in_length",  &popup->extrude_add_in_length);
                if (gui_key_enter) {
                    if (!dxf_anything_selected) {
                        messagef(omax.orange, "Drawing selection is empty.");
                    } else if (!feature_plane->is_active) {
                        messagef(omax.orange, "No feature plane is selected.");
                    } else if (IS_ZERO(popup->extrude_add_in_length) && IS_ZERO(popup->extrude_add_out_length)) {
                        messagef(omax.orange, "Total extrusion length is zero.");
                    } else {
                        GENERAL_PURPOSE_MANIFOLD_WRAPPER();
                        if (IS_ZERO(popup->extrude_add_in_length)) {
                            messagef(omax.green, "ExtrudeAdd %gmm", popup->extrude_add_out_length);
                        } else {
                            messagef(omax.green, "ExtrudeAdd %gmm %gmm", popup->extrude_add_out_length, popup->extrude_add_in_length);
                        }
                    }
                }
            } else if (state.enter_mode == EnterMode::ExtrudeCut) {
                popup_popup(true,
                        CellType::Real32, "extrude_cut_in_length", &popup->extrude_cut_in_length,
                        CellType::Real32, "extrude_cut_out_length", &popup->extrude_cut_out_length);
                if (gui_key_enter) {
                    if (!dxf_anything_selected) {
                        messagef(omax.orange, "Drawing selection is empty.");
                    } else if (!feature_plane->is_active) {
                        messagef(omax.orange, "No feature plane is selected.");
                    } else if (IS_ZERO(popup->extrude_cut_in_length) && IS_ZERO(popup->extrude_cut_out_length)) {
                        messagef(omax.orange, "Total extrusion length is zero.");
                    } else if (mesh->num_triangles == 0) {
                        messagef(omax.orange, "Current mesh is empty.");
                    } else {
                        GENERAL_PURPOSE_MANIFOLD_WRAPPER();
                        if (IS_ZERO(popup->extrude_cut_out_length)) {
                            messagef(omax.green, "ExtrudeCut %gmm", popup->extrude_cut_in_length);
                        } else {
                            messagef(omax.green, "ExtrudeCut %gmm %gmm", popup->extrude_cut_in_length, popup->extrude_cut_out_length);
                        }
                    }
                }
            } else if (state.enter_mode == EnterMode::RevolveAdd) {
                popup_popup(true, CellType::Real32, "revolve_add_dummy", &popup->revolve_add_dummy);
                if (gui_key_enter) {
                    if (!dxf_anything_selected) {
                        messagef(omax.orange, "Drawing selection is empty.");
                    } else if (!feature_plane->is_active) {
                        messagef(omax.orange, "No feature plane is selected.");
                    } else {
                        GENERAL_PURPOSE_MANIFOLD_WRAPPER();
                        messagef(omax.green, "RevolveAdd");
                    }
                }
            } else if (state.enter_mode == EnterMode::RevolveCut) {
                popup_popup(true, CellType::Real32, "revolve_cut_dummy", &popup->revolve_cut_dummy);
                if (gui_key_enter) {
                    if (!dxf_anything_selected) {
                        messagef(omax.orange, "Drawing selection is empty.");
                    } else if (!feature_plane->is_active) {
                        messagef(omax.orange, "No feature plane is selected.");
                    } else if (mesh->num_triangles == 0) {
                        messagef(omax.orange, "Current mesh is empty.");
                    } else {
                        GENERAL_PURPOSE_MANIFOLD_WRAPPER();
                        messagef(omax.green, "RevolveCut");
                    }
                }
            } else if (state.enter_mode == EnterMode::NudgeFeaturePlane) {
                popup_popup(true,
                        CellType::Real32, "feature_plane_nudge", &popup->feature_plane_nudge);
                if (gui_key_enter) {
                    result.record_me = true;
                    result.checkpoint_me = true;
                    feature_plane->signed_distance_to_world_origin += popup->feature_plane_nudge;
                    state.enter_mode = EnterMode::None;
                    messagef(omax.green, "NudgeFeaturePlane %gmm", popup->feature_plane_nudge);
                }
            } else if (state.click_modifier == ClickModifier::XYCoordinates) {
                // sus calling this a modifier but okay; make sure it's first or else bad bad
                popup_popup(true,
                        CellType::Real32, "x_coordinate", &popup->x_coordinate,
                        CellType::Real32, "y_coordinate", &popup->y_coordinate);
                if (gui_key_enter) {
                    // popup->_active_popup_unique_ID__FORNOW_name0 = NULL; // FORNOW when making box using 'X' 'X', we want the popup to trigger a reload
                    state.click_modifier = ClickModifier::None;
                    return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(popup->x_coordinate, popup->y_coordinate));
                }
            } else if (state.click_mode == ClickMode::Circle) {
                if (two_click_command->awaiting_second_click) {
                    real prev_circle_diameter = popup->circle_diameter;
                    real prev_circle_radius = popup->circle_radius;
                    real prev_circle_circumference = popup->circle_circumference;
                    popup_popup(false,
                            CellType::Real32, "circle_diameter", &popup->circle_diameter,
                            CellType::Real32, "circle_radius", &popup->circle_radius,
                            CellType::Real32, "circle_circumference", &popup->circle_circumference);
                    if (gui_key_enter) {
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->circle_radius, first_click->y));
                    } else {
                        if (prev_circle_diameter != popup->circle_diameter) {
                            popup->circle_radius = popup->circle_diameter / 2;
                            popup->circle_circumference = PI * popup->circle_diameter;
                        } else if (prev_circle_radius != popup->circle_radius) {
                            popup->circle_diameter = 2 * popup->circle_radius;
                            popup->circle_circumference = PI * popup->circle_diameter;
                        } else if (prev_circle_circumference != popup->circle_circumference) {
                            popup->circle_diameter = popup->circle_circumference / PI;
                            popup->circle_radius = popup->circle_diameter / 2;
                        }
                    }
                }
            } else if (state.click_mode == ClickMode::Line) {
                if (two_click_command->awaiting_second_click) {
                    real prev_line_length = popup->line_length;
                    real prev_line_angle  = popup->line_angle;
                    real prev_line_run    = popup->line_run;
                    real prev_line_rise   = popup->line_rise;
                    popup_popup(true,
                            CellType::Real32, "line_length", &popup->line_length,
                            CellType::Real32, "line_angle",  &popup->line_angle,
                            CellType::Real32, "line_run",    &popup->line_run,
                            CellType::Real32, "line_rise",   &popup->line_rise
                            );
                    if (gui_key_enter) {
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->line_run, first_click->y + popup->line_rise));
                    } else {
                        if ((prev_line_length != popup->line_length) || (prev_line_angle != popup->line_angle)) {
                            popup->line_run  = popup->line_length * COS(RAD(popup->line_angle));
                            popup->line_rise = popup->line_length * SIN(RAD(popup->line_angle));
                        } else if ((prev_line_run != popup->line_run) || (prev_line_rise != popup->line_rise)) {
                            popup->line_length = SQRT(popup->line_run * popup->line_run + popup->line_rise * popup->line_rise);
                            popup->line_angle = DEG(ATAN2(popup->line_rise, popup->line_run));
                        }
                    }
                }
            } else if (state.click_mode == ClickMode::Box) {
                if (two_click_command->awaiting_second_click) {
                    popup_popup(true,
                            CellType::Real32, "box_width", &popup->box_width,
                            CellType::Real32, "box_height", &popup->box_height);
                    if (gui_key_enter) {
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->box_width, first_click->y + popup->box_height));
                    }
                }
            } else if (state.click_mode == ClickMode::Move) {
                // FORNOW: this is repeated from LINE
                if (two_click_command->awaiting_second_click) {
                    real prev_move_length = popup->move_length;
                    real prev_move_angle = popup->move_angle;
                    real prev_move_run = popup->move_run;
                    real prev_move_rise = popup->move_rise;
                    popup_popup(true,
                            CellType::Real32, "move_length", &popup->move_length,
                            CellType::Real32, "move_angle", &popup->move_angle,
                            CellType::Real32, "move_run", &popup->move_run,
                            CellType::Real32, "move_rise", &popup->move_rise
                            );
                    if (gui_key_enter) {
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->move_run, first_click->y + popup->move_rise));
                    } else {
                        if ((prev_move_length != popup->move_length) || (prev_move_angle != popup->move_angle)) {
                            popup->move_run = popup->move_length * COS(RAD(popup->move_angle));
                            popup->move_rise = popup->move_length * SIN(RAD(popup->move_angle));
                        } else if ((prev_move_run != popup->move_run) || (prev_move_rise != popup->move_rise)) {
                            popup->move_length = SQRT(popup->move_run * popup->move_run + popup->move_rise * popup->move_rise);
                            popup->move_angle = DEG(ATAN2(popup->move_rise, popup->move_run));
                        }
                    }
                }
            } else if (state.click_mode == ClickMode::Fillet) {
                popup_popup(false,
                        CellType::Real32, "fillet_radius", &popup->fillet_radius);
            }
        }
        { // popup_close (FORNOW: just doing off of enter transitions)
          // NOTE: we need to do this so that the next key event doesn't get eaten by a dead popup
            bool enter_mode_transitioned_to_ENTER_MODE_NONE = ((_enter_mode_prev__NOTE_used_to_determine_when_to_close_popup_on_enter != EnterMode::None) && (state.enter_mode == EnterMode::None));
            if (0
                    || (!popup->_popup_actually_called_this_event)
                    || enter_mode_transitioned_to_ENTER_MODE_NONE
               ) {
                popup->_active_popup_unique_ID__FORNOW_name0 = NULL;
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
void history_undo() { messagef("[DEBUG] history disabled"); }
void history_redo() { messagef("[DEBUG] history disabled"); }
void history_debug_draw() { gui_printf("[DEBUG] history disabled"); }
//
#else

struct {
    ElephantStack<Event> recorded_user_events;
    ElephantStack<WorldState_ChangesToThisMustBeRecorded_state> snapshotted_world_states;
} history;

struct StackPointers {
    Event *user_event;
    WorldState_ChangesToThisMustBeRecorded_state *world_state;
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
    WorldState_ChangesToThisMustBeRecorded_state *world_state = elephant_is_empty_undo(&history.snapshotted_world_states) ? NULL : elephant_peek_undo(&history.snapshotted_world_states);
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
                WorldState_ChangesToThisMustBeRecorded_state *world_state = history.snapshotted_world_states._redo_stack.array;
                world_state < history.snapshotted_world_states._redo_stack.array + history.snapshotted_world_states._redo_stack.length;
                ++world_state
            ) {//
            world_state_free_AND_zero(world_state);
        }
        elephant_clear_redo(&history.snapshotted_world_states); // TODO ?
    }
    if (standard_event.snapshot_me) {
        WorldState_ChangesToThisMustBeRecorded_state snapshot;
        world_state_deep_copy(&snapshot, &state);
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
        messagef(omax.orange, "Undo: nothing to undo");
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
        world_state_free_AND_zero(&state);
        while (true) {
            if (begin <= elephant_undo_ptr_begin(&history.recorded_user_events)) {
                begin =  elephant_undo_ptr_begin(&history.recorded_user_events); // !
                state = {};
                break;
            }
            if (begin->snapshot_me) {
                world_state_deep_copy(&state, PEEK_UNDO().world_state);
                break;
            }
            --begin;
        }
    }
    other.please_suppress_messagef = true; {
        for (Event *event = begin; event < one_past_end; ++event) _standard_event_process_NOTE_RECURSIVE(*event);
    } other.please_suppress_messagef = false;

    messagef(omax.green, "Undo");
}

void history_redo() {
    if (elephant_is_empty_redo(&history.recorded_user_events)) {
        messagef(omax.orange, "Redo: nothing to redo");
        return;
    }

    other.please_suppress_messagef = true; {
        while (EVENT_REDO_NONEMPTY()) { // // manipulate stacks (undo <- redo)
            StackPointers popped = POP_REDO_ONTO_UNDO();
            Event *user_event = popped.user_event;
            WorldState_ChangesToThisMustBeRecorded_state *world_state = popped.world_state;

            _standard_event_process_NOTE_RECURSIVE(*user_event);
            if (world_state) {
                world_state_free_AND_zero(&state);
                world_state_deep_copy(&state, world_state);
            }

            if (user_event->checkpoint_me) break;
        }
    } other.please_suppress_messagef = false;

    messagef(omax.green, "Redo");
}

void _history_user_event_draw_helper(Event event) {
    char message[256]; {
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
            if (mouse_event->subtype == MouseEventSubtype::Drawing) {
                MouseEventDrawing *mouse_event_drawing = &mouse_event->mouse_event_drawing;
                sprintf(message, "[MOUSE-2D] %g %g", mouse_event_drawing->mouse_position.x, mouse_event_drawing->mouse_position.y);
            } else if (mouse_event->subtype == MouseEventSubtype::Mesh) {
                MouseEventMesh *mouse_event_mesh = &mouse_event->mouse_event_mesh;
                sprintf(message, "[MOUSE-3D] %g %g %g %g %g %g", mouse_event_mesh->mouse_ray_origin.x, mouse_event_mesh->mouse_ray_origin.y, mouse_event_mesh->mouse_ray_origin.z, mouse_event_mesh->mouse_ray_direction.x, mouse_event_mesh->mouse_ray_direction.y, mouse_event_mesh->mouse_ray_direction.z);
            } else { ASSERT(mouse_event->subtype == MouseEventSubtype::Popup);
                MouseEventPopup *mouse_event_popup = &mouse_event->mouse_event_popup;
                sprintf(message, "[GUI-MOUSE] %d %d", mouse_event_popup->cell_index, mouse_event_popup->cursor);
            }
        }
    }
    gui_printf("%c%c %s",
            (event.checkpoint_me)   ? 'C' : ' ',
            (event.snapshot_me)     ? 'S' : ' ',
            message);
}

void _history_world_state_draw_helper(WorldState_ChangesToThisMustBeRecorded_state *world_state) {
    gui_printf("%d drawing elements   %d stl triangles", world_state->drawing.entities.length, world_state->mesh.num_triangles);
}

void history_debug_draw() {
    gui_printf("");
    if (history.snapshotted_world_states._redo_stack.length) {
        for (////
                WorldState_ChangesToThisMustBeRecorded_state *world_state = history.snapshotted_world_states._redo_stack.array;
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
                WorldState_ChangesToThisMustBeRecorded_state *world_state = history.snapshotted_world_states._undo_stack.array + (history.snapshotted_world_states._undo_stack.length - 1);
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
        other.please_suppress_drawing_popup_popup = true;
        history_undo();
    } else if (redo) {
        other.please_suppress_drawing_popup_popup = true;
        history_redo();
    } else {
        history_process_and_potentially_record_checkpoint_and_or_snapshot_standard_fresh_user_event(freshly_baked_event);
    }
}

//////////////////////////////////////////////////
// DRAW() ////////////////////////////////////////
//////////////////////////////////////////////////


//////////////////////////////////////////////////
// TODO: SPOOF ///////////////////////////////////
//////////////////////////////////////////////////
void script_process(char *string) {
    // TODO: gui
    // TODOLATER (weird 'X' version): char *string = "^osplash.drawing\nyscx2020\ne\t50";
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
                        instabaked_event = make_mouse_event_2D(p);
                    } else if (strncmp(tag, "m3d", TAG_LENGTH) == 0) {
                        char *params = &string[i + 1 + TAG_LENGTH];
                        vec3 mouse_ray_origin;
                        vec3 mouse_ray_direction;
                        sscanf(params, "%f %f %f %f %f %f", &mouse_ray_origin.x, &mouse_ray_origin.y, &mouse_ray_origin.z, &mouse_ray_direction.x, &mouse_ray_direction.y, &mouse_ray_direction.z);
                        instabaked_event = make_mouse_event_3D(mouse_ray_origin, mouse_ray_direction);
                    } else if (strncmp(tag, "esc", TAG_LENGTH) == 0) {
                        is_instabaked = false;
                        _raw_event.type = EventType::Key;
                        RawKeyEvent *_raw_key_event = &_raw_event.raw_key_event;
                        _raw_key_event->key = GLFW_KEY_ESCAPE;
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
            RawKeyEvent *raw_key_event = &raw_event.raw_key_event;
            raw_key_event->control = control;
            {
                if ('a' <= c && c <= 'z') {
                    raw_key_event->key = 'A' + (c - 'a');
                } else if ('A' <= c && c <= 'Z') {
                    raw_key_event->shift = true;
                    raw_key_event->key = c;
                } else if (c == '{') {
                    raw_key_event->shift = true;
                    raw_key_event->key = '[';
                } else if (c == '}') {
                    raw_key_event->shift = true;
                    raw_key_event->key = ']';
                } else if (c == '\n') {
                    raw_key_event->key = GLFW_KEY_ENTER;
                } else if (c == '\t') {
                    raw_key_event->key = GLFW_KEY_TAB;
                } else {
                    raw_key_event->key = c;
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
    { // init
        setvbuf(stdout, NULL, _IONBF, 0); // don't buffer printf
        srand((unsigned int) time(NULL)); srand(0);
        _eso_init();
        _window_init();
        _soup_init();
    }
    { // callbacks
        glfwSetKeyCallback(glfw_window, callback_key);
        glfwSetCursorPosCallback(glfw_window, callback_cursor_position);
        glfwSetMouseButtonCallback(glfw_window, callback_mouse_button);
        glfwSetScrollCallback(glfw_window, callback_scroll);
        glfwSetDropCallback(glfw_window, callback_drop);
        glfwSetFramebufferSizeCallback(glfw_window, callback_framebuffer_size);
    }


    char *script;
    // script = "cz0123456789";
    // script = "^osplash.drawing\nysc<m2d 20 20><m2d 16 16><m2d 16 -16><m2d -16 -16><m2d -16 16>[50\n<m3d 0 100 0 0 -1 0><m2d 0 17.5>{47\nc<m2d 16 -16>\t\t100\nsc<m2d 32 -16><m3d 74 132 113 -0.4 -0.6 -0.7>{60\n^oomax.drawing\nsq0sq1y[3\n";
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
             ".."
             "cz"
             // "^N^obug.drawing\nysa"
             ;
    #endif
    script_process(script);

    init_cameras(); // FORNOW

    {
        // FORNOW: first frame position
        double xpos, ypos;
        glfwGetCursorPos(glfw_window, &xpos, &ypos);
        callback_cursor_position(NULL, xpos, ypos);
    }

    glfwHideWindow(glfw_window);
    uint frame = 0;
    while (!glfwWindowShouldClose(glfw_window)) {
        {
            COW1._gui_x_curr = 16;
            COW1._gui_y_curr = 16;

            glfwPollEvents();

            glfwSwapBuffers(glfw_window);
            glClearColor(omax.black.x, omax.black.y, omax.black.z, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }


        other.transform_NDC_from_Pixel = window_get_NDC_from_Pixel();

        other.please_suppress_drawing_popup_popup = false;

        if (other.stepping_one_frame_while_paused) {
            other.paused = false;
        }

        if (!other.paused) { // update
            { // time_since
                real dt = 0.0167f;
                _for_each_entity_ entity->time_since_is_selected_changed += dt;
                other.time_since_cursor_start += dt;
                other.time_since_successful_feature += dt;
                other.time_since_plane_selected += dt;
                // time_since_successful_feature = 1.0f;

                bool going_inside = 0
                    || ((state.enter_mode == EnterMode::ExtrudeAdd) && (popup->extrude_add_in_length > 0.0f))
                    || (state.enter_mode == EnterMode::ExtrudeCut);
                if (!going_inside) {
                    other.time_since_going_inside = 0.0f;
                } else {
                    other.time_since_going_inside += dt;
                }
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
            if (other.show_event_stack) history_debug_draw();
            conversation_message_buffer_draw();

            if (other.paused) {
                eso_begin(other.transform_NDC_from_Pixel, SOUP_QUADS);
                eso_color(omax.white, 0.5f);
                real x = 16;
                real y = 16;
                real w = 16;
                real h = 2.5f * w;
                for_(d, 2) {
                    real o = d * (1.7f * w);
                    eso_vertex(o + x,     y    );
                    eso_vertex(o + x,     y + h);
                    eso_vertex(o + x + w, y + h);
                    eso_vertex(o + x + w, y    );
                }
                eso_end();
            }
        }

        if (frame++ == 1) glfwShowWindow(glfw_window);

        if (other.stepping_one_frame_while_paused) {
            other.stepping_one_frame_while_paused = false;
            other.paused = true;
        }
    }
}


