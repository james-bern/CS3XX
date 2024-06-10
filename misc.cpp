#define _for_each_entity_ for (\
        Entity *entity = drawing->entities.array;\
        entity < &drawing->entities.array[drawing->entities.length];\
        ++entity)

#define _for_each_selected_entity_ _for_each_entity_ if (entity->is_selected)

template <typename T> void JUICEIT_EASYTWEEN(T *a, T b) {
    real f = 0.1f;
    if (!other.paused) *a += f * (b - *a);
}

real _JUICEIT_EASYTWEEN(real t) { return 0.287f * log(t) + 1.172f; }


real get_x_divider_Pixel() { return LINEAR_REMAP(other.x_divider_OpenGL, -1.0f, 1.0f, 0.0f, window_get_size_Pixel().x); }

vec2 magic_snap(vec2 before, bool calling_this_function_for_drawing_preview = false) {
    vec2 result = before;
    {
        if (
                ( 0 
                  || (state.click_mode == ClickMode::Line)
                  || (state.click_mode == ClickMode::Axis)
                )
                && (two_click_command->awaiting_second_click)
                && (other.shift_held)) {
            vec2 a = two_click_command->first_click;
            vec2 b = before;
            vec2 r = b - a; 
            real norm_r = norm(r);
            real factor = 360 / 15 / TAU;
            real theta = roundf(atan2(r) * factor) / factor;
            result = a + norm_r * e_theta(theta);
        } else if (
                (state.click_mode == ClickMode::BoundingBox)
                && (two_click_command->awaiting_second_click)
                && (other.shift_held)) {
            // TODO (Felipe): snap square
            result = before;
        } else if (!calling_this_function_for_drawing_preview) { // NOTE: this else does, in fact, match LAYOUT's behavior
            if (state.click_modifier == ClickModifier::Center) {
                real min_squared_distance = HUGE_VAL;
                _for_each_entity_ {
                    if (entity->type == EntityType::Line) {
                        continue;
                    } else { ASSERT(entity->type == EntityType::Arc);
                        ArcEntity *arc_entity = &entity->arc_entity;
                        real squared_distance = squared_distance_point_dxf_arc(before, arc_entity);
                        if (squared_distance < min_squared_distance) {
                            min_squared_distance = squared_distance;
                            result = arc_entity->center;
                        }
                    }
                }
            } else if (state.click_modifier == ClickModifier::Middle) {
                real min_squared_distance = HUGE_VAL;
                _for_each_entity_ {
                    real squared_distance = squared_distance_point_entity(before, entity);
                    if (squared_distance < min_squared_distance) {
                        min_squared_distance = squared_distance;
                        result = entity_get_middle(entity);
                    }
                }
            } else if (state.click_modifier == ClickModifier::End) {
                real min_squared_distance = HUGE_VAL;
                _for_each_entity_ {
                    vec2 p[2];
                    entity_get_start_and_end_points(entity, &p[0], &p[1]);
                    for_(d, 2) {
                        real squared_distance = squaredDistance(before, p[d]);
                        if (squared_distance < min_squared_distance) {
                            min_squared_distance = squared_distance;
                            result = p[d];
                        }
                    }
                }
            }
        }
    }
    return result;
}

void init_camera_drawing() {
    *camera_drawing = make_Camera2D(100.0f, {}, { -0.5f, -0.125f });
    if (drawing->entities.length) camera2D_zoom_to_bbox(&other.camera_drawing, entities_get_bbox(&drawing->entities));
}
void init_camera_mesh() {
    if (mesh->num_vertices) {
        *camera_mesh = make_OrbitCamera3D(
                CAMERA_3D_PERSPECTIVE_ANGLE_OF_VIEW,
                2.0f * MIN(150.0f, other.camera_drawing.ortho_screen_height_World),
                { RAD(-44.0f), RAD(33.0f) },
                {},
                { 0.5f, -0.125f }
                );
    }
}

bool click_mode_SELECT_OR_DESELECT() {
    return ((state.click_mode == ClickMode::Select) || (state.click_mode == ClickMode::Deselect));
}

bool _non_WINDOW__SELECT_DESELECT___OR___SET_COLOR() {
    return ((click_mode_SELECT_OR_DESELECT() && (state.click_modifier != ClickModifier::Window)) || (state.click_mode == ClickMode::Color));
}

bool _SELECT_OR_DESELECT_COLOR() {
    bool A = click_mode_SELECT_OR_DESELECT();
    bool B = (state.click_modifier == ClickModifier::Color);
    return A && B;
}

bool click_mode_SNAP_ELIGIBLE() {
    return 0
        || (state.click_mode == ClickMode::Axis)
        || (state.click_mode == ClickMode::BoundingBox)
        || (state.click_mode == ClickMode::Circle)
        || (state.click_mode == ClickMode::Line)
        || (state.click_mode == ClickMode::Measure)
        || (state.click_mode == ClickMode::Move)
        || (state.click_mode == ClickMode::Origin)
        || (state.click_mode == ClickMode::MirrorX)
        || (state.click_mode == ClickMode::MirrorY)
        ;
}

bool click_mode_SPACE_BAR_REPEAT_ELIGIBLE() {
    return 0
        || (state.click_mode == ClickMode::Axis)
        || (state.click_mode == ClickMode::BoundingBox)
        || (state.click_mode == ClickMode::Circle)
        || (state.click_mode == ClickMode::Fillet)
        || (state.click_mode == ClickMode::Line)
        || (state.click_mode == ClickMode::Measure)
        || (state.click_mode == ClickMode::Move)
        || (state.click_mode == ClickMode::Origin)
        || (state.click_mode == ClickMode::MirrorX)
        || (state.click_mode == ClickMode::MirrorY)
        ;
}

bool enter_mode_SHIFT_SPACE_BAR_REPEAT_ELIGIBLE() {
    return 0
        || (state.enter_mode == EnterMode::ExtrudeAdd)
        || (state.enter_mode == EnterMode::ExtrudeCut)
        || (state.enter_mode == EnterMode::RevolveAdd)
        || (state.enter_mode == EnterMode::RevolveCut)
        || (state.enter_mode == EnterMode::NudgeFeaturePlane)
        ;
}
