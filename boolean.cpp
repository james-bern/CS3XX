bool click_mode_SNAP_ELIGIBLE() {
    return 0
        || (state_Draw_command_is_(Axis))
        || (state_Draw_command_is_(Box))
        || (state_Draw_command_is_(CenteredBox))
        || (state_Draw_command_is_(Circle))
        || (state_Draw_command_is_(Line))
        || (state_Draw_command_is_(Copy))
        || (state_Draw_command_is_(Measure))
        || (state_Draw_command_is_(Mirror2))
        || (state_Draw_command_is_(XMirror))
        || (state_Draw_command_is_(YMirror))
        || (state_Draw_command_is_(Move))
        || (state_Draw_command_is_(Origin))
        || (state_Draw_command_is_(Polygon))
        || (state_Draw_command_is_(Rotate))
        || (state_Draw_command_is_(RCopy))
        || (state_Draw_command_is_(DiamCircle))
        ;
}

bool click_mode_15_DEG_ELIGIBLE() {
    return 0
        || (state_Draw_command_is_(Axis))
        || (state_Draw_command_is_(Box))
        || (state_Draw_command_is_(Circle))
        || (state_Draw_command_is_(Line))
        || (state_Draw_command_is_(Copy))
        || (state_Draw_command_is_(Measure))
        || (state_Draw_command_is_(Mirror2))
        || (state_Draw_command_is_(XMirror))
        || (state_Draw_command_is_(YMirror))
        || (state_Draw_command_is_(Move))
        || (state_Draw_command_is_(Origin))
        || (state_Draw_command_is_(Polygon))
        || (state_Draw_command_is_(Rotate))
        || (state_Draw_command_is_(RCopy))
        || (state_Draw_command_is_(DiamCircle))
        ;
}

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
    return (click_mode_SELECT_OR_DESELECT() && (state.click_modifier == ClickModifier::Window));
}

bool click_mode_TWO_CLICK_COMMAND() {
    return 0 ||
        (state_Draw_command_is_(Axis)) ||
        (state_Draw_command_is_(Box)) ||
        (state_Draw_command_is_(CenteredBox)) ||
        (state_Draw_command_is_(Circle)) ||
        (state_Draw_command_is_(Fillet)) ||
        (state_Draw_command_is_(DogEar)) ||
        (state_Draw_command_is_(Line)) ||
        (state_Draw_command_is_(Copy)) ||
        (state_Draw_command_is_(Measure)) ||
        (state_Draw_command_is_(Mirror2)) ||
        (state_Draw_command_is_(Move)) ||
        (state_Draw_command_is_(Polygon)) ||
        (state_Draw_command_is_(Rotate)) ||
        (state_Draw_command_is_(RCopy)) ||
        (state_Draw_command_is_(DiamCircle)) ||
        (state_Draw_command_is_(Divide2)) ||
        click_mode_WINDOW_SELECT_OR_WINDOW_DESELECT(); // fornow wonky case
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
        || (state.enter_mode == EnterMode::ExtrudeAdd)
        || (state.enter_mode == EnterMode::ExtrudeCut)
        || (state.enter_mode == EnterMode::RevolveAdd)
        || (state.enter_mode == EnterMode::RevolveCut)
        || (state.enter_mode == EnterMode::NudgePlane)
        ;
}

bool _non_WINDOW__SELECT_DESELECT___OR___SET_COLOR() {
    return ((click_mode_SELECT_OR_DESELECT() && (state.click_modifier != ClickModifier::Window)) || (state_Draw_command_is_(Color)));
}

bool _SELECT_OR_DESELECT_COLOR() {
    bool A = click_mode_SELECT_OR_DESELECT();
    bool B = (state.click_modifier == ClickModifier::Color);
    return A && B;
}
