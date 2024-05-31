/////////
// DXF //
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

auto _ARC = [&](vec2 center, real32 radius, real32 start_angle_in_degrees, real32 end_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
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
    list_push_back(&dxf->entities, entity);
};

auto ADD_LINE_ENTITY = [&](vec2 start, vec2 end, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
    Entity entity = _LINE(start, end, is_selected, color_code);
    _ADD_ENTITY(entity);
};

auto ADD_ARC_ENTITY = [&](vec2 center, real32 radius, real32 start_angle_in_degrees, real32 end_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
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

auto BUFFER_ARC_ENTITY = [&](vec2 center, real32 radius, real32 start_angle_in_degrees, real32 end_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
    Entity entity = _ARC(center, radius, start_angle_in_degrees, end_angle_in_degrees, is_selected, color_code);
    _BUFFER_ENTITY(entity);
};

auto ADD_BUFFERED_ENTITIES = [&]() {
    for (uint i = 0; i < _entity_buffer.length; ++i)  _ADD_ENTITY(_entity_buffer.array[i]);
};


auto _REMOVE_ENTITY = [&](uint i) { list_delete_at(&dxf->entities, i); };

// TODO: sort
// NOTE: arena could be cool here
// List<uint> _entity_buffer = {};
// defer { list_free_AND_zero(&_entity_buffer); };
// auto QUEUE_REMOVAL = [&](uint i) { list_delete_at(&dxf->entities, i); };

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
// POPUP //
///////////

auto POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER = [&]() {
    uint d = popup->active_cell_index;
    memset(popup->active_cell_buffer, 0, POPUP_CELL_LENGTH);
    if (popup->cell_type[d] == CellType::Real32) {
        sprintf(popup->active_cell_buffer, "%g", *((real32 *) popup->value[d]));
    } else { ASSERT(popup->cell_type[d] == CellType::String);
        strcpy(popup->active_cell_buffer, (char *) popup->value[d]);
    }
};

auto POPUP_WRITE_ACTIVE_CELL_BUFFER_INTO_CORRESPONDING_VALUE = [&]() {
    uint d = popup->active_cell_index;
    if (popup->cell_type[d] == CellType::Real32) {
        *((real32 *) popup->value[d]) = strtof(popup->active_cell_buffer, NULL);
    } else { ASSERT(popup->cell_type[d] == CellType::String);
        strcpy((char *) popup->value[d], popup->active_cell_buffer);
    }
};

auto POPUP_CLEAR_ALL_VALUES_TO_ZERO = [&]() {
    for (uint d = 0; d < popup->num_cells; ++d) {
        if (!popup->name[d]) continue;
        if (popup->cell_type[d] == CellType::Real32) {
            *((real32 *) popup->value[d]) = 0.0f;
        } else { ASSERT(popup->cell_type[d] == CellType::String);
            memset(popup->value[d], 0, POPUP_CELL_LENGTH);
        }
    }
};

auto POPUP_SELECTION_NOT_ACTIVE = [&]() -> bool { return (popup->selection_cursor == popup->cursor); };

auto POPUP_SET_ACTIVE_CELL_INDEX = [&](uint new_active_cell_index) {
    popup->active_cell_index = new_active_cell_index;
    POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER();
    popup->cursor = (uint) strlen(popup->active_cell_buffer);
    popup->selection_cursor = 0; // select whole cell
    popup->_type_of_active_cell = popup->cell_type[popup->active_cell_index];
};

///////////
// SCARY //
///////////

auto GENERAL_PURPOSE_MANIFOLD_WRAPPER = [&]() {
    bool add = ((*enter_mode == EnterMode::ExtrudeAdd) || (*enter_mode == EnterMode::RevolveAdd));
    if (!skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack) {
        result.record_me = true;
        result.snapshot_me = true;
        result.checkpoint_me = true;
        time_since->successful_feature = 0.0f;
        { // result.mesh
            CrossSectionEvenOdd cross_section = cross_section_create_FORNOW_QUADRATIC(&WORLD.dxf.entities, true);
            Mesh tmp = wrapper_manifold(
                    &WORLD.mesh,
                    cross_section.num_polygonal_loops,
                    cross_section.num_vertices_in_polygonal_loops,
                    cross_section.polygonal_loops,
                    get_M_3D_from_2D(),
                    *enter_mode,
                    (add) ? WORLD.popup.extrude_add_out_length : WORLD.popup.extrude_cut_out_length,
                    (add) ? WORLD.popup.extrude_add_in_length : WORLD.popup.extrude_cut_in_length,
                    WORLD.dxf.origin,
                    WORLD.dxf.axis_base_point,
                    WORLD.dxf.axis_angle_from_y);
            cross_section_free(&cross_section);

            mesh_free_AND_zero(&WORLD.mesh); // FORNOW
            WORLD.mesh = tmp; // FORNOW
        }
    }

    // reset some stuff
    *enter_mode = EnterMode::None;
    CLEAR_SELECTION_MASK_TO(false);
};
