

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

    Entity _make_circle(vec2 center, real radius, bool has_pseudo_point, real pseudo_point_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = {};
        entity.type = EntityType::Circle;
        entity.preview_color = get_color(ColorCode::Emphasis);
        CircleEntity *circle = &entity.circle;
        circle->center = center;
        circle->radius = radius;
        circle->has_pseudo_point = has_pseudo_point;
        circle->pseudo_point_angle_in_degrees = pseudo_point_angle_in_degrees;
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

    void _add_circle(vec2 center, real radius, bool has_pseudo_point, real pseudo_point_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = _make_circle(center, radius, has_pseudo_point, pseudo_point_angle_in_degrees, is_selected, color_code);
        _add_entity(entity);
    };

    void _buffer_add_entity(Entity entity) {
        float LENGTH_CUTOFF = 0.003f;
        if (entity_length(&entity) < LENGTH_CUTOFF) { // TODO: define glorbal const for min len
            messagef("zero length entity not created");
        } else if (entity.type == EntityType::Arc && entity.arc.radius < LENGTH_CUTOFF) {
            messagef("zero length entity not created");
        } else if (entity.type == EntityType::Circle && entity.circle.radius < LENGTH_CUTOFF) {
            messagef("zero length entity not created");
        } else {
            list_push_back(&_add_buffer, entity);
        }
    };

    void buffer_add_line(vec2 start, vec2 end, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = _make_line(start, end, is_selected, color_code);
        _buffer_add_entity(entity);
    };

    void buffer_add_arc(vec2 center, real radius, real start_angle_in_degrees, real end_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = _make_arc(center, radius, start_angle_in_degrees, end_angle_in_degrees, is_selected, color_code);
        _buffer_add_entity(entity);
    };

    void buffer_add_circle(vec2 center, real radius, bool has_pseudo_point, real pseudo_point_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = _make_circle(center, radius, has_pseudo_point, pseudo_point_angle_in_degrees, is_selected, color_code);
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
        ASSERT(entity >= drawing->entities.array);
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
    bool attempt_divide_entity_at_point(Entity *entity, vec2 point) {
        bool delete_flag = false;
        bool pseudo_point_created = false;
        if (entity->type == EntityType::Line) {
            LineEntity *line = &entity->line;
            bool point_is_on_line = 0.001 > ABS(distance(point, line->start) + distance(point, line->end) - distance(line->start, line->end)); // FORNOW
            if (point_is_on_line) {
                if (distance(line->start, point) > TINY_VAL) {
                    buffer_add_line(line->start, point, false, entity->color_code);
                    delete_flag = true;
                }
                if (distance(point, line->end) > TINY_VAL) {
                    buffer_add_line(point, line->end, false, entity->color_code);
                    delete_flag = true;
                }
            }
        } else if (entity->type == EntityType::Arc) {
            ArcEntity *arc = &entity->arc;
            real angle = DEG(ATAN2(point - arc->center));
            if (abs(distance(point, arc->center) - arc->radius) < 0.001 && ANGLE_IS_BETWEEN_CCW_DEGREES(angle, arc->start_angle_in_degrees, arc->end_angle_in_degrees)) {
                if (!ARE_EQUAL(arc->start_angle_in_degrees, angle)) {
                    buffer_add_arc(arc->center, arc->radius, arc->start_angle_in_degrees, angle, false, entity->color_code);
                    delete_flag = true;
                }
                if (!ARE_EQUAL(arc->end_angle_in_degrees, angle)) {
                    buffer_add_arc(arc->center, arc->radius, angle, arc->end_angle_in_degrees, false, entity->color_code);
                    delete_flag = true;
                }

            }
        } else { ASSERT(entity->type == EntityType::Circle);
            CircleEntity *circle = &entity->circle;
            if (squared_distance_point_dxf_circle_entity(point, circle) < TINY_VAL) {
                if (!circle->has_pseudo_point) {
                    circle->has_pseudo_point = true;
                    circle->set_pseudo_point(point);
                    pseudo_point_created = true;
                } else {
                    if (ARE_EQUAL(circle->get_pseudo_point(), point)) {
                        ;
                    } else {
                        delete_flag = true;
                        real radius = distance(circle->center, point);
                        real angle1_in_degrees = circle->pseudo_point_angle_in_degrees;
                        real angle2_in_degrees = DEG(ATAN2(point - circle->center));
                        buffer_add_arc(circle->center, radius, angle1_in_degrees, angle2_in_degrees, false, entity->color_code);
                        buffer_add_arc(circle->center, radius, angle2_in_degrees, angle1_in_degrees, false, entity->color_code);
                    }
                }
            }
        }
        if (delete_flag) {
            buffer_delete_entity(entity);
        }
        return delete_flag || pseudo_point_created;
    }
    void attempt_divide_entity_at_point(uint entity_index, vec2 point) {
        Entity *entity = &drawing->entities.array[entity_index];
        attempt_divide_entity_at_point(entity, point); 
    }

    typedef struct {
        bool fillet_success;
        Entity ent_one;
        Entity ent_two;
        Entity fillet_arc;
    } FilletResult;

    FilletResult preview_fillet(const Entity *EntOne, const Entity *EntTwo, vec2 reference_point, real radius) {

        FilletResult fillet_result = {};
        const Entity *E = EntOne;
        const Entity *F = EntTwo;

        if (E == F) {
            messagef(pallete.orange, "Fillet: clicked same entity twice");
            return fillet_result;
        }

        bool pseudoE = false;
        bool pseudoF = false;
        Entity temp1;
        Entity temp2; // it is 1am my brain is very awake
        if (EntOne->type == EntityType::Circle) { // TODO: do better when awake
            temp1 = _make_arc(EntOne->circle.center, EntOne->circle.radius, 0.0f, 359.99f);
            E = &temp1;
            pseudoE = EntOne->circle.has_pseudo_point;
        }
        if (EntTwo->type == EntityType::Circle) {
            temp2 = _make_arc(EntTwo->circle.center, EntTwo->circle.radius, 0.0f, 359.99f);
            F = &temp2;
            pseudoF = EntTwo->circle.has_pseudo_point;
        }



        bool is_line_line = (E->type == EntityType::Line) && (F->type == EntityType::Line);
        bool is_line_arc_or_arc_line = (E->type == EntityType::Line && F->type == EntityType::Arc) || (E->type == EntityType::Arc && F->type == EntityType::Line);
        bool is_arc_arc = (E->type == EntityType::Arc && F->type == EntityType::Arc);


        if (is_line_line) {
            if (distance(E->line.start, E->line.end) < radius) {
                messagef(pallete.orange, "Fillet: first line too short for given radius");
                return fillet_result;
            }
            if (distance(F->line.start, F->line.end) < radius) {
                messagef(pallete.orange, "Fillet: second line too short for given radius");
                return fillet_result;
            }
        }

        if (is_line_line) {
            //  a -- b   x          a -- B-.  
            //                           |  - 
            //           d    =>         X - D
            //    p      |            p      |
            //           c                   c

            vec2 p = reference_point;
            vec2 a;
            vec2 b;
            vec2 c;
            vec2 d;
            vec2 x;
            vec2 e_ab;
            vec2 e_cd;
            {
                a = E->line.start;
                b = E->line.end;
                c = F->line.start;
                d = F->line.end;

                LineLineXResult _x = line_line_intersection(a, b, c, d);
                if (_x.lines_are_parallel) {
                    messagef(pallete.orange, "Fillet: lines are parallel");
                    return fillet_result;
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
                    {
                        a = b;
                        b = E->line.start;
                    }
                    e_ab *= -1;
                }

                if (swap_cd) {
                    {
                        c = d;
                        d = F->line.start;
                    }
                    e_cd *= -1;
                }
            }

            { // add new lines and remove old ones
                real L; {
                    real full_angle = get_three_point_angle(a, x, c);
                    if (full_angle > PI) full_angle = TAU - full_angle;
                    L = radius / TAN(full_angle / 2);
                }
                b = x - (L * e_ab);
                d = x - (L * e_cd);
                Entity new_E = _make_line(a, b, E->is_selected, E->color_code);
                Entity new_F = _make_line(c, d, F->is_selected, F->color_code);

                // lowkey no idea what this does but copied for consistency 
                new_E.preview_color = get_color(ColorCode::Emphasis);
                new_F.preview_color = get_color(ColorCode::Emphasis);

                fillet_result.ent_one = new_E;
                fillet_result.ent_two = new_F;
            }

            // deal with creating the fillet arc
            vec2 X; {
                LineLineXResult _X = line_line_intersection(b, b + perpendicularTo(e_ab), d, d + perpendicularTo(e_cd));
                if (_X.lines_are_parallel) {
                    messagef(pallete.orange, "Fillet: ???");
                    return fillet_result;
                }
                X = _X.point;
            }

            if (!IS_ZERO(radius)) { // arc
                real theta_b_in_degrees;
                real theta_d_in_degrees;
                {
                    theta_b_in_degrees = DEG(angle_from_0_TAU(X, b));
                    theta_d_in_degrees = DEG(angle_from_0_TAU(X, d));
                    if (get_three_point_angle(b, X, d) > PI) {
                        SWAP(&theta_b_in_degrees, &theta_d_in_degrees);
                    }
                }
                fillet_result.fillet_arc = _make_arc(X, radius, theta_b_in_degrees, theta_d_in_degrees, false, E->color_code);
            } 

        } else if (is_line_arc_or_arc_line) { // this is a very straight forward function
                                              // general idea
                                              // 1. find where relative to line/arc intersection click is
                                              // 2. use that to get the fillet point
                                              // 3. ?????
                                              // 4, perfect fillet

            const Entity *EntL = E->type == EntityType::Line ? E : F;
            LineEntity line = EntL->line;
            bool swap_happened = EntL == E;

            const Entity *EntA = E->type == EntityType::Arc  ? E : F;
            ArcEntity arc = EntA->arc;

            LineArcXClosestResult intersection = line_arc_intersection_closest(&line, &arc, reference_point);

            if (intersection.no_possible_intersection) {
                messagef("FILLET: no intersection found");
                return fillet_result;
            }

            // Determine if fillet should be inside or outside the circle
            real distance_second_click_center = distance(reference_point, arc.center);
            bool fillet_inside_circle = intersection.point_is_on_line_segment && (distance_second_click_center < arc.radius);

            // Get a line parallel to selected to determine where the fillet arc should be
            vec2 line_vector = line.end - line.start;
            bool line_left = cross(line_vector, reference_point - line.start) < 0;
            vec2 line_adjust = radius * normalized(perpendicularTo(line_vector)) * (line_left ? 1.0f : -1.0f);

            LineEntity new_line; // ! color, etc. undefined
            new_line.start = line.start + line_adjust; 
            new_line.end = line.end + line_adjust; 

            // Same thing but for the arc 
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
            if (start_inside_circle == end_inside_circle) { 
                fillet_inside_circle = end_inside_circle;
            }

            ArcEntity new_arc = arc;
            new_arc.radius += radius * (fillet_inside_circle ? -1 : 1);

            // calculate fillet center and intersections
            LineArcXClosestResult fillet_point = line_arc_intersection_closest(&new_line, &new_arc, reference_point);
            vec2 fillet_center = fillet_point.point;
            vec2 line_fillet_intersect = fillet_center - line_adjust;
            vec2 arc_fillet_intersect = fillet_center - radius * (fillet_inside_circle ? -1 : 1) * normalized(fillet_center - arc.center);

            // calculate fillet angles
            real fillet_line_theta = ATAN2(line_fillet_intersect - fillet_center);
            real fillet_arc_theta = ATAN2(arc_fillet_intersect - fillet_center);

            if (fmod(TAU + fillet_line_theta - fillet_arc_theta, TAU) > PI) {
                real temp = fillet_line_theta;
                fillet_line_theta = fillet_arc_theta;
                fillet_arc_theta = temp;
            }

            // make fillet arc
            Entity fillet_arc = _make_arc(fillet_center, radius, DEG(fillet_arc_theta), DEG(fillet_line_theta), false, E->color_code);
            fillet_result.fillet_arc = fillet_arc;

            // determine which end of the line should be changed
            bool end_in_direction = (dot(normalized(fillet_center - intersection.point), normalized(line.end - intersection.point)) > 0);
            bool start_in_direction = (dot(normalized(fillet_center - intersection.point), normalized(line.start - intersection.point)) > 0);
            bool extend_start;
            if (end_in_direction != start_in_direction) {
                extend_start = end_in_direction;
            } else {
                extend_start = distance(intersection.point, line.end) > distance(intersection.point, line.start);
            }

            // handle zero radius case
            if (radius == 0 && (end_inside_circle != start_inside_circle)) {
                extend_start = fillet_inside_circle != start_inside_circle;
            }

            Entity line_to_add;
            // add the new line
            if (extend_start) {
                line_to_add = _make_line(line_fillet_intersect, EntL->line.end, EntL->is_selected, EntL->color_code);
            } else {
                line_to_add = _make_line(EntL->line.start, line_fillet_intersect, EntL->is_selected, EntL->color_code);
            }


            // arc stuff
            real divide_theta = DEG(ATAN2(fillet_center - arc.center));
            real theta_where_line_was_tangent = DEG(ATAN2(line_fillet_intersect - arc.center));

            // kinda weird but checks if divide theta > theta where line was tangent
            vec2 middle_angle_vec = entity_get_middle(&fillet_arc);
            real fillet_middle_arc = DEG(ATAN2(middle_angle_vec - arc.center));

            // this is a slight nudge to ensure that the correct angle is adjusted
            if (ARE_EQUAL(divide_theta, theta_where_line_was_tangent)) {
                real offset = DEG(ATAN2(reference_point - arc.center)); 
                fillet_middle_arc += ANGLE_IS_BETWEEN_CCW_DEGREES(offset, divide_theta, divide_theta + 180.0f) ? -1.0f : 1.0f; 
            }

            // good luck
            const Entity *arc_or_circle = swap_happened ? EntTwo : EntOne;
            Entity new_arc_or_circle;
            if (arc_or_circle->type == EntityType::Circle && !arc_or_circle->circle.has_pseudo_point) {
                new_arc_or_circle = _make_circle(arc_or_circle->circle.center, arc_or_circle->circle.radius, true, divide_theta, arc_or_circle->is_selected, arc_or_circle->color_code);
            } else {
                real start_angle = arc_or_circle->type == EntityType::Circle ? arc_or_circle->circle.pseudo_point_angle_in_degrees : arc_or_circle->arc.start_angle_in_degrees;
                real end_angle = arc_or_circle->type == EntityType::Circle ? arc_or_circle->circle.pseudo_point_angle_in_degrees : arc_or_circle->arc.end_angle_in_degrees;
                if (!(ANGLE_IS_BETWEEN_CCW_DEGREES(divide_theta, arc.end_angle_in_degrees - 0.001f, arc.end_angle_in_degrees + 0.001f) || 
                            ANGLE_IS_BETWEEN_CCW_DEGREES(divide_theta, arc.start_angle_in_degrees - 0.001f, arc.start_angle_in_degrees + 0.001f))) {
                    if (ANGLE_IS_BETWEEN_CCW_DEGREES(fillet_middle_arc, arc.start_angle_in_degrees, divide_theta)) {
                        new_arc_or_circle = _make_arc(arc.center, arc.radius, divide_theta, end_angle, arc_or_circle->is_selected, arc_or_circle->color_code);
                    } else {
                        new_arc_or_circle = _make_arc(arc.center, arc.radius, start_angle, divide_theta, arc_or_circle->is_selected, arc_or_circle->color_code);
                    }
                }
            }

            fillet_result.ent_one = swap_happened ? line_to_add : new_arc_or_circle;
            fillet_result.ent_two = swap_happened ? new_arc_or_circle : line_to_add;

        } else { ASSERT(is_arc_arc);
            ArcEntity arc_a = E->arc;
            ArcEntity arc_b = F->arc;
            real _other_fillet_radius = radius + (radius == 0 ? .001 : 0);

            bool fillet_inside_arc_a = distance(arc_a.center, reference_point) < arc_a.radius;
            bool fillet_inside_arc_b = distance(arc_b.center, reference_point) < arc_b.radius;

            ArcEntity new_arc_a = arc_a;
            new_arc_a.radius = arc_a.radius + (fillet_inside_arc_a ? -1 : 1) * _other_fillet_radius;

            ArcEntity new_arc_b = arc_b;
            new_arc_b.radius = arc_b.radius + (fillet_inside_arc_b ? -1 : 1) * _other_fillet_radius;


            ArcArcXClosestResult fillet_point = arc_arc_intersection_closest(&new_arc_a, &new_arc_b, reference_point);

            if (fillet_point.no_possible_intersection) {
                messagef("FILLET: no intersection found");
                return fillet_result;
            }

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
            Entity fillet_arc = _make_arc(fillet_center, _other_fillet_radius, DEG(fillet_arc_a_theta), DEG(fillet_arc_b_theta), false, E->color_code); // if this is changed to radius it breaks, dont ask me why
            if (radius > TINY_VAL) {
                fillet_result.fillet_arc = fillet_arc;
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

            Entity ent_one_to_add;
            if (EntOne->type == EntityType::Circle && !pseudoE) {
                ent_one_to_add = _make_circle(EntOne->circle.center, EntOne->circle.radius, true, divide_theta_a, EntOne->is_selected, EntOne->color_code); 
            } else {
                real start_angle = EntOne->type == EntityType::Circle ? EntOne->circle.pseudo_point_angle_in_degrees : E->arc.start_angle_in_degrees;
                real end_angle = EntOne->type == EntityType::Circle ? EntOne->circle.pseudo_point_angle_in_degrees : E->arc.end_angle_in_degrees;
                if ((radius == 0) != ANGLE_IS_BETWEEN_CCW_DEGREES(fillet_middle_arc_a, arc_a.start_angle_in_degrees, divide_theta_a)) {
                    ent_one_to_add = _make_arc(E->arc.center, E->arc.radius, divide_theta_a, end_angle, E->is_selected, E->color_code);
                } else {
                    ent_one_to_add = _make_arc(E->arc.center, E->arc.radius, start_angle, divide_theta_a, E->is_selected, E->color_code);
                }
            }
            fillet_result.ent_one = ent_one_to_add;


            Entity ent_two_to_add;
            if (EntTwo->type == EntityType::Circle && !pseudoF) {
                ent_two_to_add = _make_circle(EntTwo->circle.center, EntTwo->circle.radius, true, divide_theta_b, EntTwo->is_selected, EntTwo->color_code); 
            } else {
                real start_angle_in_degrees = EntTwo->type == EntityType::Circle ? EntTwo->circle.pseudo_point_angle_in_degrees : F->arc.start_angle_in_degrees;
                real end_angle_in_degrees = EntTwo->type == EntityType::Circle ? EntTwo->circle.pseudo_point_angle_in_degrees : F->arc.end_angle_in_degrees;
                if ((radius == 0) != ANGLE_IS_BETWEEN_CCW_DEGREES(fillet_middle_arc_b, arc_b.start_angle_in_degrees, divide_theta_b)) {
                    ent_two_to_add = _make_arc(F->arc.center, F->arc.radius, divide_theta_b, end_angle_in_degrees, F->is_selected, F->color_code);
                } else {
                    ent_two_to_add = _make_arc(F->arc.center, F->arc.radius, start_angle_in_degrees, divide_theta_b, F->is_selected, F->color_code);
                }
            }
            fillet_result.ent_two = ent_two_to_add;

        }

        // least sus thing ever
        fillet_result.fillet_success = true;

        return fillet_result;
    }

    void attempt_dogear(Entity *E, Entity *F, vec2 reference_point, real radius) {
        if (E == F) {
            messagef(pallete.orange, "DogEar: clicked same entity twice");
            return;
        }

        if (IS_ZERO(radius)) {
            messagef(pallete.orange, "DogEar: FORNOW: must have non-zero radius");
            return;
        }

        bool is_line_line = (E->type == EntityType::Line) && (F->type == EntityType::Line);
        if (!is_line_line) {
            messagef(pallete.orange, "DogEar: only line-line is supported");
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
                messagef(pallete.orange, "DogEar: lines are parallel");
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

        FilletResult fillet_one = preview_fillet(E, &G, e + (e - y), radius);
        FilletResult fillet_two = preview_fillet(F, &fillet_one.ent_two, f + (f - y), radius);

        _buffer_add_entity(fillet_one.ent_one);
        _buffer_add_entity(fillet_one.fillet_arc);
        _buffer_add_entity(fillet_two.ent_one);
        _buffer_add_entity(fillet_two.fillet_arc);


        #if 0
        // // single arc version
        _buffer_add_entity(G);
        #else
        // // split arc version
        Entity G1;
        Entity G2;
        {
            G1 = fillet_two.ent_two;
            G2 = fillet_two.ent_two;
            real half_theta_in_degrees =  0.5f * _WRAP_TO_0_360_INTERVAL(fillet_two.ent_two.arc.end_angle_in_degrees - fillet_two.ent_two.arc.start_angle_in_degrees);
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
        bool add = ((state_Mesh_command_is_(ExtrudeAdd)) || (state_Mesh_command_is_(RevolveAdd)));
        bool revolve = ((state_Mesh_command_is_(RevolveAdd)) || (state_Mesh_command_is_(RevolveCut)));
        if (!skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack) {
            result->record_me = true;
            result->snapshot_me = true;
            result->checkpoint_me = true;
            other.time_since_successful_feature = 0.0f;

            real out_quantity;
            real in_quantity;
            {
                if (!revolve) { // extrude
                    out_quantity = (add) ? popup->extrude_add_out_length : popup->extrude_cut_out_length;
                    in_quantity = (add) ? popup->extrude_add_in_length : popup->extrude_cut_in_length;
                } else {
                    out_quantity = (add) ? popup->revolve_add_out_angle : popup->revolve_cut_out_angle;
                    in_quantity = (add) ? popup->revolve_add_in_angle : popup->revolve_cut_in_angle;
                }

            }

            { // result.mesh
                CrossSectionEvenOdd cross_section = cross_section_create_FORNOW_QUADRATIC(&drawing->entities, true);
                Meshes tmp = wrapper_manifold(
                        &state.meshes,
                        cross_section.num_polygonal_loops,
                        cross_section.num_vertices_in_polygonal_loops,
                        cross_section.polygonal_loops,
                        get_M_3D_from_2D(),
                        state.Mesh_command,
                        out_quantity,
                        in_quantity,
                        drawing->origin,
                        drawing->axis_base_point,
                        drawing->axis_angle_from_y);
                cross_section_free(&cross_section);

                meshes_free_AND_zero(&state.meshes); // FORNOW
                state.meshes = tmp; // FORNOW
            }
        }

        // reset some stuff
        set_state_Mesh_command(None);
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

