// TODO: move make's out of cookbook and into here
// TODO: entity_offseted (reference point)

Entity entity_make_three_point_arc_or_line(vec2 a, vec2 b, vec2 c) {
    Entity result = {};
    if (false) { // full circle
        result.type = EntityType::Circle;
        result.circle.center = AVG(a, b);
        result.circle.radius = distance(a, result.circle.center);
    } else {
        vec2 p1 = AVG(a, b);
        vec2 p2 = AVG(b, c);
        vec2 n1 = normalized(perpendicularTo(b - a));
        vec2 n2 = normalized(perpendicularTo(c - b));
        LineLineXResult intersection_result = line_line_intersection(p1, p1 + n1, p2, p2 + n2);
        if (intersection_result.lines_are_parallel) { // Line
            result.type = EntityType::Line;
            result.line.start = a;
            result.line.end   = c;
        } else {
            result.type = EntityType::Arc;
            result.arc.center = intersection_result.point;
            result.arc.radius = distance(a, result.arc.center);
            result.arc.start_angle_in_degrees = DEG(ATAN2(c - result.arc.center));
            result.arc.end_angle_in_degrees   = DEG(ATAN2(a - result.arc.center));
            if (intersection_result.t_ab < 0.0) {
                SWAP(&result.arc.start_angle_in_degrees, &result.arc.end_angle_in_degrees);
            }
        }
    }
    return result;
}

Entity entity_translated(const Entity *_entity, vec2 translation_vector) {
    Entity result = *_entity;
    if (result.type == EntityType::Line) {
        LineEntity *line = &result.line;
        line->start += translation_vector;
        line->end   += translation_vector;
    } else if (result.type == EntityType::Arc) {
        ArcEntity *arc = &result.arc;
        arc->center += translation_vector;
    } else { ASSERT(result.type == EntityType::Circle);
        CircleEntity *circle = &result.circle;
        circle->center += translation_vector;
    }
    return result;
}

Entity entity_rotated(const Entity *_entity, vec2 center, real theta) {
    Entity result = *_entity;
    if (result.type == EntityType::Line) {
        LineEntity *line = &result.line;
        line->start = rotated_about(line->start, center, theta);
        line->end   = rotated_about(line->end,   center, theta);
    } else if (result.type == EntityType::Arc) {
        ArcEntity *arc = &result.arc;
        arc->center = rotated_about(arc->center, center, theta);
        arc->start_angle_in_degrees += DEG(theta);
        arc->end_angle_in_degrees   += DEG(theta);
    } else { ASSERT(result.type == EntityType::Circle);
        CircleEntity *circle = &result.circle;
        circle->center = rotated_about(circle->center, center, theta);
        circle->pseudo_point_angle_in_degrees += DEG(theta);
    }
    return result;
}

Entity entity_scaled(const Entity *_entity, vec2 center, real factor) {
    Entity result = *_entity;
    if (result.type == EntityType::Line) {
        LineEntity *line = &result.line;
        line->start = scaled_about(line->start, center, factor);
        line->end = scaled_about(line->end, center, factor);
    } else if (result.type == EntityType::Arc) {
        ArcEntity *arc = &result.arc;
        arc->center = scaled_about(arc->center, center, factor);
        arc->radius *= factor;
    } else { ASSERT(result.type == EntityType::Circle);
        CircleEntity *circle = &result.circle;
        circle->center = scaled_about(circle->center, center, factor);
        circle->radius *= factor;
    }
    return result;
}

Entity entity_offsetted(const Entity *_entity, real offset_distance, vec2 reference_point) {
    Entity result = *_entity;
    if (result.type == EntityType::Line) {
        LineEntity *line = &result.line;
        vec2 normal = normalized(perpendicularTo(line->end - line->start));
        real sign = (dot(normal, reference_point - line->start) < 0.0) ? -1 : 1;
        vec2 offset = sign * offset_distance * normal;
        line->start += offset;
        line->end += offset;
    } else if (result.type == EntityType::Arc) {
        ArcEntity *arc = &result.arc;
        bool in_circle = (distance(arc->center, reference_point) < arc->radius);
        bool in_sector = false;
        if (!in_circle) {
            // TODO: comment (diagram)
            vec2 start_point = entity_get_start_point(&result);
            vec2 end_point = entity_get_end_point(&result);
            vec2 perp_end = perpendicularTo(end_point - arc->center);
            vec2 perp_start = perpendicularTo(start_point - arc->center);
            vec2 end_to_mouse = reference_point - end_point;
            vec2 start_to_mouse = reference_point - start_point;
            real end_cross_p = cross(end_to_mouse, perp_end);
            real start_cross_p = cross(start_to_mouse, perp_start);
            real diam_cross_p = cross(end_to_mouse, start_point - end_point);
            in_sector = (end_cross_p > 0) && (start_cross_p > 0) && (diam_cross_p > 0);
        }
        int sign = (in_circle || in_sector) ? -1 : 1;
        arc->radius += sign * offset_distance;
    } else { ASSERT(result.type == EntityType::Circle);
        CircleEntity *circle = &result.circle;
        bool in_circle = distance(circle->center, reference_point) < circle->radius;
        int sign = (!in_circle) ? 1 : -1;
        circle->radius += sign * offset_distance;
    }
    return result;
}

#if 0
Entity entity_mirrored(const Entity *result, vec2 origin, real axis_angle) {
    return {};
}


#endif
