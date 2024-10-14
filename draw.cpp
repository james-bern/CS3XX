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
    // OLD VERSION:
    // vec3 y = (ARE_EQUAL(ABS(dot_product), 1.0f)) ? V3(0.0f,  0.0f, -1.0f * SGN(dot_product)) : up;
    // vec3 x = normalized(cross(y, feature_plane->normal));
    // vec3 z = cross(x, y);
    vec3 down = (ARE_EQUAL(ABS(dot_product), 1.0f)) ? V3(0.0f, 0.0f, 1.0f * SGN(dot_product)) : V3(0.0f, -1.0f, 0.0f);
    vec3 z = feature_plane->normal;
    vec3 x = normalized(cross(z, down));
    vec3 y = cross(z, x);

    // FORNOW
    if (ARE_EQUAL(ABS(dot_product), 1.0f) && SGN(dot_product) < 0.0f) {
        y *= -1;
    }

    return M4_xyzo(x, y, z, (feature_plane->signed_distance_to_world_origin) * feature_plane->normal);
}

bbox2 mesh_draw(mat4 P_3D, mat4 V_3D, mat4 M_3D) {
    bbox2 face_selection_bbox = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
    // mat4 inv_M_3D_from_2D = inverse(get_M_3D_from_2D());
    phong_draw(P_3D, V_3D, M_3D, mesh);
    // TODO: face_selection_bbox

    #if 1
    if (!other.show_details) {
        if (mesh->cosmetic_edges) {
            mat4 PVM_3D = P_3D * V_3D * M_3D;
            eso_begin(PVM_3D, SOUP_LINES); 
            // eso_color(CLAMPED_LERP(2 * time_since_successful_feature, pallete.white, pallete.black));
            eso_color(0,0,0);
            eso_size(2.0f);
            for_(i, mesh->num_cosmetic_edges) {
                for_(d, 2) {
                    eso_vertex(mesh->vertex_positions[mesh->cosmetic_edges[i][d]]);
                }
            }
            eso_end();
        }
    }
    #endif

    #if 0
    for_(pass, 2) {
        eso_begin(PVM_3D, (!other.show_details) ? SOUP_TRIANGLES : SOUP_TRI_MESH);
        eso_size(0.5f);

        // mat3 inv_transpose_V_3D = inverse(transpose(_M3(V_3D)));


        for_(i, mesh->num_triangles) {
            vec3 n = mesh->triangle_normals[i];
            vec3 p[3];
            real x_n;
            {
                for_(j, 3) p[j] = mesh->vertex_positions[mesh->triangle_indices[i][j]];
                x_n = dot(n, p[0]);
            }

            vec3 eye; {
                mat4 C = inverse(V_3D); // Is this recalculating the inverse for every triangle!?
                for_(d, 3) eye[d] = C(d, 3);
            }

            vec3 v = normalized(eye - p[0]);

            vec3 color; 
            real alpha;
            {
                // vec3 n_Camera = inv_transpose_V_3D * n;
                // vec3 color_n = V3(V2(0.66f) + 0.33f * _V2(n_Camera), 1.0f);
                // n_Camera = V3(0.5f) + 0.5f * n_Camera;
                // vec3 color_n = (n_Camera.x * monokai.red + n_Camera.y * monokai.blue + n_Camera.z * monokai.purple) / 2;
                vec3 color_n = V3(0.4f + 0.3f * MAX(0.0f, dot(n, v)));
                if ((true || feature_plane->is_active) && (dot(n, feature_plane->normal) > 0.99f) && (ABS(x_n - feature_plane->signed_distance_to_world_origin) < 0.01f)) {
                    if (pass == 0) continue;

                    // TODO:
                    if (feature_plane->is_active) {
                        // color = CLAMPED_LERP(_JUICEIT_EASYTWEEN(other.time_since_plane_selected), pallete.white, V3(0.65f, 0.67f, 0.10f));// CLAMPED_LERP(2.0f * time_since_plane_selected - 0.5f, pallete.yellow, V3(0.85f, 0.87f, 0.30f));
                        color = CLAMPED_LERP(_JUICEIT_EASYTWEEN(other.time_since_plane_selected), pallete.white, pallete.light_gray);// CLAMPED_LERP(2.0f * time_since_plane_selected - 0.5f, pallete.yellow, V3(0.85f, 0.87f, 0.30f));
                    } else color = color_n;

                    // if (2.0f * time_since_plane_selected < 0.3f) color = pallete.white; // FORNOW

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
            // color = CLAMPED_LINEAR_REMAP(time_since_successful_feature, -0.5f, 0.5f, pallete.white, color);
            eso_color(color, alpha);
            for_(d, 3) {
                eso_color(CLAMPED_LERP(mask + SIN(CLAMPED_INVERSE_LERP(p[d].y, mesh->bbox.max.y, mesh->bbox.min.y) + 0.5f * other.time_since_successful_feature), pallete.white, color), alpha);

                eso_vertex(p[d]);
            }
        }
        eso_end();
    }
    #endif

    return face_selection_bbox;
}

void conversation_draw() {
    mat4 P_2D = camera_drawing->get_P();
    mat4 V_2D = camera_drawing->get_V();
    mat4 PV_2D = P_2D * V_2D;
    mat4 inv_PV_2D = inverse(PV_2D);
    vec2 mouse_World_2D = transformPoint(inv_PV_2D, other.mouse_OpenGL);
    mat4 M_3D_from_2D = get_M_3D_from_2D();



    bool extruding = ((state_Mesh_command_is_(ExtrudeAdd)) || (state_Mesh_command_is_(ExtrudeCut)));
    bool revolving = ((state_Mesh_command_is_(RevolveAdd)) || (state_Mesh_command_is_(RevolveCut)));
    bool adding     = ((state_Mesh_command_is_(ExtrudeAdd)) || (state_Mesh_command_is_(RevolveAdd)));
    bool cutting     = ((state_Mesh_command_is_(ExtrudeCut)) || (state_Mesh_command_is_(RevolveCut)));

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
        real target = (state_Mesh_command_is_(NudgePlane)) ? popup->feature_plane_nudge : 0.0f;
        JUICEIT_EASYTWEEN(&preview->feature_plane_offset, target);
    }

    // preview
    vec2 mouse = magic_snap(mouse_World_2D, true).mouse_position; // this isn't really snapped per se (probably a bad name) -- just has the 15 deg stuff and similar

    if (two_click_command->awaiting_second_click && two_click_command->tangent_first_click) {
        //messagef(pallete.red, "wowowo");
        vec2 before = mouse;
        Entity *closest_entity = two_click_command->entity_closest_to_first_click;
        messagef(pallete.orange, "%f %f", closest_entity->arc.center.x, closest_entity->arc.center.y);
        vec2 center = closest_entity->arc.center;
        real radius = closest_entity->arc.radius;
        real d = distance(center, before);

        if (d > radius) {
            real t1 = acos(radius / d);
            real t2 = ATAN2(before - center);
            real theta = t1 + t2;
            two_click_command->first_click = { center.x + radius * COS(theta), center.y + radius * SIN(theta) };
        }
    }
    {
        vec2 target_preview_mouse = mouse;
        JUICEIT_EASYTWEEN(&preview->mouse, target_preview_mouse);
    }

    vec2 target_preview_drawing_origin = (!state_Draw_command_is_(SetOrigin)) ? drawing->origin : mouse;
    {
        JUICEIT_EASYTWEEN(&preview->drawing_origin, target_preview_drawing_origin);
    }

    // TODO: lerp
    vec2 preview_dxf_axis_base_point;
    real preview_dxf_axis_angle_from_y;
    {
        if (!state_Draw_command_is_(SetAxis)) {
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
        eso_size(dragging ? 1.0f
                : hovering ? 2.0f
                : 1.5f);
        eso_color(
                dragging ? pallete.white
                : hovering ? pallete.light_gray
                : pallete.gray);
        eso_vertex(other.x_divider_drawing_mesh_OpenGL, -1.0f);
        eso_vertex(other.x_divider_drawing_mesh_OpenGL,  1.0f);
        eso_end();
    }

    real x_divider_drawing_mesh_Pixel = get_x_divider_drawing_mesh_Pixel();

    bool moving_selected_entities = (
            (two_click_command->awaiting_second_click)
            && (0 
                || (state_Draw_command_is_(Move))
                || (state_Draw_command_is_(Rotate))
                || (state_Draw_command_is_(Copy)))
            ); // TODO: loft up

    { // draw 2D draw 2d draw
        auto DRAW_CROSSHAIR = [&](vec2 o, vec3 color) {
            real funky_OpenGL_factor = other.camera_drawing.ortho_screen_height_World / 120.0f;
            eso_begin(PV_2D, SOUP_LINES);
            eso_color(pallete.black);
            real r = 1.3 * funky_OpenGL_factor;
            eso_size(2.0f);
            eso_vertex(o - V2(r, 0));
            eso_vertex(o + V2(r, 0));
            eso_vertex(o - V2(0, r));
            eso_vertex(o + V2(0, r));
            eso_color(color);
            r = 1.2 * funky_OpenGL_factor;
            eso_size(1.0f);
            eso_vertex(o - V2(r, 0));
            eso_vertex(o + V2(r, 0));
            eso_vertex(o - V2(0, r));
            eso_vertex(o + V2(0, r));
            eso_end();
        };

        auto DRAW_BOX = [&](vec2 click_1, vec2 click_2, vec3 color) {
            eso_begin(PV_2D, SOUP_LINE_LOOP);
            eso_color(color);
            eso_vertex(click_1);
            eso_vertex(click_1.x, click_2.y);
            eso_vertex(click_2);
            eso_vertex(click_2.x, click_1.y);
            eso_end();
        };

        auto DRAW_CIRCLE = [&](vec2 click_1, vec2 click_2, vec3 color) {
            vec2 center = click_1;
            real radius = distance(click_1, click_2);
            eso_begin(PV_2D, SOUP_LINE_LOOP);
            eso_color(color);
            for_(i, NUM_SEGMENTS_PER_CIRCLE) {
                real theta = (real(i) / NUM_SEGMENTS_PER_CIRCLE) * TAU;
                eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta));
            }
            eso_end();
        };

        auto DRAW_LINE = [&](vec2 click_1, vec2 click_2, vec3 color) {
            eso_begin(PV_2D, SOUP_LINES);
            eso_color(color);
            eso_vertex(click_1);
            eso_vertex(click_2);
            eso_end();
        };

        auto DRAW_CENTERLINE = [&](vec2 click_1, vec2 click_2, vec3 color) {
            eso_begin(PV_2D, SOUP_LINES);
            eso_color(color);
            eso_vertex(click_1 + (click_1 - click_2));
            eso_vertex(click_2);
            eso_end();
        };

        auto DRAW_POLYGON = [&](vec2 click_1, vec2 click_2, vec3 color) {
            // TODO: JUICEIT_EASYTWEEN polygon_num_sides
            uint polygon_num_sides = uint(preview->polygon_num_sides);
            real delta_theta = -TAU / preview->polygon_num_sides;
            vec2 center = click_1;
            vec2 vertex_0 = click_2;
            real radius = distance(center, vertex_0);
            real theta_0 = ATAN2(vertex_0 - center);
            {
                eso_begin(PV_2D, SOUP_LINES);
                eso_stipple(true);
                eso_color(color);
                eso_vertex(center);
                eso_vertex(vertex_0);
                eso_end();
            }
            {
                eso_begin(PV_2D, SOUP_LINE_LOOP);
                eso_color(color);
                for_(i, polygon_num_sides) {
                    real theta_i = theta_0 + (i * delta_theta);
                    real theta_ip1 = theta_i + delta_theta;
                    eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta_i));
                    eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta_ip1));
                }
                eso_end();
            }
        };

        bool moving = (two_click_command->awaiting_second_click) && (state_Draw_command_is_(Move));
        bool linear_copying = (two_click_command->awaiting_second_click) && (state_Draw_command_is_(Copy));
        bool rotating = (two_click_command->awaiting_second_click) && (state_Draw_command_is_(Rotate));
        // bool moving_linear_copying_or_rotating = (moving || rotating || linear_copying);

        auto DRAW_ENTITIES_BEING_MOVED_LINEAR_COPIED_OR_ROTATED = [&](vec2 click_1, vec2 click_2, vec3 color) {
            // TODO: do this like crosshairs where they disappear more immediatelly
            if (IS_ZERO(squaredNorm(click_1 - click_2))) return; // NOTE: you want this (we're attempthing this call three times!)
            vec2 click_vector_12 = click_2 - click_1;
            real click_theta_12 = ATAN2(click_vector_12);
            mat4 M; {
                if (moving || linear_copying) {
                    M = M4_Translation(click_vector_12);
                } else { ASSERT(rotating);
                    M = M4_Translation(click_1) * M4_RotationAboutZAxis(click_theta_12) * M4_Translation(-click_1);
                }
            }
            eso_begin(PV_2D * M, SOUP_LINES);
            eso_color(color);
            _for_each_selected_entity_ eso_entity__SOUP_LINES(entity);
            eso_end();
        };



        glEnable(GL_SCISSOR_TEST);
        gl_scissor_Pixel(0, 0, x_divider_drawing_mesh_Pixel, window_height);
        {
            if (!other.hide_grid) { // grid 2D grid 2d grid // jim wtf are these supposed to mean
                mat4 PVM = PV_2D * M4_Translation(-GRID_SIDE_LENGTH / 2, -GRID_SIDE_LENGTH / 2);
                eso_begin(PVM, SOUP_LINES);
                eso_color(pallete.darker_gray);
                for (uint i = 0; i <= uint(GRID_SIDE_LENGTH / GRID_SPACING); ++i) {
                    real tmp = i * GRID_SPACING;
                    eso_vertex(tmp, 0.0f);
                    eso_vertex(tmp, GRID_SIDE_LENGTH);
                    eso_vertex(0.0f, tmp);
                    eso_vertex(GRID_SIDE_LENGTH, tmp);
                }
                eso_end();
                eso_begin(PVM, SOUP_LINE_LOOP);
                eso_color(pallete.dark_gray);
                eso_vertex(0.0f, 0.0f);
                eso_vertex(0.0f, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, 0.0f);
                eso_end();
            }
            if (1) { // axes 2D axes 2d axes axis 2D axis 2d axes crosshairs cross hairs origin 2d origin 2D origin
                real funky_OpenGL_factor = other.camera_drawing.ortho_screen_height_World / 120.0f;
                real r = 3 * funky_OpenGL_factor;
                // real LL = 1000 * funky_OpenGL_factor;

                // eso_begin(PV_2D, SOUP_LINES); {
                //     // axis
                //     eso_stipple(true);
                //     eso_color(pallete.dark_gray);
                //     if (state_Draw_command_is_(SetAxis)) {
                //         eso_color(get_color(ColorCode::Emphasis));
                //     } else if (state_Mesh_command_is_(RevolveAdd)) {
                //         eso_color(AVG(pallete.dark_gray, get_color(ColorCode::Emphasis)));
                //     } else if (state_Mesh_command_is_(RevolveCut)) {
                //         eso_color(AVG(pallete.dark_gray, get_color(ColorCode::Emphasis)));
                //     } else {
                //     }
                //     vec2 v = LL * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                //     eso_vertex(preview_dxf_axis_base_point + v);
                //     eso_vertex(preview_dxf_axis_base_point - v);
                // } eso_end();
                eso_begin(PV_2D, SOUP_POINTS); {
                    eso_overlay(true);
                    eso_color(pallete.white);
                    eso_size(6.0f);
                    eso_vertex(target_preview_drawing_origin - V2(0, 0));
                } eso_end();
                vec2 v = r * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                eso_begin(PV_2D, SOUP_LINES); {
                    eso_overlay(true);
                    eso_color(pallete.white);
                    eso_size(3.0f);
                    eso_vertex(preview_dxf_axis_base_point);
                    eso_size(0.0f);
                    eso_vertex(preview_dxf_axis_base_point + v);
                } eso_end();
                // eso_begin(PV_2D, SOUP_TRIANGLES); {
                //     eso_color(pallete.white);
                //     real eps = r / 5;
                //     eso_vertex(target_preview_drawing_origin + V2(r + 2 * eps, 0));
                //     eso_vertex(target_preview_drawing_origin + V2(r,         eps));
                //     eso_vertex(target_preview_drawing_origin + V2(r,        -eps));
                //     eso_vertex(target_preview_drawing_origin + V2(0, r + 2 * eps));
                //     eso_vertex(target_preview_drawing_origin + V2( eps, r));
                //     eso_vertex(target_preview_drawing_origin + V2(-eps, r));
                // } eso_end();
            }



            vec2 *first_click = &two_click_command->first_click;




            vec2 Draw_Enter; {
                Draw_Enter = *first_click;
                if (popup->manager.focus_group == ToolboxGroup::Draw) {
                    if (state_Draw_command_is_(Box)) Draw_Enter += V2(popup->box_width, popup->box_height);
                    if (state_Draw_command_is_(Circle)) Draw_Enter += V2(popup->circle_radius, 0.0f);
                    if (state_Draw_command_is_(Polygon)) Draw_Enter += V2(popup->polygon_distance_to_corner, 0.0f);
                    if (state_Draw_command_is_(Line)) Draw_Enter += V2(popup->line_run, popup->line_rise);
                    if (state_Draw_command_is_(Move)) Draw_Enter += V2(popup->move_run, popup->move_rise);
                    if (state_Draw_command_is_(Copy)) Draw_Enter += V2(popup->linear_copy_run, popup->linear_copy_rise);
                    if (state_Draw_command_is_(Rotate)) {
                        if (!IS_ZERO(popup->rotate_angle)) { // FORNOW
                            Draw_Enter += 10.0f * e_theta(RAD(popup->rotate_angle));
                        }
                    }
                }
            }
            vec2 Snap_Enter; {
                Snap_Enter = *first_click;
                if (popup->manager.focus_group == ToolboxGroup::Snap) {
                    if (state_Snap_command_is_(XY)) {
                        Snap_Enter = V2(popup->xy_x_coordinate, popup->xy_y_coordinate);
                    }
                }
            }

            MagicSnapResult true_snap_result = magic_snap(mouse);

            {
                JUICEIT_EASYTWEEN(&preview->popup_second_click, Draw_Enter);
                JUICEIT_EASYTWEEN(&preview->xy_xy, Snap_Enter);

                JUICEIT_EASYTWEEN(&preview->mouse_snap, true_snap_result.mouse_position, 1.0f);

                JUICEIT_EASYTWEEN(&preview->polygon_num_sides, real(popup->polygon_num_sides));
            }
            bool Snap_eating_mouse = !(state_Snap_command_is_(None) || state_Snap_command_is_(XY));
            vec2 position_mouse; {
                position_mouse = (!Snap_eating_mouse) ? mouse : preview->mouse_snap; // FORNOW
            }
            bool Draw_eating_Enter = ((popup->manager.focus_group == ToolboxGroup::Draw) &&
                    (!two_click_command->awaiting_second_click || !ARE_EQUAL(*first_click, Draw_Enter)));
            bool Snap_eating_Enter = ((popup->manager.focus_group == ToolboxGroup::Snap) && state_Snap_command_is_(XY) &&
                    (!two_click_command->awaiting_second_click || !ARE_EQUAL(*first_click, Snap_Enter)));
            vec3 target_color_mouse; {
                target_color_mouse = get_color(ColorCode::Emphasis);
                // if (Snap_eating_mouse) target_color_mouse = AVG(get_color(ColorCode::Emphasis), get_accent_color(ToolboxGroup::Snap));
                if (Snap_eating_mouse) target_color_mouse = get_accent_color(ToolboxGroup::Snap);
                if  (Draw_eating_Enter) {
                    target_color_mouse = CLAMPED_LERP(_JUICEIT_EASYTWEEN(-0.7f + 1.3f * 
                                MIN(other.time_since_popup_second_click_not_the_same, other.time_since_mouse_moved)
                                ), target_color_mouse, 0.2f * target_color_mouse);
                }
                if  (Snap_eating_Enter) {
                    target_color_mouse = CLAMPED_LERP(_JUICEIT_EASYTWEEN(-0.7f + 1.3f * 
                                MIN(other.time_since_popup_second_click_not_the_same, other.time_since_mouse_moved)
                                ), target_color_mouse, 0.2f * target_color_mouse);
                }
            }
            JUICEIT_EASYTWEEN(&preview->color_mouse, target_color_mouse);



            // vec2 click_vector = (position_mouse - *first_click);
            // real click_theta = ATAN2(click_vector);

            { // entities
                eso_begin(PV_2D, SOUP_LINES); {
                    // entities 2D entities 2d entities drawing 2D drawing 2d drawing
                    _for_each_entity_ {
                        if (entity->is_selected && (rotating || moving)) continue;
                        eso_color((entity->is_selected) ? get_color(ColorCode::Selection) : get_color(entity->color_code));
                        eso_size(1.5f);
                        eso_entity__SOUP_LINES(entity);
                    }
                } eso_end();


                { // dots snap_divide_dot
                    if (other.show_details) { // dots
                        eso_begin(PV_2D, SOUP_POINTS);
                        eso_size(3.0f);
                        eso_color(pallete.white);
                        _for_each_entity_ {
                            if (entity->is_selected && (rotating || moving)) continue;
                            if (entity->type == EntityType::Circle) {
                                CircleEntity *circle = &entity->circle;
                                if (circle->has_pseudo_point) eso_vertex(circle->get_pseudo_point());
                                continue;
                            }
                            eso_vertex(entity_get_start_point(entity));
                            eso_vertex(entity_get_end_point(entity));
                        }
                        eso_end();
                    }

                    { // snap_divide_dot
                        eso_begin(PV_2D, SOUP_POINTS);
                        eso_color(pallete.light_gray);
                        JUICEIT_EASYTWEEN(&other.size_snap_divide_dot, 0.0f, 0.5f);
                        eso_size(other.size_snap_divide_dot);
                        eso_vertex(other.snap_divide_dot);
                        eso_end();
                    }
                }
            }

            { // annotations

                // new-style annotations
                // FORNOW (this is sloppy and bad)
                #define ANNOTATION(Name, NAME) \
                do { \
                    if (state_Draw_command_is_(Name)) { \
                        DRAW_##NAME(*first_click, position_mouse, preview->color_mouse); \
                        DRAW_##NAME(*first_click, preview->popup_second_click, get_accent_color(ToolboxGroup::Draw)); \
                        DRAW_##NAME(*first_click, preview->xy_xy, get_accent_color(ToolboxGroup::Snap)); \
                    } \
                } while (0)

                if (two_click_command->awaiting_second_click) {
                    ANNOTATION(Line, LINE);
                    ANNOTATION(CenterLine, CENTERLINE);
                    ANNOTATION(Box, BOX);
                    ANNOTATION(Circle, CIRCLE);
                    ANNOTATION(Polygon, POLYGON);

                    ANNOTATION(Move, LINE);
                    ANNOTATION(Copy, LINE);
                    ANNOTATION(Rotate, LINE);
                    // NOTE: this is still kinda broken
                    ANNOTATION(Move, ENTITIES_BEING_MOVED_LINEAR_COPIED_OR_ROTATED);
                    ANNOTATION(Copy, ENTITIES_BEING_MOVED_LINEAR_COPIED_OR_ROTATED);
                    ANNOTATION(Rotate, ENTITIES_BEING_MOVED_LINEAR_COPIED_OR_ROTATED);
                }

                { // entity snapped to
                  // TODO: Intersect
                    if (true_snap_result.snapped) {
                        Entity *entity_snapped_to = &drawing->entities.array[true_snap_result.entity_index_snapped_to];
                        eso_begin(PV_2D, SOUP_LINES);
                        // eso_overlay(true);
                        eso_color(get_accent_color(ToolboxGroup::Snap));
                        eso_entity__SOUP_LINES(entity_snapped_to);
                        eso_end();
                    }
                }

                { // crosshairs
                    if (state_Snap_command_is_(XY)) {
                        if (popup->manager.focus_group == ToolboxGroup::Snap) {
                            if (!Snap_eating_Enter) other.time_since_popup_second_click_not_the_same = 0.0f;
                        }
                        if (Snap_eating_Enter) DRAW_CROSSHAIR(preview->xy_xy, get_accent_color(ToolboxGroup::Snap));
                    } else if (!state_Snap_command_is_(None)) {
                        DRAW_CROSSHAIR(preview->mouse_snap, get_accent_color(ToolboxGroup::Snap));
                    }

                    if (two_click_command->awaiting_second_click && !state_Draw_command_is_(None)) {
                        if (popup->manager.focus_group == ToolboxGroup::Draw) {
                            if (!Draw_eating_Enter) other.time_since_popup_second_click_not_the_same = 0.0f;
                        }
                        if (Draw_eating_Enter) DRAW_CROSSHAIR(preview->popup_second_click, pallete.cyan);
                    }
                }

                { // experimental preview part B
                  // NOTE: circle <-> circle is wonky
                    if (state_Draw_command_is_(Offset)) {
                        DXFFindClosestEntityResult closest_result = dxf_find_closest_entity(&drawing->entities, mouse);
                        if (closest_result.success) {
                            Entity *_closest_entity = closest_result.closest_entity;
                            Entity target_entity = entity_offsetted(_closest_entity, popup->offset_distance, mouse);
                            vec2 target_start, target_end, target_middle, target_opposite;
                            if (target_entity.type != EntityType::Circle) {
                                entity_get_start_and_end_points(&target_entity, &target_start, &target_end);
                                target_middle = entity_get_middle(&target_entity);
                                target_opposite = target_middle;
                            } else { ASSERT(target_entity.type == EntityType::Circle);
                                CircleEntity *circle = &target_entity.circle;
                                real angle; {
                                    if (ARE_EQUAL(preview->offset_entity_end, preview->offset_entity_start)) {
                                        angle = 0.0;
                                    } else {
                                        angle = (PI / 2) - ATAN2(normalized(preview->offset_entity_end - preview->offset_entity_start));
                                    }
                                }
                                // real angle = ATAN2(preview->offset_entity_middle - circle->center);
                                // real angle = ATAN2(mouse - circle->center);
                                // TODO: something else?
                                target_middle   = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle);
                                target_start    = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle - PI / 2);
                                target_end      = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle + PI / 2);
                                target_opposite = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle + PI);

                            }

                            // reasonable line <-> arc behavior
                            if (1) { // heuristic (FORNOW: minimize max distance)
                                real D2na = squaredDistance(preview->offset_entity_start, target_start);
                                real D2nb = squaredDistance(preview->offset_entity_end, target_end);
                                real D2ya = squaredDistance(preview->offset_entity_start, target_end);
                                real D2yb = squaredDistance(preview->offset_entity_end, target_start);
                                real max_D2_no_swap = MAX(D2na, D2nb);
                                real max_D2_yes_swap = MAX(D2ya, D2yb);
                                if (max_D2_no_swap > max_D2_yes_swap) {
                                    SWAP(&target_start, &target_end);
                                }
                            }

                            JUICEIT_EASYTWEEN(&preview->offset_entity_start, target_start);
                            JUICEIT_EASYTWEEN(&preview->offset_entity_end, target_end);
                            JUICEIT_EASYTWEEN(&preview->offset_entity_middle, target_middle);
                            JUICEIT_EASYTWEEN(&preview->offset_entity_opposite, target_opposite);

                            // TODO: could try a crescent moon kind of a situation
                            // TODO: just need a three point arc lambda
                            //       (and could in theory fillet the arcs)

                            vec2 a = preview->offset_entity_start;
                            vec2 b = preview->offset_entity_middle;
                            vec2 c = preview->offset_entity_end;
                            vec2 d = preview->offset_entity_opposite;
                            Entity dummy = entity_make_three_point_arc_or_line(a, b, c);
                            Entity dummy2 = entity_make_three_point_arc_or_line(a, d, c);
                            eso_begin(PV_2D, SOUP_LINES);
                            { // eso_vertex
                              // eso_color(1.0f, 0.0f, 1.0f);
                              // eso_vertex(a);
                              // eso_vertex(b);
                              // eso_vertex(b);
                              // eso_vertex(c);
                              // eso_color((distance(b, d) / distance(a, c)) * get_color(ColorCode::Emphasis));
                                eso_color(get_color(ColorCode::Emphasis));
                                eso_entity__SOUP_LINES(&dummy2);
                                eso_color(get_color(ColorCode::Emphasis));
                                eso_entity__SOUP_LINES(&dummy);
                            }
                            eso_end();
                        }
                    }
                }

                // FORNOW: old-style annotations
                if (!two_click_command->awaiting_second_click) {
                } else {
                    if (state_Xsel_command_is_(Window)) {
                        eso_begin(PV_2D, SOUP_LINE_LOOP);
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(first_click->x, first_click->y);
                        eso_vertex(mouse.x, first_click->y);
                        eso_vertex(mouse.x, mouse.y);
                        eso_vertex(first_click->x, mouse.y);
                        eso_end();
                    }

                    if (state_Draw_command_is_(CenterBox)) {                
                        vec2 one_corner = mouse;
                        vec2 center = *first_click;
                        real other_y = 2 * center.y - one_corner.y;
                        real other_x = 2 * center.x - one_corner.x;
                        eso_begin(PV_2D, SOUP_LINE_LOOP);
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(one_corner);
                        eso_vertex(V2(one_corner.x, other_y));
                        eso_vertex(V2(other_x, other_y));
                        if (two_click_command->tangent_first_click) {
                            two_click_command->tangent_first_click = false;
                        }
                        eso_vertex(V2(other_x, one_corner.y));
                        eso_end();
                    }
                    if (state_Draw_command_is_(Measure)) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(two_click_command->first_click);
                        eso_vertex(mouse);
                        eso_end();
                    }
                    if (state_Draw_command_is_(Mirror2)) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(two_click_command->first_click);
                        eso_vertex(mouse);
                        eso_end();
                    }
                    if (state_Draw_command_is_(Rotate)) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(two_click_command->first_click);
                        eso_vertex(mouse);
                        eso_end();
                    }
                    if (state_Draw_command_is_(DiamCircle)) {
                        vec2 edge_one = two_click_command->first_click;
                        vec2 edge_two = mouse;
                        vec2 center = (edge_one + edge_two) / 2;
                        real radius = norm(edge_one - center);
                        eso_begin(PV_2D, SOUP_LINE_LOOP);
                        eso_color(get_color(ColorCode::Emphasis));
                        for_(i, NUM_SEGMENTS_PER_CIRCLE) {
                            real theta = (real(i) / NUM_SEGMENTS_PER_CIRCLE) * TAU;
                            eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta));
                        }
                        eso_end();
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_stipple(true);
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(edge_one);
                        eso_vertex(edge_two);
                        eso_end();
                    }
                    if (state_Draw_command_is_(Divide2)) {
                        if (two_click_command->awaiting_second_click) {
                            eso_begin(PV_2D, SOUP_LINES);
                            eso_color(get_color(ColorCode::Emphasis));
                            eso_entity__SOUP_LINES(two_click_command->entity_closest_to_first_click);
                            eso_end();
                        }
                    }
                    if (state_Draw_command_is_(Fillet)) {
                        if (two_click_command->awaiting_second_click) {
                            eso_begin(PV_2D, SOUP_LINES);
                            eso_color(get_color(ColorCode::Emphasis));
                            eso_entity__SOUP_LINES(two_click_command->entity_closest_to_first_click);
                            eso_end();
                        }
                    }
                    if (state_Draw_command_is_(DogEar)) {
                        if (two_click_command->awaiting_second_click) {
                            eso_begin(PV_2D, SOUP_LINES);
                            eso_color(get_color(ColorCode::Emphasis));
                            eso_entity__SOUP_LINES(two_click_command->entity_closest_to_first_click);
                            eso_end();
                        }
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
            bool moving_stuff = ((state_Draw_command_is_(SetOrigin)) || (state_Mesh_command_is_(NudgePlane)));
            vec3 target_preview_tubes_color = (0) ? V3(0)
                : (moving_selected_entities) ? get_color(ColorCode::Emphasis)
                : (adding) ? pallete.green
                : (cutting) ? pallete.red
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
                    NUM_TUBE_STACKS_INCLUSIVE = MIN(64U, uint(ROUND(L / 0.1f)) + 2);
                    vec3 axis = V3(e_theta(PI / 2 + preview_dxf_axis_angle_from_y), 0.0f);
                    mat4 R_0 = M4_RotationAbout(axis, a);
                    mat4 R_inc = M4_RotationAbout(axis, L / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                    mat4 T_a = M4_Translation(V3(preview_dxf_axis_base_point, 0.0f));
                    mat4 inv_T_a = inverse(T_a);
                    // M_incr = T_o * T_a * R_a * inv_T_a * inv_T_o;
                    M_incr = T_a * R_inc * inv_T_a;
                    M = M_3D_from_2D * inv_T_o * T_a * R_0 * inv_T_a;
                } else if (state_Draw_command_is_(SetOrigin)) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0, 0, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else if (state_Mesh_command_is_(NudgePlane)) {
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
                            color = CLAMPED_LERP(-0.5f + SQRT(2.0f * entity->time_since_is_selected_changed), pallete.white, preview->tubes_color);
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
            real r = other.camera_mesh.ortho_screen_height_World / 100.0f;
            eso_begin(PV_3D * M_3D_from_2D * M4_Translation(0.0f, 0.0f, Z_FIGHT_EPS), SOUP_LINES);
            eso_color(pallete.white);
            eso_vertex(0, 0.0f);
            eso_vertex( r, 0.0f);
            eso_vertex(0.0f, 0);
            eso_vertex(0.0f,  r);
            if (revolving) {
                // TODO: clip this to the feature_plane
                real LL = 100.0f;
                vec2 v = LL * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                vec2 a = preview_dxf_axis_base_point + v;
                vec2 b = preview_dxf_axis_base_point - v;
                eso_color(get_color(ColorCode::Emphasis));
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
                eso_color(pallete.darker_gray);
                eso_size(2.0f);
                for (uint i = 0; i <= uint(GRID_SIDE_LENGTH / GRID_SPACING); ++i) {
                    real tmp = i * GRID_SPACING;
                    eso_vertex(tmp, 0.0f);
                    eso_vertex(tmp, GRID_SIDE_LENGTH);
                    eso_vertex(0.0f, tmp);
                    eso_vertex(GRID_SIDE_LENGTH, tmp);
                }
                eso_end();
                // eso_size(4.0f);
                // eso_begin(transform, SOUP_LINE_LOOP);
                // eso_color(pallete.dark_gray);
                // eso_vertex(0.0f, 0.0f);
                // eso_vertex(0.0f, GRID_SIDE_LENGTH);
                // eso_vertex(GRID_SIDE_LENGTH, GRID_SIDE_LENGTH);
                // eso_vertex(GRID_SIDE_LENGTH, 0.0f);
                // eso_end();
            }
        }

        { // feature plane feature-plane feature_plane // floating sketch plane; selection plane NOTE: transparent
            {
                bbox2 face_selection_bbox; {
                    // TODO fix
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
                    if (state_Mesh_command_is_(NudgePlane)) {
                        PVM *= M4_Translation(0.0f, 0.0f, preview->feature_plane_offset);
                        target_feature_plane_color = get_color(ColorCode::Emphasis); 
                    } else if (state_Draw_command_is_(SetOrigin)) {
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
                    if (state.Draw_command.flags & SNAPPER) {
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
        vec3 color = pallete.white;
        // {
        //     color = pallete.white;
        //     if ((state_Draw_command_is_(SetColor)) && (state.click_modifier != ClickModifier::OfSelection)) {
        //         color = get_color(state.click_color_code);
        //     }
        // }

        // TODO: somehow macro this

        String STRING_EMPTY_STRING = {};
        String Top_string = (state_Draw_command_is_(None)) ? STRING_EMPTY_STRING : state.Draw_command.name;
        String Bot_string; {
            if (0) ;
            else if (!state_Snap_command_is_(None)) Bot_string = state.Snap_command.name;
            else if (!state_Xsel_command_is_(None)) Bot_string = state.Xsel_command.name;
            else if (!state_Colo_command_is_(None)) Bot_string = state.Colo_command.name;
            else Bot_string = STRING("");
        }

        #if 0
        { // spoof callback_cursor_position
            double xpos, ypos;
            glfwGetCursorPos(glfw_window, &xpos, &ypos);
            void callback_cursor_position(GLFWwindow *, double xpos, double ypos);
            callback_cursor_position(NULL, xpos, ypos);
        }
        #endif

        EasyTextPen pen = { other.mouse_Pixel + V2(12.0f, 16.0f), 12.0f, color, true, 1.0f - preview->cursor_subtext_alpha };
        easy_text_draw(&pen, Top_string);
        easy_text_draw(&pen, Bot_string);
    }

    void history_debug_draw(); // forward declaration


#if 0
    if (other.show_help) {
        eso_begin(M4_Identity(), SOUP_QUADS); {
            eso_overlay(true);
            eso_color(pallete.black, 0.7f);
            eso_vertex(-1.0f, -1.0f);
            eso_vertex(-1.0f,  1.0f);
            eso_vertex( 1.0f,  1.0f);
            eso_vertex( 1.0f, -1.0f);
        } eso_end();

        auto command_to_string = [](Command command) -> char* {

            bool control = command.mods & MOD_CTRL;
            bool shift = command.mods & MOD_SHIFT;
            bool alt = command.mods & MOD_ALT;
            KeyEvent tmp = { {}, command.key, control, shift, alt }; 

            return key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(&tmp);
        };
        EasyTextPen pen1 = { V2(25.0f, 16.0f), 16.0f, pallete.white, true}; // FORNOW
        #define PRINT_COMMAND(PEN, NAME) \
        easy_text_drawf(PEN, "  %s: %s", #NAME, \
                command_to_string(commands.NAME));
        EasyTextPen pen2 = pen1;
        pen2.origin.x += 450.0f;


        //////////////////////////////////////////
        //////  SNAP COMMANDS  ///////////////////
        //////////////////////////////////////////

        easy_text_drawf(&pen1, "SNAP COMMANDS\n");
        PRINT_COMMAND(&pen1, Center);
        PRINT_COMMAND(&pen1, End);
        PRINT_COMMAND(&pen1, Middle);
        PRINT_COMMAND(&pen1, Perp);
        PRINT_COMMAND(&pen1, Quad);
        PRINT_COMMAND(&pen1, XY);
        PRINT_COMMAND(&pen1, Zero);


        //////////////////////////////////////////
        //////  Select COMMANDS  /////////////////
        //////////////////////////////////////////

        easy_text_drawf(&pen1, "\nSELECT COMMANDS\n");
        PRINT_COMMAND(&pen1, SetColor);
        PRINT_COMMAND(&pen1, Connected);
        PRINT_COMMAND(&pen1, All);
        PRINT_COMMAND(&pen1, Window);


        //////////////////////////////////////////
        //////  OTHER COMMANDS  //////////////////
        //////////////////////////////////////////

        easy_text_drawf(&pen1, "\nOTHER COMMANDS\n");
        PRINT_COMMAND(&pen1, SetAxis);
        PRINT_COMMAND(&pen1, Box);
        PRINT_COMMAND(&pen1, SetOrigin);
        PRINT_COMMAND(&pen1, Circle);
        PRINT_COMMAND(&pen1, ClearDrawing);
        PRINT_COMMAND(&pen1, ClearMesh);
        PRINT_COMMAND(&pen1, CyclePlane);
        PRINT_COMMAND(&pen1, DELETE_SELECTED); // TODO
        PRINT_COMMAND(&pen1, DELETE_SELECTED_ALTERNATE); //TODO
        PRINT_COMMAND(&pen1, Deselect);
        PRINT_COMMAND(&pen1, DIVIDE_NEAREST);
        PRINT_COMMAND(&pen1, ZoomDrawing);
        PRINT_COMMAND(&pen1, Escape); // TODO
        PRINT_COMMAND(&pen1, ExtrudeAdd);
        PRINT_COMMAND(&pen1, ExtrudeCut);
        PRINT_COMMAND(&pen1, Fillet);
        PRINT_COMMAND(&pen1, HELP_MENU);
        PRINT_COMMAND(&pen1, Line);
        PRINT_COMMAND(&pen1, Copy);
        PRINT_COMMAND(&pen1, Measure);
        PRINT_COMMAND(&pen1, Mirror2);
        PRINT_COMMAND(&pen1, MirrorX);
        PRINT_COMMAND(&pen1, MirrorY);
        PRINT_COMMAND(&pen2, Move);
        PRINT_COMMAND(&pen2, NEXT_POPUP_BAR);
        PRINT_COMMAND(&pen2, NudgePlane);
        PRINT_COMMAND(&pen2, Offset);
        PRINT_COMMAND(&pen1, OpenDXF);
        PRINT_COMMAND(&pen1, OpenSTL);
        PRINT_COMMAND(&pen2, Polygon);
        PRINT_COMMAND(&pen2, POWER_FILLET);
        PRINT_COMMAND(&pen2, PREVIOUS_HOT_KEY_2D);
        PRINT_COMMAND(&pen2, PREVIOUS_HOT_KEY_3D);
        PRINT_COMMAND(&pen2, PRINT_HISTORY);
        PRINT_COMMAND(&pen2, Color0);
        PRINT_COMMAND(&pen2, Color1);
        PRINT_COMMAND(&pen2, Color2);
        PRINT_COMMAND(&pen2, Color3);
        PRINT_COMMAND(&pen2, Color4);
        PRINT_COMMAND(&pen2, Color5);
        PRINT_COMMAND(&pen2, Color6);
        PRINT_COMMAND(&pen2, Color7);
        PRINT_COMMAND(&pen2, Color8);
        PRINT_COMMAND(&pen2, Color9);
        PRINT_COMMAND(&pen2, Redo);
        PRINT_COMMAND(&pen2, REDO_ALTERNATE);
        PRINT_COMMAND(&pen2, Scale);
        PRINT_COMMAND(&pen2, RevolveAdd);
        PRINT_COMMAND(&pen2, RevolveCut);
        PRINT_COMMAND(&pen2, Rotate);
        PRINT_COMMAND(&pen2, RCopy);
        PRINT_COMMAND(&pen2, SaveDXF);
        PRINT_COMMAND(&pen2, SaveSTL);
        PRINT_COMMAND(&pen2, Select);
        PRINT_COMMAND(&pen2, TOGGLE_BUTTONS);
        PRINT_COMMAND(&pen2, TOGGLE_DRAWING_DETAILS);
        PRINT_COMMAND(&pen2, TOGGLE_EVENT_STACK);
        PRINT_COMMAND(&pen2, HidePlane);
        PRINT_COMMAND(&pen2, TOGGLE_GRID);
        PRINT_COMMAND(&pen2, TOGGLE_LIGHT_MODE);
        PRINT_COMMAND(&pen2, Divide2);
        PRINT_COMMAND(&pen2, DiamCircle);
        PRINT_COMMAND(&pen2, Undo);
        PRINT_COMMAND(&pen2, UNDO_ALTERNATE);
        PRINT_COMMAND(&pen2, ZoomMesh);
    }
#endif

    if (other.show_event_stack) history_debug_draw();

    { // paused; slowmo
        real x = 12.0f;
        real y = window_get_height_Pixel() - 12.0f;
        real w = 6.0f;
        real h = 2.5f * w;
        if (other.paused) {
            eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
            eso_overlay(true);
            eso_color(pallete.green);
            for_(i, 2) {
                real o = i * (1.7f * w);
                eso_vertex(x     + o, y    );
                eso_vertex(x     + o, y - h);
                eso_vertex(x + w + o, y - h);
                eso_vertex(x + w + o, y    );
            }
            eso_end();
        }
        if (other.slowmo) {
            eso_begin(other.OpenGL_from_Pixel, SOUP_TRIANGLES);
            eso_overlay(true);
            eso_color(pallete.yellow);
            {
                eso_vertex(x    , y - h);
                eso_vertex(x    , y    );
                eso_vertex(x + h, y    );
            }
            eso_end();
        }
    }

    { // details
        uint num_lines;
        uint num_arcs;
        uint num_circles;
        {
            num_lines = 0;
            num_arcs = 0;
            num_circles = 0;
            _for_each_entity_ {
                if (entity->type == EntityType::Line) {
                    ++num_lines;
                } else if (entity->type == EntityType::Arc) {
                    ++num_arcs;
                } else { ASSERT(entity->type == EntityType::Circle);
                    ++num_circles;
                }
            }
        }

        real height = 12.0f;
        EasyTextPen pen = { V2(96.0f, window_get_height_Pixel() - 13.0f), height, 0.5f * get_accent_color(ToolboxGroup::Draw) };
        easy_text_drawf(&pen, "%d lines %d arcs %d circles", num_lines, num_arcs, num_circles);
        pen = { V2(get_x_divider_drawing_mesh_Pixel() + 7.0f, window_get_height_Pixel() - 13.0f), height, 0.5f * get_accent_color(ToolboxGroup::Mesh) };
        easy_text_drawf(&pen, "%d triangles", mesh->num_triangles);
    }

}

