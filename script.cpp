void script_process(String string) {
    // TODO: gui
    // TODOLATER (weird 'X' version): char *string = "^osplash.drawing\nyscx2020\ne\t50";
    #define TAG_LENGTH 3
    bool control = false;
    for_(i, string.length) {
        char c = string.data[i];
        if (c == '^') {
            control = true;
        } else if (c == '<') {
            bool is_instabaked = true;
            Event instabaked_event = {};
            RawEvent _raw_event = {};
            {
                uint next_i; {
                    next_i = i;
                    while (string.data[++next_i] != '>') {} // ++next_i intentional
                }
                {
                    char *tag = &string.data[i + 1];
                    if (strncmp(tag, "m2d", TAG_LENGTH) == 0) {
                        char *params = &string.data[i + 1 + TAG_LENGTH];
                        vec2 unsnapped_mouse_position; {
                            sscanf(params, "%f %f", &unsnapped_mouse_position.x, &unsnapped_mouse_position.y);
                        }
                        instabaked_event = make_mouse_event_2D(unsnapped_mouse_position);
                    } else if (strncmp(tag, "m3d", TAG_LENGTH) == 0) {
                        char *params = &string.data[i + 1 + TAG_LENGTH];
                        vec3 mouse_ray_origin;
                        vec3 mouse_ray_direction;
                        sscanf(params, "%f %f %f %f %f %f", &mouse_ray_origin.x, &mouse_ray_origin.y, &mouse_ray_origin.z, &mouse_ray_direction.x, &mouse_ray_direction.y, &mouse_ray_direction.z);
                        mouse_ray_direction = normalized(mouse_ray_direction);
                        instabaked_event = make_mouse_event_3D(mouse_ray_origin, mouse_ray_direction);
                        // DEBUGBREAK();
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
                history_process_event(instabaked_event);
            } else {
                Event freshly_baked_event = bake_event(_raw_event);
                history_process_event(freshly_baked_event);
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
                } else if (c == '\33') {
                    raw_key_event->key = GLFW_KEY_ESCAPE;
                } else if (c == '\b') {
                    raw_key_event->key = GLFW_KEY_BACKSPACE;
                } else {
                    raw_key_event->key = c;
                }
            }
            control = false;
            Event freshly_baked_event = bake_event(raw_event);
            history_process_event(freshly_baked_event);
        }
    }
}
