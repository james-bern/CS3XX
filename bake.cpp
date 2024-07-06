KeyEventSubtype classify_baked_subtype_of_raw_key_event(RawKeyEvent *raw_key_event) {
    if (!popup->_FORNOW_active_popup_unique_ID__FORNOW_name0) return KeyEventSubtype::Hotkey;

    uint key = raw_key_event->key;
    bool control = raw_key_event->control;
    // bool shift = raw_key_event->shift;

    // TODO: these need to take control into account
    bool key_is_digit = ('0' <= key) && (key <= '9');
    bool key_is_period = (key == '.');
    bool key_is_hyphen = (key == '-');
    bool key_is_alpha = ('A' <= key) && (key <= 'Z');
    bool key_is_delete = (key == GLFW_KEY_BACKSPACE) || (key == GLFW_KEY_DELETE);
    bool key_is_enter = (key == GLFW_KEY_ENTER);
    bool key_is_nav = (key == GLFW_KEY_TAB) || (key == GLFW_KEY_LEFT) || (key == GLFW_KEY_RIGHT);
    bool key_is_ctrl_a = (key == 'A') && (control);
    bool key_is_slash = (key == '/') || (key == '\\');
    bool key_is_colon = (key == ':');
    bool key_is_space = (key == ' ');
    bool key_is_underscore = (key == '_');

    bool is_consumable_by_popup; {
        is_consumable_by_popup = false;
        if (!_SELECT_OR_DESELECT_COLOR()) is_consumable_by_popup |= key_is_digit;
        is_consumable_by_popup |= key_is_delete;
        is_consumable_by_popup |= key_is_enter;
        is_consumable_by_popup |= key_is_nav;
        is_consumable_by_popup |= key_is_ctrl_a;
        if (popup->_type_of_active_cell == CellType::Real) {
            is_consumable_by_popup |= key_is_hyphen;
            is_consumable_by_popup |= key_is_period;
        } else if (popup->_type_of_active_cell == CellType::Uint) {
            ;
        } else if (popup->_type_of_active_cell == CellType::String) {
            is_consumable_by_popup |= key_is_alpha;
            is_consumable_by_popup |= key_is_colon;
            is_consumable_by_popup |= key_is_hyphen;
            is_consumable_by_popup |= key_is_period;
            is_consumable_by_popup |= key_is_slash;
            is_consumable_by_popup |= key_is_space;
            is_consumable_by_popup |= key_is_underscore;
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
        mouse_event->mouse_Pixel = raw_mouse_event->mouse_Pixel;
        mouse_event->mouse_held = raw_mouse_event->mouse_held;
        {
            if (raw_mouse_event->pane == Pane::Drawing) {
                mat4 World_2D_from_OpenGL = inverse(camera_drawing->get_PV());
                vec2 mouse_World_2D = transformPoint(World_2D_from_OpenGL, other.mouse_OpenGL);

                mouse_event->subtype = MouseEventSubtype::Drawing;

                MouseEventDrawing *mouse_event_drawing = &mouse_event->mouse_event_drawing;
                mouse_event_drawing->mouse_position = magic_snap(mouse_World_2D);
            } else if (raw_mouse_event->pane == Pane::Mesh) {
                mat4 World_3D_from_OpenGL = inverse(camera_mesh->get_PV());
                vec3 point_a = transformPoint(World_3D_from_OpenGL, V3(other.mouse_OpenGL, -1.0f));
                vec3 point_b = transformPoint(World_3D_from_OpenGL, V3(other.mouse_OpenGL,  1.0f));

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
            } else { ASSERT(raw_mouse_event->pane == Pane::DrawingMeshSeparator);
                event = {};
            }
        }
    }
    return event;
}

////////////////////////////////////////////////////////////////////////////////

Event make_mouse_event_2D(vec2 mouse_position) {
    Event event = {};
    event.type = EventType::Mouse;
    MouseEvent *mouse_event = &event.mouse_event;
    mouse_event->subtype = MouseEventSubtype::Drawing;
    MouseEventDrawing *mouse_event_drawing = &mouse_event->mouse_event_drawing;
    mouse_event_drawing->mouse_position = mouse_position;
    return event;
}
Event make_mouse_event_2D(real x, real y) { return make_mouse_event_2D({ x, y }); }

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

