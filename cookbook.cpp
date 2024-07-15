struct Cookbook {
    Event event;
    StandardEventProcessResult *result;
    bool skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack;

    List<Entity> _add_buffer;
    List<uint> _delete_buffer;

    Entity _make_line(vec2 start, vec2 end, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = {};
        entity.type = EntityType::Line;
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
        _buffer_delete_entity_DEPRECATED_INDEX_VERSION(uint(entity - drawing->entities.array));
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

    // DOES NOT EXTEND LINE
    void divide_entity_at_point(Entity *entity, vec2 point) {
        bool delete_flag = false;
        if (entity->type == EntityType::Line) {
            LineEntity line = entity->line;
            if (ARE_EQUAL(distance(point, line.start), distance(point, line.end))) {
                if (distance(line.start, point) > TINY_VAL) {
                    buffer_add_line(line.start, point, entity->is_selected, entity->color_code);
                    delete_flag = true;
                }
                if (distance(point, line.end) > TINY_VAL) {
                    buffer_add_line(point, line.end, entity->is_selected, entity->color_code);
                    delete_flag = true;
                }
            }
        } else { ASSERT(entity->type == EntityType::Arc);
            ArcEntity arc = entity->arc;
            real angle = DEG(ATAN2(point - arc.center));
            if (ARE_EQUAL(distance(point, arc.center), arc.radius) && ANGLE_IS_BETWEEN_CCW_DEGREES(angle, arc.start_angle_in_degrees, arc.end_angle_in_degrees)) {
                if (!ARE_EQUAL(arc.start_angle_in_degrees, angle)) {
                    buffer_add_arc(arc.center, arc.radius, arc.start_angle_in_degrees, angle);
                    delete_flag = true;
                }
                if (!ARE_EQUAL(arc.end_angle_in_degrees, angle)) {
                    buffer_add_arc(arc.center, arc.radius, angle, arc.end_angle_in_degrees);
                    delete_flag = true;
                }

            }
        }
        if (delete_flag) {
            buffer_delete_entity(entity);
        }
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

