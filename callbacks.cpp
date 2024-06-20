Queue<RawEvent> raw_event_queue;

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
            if (!((popup->_FORNOW_active_popup_unique_ID__FORNOW_name0) && (popup->cell_type[popup->active_cell_index] == CellType::String))) { // FORNOW
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
            queue_enqueue(&raw_event_queue, raw_event);
        }
    }
}

void callback_cursor_position(GLFWwindow *, double xpos, double ypos) {
    // mouse_*
    vec2 delta_mouse_OpenGL;
    vec2 delta_mouse_World_2D;
    {
        vec2 prev_mouse_OpenGL = other.mouse_OpenGL;
        other.mouse_Pixel = { real(xpos), real(ypos) };
        other.mouse_OpenGL = transformPoint(other.OpenGL_from_Pixel, other.mouse_Pixel);
        delta_mouse_OpenGL = other.mouse_OpenGL - prev_mouse_OpenGL;
        delta_mouse_World_2D = transformVector(inverse(camera_drawing->get_PV()), delta_mouse_OpenGL);
    }

    { // hot_pane
        real x_divider_Pixel = get_x_divider_Pixel();
        real eps = 6.0f;
        real x_mouse_Pixel = other.mouse_Pixel.x;
        if (
                1
                && (popup->_FORNOW_active_popup_unique_ID__FORNOW_name0)
                && (popup->_FORNOW_info_mouse_is_hovering)
           ) {
            other.hot_pane = Pane::Popup;
        } else if (x_mouse_Pixel < x_divider_Pixel - eps) {
            other.hot_pane = Pane::Drawing;
        } else if (x_mouse_Pixel < x_divider_Pixel + eps) {
            other.hot_pane = Pane::Separator;
        } else {
            other.hot_pane = Pane::Mesh;
        }
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
            queue_enqueue(&raw_event_queue, raw_event);
        }
    }

    { // dragging divider
        if (other.mouse_left_drag_pane == Pane::Separator) {
            real prev_x_divider_OpenGL = other.x_divider_OpenGL;
            real prev_x_divider_Pixel = get_x_divider_Pixel();
            other.x_divider_OpenGL = MAG_CLAMP(LINEAR_REMAP(real(xpos), 0.0f, window_get_width_Pixel(), -1.0f, 1.0f), 0.9975f); // *
            real x_divider_Pixel = get_x_divider_Pixel();

            real dx_divider_OpenGL = 0.5f * (other.x_divider_OpenGL - prev_x_divider_OpenGL);
            camera_drawing->post_nudge_OpenGL.x += dx_divider_OpenGL;
            camera_mesh->post_nudge_OpenGL.x += dx_divider_OpenGL;

            real width_ratio_drawing = (x_divider_Pixel / prev_x_divider_Pixel);
            camera_drawing->ortho_screen_height_World /= width_ratio_drawing;
            real window_width_Pixel = window_get_width_Pixel();
            real width_ratio_mesh = (window_width_Pixel - x_divider_Pixel) / (window_width_Pixel - prev_x_divider_Pixel);
            { // camera_mesh->*
                bool is_perspective_camera = (!IS_ZERO(camera_mesh->angle_of_view));
                Camera tmp_2D = make_EquivalentCamera2D(camera_mesh);
                tmp_2D.ortho_screen_height_World /= width_ratio_mesh;
                if (is_perspective_camera) {
                    camera_mesh->persp_distance_to_origin_World = ((0.5f * tmp_2D.ortho_screen_height_World) / TAN(0.5f * camera_mesh->angle_of_view));
                } else {
                    camera_mesh->ortho_screen_height_World = tmp_2D.ortho_screen_height_World;
                }
            }
        }
    }

    { // moving cameras
        // mouse_left_drag_pane
        if (other.mouse_left_drag_pane == Pane::Mesh) {
            real fac = 2.0f;
            camera_mesh->euler_angles.y -= fac * delta_mouse_OpenGL.x;
            camera_mesh->euler_angles.x += fac * delta_mouse_OpenGL.y;
            camera_mesh->euler_angles.x = CLAMP(camera_mesh->euler_angles.x, -RAD(90), RAD(90));
        }

        // mouse_right_drag_pane
        if (other.mouse_right_drag_pane == Pane::Drawing) {
            camera_drawing->pre_nudge_World -= delta_mouse_World_2D;
        } else if (other.mouse_right_drag_pane == Pane::Mesh) {
            Camera tmp_2D = make_EquivalentCamera2D(camera_mesh);
            tmp_2D.pre_nudge_World -= transformVector(inverse(tmp_2D.get_PV()), delta_mouse_OpenGL);
            camera_mesh->pre_nudge_World = tmp_2D.pre_nudge_World;
        }
    }
}

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
            queue_enqueue(&raw_event_queue, raw_event);
        } else { ASSERT(action == GLFW_RELEASE);
            other.mouse_left_drag_pane = Pane::None;
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
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
    vec2 mouse_position_World_before  = transformPoint(inverse(camera_2D->get_PV()), other.mouse_OpenGL);
    camera_2D->ortho_screen_height_World *= (1.0f - 0.1f * real(yoffset));
    vec2 mouse_position_World_after = transformPoint(inverse(camera_2D->get_PV()), other.mouse_OpenGL);
    camera_2D->pre_nudge_World -= (mouse_position_World_after - mouse_position_World_before);
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
            camera_mesh->persp_distance_to_origin_World = ((0.5f * tmp_2D.ortho_screen_height_World) / TAN(0.5f * camera_mesh->angle_of_view));
        } else {
            camera_mesh->ortho_screen_height_World = tmp_2D.ortho_screen_height_World;
        }
        camera_mesh->pre_nudge_World = tmp_2D.pre_nudge_World;
    }
}

void callback_framebuffer_size(GLFWwindow *, int width, int height) {
    glViewport(0, 0, width, height);
}

void callback_drop(GLFWwindow *, int count, const char **paths) {
    if (count > 0) {
        void script_process(String);

        char *filename = (char *) paths[0];
        String string_filename = STRING(filename);
        script_process(STRING("\033"));
        script_process(STRING("^o"));
        script_process(string_filename);
        script_process(STRING("\n"));
    }
}

