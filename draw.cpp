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
    vec3 o = (feature_plane->signed_distance_to_world_origin) * feature_plane->normal;

    if (for_drawing) {
        // FINITE_EASYTWEEN(&feature_plane->x_angle, feature_plane->mirror_x ? PI : 0.0f, PI / 180 / 2);
        // FINITE_EASYTWEEN(&feature_plane->y_angle, feature_plane->mirror_y ? PI : 0.0f, PI / 180 / 2);
        JUICEIT_EASYTWEEN(&feature_plane->x_angle, feature_plane->mirror_x ? PI : 0.0f);
        JUICEIT_EASYTWEEN(&feature_plane->y_angle, feature_plane->mirror_y ? PI : 0.0f);
        // TODO: Change 10 to function of feature_plane size
        JUICEIT_EASYTWEEN(&feature_plane->offset, 10 * MAX(SIN(feature_plane->x_angle), SIN(feature_plane->y_angle)));

        x = transformVector(M4_RotationAbout(y, feature_plane->x_angle), x);
        y = transformVector(M4_RotationAbout(x, feature_plane->y_angle), y);
        o += feature_plane->offset * feature_plane->normal;
    } else {
        if (feature_plane->mirror_x) x *= - 1;
        if (feature_plane->mirror_y) y *= - 1;
    }

    return M4_xyzo(x, y, z, o);
}


void conversation_draw() {

    mat4 P_2D = camera_drawing->get_P();
    mat4 V_2D = camera_drawing->get_V();
    mat4 PV_2D = P_2D * V_2D;
    mat4 inv_PV_2D = inverse(PV_2D);
    vec2 mouse_World_2D = transformPoint(inv_PV_2D, other.mouse_OpenGL);
    mat4 M_3D_from_2D = get_M_3D_from_2D(true);

    TransformMouseDrawingPositionResult mouse_no_snap_potentially_15_deg__WHITE = transform_mouse_drawing_position(mouse_World_2D, other.shift_held, true );
    TransformMouseDrawingPositionResult mouse_transformed__PINK                = transform_mouse_drawing_position(mouse_World_2D, other.shift_held, false);
    JUICEIT_EASYTWEEN(&preview->mouse_no_snap_potentially_15_deg__WHITE_position, mouse_no_snap_potentially_15_deg__WHITE.mouse_position);

    bool extruding = ((state_Mesh_command_is_(ExtrudeAdd)) || (state_Mesh_command_is_(ExtrudeCut)));
    bool revolving = ((state_Mesh_command_is_(RevolveAdd)) || (state_Mesh_command_is_(RevolveCut)));
    bool adding     = ((state_Mesh_command_is_(ExtrudeAdd)) || (state_Mesh_command_is_(RevolveAdd)));
    bool cutting     = ((state_Mesh_command_is_(ExtrudeCut)) || (state_Mesh_command_is_(RevolveCut)));

    FORNOW_UNUSED(revolving);

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


    vec2 target_preview_drawing_origin = (!state_Draw_command_is_(SetOrigin)) ? drawing->origin : mouse_no_snap_potentially_15_deg__WHITE.mouse_position; // NOTE: we use this so we don't double tween SetOrigin's effect on the preview entities on the right (or something like this ??)
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
            preview_dxf_axis_base_point = mouse_no_snap_potentially_15_deg__WHITE.mouse_position;
            preview_dxf_axis_angle_from_y = drawing->axis_angle_from_y;
        } else {
            preview_dxf_axis_base_point = two_click_command->first_click;
            preview_dxf_axis_angle_from_y = ATAN2(mouse_no_snap_potentially_15_deg__WHITE.mouse_position - preview_dxf_axis_base_point) - PI / 2;
        }
    }


    mat4 P_3D = camera_mesh->get_P();
    mat4 V_3D = camera_mesh->get_V();
    mat4 PV_3D = P_3D * V_3D;
    mat4 World_3D_from_OpenGL = inverse(PV_3D);
    MagicSnapResult3D mouse_snap_result_3D; {
        vec3 mouse_ray_origin = transformPoint(World_3D_from_OpenGL, V3(other.mouse_OpenGL, -1.0f));
        vec3 mouse_ray_end = transformPoint(World_3D_from_OpenGL, V3(other.mouse_OpenGL,  1.0f));
        vec3 mouse_ray_direction = normalized(mouse_ray_end - mouse_ray_origin);
        mouse_snap_result_3D = magic_snap_raycast(mouse_ray_origin, mouse_ray_direction);
    }

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
        eso_color(pallete.white);
        eso_vertex(other.x_divider_drawing_mesh_OpenGL, -1.0f);
        eso_vertex(other.x_divider_drawing_mesh_OpenGL,  1.0f);
        eso_end();
    }

    real x_divider_drawing_mesh_Pixel = get_x_divider_drawing_mesh_Pixel();

    bool moving_selected_entities = (
            (state_Draw_command_is_(Move)) || 
            ((two_click_command->awaiting_second_click)
             && (0 
                 || (state_Draw_command_is_(Rotate))
                 || (state_Draw_command_is_(Copy)))
            )); // TODO: loft up

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

        auto DRAW_DOTTED_LINE = [&](vec2 click_1, vec2 click_2, vec3 color) {
            eso_begin(PV_2D, SOUP_LINES);
            eso_stipple(true);
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

        // TODO: COPY


        // TODO: use trick from two functions down

        auto DRAW_LCOPY = [&](vec2, vec2, vec3 color) {
            uint num_additional_copies = uint(preview->linear_copy_num_additional_copies);
            vec2 translation = { preview->linear_copy_run, preview->linear_copy_rise };
            for_(i, num_additional_copies) {
                mat4 M = M4_Translation((i + 1) * translation);

                eso_begin(PV_2D * M, SOUP_LINES);
                eso_color(color);
                _for_each_selected_entity_ eso_entity__SOUP_LINES(entity);
                eso_end();
            }
        };

        auto DRAW_RCOPY = [&](vec2 click_1, vec2, vec3 color) {
            uint num_total_copies = uint(preview->rcopy_num_total_copies);
            real angle = preview->rcopy_angle;
            for (uint i = 1; i <= num_total_copies; ++i) { // FONOW: this should be < but other problems
                mat4 M = M4_Translation(click_1) * M4_RotationAboutZAxis(i * RAD(angle)) * M4_Translation(-click_1);

                eso_begin(PV_2D * M, SOUP_LINES);
                eso_color(color);
                _for_each_selected_entity_ eso_entity__SOUP_LINES(entity);
                eso_end();
            }
        };

        bool moving = state_Draw_command_is_(Move);
        bool linear_copying = (state_Draw_command_is_(Copy));
        bool rotating = (state_Draw_command_is_(Rotate));
        // bool moving_linear_copying_or_rotating = (moving || rotating || linear_copying);

        auto DRAW_ENTITIES_BEING_MOVED_LINEAR_COPIED_OR_ROTATED = [&](vec2 click_1, vec2 click_2, vec3 color) {
            // TODO: do this like crosshairs where they disappear more immediatelly

            //if (IS_ZERO(squaredNorm(click_1 - click_2))) return; // NOTE: you want this (we're attempthing this call three times!)
            // changed how the call structure worked so maybe we dont need it
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
                Draw_Enter = (!two_click_command->awaiting_second_click) ? V2(0, 0) : *first_click;
                // if (popup->manager.focus_group == ToolboxGroup::Draw) {
                if (state_Draw_command_is_(Box)) Draw_Enter += V2(popup->box_width, popup->box_height);
                if (state_Draw_command_is_(Circle)) Draw_Enter += V2(popup->circle_radius, 0.0f);
                if (state_Draw_command_is_(Polygon)) Draw_Enter += V2(popup->polygon_distance_to_corner, 0.0f);
                if (state_Draw_command_is_(Line)) Draw_Enter += V2(popup->line_run, popup->line_rise);
                if (state_Draw_command_is_(Move)) Draw_Enter += V2(popup->move_run, popup->move_rise);
                if (state_Draw_command_is_(Copy)) Draw_Enter += V2(popup->linear_copy_run, popup->linear_copy_rise);
                if (state_Draw_command_is_(Rotate)) Draw_Enter += 10.0f * e_theta(RAD(popup->rotate_angle));
                // }
            }
            vec2 Snap_Enter; {
                Snap_Enter = *first_click;
                // if (popup->manager.focus_group == ToolboxGroup::Snap) {
                if (state_Snap_command_is_(XY)) {
                    Snap_Enter = V2(popup->xy_x_coordinate, popup->xy_y_coordinate);
                }
                // }
            }


            {
                JUICEIT_EASYTWEEN(&preview->mouse_from_Draw_Enter__BLUE_position, Draw_Enter);
                JUICEIT_EASYTWEEN(&preview->xy_xy, Snap_Enter);

                JUICEIT_EASYTWEEN(&preview->mouse_transformed__PINK_position, mouse_transformed__PINK.mouse_position, 1.0f);

                JUICEIT_EASYTWEEN(&preview->polygon_num_sides, real(popup->polygon_num_sides));

                JUICEIT_EASYTWEEN(&preview->rcopy_num_total_copies, real(popup->rcopy_num_total_copies));
                JUICEIT_EASYTWEEN(&preview->rcopy_angle, popup->rcopy_angle);

                JUICEIT_EASYTWEEN(&preview->linear_copy_run, popup->linear_copy_run);
                JUICEIT_EASYTWEEN(&preview->linear_copy_rise, popup->linear_copy_rise);
                JUICEIT_EASYTWEEN(&preview->linear_copy_num_additional_copies, real(popup->linear_copy_num_additional_copies));
            }
            bool Snap_eating_mouse = !(state_Snap_command_is_(None) || state_Snap_command_is_(XY));
            // bool Draw_eating_Enter = ((popup->manager.focus_group == ToolboxGroup::Draw) &&
            //         (!two_click_command->awaiting_second_click || !ARE_EQUAL(*first_click, Draw_Enter)));
            // bool Snap_eating_Enter = ((popup->manager.focus_group == ToolboxGroup::Snap) && state_Snap_command_is_(XY) &&
            //         (!two_click_command->awaiting_second_click || !ARE_EQUAL(*first_click, Snap_Enter)));



            vec3 BLUE = get_accent_color(ToolboxGroup::Draw);
            vec3 PINK = get_accent_color(ToolboxGroup::Snap);
            vec3 WHITE = get_color(ColorCode::Emphasis);
            vec3 GRAY = pallete.gray;

            vec2 mouse_WHITE_or_PINK_position__depending_on_whether_snap_is_active;
            vec3 WHITE_or_PINK_depending_on_whether_snap_is_active;
            {
                if (!Snap_eating_mouse) {
                    WHITE_or_PINK_depending_on_whether_snap_is_active = WHITE;
                    mouse_WHITE_or_PINK_position__depending_on_whether_snap_is_active = mouse_no_snap_potentially_15_deg__WHITE.mouse_position;
                } else {
                    WHITE_or_PINK_depending_on_whether_snap_is_active = PINK;
                    mouse_WHITE_or_PINK_position__depending_on_whether_snap_is_active = preview->mouse_transformed__PINK_position;
                }
            }



            // vec2 click_vector = (mouse_WHITE_or_PINK_position__depending_on_whether_snap_is_active - *first_click);
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
                        eso_size(2.0f);
                        eso_color(get_accent_color(ToolboxGroup::Snap));
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
              // FORNOW (this is sloppy and bad) nate: just made it more sloppy and bad
                #define ANNOTATION(Name, NAME) \
                do { \
                    if (state_Draw_command_is_(Name)) { \
                        DRAW_##NAME(V2(0, 0), V2(0, 0), GRAY); /*NOTE: this only actually shows up for Move and Copy and Rotate and probably some other stuff we're forgetting lol whoops (length lines/circles are invisible -- because no POINTS) */ \
                        if (!two_click_command->awaiting_second_click) { \
                        } else { \
                            bool Snap_is_active = !state_Snap_command_is_(None); \
                            if ((Snap_is_active) || (other.time_since_mouse_moved < 1.0f)) { \
                                DRAW_##NAME(*first_click, mouse_WHITE_or_PINK_position__depending_on_whether_snap_is_active, WHITE_or_PINK_depending_on_whether_snap_is_active); \
                            } \
                        } \
                        vec2 tmp = (!two_click_command->awaiting_second_click) ? V2(0, 0) : *first_click; \
                        DRAW_##NAME(tmp, preview->mouse_from_Draw_Enter__BLUE_position, BLUE); \
                        if (state_Snap_command_is_(XY)) { \
                            DRAW_##NAME(tmp, preview->xy_xy, PINK); \
                        } \
                    } \
                } while (0)

                if (two_click_command->awaiting_second_click) {
                    ANNOTATION(Line, LINE);
                    ANNOTATION(CenterLine, CENTERLINE);
                    ANNOTATION(Box, BOX);
                    ANNOTATION(Circle, CIRCLE);
                    ANNOTATION(Polygon, POLYGON);
                } 

                ANNOTATION(Move, DOTTED_LINE);
                ANNOTATION(Rotate, DOTTED_LINE);

                ANNOTATION(Move, ENTITIES_BEING_MOVED_LINEAR_COPIED_OR_ROTATED);
                ANNOTATION(Rotate, ENTITIES_BEING_MOVED_LINEAR_COPIED_OR_ROTATED); // NOTE: don't move this outside no matter how much you want to
                ANNOTATION(Copy, LCOPY);
                ANNOTATION(Copy, DOTTED_LINE);
                ANNOTATION(RCopy, RCOPY);


                { // entity snapped to
                  // TODO: Intersect
                    if (mouse_transformed__PINK.snapped) {
                        Entity *entity_snapped_to = &drawing->entities.array[mouse_transformed__PINK.entity_index_snapped_to];
                        eso_begin(PV_2D, SOUP_LINES);
                        // eso_overlay(true);
                        eso_color(WHITE_or_PINK_depending_on_whether_snap_is_active);
                        eso_entity__SOUP_LINES(entity_snapped_to);
                        eso_end();
                    }
                }

                { // crosshairs
                    if (state_Snap_command_is_(XY)) {
                        DRAW_CROSSHAIR(preview->xy_xy, PINK);
                    } else if (!state_Snap_command_is_(None)) {
                        DRAW_CROSSHAIR(preview->mouse_transformed__PINK_position, PINK);
                    }

                    if (!state_Draw_command_is_(None) && (popup->manager.get_tag(ToolboxGroup::Draw) != NULL)) {
                        DRAW_CROSSHAIR(preview->mouse_from_Draw_Enter__BLUE_position, BLUE);
                    }
                }

                { // experimental preview part B
                  // NOTE: circle <-> circle is wonky
                    if (state_Draw_command_is_(Offset)) {
                        DXFFindClosestEntityResult closest_result = dxf_find_closest_entity(&drawing->entities, mouse_no_snap_potentially_15_deg__WHITE.mouse_position);
                        if (closest_result.success) {
                            Entity *_closest_entity = closest_result.closest_entity;
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
                        eso_vertex(mouse_no_snap_potentially_15_deg__WHITE.mouse_position.x, first_click->y);
                        eso_vertex(mouse_no_snap_potentially_15_deg__WHITE.mouse_position.x, mouse_no_snap_potentially_15_deg__WHITE.mouse_position.y);
                        eso_vertex(first_click->x, mouse_no_snap_potentially_15_deg__WHITE.mouse_position.y);
                        eso_end();
                    }

                    if (state_Draw_command_is_(CenterBox)) {                
                        vec2 one_corner = mouse_no_snap_potentially_15_deg__WHITE.mouse_position;
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
                        eso_vertex(mouse_no_snap_potentially_15_deg__WHITE.mouse_position);
                        eso_end();
                    }
                    if (state_Draw_command_is_(Mirror2)) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(two_click_command->first_click);
                        eso_vertex(mouse_no_snap_potentially_15_deg__WHITE.mouse_position);
                        eso_end();
                    }
                    if (state_Draw_command_is_(DiamCircle)) {
                        vec2 edge_one = two_click_command->first_click;
                        vec2 edge_two = mouse_no_snap_potentially_15_deg__WHITE.mouse_position;
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
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_entity__SOUP_LINES(two_click_command->entity_closest_to_first_click);
                        eso_end();
                    }
                    if (state_Draw_command_is_(Fillet)) {
                        DXFFindClosestEntityResult closest_result = dxf_find_closest_entity(&drawing->entities, mouse_no_snap_potentially_15_deg__WHITE.mouse_position);
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(get_color(ColorCode::Traverse));
                        eso_entity__SOUP_LINES(two_click_command->entity_closest_to_first_click);
                        vec2 average_click = AVG(two_click_command->first_click, mouse_no_snap_potentially_15_deg__WHITE.mouse_position);
                        if (closest_result.success) {
                            FilletResult fillet_result = preview_fillet(two_click_command->entity_closest_to_first_click, closest_result.closest_entity, average_click, popup->fillet_radius);
                            if (fillet_result.fillet_success) {
                                eso_color(get_color(ColorCode::Emphasis));
                                eso_entity__SOUP_LINES(&fillet_result.ent_one);
                                eso_entity__SOUP_LINES(&fillet_result.ent_two);
                                eso_entity__SOUP_LINES(&fillet_result.fillet_arc);
                            }
                        }
                        Entity t = _make_line(V2(0), average_click);
                        eso_entity__SOUP_LINES(&t);
                        eso_end();
                    }
                    if (state_Draw_command_is_(DogEar)) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(get_color(ColorCode::Emphasis));
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



        // mat4 inv_T_o = M4_Translation(-preview->drawing_origin);



        {
            // TODO: revolve crap

            JUICEIT_EASYTWEEN(&preview->manifold_wrapper_tweener, 1.0f);
            mat4 I = M4_Identity();


            // TODO: do the tool mesh drawing with two clip planes
            //       (will work roughly the same for revolve)
            // TODO: tool_* -> tool_prev_intersection_* (
            // TODO: think about this on paper (there may be a simpler solution that's just clip planes
            //       on prev and curr)
            if (!meshes->was_cut) {
                mat4 S = meshes->M_3D_from_2D * M4_Scaling(1.0f, 1.0f, preview->manifold_wrapper_tweener) * inverse(meshes->M_3D_from_2D);
                fancy_draw(P_3D, V_3D, I, &meshes->prev_draw);
                fancy_draw(P_3D, V_3D, S, &meshes->tool_draw);
            } else {
                // TODO: this is better accomplished via clip planes
                // (NOTE: cutting (if we're going in both directions) will actually produce two disjoint meshes)
                // ====      ====            
                // ====      ^^^^            
                // ====  ->                  
                // ====      vvvv            
                // ====      ====            


                mat4 S = meshes->M_3D_from_2D * M4_Scaling(1.0f, 1.0f, (1.0f - preview->manifold_wrapper_tweener)) * inverse(meshes->M_3D_from_2D); // TODO: not quite the right transform
                fancy_draw(P_3D, V_3D, I, &meshes->draw);
                if (ABS(preview->manifold_wrapper_tweener) < 0.97f) fancy_draw(P_3D, V_3D, S, &meshes->tool_draw);
            }
        }

        mat4 PVM_feature_plane = PV_3D * M_3D_from_2D;

        { // feature plane feature-plane feature_plane // floating sketch plane; selection plane NOTE: transparent
            {
                bbox2 face_selection_bbox; { // FORNOW: recompute every frame on CPU
                    face_selection_bbox = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
                    { // we want this to be done regardless for HidePlane tweening

                        mat4 inv_M_3D_from_2D = inverse(get_M_3D_from_2D());

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
                    dxf_selection_bbox = entities_get_bbox(&drawing->entities, true);

                    // TODO: this should incorporate a preview of the fact that some entities are moving
                    if (moving_selected_entities) {
                        vec2 T = (preview->mouse_no_snap_potentially_15_deg__WHITE_position - two_click_command->first_click);
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
                    target_bbox = bbox_inflate(preview->feature_plane, 2.0f); // HACK
                }

                JUICEIT_EASYTWEEN(&preview->feature_plane.min, target_bbox.min);
                JUICEIT_EASYTWEEN(&preview->feature_plane.max, target_bbox.max);
                if (other.time_since_plane_selected == 0.0f) { // FORNOW
                    preview->feature_plane = face_selection_bbox;
                    preview->feature_plane_alpha = 0.0f;

                    // FORNOW
                    if (face_selection_bbox.min[0] > face_selection_bbox.max[0]) {
                        preview->feature_plane = {};
                    }

                    target_bbox = bbox_inflate(target_bbox, -10.0f);
                }
            }
        }

        JUICEIT_EASYTWEEN(&preview->feature_plane_alpha, (feature_plane->is_active) ? 0.35f : 0.0f);

        // NOTE this is hacky as hell there's something fancy we can probs do with stentcil buffering the lines
        //      that is more correct than the 35/35 split, but FORNOW this is k
        #if 1
        glDisable(GL_DEPTH_TEST);
        { // draw feature plane
            glDisable(GL_CULL_FACE); // FORNOW

            // vec2 center = (preview->feature_plane.max + preview->feature_plane.min) / 2.0f;
            // mat4 scaling_about_center = M4_Translation(center) * M4_Scaling(f) * M4_Translation(-center);
            eso_begin(PVM_feature_plane * M4_Translation(0.0f, 0.0f, 2 * Z_FIGHT_EPS)/* * scaling_about_center*/, SOUP_QUADS);
            eso_color(pallete.white, preview->feature_plane_alpha);
            eso_bbox_SOUP_QUADS(preview->feature_plane);
            eso_end();

            glEnable(GL_CULL_FACE); // FORNOW
        }
        glEnable(GL_DEPTH_TEST);
        #endif
        if (!other.hide_grid) { // grid 3D grid 3d grid
            JUICEIT_EASYTWEEN(&preview->bbox_min_y, meshes->work.bbox.min.y);
            real r = 0.5f * GRID_SIDE_LENGTH;
            real f; { // backface culling (check sign of rasterized triangle)
                vec2 a = _V2(transformPoint(PV_3D, V3(0.0f, 0.0f, 0.0f)));
                vec2 b = _V2(transformPoint(PV_3D, V3(1.0f, 0.0f, 0.0f)));
                vec2 c = _V2(transformPoint(PV_3D, V3(0.0f, 0.0f, 1.0f)));
                f = cross(normalized(c - a) , normalized(b - a));
            }
            mat4 transform = PV_3D * M4_Translation(0, 2 * Z_FIGHT_EPS - SGN(f) * Z_FIGHT_EPS, 0);
            eso_begin(transform, SOUP_LINES);
            eso_color(pallete.black, CLAMPED_LINEAR_REMAP(f, 1.0f, 0.0f, 0.07f, 0.02f));
            eso_size(1.0f);
            for (uint i = 1; i <= uint(GRID_SIDE_LENGTH / GRID_SPACING) - 1; ++i) {
                real tmp = i * GRID_SPACING;
                eso_vertex(-r + tmp,              preview->bbox_min_y, -r + 0.0f);
                eso_vertex(-r + tmp,              preview->bbox_min_y, -r + GRID_SIDE_LENGTH);
                eso_vertex(-r + 0.0f,             preview->bbox_min_y, -r + tmp);
                eso_vertex(-r + GRID_SIDE_LENGTH, preview->bbox_min_y, -r + tmp);
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
        { // draw feature plane
            glDisable(GL_CULL_FACE); // FORNOW

            // vec2 center = (preview->feature_plane.max + preview->feature_plane.min) / 2.0f;
            // mat4 scaling_about_center = M4_Translation(center) * M4_Scaling(f) * M4_Translation(-center);
            eso_begin(PVM_feature_plane * M4_Translation(0.0f, 0.0f, 2 * Z_FIGHT_EPS)/* * scaling_about_center*/, SOUP_QUADS);
            eso_color(pallete.white, preview->feature_plane_alpha);
            eso_bbox_SOUP_QUADS(preview->feature_plane);
            eso_end();

            glEnable(GL_CULL_FACE); // FORNOW
        }



        if (feature_plane->is_active) { // selection 2d selection 2D selection tube tubes slice slices stack stacks wire wireframe wires frame (FORNOW: ew) cage cageit
            ;
            // FORNOW
            bool moving_stuff = ((state_Draw_command_is_(SetOrigin)) || (state_Mesh_command_is_(NudgePlane)));
            vec3 target_preview_tubes_color = (0) ? V3(0)
                : (moving_selected_entities) ? get_color(ColorCode::Emphasis)
                : (adding) ? V3(0.0f, 1.0f, 0.0f)
                : (cutting) ? V3(1.0f, 0.0f, 0.0f)
                : (moving_stuff) ? get_color(ColorCode::Emphasis)
                : V3(0);
            JUICEIT_EASYTWEEN(&preview->tubes_color, target_preview_tubes_color);

            #if 1
            mat4 T_o = M4_Translation(preview->drawing_origin);
            mat4 inv_T_o = inverse(T_o);
            glDisable(GL_DEPTH_TEST);
            eso_begin(PV_3D * M_3D_from_2D * inv_T_o, SOUP_LINES); {
                eso_overlay(true);
                _for_each_selected_entity_ {
                    real alpha;
                    vec3 color;
                    alpha = CLAMP(1.0f * MIN(entity->time_since_is_selected_changed, other.time_since_plane_selected), 0.0f, 1.0f); // TODO:preview->alpha
                    color = preview->tubes_color;
                    eso_color(color, alpha);
                    eso_entity__SOUP_LINES(entity, extruding, preview->extrude_out_length, -preview->extrude_in_length);
                } eso_end();
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
                mat4 M = M_3D_from_2D * inv_T_o * T_a * R_0 * inv_T_a;
                for_(tube_stack_index, NUM_TUBE_STACKS_INCLUSIVE) {
                    eso_begin(PV_3D * M, SOUP_LINES); {
                        eso_overlay(true);
                        _for_each_selected_entity_ {
                            real alpha;
                            vec3 color;
                            // if (entity->is_selected) {
                            alpha = CLAMP(-0.2f + 3.0f * MIN(entity->time_since_is_selected_changed, other.time_since_plane_selected), 0.0f, 1.0f);
                            color = CLAMPED_LERP(-0.5f + SQRT(2.0f * entity->time_since_is_selected_changed), pallete.black, preview->tubes_color);
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
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0.0f, 0.0f, preview->feature_plane_offset + Z_FIGHT_EPS);
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
                            // if (entity->is_selected) {
                            alpha = CLAMP(-0.2f + 3.0f * MIN(entity->time_since_is_selected_changed, other.time_since_plane_selected), 0.0f, 1.0f);
                            color = CLAMPED_LERP(-0.5f + SQRT(2.0f * entity->time_since_is_selected_changed), pallete.black, preview->tubes_color);
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

            // eso_begin(PV_3D * M_3D_from_2D, SOUP_LINES);
            // eso_overlay(true);
            // eso_color(pallete.black);
            // eso_size(1.0f);
            // for_(triangle_index, fornow_global_selection_num_triangles) {
            //     uint3 tuple = fornow_global_selection_triangle_tuples[triangle_index];
            //     for_(d, 3) {
            //         uint dp1 = (d + 1) % 3;
            //         eso_vertex(fornow_global_selection_vertex_positions[tuple[d]]);
            //         eso_vertex(fornow_global_selection_vertex_positions[tuple[dp1]]);
            //     }
            // }
            // eso_end();
        };

        // if (mouse_snap_result_3D.hit_mesh) {
        //     eso_begin(PV_3D, SOUP_POINTS);
        //     eso_size(20);
        //     eso_color(get_color(ColorCode::Emphasis));
        //     eso_vertex(mouse_snap_result_3D.mouse_position);
        //     eso_end();
        // }

        if (!mesh_two_click_command->awaiting_second_click) {

        } else if (state_Mesh_command_is_(Measure3D)) {
            eso_begin(PV_3D, SOUP_POINTS);
            eso_overlay(true);
            eso_size(20);
            eso_color(get_color(ColorCode::Emphasis));
            eso_vertex(mesh_two_click_command->first_click);
            eso_end();

            if (mouse_snap_result_3D.hit_mesh) {
                eso_begin(PV_3D, SOUP_LINES);
                eso_color(get_color(ColorCode::Emphasis));
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



    if (other.show_history) history_debug_draw();

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

        { // number of elements, etc. fps
          // FORNOW: ew

            real eps = 4.0f;
            real height = 12.0f;
            static char scratch_buffer[1024];

            if (other.show_details) {
                {
                    EasyTextPen pen = { V2(0.0f, window_get_height_Pixel() - height), height, pallete.white };
                    sprintf(scratch_buffer, "%d lines %d arcs %d circles", num_lines, num_arcs, num_circles);
                    real w = _easy_text_dx(&pen, scratch_buffer);
                    real W = get_x_divider_drawing_mesh_Pixel();
                    pen.origin.x = (W - w) / 2;
                    easy_text_drawf(&pen, scratch_buffer);
                }

                {
                    EasyTextPen pen = { V2(0.0f, window_get_height_Pixel() - height), height, pallete.black };
                    sprintf(scratch_buffer, "%d triangles %d vertices", meshes->work.num_triangles, meshes->work.num_vertices);
                    real w = _easy_text_dx(&pen, scratch_buffer);
                    real W = window_get_width_Pixel() - get_x_divider_drawing_mesh_Pixel();
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
                EasyTextPen pen = { V2(0.0f, window_get_height_Pixel() - height), height, pallete.black };
                sprintf(scratch_buffer, "%d fps", fps);
                real w = _easy_text_dx(&pen, scratch_buffer);
                pen.origin.x = window_get_width_Pixel() - w - eps - eps;
                easy_text_drawf(&pen, scratch_buffer);
            }
        }
    }

}

