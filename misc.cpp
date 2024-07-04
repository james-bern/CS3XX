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


real get_x_divider_drawing_mesh_Pixel() {
    return LINEAR_REMAP(other.x_divider_drawing_mesh_OpenGL, -1.0f, 1.0f, 0.0f, window_get_size_Pixel().x);
}

real get_x_divider_stamp_drawing_Pixel() {
    return LINEAR_REMAP(other.x_divider_stamp_drawing_OpenGL, -1.0f, 1.0f, 0.0f, window_get_size_Pixel().x);
}


vec2 magic_snap(vec2 before, bool calling_this_function_for_drawing_preview = false) {
    vec2 result = before;
    {
        if (
                click_mode_15_DEG_ELIGIBLE()
                && (two_click_command->awaiting_second_click)
                && (other.shift_held)) {
            vec2 a = two_click_command->first_click;
            vec2 b = before;
            vec2 r = b - a; 
            real norm_r = norm(r);
            real factor = 360 / 15 / TAU;
            real theta = roundf(ATAN2(r) * factor) / factor;
            result = a + norm_r * e_theta(theta);
        } else if (
                (state.click_mode == ClickMode::Box)
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
                        ArcEntity *arc = &entity->arc;
                        real squared_distance = squared_distance_point_dxf_arc_entity(before, arc);
                        if (squared_distance < min_squared_distance) {
                            min_squared_distance = squared_distance;
                            result = arc->center;
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
            } else if (state.click_modifier == ClickModifier::Perpendicular) { // layout also does a divide which can be added if wanted
                real min_squared_distance = HUGE_VAL;
                vec2 click_one = two_click_command->first_click;
                Entity* closest_entity = NULL;
                _for_each_entity_ {
                    real squared_distance = squared_distance_point_entity(before, entity);
                    if (squared_distance < min_squared_distance) {
                        min_squared_distance = squared_distance;
                        closest_entity = entity;
                    }
                }

                if (closest_entity == NULL) {
                } else if (closest_entity->type == EntityType::Line) {
                    vec2 a_to_b = closest_entity->line.end - closest_entity->line.start;
                    vec2 a_to_p = click_one - closest_entity->line.start;
                    real t = dot(a_to_p, a_to_b) / dot(a_to_b, a_to_b);
                    result = closest_entity->line.start + t * a_to_b; 
                } else if (closest_entity->type == EntityType::Arc) { // layout pretends the arc is a full circle for perp
                    vec2 normalized_in_direction = normalized(click_one - closest_entity->arc.center);
                    result = closest_entity->arc.center + closest_entity->arc.radius * normalized_in_direction;
                }
            } else if (state.click_modifier == ClickModifier::Quad) {
                real min_squared_distance = HUGE_VAL;

                // TODO: find_closest_entity( ,...)
                Entity *closest_entity = NULL;
                _for_each_entity_ {
                    if (entity->type == EntityType::Line) continue;
                    ASSERT(entity->type == EntityType::Arc);
                    ArcEntity *arc = &entity->arc;
                    real squared_distance = squared_distance_point_dxf_arc_entity(before, arc);
                    if (squared_distance < min_squared_distance) {
                        min_squared_distance = squared_distance;
                        closest_entity = entity;
                    }
                }
                if (closest_entity) {
                    ArcEntity *arc = &closest_entity->arc;
                    real angle; {
                        angle = LINEAR_REMAP(angle_from_0_TAU(arc->center, before), 0.0f, TAU, 0.0f, 4.0f);
                        angle = (ROUND(angle) / 4.0f) * TAU;
                    }
                    result = get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, angle);
                    // float theta = ATAN2(before - center);
                    // result = theta < PI * -3 / 4 ? V2(center.x + radius, center.y) :
                    //          theta < PI * -1 / 4 ? V2(center.x, center.y + radius) :
                    //          theta < PI *  1 / 4 ? V2(center.x - radius, center.y) :
                    //          theta < PI *  3 / 4 ? V2(center.x, center.y - radius) :
                    //          V2(center.x + radius, center.y);
                }
            }
        }
    }
    return result;
}

void init_camera_drawing() {
    *camera_drawing = make_Camera2D(100.0f, {}, { AVG(-1.0f, other.x_divider_drawing_mesh_OpenGL), 0.0f });
    if (drawing->entities.length) {
        bbox2 bbox = entities_get_bbox(&drawing->entities);
        real f = (get_x_divider_stamp_drawing_Pixel() / window_get_width_Pixel());
        vec2 L = (bbox.max - bbox.min);
        camera_drawing->ortho_screen_height_World = MAX((L.x / f) / window_get_aspect(), L.y);
        camera_drawing->ortho_screen_height_World += 64.0f * (camera_drawing->ortho_screen_height_World / window_get_height_Pixel());
        camera_drawing->pre_nudge_World = AVG(bbox.min, bbox.max);
    }
}
void init_camera_mesh() {
    *camera_mesh = make_OrbitCamera3D(
            200.0f,
            CAMERA_3D_PERSPECTIVE_ANGLE_OF_VIEW,
            { RAD(-44.0f), RAD(33.0f) },
            {},
            { AVG(other.x_divider_drawing_mesh_OpenGL, 1.0f), 0.0f }
            );
    // // TODO: rasterize the bounding box
    // mat4 PV = camera_get_PV(camera_mesh);
    // bbox2 B = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
    // B += _V2(transformPoint(PV, V3(mesh->bbox.min.x, mesh->bbox.min.y, mesh->bbox.min.z)));
    // B += _V2(transformPoint(PV, V3(mesh->bbox.max.x, mesh->bbox.min.y, mesh->bbox.min.z)));
    // B += _V2(transformPoint(PV, V3(mesh->bbox.min.x, mesh->bbox.max.y, mesh->bbox.min.z)));
    // B += _V2(transformPoint(PV, V3(mesh->bbox.max.x, mesh->bbox.max.y, mesh->bbox.min.z)));
    // B += _V2(transformPoint(PV, V3(mesh->bbox.min.x, mesh->bbox.min.y, mesh->bbox.max.z)));
    // B += _V2(transformPoint(PV, V3(mesh->bbox.max.x, mesh->bbox.min.y, mesh->bbox.max.z)));
    // B += _V2(transformPoint(PV, V3(mesh->bbox.min.x, mesh->bbox.max.y, mesh->bbox.max.z)));
    // B += _V2(transformPoint(PV, V3(mesh->bbox.max.x, mesh->bbox.max.y, mesh->bbox.max.z)));
    // real f = 1.0f - (get_x_divider_Pixel() / window_get_width_Pixel());
    // vec2 L_OpenGL = (B.max - B.min);
    // real fac = camera_mesh->persp_distance_to_origin_World * TAN(camera_mesh->angle_of_view);;
    // vec2 L = L_OpenGL * fac;
    // camera_mesh->persp_distance_to_origin_World = (0.5f * MAX((L.x / f) / window_get_aspect(), L.y)) / TAN(camera_mesh->angle_of_view);
    // camera_mesh->pre_nudge_World = fac * (V2(0.5f) + 0.5f * AVG(B.min, B.max));
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
