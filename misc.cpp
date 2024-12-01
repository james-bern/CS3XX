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

template <typename T> void FINITE_EASYTWEEN(T *a, T b, T threshold, real multiplier = 1.0f) {
    real f = multiplier * 0.1f;
    if (IS_ZERO(multiplier)) f = 1.0f;
    if (!other.paused) {
        T signed_delta = b - *a;
        if (-threshold < signed_delta && signed_delta < threshold) *a = b;
        else *a += f * signed_delta;
    } 
}

real _JUICEIT_EASYTWEEN(real t) { return 0.287f * log(t) + 1.172f; }


real get_x_divider_drawing_mesh_Pixel() {
    return LINEAR_REMAP(other.x_divider_drawing_mesh_OpenGL, -1.0f, 1.0f, 0.0f, window_get_size_Pixel().x);
}

// NOTE: this is the function formerly known as magic_snap
TransformMouseDrawingPositionResult transform_mouse_drawing_position(
        vec2 unsnapped_position,
        bool shift_held,
        bool dont_actually_snap_just_potentially_do_the_15_deg_stuff
        ) {
    vec2 before = unsnapped_position;
    TransformMouseDrawingPositionResult result = {};
    result.mouse_position = before;
    {
        bool do_the_15_deg_stuff = 1
            && (shift_held)
            && (state.Draw_command.flags & SHIFT_15)
            && (two_click_command->awaiting_second_click);

        if (do_the_15_deg_stuff) {
            vec2 a = two_click_command->first_click;
            vec2 b = before;
            vec2 r = b - a; 
            real norm_r = norm(r);
            real factor = 360 / 15 / TAU;
            real theta = roundf(ATAN2(r) * factor) / factor;
            result.mouse_position = a + norm_r * e_theta(theta);
        } else if (!dont_actually_snap_just_potentially_do_the_15_deg_stuff) { // NOTE: this else does, in fact, match LAYOUT's behavior
            DXFFindClosestEntityResult closest_entity_info = {};

            // TODO: need to filter End and Middle as well to ignore circles
            // (this all needs to be cleaned up)
            if (0
                    || state_Snap_command_is_(Center)
                    || state_Snap_command_is_(Quad)
                    //|| state_Snap_command_is_(Tangent)
               ) {
                real min_squared_distance = HUGE_VAL;
                Entity *temp_entity = NULL;
                _for_each_entity_ {
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
                    if (closest_entity->type != EntityType::Circle) {
                        result.mouse_position = entity_get_middle(closest_entity);
                        result.snapped = true;
                    }
                } else if (state_Snap_command_is_(End)) { // this one is a little custom
                    real min_squared_distance = HUGE_VAL;
                    _for_each_entity_ {
                        uint count = 0;
                        vec2 p[2] = {};
                        if (entity->type == EntityType::Circle) {
                            CircleEntity *circle = &entity->circle;
                            if (circle->has_pseudo_point) {
                                p[count++] = circle->get_pseudo_point();
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
                    _for_each_entity_ {
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
                } else if (state_Snap_command_is_(Perp)) { 
                    vec2 click_one = two_click_command->awaiting_second_click ? two_click_command->first_click : before;
                    if (closest_entity->type == EntityType::Line) {
                        vec2 a_to_b = closest_entity->line.end - closest_entity->line.start;
                        vec2 a_to_p = click_one - closest_entity->line.start;
                        real t = dot(a_to_p, a_to_b) / dot(a_to_b, a_to_b);
                        result.mouse_position = closest_entity->line.start + t * a_to_b; 
                        result.snapped = true;
                    } else if (closest_entity->type == EntityType::Arc) { // layout pretends the arc is a full circle for perp
                        vec2 normalized_in_direction = normalized(click_one - closest_entity->arc.center);
                        vec2 perp_one = closest_entity->arc.center + closest_entity->arc.radius * normalized_in_direction;
                        vec2 perp_two = closest_entity->arc.center - closest_entity->arc.radius * normalized_in_direction;

                        result.mouse_position = distance(perp_one, before) < distance(perp_two, before) ? perp_one : perp_two;
                        result.snapped = true;
                    } else { ASSERT(closest_entity->type == EntityType::Circle);
                        vec2 normalized_in_direction = normalized(click_one - closest_entity->circle.center);
                        vec2 perp_one = closest_entity->circle.center + closest_entity->circle.radius * normalized_in_direction;
                        vec2 perp_two = closest_entity->circle.center - closest_entity->circle.radius * normalized_in_direction;

                        result.mouse_position = distance(perp_one, before) < distance(perp_two, before) ? perp_one : perp_two;
                        result.snapped = true;
                    }
                }
            }
        }
    }
    return result;
}

MagicSnapResult3D magic_snap_raycast(vec3 origin, vec3 dir) {
    MagicSnapResult3D result{};
    WorkMesh* mesh = &meshes->work;

    int index_of_first_triangle_hit_by_ray = -1;
    vec3 exact_hit_pos;
    {
        real min_distance = HUGE_VAL;
        for_(i, mesh->num_triangles) {
            vec3 p[3]; {
                for_(j, 3) p[j] = mesh->vertex_positions[mesh->triangle_tuples[i][j]];
            }
            RayTriangleIntersectionResult ray_triangle_intersection_result = ray_triangle_intersection(origin, dir, p[0], p[1], p[2]);
            if (ray_triangle_intersection_result.hit) {
                if (ray_triangle_intersection_result.distance < min_distance) {
                    min_distance = ray_triangle_intersection_result.distance;
                    exact_hit_pos = ray_triangle_intersection_result.pos;
                    index_of_first_triangle_hit_by_ray = i; // FORNOW
                }
            }
        }
    }

    if (index_of_first_triangle_hit_by_ray != -1) { // something hit
        result.triangle_index = index_of_first_triangle_hit_by_ray;
        result.hit_mesh = true;
        result.mouse_position = exact_hit_pos;

        if (!state_Snap_command_is_(None)) { // TODO: Change to 3D specific snap type?
            real min_distance = HUGE_VAL;
            for_(i, 3) {
                vec3 vertex_pos = mesh->vertex_positions[mesh->triangle_tuples[index_of_first_triangle_hit_by_ray][i]];
                real dist = squaredDistance(exact_hit_pos, vertex_pos);
                if (dist < min_distance) {
                    min_distance = dist;
                    result.mouse_position = vertex_pos;
                    result.snapped = true;
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
        real eps = 0.0f;
        real f = ((get_x_divider_drawing_mesh_Pixel() - eps) / window_get_width_Pixel());
        vec2 L = (bbox.max - bbox.min);
        camera_drawing->ortho_screen_height_World = MAX((L.x / f) / window_get_aspect(), L.y);
        camera_drawing->ortho_screen_height_World += 400.0f * (camera_drawing->ortho_screen_height_World / window_get_height_Pixel());
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




