bool click_mode_SNAP_ELIGIBLE() {
    return 0
        || (state.click_mode == ClickMode::Axis)
        || (state.click_mode == ClickMode::Box)
        || (state.click_mode == ClickMode::CenteredBox)
        || (state.click_mode == ClickMode::Circle)
        || (state.click_mode == ClickMode::Deselect)
        || (state.click_mode == ClickMode::Line)
        || (state.click_mode == ClickMode::LinearCopy)
        || (state.click_mode == ClickMode::Measure)
        || (state.click_mode == ClickMode::MirrorLine)
        || (state.click_mode == ClickMode::MirrorX)
        || (state.click_mode == ClickMode::MirrorY)
        || (state.click_mode == ClickMode::Move)
        || (state.click_mode == ClickMode::Origin)
        || (state.click_mode == ClickMode::Polygon)
        || (state.click_mode == ClickMode::Select)
        || (state.click_mode == ClickMode::RotateCopy)
        || (state.click_mode == ClickMode::RotateCopy)
        || (state.click_mode == ClickMode::TwoEdgeCircle)
        ;
}

bool click_mode_15_DEG_ELIGIBLE() {
    return 0
        || (state.click_mode == ClickMode::Axis)
        || (state.click_mode == ClickMode::Box)
        || (state.click_mode == ClickMode::Circle)
        || (state.click_mode == ClickMode::Line)
        || (state.click_mode == ClickMode::LinearCopy)
        || (state.click_mode == ClickMode::Measure)
        || (state.click_mode == ClickMode::MirrorLine)
        || (state.click_mode == ClickMode::MirrorX)
        || (state.click_mode == ClickMode::MirrorY)
        || (state.click_mode == ClickMode::Move)
        || (state.click_mode == ClickMode::Origin)
        || (state.click_mode == ClickMode::Polygon)
        || (state.click_mode == ClickMode::Rotate)
        || (state.click_mode == ClickMode::RotateCopy)
        || (state.click_mode == ClickMode::TwoEdgeCircle)
        ;
}

bool click_mode_SPACE_BAR_REPEAT_ELIGIBLE() {
    return 0
        || (state.click_mode == ClickMode::Axis)
        || (state.click_mode == ClickMode::Box)
        || (state.click_mode == ClickMode::Circle)
        || (state.click_mode == ClickMode::Fillet)
        || (state.click_mode == ClickMode::Line)
        || (state.click_mode == ClickMode::Measure)
        || (state.click_mode == ClickMode::MirrorLine)
        || (state.click_mode == ClickMode::MirrorX)
        || (state.click_mode == ClickMode::MirrorY)
        || (state.click_mode == ClickMode::Move)
        || (state.click_mode == ClickMode::Origin)
        || (state.click_mode == ClickMode::Polygon)
        || (state.click_mode == ClickMode::Rotate)
        || (state.click_mode == ClickMode::RotateCopy)
        || (state.click_mode == ClickMode::TwoClickDivide)
        || (state.click_mode == ClickMode::TwoEdgeCircle)
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
