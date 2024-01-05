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

bool point_points_coincide(real32 x_A, real32 y_A, real32 x_B, real32 y_B) {
    real32 dx = (x_A - x_B);
    real32 dy = (y_A - y_B);
    real32 D2 = (dx * dx) + (dy * dy);
    return (D2 < 1e-5);
};

struct DXFLine {
    int color;
    double start_x;
    double start_y;
    double end_x;
    double end_y;
    double _;
};

struct DXFArc {
    int color;
    double center_x;
    double center_y;
    double radius;
    double start_angle;
    double end_angle;
};


#define DXF_ENTITY_TYPE_LINE 0
#define DXF_ENTITY_TYPE_ARC  1
struct DXFEntity {
    int type;
    union {
        DXFLine line;
        DXFArc arc;
    };
};

void entity_get_start_and_end_points(DXFEntity *E, real32 *start_x, real32 *start_y, real32 *end_x, real32 *end_y) {
    if (E->type == DXF_ENTITY_TYPE_LINE) {
        *start_x = E->line.start_x;
        *start_y = E->line.start_y;
        *end_x   = E->line.end_x;
        *end_y   = E->line.end_y;
    } else { ASSERT(E->type == DXF_ENTITY_TYPE_ARC);
        *start_x = E->arc.center_x + E->arc.radius * cos(RAD(E->arc.start_angle));
        *start_y = E->arc.center_y + E->arc.radius * sin(RAD(E->arc.start_angle));
        *end_x   = E->arc.center_x + E->arc.radius * cos(RAD(E->arc.end_angle));
        *end_y   = E->arc.center_y + E->arc.radius * sin(RAD(E->arc.end_angle));
    }
}

struct DXFGroup {
    int num_entities;
    DXFEntity *entities;
};

void dxf_free(DXFGroup *dxf) {
    ASSERT(dxf->entities);
    free(dxf->entities);
}

void dxf_load(char *filename, DXFGroup *dxf) {
    _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(filename);
    *dxf = {};
    dxf->num_entities = 8;
    dxf->entities = (DXFEntity *) calloc(dxf->num_entities, sizeof(DXFEntity));
    dxf->entities[0] = { DXF_ENTITY_TYPE_LINE, 0, 0.0, 0.0, 1.0, 0.0 };
    dxf->entities[7] = { DXF_ENTITY_TYPE_LINE, 1, 1.0, 0.0, 1.0, 1.0 };
    dxf->entities[2] = { DXF_ENTITY_TYPE_LINE, 2, 0.0, 1.0, 0.0, 0.0 };
    dxf->entities[6] = { DXF_ENTITY_TYPE_ARC,  3, 0.5, 1.0, 0.5,    0.0, 180.0 };
    dxf->entities[4] = { DXF_ENTITY_TYPE_ARC,  4, 0.5, 1.0, 0.25,   0.0, 180.0 };
    dxf->entities[5] = { DXF_ENTITY_TYPE_ARC,  5, 0.5, 1.0, 0.25, 180.0, 360.0 };
    dxf->entities[3] = { DXF_ENTITY_TYPE_ARC,  6, 0.5, 1.0, 0.1,    0.0, 180.0 };
    dxf->entities[1] = { DXF_ENTITY_TYPE_ARC,  7, 0.5, 1.0, 0.1,  180.0, 360.0 };
}

void _dxf_eso_color(int color) {
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

void dxf_draw(Camera2D *camera, DXFGroup *dxf, int override_color = -1) {
    int NUM_SEGMENTS_PER_CIRCLE = 64;
    eso_begin(camera_get_PV(camera), SOUP_LINES);
    for (DXFEntity *entity = dxf->entities; entity < dxf->entities + dxf->num_entities; ++entity) {
        if (entity->type == DXF_ENTITY_TYPE_LINE) {
            DXFLine *line = &entity->line;
            _dxf_eso_color((override_color != -1) ? override_color : line->color);
            eso_vertex(line->start_x, line->start_y);
            eso_vertex(line->end_x,   line->end_y);
        } else { ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
            DXFArc *arc = &entity->arc;
            _dxf_eso_color((override_color != -1) ? override_color : arc->color);
            double start_angle = RAD(arc->start_angle);
            double end_angle = RAD(arc->end_angle);
            double delta_angle = end_angle - start_angle;
            int num_segments = (int) (1 + (delta_angle / TAU) * NUM_SEGMENTS_PER_CIRCLE);
            double increment = delta_angle / num_segments;
            double current_angle = start_angle;
            for (int i = 0; i <= num_segments; ++i) {
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
void dxf_pick_loop(Camera2D *camera, int num_loops, DXFGroup *loops, DXFGroup **selected_loop) {
    if (globals.mouse_left_pressed) {
        if (*selected_loop == NULL) {
            *selected_loop = &loops[0];
        } else {
            *selected_loop = NULL;
        }
    }
}

void dxf_assemble_sorted_loops(DXFGroup *dxf, int *num_loops, DXFGroup **loops) {
    ASSERT(dxf->num_entities);

    // build loops as array list of array lists of entities
    StretchyBuffer<StretchyBuffer<DXFEntity>> result = {}; {
        bool *entity_already_added = (bool *) calloc(dxf->num_entities, sizeof(bool));
        while (true) {
            { // seed loop
                bool added_and_seeded_new_loop = false;
                for (int i = 0; i < dxf->num_entities; ++i) {
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
                    for (int i = 0; i < dxf->num_entities; ++i) {
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
                        bool is_next_entity_NOTE_DXF_NOT_oriented = 
                            point_points_coincide(end_x_prev, end_y_prev, start_x_i, start_y_i) ||
                            point_points_coincide(end_x_prev, end_y_prev, end_x_i,   end_y_i);
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
    *loops = (DXFGroup *) calloc(*num_loops, sizeof(DXFGroup));
    for (int i = 0; i < *num_loops; ++i) {
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
    int type;

};

struct Part {
    int num_groups;
    DXFGroup *groups;

    StretchyBuffer<PartOperation> operations;
};

int main() {

    int num_loops;
    DXFGroup *loops;
    {
        DXFGroup dxf;
        dxf_load("...", &dxf);
        dxf_assemble_sorted_loops(&dxf, &num_loops, &loops);
        dxf_free(&dxf);
    }

    DXFGroup *selected_loop = NULL;

    // Part part = {};
    // part_boss(&part, loops[0]);
    // part_cut(&part, loops[1]);


    Camera2D camera = { 5.0 };
    while (cow_begin_frame()) {
        camera_move(&camera);

        // dxf_draw(&camera, &dxf);
        dxf_pick_loop(&camera, num_loops, loops, &selected_loop);
        for (DXFGroup *loop = loops; loop < loops + num_loops; ++loop) {
            dxf_draw(&camera, loop, (loop == selected_loop) ? COLOR_TRAVERSE : COLOR_ETCH);
        }
        // part_draw(&part);
    }
}
