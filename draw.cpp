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

    if (mesh->cosmetic_edges) {
        eso_begin(PVM_3D, SOUP_LINES); 
        // eso_color(CLAMPED_LERP(2 * time_since_successful_feature, omax.white, omax.black));
        eso_color(0.0f, 0.0f, 0.0f);
        // 3 * num_triangles * 2 / 2
        for_(i, mesh->num_cosmetic_edges) {
            for_(d, 2) {
                eso_vertex(mesh->vertex_positions[mesh->cosmetic_edges[i][d]]);
            }
        }
        eso_end();
    }
    for_(pass, 2) {
        eso_begin(PVM_3D, (!other.show_details) ? SOUP_TRIANGLES : SOUP_OUTLINED_TRIANGLES);

        mat3 inv_transpose_V_3D = inverse(transpose(M3(V_3D(0, 0), V_3D(0, 1), V_3D(0, 2), V_3D(1, 0), V_3D(1, 1), V_3D(1, 2), V_3D(2, 0), V_3D(2, 1), V_3D(2, 2))));


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
                vec3 color_n = V3(0.5f + 0.5f * n_Camera.x, 0.5f + 0.5f * n_Camera.y, 1.0f);
                if ((feature_plane->is_active) && (dot(n, feature_plane->normal) > 0.99f) && (ABS(x_n - feature_plane->signed_distance_to_world_origin) < 0.01f)) {
                    if (pass == 0) continue;

                    // TODO:
                    color = CLAMPED_LERP(_JUICEIT_EASYTWEEN(other.time_since_plane_selected), omax.white, V3(0.85f, 0.87f, 0.30f));// CLAMPED_LERP(2.0f * time_since_plane_selected - 0.5f, omax.yellow, V3(0.85f, 0.87f, 0.30f));

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
    mat4 P_2D = camera_get_P(&other.camera_drawing);
    mat4 V_2D = camera_get_V(&other.camera_drawing);
    mat4 PV_2D = P_2D * V_2D;
    mat4 inv_PV_2D = inverse(camera_get_PV(camera_drawing));
    vec2 mouse_World_2D = transformPoint(inv_PV_2D, other.mouse_OpenGL);
    mat4 M_3D_from_2D = get_M_3D_from_2D();

    bool extruding = ((state.enter_mode == EnterMode::ExtrudeAdd) || (state.enter_mode == EnterMode::ExtrudeCut));
    bool revolving = ((state.enter_mode == EnterMode::RevolveAdd) || (state.enter_mode == EnterMode::RevolveCut));
    bool adding     = ((state.enter_mode == EnterMode::ExtrudeAdd) || (state.enter_mode == EnterMode::RevolveAdd));
    bool cutting     = ((state.enter_mode == EnterMode::ExtrudeCut) || (state.enter_mode == EnterMode::RevolveCut));

    // FORNOW: repeated computation; TODO function
    bool dxf_anything_selected;
    {
        dxf_anything_selected = false;
        _for_each_selected_entity_ {
            dxf_anything_selected = true;
            break;
        }
    }



    { // preview_extrude_in_length
        real target = (adding) ? popup->extrude_add_in_length : popup->extrude_cut_in_length;
        JUICEIT_EASYTWEEN(&preview->extrude_in_length, target);
    }
    { // preview_extrude_out_length
        real target = (adding) ? popup->extrude_add_out_length : popup->extrude_cut_out_length;
        JUICEIT_EASYTWEEN(&preview->extrude_out_length, target);
    }
    // TODO
    { // preview_feature_plane_offset
        real target = (state.enter_mode == EnterMode::NudgeFeaturePlane) ? popup->feature_plane_nudge : 0.0f;
        JUICEIT_EASYTWEEN(&preview->feature_plane_offset, target);
    }

    // preview
    vec2 preview_mouse = magic_snap(mouse_World_2D, true);
    vec2 preview_dxf_origin; {
        if (state.click_mode != ClickMode::Origin) {
            preview_dxf_origin = drawing->origin;
        } else {
            preview_dxf_origin = preview_mouse;
        }
    }
    vec2 preview_dxf_axis_base_point;
    real preview_dxf_axis_angle_from_y;
    {
        if (state.click_mode != ClickMode::Axis) {
            preview_dxf_axis_base_point = drawing->axis_base_point;
            preview_dxf_axis_angle_from_y = drawing->axis_angle_from_y;
        } else if (!two_click_command->awaiting_second_click) {
            preview_dxf_axis_base_point = preview_mouse;
            preview_dxf_axis_angle_from_y = drawing->axis_angle_from_y;
        } else {
            preview_dxf_axis_base_point = two_click_command->first_click;
            preview_dxf_axis_angle_from_y = atan2(preview_mouse - preview_dxf_axis_base_point) - PI / 2;
        }
    }


    mat4 P_3D = camera_get_P(&other.camera_mesh);
    mat4 V_3D = camera_get_V(&other.camera_mesh);
    mat4 PV_3D = P_3D * V_3D;

    uint window_width, window_height; {
        vec2 _window_size = window_get_size_Pixel();
        window_width = uint(_window_size.x);
        window_height = uint(_window_size.y);
    }

    { // panes
        bool dragging = (other.mouse_left_drag_pane == Pane::Separator);
        bool hovering = ((other.mouse_left_drag_pane == Pane::None) && (other.hot_pane == Pane::Separator));
        eso_begin(M4_Identity(), SOUP_LINES,
                dragging ? 2.0f
                : hovering ? 6.0f
                : 4.0f, true);
        eso_color(
                dragging ? omax.light_gray
                : hovering ? omax.white
                : omax.dark_gray);
        eso_vertex(other.x_divider_OpenGL,  1.0f);
        eso_vertex(other.x_divider_OpenGL, -1.0f);
        eso_end();
    }

    real x_divider_Pixel = get_x_divider_Pixel();

    { // draw 2D draw 2d draw
        vec2 *first_click = &two_click_command->first_click;

        {
            glEnable(GL_SCISSOR_TEST);
            gl_scissor_TODO_CHECK_ARGS(0, 0, x_divider_Pixel, window_height);
        }

        {
            #if 0
            // TODO: section view
            mesh_draw(P_2D, V_2D, inverse(get_M_3D_from_2D()));
            #endif
        }

        {
            if (other.show_grid) { // grid 2D grid 2d grid
                eso_begin(PV_2D, SOUP_LINES, 2.0f);
                eso_color(omax.dark_gray);
                for (uint i = 0; i <= uint(GRID_SIDE_LENGTH / GRID_SPACING); ++i) {
                    real tmp = i * GRID_SPACING;
                    eso_vertex(tmp, 0.0f);
                    eso_vertex(tmp, GRID_SIDE_LENGTH);
                    eso_vertex(0.0f, tmp);
                    eso_vertex(GRID_SIDE_LENGTH, tmp);
                }
                eso_end();
                eso_begin(PV_2D, SOUP_LINE_LOOP, 2.0f);
                eso_vertex(0.0f, 0.0f);
                eso_vertex(0.0f, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, 0.0f);
                eso_end();
            }
            if (1) { // axes 2D axes 2d axes axis 2D axis 2d axes crosshairs cross hairs origin 2d origin 2D origin
                real funky_OpenGL_factor = other.camera_drawing.ortho_screen_height_World / 120.0f;
                real LL = 1000 * funky_OpenGL_factor;

                eso_color(omax.white);
                if (0) {
                    eso_begin(PV_2D, SOUP_LINES, 1.5f); {
                        // axis
                        vec2 v = LL * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                        eso_vertex(preview_dxf_axis_base_point + v);
                        eso_vertex(preview_dxf_axis_base_point - v);
                    } eso_end();
                }
                eso_begin(PV_2D, SOUP_LINES, 3.0f); {
                    // origin
                    real r = funky_OpenGL_factor;
                    eso_vertex(preview_dxf_origin - V2(r, 0));
                    eso_vertex(preview_dxf_origin + V2(r, 0));
                    eso_vertex(preview_dxf_origin - V2(0, r));
                    eso_vertex(preview_dxf_origin + V2(0, r));
                } eso_end();
            }
            { // entities
                eso_begin(PV_2D, SOUP_LINES);
                _for_each_entity_ {
                    ColorCode color_code = (!entity->is_selected) ? entity->color_code : ColorCode::Selection;
                    real dx = 0.0f;
                    real dy = 0.0f;
                    if ((state.click_mode == ClickMode::Move) && (two_click_command->awaiting_second_click)) {
                        if (entity->is_selected) {
                            dx = preview_mouse.x - first_click->x;
                            dy = preview_mouse.y - first_click->y;
                            color_code = ColorCode::WaterOnly;
                        }
                    }
                    eso_color(get_color(color_code));
                    eso_entity__SOUP_LINES(entity, dx, dy);
                }
                eso_end();
            }
            { // dots
                if (other.show_details) {
                    eso_begin(camera_get_PV(&other.camera_drawing), SOUP_POINTS, 4.0f);
                    eso_color(omax.white);
                    _for_each_entity_ {
                        real start_x, start_y, end_x, end_y;
                        entity_get_start_and_end_points(entity, &start_x, &start_y, &end_x, &end_y);
                        eso_vertex(start_x, start_y);
                        eso_vertex(end_x, end_y);
                    }
                    eso_end();
                }
            }

            if (two_click_command->awaiting_second_click) {
                if (
                        0
                        || (state.click_modifier == ClickModifier::Window)
                        ||(state.click_mode == ClickMode::BoundingBox )
                   ) {
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(basic.cyan);
                    eso_vertex(first_click->x, first_click->y);
                    eso_vertex(preview_mouse.x, first_click->y);
                    eso_vertex(preview_mouse.x, preview_mouse.y);
                    eso_vertex(first_click->x, preview_mouse.y);
                    eso_end();
                }
                if (state.click_mode == ClickMode::Measure) { // measure line_entity
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(basic.cyan);
                    eso_vertex(two_click_command->first_click);
                    eso_vertex(preview_mouse);
                    eso_end();
                }
                if (state.click_mode == ClickMode::Line) { // measure line_entity
                    eso_begin(PV_2D, SOUP_LINES);
                    eso_color(basic.cyan);
                    eso_vertex(two_click_command->first_click);
                    eso_vertex(preview_mouse);
                    eso_end();
                }
                if (state.click_mode == ClickMode::Circle) {
                    vec2 c = { two_click_command->first_click.x, two_click_command->first_click.y };
                    vec2 p = preview_mouse;
                    real r = norm(c - p);
                    eso_begin(PV_2D, SOUP_LINE_LOOP);
                    eso_color(basic.cyan);
                    for_(i, NUM_SEGMENTS_PER_CIRCLE) eso_vertex(c + r * e_theta(real(i) / NUM_SEGMENTS_PER_CIRCLE * TAU));
                    eso_end();
                }
                if (state.click_mode == ClickMode::Fillet) {
                    // FORNOW
                    DXFFindClosestEntityResult dxf_find_closest_entity_result = dxf_find_closest_entity(&drawing->entities, two_click_command->first_click.x, two_click_command->first_click.y);
                    if (dxf_find_closest_entity_result.success) {
                        uint i = dxf_find_closest_entity_result.index;
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(get_color(ColorCode::WaterOnly));
                        eso_entity__SOUP_LINES(&drawing->entities.array[i]);
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
            gl_scissor_TODO_CHECK_ARGS(x_divider_Pixel, 0, window_width - x_divider_Pixel, window_height);
        }


        if (feature_plane->is_active) { // selection 2d selection 2D selection tube tubes slice slices stack stacks wire wireframe wires frame (FORNOW: ew)
            ;
            // FORNOW
            bool moving_stuff = ((state.click_mode == ClickMode::Origin) || (state.enter_mode == EnterMode::NudgeFeaturePlane));
            vec3 target_preview_tubes_color = (0) ? V3(0)
                : (adding) ? get_color(ColorCode::Traverse)
                : (cutting) ? get_color(ColorCode::Quality1)
                : (moving_stuff) ? get_color(ColorCode::WaterOnly)
                : omax.yellow;
            JUICEIT_EASYTWEEN(&preview->tubes_color, target_preview_tubes_color);

            uint NUM_TUBE_STACKS_INCLUSIVE;
            mat4 M;
            mat4 M_incr;
            {
                mat4 T_o = M4_Translation(preview_dxf_origin);
                mat4 inv_T_o = M4_Translation(-preview_dxf_origin);
                if (extruding) {
                    real a = -preview->extrude_in_length;
                    real L = preview->extrude_out_length + preview->extrude_in_length;
                    NUM_TUBE_STACKS_INCLUSIVE = MIN(64U, uint(roundf(L / 2.5f)) + 2);
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0.0f, 0.0f, a + Z_FIGHT_EPS);
                    M_incr = M4_Translation(0.0f, 0.0f, L / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                } else if (revolving) {
                    NUM_TUBE_STACKS_INCLUSIVE = 64;
                    M = M_3D_from_2D * inv_T_o;
                    { // M_incr
                        real a = 0.0f;
                        real b = TAU;
                        mat4 R_a = M4_RotationAbout(V3(e_theta(PI / 2 + preview_dxf_axis_angle_from_y), 0.0f), (b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                        mat4 T_a = M4_Translation(V3(preview_dxf_axis_base_point, 0.0f));
                        mat4 inv_T_a = inverse(T_a);
                        M_incr = T_o * T_a * R_a * inv_T_a * inv_T_o;
                    }
                } else if (state.click_mode == ClickMode::Origin) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0, 0, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else if (state.enter_mode == EnterMode::NudgeFeaturePlane) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0.0f, 0.0f, preview->feature_plane_offset + Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else { // default
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0, 0, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                }

                for_(tube_stack_index, NUM_TUBE_STACKS_INCLUSIVE) {
                    eso_begin(PV_3D * M, SOUP_LINES, 5.0f); {
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
            eso_color(omax.white);
            eso_begin(PV_3D * M_3D_from_2D * M4_Translation(0.0f, 0.0f, Z_FIGHT_EPS), SOUP_LINES, 2.0f);
            eso_vertex(-r, 0.0f);
            eso_vertex( r, 0.0f);
            eso_vertex(0.0f, -r);
            eso_vertex(0.0f,  r);
            if (0) {
                // TODO: clip this to the feature_plane
                real LL = 10.0f;
                vec2 v = LL * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                vec2 a = preview_dxf_axis_base_point + v;
                vec2 b = preview_dxf_axis_base_point - v;
                eso_vertex(-preview_dxf_origin + a);
                eso_vertex(-preview_dxf_origin + b); // FORNOW
            }
            eso_end();
        }


        { // feature plane feature-plane feature_plane
            bbox2 face_selection_bbox = mesh_draw(P_3D, V_3D, M4_Identity());
            bbox2 dxf_selection_bbox = entities_get_bbox(&drawing->entities, true);
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
            JUICEIT_EASYTWEEN(&preview->feature_plane.min, target_bbox.min);
            JUICEIT_EASYTWEEN(&preview->feature_plane.max, target_bbox.max);
            if (other.time_since_plane_selected == 0.0f) { // FORNOW
                preview->feature_plane = target_bbox;
            }
        }

        if (feature_plane->is_active) { // floating sketch plane; selection plane NOTE: transparent
            bool draw = true;
            mat4 PVM = PV_3D * M_3D_from_2D;
            vec3 target_feature_plane_color = omax.yellow;
            real sign = -1.0f;
            {
                if (state.enter_mode == EnterMode::NudgeFeaturePlane) {
                    PVM *= M4_Translation(-drawing->origin.x, -drawing->origin.y, preview->feature_plane_offset);
                    target_feature_plane_color = { 0.0f, 1.0f, 1.0f };
                    sign = 1.0f;
                    draw = true;
                } else if (state.click_mode == ClickMode::Origin) {
                    target_feature_plane_color = { 0.0f, 1.0f, 1.0f };
                    sign = 1.0f;
                    draw = true;
                } else {
                    if (dxf_anything_selected) PVM *= M4_Translation(-drawing->origin.x, -drawing->origin.y, 0.0f); // FORNOW
                }
            }

            JUICEIT_EASYTWEEN(&preview->feature_plane_color, target_feature_plane_color);

            if (draw) {
                real f = CLAMPED_LERP(SQRT(3.0f * other.time_since_plane_selected), 0.0f, 1.0f);
                vec2 center = (preview->feature_plane.max + preview->feature_plane.min) / 2.0f;
                mat4 scaling_about_center = M4_Translation(center) * M4_Scaling(f) * M4_Translation(-center);
                eso_begin(PVM * M4_Translation(0.0f, 0.0f, sign * Z_FIGHT_EPS) * scaling_about_center, SOUP_QUADS);
                eso_color(preview->feature_plane_color, f * 0.35f);
                eso_bbox_SOUP_QUADS(preview->feature_plane);
                eso_end();
            }
        }

        if (other.show_grid) { // grid 3D grid 3d grid
                               // conversation_draw_3D_grid_box(P_3D, V_3D);
        }

        glDisable(GL_SCISSOR_TEST);
    }


    { // cursor

        if (0) { // cursor
            eso_begin(other.OpenGL_from_Pixel, SOUP_TRIANGLES);
            eso_color(omax.white);
            eso_vertex(other.mouse_Pixel);
            eso_vertex(other.mouse_Pixel + V2(12.0f, 10.0f));
            eso_vertex(other.mouse_Pixel + V2(6.0f, 16.0f));
            eso_end();
        }

        vec3 color; {
            color = omax.white;
            if ((state.click_mode == ClickMode::Color) && (state.click_modifier != ClickModifier::Selected)) {
                color = get_color(state.click_color_code);
            }
        }
        real alpha = ((other.hot_pane == Pane::Drawing) && (other.mouse_left_drag_pane == Pane::None)) ? 1.0f : 0.5f;

        String string_click_mode = STRING(
                (state.click_mode == ClickMode::None)        ? ""         :
                (state.click_mode == ClickMode::Axis)        ? "AXIS"     :
                (state.click_mode == ClickMode::BoundingBox) ? "BOX"      :
                (state.click_mode == ClickMode::Circle)      ? "CIRCLE"   :
                (state.click_mode == ClickMode::Color)       ? "COLOR"    :
                (state.click_mode == ClickMode::Deselect)    ? "DESELECT" :
                (state.click_mode == ClickMode::Fillet)      ? "FILLET"   :
                (state.click_mode == ClickMode::Line)        ? "LINE"     :
                (state.click_mode == ClickMode::Measure)     ? "MEASURE"  :
                (state.click_mode == ClickMode::Move)        ? "MOVE"     :
                (state.click_mode == ClickMode::Origin)      ? "ORIGIN"   :
                (state.click_mode == ClickMode::Select)      ? "SELECT"   :
                (state.click_mode == ClickMode::MirrorX)     ? "MIRROR X" :
                (state.click_mode == ClickMode::MirrorY)     ? "MIRROR Y" :
                "???MODE???");

        String string_click_modifier = STRING(
                (state.click_modifier == ClickModifier::None)      ? ""          :
                (state.click_modifier == ClickModifier::Center)    ? "CENTER"    :
                (state.click_modifier == ClickModifier::Connected) ? "CONNECTED" :
                (state.click_modifier == ClickModifier::End)       ? "END"       :
                (state.click_modifier == ClickModifier::Color)     ? "COLOR"     :
                (state.click_modifier == ClickModifier::Middle)    ? "MIDDLE"    :
                (state.click_modifier == ClickModifier::Selected)  ? "SELECTED"  :
                (state.click_modifier == ClickModifier::Window)    ? "WINDOW"    :
                (state.click_modifier == ClickModifier::XY)        ? "XY"        :
                "???MODIFIER???");

        EasyTextPen pen = { other.mouse_Pixel + V2(12.0f, 16.0f), 12.0f, color, true, 1.0f - alpha };
        easy_text_draw(&pen, string_click_mode);
        easy_text_draw(&pen, string_click_modifier);
    }

    if (other.show_help) {
        { // FORNOW TODO: actually implement help
            messagef(omax.pink, "TODO: update help popup");
            other.show_help = false;
        }
    }

    void history_debug_draw(); // forward declaration

    void _messages_draw(); // forward declaration
    _messages_draw();

    if (other.show_event_stack) history_debug_draw();

    if (other.paused) { // pause 
        real x = 12.0f;
        real y = window_get_height_Pixel() - 12.0f;
        real w = 6.0f;
        real h = -2.5f * w;
        eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS, 0.0f, true);
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

