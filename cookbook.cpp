

struct Cookbook {
    Event event;
    StandardEventProcessResult *result;
    bool skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack;

    List<Entity> _add_buffer;
    List<uint> _delete_buffer;

    Entity _make_line(vec2 start, vec2 end, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = {};
        entity.type = EntityType::Line;
        entity.preview_color = get_color(ColorCode::Emphasis);
        LineEntity *line = &entity.line;
        line->start = start;
        line->end = end;
        entity.is_selected = is_selected;
        entity.color_code = color_code;
        return entity;
    };

    Entity _make_arc(vec2 center, real radius, real start_angle_in_degrees, real end_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = {};
        entity.type = EntityType::Arc;
        entity.preview_color = get_color(ColorCode::Emphasis);
        ArcEntity *arc = &entity.arc;
        arc->center = center;
        arc->radius = radius;
        arc->start_angle_in_degrees = start_angle_in_degrees;
        arc->end_angle_in_degrees = end_angle_in_degrees;
        entity.is_selected = is_selected;
        entity.color_code = color_code;
        return entity;
    };
    void _add_entity(Entity entity) {
        ASSERT(_delete_buffer.length == 0);
        list_push_back(&drawing->entities, entity);
    };

    void _add_line(vec2 start, vec2 end, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = _make_line(start, end, is_selected, color_code);
        _add_entity(entity);
    };

    void _add_arc(vec2 center, real radius, real start_angle_in_degrees, real end_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = _make_arc(center, radius, start_angle_in_degrees, end_angle_in_degrees, is_selected, color_code);
        _add_entity(entity);
    };

    void _buffer_add_entity(Entity entity) {
        list_push_back(&_add_buffer, entity);
    };

    void buffer_add_line(vec2 start, vec2 end, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = _make_line(start, end, is_selected, color_code);
        _buffer_add_entity(entity);
    };

    void buffer_add_arc(vec2 center, real radius, real start_angle_in_degrees, real end_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = _make_arc(center, radius, start_angle_in_degrees, end_angle_in_degrees, is_selected, color_code);
        _buffer_add_entity(entity);
    };

    // void consume_buffered_adds() {
    // };


    void _delete_entity(uint i) {
        list_delete_at(&drawing->entities, i);
    };

    void _buffer_delete_entity_DEPRECATED_INDEX_VERSION(uint i) {
        list_push_back(&_delete_buffer, i);
    };

    void buffer_delete_entity(Entity *entity) {
        ASSERT(entity > drawing->entities.array);
        uint i = uint(entity - drawing->entities.array);
        _buffer_delete_entity_DEPRECATED_INDEX_VERSION(i);
    };

    static int _compare_delete_buffer(const void *_a, const void *_b) {
        uint a = *((uint *)(_a));
        uint b = *((uint *)(_b));

        if (a > b) return -1;
        if (a < b) return 1;
        return 0;
    }

    void end_frame() {
        { // _delete_buffer
            qsort(
                    _delete_buffer.array,
                    _delete_buffer.length,
                    sizeof(_delete_buffer.array[0]),
                    _compare_delete_buffer
                 );
            for_(i, _delete_buffer.length) _delete_entity(_delete_buffer.array[i]);
            list_free_AND_zero(&_delete_buffer);
        }

        { // _add_buffer
            for_(i, _add_buffer.length) _add_entity(_add_buffer.array[i]);
            list_free_AND_zero(&_add_buffer);
        }
    };

    void entity_set_is_selected(Entity *entity, bool is_selected) {
        if (entity->is_selected != is_selected) {
            result->record_me = true;
            result->checkpoint_me = !((event.type == EventType::Mouse) && (event.mouse_event.mouse_held)); // FORNOW
            entity->is_selected = is_selected;
            entity->time_since_is_selected_changed = 0.0f;
        }
    };

    void set_is_selected_for_all_entities(bool is_selected) {
        _for_each_entity_ entity_set_is_selected(entity, is_selected);
    };

    void entity_set_color(Entity *entity, ColorCode color_code) {
        if (entity->color_code != color_code) {
            result->record_me = true;
            result->checkpoint_me = true;
            entity->color_code = color_code;
        }
    };

    // DOES NOT EXTEND Line
    void divide_entity_at_point(uint entity_index, vec2 point) {
        Entity *entity = &drawing->entities.array[entity_index];
        bool delete_flag = false;
        if (entity->type == EntityType::Line) {
            LineEntity line = entity->line;
            // messagef(omax.orange, "%f", distance(point, line.start) - distance(point, line.
            bool point_is_on_line = ARE_EQUAL(distance(point, line.start) + distance(point, line.end), distance(line.start, line.end)); // FORNOW
            if (point_is_on_line) {
                if (distance(line.start, point) > TINY_VAL) {
                    buffer_add_line(line.start, point, false, entity->color_code);
                    delete_flag = true;
                }
                if (distance(point, line.end) > TINY_VAL) {
                    buffer_add_line(point, line.end, false, entity->color_code);
                    delete_flag = true;
                }
            }
        } else { ASSERT(entity->type == EntityType::Arc);
            ArcEntity arc = entity->arc;
            real angle = DEG(ATAN2(point - arc.center));
            if (abs(distance(point, arc.center) - arc.radius) < 0.001 && ANGLE_IS_BETWEEN_CCW_DEGREES(angle, arc.start_angle_in_degrees, arc.end_angle_in_degrees)) {
                if (!ARE_EQUAL(arc.start_angle_in_degrees, angle)) {
                    buffer_add_arc(arc.center, arc.radius, arc.start_angle_in_degrees, angle, false, entity->color_code);
                    delete_flag = true;
                }
                if (!ARE_EQUAL(arc.end_angle_in_degrees, angle)) {
                    buffer_add_arc(arc.center, arc.radius, angle, arc.end_angle_in_degrees, false, entity->color_code);
                    delete_flag = true;
                }

            }
        }
        if (delete_flag) {
            _buffer_delete_entity_DEPRECATED_INDEX_VERSION(entity_index);
        }
    }

    void attempt_fillet(Entity *E, Entity *F, vec2 reference_point, real radius) {
        if (E == F) {
            messagef(omax.orange, "Fillet: clicked same entity twice");
            return;
        }

        bool is_line_line = (E->type == EntityType::Line) && (F->type == EntityType::Line);
        bool is_line_arc_or_arc_line = (E->type == EntityType::Line && F->type == EntityType::Arc) || (E->type == EntityType::Arc && F->type == EntityType::Line);
        bool is_arc_arc = (E->type == EntityType::Arc && F->type == EntityType::Arc);
        if (is_line_line) {
            //  a -- b   x          a -- B-.  
            //                           |  - 
            //           d    =>         X - D
            //    p      |            p      |
            //           c                   c

            vec2 p = reference_point;
            vec2 a;
            vec2 *b_ptr;
            vec2 c;
            vec2 *d_ptr;
            vec2 x;
            vec2 e_ab;
            vec2 e_cd;
            {
                a     =  E->line.start;
                b_ptr = &E->line.end;
                c     =  F->line.start;
                d_ptr = &F->line.end;

                LineLineXResult _x = line_line_intersection(a, *b_ptr, c, *d_ptr);
                if (_x.lines_are_parallel) {
                    messagef(omax.orange, "Fillet: lines are parallel");
                    return;
                }
                x = _x.point;

                e_ab = normalized(*b_ptr - a);
                e_cd = normalized(*d_ptr - c);

                bool swap_ab, swap_cd; {
                    vec2 v_xp_in_edge_basis = inverse(hstack(e_ab, e_cd)) * (p - x);
                    swap_ab = (v_xp_in_edge_basis.x > 0.0f);
                    swap_cd = (v_xp_in_edge_basis.y > 0.0f);
                }

                if (swap_ab) {
                    {
                        a = *b_ptr;
                        b_ptr = &E->line.start;
                    }
                    e_ab *= -1;
                }

                if (swap_cd) {
                    {
                        c = *d_ptr;
                        d_ptr = &F->line.start;
                    }
                    e_cd *= -1;
                }
            }

            { // overwrite b, d
                real L; {
                    real full_angle = get_three_point_angle(a, x, c);
                    if (full_angle > PI) full_angle = TAU - full_angle;
                    L = radius / TAN(full_angle / 2);
                }
                *b_ptr = x - (L * e_ab);
                *d_ptr = x - (L * e_cd);
            }

            vec2 X; {
                LineLineXResult _X = line_line_intersection(*b_ptr, *b_ptr + perpendicularTo(e_ab), *d_ptr, *d_ptr + perpendicularTo(e_cd));
                if (_X.lines_are_parallel) {
                    messagef(omax.orange, "Fillet: ???");
                    return;
                }
                X = _X.point;
            }

            if (!IS_ZERO(radius)) { // arc
                real theta_b_in_degrees;
                real theta_d_in_degrees;
                {
                    theta_b_in_degrees = DEG(angle_from_0_TAU(X, *b_ptr));
                    theta_d_in_degrees = DEG(angle_from_0_TAU(X, *d_ptr));
                    if (get_three_point_angle(*b_ptr, X, *d_ptr) > PI) {
                        SWAP(&theta_b_in_degrees, &theta_d_in_degrees);
                    }
                }
                buffer_add_arc(X, radius, theta_b_in_degrees, theta_d_in_degrees, false, E->color_code);
            }

            { // aesthetics
                E->preview_color = get_color(ColorCode::Emphasis);
                F->preview_color = get_color(ColorCode::Emphasis);
            }
        } else if (is_line_arc_or_arc_line) {
            // general idea
            // 1. find what quadrant the click is in
            // 2. use that to get the intersect between line and circle
            // 3. ?????
            // 4, perfect fillet

            Entity *EntL = E->type == EntityType::Line ? E : F;
            Entity *EntA  = E->type == EntityType::Arc  ? E : F;

            LineEntity line = EntL->line;
            ArcEntity arc = EntA->arc;

            // get closest intersection point
            // in current version both points can always work
            // this is only checking for the 
            LineArcXClosestResult intersection = line_arc_intersection_closest(&line, &arc, reference_point);

            if (!intersection.no_possible_intersection) {
                // Now have to decide which of the 4 possible fillets to do
                // This currently only depends on the line as the arc can  
                //   wrap both directions
                // Check to see if one predicted by click position works otherwise
                //   fillet from opposite side (inside/outside) of circle
                //
                //           \
                //        B   \   A
                //            |
                //      ------|------
                //            /
                //        C  /   D
                //      

                // in this case we can do any fillet
                // in cases where the radius is massive weird stuff happens
                // thats on the user though, or at least for now
                bool all_fillets_valid = intersection.point_is_on_line_segment;

                // if click is inside the circle when both work
                // TODO: better check for this as a line outside of arc still says outside
                real distance_second_click_center = distance(reference_point, arc.center);
                bool fillet_inside_circle = (all_fillets_valid && distance_second_click_center < arc.radius);

                real start_val = dot(normalized(intersection.point - arc.center), normalized(intersection.point - line.start)); 
                real end_val = dot(normalized(intersection.point - arc.center), normalized(intersection.point - line.end));
                bool start_inside_circle = start_val > -TINY_VAL;
                bool end_inside_circle = end_val > -TINY_VAL;
                if (abs(distance(intersection.point, line.start)) < 0.001f) {
                    start_inside_circle = end_inside_circle;
                }
                if (abs(distance(intersection.point, line.end)) < 0.001f) {
                    end_inside_circle = start_inside_circle;
                }
                if (!(start_inside_circle ^ (end_inside_circle ))) {
                    fillet_inside_circle = end_inside_circle ;
                }

                vec2 line_vector = line.end - line.start;
                bool line_left = cross(line_vector, reference_point - line.start) < 0;
                vec2 line_adjust = radius * normalized(perpendicularTo(line_vector)) * (line_left ? 1 : -1);
                LineEntity new_line; // ! color, etc. undefined
                new_line.start = line.start + line_adjust; 
                new_line.end = line.end + line_adjust; 

                ArcEntity new_arc = arc;
                new_arc.radius += radius * (fillet_inside_circle ? -1 : 1);

                LineArcXClosestResult fillet_point = line_arc_intersection_closest(&new_line, &new_arc, reference_point);

                vec2 fillet_center = fillet_point.point;
                vec2 line_fillet_intersect = fillet_center - line_adjust;
                vec2 arc_fillet_intersect = fillet_center - radius * (fillet_inside_circle ? -1 : 1) * normalized(fillet_center - arc.center);
                real fillet_line_theta = ATAN2(line_fillet_intersect - fillet_center);
                real fillet_arc_theta = ATAN2(arc_fillet_intersect - fillet_center);

                if (fmod(TAU + fillet_line_theta - fillet_arc_theta, TAU) > PI) {
                    real temp = fillet_line_theta;
                    fillet_line_theta = fillet_arc_theta;
                    fillet_arc_theta = temp;
                }

                Entity fillet_arc = _make_arc(fillet_center, radius, DEG(fillet_arc_theta), DEG(fillet_line_theta));
                if (radius > TINY_VAL) {
                    _buffer_add_entity(fillet_arc);
                }
                // TODO: MAKE THIS WORK FOR 0 RADIUS FILLETS
                bool end_in_direction = (dot(normalized(fillet_center - intersection.point), normalized(line.end - intersection.point)) > 0);
                bool start_in_direction = (dot(normalized(fillet_center - intersection.point), normalized(line.start - intersection.point)) > 0);
                bool extend_start;
                if (end_in_direction ^ start_in_direction) {
                    extend_start = end_in_direction;
                } else {
                    if (distance(intersection.point, line.end) > distance(intersection.point, line.start)) {
                        extend_start = true;
                    } else {
                        extend_start = false;
                    }
                }
                if (radius == 0 && (end_inside_circle != start_inside_circle)) {
                    extend_start = fillet_inside_circle != start_inside_circle;
                }
                if (extend_start) {
                    EntL->line.start = line_fillet_intersect;
                } else {
                    EntL->line.end = line_fillet_intersect;
                }

                real divide_theta = DEG(ATAN2(fillet_center - arc.center));
                real theta_where_line_was_tangent = DEG(ATAN2(line_fillet_intersect - arc.center));

                // kinda weird but checks if divide theta > theta where line was tangent
                real offset = DEG(ATAN2(reference_point - arc.center)); 
                vec2 middle_angle_vec = entity_get_middle(&fillet_arc);
                real fillet_middle_arc = DEG(ATAN2(middle_angle_vec - arc.center));
                if (ARE_EQUAL(divide_theta, theta_where_line_was_tangent)) {
                    if (ANGLE_IS_BETWEEN_CCW_DEGREES(offset, divide_theta, divide_theta + 180.0f)) {
                        fillet_middle_arc -= 1.0f;
                    } else {
                        fillet_middle_arc += 1.0f;
                    }
                }
                if (!(ANGLE_IS_BETWEEN_CCW_DEGREES(divide_theta, arc.end_angle_in_degrees - 0.001, arc.end_angle_in_degrees+ 0.001) || ANGLE_IS_BETWEEN_CCW_DEGREES(divide_theta, arc.start_angle_in_degrees - 0.001, arc.start_angle_in_degrees + 0.001))) {
                    //messagef(omax.red, "%d %d", ANGLE_IS_BETWEEN_CCW_DEGREES(divide_theta, arc.end_angle_in_degrees - 0.001, arc.end_angle_in_degrees+ 0.001), ANGLE_IS_BETWEEN_CCW_DEGREES(divide_theta, arc.start_angle_in_degrees - 0.001, arc.start_angle_in_degrees + 0.001) ); 
                    if (ANGLE_IS_BETWEEN_CCW_DEGREES(fillet_middle_arc, arc.start_angle_in_degrees, divide_theta)) {
                        EntA->arc.start_angle_in_degrees = divide_theta;
                    } else {
                        EntA->arc.end_angle_in_degrees = divide_theta;
                    }
                }
            }
        } else { ASSERT(is_arc_arc);
            ArcEntity arc_a = E->arc;
            ArcEntity arc_b = F->arc;
            real _other_fillet_radius = radius + (radius == 0 ? 1 : 0);

            bool fillet_inside_arc_a = distance(arc_a.center, reference_point) < arc_a.radius;
            bool fillet_inside_arc_b = distance(arc_b.center, reference_point) < arc_b.radius;

            ArcEntity new_arc_a = arc_a;
            new_arc_a.radius = arc_a.radius + (fillet_inside_arc_a ? -1 : 1) * _other_fillet_radius;

            ArcEntity new_arc_b = arc_b;
            new_arc_b.radius = arc_b.radius + (fillet_inside_arc_b ? -1 : 1) * _other_fillet_radius;


            ArcArcXClosestResult fillet_point = arc_arc_intersection_closest(&new_arc_a, &new_arc_b, reference_point);

            if (!fillet_point.no_possible_intersection) {
                vec2 fillet_center = fillet_point.point;
                vec2 arc_a_fillet_intersect = fillet_center - _other_fillet_radius * (fillet_inside_arc_a ? -1 : 1) * normalized(fillet_center - arc_a.center);
                vec2 arc_b_fillet_intersect = fillet_center - _other_fillet_radius * (fillet_inside_arc_b ? -1 : 1) * normalized(fillet_center - arc_b.center);
                real fillet_arc_a_theta = ATAN2(arc_a_fillet_intersect - fillet_center);
                real fillet_arc_b_theta = ATAN2(arc_b_fillet_intersect - fillet_center);

                // a swap so the fillet goes the right way
                // (smallest angle
                if (fmod(TAU + fillet_arc_a_theta - fillet_arc_b_theta, TAU) < PI) {
                    real temp = fillet_arc_b_theta;
                    fillet_arc_b_theta = fillet_arc_a_theta;
                    fillet_arc_a_theta = temp;
                }
                Entity fillet_arc = _make_arc(fillet_center, _other_fillet_radius, DEG(fillet_arc_a_theta), DEG(fillet_arc_b_theta)); // if this is changed to radius it breaks, dont ask me why
                if (radius > TINY_VAL) {
                    _buffer_add_entity(fillet_arc);
                }

                real divide_theta_a = DEG(ATAN2(fillet_center - arc_a.center));
                real divide_theta_b = DEG(ATAN2(fillet_center - arc_b.center));
                if (radius == 0) {
                    ArcArcXClosestResult zero_intersect = arc_arc_intersection_closest(&arc_a, &arc_b, reference_point);
                    divide_theta_a = zero_intersect.theta_a;
                    divide_theta_b = zero_intersect.theta_b;
                }

                vec2 middle_angle_vec = entity_get_middle(&fillet_arc);
                real fillet_middle_arc_a = DEG(ATAN2(middle_angle_vec - arc_a.center));
                real fillet_middle_arc_b = DEG(ATAN2(middle_angle_vec - arc_b.center));
                if ((radius == 0) ^ ANGLE_IS_BETWEEN_CCW_DEGREES(fillet_middle_arc_a, arc_a.start_angle_in_degrees, divide_theta_a)) {
                    E->arc.start_angle_in_degrees = divide_theta_a;
                } else {
                    E->arc.end_angle_in_degrees = divide_theta_a;
                }
                if ((radius == 0) ^ ANGLE_IS_BETWEEN_CCW_DEGREES(fillet_middle_arc_b, arc_b.start_angle_in_degrees, divide_theta_b)) {
                    F->arc.start_angle_in_degrees = divide_theta_b;
                } else {
                    F->arc.end_angle_in_degrees = divide_theta_b;

                }
            }
        }
    }

    void attempt_dogear(Entity *E, Entity *F, vec2 reference_point, real radius) {
        if (E == F) {
            messagef(omax.orange, "DogEar: clicked same entity twice");
            return;
        }

        if (IS_ZERO(radius)) {
            messagef(omax.orange, "DogEar: FORNOW: must have non-zero radius");
            return;
        }

        bool is_line_line = (E->type == EntityType::Line) && (F->type == EntityType::Line);
        if (!is_line_line) {
            messagef(omax.orange, "DogEar: only line-line is supported");
            return;
        }

        //                                    ,--.
        //  a -- b      x          a -- b    e     x
        //                                  :   y  :
        //                                  ,      f
        //                   =>              +.__.'
        //    p         d            p             d
        //              |                          |
        //              c                          c

        // FORNOW: this block is repeated from fillet
        vec2 p = reference_point;
        vec2 a;
        vec2 b;
        vec2 c;
        vec2 d;
        vec2 x;
        vec2 e_ab;
        vec2 e_cd;
        // vec2 *b_ptr;
        // vec2 *d_ptr;
        {
            a     =  E->line.start;
            b     =  E->line.end;
            // b_ptr = &E->line.end;
            c     =  F->line.start;
            d     =  F->line.end;
            // d_ptr = &F->line.end;

            LineLineXResult _x = line_line_intersection(a, b, c, d);
            if (_x.lines_are_parallel) {
                messagef(omax.orange, "Fillet: lines are parallel");
                return;
            }
            x = _x.point;

            e_ab = normalized(b - a);
            e_cd = normalized(d - c);

            bool swap_ab, swap_cd; {
                vec2 v_xp_in_edge_basis = inverse(hstack(e_ab, e_cd)) * (p - x);
                swap_ab = (v_xp_in_edge_basis.x > 0.0f);
                swap_cd = (v_xp_in_edge_basis.y > 0.0f);
            }

            if (swap_ab) {
                SWAP(&a, &b);
                e_ab *= -1;
                // b_ptr = &E->line.start;
            }

            if (swap_cd) {
                SWAP(&c, &d);
                e_cd *= -1;
                // d_ptr = &F->line.start;
            }
        }

        vec2 y;
        vec2 e_xy;
        real theta_yx_in_degrees;
        {
            e_xy = -normalized(e_ab + e_cd);
            y = x + radius * e_xy;
            theta_yx_in_degrees = DEG(ATAN2(-e_xy));
        }

        Entity G = _make_arc(y, radius, theta_yx_in_degrees - 180.0f, theta_yx_in_degrees + 180.0f, false, E->color_code);

        vec2 e;
        vec2 f;
        {
            // FORNOW: sloppy (use of a, c is wrong i think)
            LineArcXClosestResult _e = line_arc_intersection_closest(&E->line, &G.arc, a);
            ASSERT(!_e.no_possible_intersection);
            e = _e.point;
            LineArcXClosestResult _f = line_arc_intersection_closest(&F->line, &G.arc, c);
            ASSERT(!_f.no_possible_intersection);
            f = _f.point;
        }

        attempt_fillet(E, &G, e + (e - y), radius);
        attempt_fillet(F, &G, f + (f - y), radius);

        #if 0
        // // single arc version
        _buffer_add_entity(G);
        #else
        // // split arc version
        Entity G1;
        Entity G2;
        {
            G1 = G;
            G2 = G;
            real half_theta_in_degrees =  0.5f * _WRAP_TO_0_360_INTERVAL(G.arc.end_angle_in_degrees - G.arc.start_angle_in_degrees);
            G1.arc.end_angle_in_degrees -= half_theta_in_degrees;
            G2.arc.start_angle_in_degrees = G1.arc.end_angle_in_degrees;
        }
        _buffer_add_entity(G1);
        _buffer_add_entity(G2);
        #endif
    }


    ///////////
    // SCARY //
    ///////////

    void manifold_wrapper() {
        bool add = ((state.enter_mode == EnterMode::ExtrudeAdd) || (state.enter_mode == EnterMode::RevolveAdd));
        if (!skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack) {
            result->record_me = true;
            result->snapshot_me = true;
            result->checkpoint_me = true;
            other.time_since_successful_feature = 0.0f;
            { // result.mesh
                CrossSectionEvenOdd cross_section = cross_section_create_FORNOW_QUADRATIC(&drawing->entities, true);
                Mesh tmp = wrapper_manifold(
                        &state.mesh,
                        cross_section.num_polygonal_loops,
                        cross_section.num_vertices_in_polygonal_loops,
                        cross_section.polygonal_loops,
                        get_M_3D_from_2D(),
                        state.enter_mode,
                        (add) ? popup->extrude_add_out_length : popup->extrude_cut_out_length,
                        (add) ? popup->extrude_add_in_length : popup->extrude_cut_in_length,
                        drawing->origin,
                        drawing->axis_base_point,
                        drawing->axis_angle_from_y);
                cross_section_free(&cross_section);

                mesh_free_AND_zero(&state.mesh); // FORNOW
                state.mesh = tmp; // FORNOW
            }
        }

        // reset some stuff
        state.enter_mode = EnterMode::None;
        set_is_selected_for_all_entities(false);
    };
};

Cookbook make_Cookbook(Event event, StandardEventProcessResult *result, bool skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack) {
    Cookbook cookbook = {};
    cookbook.event = event;
    cookbook.result = result;
    cookbook.skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack = skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack;
    return cookbook;
}

void cookbook_free(Cookbook *cookbook) {
    list_free_AND_zero(&cookbook->_add_buffer);
    list_free_AND_zero(&cookbook->_delete_buffer);
}

