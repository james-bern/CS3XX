StandardEventProcessResult standard_event_process(Event event); // forward declaration

#ifdef DEBUG_HISTORY_DISABLE_HISTORY_ENTIRELY
//
void history_process_event(Event standard_event) { _standard_event_process_NOTE_RECURSIVE(standard_event); }
void history_undo() { messagef("[DEBUG] history disabled"); }
void history_redo() { messagef("[DEBUG] history disabled"); }
void history_debug_draw() { gui_printf("[DEBUG] history disabled"); }
//
#else
//
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

void history_process_event(Event standard_event) {
    StandardEventProcessResult tmp = standard_event_process(standard_event);
    standard_event.record_me = tmp.record_me;
    standard_event.checkpoint_me = tmp.checkpoint_me;
    #ifndef DEBUG_HISTORY_DISABLE_SNAPSHOTTING
    standard_event.snapshot_me = tmp.snapshot_me;
    #endif
    if (standard_event.record_me) PUSH_UNDO_CLEAR_REDO(standard_event);
}

void history_undo() {
    if (elephant_is_empty_undo(&history.recorded_user_events)) {
        messagef(pallete.orange, "Undo: nothing to undo");
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
        for (Event *event = begin; event < one_past_end; ++event) standard_event_process(*event);
    } other.please_suppress_messagef = false;

    messagef(pallete.light_gray, "Undo");
}


void history_redo() {
    if (elephant_is_empty_redo(&history.recorded_user_events)) {
        messagef(pallete.orange, "Redo: nothing to redo");
        return;
    }

    other.please_suppress_messagef = true;
    {
        while (EVENT_REDO_NONEMPTY()) { // // manipulate stacks (undo <- redo)
            StackPointers popped = POP_REDO_ONTO_UNDO();
            Event *user_event = popped.user_event;
            WorldState_ChangesToThisMustBeRecorded_state *world_state = popped.world_state;

            standard_event_process(*user_event);
            if (world_state) {
                world_state_free_AND_zero(&state);
                world_state_deep_copy(&state, world_state);
            }

            if (user_event->checkpoint_me) break;
        }
    }
    other.please_suppress_messagef = false;

    messagef(pallete.light_gray, "Redo");
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

void history_debug_draw() {
    { // FORNOW
        eso_begin(M4_Identity(), SOUP_QUADS);
        eso_overlay(true);
        eso_color(pallete.black, 0.7f);
        BoundingBox<2> bbox_OpenGL = { -1.0f, -1.0f, 1.0f, 1.0f };
        eso_bbox_SOUP_QUADS(bbox_OpenGL);
        eso_end();
    }
    EasyTextPen pen;

    auto _history_user_event_draw_helper = [&](Event event) {
        char message[256]; {
            // TODO: handle shift and control with the special characters
            if (event.type == EventType::Key) {
                KeyEvent *key_event = &event.key_event;
                char *boxed; {
                    if (key_event->subtype == KeyEventSubtype::Hotkey) {
                        boxed = "[KEY_HOTKEY]";
                    } else { ASSERT(key_event->subtype == KeyEventSubtype::Popup);
                        boxed = "[KEY_POPUP]";
                    }
                }
                if (!key_event->_name_of_spoofing_button) {
                    sprintf(message, "%s %s", boxed, key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(key_event));
                } else {
                    boxed = "[BUTTON]"; // FORNOW
                    sprintf(message, "%s %s", boxed, key_event->_name_of_spoofing_button);
                }
            } else { ASSERT(event.type == EventType::Mouse);
                MouseEvent *mouse_event = &event.mouse_event;
                if (mouse_event->subtype == MouseEventSubtype::Drawing) {
                    MouseEventDrawing *mouse_event_drawing = &mouse_event->mouse_event_drawing;
                    sprintf(message, "[MOUSE_DRAWING] %g %g", mouse_event_drawing->unsnapped_position.x, mouse_event_drawing->unsnapped_position.y);
                } else if (mouse_event->subtype == MouseEventSubtype::Mesh) {
                    MouseEventMesh *mouse_event_mesh = &mouse_event->mouse_event_mesh;
                    sprintf(message, "[MOUSE_MESH] %g %g %g %g %g %g", mouse_event_mesh->mouse_ray_origin.x, mouse_event_mesh->mouse_ray_origin.y, mouse_event_mesh->mouse_ray_origin.z, mouse_event_mesh->mouse_ray_direction.x, mouse_event_mesh->mouse_ray_direction.y, mouse_event_mesh->mouse_ray_direction.z);
                } else if (mouse_event->subtype == MouseEventSubtype::Popup) {
                    MouseEventPopup *mouse_event_popup = &mouse_event->mouse_event_popup;
                    sprintf(message, "[MOUSE_POPUP] %d %d", mouse_event_popup->cell_index, mouse_event_popup->cursor);
                } else { ASSERT(mouse_event->subtype == MouseEventSubtype::ToolboxButton);
                    MouseEventToolboxButton *mouse_event_toolbox_button = &mouse_event->mouse_event_toolbox_button;
                    sprintf(message, "[MOUSE_TOOLBOX_BUTTON] %s", mouse_event_toolbox_button->name);
                }
            }
        }
        easy_text_drawf(&pen, "%c%c %s",
                (event.checkpoint_me)   ? 'C' : ' ',
                (event.snapshot_me)     ? 'S' : ' ',
                message);
    };

    auto _history_world_state_draw_helper = [&](WorldState_ChangesToThisMustBeRecorded_state *world_state) {
        easy_text_drawf(&pen, "%d elements  %d triangles", world_state->drawing.entities.length, world_state->meshes.work.num_triangles);
    };

    pen = { V2(12.0f, 12.0f), 12.0f, pallete.white, true };

    { // recorded_user_events
        if (history.recorded_user_events._redo_stack.length) {
            for (////
                    Event *event = history.recorded_user_events._redo_stack.array;
                    event < history.recorded_user_events._redo_stack.array + history.recorded_user_events._redo_stack.length;
                    ++event
                ) {//
                _history_user_event_draw_helper(*event);
            }
            pen.color = pallete.cyan; {
                easy_text_drawf(&pen, "^ redo (%d)", elephant_length_redo(&history.recorded_user_events));
            } pen.color = pallete.white;
        }
        if ((history.recorded_user_events._redo_stack.length) || (history.recorded_user_events._undo_stack.length)) {
            pen.color = pallete.cyan; {
                easy_text_drawf(&pen, "  RECORDED_EVENTS");
            } pen.color = pallete.white;
        } else {
            pen.color = pallete.cyan; {
                easy_text_drawf(&pen, "--- no history ---");
            } pen.color = pallete.white;
        }
        if (history.recorded_user_events._undo_stack.length) {
            pen.color = pallete.cyan; {
                easy_text_drawf(&pen, "v undo (%d)", elephant_length_undo(&history.recorded_user_events));
            } pen.color = pallete.white;
            for (////
                    Event *event = history.recorded_user_events._undo_stack.array + (history.recorded_user_events._undo_stack.length - 1);
                    event >= history.recorded_user_events._undo_stack.array;
                    --event
                ) {//
                _history_user_event_draw_helper(*event);
            }
        }
    }

    pen.origin.x += get_x_divider_drawing_mesh_Pixel() + 12.0f;
    pen.offset_Pixel = {};

    { // snapshotted_world_states
        if (history.snapshotted_world_states._redo_stack.length) {
            for (////
                    WorldState_ChangesToThisMustBeRecorded_state *world_state = history.snapshotted_world_states._redo_stack.array;
                    world_state < history.snapshotted_world_states._redo_stack.array + history.snapshotted_world_states._redo_stack.length;
                    ++world_state
                ) {//
                _history_world_state_draw_helper(world_state);
            }
            pen.color = pallete.cyan; {
                easy_text_drawf(&pen, "^ redo (%d)", elephant_length_redo(&history.snapshotted_world_states));
            } pen.color = pallete.white;
        }
        if ((history.snapshotted_world_states._redo_stack.length) || (history.snapshotted_world_states._undo_stack.length)) {
            pen.color = pallete.cyan; {
                easy_text_drawf(&pen, "  SNAPSHOTTED_WORLD_STATES");
            } pen.color = pallete.white;
        }
        if (history.snapshotted_world_states._undo_stack.length) {
            pen.color = pallete.cyan; {
                easy_text_drawf(&pen, "v undo (%d)", elephant_length_undo(&history.snapshotted_world_states));
            } pen.color = pallete.white;
            for (////
                    WorldState_ChangesToThisMustBeRecorded_state *world_state = history.snapshotted_world_states._undo_stack.array + (history.snapshotted_world_states._undo_stack.length - 1);
                    world_state >= history.snapshotted_world_states._undo_stack.array;
                    --world_state
                ) {//
                _history_world_state_draw_helper(world_state);
            }
        }
    }
}
//
#endif
