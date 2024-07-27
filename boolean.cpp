bool click_mode_SPACE_BAR_REPEAT_ELIGIBLE() {
    return 0
        || (state_Draw_command_is_(Axis))
        || (state_Draw_command_is_(Box))
        || (state_Draw_command_is_(Circle))
        || (state_Draw_command_is_(Fillet))
        || (state_Draw_command_is_(Line))
        || (state_Draw_command_is_(Measure))
        || (state_Draw_command_is_(Mirror2))
        || (state_Draw_command_is_(XMirror))
        || (state_Draw_command_is_(YMirror))
        || (state_Draw_command_is_(Move))
        || (state_Draw_command_is_(Origin))
        || (state_Draw_command_is_(Polygon))
        || (state_Draw_command_is_(Rotate))
        || (state_Draw_command_is_(RCopy))
        || (state_Draw_command_is_(Divide2))
        || (state_Draw_command_is_(DiamCircle))
        ;
}

bool click_mode_SELECT_OR_DESELECT() {
    return ((state_Draw_command_is_(Select)) || (state_Draw_command_is_(Deselect)));
}

bool click_mode_WINDOW_SELECT_OR_WINDOW_DESELECT() {
    return (click_mode_SELECT_OR_DESELECT() && (state_Snap_command_is_(Window)));
}


bool first_click_must_acquire_entity() {
    return 0 ||
        (state_Draw_command_is_(DogEar)) ||
        (state_Draw_command_is_(Fillet)) ||
        (state_Draw_command_is_(Divide2))
        ;
}

bool enter_mode_SHIFT_SPACE_BAR_REPEAT_ELIGIBLE() {
    return 0
        || (state_Mesh_command_is_(ExtrudeAdd))
        || (state_Mesh_command_is_(ExtrudeCut))
        || (state_Mesh_command_is_(RevolveAdd))
        || (state_Mesh_command_is_(RevolveCut))
        || (state_Mesh_command_is_(NudgePlane))
        ;
}

bool _non_WINDOW__SELECT_DESELECT___OR___SET_COLOR() {
    return (
            ((!state_Xsel_command_is_(Window)) && click_mode_SELECT_OR_DESELECT())
            || (state_Draw_command_is_(Color)));
}

bool _SELECT_OR_DESELECT_COLOR() {
    bool A = click_mode_SELECT_OR_DESELECT();
    bool B = (state_Snap_command_is_(Color));
    return A && B;
}
