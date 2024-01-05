// TODO: cow_real actually supporting real32 or real64

// NOTE: This is a little CAD program Jim is actively developing.
//       Feel free to ignore.

// roadmap
// / import dxf
// / detect loops
// - 2D picking of loops by clicking on DXF
// - compile/link manifold
// - boss ('b') and cut ('c') -- (or 'e'/'E'?)
// - offsetting with x
// - planar surface picking (and if no 3D geometry yet, your choice of the xy yz and zx planes)
// - cut has depth
// ? how are we storing the sequence of operations
// NOTE: for a first cut it could be destructive like omax itself
//       basically just like a conversational mill
//       don't store anything but the STL created so far (and maybe an undo stack eventually)
//       could at least print to the terminal what you've done
//       should preview the cut on the surface before you accept it (eventually) -- and let you nudge it in x, y, and z

#include "cs345.cpp"

#define COLOR_TRAVERSE        0
#define COLOR_QUALITY_1       1
#define COLOR_QUALITY_2       2
#define COLOR_QUALITY_3       3
#define COLOR_QUALITY_4       4
#define COLOR_QUALITY_5       5
#define COLOR_ETCH            6
#define COLOR_LEAD_IO         9
#define COLOR_QUALITY_SLIT_1 21
#define COLOR_QUALITY_SLIT_2 22
#define COLOR_QUALITY_SLIT_3 23
#define COLOR_QUALITY_SLIT_4 24
#define COLOR_QUALITY_SLIT_5 25

real32 TOLERANCE_DEFAULT = 1e-5;
real32 EPSILON_DEFAULT = 2e-3;



struct DXFLineSegment {
    int32 color;
    real32 start_x;
    real32 start_y;
    real32 end_x;
    real32 end_y;
    real32 _;
};

struct DXFArc {
    int32 color;
    real32 center_x;
    real32 center_y;
    real32 radius;
    real32 start_angle;
    real32 end_angle;
};

#define DXF_ENTITY_TYPE_LINE 0
#define DXF_ENTITY_TYPE_ARC  1
struct DXFEntity {
    u32 type;
    union {
        DXFLineSegment line_segment;
        DXFArc arc;
    };
};

void arc_get_start_and_end_points(real32 center_x, real32 center_y, real32 radius, real32 start_angle, real32 end_angle, real32 *start_x, real32 *start_y, real32 *end_x, real32 *end_y) {
    *start_x = center_x + radius * cos(RAD(start_angle));
    *start_y = center_y + radius * sin(RAD(start_angle));
    *end_x   = center_x + radius * cos(RAD(end_angle));
    *end_y   = center_y + radius * sin(RAD(end_angle));
}

void entity_get_start_and_end_points(DXFEntity *entity, real32 *start_x, real32 *start_y, real32 *end_x, real32 *end_y) {
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLineSegment *line_segment = &entity->line_segment;
        *start_x = line_segment->start_x;
        *start_y = line_segment->start_y;
        *end_x = line_segment->end_x;
        *end_y = line_segment->end_y;
    } else {
        ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
        DXFArc *arc = &entity->arc;
        arc_get_start_and_end_points(arc->center_x, arc->center_y, arc->radius, arc->start_angle, arc->end_angle, start_x, start_y, end_x, end_y);
    }
}

struct DXF {
    u32 num_entities;
    DXFEntity *entities;
};

real32 squared_distance_point_point(real32 x_A, real32 y_A, real32 x_B, real32 y_B) {
    real32 dx = (x_A - x_B);
    real32 dy = (y_A - y_B);
    return (dx * dx) + (dy * dy);
};

real32 squared_distance_point_line_segment(real32 x, real32 y, real32 start_x, real32 start_y, real32 end_x, real32 end_y) {
    real32 l2 = squared_distance_point_point(start_x, start_y, end_x, end_y);
    if (l2 < TINY_VAL) return squared_distance_point_point(x, y, start_x, start_y);
    real32 num = (x - start_x) * (end_x - start_x) + (y - start_y) * (end_y - start_y);
    real32 t = MIN(MAX(num / l2, 0), 1);
    real32 P_x = start_x + t * (end_x - start_x);
    real32 P_y = start_y + t * (end_y - start_y);
    return (P_x - x) * (P_x - x) + (P_y - y) * (P_y - y);
}

real32 squared_distance_point_circle(real32 x, real32 y, real32 center_x, real32 center_y, real32 radius) {
    return pow(sqrt(squared_distance_point_point(x, y, center_x, center_y)) - radius, 2);
}

real32 squared_distance_point_arc(real32 x, real32 y, real32 center_x, real32 center_y, real32 radius, real32 start_angle, real32 end_angle) {
    bool point_in_sector = false; {
        real32 v_x = x - center_x;
        real32 v_y = y - center_y;
        // forgive me rygorous
        real32 angle = DEG(atan2(v_y, v_x));
        point_in_sector =
            IS_BETWEEN(angle, start_angle, end_angle)
            || IS_BETWEEN(angle + 360, start_angle, end_angle)
            || IS_BETWEEN(angle - 360, start_angle, end_angle);
    }
    if (point_in_sector) {
        return squared_distance_point_circle(x, y, center_x, center_y, radius);
    } else {
        real32 start_x, start_y, end_x, end_y;
        arc_get_start_and_end_points(center_x, center_y, radius, start_angle, end_angle, &start_x, &start_y, &end_x, &end_y);
        return MIN(squared_distance_point_point(x, y, start_x, start_y), squared_distance_point_point(x, y, end_x, end_y));
    }
    return HUGE_VAL;
}

real32 squared_distance_point_entity(real32 x, real32 y, DXFEntity *entity) {
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLineSegment *line_segment = &entity->line_segment;
        return squared_distance_point_line_segment(x, y, line_segment->start_x, line_segment->start_y, line_segment->end_x, line_segment->end_y);
    } else {
        ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
        DXFArc *arc = &entity->arc;
        return squared_distance_point_arc(x, y, arc->center_x, arc->center_y, arc->radius, arc->start_angle, arc->end_angle);
    }
}

real32 squared_distance_point_dxf(real32 x, real32 y, DXF *dxf) {
    double result = HUGE_VAL;
    for (DXFEntity *entity = dxf->entities; entity < dxf->entities + dxf->num_entities; ++entity) {
        result = MIN(result, squared_distance_point_entity(x, y, entity));
    }
    return result;
}





void dxf_free(DXF *dxf) {
    ASSERT(dxf->entities);
    free(dxf->entities);
}

void dxf_load(char *filename, DXF *dxf) {
    _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(filename);
    *dxf = {};
    dxf->num_entities = 8;
    dxf->entities = (DXFEntity *) calloc(dxf->num_entities, sizeof(DXFEntity));
    dxf->entities[0] = { DXF_ENTITY_TYPE_LINE, 0, 0.0, 0.0, 1.0, 0.0 };
    dxf->entities[1] = { DXF_ENTITY_TYPE_LINE, 1, 1.0, 0.0, 1.0, 1.0 };
    dxf->entities[2] = { DXF_ENTITY_TYPE_LINE, 2, 0.0, 1.0, 0.0, 0.0 };
    dxf->entities[3] = { DXF_ENTITY_TYPE_ARC,  3, 0.5, 1.0, 0.5,    0.0, 180.0 };
    dxf->entities[4] = { DXF_ENTITY_TYPE_ARC,  4, 0.5, 1.0, 0.25,   0.0, 180.0 };
    dxf->entities[5] = { DXF_ENTITY_TYPE_ARC,  5, 0.5, 1.0, 0.25, 180.0, 360.0 };
    dxf->entities[6] = { DXF_ENTITY_TYPE_ARC,  6, 0.5, 1.0, 0.1,    0.0, 180.0 };
    dxf->entities[7] = { DXF_ENTITY_TYPE_ARC,  7, 0.5, 1.0, 0.1,  180.0, 360.0 };
}

void _dxf_eso_color(u32 color) {
    if      (color      == 0) { eso_color( 83 / 255.0, 255 / 255.0,  85 / 255.0); }
    else if (color % 10 == 1) { eso_color(255 / 255.0,   0 / 255.0,   0 / 255.0); }
    else if (color % 10 == 2) { eso_color(238 / 255.0,   0 / 255.0, 119 / 255.0); }
    else if (color % 10 == 3) { eso_color(255 / 255.0,   0 / 255.0, 255 / 255.0); }
    else if (color % 10 == 4) { eso_color(170 / 255.0,   1 / 255.0, 255 / 255.0); }
    else if (color % 10 == 5) { eso_color(  0 / 255.0,  85 / 255.0, 255 / 255.0); }
    else if (color      == 6) { eso_color(136 / 255.0, 136 / 255.0, 136 / 255.0); }
    else if (color      == 7) { eso_color(205 / 255.0, 205 / 255.0, 205 / 255.0); }
    else if (color      == 8) { eso_color(  0 / 255.0, 255 / 255.0, 255 / 255.0); }
    else if (color      == 9) { eso_color(204 / 255.0, 136 / 255.0,   1 / 255.0); }
    else { eso_color(1.0, 1.0, 1.0); }
}

void dxf_draw(Camera2D *camera2D, DXF *dxf, int32 override_color = -1) {
    u32 NUM_SEGMENTS_PER_CIRCLE = 64;
    eso_begin(camera_get_PV(camera2D), SOUP_LINES);
    for (DXFEntity *entity = dxf->entities; entity < dxf->entities + dxf->num_entities; ++entity) {
        if (entity->type == DXF_ENTITY_TYPE_LINE) {
            DXFLineSegment *line_segment = &entity->line_segment;
            _dxf_eso_color((override_color != -1) ? override_color : line_segment->color);
            eso_vertex(line_segment->start_x, line_segment->start_y);
            eso_vertex(line_segment->end_x,   line_segment->end_y);
        } else {
            ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
            DXFArc *arc = &entity->arc;
            _dxf_eso_color((override_color != -1) ? override_color : arc->color);
            real32 start_angle = RAD(arc->start_angle);
            real32 end_angle = RAD(arc->end_angle);
            real32 delta_angle = end_angle - start_angle;
            u32 num_segments = (u32) (1 + (delta_angle / TAU) * NUM_SEGMENTS_PER_CIRCLE);
            real32 increment = delta_angle / num_segments;
            real32 current_angle = start_angle;
            for (u32 i = 0; i <= num_segments; ++i) {
                eso_vertex(
                        arc->center_x + arc->radius * cos(current_angle),
                        arc->center_y + arc->radius * sin(current_angle));
                current_angle += increment;
                eso_vertex(
                        arc->center_x + arc->radius * cos(current_angle),
                        arc->center_y + arc->radius * sin(current_angle));
            }
        }
    }
    eso_end();
}
void dxf_pick_loop(Camera2D *camera2D, u32 num_loops, DXF *loops, DXF **hot_loop, DXF **selected_loop, real32 epsilon = EPSILON_DEFAULT) {
    { // hot
      // TODO: this is silly; i want to be able to tell cow to use a 32 bit float
      //       (cow_real)
        real64 PV[16];
        _camera_get_PV(camera2D, PV);
        real64 x, y;
        _input_get_mouse_position_and_change_in_position_in_world_coordinates(PV, &x, &y, NULL, NULL);

        *hot_loop = NULL;
        double hot_squared_distance = HUGE_VAL;
        for (DXF *loop = loops; loop < loops + num_loops; ++loop) {
            double squared_distance = squared_distance_point_dxf(x, y, loop);
            if (squared_distance < MIN(epsilon, hot_squared_distance)) {
                hot_squared_distance = squared_distance;
                *hot_loop = loop;
            }
        }
    }

    // selected
    if (hot_loop != NULL) {
        if (globals.mouse_left_pressed) {
            *selected_loop = (*selected_loop != *hot_loop) ? *hot_loop : NULL;
        }
    }
}

void dxf_assemble_sorted_loops(DXF *dxf, u32 *num_loops, DXF **loops) {
    ASSERT(dxf->num_entities);

    // build loops as array list of array lists of entities
    StretchyBuffer<StretchyBuffer<DXFEntity>> result = {}; {
        bool *entity_already_added = (bool *) calloc(dxf->num_entities, sizeof(bool));
        while (true) {
            { // seed loop
                bool added_and_seeded_new_loop = false;
                for (u32 i = 0; i < dxf->num_entities; ++i) {
                    if (!entity_already_added[i]) {
                        added_and_seeded_new_loop = true;
                        entity_already_added[i] = true;
                        sbuff_push_back(&result, {});
                        sbuff_push_back(&result.data[result.length - 1], dxf->entities[i]);
                        break;
                    }
                }
                if (!added_and_seeded_new_loop) break;
            }
            { // continue, complete and TODO:reverse loop
                while (true) {
                    bool added_new_entity_to_loop = false;
                    for (u32 i = 0; i < dxf->num_entities; ++i) {
                        if (entity_already_added[i]) continue;
                        real32 start_x_prev, start_y_prev, end_x_prev, end_y_prev;
                        real32 start_x_i, start_y_i, end_x_i, end_y_i;
                        {
                            entity_get_start_and_end_points(
                                    &result.data[result.length - 1].data[result.data[result.length - 1].length - 1],
                                    &start_x_prev, &start_y_prev, &end_x_prev, &end_y_prev);
                            entity_get_start_and_end_points(&dxf->entities[i], &start_x_i, &start_y_i, &end_x_i, &end_y_i);
                        }
                        // NOTE: do NOT assume DXF/OMAX's loops are oriented (so need two checks);
                        real32 tolerance = TOLERANCE_DEFAULT;
                        bool is_next_entity_NOTE_DXF_NOT_oriented = 
                            (squared_distance_point_point(end_x_prev, end_y_prev, start_x_i, start_y_i) < tolerance) ||
                            (squared_distance_point_point(end_x_prev, end_y_prev, end_x_i,   end_y_i) < tolerance);
                        if (is_next_entity_NOTE_DXF_NOT_oriented) {
                            added_new_entity_to_loop = true;
                            entity_already_added[i] = true;
                            sbuff_push_back(&result.data[result.length - 1], dxf->entities[i]);
                            break;
                        }
                    }
                    if (!added_new_entity_to_loop) break;
                }
            }
        }
        free(entity_already_added);
    }

    // copy over from array lists into bare arrays
    *num_loops = result.length;
    *loops = (DXF *) calloc(*num_loops, sizeof(DXF));
    for (u32 i = 0; i < *num_loops; ++i) {
        (*loops)[i].num_entities = result.data[i].length;
        (*loops)[i].entities = (DXFEntity *) calloc((*loops)[i].num_entities, sizeof(DXFEntity));
        memcpy((*loops)[i].entities, result.data[i].data, (*loops)[i].num_entities * sizeof(DXFEntity));
    }

    // free array lists
    for (int i = 0; i < result.length; ++i) sbuff_free(&result.data[i]);
    sbuff_free(&result);
}


#define PART_OPERATION_TYPE_BOSS 0
#define PART_OPERATION_TYPE_CUT  1
struct PartOperation {
    u32 type;

};

struct Part {
    u32 num_groups;
    DXF *groups;

    StretchyBuffer<PartOperation> operations;
};

int main() {

    u32 num_loops;
    DXF *loops;
    {
        DXF dxf;
        dxf_load("...", &dxf);
        dxf_assemble_sorted_loops(&dxf, &num_loops, &loops);
        dxf_free(&dxf);
    }

    DXF *selected_loop = NULL;
    DXF *hot_loop = NULL;

    // Part part = {};
    // part_boss(&part, loops[0]);
    // part_cut(&part, loops[1]);


    Camera2D camera2D = { 5.0, 3.0 };
    Camera3D camera3D = { 5.0, RAD(0.0), RAD(0.0), RAD(0.0), -3.0 };
    while (cow_begin_frame()) {
        camera_move(&camera2D);

        // dxf_draw(&camera2D, &dxf);
        dxf_pick_loop(&camera2D, num_loops, loops, &hot_loop, &selected_loop);
        for (DXF *loop = loops; loop < loops + num_loops; ++loop) {
            dxf_draw(&camera2D, loop, (loop == hot_loop && loop == selected_loop) ? COLOR_QUALITY_1 : (loop == hot_loop) ? COLOR_TRAVERSE : (loop == selected_loop) ? COLOR_LEAD_IO : COLOR_ETCH);
        }
        // part_draw(&part);
    }
}
