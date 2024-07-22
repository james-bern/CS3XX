// // TODO: (Jim) stuff for alpha
// TODO: fix in/out relationship (right now they just seem to add)
// TODO: tubes
// TODO: manifold_wrapper
// XXX: fix origin axis relationship with revolve
// XXX: - tubes
// XXX: - manifold_wrapper
// TODO: draw axis on RHS when revolving
// TODO: 3D-picking is broken for non xyz planes
// TODO: revisit extruded cut on the botton of box with name (why did the students need to flip their names)

mat4 get_M_3D_from_2D() {
    vec3 up = { 0.0f, 1.0f, 0.0f };
    real dot_product = dot(feature_plane->normal, up);
    vec3 y = (ARE_EQUAL(ABS(dot_product), 1.0f)) ? V3(0.0f,  0.0f, -1.0f * SGN(dot_product)) : up;
    vec3 x = normalized(cross(y, feature_plane->normal));
    vec3 z = cross(x, y);

    // FORNOW
    if (ARE_EQUAL(ABS(dot_product), 1.0f) && SGN(dot_product) < 0.0f) {
        y *= -1;
    }

    return M4_xyzo(x, y, z, (feature_plane->signed_distance_to_world_origin) * feature_plane->normal);
}

bbox2 mesh_draw(mat4 P_3D, mat4 V_3D, mat4 M_3D) {
    bbox2 face_selection_bbox = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
    mat4 inv_M_3D_from_2D = inverse(get_M_3D_from_2D());

    mat4 PVM_3D = P_3D * V_3D * M_3D;

    if (!other.show_details) {
        if (mesh->cosmetic_edges) {
            eso_begin(PVM_3D, SOUP_LINES); 
            // eso_color(CLAMPED_LERP(2 * time_since_successful_feature, omax.white, omax.black));
            eso_color(0,0,0);
            eso_size(1.0f);
            for_(i, mesh->num_cosmetic_edges) {
                for_(d, 2) {
                    eso_vertex(mesh->vertex_positions[mesh->cosmetic_edges[i][d]]);
                }
            }
            eso_end();
        }
    }

    for_(pass, 2) {
        eso_begin(PVM_3D, (!other.show_details) ? SOUP_TRIANGLES : SOUP_TRI_MESH);
        eso_size(0.5f);

        mat3 inv_transpose_V_3D = inverse(transpose(_M3(V_3D)));


        for_(i, mesh->num_triangles) {
            vec3 n = mesh->triangle_normals[i];
            vec3 p[3];
            real x_n;
            {
                for_(j, 3) p[j] = mesh->vertex_positions[mesh->triangle_indices[i][j]];
                x_n = dot(n, p[0]);
            }
            vec3 color; 
            real alpha;
            {
                vec3 n_Camera = inv_transpose_V_3D * n;
                vec3 color_n = V3(V2(0.5f) + 0.5f * _V2(n_Camera), 1.0f);
                if ((true || feature_plane->is_active) && (dot(n, feature_plane->normal) > 0.99f) && (ABS(x_n - feature_plane->signed_distance_to_world_origin) < 0.01f)) {
                    if (pass == 0) continue;

                    // TODO:
                    if (feature_plane->is_active) {
                        color = CLAMPED_LERP(_JUICEIT_EASYTWEEN(other.time_since_plane_selected), omax.white, V3(0.65f, 0.67f, 0.10f));// CLAMPED_LERP(2.0f * time_since_plane_selected - 0.5f, omax.yellow, V3(0.85f, 0.87f, 0.30f));
                    } else color = color_n;

                    // if (2.0f * time_since_plane_selected < 0.3f) color = omax.white; // FORNOW

                    alpha = CLAMPED_LERP(_JUICEIT_EASYTWEEN(other.time_since_going_inside), 1.0f, 0.7f);

                    face_selection_bbox += _V2(transformPoint(inv_M_3D_from_2D, p[0]));
                    face_selection_bbox += _V2(transformPoint(inv_M_3D_from_2D, p[1]));
                    face_selection_bbox += _V2(transformPoint(inv_M_3D_from_2D, p[2]));


                } else {
                    if (pass == 1) continue;
                    color = color_n;
                    alpha = 1.0f;
                }
            }
            real mask = CLAMP(1.2f * other.time_since_successful_feature, 0.0f, 2.0f);
            // color = CLAMPED_LINEAR_REMAP(time_since_successful_feature, -0.5f, 0.5f, omax.white, color);
            eso_color(color, alpha);
            for_(d, 3) {
                eso_color(CLAMPED_LERP(mask + SIN(CLAMPED_INVERSE_LERP(p[d].y, mesh->bbox.max.y, mesh->bbox.min.y) + 0.5f * other.time_since_successful_feature), omax.white, color), alpha);

                eso_vertex(p[d]);
            }
        }
        eso_end();
    }

    return face_selection_bbox;
}

void conversation_draw() {
    mat4 P_2D = camera_drawing->get_P();
    mat4 V_2D = camera_drawing->get_V();
    mat4 PV_2D = P_2D * V_2D;
    mat4 inv_PV_2D = inverse(PV_2D);
    vec2 mouse_World_2D = transformPoint(inv_PV_2D, other.mouse_OpenGL);
    mat4 M_3D_from_2D = get_M_3D_from_2D();

    bool extruding = ((state.enter_mode == EnterMode::ExtrudeAdd) || (state.enter_mode == EnterMode::ExtrudeCut));
    bool revolving = ((state.enter_mode == EnterMode::RevolveAdd) || (state.enter_mode == EnterMode::RevolveCut));
    bool adding     = ((state.enter_mode == EnterMode::ExtrudeAdd) || (state.enter_mode == EnterMode::RevolveAdd));
    bool cutting     = ((state.enter_mode == EnterMode::ExtrudeCut) || (state.enter_mode == EnterMode::RevolveCut));

    { // preview->extrude_in_length
        real target = (adding) ? popup->extrude_add_in_length : popup->extrude_cut_in_length;
        JUICEIT_EASYTWEEN(&preview->extrude_in_length, target);
    }
    { // preview->extrude_out_length
        real target = (adding) ? popup->extrude_add_out_length : popup->extrude_cut_out_length;
        JUICEIT_EASYTWEEN(&preview->extrude_out_length, target);
    }
    { // preview->revolve_in_angle
        real target = (adding) ? popup->revolve_add_in_angle : popup->revolve_cut_in_angle;
        JUICEIT_EASYTWEEN(&preview->revolve_in_angle, target);
    }
    { // preview->revolve_out_angle
        real target = (adding) ? popup->revolve_add_out_angle : popup->revolve_cut_out_angle;
        JUICEIT_EASYTWEEN(&preview->revolve_out_angle, target);
    }

    // TODO
    { // preview_feature_plane_offset
        real target = (state.enter_mode == EnterMode::NudgePlane) ? popup->feature_plane_nudge : 0.0f;
        JUICEIT_EASYTWEEN(&preview->feature_plane_offset, target);
    }

    // preview
    vec2 mouse = magic_snap(mouse_World_2D, true).mouse_position;

    {
        vec2 target_preview_mouse = mouse;
        JUICEIT_EASYTWEEN(&preview->mouse, target_preview_mouse);
    }

    vec2 target_preview_drawing_origin = (state.click_mode != ClickMode::Origin) ? drawing->origin : mouse;
    {
        JUICEIT_EASYTWEEN(&preview->drawing_origin, target_preview_drawing_origin);
    }

    // TODO: lerp
    vec2 preview_dxf_axis_base_point;
    real preview_dxf_axis_angle_from_y;
    {
        if (state.click_mode != ClickMode::Axis) {
            preview_dxf_axis_base_point = drawing->axis_base_point;
            preview_dxf_axis_angle_from_y = drawing->axis_angle_from_y;
        } else if (!two_click_command->awaiting_second_click) {
            preview_dxf_axis_base_point = mouse;
            preview_dxf_axis_angle_from_y = drawing->axis_angle_from_y;
        } else {
            preview_dxf_axis_base_point = two_click_command->first_click;
            preview_dxf_axis_angle_from_y = ATAN2(mouse - preview_dxf_axis_base_point) - PI / 2;
        }
    }


    mat4 P_3D = camera_mesh->get_P();
    mat4 V_3D = camera_mesh->get_V();
    mat4 PV_3D = P_3D * V_3D;

    uint window_width, window_height; {
        vec2 _window_size = window_get_size_Pixel();
        window_width = uint(_window_size.x);
        window_height = uint(_window_size.y);
    }

    { // drawing mesh panes
        bool dragging = (other.mouse_left_drag_pane == Pane::Separator);
        bool hovering = ((other.mouse_left_drag_pane == Pane::None) && (other.hot_pane == Pane::Separator));
        eso_begin(M4_Identity(), SOUP_LINES);
        // eso_overlay(true);
        eso_color(
                dragging ? omax.light_gray
                : hovering ? omax.white
                : omax.gray);
        eso_vertex(other.x_divider_drawing_mesh_OpenGL,  1.0f);
        eso_vertex(other.x_divider_drawing_mesh_OpenGL, -1.0f);
        eso_end();
    }

    real x_divider_drawing_mesh_Pixel = get_x_divider_drawing_mesh_Pixel();

    bool moving_selected_entities = (
            (two_click_command->awaiting_second_click)
            && (0 
                || (state.click_mode == ClickMode::Move)
                || (state.click_mode == ClickMode::Rotate)
                || (state.click_mode == ClickMode::LinearCopy))
            ); // TODO: loft up

    { // draw 2D draw 2d draw
        vec2 *first_click = &two_click_command->first_click;
        vec2 click_vector = (mouse - *first_click);
        real click_theta = ATAN2(click_vector);

        glEnable(GL_SCISSOR_TEST);
        gl_scissor_Pixel(0, 0, x_divider_drawing_mesh_Pixel, window_height);

        {
            #if 0
            // TODO: section view
            mesh_draw(P_2D, V_2D, inverse(get_M_3D_from_2D()));
            #endif
        }

        {
            if (!other.hide_grid) { // grid 2D grid 2d grid // jim wtf are these supposed to mean
                eso_begin(PV_2D, SOUP_LINES);
                eso_color(omax.dark_gray);
                for (uint i = 0; i <= uint(GRID_SIDE_LENGTH / GRID_SPACING); ++i) {
                    real tmp = i * GRID_SPACING;
                    eso_vertex(tmp, 0.0f);
                    eso_vertex(tmp, GRID_SIDE_LENGTH);
                    eso_vertex(0.0f, tmp);
                    eso_vertex(GRID_SIDE_LENGTH, tmp);
                }
                eso_end();
                eso_begin(PV_2D, SOUP_LINE_LOOP);
                eso_color(omax.dark_gray);
                eso_vertex(0.0f, 0.0f);
                eso_vertex(0.0f, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, 0.0f);
                eso_end();
            }
            if (1) { // axes 2D axes 2d axes axis 2D axis 2d axes crosshairs cross hairs origin 2d origin 2D origin
                real funky_OpenGL_factor = other.camera_drawing.ortho_screen_height_World / 120.0f;
                real LL = 1000 * funky_OpenGL_factor;

                eso_begin(PV_2D, SOUP_LINES); {
                    // axis
                    eso_stipple(true);
                    eso_color(omax.dark_gray);
                    if (state.click_mode == ClickMode::Axis) {
                        eso_color(omax.cyan);
                    } else if (state.enter_mode == EnterMode::RevolveAdd) {
                        eso_color(AVG(omax.dark_gray, omax.cyan));
                    } else if (state.enter_mode == EnterMode::RevolveCut) {
                        eso_color(AVG(omax.dark_gray, omax.cyan));
                    } else {
                    }
                    vec2 v = LL * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                    eso_vertex(preview_dxf_axis_base_point + v);
                    eso_vertex(preview_dxf_axis_base_point - v);
                } eso_end();
                eso_begin(PV_2D, SOUP_LINES); {
                    // origin
                    eso_color(omax.white);
                    real r = funky_OpenGL_factor;
                    eso_vertex(target_preview_drawing_origin - V2(r, 0));
                    eso_vertex(target_preview_drawing_origin + V2(r, 0));
                    eso_vertex(target_preview_drawing_origin - V2(0, r));
                    eso_vertex(target_preview_drawing_origin + V2(0, r));
                } eso_end();
            }

            { // entities
                bool moving = (two_click_command->awaiting_second_click) && (state.click_mode == ClickMode::Move);
                bool linear_copying = (two_click_command->awaiting_second_click) && (state.click_mode == ClickMode::LinearCopy);
                bool rotating = (two_click_command->awaiting_second_click) && (state.click_mode == ClickMode::Rotate);
                bool draw_annotation_line = (moving || rotating || linear_copying);



                eso_begin(PV_2D, SOUP_LINES); {
                    if (draw_annotation_line) {
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(mouse);
                        eso_vertex(*first_click);
                    }

                    // entities 2D entities 2d entities
                    // drawing 2D drawing 2d drawing
                    _for_each_entity_ {
                        if (entity->is_selected && (rotating || moving)) continue;
                        eso_color(entity->preview_color);
                        eso_size(1.5f);
                        eso_entity__SOUP_LINES(entity);
                    }
                } eso_end();

                if (draw_annotation_line) {
                    mat4 M; {
                        if (moving || linear_copying) {
                            M = M4_Translation(click_vector);
                        } else { ASSERT(rotating);
                            M = M4_Translation(*first_click) * M4_RotationAboutZAxis(click_theta) * M4_Translation(-*first_click);
                        }
                    }
                    eso_begin(PV_2D * M, SOUP_LINES);
                    eso_color(get_color(ColorCode::Emphasis));
                    _for_each_selected_entity_ eso_entity__SOUP_LINES(entity);
                    eso_end();
                }

                if (other.show_details) {
                    eso_begin(PV_2D, SOUP_POINTS);
                    eso_color(omax.white);
                    eso_size(3.0f);
                    _for_each_entity_ {
                        if (entity->is_selected && (rotating || moving)) continue;
                        eso_vertex(entity_get_start_point(entity));
                        eso_vertex(entity_get_end_point(entity));
                    }
                    eso_end();
                }
            }


            if (two_click_command->awaiting_second_click) {
                if (
                        0
                        || (state.click_modifier == ClickModifier::Window)
                        || (state.click_mode == ClickMode::Box)
                   ) {
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(basic.cyan);
                    eso_vertex(first_click->x, first_click->y);
                    eso_vertex(mouse.x, first_click->y);
                    eso_vertex(mouse.x, mouse.y);
                    eso_vertex(first_click->x, mouse.y);
                    eso_end();
                }
                if (state.click_mode == ClickMode::CenteredBox) {                
                    vec2 one_corner = mouse;
                    vec2 center = *first_click;
                    real other_y = 2 * center.y - one_corner.y;
                    real other_x = 2 * center.x - one_corner.x;
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(basic.cyan);
                    eso_vertex(one_corner);
                    eso_vertex(V2(one_corner.x, other_y));
                    eso_vertex(V2(other_x, other_y));
                    eso_vertex(V2(other_x, one_corner.y));
                    eso_end();
                }
                if (state.click_mode == ClickMode::Measure) {
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(basic.cyan);
                    eso_vertex(two_click_command->first_click);
                    eso_vertex(mouse);
                    eso_end();
                }
                if (state.click_mode == ClickMode::MirrorLine) {
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(basic.cyan);
                    eso_vertex(two_click_command->first_click);
                    eso_vertex(mouse);
                    eso_end();
                }
                if (state.click_mode == ClickMode::Line) {
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(basic.cyan);
                    eso_vertex(two_click_command->first_click);
                    eso_vertex(mouse);
                    eso_end();
                }
                if (state.click_mode == ClickMode::Rotate) {
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(basic.cyan);
                    eso_vertex(two_click_command->first_click);
                    eso_vertex(mouse);
                    eso_end();
                }
                if (state.click_mode == ClickMode::Circle) {
                    vec2 center = two_click_command->first_click;
                    vec2 point = mouse;
                    real radius = distance(center, point);
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(basic.cyan);
                    for_(i, NUM_SEGMENTS_PER_CIRCLE) {
                        real theta = (real(i) / NUM_SEGMENTS_PER_CIRCLE) * TAU;
                        eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta));
                    }
                    eso_end();
                }
                if (state.click_mode == ClickMode::TwoEdgeCircle) {
                    vec2 edge_one = two_click_command->first_click;
                    vec2 edge_two = mouse;
                    vec2 center = (edge_one + edge_two) / 2;
                    real radius = norm(edge_one - center);
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(basic.cyan);
                    for_(i, NUM_SEGMENTS_PER_CIRCLE) {
                        real theta = (real(i) / NUM_SEGMENTS_PER_CIRCLE) * TAU;
                        eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta));
                    }
                    eso_end();
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_stipple(true);
                    eso_color(basic.cyan);
                    eso_vertex(edge_one);
                    eso_vertex(edge_two);
                    eso_end();
                }
                if (state.click_mode == ClickMode::TwoClickDivide) {
                    if (two_click_command->awaiting_second_click) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(basic.cyan);
                        eso_entity__SOUP_LINES(two_click_command->entity_closest_to_first_click);
                        eso_end();
                    }
                }
                if (state.click_mode == ClickMode::Polygon) {
                    uint polygon_num_sides = MAX(3U, popup->polygon_num_sides);
                    real delta_theta = TAU / polygon_num_sides;
                    vec2 center = two_click_command->first_click;
                    vec2 vertex_0 = mouse;
                    real radius = distance(center, vertex_0);
                    real theta_0 = ATAN2(vertex_0 - center);
                    {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_stipple(true);
                        eso_color(basic.cyan);
                        eso_vertex(center);
                        eso_vertex(vertex_0);
                        eso_end();
                    }
                    {
                        eso_begin(PV_2D, SOUP_LINE_LOOP);
                        eso_color(basic.cyan);
                        for_(i, polygon_num_sides) {
                            real theta_i = theta_0 + (i * delta_theta);
                            real theta_ip1 = theta_i + delta_theta;
                            eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta_i));
                            eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta_ip1));
                        }
                        eso_end();
                    }
                }
                if (state.click_mode == ClickMode::Fillet) {
                    if (two_click_command->awaiting_second_click) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(basic.cyan);
                        eso_entity__SOUP_LINES(two_click_command->entity_closest_to_first_click);
                        eso_end();
                    }
                }
                if (state.click_mode == ClickMode::DogEar) {
                    if (two_click_command->awaiting_second_click) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(basic.cyan);
                        eso_entity__SOUP_LINES(two_click_command->entity_closest_to_first_click);
                        eso_end();
                    }
                }
            }
        }
        glDisable(GL_SCISSOR_TEST);
    }

    { // 3D draw 3D 3d draw 3d
        {
            glEnable(GL_SCISSOR_TEST);
            gl_scissor_Pixel(x_divider_drawing_mesh_Pixel, 0, window_width - x_divider_drawing_mesh_Pixel, window_height);
        }


        mat4 inv_T_o = M4_Translation(-preview->drawing_origin);
        if (feature_plane->is_active) { // selection 2d selection 2D selection tube tubes slice slices stack stacks wire wireframe wires frame (FORNOW: ew)
            ;
            // FORNOW
            bool moving_stuff = ((state.click_mode == ClickMode::Origin) || (state.enter_mode == EnterMode::NudgePlane));
            vec3 target_preview_tubes_color = (0) ? V3(0)
                : (moving_selected_entities) ? get_color(ColorCode::Emphasis)
                : (adding) ? get_color(ColorCode::Traverse)
                : (cutting) ? get_color(ColorCode::Quality1)
                : (moving_stuff) ? get_color(ColorCode::Emphasis)
                : get_color(ColorCode::Selection);
            JUICEIT_EASYTWEEN(&preview->tubes_color, target_preview_tubes_color);

            uint NUM_TUBE_STACKS_INCLUSIVE;
            mat4 M;
            mat4 M_incr;
            {
                // mat4 T_o = M4_Translation(preview->drawing_origin);
                if (extruding) {
                    real a = -preview->extrude_in_length;
                    real L = preview->extrude_out_length + preview->extrude_in_length;
                    NUM_TUBE_STACKS_INCLUSIVE = MIN(64U, uint(ROUND(L / 2.5f)) + 2);
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0.0f, 0.0f, a + Z_FIGHT_EPS);
                    M_incr = M4_Translation(0.0f, 0.0f, L / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                } else if (revolving) {
                    real a = -RAD(preview->revolve_out_angle);
                    real b = RAD(preview->revolve_in_angle);
                    real L = b - a;
                    NUM_TUBE_STACKS_INCLUSIVE = MIN(64U, uint(ROUND(L / .1)) + 2);
                    vec3 axis = V3(e_theta(PI / 2 + preview_dxf_axis_angle_from_y), 0.0f);
                    mat4 R_0 = M4_RotationAbout(axis, a);
                    mat4 R_inc = M4_RotationAbout(axis, L / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                    mat4 T_a = M4_Translation(V3(preview_dxf_axis_base_point, 0.0f));
                    mat4 inv_T_a = inverse(T_a);
                    // M_incr = T_o * T_a * R_a * inv_T_a * inv_T_o;
                    M_incr = T_a * R_inc * inv_T_a;
                    M = M_3D_from_2D * inv_T_o * T_a * R_0 * inv_T_a;
                } else if (state.click_mode == ClickMode::Origin) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0, 0, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else if (state.enter_mode == EnterMode::NudgePlane) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0.0f, 0.0f, preview->feature_plane_offset + Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else { // default
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0, 0, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                }

                mat4 T_Move;
                if (moving_selected_entities) {
                    T_Move = M4_Translation(preview->mouse - two_click_command->first_click);
                } else {
                    T_Move = M4_Identity();
                }
                for_(tube_stack_index, NUM_TUBE_STACKS_INCLUSIVE) {
                    eso_begin(PV_3D * M * T_Move, SOUP_LINES); {
                        _for_each_selected_entity_ {
                            real alpha;
                            vec3 color;
                            // if (entity->is_selected) {
                            alpha = CLAMP(-0.2f + 3.0f * MIN(entity->time_since_is_selected_changed, other.time_since_plane_selected), 0.0f, 1.0f);
                            color = CLAMPED_LERP(-0.5f + SQRT(2.0f * entity->time_since_is_selected_changed), omax.white, preview->tubes_color);
                            // } else {
                            //     alpha = CLAMPED_LERP(5.0f * entity->time_since_is_selected_changed, 1.0f, 0.0f);
                            //     color = get_color(color);
                            // }
                            eso_color(color, alpha);
                            eso_entity__SOUP_LINES(entity);
                        }
                    } eso_end();
                    M *= M_incr;
                }
            }
        }

        if (feature_plane->is_active) { // axes 3D axes 3d axes axis 3D axis 3d axis
            real r = other.camera_mesh.ortho_screen_height_World / 120.0f;
            eso_begin(PV_3D * M_3D_from_2D * M4_Translation(0.0f, 0.0f, Z_FIGHT_EPS), SOUP_LINES);
            eso_color(omax.white);
            eso_vertex(-r, 0.0f);
            eso_vertex( r, 0.0f);
            eso_vertex(0.0f, -r);
            eso_vertex(0.0f,  r);
            if (revolving) {
                // TODO: clip this to the feature_plane
                real LL = 100.0f;
                vec2 v = LL * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                vec2 a = preview_dxf_axis_base_point + v;
                vec2 b = preview_dxf_axis_base_point - v;
                eso_color(omax.cyan);
                eso_vertex(-preview->drawing_origin + a);
                eso_vertex(-preview->drawing_origin + b); // FORNOW
            }
            eso_end();
        }

        if (!other.hide_grid) { // grid 3D grid 3d grid
            for_(k, 6) {
                real r = 0.5f * GRID_SIDE_LENGTH;
                mat4 M0 = M4_Translation(-r, -r, r);
                mat4 M1; {
                    M1 = {}; // FORNOW compiler warning
                    if (k == 0) M1 = M4_Identity();
                    if (k == 1) M1 = M4_RotationAboutYAxis(PI);
                    if (k == 2) M1 = M4_RotationAboutXAxis( PI / 2);
                    if (k == 3) M1 = M4_RotationAboutXAxis(-PI / 2);
                    if (k == 4) M1 = M4_RotationAboutYAxis( PI / 2);
                    if (k == 5) M1 = M4_RotationAboutYAxis(-PI / 2);
                }
                mat4 PVM1 = PV_3D * M1;
                { // backface culling (check sign of rasterized triangle)
                    vec2 a = _V2(transformPoint(PVM1, V3(0.0f, 0.0f, r)));
                    vec2 b = _V2(transformPoint(PVM1, V3(1.0f, 0.0f, r)));
                    vec2 c = _V2(transformPoint(PVM1, V3(0.0f, 1.0f, r)));
                    if (cross(b - a , c - a) > 0.0f) continue;
                }
                mat4 transform = PVM1 * M0;
                eso_begin(transform, SOUP_LINES);
                eso_color(omax.dark_gray);
                eso_size(1.0f);
                for (uint i = 0; i <= uint(GRID_SIDE_LENGTH / GRID_SPACING); ++i) {
                    real tmp = i * GRID_SPACING;
                    eso_vertex(tmp, 0.0f);
                    eso_vertex(tmp, GRID_SIDE_LENGTH);
                    eso_vertex(0.0f, tmp);
                    eso_vertex(GRID_SIDE_LENGTH, tmp);
                }
                eso_end();
                eso_begin(transform, SOUP_LINE_LOOP);
                eso_color(omax.dark_gray);
                eso_vertex(0.0f, 0.0f);
                eso_vertex(0.0f, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, 0.0f);
                eso_end();
            }
        }

        { // feature plane feature-plane feature_plane // floating sketch plane; selection plane NOTE: transparent
            {
                bbox2 face_selection_bbox; {
                    face_selection_bbox = mesh_draw(P_3D, V_3D, M4_Identity());
                }
                bbox2 dxf_selection_bbox; {
                    dxf_selection_bbox = entities_get_bbox(&drawing->entities, true);

                    // TODO: this should incorporate a preview of the fact that some entities are moving
                    if (moving_selected_entities) {
                        vec2 T = (preview->mouse - two_click_command->first_click);
                        dxf_selection_bbox.min += T;
                        dxf_selection_bbox.max += T;
                    }

                    dxf_selection_bbox.min -= target_preview_drawing_origin;
                    dxf_selection_bbox.max -= target_preview_drawing_origin;
                }
                bbox2 target_bbox; {
                    target_bbox = face_selection_bbox + dxf_selection_bbox;
                    for_(d, 2) {
                        if (target_bbox.min[d] > target_bbox.max[d]) {
                            target_bbox.min[d] = 0.0f;
                            target_bbox.max[d] = 0.0f;
                        }
                    }
                    {
                        real eps = 10.0f;
                        target_bbox.min[0] -= eps;
                        target_bbox.max[0] += eps;
                        target_bbox.min[1] -= eps;
                        target_bbox.max[1] += eps;
                    }
                }
                if (!feature_plane->is_active) {
                    target_bbox.min -= V2(10.0f);
                    target_bbox.max += V2(10.0f);
                }
                JUICEIT_EASYTWEEN(&preview->feature_plane.min, target_bbox.min);
                JUICEIT_EASYTWEEN(&preview->feature_plane.max, target_bbox.max);
                // if (other.time_since_plane_selected == 0.0f) { // FORNOW
                //     preview->feature_plane = target_bbox;
                // }
            }

            {
                mat4 PVM = PV_3D * M_3D_from_2D;
                vec3 target_feature_plane_color = get_color(ColorCode::Selection);
                {
                    if (state.enter_mode == EnterMode::NudgePlane) {
                        PVM *= M4_Translation(0.0f, 0.0f, preview->feature_plane_offset);
                        target_feature_plane_color = get_color(ColorCode::Emphasis); 
                    } else if (state.click_mode == ClickMode::Origin) {
                        target_feature_plane_color = get_color(ColorCode::Emphasis); 
                    } else if (moving_selected_entities) {
                        target_feature_plane_color = get_color(ColorCode::Emphasis); 
                    }
                }

                JUICEIT_EASYTWEEN(&preview->feature_plane_color, target_feature_plane_color);

                {
                    real f = CLAMPED_LERP(SQRT(other.time_since_plane_deselected), 1.0f, 0.0f);
                    if (feature_plane->is_active) f = CLAMPED_LERP(SQRT(3.0f * other.time_since_plane_selected), f, 1.0f);
                    // vec2 center = (preview->feature_plane.max + preview->feature_plane.min) / 2.0f;
                    // mat4 scaling_about_center = M4_Translation(center) * M4_Scaling(f) * M4_Translation(-center);
                    eso_begin(PVM * M4_Translation(0.0f, 0.0f, Z_FIGHT_EPS)/* * scaling_about_center*/, SOUP_QUADS);
                    eso_color(preview->feature_plane_color, f * 0.4f);
                    eso_bbox_SOUP_QUADS(preview->feature_plane);
                    eso_end();
                }
            }
        }

        glDisable(GL_SCISSOR_TEST);
    }


    { // cursor

        bool drag_none = (other.mouse_left_drag_pane == Pane::None);
        bool drag_drawing = (other.mouse_left_drag_pane == Pane::Drawing);
        bool drag_popup = (other.mouse_left_drag_pane == Pane::Popup);
        bool drag_separator = (other.mouse_left_drag_pane == Pane::Separator);
        bool drag_toolbox = (other.mouse_left_drag_pane == Pane::Toolbox);
        bool hot_popup = (other.hot_pane == Pane::Popup);
        bool hot_drawing = (other.hot_pane == Pane::Drawing);
        bool hot_separator = (other.hot_pane == Pane::Separator);
        bool hot_toolbox = (other.hot_pane == Pane::Toolbox);
        bool drag_none_and_hot_popup = (drag_none && hot_popup);
        bool drag_none_and_hot_separator = (drag_none && hot_separator);
        bool drag_none_and_hot_drawing = (drag_none && hot_drawing);
        bool drag_none_and_hot_toolbox = (drag_none && hot_toolbox);

        {
            GLFWcursor *next; {
                if (drag_none_and_hot_popup || drag_popup) {
                    next = other.cursors.ibeam;
                } else if (drag_none_and_hot_separator || drag_separator) {
                    next = other.cursors.hresize;
                } else if (drag_none_and_hot_drawing || drag_drawing) {
                    if (click_mode_SNAP_ELIGIBLE()) {
                        next = other.cursors.crosshair;
                    } else {
                        next = NULL;
                    }
                } else if (drag_none_and_hot_toolbox || drag_toolbox) {
                    next = other.cursors.hand;
                } else {
                    next = NULL;
                }
            }
            if (other.cursors.curr != next) {
                other.cursors.curr = next;
                glfwSetCursor(glfw_window, next);
            }
        }

        {
            real target = (drag_none_and_hot_drawing || drag_drawing) ? 1.0f : 0.0f;
            JUICEIT_EASYTWEEN(&preview->cursor_subtext_alpha, target, 2.0f);
        }
        vec3 color; {
            color = omax.white;
            if ((state.click_mode == ClickMode::Color) && (state.click_modifier != ClickModifier::Selected)) {
                color = get_color(state.click_color_code);
            }
        }

        String string_click_mode = STRING(
                (state.click_mode == ClickMode::None)           ? ""                :
                (state.click_mode == ClickMode::Axis)           ? "Axis"            :
                (state.click_mode == ClickMode::Box)            ? "Box"             :
                (state.click_mode == ClickMode::CenteredBox)    ? "CenteredBox"     :
                (state.click_mode == ClickMode::Circle)         ? "Circle"          :
                (state.click_mode == ClickMode::Color)          ? "Color"           :
                (state.click_mode == ClickMode::Deselect)       ? "Deselect"        :
                (state.click_mode == ClickMode::DogEar)         ? "DogEar"          :
                (state.click_mode == ClickMode::Fillet)         ? "Fillet"          :
                (state.click_mode == ClickMode::Line)           ? "Line"            :
                (state.click_mode == ClickMode::LinearCopy)     ? "LinearCopy"      :
                (state.click_mode == ClickMode::Measure)        ? "Measure"         :
                (state.click_mode == ClickMode::Move)           ? "Move"            :
                (state.click_mode == ClickMode::Offset)         ? "Offset"          :
                (state.click_mode == ClickMode::Origin)         ? "Origin"          :
                (state.click_mode == ClickMode::Polygon)        ? "Polygon"         :
                (state.click_mode == ClickMode::PowerFillet)    ? "PowerFillet"     :
                (state.click_mode == ClickMode::Select)         ? "Select"          :
                (state.click_mode == ClickMode::Rotate)         ? "Rotate"          :
                (state.click_mode == ClickMode::RotateCopy)     ? "RotateCopy"      :
                (state.click_mode == ClickMode::MirrorLine)     ? "MirrorLine"      :
                (state.click_mode == ClickMode::MirrorX)        ? "MirrorX"         :
                (state.click_mode == ClickMode::MirrorY)        ? "MirrorY"         :
                (state.click_mode == ClickMode::TwoEdgeCircle)  ? "TwoEdgeCircle"   :
                (state.click_mode == ClickMode::TwoClickDivide) ? "TwoClickDivide"  :
                "???MODE???");

        String string_click_modifier = STRING(
                (state.click_modifier == ClickModifier::None)           ? ""                :
                (state.click_modifier == ClickModifier::Center)         ? "Center"          :
                (state.click_modifier == ClickModifier::Connected)      ? "Connected"       :
                (state.click_modifier == ClickModifier::End)            ? "End"             :
                (state.click_modifier == ClickModifier::Color)          ? "Color"           :
                (state.click_modifier == ClickModifier::Middle)         ? "Middle"          :
                (state.click_modifier == ClickModifier::Perpendicular)  ? "Perpendicular"   :
                (state.click_modifier == ClickModifier::Quad)           ? "Quad"            :
                (state.click_modifier == ClickModifier::Selected)       ? "Selected"        :
                (state.click_modifier == ClickModifier::Window)         ? "Window"          :
                (state.click_modifier == ClickModifier::XY)             ? "XY"              :
                "???MODIFIER???");

        { // spoof callback_cursor_position
            double xpos, ypos;
            glfwGetCursorPos(glfw_window, &xpos, &ypos);
            void callback_cursor_position(GLFWwindow *, double xpos, double ypos);
            callback_cursor_position(NULL, xpos, ypos);
        }

        EasyTextPen pen = { other.mouse_Pixel + V2(12.0f, 16.0f), 12.0f, color, true, 1.0f - preview->cursor_subtext_alpha };
        easy_text_draw(&pen, string_click_mode);
        easy_text_draw(&pen, string_click_modifier);
    }

    void history_debug_draw(); // forward declaration


    if (other.show_help) {
        eso_begin(M4_Identity(), SOUP_QUADS); {
            eso_overlay(true);
            eso_color(omax.black, 0.7f);
            eso_vertex(-1.0f, -1.0f);
            eso_vertex(-1.0f,  1.0f);
            eso_vertex( 1.0f,  1.0f);
            eso_vertex( 1.0f, -1.0f);
        } eso_end();

        auto keybind_to_string = [](Keybind keybind) -> char* {

            bool control = keybind.mods & MOD_CTRL;
            bool shift = keybind.mods & MOD_SHIFT;
            bool alt = keybind.mods & MOD_ALT;
            KeyEvent tmp = { {}, keybind.key, control, shift, alt }; 

            return key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(&tmp);
        };
        EasyTextPen pen1 = { V2(25.0f, 16.0f), 16.0f, omax.white, true}; // FORNOW
        #define PRINT_KEYBIND(PEN, NAME) \
        easy_text_drawf(PEN, "  %s: %s", #NAME, \
                keybind_to_string(keybinds.NAME));
        EasyTextPen pen2 = pen1;
        pen2.origin_Pixel.x += 450.0f;


        //////////////////////////////////////////
        //////  SNAP COMMANDS  ///////////////////
        //////////////////////////////////////////

        easy_text_drawf(&pen1, "SNAP COMMANDS\n");
        PRINT_KEYBIND(&pen1, CENTER);
        PRINT_KEYBIND(&pen1, END);
        PRINT_KEYBIND(&pen1, MIDDLE);
        PRINT_KEYBIND(&pen1, PERPENDICULAR);
        PRINT_KEYBIND(&pen1, QUAD);
        PRINT_KEYBIND(&pen1, XY);
        PRINT_KEYBIND(&pen1, ZERO);


        //////////////////////////////////////////
        //////  SELECT COMMANDS  /////////////////
        //////////////////////////////////////////

        easy_text_drawf(&pen1, "\nSELECT COMMANDS\n");
        PRINT_KEYBIND(&pen1, COLOR);
        PRINT_KEYBIND(&pen1, SELECT_CONNECTED);
        PRINT_KEYBIND(&pen1, SELECT_ALL);
        PRINT_KEYBIND(&pen1, SELECT_WINDOW);


        //////////////////////////////////////////
        //////  OTHER COMMANDS  //////////////////
        //////////////////////////////////////////

        easy_text_drawf(&pen1, "\nOTHER COMMANDS\n");
        PRINT_KEYBIND(&pen1, AXIS);
        PRINT_KEYBIND(&pen1, BOX);
        PRINT_KEYBIND(&pen1, CHANGE_ORIGIN);
        PRINT_KEYBIND(&pen1, CIRCLE);
        PRINT_KEYBIND(&pen1, CLEAR_DRAWING);
        PRINT_KEYBIND(&pen1, CLEAR_MESH);
        PRINT_KEYBIND(&pen1, CYCLE_FEATURE_PLANE);
        PRINT_KEYBIND(&pen1, DELETE_SELECTED); // TODO
        PRINT_KEYBIND(&pen1, DELETE_SELECTED_ALTERNATE); //TODO
        PRINT_KEYBIND(&pen1, DESELECT);
        PRINT_KEYBIND(&pen1, DIVIDE_NEAREST);
        PRINT_KEYBIND(&pen1, DRAWING_FRAME);
        PRINT_KEYBIND(&pen1, EXIT_COMMAND); // TODO
        PRINT_KEYBIND(&pen1, EXTRUDE_ADD);
        PRINT_KEYBIND(&pen1, EXTRUDE_CUT);
        PRINT_KEYBIND(&pen1, FILLET);
        PRINT_KEYBIND(&pen1, HELP_MENU);
        PRINT_KEYBIND(&pen1, LINE);
        PRINT_KEYBIND(&pen1, LINEAR_COPY);
        PRINT_KEYBIND(&pen1, LOAD_FILE);
        PRINT_KEYBIND(&pen1, MEASURE);
        PRINT_KEYBIND(&pen1, MIRROR_LINE);
        PRINT_KEYBIND(&pen1, MIRROR_X);
        PRINT_KEYBIND(&pen1, MIRROR_Y);
        PRINT_KEYBIND(&pen2, MOVE);
        PRINT_KEYBIND(&pen2, NEXT_POPUP_BAR);
        PRINT_KEYBIND(&pen2, NUDGE_PLANE);
        PRINT_KEYBIND(&pen2, OFFSET);
        PRINT_KEYBIND(&pen2, POLYGON);
        PRINT_KEYBIND(&pen2, POWER_FILLET);
        PRINT_KEYBIND(&pen2, PREVIOUS_HOT_KEY_2D);
        PRINT_KEYBIND(&pen2, PREVIOUS_HOT_KEY_3D);
        PRINT_KEYBIND(&pen2, PRINT_HISTORY);
        PRINT_KEYBIND(&pen2, QUALITY_0);
        PRINT_KEYBIND(&pen2, QUALITY_1);
        PRINT_KEYBIND(&pen2, QUALITY_2);
        PRINT_KEYBIND(&pen2, QUALITY_3);
        PRINT_KEYBIND(&pen2, QUALITY_4);
        PRINT_KEYBIND(&pen2, QUALITY_5);
        PRINT_KEYBIND(&pen2, QUALITY_6);
        PRINT_KEYBIND(&pen2, QUALITY_7);
        PRINT_KEYBIND(&pen2, QUALITY_8);
        PRINT_KEYBIND(&pen2, QUALITY_9);
        PRINT_KEYBIND(&pen2, REDO);
        PRINT_KEYBIND(&pen2, REDO_ALTERNATE);
        PRINT_KEYBIND(&pen2, RESIZE);
        PRINT_KEYBIND(&pen2, REVOLVE_ADD);
        PRINT_KEYBIND(&pen2, REVOLVE_CUT);
        PRINT_KEYBIND(&pen2, ROTATE);
        PRINT_KEYBIND(&pen2, ROTATE_COPY);
        PRINT_KEYBIND(&pen2, SAVE);
        PRINT_KEYBIND(&pen2, SAVE_AS);
        PRINT_KEYBIND(&pen2, SELECT);
        PRINT_KEYBIND(&pen2, TOGGLE_BUTTONS);
        PRINT_KEYBIND(&pen2, TOGGLE_DRAWING_DETAILS);
        PRINT_KEYBIND(&pen2, TOGGLE_EVENT_STACK);
        PRINT_KEYBIND(&pen2, TOGGLE_FEATURE_PLANE);
        PRINT_KEYBIND(&pen2, TOGGLE_GRID);
        PRINT_KEYBIND(&pen2, TOGGLE_LIGHT_MODE);
        PRINT_KEYBIND(&pen2, TWO_CLICK_DIVIDE);
        PRINT_KEYBIND(&pen2, TWO_EDGE_CIRCLE);
        PRINT_KEYBIND(&pen2, UNDO);
        PRINT_KEYBIND(&pen2, UNDO_ALTERNATE);
        PRINT_KEYBIND(&pen2, ZOOM_3D_CAMERA);
    }

    if (other.show_event_stack) history_debug_draw();

    if (other.paused) { // pause 
        real x = 12.0f;
        real y = window_get_height_Pixel() - 12.0f;
        real w = 6.0f;
        real h = -2.5f * w;
        eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
        eso_overlay(true);
        eso_color(omax.green);
        for_(d, 2) {
            real o = d * (1.7f * w);
            eso_vertex(x     + o, y    );
            eso_vertex(x     + o, y + h);
            eso_vertex(x + w + o, y + h);
            eso_vertex(x + w + o, y    );
        }
        eso_end();
    }

}

