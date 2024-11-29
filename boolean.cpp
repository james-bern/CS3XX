bool click_mode_SPACE_BAR_REPEAT_ELIGIBLE() {
    return 0
        || (state_Draw_command_is_(Box))
        || (state_Draw_command_is_(Circle))
        || (state_Draw_command_is_(Fillet))
        || (state_Draw_command_is_(Line))
        || (state_Draw_command_is_(Measure))
        || (state_Draw_command_is_(Mirror2))
        || (state_Draw_command_is_(MirrorX))
        || (state_Draw_command_is_(MirrorY))
        || (state_Draw_command_is_(Translate))
        || (state_Draw_command_is_(SetOrigin))
        || (state_Draw_command_is_(Polygon))
        || (state_Draw_command_is_(Rotate))
        || (state_Draw_command_is_(RCopy))
        || (state_Draw_command_is_(Divide2))
        ;
}

bool SELECT_OR_DESELECT() {
    return ((state_Draw_command_is_(Select)) || (state_Draw_command_is_(Deselect)));
}

bool WINDOW_SELECT_OR_WINDOW_DESELECT() {
    return (SELECT_OR_DESELECT() && (state_Xsel_command_is_(Window)));
}

bool _non_WINDOW__SELECT_DESELECT___OR___SET_COLOR() {
    return (
            ((!state_Xsel_command_is_(Window)) && SELECT_OR_DESELECT())
            || (state_Draw_command_is_(SetColor)));
}

bool _SELECT_OR_DESELECT_COLOR() {
    bool A = SELECT_OR_DESELECT();
    bool B = (state_Draw_command_is_(SetColor));
    return A && B;
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

