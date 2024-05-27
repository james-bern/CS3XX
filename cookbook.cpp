/////////
// DXF //
/////////

auto _DXF_LINE = [&](vec2 start, vec2 end, bool32 is_selected = false, uint32 color_code = DXF_COLOR_CODE_TRAVERSE) {
    DXFEntity result = {};
    result.type = DXF_ENTITY_TYPE_LINE;
    DXFLine *line = &result.line;
    line->start = start;
    line->end = end;
    result.is_selected = is_selected;
    result.color_code = color_code;
    return result;
};

auto _DXF_ARC = [&](vec2 center, real32 radius, real32 start_angle_in_degrees, real32 end_angle_in_degrees, bool32 is_selected = false, uint32 color_code = DXF_COLOR_CODE_TRAVERSE) {
    DXFEntity result = {};
    result.type = DXF_ENTITY_TYPE_ARC;
    DXFArc *arc = &result.arc;
    arc->center = center;
    arc->radius = radius;
    arc->start_angle_in_degrees = start_angle_in_degrees;
    arc->end_angle_in_degrees = end_angle_in_degrees;
    result.is_selected = is_selected;
    result.color_code = color_code;
    return result;
};
auto _DXF_ADD_ENTITY = [&](DXFEntity entity) {
    list_push_back(&dxf->entities, entity);
};

auto DXF_ADD_LINE = [&](vec2 start, vec2 end, bool32 is_selected = false, uint32 color_code = DXF_COLOR_CODE_TRAVERSE) {
    DXFEntity entity = _DXF_LINE(start, end, is_selected, color_code);
    _DXF_ADD_ENTITY(entity);
};

auto DXF_ADD_ARC = [&](vec2 center, real32 radius, real32 start_angle_in_degrees, real32 end_angle_in_degrees, bool32 is_selected = false, uint32 color_code = DXF_COLOR_CODE_TRAVERSE) {
    DXFEntity entity = _DXF_ARC(center, radius, start_angle_in_degrees, end_angle_in_degrees, is_selected, color_code);
    _DXF_ADD_ENTITY(entity);
};

List<DXFEntity> _entity_buffer = {};
defer { list_free_AND_zero(&_entity_buffer); };

auto _DXF_BUFFER_ENTITY = [&](DXFEntity entity) {
    list_push_back(&_entity_buffer, entity);
};

auto DXF_BUFFER_LINE = [&](vec2 start, vec2 end, bool32 is_selected = false, uint32 color_code = DXF_COLOR_CODE_TRAVERSE) {
    DXFEntity entity = _DXF_LINE(start, end, is_selected, color_code);
    _DXF_BUFFER_ENTITY(entity);
};

auto DXF_BUFFER_ARC = [&](vec2 center, real32 radius, real32 start_angle_in_degrees, real32 end_angle_in_degrees, bool32 is_selected = false, uint32 color_code = DXF_COLOR_CODE_TRAVERSE) {
    DXFEntity entity = _DXF_ARC(center, radius, start_angle_in_degrees, end_angle_in_degrees, is_selected, color_code);
    _DXF_BUFFER_ENTITY(entity);
};

auto DXF_ADD_BUFFERED_ENTITIES = [&]() {
    for (uint32 i = 0; i < _entity_buffer.length; ++i)  _DXF_ADD_ENTITY(_entity_buffer.array[i]);
};


auto _DXF_REMOVE_ENTITY = [&](uint32 i) { list_delete_at(&dxf->entities, i); };

// TODO: sort
// NOTE: arena could be cool here
// List<uint32> _entity_buffer = {};
// defer { list_free_AND_zero(&_entity_buffer); };
// auto DXF_QUEUE_REMOVAL = [&](uint32 i) { list_delete_at(&dxf->entities, i); };

auto DXF_ENTITY_SET_IS_SELECTED = [&](DXFEntity *entity, bool32 is_selected) {
    if (entity->is_selected != is_selected) {
        result.record_me = true;
        result.checkpoint_me = (!event.mouse_held);
        entity->is_selected = is_selected;
        entity->time_since_is_selected_changed = 0.0f;
    }
};

auto DXF_CLEAR_SELECTION_MASK_TO = [&](bool32 is_selected) {
    _for_each_entity_ DXF_ENTITY_SET_IS_SELECTED(entity, is_selected);
};

auto DXF_ENTITY_SET_COLOR = [&](DXFEntity *entity, uint32 color_code) {
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
    uint32 d = popup->active_cell_index;
    memset(popup->active_cell_buffer, 0, POPUP_CELL_LENGTH);
    if (popup->cell_type[d] == POPUP_CELL_TYPE_REAL32) {
        sprintf(popup->active_cell_buffer, "%g", *((real32 *) popup->value[d]));
    } else { ASSERT(popup->cell_type[d] == POPUP_CELL_TYPE_CSTRING);
        strcpy(popup->active_cell_buffer, (char *) popup->value[d]);
    }
};

auto POPUP_WRITE_ACTIVE_CELL_BUFFER_INTO_CORRESPONDING_VALUE = [&]() {
    uint32 d = popup->active_cell_index;
    if (popup->cell_type[d] == POPUP_CELL_TYPE_REAL32) {
        *((real32 *) popup->value[d]) = strtof(popup->active_cell_buffer, NULL);
    } else { ASSERT(popup->cell_type[d] == POPUP_CELL_TYPE_CSTRING);
        strcpy((char *) popup->value[d], popup->active_cell_buffer);
    }
};

auto POPUP_CLEAR_ALL_VALUES_TO_ZERO = [&]() {
    for (uint32 d = 0; d < popup->num_cells; ++d) {
        if (!popup->name[d]) continue;
        if (popup->cell_type[d] == POPUP_CELL_TYPE_REAL32) {
            *((real32 *) popup->value[d]) = 0.0f;
        } else { ASSERT(popup->cell_type[d] == POPUP_CELL_TYPE_CSTRING);
            memset(popup->value[d], 0, POPUP_CELL_LENGTH);
        }
    }
};

auto POPUP_SELECTION_NOT_ACTIVE = [&]() -> bool { return (popup->selection_cursor == popup->cursor); };

auto POPUP_SET_ACTIVE_CELL_INDEX = [&](uint new_active_cell_index) {
    popup->active_cell_index = new_active_cell_index;
    POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER();
    popup->cursor = (uint32) strlen(popup->active_cell_buffer);
    popup->selection_cursor = 0; // select whole cell
    popup->_type_of_active_cell = popup->cell_type[popup->active_cell_index];
};

///////////
// SCARY //
///////////

auto GENERAL_PURPOSE_MANIFOLD_WRAPPER = [&]() {
    bool32 add = ((global_world_state.modes.enter_mode == ENTER_MODE_EXTRUDE_ADD) || (global_world_state.modes.enter_mode == ENTER_MODE_REVOLVE_ADD));
    if (!skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack) {
        result.record_me = true;
        result.snapshot_me = true;
        result.checkpoint_me = true;
        time_since->successful_feature = 0.0f;
        { // result.mesh
            CrossSectionEvenOdd cross_section = cross_section_create_FORNOW_QUADRATIC(&global_world_state.dxf.entities, true);
            Mesh tmp = wrapper_manifold(
                    &global_world_state.mesh,
                    cross_section.num_polygonal_loops,
                    cross_section.num_vertices_in_polygonal_loops,
                    cross_section.polygonal_loops,
                    get_M_3D_from_2D(),
                    global_world_state.modes.enter_mode,
                    (add) ? global_world_state.popup.extrude_add_out_length : global_world_state.popup.extrude_cut_out_length,
                    (add) ? global_world_state.popup.extrude_add_in_length : global_world_state.popup.extrude_cut_in_length,
                    global_world_state.dxf.origin,
                    global_world_state.dxf.axis_base_point,
                    global_world_state.dxf.axis_angle_from_y);
            cross_section_free(&cross_section);

            mesh_free_AND_zero(&global_world_state.mesh); // FORNOW
            global_world_state.mesh = tmp; // FORNOW
        }
    }

    // reset some stuff
    global_world_state.modes.enter_mode = ENTER_MODE_NONE;
    DXF_CLEAR_SELECTION_MASK_TO(false);
};
