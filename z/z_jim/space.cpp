// TODO: this one can actually go in the above layer?
// (do it once everything is more firmed up)

    if (
            (event.type == USER_EVENT_TYPE_KEY_PRESS)
            && (event.key == ' ')
            && (!event.super)
            && (!event.shift)
       ) {
        event = {};
        event.type = USER_EVENT_TYPE_KEY_PRESS;
        event.key = _global_screen_state.space_bar_event_key;
    }


char space_bar_event_key;
