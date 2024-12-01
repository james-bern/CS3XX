// // TODO: need to review how the undo-redo system actually works
// i think we have a fundamental off by one in the new memory handling approach


//
// terminology
// ===========
// big checkpoint (changes the mesh -- drastically)
// little checkpoint (changes the other state, e.g., dxf, feature_plane, ...)
//
#if 0
struct History {
    struct {
        Stack<UserEvent> undo_stack;
        Stack<UserEvent> redo_stack;
    } user_events;
    struct {
        Stack<UserEvent> undo_stack;
        Stack<UserEvent> redo_stack;
    } state_immediately_AFTER_big_checkpoint_user_events;
}
#endif
//
// user_events (elephant stack)
// ==================
// . . C . . . c . . . . c . . $ $ $ $ $ $
//     ^                       ^ ^
//     |                       | |
//     |                    undo redo
//     super-Undo
//
// (.)-regular event, little-(c)heckpoint, big-(C)heckpoint
// ($)-redo event (types not drawn)
//
// state_immediately_AFTER_big_checkpoint_user_events (elephant stack)
// ====================
// C C C . D D D D
//       ^
//       |
//       state
//
// pseudocode
// ==========
// do-fresh-event
// - free user_events.redo-stack
// - process_event
// - push into user_event_history
// - if was big checkpoint 
//   - push big checkpoint stack
//


// TODO TODO TODO TODO
// NOTE: with the current-ish architecture, we should push the *result*.mesh of a super checkpoint event to the stack (TODO not what we did in notebook)
//       (the previous mesh is going away -- or, rather, is already stored on the undo stack)


// TODO: get the mesh/dxf split on a mesh-changing feature into the code

// TODO: make good map implementation
// TODO: don't store FancyMesh as a bunch of alloc'd pointers

void history_process_and_record_standard_fresh_user_event(UserEvent fresh_event_from_user) {
    StandardEventProcessResult standard_event_process_result = standard_event_process(fresh_event_from_user);
    uint32 category = standard_event_process_result.category;

    if (category == PROCESSED_EVENT_CATEGORY_DONT_RECORD) {
    } else if (category == PROCESSED_EVENT_CATEGORY_KILL_HISTORY) {
        ASSERT(false); // TODO
                       // history_B_redo = history_C_one_past_end_of_redo = history_A_undo;
                       // queue_free_AND_zero(&queue_of_fresh_events_from_user);
                       // super_stacks_do__NOTE_clears_redo_stack__FORNOW_TODO_ANOTHER_BETTER_SIMPLER_FUNCTION();
    } else {
        { // FORNOW (sloppy): assign checkpoint_type
            fresh_event_from_user.checkpoint_type = CHECKPOINT_TYPE_NONE; // FORNOW (not necessary)
            if ((
                        0
                        || (category == PROCESSED_EVENT_CATEGORY_CHECKPOINT)
                        || (category == PROCESSED_EVENT_CATEGORY_SUPER_CHECKPOINT)
                )
                    && (!fresh_event_from_user.checkpoint_ineligible)
               ) {
                if (category == PROCESSED_EVENT_CATEGORY_CHECKPOINT) {
                    fresh_event_from_user.checkpoint_type = CHECKPOINT_TYPE_CHECKPOINT;
                } else if (category == PROCESSED_EVENT_CATEGORY_SUPER_CHECKPOINT) {
                    fresh_event_from_user.checkpoint_type = CHECKPOINT_TYPE_SUPER_CHECKPOINT;
                }
            } 
        }

        { // history_push_back(...)
            *history_B_redo++ = fresh_event_from_user;
            history_C_one_past_end_of_redo = history_B_redo; // kill redo "stack"
        }

        if (category == PROCESSED_EVENT_CATEGORY_SUPER_CHECKPOINT) { // push super checkpoint if necessary
            ASSERT(standard_event_process_result.mesh.num_vertices);

            { // TODO name

                // // 0) free the redo stack
                _super_stacks_free_redo_stack();

                // // 1) push shallow copy of entire state onto the undo stack
                stack_push(&super_undo_stack, state);

                // // 2a) deep copy everything but mesh
                { // FORNOW TODO: arena copy
                    WorldState deep_copy; {
                        deep_copy = state;
                        deep_copy.dxf.entities = {};
                        deep_copy.dxf.is_selected = {};
                        list_clone(&deep_copy.dxf.entities,    &state.dxf.entities   );
                        list_clone(&deep_copy.dxf.is_selected, &state.dxf.is_selected);
                    }
                    state = deep_copy;
                }
                // 2b) update mesh from standard_event_process_result
                state.mesh = standard_event_process_result.mesh;

            }

        }

    }

}

void history_perform_undo() {
    #define _UNDO_STACK_NONEMPTY_ (history_A_undo != history_B_redo)

    if (_UNDO_STACK_NONEMPTY_) {

        // TODO: think this over

        bool32 just_undid_super_checkpoint = false;
        // // move history_B_redo
        // pop back _through_ a first checkpoint__NOTE_set_in_new_event_process
        do {
            --history_B_redo;
            just_undid_super_checkpoint |= (history_B_redo->checkpoint_type == CHECKPOINT_TYPE_SUPER_CHECKPOINT);
        } while ((_UNDO_STACK_NONEMPTY_) && (history_B_redo->checkpoint_type == CHECKPOINT_TYPE_NONE));
        // * short-circuit pop back _up to_ a second checkpoint
        while ((_UNDO_STACK_NONEMPTY_) && ((history_B_redo - 1)->checkpoint_type == CHECKPOINT_TYPE_NONE)) {
            --history_B_redo;
        }

        // TODO: think this over

        state = {}; // TODO: think this over
        {
            if (!just_undid_super_checkpoint) {
                _super_stacks_deepcopy_peek_undo_stack_NOTE_frees();
                // TODO: name

            } else {
                super_stacks_undo_and_load();
                // TODO: name

            }
        }

        UserEvent *history_D_one_after_last_super_checkpoint; {
            history_D_one_after_last_super_checkpoint = history_B_redo;
            while ((history_A_undo != history_D_one_after_last_super_checkpoint) && (history_D_one_after_last_super_checkpoint - 1)->checkpoint_type != CHECKPOINT_TYPE_SUPER_CHECKPOINT) --history_D_one_after_last_super_checkpoint;
        }

        for (UserEvent *event = history_D_one_after_last_super_checkpoint; event < history_B_redo; ++event) standard_event_process(*event);

        conversation_messagef("[undo] success");
    } else {
        conversation_messagef("[undo] nothing to undo");
    }
}

void history_perform_redo() {
    if (history_B_redo != history_C_one_past_end_of_redo) {
        do {
            {
                bool32 top_of_redo_stack_is_super_checkpoint = (history_B_redo->checkpoint_type != CHECKPOINT_TYPE_SUPER_CHECKPOINT);
                standard_event_process(*history_B_redo, top_of_redo_stack_is_super_checkpoint);
                if (top_of_redo_stack_is_super_checkpoint) {
                    super_stacks_redo_and_load();
                    // TODO: name

                }
            }
            ++history_B_redo;
        } while ((history_B_redo != history_C_one_past_end_of_redo) && ((history_B_redo - 1)->checkpoint_type == CHECKPOINT_TYPE_NONE));
        conversation_messagef("[redo] success");
    } else {
        conversation_messagef("[redo] nothing to redo");
    }
}

