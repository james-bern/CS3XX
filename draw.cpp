// // FUN PROJECTS FOR NATE and JIM
// TODO: each entity could have a target_color and current_color (just for drawing) -- BBPR and all that jazz could be replaced by a massive
//       block that determines the entities target_color
// TODO: divide on snap is deselecting even for End snap (End snap should never ever divide)
// TODO: RevolveAdd and RevolveCut need to start at 360
// TODO: cooldown on bbpr restore so less flickery 
// TODO: select window preview
// TODO: select connected preview
// TODO: Translate, Rotate, Scale etc. should refuse to activate if nothing is selected (their buttons should also be grayed out)
// TODO: properly stipple arcs by passing center and radius to shader ( theta(L_stipple, r) )
// TODO: attractive sparkly swoosh of light (see olllld commits) while extruding
// TODO: undo and redo buttons (where to put these? -- i think along the top in a separate pane)
// TODO: press TAB to switch from Box to CenterBox
// TODO: make BLUE not show up until the user has keyed something into the draw Enter popup
// TODO: PINK endpoint snaps should draw a big pink dot
// TODO: z-sort the feature plane and grid (probably after breaking the feature plane into two pieces oh god wai)
// XXXX: highlight 3D faces before clicking to spawn plane
// XXXX: feature plane tween needs work when clicking on mesh

// // TODO: (Jim) stuff for alpha
// TODO: fix in/out relationship (right now they just seem to add)
// TODO: tubes
// TODO: manifold_wrapper_wrapper
// XXX: fix origin axis relationship with revolve
// XXX: - tubes
// XXX: - manifold_wrapper_wrapper
// TODO: draw axis on RHS when revolving
// TODO: 3D-picking is broken for non xyz planes
// TODO: revisit extruded cut on the botton of box with name (why did the students need to flip their names)

mat4 get_M_3D_from_2D(bool for_drawing = false) {
    vec3 up = { 0.0f, 1.0f, 0.0f };
    real dot_product = dot(feature_plane->normal, up);
    vec3 down = (ARE_EQUAL(ABS(dot_product), 1.0f)) ? V3(0.0f, 0.0f, 1.0f * SGN(dot_product)) : V3(0.0f, -1.0f, 0.0f);

    vec3 z = feature_plane->normal;
    vec3 x = normalized(cross(z, down));
    vec3 y = cross(z, x);
    vec3 o = {};

    mat4 M_2D = M4_Identity();

    if (for_drawing) {
        M_2D *= M4_Translation(-preview->drawing_origin);

        o += preview->feature_plane_signed_distance_to_world_origin * feature_plane->normal;

        // TODO: move sketch and rotate sketch goes in here too
        x = transformVector(M4_RotationAbout(y, -preview->feature_plane_mirror_x_angle), x);
        y = transformVector(M4_RotationAbout(x, preview->feature_plane_mirror_y_angle), y);
        o += preview->feature_plane_mirror_XXX_bump * feature_plane->normal;

        mat4 R = M4_RotationAbout(z, preview->feature_plane_rotation_angle);
        x = transformVector(R, x);
        y = transformVector(R, y);
    } else {
        M_2D *= M4_Translation(-drawing->origin);

        o += (feature_plane->signed_distance_to_world_origin) * feature_plane->normal;

        if (feature_plane->mirror_x) x *= -1;
        if (feature_plane->mirror_y) y *= -1;

        mat4 R = M4_RotationAbout(z, feature_plane->rotation_angle);
        x = transformVector(R, x);
        y = transformVector(R, y);
    }

    return M4_xyzo(x, y, z, o) * M_2D;
}


void conversation_draw() {

    // FORNOW: here
    for_(i, ARRAY_LENGTH(pallete->_data)) {
        JUICEIT_EASYTWEEN(&pallete->_data[i], target_pallete->_data[i]);
    }

    mat4 P_2D = camera_drawing->get_P();
    mat4 V_2D = camera_drawing->get_V();
    mat4 PV_2D = P_2D * V_2D;
    mat4 inv_PV_2D = inverse(PV_2D);
    vec2 mouse_World_2D = transformPoint(inv_PV_2D, other.mouse_OpenGL);
    mat4 M_3D_from_2D = get_M_3D_from_2D(true);
    mat4 P_3D = camera_mesh->get_P();
    mat4 V_3D = camera_mesh->get_V();
    mat4 PV_3D = P_3D * V_3D;
    mat4 World_3D_from_OpenGL = inverse(PV_3D);

    TransformMouseDrawingPositionResult mouse_no_snap_potentially_15_deg__WHITE = transform_mouse_drawing_position(mouse_World_2D, other.shift_held, true );
    TransformMouseDrawingPositionResult mouse_transformed__PINK                = transform_mouse_drawing_position(mouse_World_2D, other.shift_held, false);
    JUICEIT_EASYTWEEN(&preview->mouse_no_snap_potentially_15_deg__WHITE_position, mouse_no_snap_potentially_15_deg__WHITE.mouse_position);

    MagicSnapResult3D mouse_snap_result_3D; {
        vec3 mouse_ray_origin = transformPoint(World_3D_from_OpenGL, V3(other.mouse_OpenGL, -1.0f));
        vec3 mouse_ray_end = transformPoint(World_3D_from_OpenGL, V3(other.mouse_OpenGL,  1.0f));
        vec3 mouse_ray_direction = normalized(mouse_ray_end - mouse_ray_origin);
        mouse_snap_result_3D = magic_snap_raycast(mouse_ray_origin, mouse_ray_direction);
    }

    { // FORNOW: hover_plane TODO (this conversion should be a function of something -- or honestly just included in the result)
        WorkMesh *mesh = &meshes->work;
        if (mouse_snap_result_3D.hit_mesh) {
            hover_plane->is_active = true;
            hover_plane->normal = mesh->triangle_normals[mouse_snap_result_3D.triangle_index];
            hover_plane->signed_distance_to_world_origin = dot(hover_plane->normal, mouse_snap_result_3D.mouse_position);
        } else {
            hover_plane->is_active = false;
        }
    }

    bool extruding = ((state_Mesh_command_is_(ExtrudeAdd)) || (state_Mesh_command_is_(ExtrudeCut)));
    bool revolving = ((state_Mesh_command_is_(RevolveAdd)) || (state_Mesh_command_is_(RevolveCut)));
    bool adding     = ((state_Mesh_command_is_(ExtrudeAdd)) || (state_Mesh_command_is_(RevolveAdd)));
    bool cutting     = ((state_Mesh_command_is_(ExtrudeCut)) || (state_Mesh_command_is_(RevolveCut)));

    { // Tweening
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

        // TODO: Make fancy
        { // feature plane
            { // preview->feature_plane_signed_distance_to_world_origin
                real target = feature_plane->signed_distance_to_world_origin;
                if (state_Mesh_command_is_(NudgePlane)) target += popup->feature_plane_nudge;
                JUICEIT_EASYTWEEN(&preview->feature_plane_signed_distance_to_world_origin, target);
            }
            { // preview->feature_plane_mirror_x_angle, preview->feature_plane_mirror_y_angle, preview->feature_plane_mirror_XXX_bump
                JUICEIT_EASYTWEEN(&preview->feature_plane_mirror_x_angle, feature_plane->mirror_x ? PI : 0.0f);
                JUICEIT_EASYTWEEN(&preview->feature_plane_mirror_y_angle, feature_plane->mirror_y ? PI : 0.0f);
                // TODO: Change 10 to function of feature_plane size
                JUICEIT_EASYTWEEN(&preview->feature_plane_mirror_XXX_bump, 10 * MAX(SIN(preview->feature_plane_mirror_x_angle), SIN(preview->feature_plane_mirror_y_angle)));
            }
            { // preview->feature_plane_rotation_angle
                real target = feature_plane->rotation_angle;
                if (state_Mesh_command_is_(RotatePlane))
                    target += RAD(popup->feature_plane_rotate_plane_angle);
                JUICEIT_EASYTWEEN(&preview->feature_plane_rotation_angle, target);
            }
            { // preview->drawing_origin
                vec2 target = (!state_Draw_command_is_(SetOrigin)) ? drawing->origin : mouse_transformed__PINK.mouse_position;
                JUICEIT_EASYTWEEN(&preview->drawing_origin, target);
            }
        }
    }

    // TODO: lerp
    vec2 preview_dxf_axis_base_point;
    real preview_dxf_axis_angle_from_y;
    {
        if (!state_Draw_command_is_(SetAxis)) {
            preview_dxf_axis_base_point = drawing->axis_base_point;
            preview_dxf_axis_angle_from_y = drawing->axis_angle_from_y;
        } else if (!two_click_command->awaiting_second_click) {
            preview_dxf_axis_base_point = mouse_no_snap_potentially_15_deg__WHITE.mouse_position;
            preview_dxf_axis_angle_from_y = drawing->axis_angle_from_y;
        } else {
            preview_dxf_axis_base_point = two_click_command->first_click;
            preview_dxf_axis_angle_from_y = ATAN2(mouse_no_snap_potentially_15_deg__WHITE.mouse_position - preview_dxf_axis_base_point) - PI / 2;
        }
    }

    uint window_width, window_height; {
        vec2 _window_size = window_get_size_Pixel();
        window_width = uint(_window_size.x);
        window_height = uint(_window_size.y);
    }

    real x_divider_drawing_mesh_Pixel = get_x_divider_drawing_mesh_Pixel();

    { // drawing mesh panes
        bool dragging = (other.mouse_left_drag_pane == Pane::Separator);
        bool hovering = ((other.mouse_left_drag_pane == Pane::None) && (other.hot_pane == Pane::Separator));
        eso_begin(M4_Identity(), SOUP_LINES);
        // eso_overlay(true);
        real f = (pallete_2D->dark_light_tween + pallete_3D->dark_light_tween) / 2;
        eso_size(dragging ? 1.0f
                : hovering ? 2.0f
                : 1.5f);
        eso_color(
                LERP(f,
                    hovering || dragging ? basic.dark_gray : basic.darker_gray,
                    hovering || dragging ? basic.light_gray : basic.lighter_gray
                    )
                );
        eso_vertex(other.x_divider_drawing_mesh_OpenGL, -1.0f);
        eso_vertex(other.x_divider_drawing_mesh_OpenGL,  1.0f);
        eso_end();
    }

    { // 3D draw 3D 3d draw 3d
        {
            glEnable(GL_SCISSOR_TEST);
            gl_scissor_Pixel(x_divider_drawing_mesh_Pixel, 0, window_width - x_divider_drawing_mesh_Pixel, window_height);
        }

        { // Draw mesh
            real *scale = &preview->tween_extrude_add_scale;
            JUICEIT_EASYTWEEN(scale, 1.0f);
            mat4 M = meshes->M_3D_from_2D * M4_Scaling(1.0f, 1.0f, preview->tween_extrude_add_scale) * inverse(meshes->M_3D_from_2D);
            fancy_draw(P_3D, V_3D, M, &meshes->draw);
        }

        JUICEIT_EASYTWEEN(&preview->feature_plane_alpha, (feature_plane->is_active) ? 0.4f : 0.0f);

        // NOTE this is hacky as hell there's something fancy we can probs do with stentcil buffering the lines
        //      that is more correct than the 35/35 split, but FORNOW this is k
        #if 1
        auto draw_feature_plane = [&]() { // draw feature plane
            glDisable(GL_CULL_FACE); // FORNOW

            // vec2 center = (preview->feature_plane_bbox.max + preview->feature_plane_bbox.min) / 2.0f;
            // mat4 scaling_about_center = M4_Translation(center) * M4_Scaling(f) * M4_Translation(-center);
            eso_begin(PV_3D * M_3D_from_2D * M4_Translation(0.0f, 0.0f, 2 * Z_FIGHT_EPS)/* * scaling_about_center*/, SOUP_QUADS);
            eso_color(pallete_3D->feature_plane, preview->feature_plane_alpha);
            eso_bbox_SOUP_QUADS(preview->feature_plane_bbox);
            eso_end();

            glEnable(GL_CULL_FACE); // FORNOW
        };

        glDisable(GL_DEPTH_TEST);
        draw_feature_plane();
        glEnable(GL_DEPTH_TEST);
        #endif
        #if 0
        if (!other.hide_grid) { // grid 3D grid 3d grid
            JUICEIT_EASYTWEEN(&preview->bbox_min_y, meshes->work.bbox.min.y);
            real r = 0.5f * GRID_SIDE_LENGTH;
            real f; { // backface culling (check sign of rasterized triangle)
                vec2 a = _V2(transformPoint(PV_3D, V3(0.0f, 0.0f, 0.0f)));
                vec2 b = _V2(transformPoint(PV_3D, V3(1.0f, 0.0f, 0.0f)));
                vec2 c = _V2(transformPoint(PV_3D, V3(0.0f, 0.0f, 1.0f)));
                f = cross(normalized(c - a) , normalized(b - a));
            }
            mat4 transform = PV_3D * M4_Translation(0, -2 * Z_FIGHT_EPS, 0);
            eso_begin(transform, SOUP_LINES);
            eso_color(pallete_3D->grid, CLAMPED_LINEAR_REMAP(f, 1.0f, 0.0f, pallete_3D->grid, pallete_3D->background));
            eso_size(1.0f);
            if (f > 0.0) {
                for (uint i = 1; i <= uint(GRID_SIDE_LENGTH / GRID_SPACING) - 1; ++i) {
                    real tmp = i * GRID_SPACING;
                    eso_vertex(-r + tmp,              preview->bbox_min_y, -r + 0.0f);
                    eso_vertex(-r + tmp,              preview->bbox_min_y, -r + GRID_SIDE_LENGTH);
                    eso_vertex(-r + 0.0f,             preview->bbox_min_y, -r + tmp);
                    eso_vertex(-r + GRID_SIDE_LENGTH, preview->bbox_min_y, -r + tmp);
                }
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
        #endif
        draw_feature_plane();

        if (feature_plane->is_active) { // selection 2d selection 2D selection tube tubes slice slices stack stacks wire wireframe wires frame (FORNOW: ew) cage cageit
            ;
            // FORNOW
            bool moving_stuff = ((state_Draw_command_is_(SetOrigin)) || (state_Mesh_command_is_(NudgePlane)));
            vec3 target_preview_tubes_color = (0) ? V3(0)
                : (moving_stuff) ? pallete_2D->emphasis
                : (adding) ? V3(0.0f, 1.0f, 0.0f)
                : (cutting) ? V3(1.0f, 0.0f, 0.0f)
                : V3(0);
            JUICEIT_EASYTWEEN(&preview->tubes_color, target_preview_tubes_color);

            #if 1
            // mat4 T_o = M4_Translation(preview->drawing_origin);
            // mat4 inv_T_o = inverse(T_o);
            glDisable(GL_DEPTH_TEST);
            if (extruding) {
                eso_begin(PV_3D * M_3D_from_2D, SOUP_LINES); 
                eso_overlay(true);
                _for_each_selected_entity_ {
                    real alpha;
                    vec3 color;
                    alpha = CLAMP(1.0f * MIN(entity->time_since_is_selected_changed, other.time_since_plane_selected), 0.0f, 1.0f); // TODO:preview->alpha
                    color = preview->tubes_color;
                    eso_color(color, alpha);
                    eso_entity__SOUP_LINES(entity, extruding, preview->extrude_out_length, -preview->extrude_in_length);
                } 
                eso_end();
            }

            if (revolving) {
                real a = -RAD(preview->revolve_out_angle);
                real b = RAD(preview->revolve_in_angle);
                real L = b - a;
                uint NUM_TUBE_STACKS_INCLUSIVE = MIN(64U, uint(ROUND(L / 0.1f)) + 2);
                vec3 axis = V3(e_theta(PI / 2 + preview_dxf_axis_angle_from_y), 0.0f);
                mat4 R_0 = M4_RotationAbout(axis, a);
                mat4 R_inc = M4_RotationAbout(axis, L / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                mat4 T_a = M4_Translation(V3(preview_dxf_axis_base_point, 0.0f));
                mat4 inv_T_a = inverse(T_a);
                // M_incr = T_o * T_a * R_a * inv_T_a * inv_T_o;
                mat4 M_incr = T_a * R_inc * inv_T_a;
                mat4 M = M_3D_from_2D * T_a * R_0 * inv_T_a;
                for_(tube_stack_index, NUM_TUBE_STACKS_INCLUSIVE) {
                    eso_begin(PV_3D * M, SOUP_LINES); {
                        eso_overlay(true);
                        _for_each_selected_entity_ {
                            real alpha;
                            vec3 color;
                            // if (entity->is_selected) {
                            alpha = CLAMP(-0.2f + 3.0f * MIN(entity->time_since_is_selected_changed, other.time_since_plane_selected), 0.0f, 1.0f);
                            color = CLAMPED_LERP(-0.5f + SQRT(2.0f * entity->time_since_is_selected_changed), pallete_3D->background, preview->tubes_color);
                            eso_color(color, alpha);
                            eso_entity__SOUP_LINES(entity);
                        }
                        } eso_end();
                        M *= M_incr;
                    }
            }
            glEnable(GL_DEPTH_TEST);
            #else
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
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0, 0, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else { // default
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0, 0, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                }

                mat4 T_Move;
                if (moving_selected_entities) {
                    T_Move = M4_Translation(preview->mouse_no_snap_potentially_15_deg__WHITE_position - two_click_command->first_click);
                } else {
                    T_Move = M4_Identity();
                }
                for_(tube_stack_index, NUM_TUBE_STACKS_INCLUSIVE) {
                    eso_begin(PV_3D * M * T_Move, SOUP_LINES); {
                        eso_overlay(true);
                        _for_each_selected_entity_ {
                            real alpha;
                            vec3 color;
                            alpha = CLAMP(-0.2f + 3.0f * MIN(entity->time_since_is_selected_changed, other.time_since_plane_selected), 0.0f, 1.0f);
                            color = CLAMPED_LERP(-0.5f + SQRT(2.0f * entity->time_since_is_selected_changed), pallete.black, preview->tubes_color);
                            eso_color(color, alpha);
                            eso_entity__SOUP_LINES(entity);
                        }
                    } eso_end();
                    M *= M_incr;
                }
            }
            #endif
        }

        if (0 && fornow_global_selection_triangle_tuples) { // selection triangulation 3d 3D
            eso_begin(PV_3D * M_3D_from_2D, SOUP_TRIANGLES);
            eso_overlay(true);
            eso_color(0.0f, 1.0f, 0.0f, 0.5f);
            for_(triangle_index, fornow_global_selection_num_triangles) {
                uint3 tuple = fornow_global_selection_triangle_tuples[triangle_index];
                for_(d, 3) {
                    eso_vertex(fornow_global_selection_vertex_positions[tuple[d]]);
                }
            }
            eso_end();

        };

        if (!mesh_two_click_command->awaiting_second_click) {}
        else if (state_Mesh_command_is_(Measure3D)) {
            eso_begin(PV_3D, SOUP_POINTS);
            eso_overlay(true);
            eso_size(20);
            eso_color(basic.white);
            eso_vertex(mesh_two_click_command->first_click);
            eso_end();

            if (mouse_snap_result_3D.hit_mesh) {
                eso_begin(PV_3D, SOUP_LINES);
                eso_color(basic.white);
                eso_vertex(mesh_two_click_command->first_click);
                eso_vertex(mouse_snap_result_3D.mouse_position);
                eso_end();
            }
        }

        {
            if (0) { // world origin 3D
                glDisable(GL_DEPTH_TEST);
                eso_begin(PV_3D, SOUP_POINTS);
                eso_overlay(true);
                eso_size(4.0f);
                eso_color(0.0f, 0.0f, 0.0f, 0.1f);
                eso_vertex(0.0f, 0.0f, 0.0f);
                eso_end();
            }

            { // axes 3D axes 3d axes axis 3D axis 3d axis
                real r = 2 * other.camera_mesh.ortho_screen_height_World / 100.0f;
                mat4 transform = PV_3D * M_3D_from_2D * M4_Translation(0.0f, 0.0f, 3 * Z_FIGHT_EPS);

                if (feature_plane->is_active) {
                    eso_begin(transform, SOUP_LINES);
                    eso_overlay(true);
                    eso_size(2.0f);
                    eso_color(1.0f, 0.0f, 0.0f);
                    eso_vertex(0.0f, 0.0f);
                    eso_vertex(r, 0.0f);
                    eso_color(0.0f, 1.0f, 0.0f);
                    eso_vertex(0.0f, 0.0f);
                    eso_vertex(0.0f, r);
                    eso_end();
                } else {
                    // eso_begin(transform, SOUP_POINTS);
                    // eso_overlay(true);
                    // eso_color(color);
                    // eso_size(5.0f);
                    // eso_vertex(0.0f, 0.0f);
                    // eso_end();
                }
            }
            glEnable(GL_DEPTH_TEST);
        }

        glDisable(GL_SCISSOR_TEST);
    }



    { // draw 2D draw 2d draw
        {
            glEnable(GL_SCISSOR_TEST);
            gl_scissor_Pixel(0, 0, x_divider_drawing_mesh_Pixel, window_height);
        }
        {
            if (!other.hide_grid) { // grid 2D grid 2d grid
                mat4 PVM = PV_2D * M4_Translation(-GRID_SIDE_LENGTH / 2, -GRID_SIDE_LENGTH / 2);
                eso_begin(PVM, SOUP_LINES);
                eso_color(pallete_2D->grid);
                for (uint i = 0; i <= uint(GRID_SIDE_LENGTH / GRID_SPACING); ++i) {
                    real tmp = i * GRID_SPACING;
                    eso_vertex(tmp, 0.0f);
                    eso_vertex(tmp, GRID_SIDE_LENGTH);
                    eso_vertex(0.0f, tmp);
                    eso_vertex(GRID_SIDE_LENGTH, tmp);
                }
                eso_end();
                eso_begin(PVM, SOUP_LINE_LOOP);
                eso_color(pallete_2D->grid_accent);
                eso_vertex(0.0f, 0.0f);
                eso_vertex(0.0f, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, 0.0f);
                eso_end();
            }
            if (1) { // axes 2D axes 2d axes axis 2D axis 2d axes origin 2d origin 2D origin
                real funky_OpenGL_factor = other.camera_drawing.ortho_screen_height_World / 120.0f;
                real r = 3 * funky_OpenGL_factor;
                // real LL = 1000 * funky_OpenGL_factor;

                eso_begin(PV_2D, SOUP_POINTS); {
                    eso_overlay(true);
                    eso_color(pallete_2D->axis);
                    eso_size(6.0f);
                    eso_vertex(preview->drawing_origin);
                } eso_end();
                vec2 v = r * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                eso_begin(PV_2D, SOUP_LINES); {
                    eso_overlay(true);
                    eso_color(pallete_2D->axis);
                    eso_size(3.0f);
                    eso_vertex(preview_dxf_axis_base_point);
                    eso_size(0.0f);
                    eso_vertex(preview_dxf_axis_base_point + v);
                } eso_end();
            }

            { // tween snap values
                vec2 Draw_Enter; {
                    Draw_Enter = (!two_click_command->awaiting_second_click) ? V2(0, 0) : two_click_command->first_click;
                    if (state_Draw_command_is_(Box)) Draw_Enter += V2(popup->box_width, popup->box_height);
                    if (state_Draw_command_is_(Circle)) Draw_Enter += V2(popup->circle_radius, 0.0f);
                    if (state_Draw_command_is_(Polygon)) Draw_Enter += V2(popup->polygon_distance_to_corner, 0.0f);
                    if (state_Draw_command_is_(Line)) Draw_Enter += V2(popup->line_run, popup->line_rise);
                    if (state_Draw_command_is_(Translate)) Draw_Enter += V2(popup->move_run, popup->move_rise);
                    if (state_Draw_command_is_(LCopy)) Draw_Enter += V2(popup->lcopy_run, popup->lcopy_rise);
                    if (state_Draw_command_is_(Rotate)) Draw_Enter += 10.0f * e_theta(RAD(popup->rotate_angle));
                }

                vec2 Snap_Enter; {
                    Snap_Enter = two_click_command->first_click;
                    if (state_Snap_command_is_(XY)) {
                        Snap_Enter = V2(popup->xy_x_coordinate, popup->xy_y_coordinate);
                    }
                }

                {
                    JUICEIT_EASYTWEEN(&preview->mouse_from_Draw_Enter__BLUE_position, Draw_Enter);
                    JUICEIT_EASYTWEEN(&preview->xy_xy, Snap_Enter);

                    JUICEIT_EASYTWEEN(&preview->mouse_transformed__PINK_position, mouse_transformed__PINK.mouse_position, 1.5f);

                    JUICEIT_EASYTWEEN(&preview->polygon_num_sides, real(popup->polygon_num_sides));

                    JUICEIT_EASYTWEEN(&preview->scale_factor, popup->scale_factor);

                    JUICEIT_EASYTWEEN(&preview->rcopy_last_angle, TAU - TAU / MAX(1, popup->rcopy_num_total_copies));

                    JUICEIT_EASYTWEEN(&preview->lcopy_run, popup->lcopy_run);
                    JUICEIT_EASYTWEEN(&preview->lcopy_rise, popup->lcopy_rise);
                    JUICEIT_EASYTWEEN(&preview->lcopy_num_additional_copies, real(popup->lcopy_num_additional_copies));
                }
            }

            bool Snap_eating_mouse = !(state_Snap_command_is_(None) || state_Snap_command_is_(XY));
            // bool Draw_eating_Enter = ((popup->manager.focus_group == ToolboxGroup::Draw) &&
            //         (!two_click_command->awaiting_second_click || !ARE_EQUAL(*first_click, Draw_Enter)));
            // bool Snap_eating_Enter = ((popup->manager.focus_group == ToolboxGroup::Snap) && state_Snap_command_is_(XY) &&
            //         (!two_click_command->awaiting_second_click || !ARE_EQUAL(*first_click, Snap_Enter)));

            { // CHOWDER (LINES and POINTS -- entities, annotations)

                // // NOTE: Currently just shimming this API with eso; TODO: implement properly with stew
                // //       NOTE: it's an imperfect shim with the way size, color are carried over, but is close enough

                // BEGIN shimmed API

                static struct {
                    mat4 PV;
                    mat4 M;
                    u8   primitive;
                    real size;
                    vec3 color;
                    bool stipple;

                    bool began;
                } _chowder;

                auto _CHOWDER_CYCLE_ESO = [&]() {
                    eso_end();
                    eso_begin(_chowder.PV * _chowder.M, _chowder.primitive);
                    eso_color(_chowder.color);
                    eso_size(_chowder.size);
                    eso_stipple(_chowder.stipple);
                };

                auto chowder_set_PV = [&](mat4 PV) {
                    _chowder.PV = PV;
                    _CHOWDER_CYCLE_ESO();
                };
                auto chowder_reset_PV = [&]() { chowder_set_PV(M4_Identity()); };
                FORNOW_UNUSED(chowder_reset_PV);

                auto chowder_set_M = [&](mat4 M) {
                    _chowder.M = M;
                    _CHOWDER_CYCLE_ESO();
                };
                auto chowder_reset_M = [&]() { chowder_set_M(M4_Identity()); };

                auto chowder_set_primitive = [&](u8 primitive) {
                    _chowder.primitive = primitive;
                    _CHOWDER_CYCLE_ESO();
                };
                auto chowder_reset_primitive = [&]() { chowder_set_primitive(SOUP_LINES); };

                auto chowder_set_color = [&](vec3 color) {
                    _chowder.color = color;
                    eso_color(color);
                };

                auto chowder_set_size = [&](real size) {
                    _chowder.size = size;
                    eso_size(size);
                };
                auto chowder_reset_size = [&]() { chowder_set_size(1.0f); };

                auto chowder_set_stipple = [&](bool stipple) {
                    _chowder.stipple = stipple;
                    _CHOWDER_CYCLE_ESO();
                };
                auto chowder_reset_stipple = [&]() { chowder_set_stipple(false); };


                auto chowder_begin = [&]() {
                    ASSERT(!_chowder.began);
                    _chowder.began = true;
                    eso_begin(M4_Identity(), SOUP_LINES); // FORNOW: just a dummy
                };

                auto chowder_end = []() {
                    ASSERT(_chowder.began);
                    _chowder.began = false;
                    eso_end();
                };

                auto chowder_entity = [&](Entity *entity) { eso_entity__SOUP_LINES(entity); };
                auto chowder_set_color_AND_chower_entity = [&](Entity *entity) {
                    chowder_set_color(get_color_from_color_code(entity->color_code));
                    chowder_entity(entity);
                };
                auto chowder_vertex = [](vec2 p) { eso_vertex(p); };
                auto chowder_vertex2 = [](real x, real y) { eso_vertex(x, y); };
                // END shimmed API


                bool very_special_boolean = 
                    (0
                     || state_Draw_command_is_(Translate)
                     || state_Draw_command_is_(Rotate)
                     // NOTE: TODOLATER when XMirror and YMirror have toggle button to erase original entities
                    );


                auto render_2d = [&](mat4 PV, mat4 base_M, bool is_feature_plane_pass) {
                    chowder_begin();
                    chowder_set_primitive(SOUP_LINES);
                    chowder_reset_size();
                    chowder_set_PV(PV);
                    chowder_set_M(base_M);
                    bbox2 drawn_entities_bbox = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
                    
                    { // entities
                        _for_each_entity_ {
                            if (!(entity->is_selected && very_special_boolean) && !is_feature_plane_pass) {
                                chowder_set_color_AND_chower_entity(entity);
                            }
                        }

                        if (other.show_details_2D) { // dots
                            chowder_set_size(2.0f);
                            chowder_set_primitive(SOUP_POINTS);
                            chowder_set_color(pallete_2D->dots);
                            _for_each_entity_ {
                                if (0
                                        || state_Draw_command_is_(Translate)
                                        || state_Draw_command_is_(Rotate)
                                   ) continue;

                                if (entity->type == EntityType::Circle) {
                                    CircleEntity *circle = &entity->circle;
                                    if (circle->has_pseudo_point) eso_vertex(circle->get_pseudo_point());
                                    continue;
                                }
                                chowder_vertex(entity_get_start_point(entity));
                                chowder_vertex(entity_get_end_point(entity));
                            }
                            chowder_reset_primitive();
                            chowder_reset_size();
                        }
                    }

                    { // gray underlay
                        chowder_set_stipple(true);
                        _for_each_entity_ {
                            if (entity->is_selected && very_special_boolean) {
                                chowder_set_color(pallete_2D->drawing_underlay);
                                chowder_entity(entity);
                                drawn_entities_bbox += entity_get_bbox(entity);
                            }
                        }
                        chowder_reset_stipple();
                    }

                    // FORNOW very sus
                    static real bbpr_alpha;
                    bool bbpr = 0
                        || state_Draw_command_is_(Fillet)
                        || state_Draw_command_is_(DogEar)
                        || state_Draw_command_is_(Divide2)
                        || state_Draw_command_is_(Offset)
                        || state_Draw_command_is_(Select)
                        || state_Draw_command_is_(Deselect)
                        || state_Draw_command_is_(Rotate)
                        || state_Draw_command_is_(Translate)
                        || state_Draw_command_is_(Scale)
                        || state_Draw_command_is_(RCopy)
                        || state_Draw_command_is_(LCopy)
                        || state_Draw_command_is_(XMirror)
                        || state_Draw_command_is_(YMirror)
                        || (!state_Snap_command_is_(None))
                        ;

                    if (!is_feature_plane_pass) { // BBPR big bad panic rectangle bbpr
                        JUICEIT_EASYTWEEN(&bbpr_alpha, (bbpr ? 0.60f : 0.0f), 0);//0.5f);
                        chowder_end();
                        glDepthMask(GL_FALSE);
                        eso_begin(M4_Identity(), SOUP_TRIANGLES);
                        eso_color(pallete_2D->background, bbpr_alpha);
                        eso_vertex(-1.0f, -1.0f);
                        eso_vertex( 1.0f, -1.0f);
                        eso_vertex( 1.0f,  1.0f);

                        eso_vertex(-1.0f, -1.0f);
                        eso_vertex( 1.0f,  1.0f);
                        eso_vertex(-1.0f,  1.0f);
                        eso_end();
                        glDepthMask(GL_TRUE);
                        chowder_begin();
                        _CHOWDER_CYCLE_ESO();
                    };

                    { // FORNOW selected entities white highlight (going on top of the BBPR)
                        if (!very_special_boolean) {
                            _for_each_selected_entity_ {
                                chowder_set_color(pallete_2D->selection);
                                chowder_entity(entity);
                                drawn_entities_bbox += entity_get_bbox(entity);
                            }
                        }
                    }

                    { // annotations (BLUE, WHITE, PINK)
                        vec3 BLUE = pallete_2D->draw;
                        vec3 WHITE = pallete_2D->emphasis;
                        vec3 PINK = pallete_2D->snap;

                        vec2 mouse_WHITE_or_PINK_position__depending_on_whether_snap_is_active;
                        vec3 WHITE_or_PINK_depending_on_whether_snap_is_active;
                        {
                            if (!Snap_eating_mouse) {
                                WHITE_or_PINK_depending_on_whether_snap_is_active = WHITE;
                                mouse_WHITE_or_PINK_position__depending_on_whether_snap_is_active = mouse_no_snap_potentially_15_deg__WHITE.mouse_position;
                                preview->mouse_no_snap_potentially_15_deg__WHITE_position = mouse_WHITE_or_PINK_position__depending_on_whether_snap_is_active; // Important! (this keeps box from center to center corners in splash from looking weird)
                            } else {
                                WHITE_or_PINK_depending_on_whether_snap_is_active = PINK;
                                mouse_WHITE_or_PINK_position__depending_on_whether_snap_is_active = preview->mouse_transformed__PINK_position;
                            }
                        }

                        #define DRAW2D_PASS_Mouse     0 // NOTE: this one goes black after inactivity FORNOW, so leave it on bottom
                        #define DRAW2D_PASS_DrawEnter 1
                        #define DRAW2D_PASS_XY        2
                        for_(pass, 3) {
                            if (
                                    1
                                    && (pass == DRAW2D_PASS_Mouse)
                                    && (very_special_boolean && (!two_click_command->awaiting_second_click)) 
                               ) continue;


                            // bool TODO

                            if ((pass == DRAW2D_PASS_DrawEnter) &&
                                    (
                                     0
                                     || (!popup->manager.is_active(ToolboxGroup::Draw))
                                     || (state.Draw_command.flags & FOCUS_THIEF) // NOTE: works FORNOW, but only incidentally
                                     || (state_Draw_command_is_(None)) // NOTE: works FORNOW, but only incidentally
                                    )
                            ) continue;

                            if ((pass == DRAW2D_PASS_XY) && (!state_Snap_command_is_(XY))) continue;

                            vec3 color;
                            {
                                if (pass == DRAW2D_PASS_DrawEnter) {
                                    color = BLUE;
                                } else if (pass == DRAW2D_PASS_XY) {
                                    color = PINK;
                                } else { ASSERT(pass == DRAW2D_PASS_Mouse);
                                    color = WHITE_or_PINK_depending_on_whether_snap_is_active;
                                    if ((state_Snap_command_is_(None) || state_Snap_command_is_(XY))) {
                                        color = CLAMPED_LERP(_JUICEIT_EASYTWEEN(other.time_since_mouse_moved - 1.3f), color, pallete_2D->inactive_WHITE);
                                    }
                                }
                            }

                            vec2 click_1;
                            union {
                                vec2 click_2;
                                vec2 crosshair;
                            };
                            {
                                if (two_click_command->awaiting_second_click) {
                                    click_1 = two_click_command->first_click;
                                } else { // XXX: two click commands with option for shortcutting first-Enter 

                                    click_1 = {}; // NOTE: this is an assumption ("logic"/assumption is repeated in process on Draw_Enter)

                                    // Exceptions (repeated logic):
                                    if (state_Draw_command_is_(Scale) && (!two_click_command->awaiting_second_click)) {
                                        bbox2 bbox = entities_get_bbox(&drawing->entities, true);
                                        vec2 bbox_center = AVG(bbox.min, bbox.max);
                                        click_1 = bbox_center;
                                    }

                                }

                                if (pass == DRAW2D_PASS_DrawEnter) {
                                    click_2 = preview->mouse_from_Draw_Enter__BLUE_position;
                                } else if (pass == DRAW2D_PASS_XY) {
                                    click_2 = preview->xy_xy;
                                } else { ASSERT(pass == DRAW2D_PASS_Mouse);
                                    click_2 = mouse_WHITE_or_PINK_position__depending_on_whether_snap_is_active;
                                }
                            }

                            vec2 click_vector = click_2 - click_1;
                            real click_length = norm(click_vector);
                            real click_angle = ATAN2(click_vector);
                            vec2 click_average_click = AVG(click_1, click_2);

                            //

                            chowder_set_color(color);

                            // TODO: Add to bbox ?
                            if (!is_feature_plane_pass) { // vanilla two click commands
                                if (two_click_command->awaiting_second_click) {
                                    if (0) {
                                    } else if (state_Draw_command_is_(Box) || state_Xsel_command_is_(Window)) {
                                        chowder_vertex2(click_1.x, click_1.y);

                                        chowder_vertex2(click_1.x, click_2.y);
                                        chowder_vertex2(click_1.x, click_2.y);

                                        chowder_vertex2(click_2.x, click_2.y);
                                        chowder_vertex2(click_2.x, click_2.y);

                                        chowder_vertex2(click_2.x, click_1.y);
                                        chowder_vertex2(click_2.x, click_1.y);

                                        chowder_vertex2(click_1.x, click_1.y);
                                    } else if (state_Draw_command_is_(Circle)) {
                                        // FORNOW: 2x inefficient
                                        real angle = 0.0f;
                                        real dangle = (TAU / NUM_SEGMENTS_PER_CIRCLE);
                                        for_(i, NUM_SEGMENTS_PER_CIRCLE) {
                                            chowder_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(click_1, click_length, angle));
                                            angle += dangle;
                                            chowder_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(click_1, click_length, angle));
                                        }
                                    } else if (state_Draw_command_is_(Line)) {
                                        chowder_vertex(click_1);
                                        chowder_vertex(click_2);
                                    } else if (state_Draw_command_is_(Polygon)) {
                                        real dangle = -TAU / preview->polygon_num_sides;
                                        real angle = click_angle;

                                        // FORNOW: 2x inefficient
                                        uint num_sides = uint(preview->polygon_num_sides);
                                        for_(i, num_sides) {
                                            chowder_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(click_1, click_length, angle));
                                            angle += dangle;
                                            chowder_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(click_1, click_length, angle));
                                            if (i == (num_sides - 1)) {
                                                chowder_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(click_1, click_length, angle));
                                                chowder_vertex(click_2);
                                            }
                                        }
                                    }
                                }
                            }



                            // TODO: Add to bbox
                            if (!is_feature_plane_pass) { // Fillet, DogEar, Divide2, Join2
                                // TODO: select connected should preview the entire connected selection
                                if (pass == DRAW2D_PASS_Mouse) {
                                    if (1
                                            && state_Draw_command_is_(Select)
                                            && state_Xsel_command_is_(Window)
                                            && two_click_command->awaiting_second_click
                                    ) {
                                        _for_each_entity_ {
                                            if (entity->is_selected) continue;
                                            bbox2 window = {
                                                MIN(click_1.x, click_2.x),
                                                MIN(click_1.y, click_2.y),
                                                MAX(click_1.x, click_2.x),
                                                MAX(click_1.y, click_2.y)
                                            };
                                            if (bbox_contains(window, entity_get_bbox(entity))) chowder_set_color_AND_chower_entity(entity);
                                        }
                                    } else {
                                        if (two_click_command->awaiting_second_click) { // FORNOW
                                            DXFFindClosestEntityResult closest_result = dxf_find_closest_entity(&drawing->entities, mouse_no_snap_potentially_15_deg__WHITE.mouse_position);
                                            if (closest_result.success) { // highlight (TODO: for 3D face as well)
                                                if (
                                                        0
                                                        || (state_Draw_command_is_(Divide2) && (!two_click_command->awaiting_second_click))
                                                        || (state_Draw_command_is_(DogEar) && (!two_click_command->awaiting_second_click))
                                                        || (state_Draw_command_is_(Fillet) && (!two_click_command->awaiting_second_click))
                                                        || (state_Draw_command_is_(Join2) && (!two_click_command->awaiting_second_click))
                                                        || (state_Draw_command_is_(Select) && (!closest_result.closest_entity->is_selected))
                                                        || (state_Draw_command_is_(Deselect) && (closest_result.closest_entity->is_selected))
                                                ) {
                                                    // TODO: consider having closest result return a nill entity
                                                    //       that is valid to pass to draw *shrug*
                                                    chowder_set_color_AND_chower_entity(closest_result.closest_entity);
                                                }
                                            }

                                            if (two_click_command->awaiting_second_click) {
                                                if (0) {
                                                } else if (state_Draw_command_is_(Fillet) || state_Draw_command_is_(DogEar)) {
                                                    if (closest_result.success) {
                                                        if (state_Draw_command_is_(Fillet)) {
                                                            FilletResult fillet_result = preview_fillet(two_click_command->entity_closest_to_first_click, closest_result.closest_entity, click_average_click, popup->fillet_radius);
                                                            if (fillet_result.fillet_success) {
                                                                chowder_set_color_AND_chower_entity(&fillet_result.ent_one);
                                                                chowder_set_color_AND_chower_entity(&fillet_result.ent_two);
                                                                chowder_set_color_AND_chower_entity(&fillet_result.fillet_arc);
                                                            } else {
                                                                chowder_set_color_AND_chower_entity(two_click_command->entity_closest_to_first_click);
                                                                chowder_set_color_AND_chower_entity(closest_result.closest_entity);
                                                            }
                                                        } else { ASSERT(state_Draw_command_is_(DogEar)); 
                                                            DogEarResult dogear_result = preview_dogear(two_click_command->entity_closest_to_first_click, closest_result.closest_entity, click_average_click, popup->dogear_radius);
                                                            if (dogear_result.dogear_success) {
                                                                chowder_set_color_AND_chower_entity(&dogear_result.ent_one);
                                                                chowder_set_color_AND_chower_entity(&dogear_result.ent_two);
                                                                chowder_set_color_AND_chower_entity(&dogear_result.fillet_arc_one);
                                                                chowder_set_color_AND_chower_entity(&dogear_result.fillet_arc_two);
                                                                chowder_set_color_AND_chower_entity(&dogear_result.dogear_arc);
                                                            } else {
                                                                chowder_set_color_AND_chower_entity(closest_result.closest_entity);
                                                            }
                                                        }
                                                    }

                                                    // { // X
                                                    //     real funky_OpenGL_factor = other.camera_drawing.ortho_screen_height_World / 120.0f;

                                                    //     chowder_set_size(2.0f);
                                                    //     real r = 0.5f * funky_OpenGL_factor;
                                                    //     eso_vertex(click_average_click + V2( r,  r));
                                                    //     eso_vertex(click_average_click + V2(-r, -r));
                                                    //     eso_vertex(click_average_click + V2(-r,  r));
                                                    //     eso_vertex(click_average_click + V2( r, -r));
                                                    //     chowder_reset_size();
                                                    // }
                                                } else if (state_Draw_command_is_(Divide2)) {
                                                    // TODO animated dotted line
                                                    // TODO dotted arcs (here (easy) and glsl (hard))

                                                    if (closest_result.success) {

                                                        ClosestIntersectionResult intersection_result =
                                                            closest_intersection(
                                                                    two_click_command->entity_closest_to_first_click,
                                                                    closest_result.closest_entity,
                                                                    mouse_no_snap_potentially_15_deg__WHITE.mouse_position
                                                                    );

                                                        // vec3 pallete_failure = V3(1.0f, 0.0f, 0.0f);
                                                        // vec3 pallete_warning = V3(1.0f, 1.0f, 0.0f);

                                                        bool other_condition = (
                                                                intersection_result.point_is_on_entity_b
                                                                // || intersection_result.point_is_on_entity_a
                                                                );
                                                        bool failure = (intersection_result.no_possible_intersection || (!other_condition));
                                                        if (failure) {
                                                            // chowder_set_color(pallete_failure);
                                                            chowder_set_color_AND_chower_entity(two_click_command->entity_closest_to_first_click);
                                                            chowder_set_color_AND_chower_entity(closest_result.closest_entity);
                                                        } else {

                                                            auto Q = [&](Entity *entity) {
                                                                if (entity->type == EntityType::Line) {
                                                                    LineEntity *line = &entity->line;
                                                                    chowder_vertex(line->start);
                                                                    chowder_vertex(intersection_result.point);
                                                                }
                                                            };


                                                            chowder_set_color_AND_chower_entity(two_click_command->entity_closest_to_first_click);
                                                            chowder_set_color_AND_chower_entity(closest_result.closest_entity);


                                                            chowder_set_stipple(true);
                                                            {
                                                                // chowder_set_color(pallete_2D->two_click_blend);
                                                                // chowder_set_color(pallete_2D->two_click_first_click);
                                                                chowder_set_color(get_color_from_color_code(two_click_command->entity_closest_to_first_click->color_code));
                                                                Q(two_click_command->entity_closest_to_first_click);
                                                                // Q(closest_result.closest_entity);
                                                            }
                                                            chowder_reset_stipple();

                                                            // TODO: dotted lines to point
                                                            // TODO: FAILURE/WARNING color for point off entity
                                                            // TODO: SUCCESS color for point on entity
                                                            chowder_set_primitive(SOUP_POINTS);
                                                            chowder_set_size(4.0);
                                                            {
                                                                chowder_set_color(get_color_from_color_code(closest_result.closest_entity->color_code));
                                                                chowder_vertex(intersection_result.point);
                                                            }
                                                            chowder_reset_size();
                                                            chowder_set_primitive(SOUP_LINES);
                                                        }
                                                    }
                                                } else if (state_Draw_command_is_(Join2)) {
                                                    chowder_entity(two_click_command->entity_closest_to_first_click);
                                                } else if (state_Draw_command_is_(Select)) {
                                                    // TODO: draw all selected entities in white
                                                }
                                            }
                                        }
                                    }
                                }
                            }



                            { // Translate, Rotate, Scale, XMirror, YMirror, LCopy, RCopy,
                            // TODO: Scale is still drawing DRAW2D_PASS_Mouse (FORNOW underneath so you can't see it)
                                bool is_mirror_command = (state_Draw_command_is_(XMirror) || state_Draw_command_is_(YMirror));
                                bool wonko_case = ((pass == DRAW2D_PASS_DrawEnter) != (is_mirror_command)); // *shrug*
                                if (two_click_command->awaiting_second_click || wonko_case) {
                                    uint reps;
                                    mat4 M;
                                    {
                                        reps = 1;
                                        if (0) {
                                        } else if (state_Draw_command_is_(Translate)) {
                                            M = M4_Translation(click_vector);
                                        } else if (state_Draw_command_is_(Rotate)) {
                                            M = M4_Translation(click_1) * M4_RotationAboutZAxis(click_angle) * M4_Translation(-click_1);
                                        } else if (state_Draw_command_is_(Scale)) {
                                            M = M4_Translation(click_1) * M4_Scaling(preview->scale_factor) * M4_Translation(-click_1);
                                        } else if (state_Draw_command_is_(XMirror)) {
                                            M = M4_Translation(crosshair) * M4_Scaling(-1.0f, 1.0f) * M4_Translation(-crosshair);
                                        } else if (state_Draw_command_is_(YMirror)) {
                                            M = M4_Translation(crosshair) * M4_Scaling(1.0f, -1.0f) * M4_Translation(-crosshair);
                                        } else if (state_Draw_command_is_(LCopy)) {
                                            real preview_num_additional_copies = MAX(1.0f, preview->lcopy_num_additional_copies);
                                            uint num_additional_copies = MAX(1, popup->lcopy_num_additional_copies);
                                            vec2 total_translation = preview_num_additional_copies * click_vector;
                                            vec2 fractional_translation = total_translation / num_additional_copies;
                                            M = M4_Translation(fractional_translation);
                                            reps = popup->lcopy_num_additional_copies;
                                        } else if (state_Draw_command_is_(RCopy)) {
                                            uint num_total_copies = MAX(1U, popup->rcopy_num_total_copies);
                                            real total_angle = preview->rcopy_last_angle;
                                            real fractional_angle = total_angle / MAX(1, num_total_copies - 1);
                                            M = M4_Translation(click_1) * M4_RotationAboutZAxis(fractional_angle) * M4_Translation(-click_1);
                                            reps = num_total_copies - 1;
                                        } else {
                                            reps = 0;
                                        }
                                    }

                                    { // FORNOW: perhaps a bit silly on the CPU side, but i think worth for simplicity
                                        mat4 M_accumulator = M;
                                        for_(i, reps) {
                                            chowder_set_M(base_M * M_accumulator);
                                            _for_each_selected_entity_ {
                                                chowder_entity(entity);

                                                bbox2 box = entity_get_bbox(entity);
                                                box.min = transformPoint(M_accumulator, box.min);
                                                box.max = transformPoint(M_accumulator, box.max);
                                                drawn_entities_bbox += box;
                                            }
                                            chowder_reset_M();
                                            M_accumulator *= M;
                                        }
                                    }

                                }
                            }


                            // TODO dotted line
                            // TODO animated dotted line
                            // TODO Update bbox
                            { // one-click commands
                                if (0) {
                                } else if (state_Draw_command_is_(Offset)) {
                                    bbpr = true;
                                    DXFFindClosestEntityResult closest_result = dxf_find_closest_entity(&drawing->entities, mouse_no_snap_potentially_15_deg__WHITE.mouse_position);
                                    if (closest_result.success) {
                                        Entity *_closest_entity = closest_result.closest_entity;
                                        chowder_set_color(pallete_2D->two_click_first_click);
                                        chowder_entity(_closest_entity);

                                        chowder_set_color(pallete_2D->two_click_second_click);
                                        Entity target_entity = entity_offsetted(_closest_entity, popup->offset_distance, mouse_no_snap_potentially_15_deg__WHITE.mouse_position);
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
                                            // real angle = ATAN2(mouse_no_snap_potentially_15_deg__WHITE - circle->center);
                                            // TODO: something else?
                                            target_middle   = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle);
                                            target_start    = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle - PI / 2);
                                            target_end      = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle + PI / 2);
                                            target_opposite = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle + PI);

                                        }

                                        {
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
                                        }

                                        vec2 a = preview->offset_entity_start;
                                        vec2 b = preview->offset_entity_middle;
                                        vec2 c = preview->offset_entity_end;
                                        vec2 d = preview->offset_entity_opposite;
                                        Entity dummy = entity_make_three_point_arc_or_line(a, b, c);
                                        Entity dummy2 = entity_make_three_point_arc_or_line(a, d, c);

                                        chowder_entity(&dummy2);
                                        chowder_entity(&dummy);

                                    }
                                    // TODO
                                    chowder_set_color(pallete_2D->two_click_blend);
                                }
                            }

                            // TODO Update bbox ?
                            if (!is_feature_plane_pass) { // dotted lines
                                chowder_set_stipple(true);
                                if (
                                        ((pass == DRAW2D_PASS_DrawEnter) || (two_click_command->awaiting_second_click))
                                        &&
                                        (0
                                         || (state_Draw_command_is_(Translate))
                                         || (state_Draw_command_is_(Rotate))
                                         // || (state_Draw_command_is_(Polygon))
                                         || (state_Draw_command_is_(Fillet))
                                         || (state_Draw_command_is_(DogEar))
                                         || (state_Draw_command_is_(Measure))
                                        )
                                ) {
                                    chowder_vertex(click_1);
                                    chowder_vertex(click_2);
                                }
                                {
                                    real R = 1024.0f;
                                    if (state_Draw_command_is_(XMirror)) {
                                        chowder_vertex(crosshair - V2(0.0f, R)) ;
                                        chowder_vertex(crosshair + V2(0.0f, R)) ;
                                    }
                                    if (state_Draw_command_is_(YMirror)) {
                                        chowder_vertex(crosshair - V2(R, 0.0f)) ;
                                        chowder_vertex(crosshair + V2(R, 0.0f)) ;
                                    }
                                }
                                chowder_reset_stipple();
                            }


                            if (!state_Snap_command_is_(None) && !is_feature_plane_pass) { // snapped (PINK) entity
                                if (mouse_transformed__PINK.snapped) {
                                    // chowder_set_color(PINK);

                                    // chowder_set_stipple(true);
                                    Entity *entity_snapped_to = &drawing->entities.array[mouse_transformed__PINK.entity_index_snapped_to];
                                    chowder_set_color_AND_chower_entity(entity_snapped_to);

                                    { // FORNOW: Center, End, Intersect additional annotations
                                        _for_each_entity_ {
                                            if (0
                                                    || state_Snap_command_is_(End)
                                                    || state_Snap_command_is_(Intersect)
                                               ) {
                                                if (IS_ZERO(squared_distance_point_entity(mouse_transformed__PINK.mouse_position, entity))) {
                                                    chowder_set_color_AND_chower_entity(entity);
                                                }
                                            } else if (0
                                                    || state_Snap_command_is_(Center)
                                                    ) {
                                                if (0
                                                        || entity->type == EntityType::Arc
                                                        || entity->type == EntityType::Circle
                                                   ) {
                                                    vec2 center = (entity->type == EntityType::Arc) ? entity->arc.center : entity->circle.center;
                                                    if (IS_ZERO(squaredDistance(mouse_transformed__PINK.mouse_position, center))) {
                                                        chowder_set_color_AND_chower_entity(entity);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    chowder_reset_stipple();

                                    chowder_set_color(get_color_from_color_code(entity_snapped_to->color_code));
                                    chowder_set_primitive(SOUP_POINTS);
                                    chowder_set_size(4.0f);
                                    chowder_vertex(mouse_transformed__PINK.mouse_position);
                                    chowder_reset_size();
                                    chowder_reset_primitive();
                                }
                            }

                            // TODO Update bbox ?
                            if (!is_feature_plane_pass) { // crosshairs
                                bool not_drawing_on_top_of_system_cursor = ((pass != DRAW2D_PASS_Mouse) || Snap_eating_mouse);
                                if (not_drawing_on_top_of_system_cursor) {
                                    real funky_OpenGL_factor = other.camera_drawing.ortho_screen_height_World / 120.0f;

                                    chowder_set_color(basic.black);
                                    chowder_set_size(2.0f);
                                    real r = 1.3 * funky_OpenGL_factor;
                                    chowder_vertex(crosshair - V2(r, 0));
                                    chowder_vertex(crosshair + V2(r, 0));
                                    chowder_vertex(crosshair - V2(0, r));
                                    chowder_vertex(crosshair + V2(0, r));
                                    chowder_reset_size();

                                    chowder_set_color(color);
                                    r = 1.2 * funky_OpenGL_factor;
                                    chowder_vertex(crosshair - V2(r, 0));
                                    chowder_vertex(crosshair + V2(r, 0));
                                    chowder_vertex(crosshair - V2(0, r));
                                    chowder_vertex(crosshair + V2(0, r));
                                }
                            }
                        }
                    }
                    chowder_end();

                    return drawn_entities_bbox;
                };
                render_2d(PV_2D, M4_Identity(), false);

                { // feature plane feature-plane feature_plane // floating sketch plane; selection plane NOTE: transparent
                    bbox2 face_selection_bbox; { // FORNOW: recompute every frame on CPU
                        face_selection_bbox = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
                        { // we want this to be done regardless for HidePlane tweening
                            // TODO choose true or false (or something else)
                            // Currently false because we already tween using this as target
                            mat4 inv_M_3D_from_2D = inverse(get_M_3D_from_2D(false));

                            WorkMesh *mesh = &meshes->work;
                            for_(triangle_index, mesh->num_triangles) {
                                vec3 n = mesh->triangle_normals[triangle_index];
                                if (dot(n, feature_plane->normal) > 0.99f) {
                                    uint3 tuple = mesh->triangle_tuples[triangle_index];
                                    vec3 a = mesh->vertex_positions[tuple[0]];
                                    real sd = dot(a, n);
                                    if (ABS(sd - feature_plane->signed_distance_to_world_origin) < 0.01f) {
                                        for_(d, 3) {
                                            vec3 p_3D = mesh->vertex_positions[tuple[d]];
                                            vec2 p_2D = _V2(transformPoint(inv_M_3D_from_2D, p_3D));
                                            face_selection_bbox += p_2D;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    bbox2 dxf_selection_bbox; {
                        dxf_selection_bbox = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();

                        if (feature_plane->is_active) {
                            glDisable(GL_DEPTH_TEST);
                            gl_scissor_Pixel(x_divider_drawing_mesh_Pixel, 0, window_width - x_divider_drawing_mesh_Pixel, window_height);

                            dxf_selection_bbox += render_2d(PV_3D, M_3D_from_2D, true);

                            glEnable(GL_DEPTH_TEST);
                            gl_scissor_Pixel(0, 0, x_divider_drawing_mesh_Pixel, window_height);
                        }
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
                            real eps = 5.0f;
                            target_bbox.min[0] -= eps;
                            target_bbox.max[0] += eps;
                            target_bbox.min[1] -= eps;
                            target_bbox.max[1] += eps;
                        }
                    }

                    if (!feature_plane->is_active) {
                        target_bbox = bbox_inflate(preview->feature_plane_bbox, 2.0f); // HACK
                    }

                    JUICEIT_EASYTWEEN(&preview->feature_plane_bbox.min, target_bbox.min);
                    JUICEIT_EASYTWEEN(&preview->feature_plane_bbox.max, target_bbox.max);
                    if (other.time_since_plane_selected == 0.0f) { // FORNOW

                        preview->feature_plane_bbox = face_selection_bbox;
                        preview->feature_plane_alpha = 0.0f;

                        // FORNOW
                        if (face_selection_bbox.min[0] > face_selection_bbox.max[0]) {
                            preview->feature_plane_bbox = {};
                        }

                        target_bbox = bbox_inflate(target_bbox, -10.0f);
                    }
                }
            }

            { // TODOLATER: dots snap_divide_dot

                { // snap_divide_dot
                    eso_begin(PV_2D, SOUP_POINTS);
                    eso_color(pallete_2D->dots);
                    JUICEIT_EASYTWEEN(&other.size_snap_divide_dot, 0.0f, 0.5f);
                    eso_size(other.size_snap_divide_dot);
                    eso_vertex(other.snap_divide_dot);
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
        vec3 color = pallete_2D->foreground;

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



    if (other.show_history) history_debug_draw();

    { // paused; slowmo
        real x = 12.0f;
        real y = window_get_height_Pixel() - 12.0f;
        real w = 6.0f;
        real h = 2.5f * w;
        if (other.paused) {
            eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
            eso_overlay(true);
            eso_color(basic.green);
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
            eso_color(basic.yellow);
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
        uint num_selected_lines;
        uint num_selected_arcs;
        uint num_selected_circles;
        bool any_entities_selected;
        {
            num_lines = 0;
            num_arcs = 0;
            num_circles = 0;
            num_selected_lines = 0;
            num_selected_arcs = 0;
            num_selected_circles = 0;
            _for_each_entity_ {
                if (entity->type == EntityType::Line) {
                    ++num_lines;
                    if (entity->is_selected) ++num_selected_lines;
                } else if (entity->type == EntityType::Arc) {
                    ++num_arcs;
                    if (entity->is_selected) ++num_selected_arcs;
                } else { ASSERT(entity->type == EntityType::Circle);
                    ++num_circles;
                    if (entity->is_selected) ++num_selected_circles;
                }
            }
            any_entities_selected = ((num_selected_lines != 0) || (num_selected_arcs != 0) || (num_selected_circles != 0));
        }

        { // number of elements, etc. fps
            // FORNOW: ew

            real eps = 4.0f;
            real height = 12.0f;
            static char scratch_buffer[1024];

            { // details
                if (other.show_details_2D) {
                    EasyTextPen pen = { V2(0.0f, window_get_height_Pixel() - height), height, pallete_2D->foreground };
                    if (!any_entities_selected) {
                        sprintf(scratch_buffer, "%d lines %d arcs %d circles", num_lines, num_arcs, num_circles);
                    } else {
                        sprintf(scratch_buffer, "%d/%d lines  %d/%d arcs  %d/%d circles",
                                num_selected_lines,
                                num_lines,
                                num_selected_arcs,
                                num_arcs,
                                num_selected_circles,
                                num_circles
                                );
                    }
                    real w = _easy_text_dx(&pen, scratch_buffer);
                    real W = get_x_divider_drawing_mesh_Pixel();
                    pen.origin.x = (W - w) / 2;
                    easy_text_drawf(&pen, scratch_buffer);
                }

                if (other.show_details_3D) {
                    EasyTextPen pen = { V2(0.0f, window_get_height_Pixel() - height), height, pallete_2D->foreground };
                    sprintf(scratch_buffer, "%d triangles %d vertices", meshes->work.num_triangles, meshes->work.num_vertices);
                    real w = _easy_text_dx(&pen, scratch_buffer);
                    real W = window_get_width_Pixel() - get_x_divider_drawing_mesh_Pixel();
                    pen.color = pallete_3D->foreground;
                    pen.origin.x = get_x_divider_drawing_mesh_Pixel() + (W - w) / 2;
                    easy_text_drawf(&pen, scratch_buffer);
                }
            }

            static int fps; {
                static int measured_fps;

                // grab and smooth fps
                {
                    const int N_MOVING_WINDOW = 5;
                    static std::chrono::steady_clock::time_point prev_timestamps[N_MOVING_WINDOW];
                    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
                    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp - prev_timestamps[N_MOVING_WINDOW - 1]);
                    measured_fps = (int) round(N_MOVING_WINDOW / (nanos.count() / 1000000000.));

                    for (int i = N_MOVING_WINDOW - 1; i >= 1; --i) {
                        prev_timestamps[i] = prev_timestamps[i - 1];
                    }
                    prev_timestamps[0] = timestamp;
                }

                {
                    static std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
                    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - timestamp);
                    if (nanos.count() > 166666666 / 1.5) {
                        timestamp = std::chrono::steady_clock::now();
                        fps = measured_fps;
                    }
                }
            }

            if (other.show_debug) {
                EasyTextPen pen = { V2(0.0f, window_get_height_Pixel() - height), height, pallete_3D->foreground };
                sprintf(scratch_buffer, "%d fps", fps);
                real w = _easy_text_dx(&pen, scratch_buffer);
                pen.origin.x = window_get_width_Pixel() - w - eps - eps;
                easy_text_drawf(&pen, scratch_buffer);
            }
        }
    }

}

