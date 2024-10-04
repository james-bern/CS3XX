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
        circle->pseudo_point_angle += theta;
    }
    return result;
}


#if 0
Entity entity_mirrored(const Entity *result, vec2 origin, real axis_angle) {
    return {};
}


Entity entity_offset(Entity *result) {
    return {};
}
#endif
