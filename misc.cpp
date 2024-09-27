#define _for_each_entity_ for (\
        Entity *entity = drawing->entities.array;\
        entity < &drawing->entities.array[drawing->entities.length];\
        ++entity)

#define _for_each_selected_entity_ _for_each_entity_ if (entity->is_selected) 

template <typename T> void JUICEIT_EASYTWEEN(T *a, T b, real multiplier) {
    real f = multiplier * 0.1f;
    if (IS_ZERO(multiplier)) f = 1.0f;
    if (!other.paused) *a += f * (b - *a);
}

real _JUICEIT_EASYTWEEN(real t) { return 0.287f * log(t) + 1.172f; }


real get_x_divider_drawing_mesh_Pixel() {
    return LINEAR_REMAP(other.x_divider_drawing_mesh_OpenGL, -1.0f, 1.0f, 0.0f, window_get_size_Pixel().x);
}

#define __snap_for__ _for_each_entity_ if (!( \
            1 \
            && (state.Draw_command.flags & EXCLUDE_SELECTED_FROM_SECOND_CLICK_SNAP) \
            && (two_click_command->awaiting_second_click) \
            && (entity->is_selected) \
            ))

MagicSnapResult magic_snap(vec2 before, bool calling_this_function_for_drawing_preview = false) {
    MagicSnapResult result = {};
    result.mouse_position = before;
    {
        if (1
                && (state.Draw_command.flags & SHIFT_15)
                && (two_click_command->awaiting_second_click)
                && (other.shift_held)) {
            vec2 a = two_click_command->first_click;
            vec2 b = before;
            vec2 r = b - a; 
            real norm_r = norm(r);
            real factor = 360 / 15 / TAU;
            real theta = roundf(ATAN2(r) * factor) / factor;
            result.mouse_position = a + norm_r * e_theta(theta);
        } else if (
                (state_Draw_command_is_(Box))
                && (two_click_command->awaiting_second_click)
                && (other.shift_held)) {
            // TODO (Felipe): snap square
            result.mouse_position = before;
        } else if (!calling_this_function_for_drawing_preview) { // NOTE: this else does, in fact, match LAYOUT's behavior
            DXFFindClosestEntityResult closest_entity_info = {};

            if (0
                    || state_Snap_command_is_(Center)
                    || state_Snap_command_is_(Quad)
                    //|| state_Snap_command_is_(Tangent)
               ) {
                real min_squared_distance = HUGE_VAL;
                Entity *temp_entity = NULL;
                __snap_for__ {
                    if (entity->type == EntityType::Line) {
                        continue;
                    } else if (entity->type == EntityType::Arc) {
                        ArcEntity *arc = &entity->arc;
                        real squared_distance = squared_distance_point_dxf_arc_entity(before, arc);
                        if (squared_distance < min_squared_distance) {
                            min_squared_distance = squared_distance;
                            temp_entity = entity;
                        }
                    } else { ASSERT(entity->type == EntityType::Circle); 
                        CircleEntity *circle = &entity->circle;
                        real squared_distance = squared_distance_point_dxf_circle_entity(before, circle);
                        if (squared_distance < min_squared_distance) {
                            min_squared_distance = squared_distance;
                            temp_entity = entity;
                        }
                    }
                }
                if (!ARE_EQUAL(min_squared_distance, HUGE_VAL)) {
                    closest_entity_info.success = true;
                    closest_entity_info.closest_entity = temp_entity;
                } else {
                    closest_entity_info.success = false;
                }
            } else { // NOTE (Jim): ? else if (!state_Snap_command_is_(None))
                closest_entity_info = dxf_find_closest_entity(&drawing->entities, before);
            }

            if (closest_entity_info.success) {
                Entity *closest_entity = closest_entity_info.closest_entity;
                result.entity_index_snapped_to = uint(closest_entity_info.closest_entity - drawing->entities.array); //TODO TODO TODO
                if (state_Snap_command_is_(Center)) {
                    result.snapped = true;
                    if (closest_entity->type == EntityType::Arc) {
                        ArcEntity *arc = &closest_entity->arc;
                        result.mouse_position = arc->center;
                    } else { ASSERT(closest_entity->type == EntityType::Circle);
                        CircleEntity *circle = &closest_entity->circle;
                        result.mouse_position = circle->center;
                    }
                } else if (state_Snap_command_is_(Quad)) {
                    result.snapped = true;
                    vec2 center;
                    real radius;
                    real angle;
                    {
                        if (closest_entity->type == EntityType::Arc) {
                            ArcEntity *arc = &closest_entity->arc;
                            center = arc->center;
                            radius = arc->radius;
                        } else { ASSERT(closest_entity->type == EntityType::Circle);
                            CircleEntity *circle = &closest_entity->circle;
                            center = circle->center;
                            radius = circle->radius;
                        }
                        { // angle
                            angle = LINEAR_REMAP(angle_from_0_TAU(center, before), 0.0f, TAU, 0.0f, 4.0f);
                            angle = (ROUND(angle) / 4.0f) * TAU;
                        }
                    }
                    result.mouse_position = get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, angle);
                } else if (state_Snap_command_is_(Middle)) {
                    ASSERT(closest_entity->type != EntityType::Circle); // TODO
                    result.mouse_position = entity_get_middle(closest_entity);
                    result.snapped = true;
                } else if (state_Snap_command_is_(End)) { // this one is a little custom
                    real min_squared_distance = HUGE_VAL;
                    __snap_for__ {
                        uint count = 0;
                        vec2 p[2] = {};
                        if (entity->type == EntityType::Circle) {
                            CircleEntity *circle = &entity->circle;
                            if (circle->has_pseudo_point) {
                                p[count++] = circle->pseudo_point;
                            }
                        } else {
                            entity_get_start_and_end_points(entity, &p[0], &p[1]);
                            count = 2;
                        }
                        for_(d, count) {
                            real squared_distance = squaredDistance(before, p[d]);
                            if (squared_distance < min_squared_distance) {
                                min_squared_distance = squared_distance;
                                result.mouse_position = p[d];
                                result.entity_index_snapped_to = uint(entity - drawing->entities.array);
                                result.snapped = true;
                            }
                        }
                    }
                } else if (state_Snap_command_is_(Intersect)) { // this one is a little custom
                                                                // TODO Circle
                    real min_squared_distance = HUGE_VAL;
                    Entity *temp_entity = NULL;
                    __snap_for__ {
                        real squared_distance = squared_distance_point_entity(before, entity);
                        if (squared_distance < min_squared_distance && entity != closest_entity) {
                            min_squared_distance = squared_distance;
                            temp_entity = entity;
                        }
                    }
                    if (temp_entity != NULL) {
                        ClosestIntersectionResult res = closest_intersection(closest_entity, temp_entity, before);
                        if (!res.no_possible_intersection) {
                            result.mouse_position = res.point;
                            result.entity_index_snapped_to = uint(closest_entity_info.closest_entity - drawing->entities.array); //TODO TODO TODO
                            result.entity_index_intersect = uint(temp_entity - drawing->entities.array);
                            result.snapped = true;
                            result.split_intersect = true;
                        }
                        // else messagef(pallete.orange, "no intersection found");
                    }
                    // else messagef(pallete.orange, "no intersection found");
                } else if (state_Snap_command_is_(Perp)) { // layout also does a divide which can be added if wanted
                                                           // TODO Circle
                    vec2 click_one = two_click_command->awaiting_second_click ? two_click_command->first_click : before;
                    if (closest_entity->type == EntityType::Line) {
                        vec2 a_to_b = closest_entity->line.end - closest_entity->line.start;
                        vec2 a_to_p = click_one - closest_entity->line.start;
                        real t = dot(a_to_p, a_to_b) / dot(a_to_b, a_to_b);
                        result.mouse_position = closest_entity->line.start + t * a_to_b; 
                        result.snapped = true;
                    } else if (closest_entity->type == EntityType::Arc) { // layout pretends the arc is a full circle for perp
                        vec2 normalized_in_direction = normalized(click_one - closest_entity->arc.center);
                        result.mouse_position = closest_entity->arc.center + closest_entity->arc.radius * normalized_in_direction;
                        result.snapped = true;
                    }
                } /*else if (state_Snap_command_is_(Tangent)) {
                // TODO TODO TODO
                vec2 mouse = before;

                if (two_click_command->awaiting_second_click) {
                mouse = two_click_command->first_click;
                }
                if (two_click_command->awaiting_second_click && two_click_command->tangent_first_click) {
                mouse = two_click_command->first_click;

                ArcEntity c2 = closest_entity->arc;
                ArcEntity c1 = two_click_command->entity_closest_to_first_click->arc;

                vec2 center_diff = c2.center - c1.center;
                real dist = distance(c1.center, c2.center);
                real angle = ATAN2(center_diff);

                real phi1 = acos((c1.radius - c2.radius) / dist);
                real phi2 = acos((c1.radius + c2.radius) / dist);

                real theta1a = angle + phi1;
                real theta1b = angle - phi1;
                real theta2a = angle + phi2;
                real theta2b = angle - phi2;

                vec2 p1a1 = c1.center + V2(c1.radius * COS(theta1a), c1.radius * SIN(theta1a));
                vec2 p1a2 = c1.center + V2(c1.radius * COS(theta1b), c1.radius * SIN(theta1b));
                if (distance(mouse, p1a1) > distance(mouse, p1a1)) {
                p1a1 = p1a2;
                theta1a = theta1b;
                }
                vec2 p1b = c2.center + V2(c2.radius * COS(theta1a), c2.radius * SIN(theta1a));

                vec2 p2a1 = c1.center + V2(c1.radius * COS(theta2a), c1.radius * SIN(theta2a));
                vec2 p2a2 = c1.center + V2(c1.radius * COS(theta2b), c1.radius * SIN(theta2b));
                if (distance(mouse, p2a1) > distance(mouse, p2a2)) {
                p2a1 = p2a2;
                theta2a = theta2b;
                }
                vec2 p2b = c2.center - V2(c2.radius * COS(theta2a), c2.radius * SIN(theta2a));

                if (distance(before, p1b) > distance(before, p2b)) {
                two_click_command->first_click = p2a1;
                result.mouse_position = p2b;
                } else {
                two_click_command->first_click = p1a1;
                result.mouse_position = p1b;
                }

                two_click_command->tangent_first_click = false;
                result.snapped = true;
                result.split_tangent_2 = true;
                result.entity_index_tangent_2 = uint(two_click_command->entity_closest_to_first_click - drawing->entities.array);

                } else if (two_click_command->awaiting_second_click) {
                vec2 center = closest_entity->arc.center;
                real radius = closest_entity->arc.radius;
                real d = distance(center, mouse);

                if (d > radius) {
                real t1 = ATAN2(mouse - center);
                real t2 = acos(radius / d);
                real theta1 = t1 + t2;
                real theta2 = t1 - t2;
                vec2 tan1 = { center.x + radius * COS(theta1), center.y + radius * SIN(theta1) };
                vec2 tan2 = { center.x + radius * COS(theta2), center.y + radius * SIN(theta2) };
                result.mouse_position = distance(before, tan1) < distance(before, tan2) ? tan1 : tan2;
                result.snapped = true;
                }
                } else {
                messagef(pallete.light_gray, "wowowwowowo");
                two_click_command->tangent_first_click = true; 
                two_click_command->entity_closest_to_first_click = closest_entity;
                messagef(pallete.red, "%f %f", closest_entity->arc.center.x, closest_entity->arc.center.y);
            }
            }*/
            }
        }
    }


    return result;
}

void init_camera_drawing() {
    *camera_drawing = make_Camera2D(100.0f, {}, { AVG(-1.0f, other.x_divider_drawing_mesh_OpenGL), 0.0f });
    if (drawing->entities.length) {
        bbox2 bbox = entities_get_bbox(&drawing->entities);
        real eps = 150.0f;
        real f = ((get_x_divider_drawing_mesh_Pixel() - eps) / window_get_width_Pixel());
        vec2 L = (bbox.max - bbox.min);
        camera_drawing->ortho_screen_height_World = MAX((L.x / f) / window_get_aspect(), L.y);
        camera_drawing->ortho_screen_height_World += 96.0f * (camera_drawing->ortho_screen_height_World / window_get_height_Pixel());
        camera_drawing->pre_nudge_World = V2(-(eps / 2) * (camera_drawing->ortho_screen_height_World / window_get_height_Pixel()), 0.0f) + AVG(bbox.min, bbox.max);
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




