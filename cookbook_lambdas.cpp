/////////
// Drawing //
/////////

auto _LINE = [&](vec2 start, vec2 end, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
    Entity entity = {};
    entity.type = EntityType::Line;
    LineEntity *line_entity = &entity.line_entity;
    line_entity->start = start;
    line_entity->end = end;
    entity.is_selected = is_selected;
    entity.color_code = color_code;
    return entity;
};

auto _ARC = [&](vec2 center, real radius, real start_angle_in_degrees, real end_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
    Entity entity = {};
    entity.type = EntityType::Arc;
    ArcEntity *arc_entity = &entity.arc_entity;
    arc_entity->center = center;
    arc_entity->radius = radius;
    arc_entity->start_angle_in_degrees = start_angle_in_degrees;
    arc_entity->end_angle_in_degrees = end_angle_in_degrees;
    entity.is_selected = is_selected;
    entity.color_code = color_code;
    return entity;
};
auto _ADD_ENTITY = [&](Entity entity) {
    list_push_back(&drawing->entities, entity);
};

auto ADD_LINE_ENTITY = [&](vec2 start, vec2 end, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
    Entity entity = _LINE(start, end, is_selected, color_code);
    _ADD_ENTITY(entity);
};

auto ADD_ARC_ENTITY = [&](vec2 center, real radius, real start_angle_in_degrees, real end_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
    Entity entity = _ARC(center, radius, start_angle_in_degrees, end_angle_in_degrees, is_selected, color_code);
    _ADD_ENTITY(entity);
};

List<Entity> _entity_buffer = {};
defer { list_free_AND_zero(&_entity_buffer); };

auto _BUFFER_ENTITY = [&](Entity entity) {
    list_push_back(&_entity_buffer, entity);
};

auto BUFFER_LINE_ENTITY = [&](vec2 start, vec2 end, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
    Entity entity = _LINE(start, end, is_selected, color_code);
    _BUFFER_ENTITY(entity);
};

auto BUFFER_ARC_ENTITY = [&](vec2 center, real radius, real start_angle_in_degrees, real end_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
    Entity entity = _ARC(center, radius, start_angle_in_degrees, end_angle_in_degrees, is_selected, color_code);
    _BUFFER_ENTITY(entity);
};

auto ADD_BUFFERED_ENTITIES = [&]() {
    for_(i, _entity_buffer.length)  _ADD_ENTITY(_entity_buffer.array[i]);
};


auto _REMOVE_ENTITY = [&](uint i) { list_delete_at(&drawing->entities, i); };

// TODO: sort
// NOTE: arena could be cool here
// List<uint> _entity_buffer = {};
// defer { list_free_AND_zero(&_entity_buffer); };
// auto QUEUE_REMOVAL = [&](uint i) { list_delete_at(&drawing->entities, i); };

auto ENTITY_SET_IS_SELECTED = [&](Entity *entity, bool is_selected) {
    if (entity->is_selected != is_selected) {
        result.record_me = true;
        result.checkpoint_me = !((event.type == EventType::Mouse) && (event.mouse_event.mouse_held)); // FORNOW
        entity->is_selected = is_selected;
        entity->time_since_is_selected_changed = 0.0f;
    }
};

auto CLEAR_SELECTION_MASK_TO = [&](bool is_selected) {
    _for_each_entity_ ENTITY_SET_IS_SELECTED(entity, is_selected);
};

auto ENTITY_SET_COLOR = [&](Entity *entity, ColorCode color_code) {
    if (entity->color_code != color_code) {
        result.record_me = true;
        result.checkpoint_me = true;
        entity->color_code = color_code;
    }
};

///////////
// SCARY //
///////////

auto GENERAL_PURPOSE_MANIFOLD_WRAPPER = [&]() {
    bool add = ((state.enter_mode == EnterMode::ExtrudeAdd) || (state.enter_mode == EnterMode::RevolveAdd));
    if (!skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack) {
        result.record_me = true;
        result.snapshot_me = true;
        result.checkpoint_me = true;
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
    CLEAR_SELECTION_MASK_TO(false);
};