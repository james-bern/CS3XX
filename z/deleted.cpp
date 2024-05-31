#if 0
UserEvent HOTKEY_event(uint32 key, bool32 super = false, bool32 shift = false) {
    ASSERT(!(('a' <= key) && (key <= 'z')));
    UserEvent event = {};
    event.type = USER_EVENT_TYPE_HOTKEY_PRESS;
    event.key = key;
    event.super = super;
    event.shift = shift;
    return event;
}
#endif


