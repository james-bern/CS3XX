// NOTE: This is a little CAD program Jim is actively developing.
//       Feel free to ignore.

// roadmap
// - import dxf
// - detect loops
// - compile/link manifold
// - boss and cut
// - loops have transforms
// - planar surface picking
// - cut has depth
// ? how are we storing the sequence of operations

#include "cs345.cpp"

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

struct DXFArc{
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

void dxf_load(char *filename, DXFGroup *dxf) {
    _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(filename);
    *dxf = {};
    dxf->num_entities = 8;
    dxf->entities = (DXFEntity *) calloc(dxf->num_entities, sizeof(DXFEntity));
    dxf->entities[0] = { DXF_ENTITY_TYPE_LINE, 0, 0.0, 0.0, 1.0, 0.0 };
    dxf->entities[1] = { DXF_ENTITY_TYPE_LINE, 0, 1.0, 0.0, 1.0, 1.0 };
    dxf->entities[2] = { DXF_ENTITY_TYPE_LINE, 0, 0.0, 1.0, 0.0, 0.0 };
    dxf->entities[3] = { DXF_ENTITY_TYPE_ARC,  0, 0.5, 1.0, 0.5,    0.0, 180.0 };
    dxf->entities[4] = { DXF_ENTITY_TYPE_ARC,  1, 0.5, 1.0, 0.25,   0.0, 180.0 };
    dxf->entities[5] = { DXF_ENTITY_TYPE_ARC,  1, 0.5, 1.0, 0.25, 180.0, 360.0 };
    dxf->entities[6] = { DXF_ENTITY_TYPE_ARC,  5, 0.5, 1.0, 0.1,    0.0, 180.0 };
    dxf->entities[7] = { DXF_ENTITY_TYPE_ARC,  5, 0.5, 1.0, 0.1,  180.0, 360.0 };
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

void dxf_draw(Camera2D *camera, DXFGroup *dxf) {
    int NUM_SEGMENTS_PER_CIRCLE = 64;
    eso_begin(camera_get_PV(camera), SOUP_LINES);
    for (DXFEntity *entity = dxf->entities; entity < dxf->entities + dxf->num_entities; ++entity) {
        if (entity->type == DXF_ENTITY_TYPE_LINE) {
            DXFLine *line = &entity->line;
            _dxf_eso_color(line->color);
            eso_vertex(line->start_x, line->start_y);
            eso_vertex(line->end_x,   line->end_y);
        } else { ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
            DXFArc *arc = &entity->arc;
            _dxf_eso_color(arc->color);
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

void dxf_assemble_sorted_loops(DXFGroup *dxf, int *num_loops, DXFGroup **loops) {
    ASSERT(dxf->num_entities);

    // build loops as array list of array lists of entities
    StretchyBuffer<StretchyBuffer<DXFEntity>> result = {}; {
        bool *used = (bool *) calloc(dxf->num_entities, sizeof(bool));
        while (true) {
            { // seed loop
                bool added = false;
                for (int i = 0; i < dxf->num_entities; ++i) {
                    if (!used[i]) {
                        added = true;
                        used[i] = true;
                        sbuff_push_back(&result, {});
                        sbuff_push_back(&result.data[result.length - 1], dxf->entities[i]);
                        break;
                    }
                }
                if (!added) break;
            }
            { // continue, complete and TODO:reverse loop
                while (true) {
                    bool added = false;
                    for (int i = 0; i < dxf->num_entities; ++i) {
                        if (used[i]) continue;
                        real32 start_x_prev, start_y_prev, end_x_prev, end_y_prev;
                        real32 start_x_i, start_y_i, end_x_i, end_y_i;
                        {
                            entity_get_start_and_end_points(
                                    &result.data[result.length - 1].data[result.data[result.length - 1].length - 1],
                                    &start_x_prev, &start_y_prev, &end_x_prev, &end_y_prev);
                            entity_get_start_and_end_points(&dxf->entities[i], &start_x_i, &start_y_i, &end_x_i, &end_y_i);
                        }
                        // NOTE: do NOT assume DXF/OMAX's loops are oriented (so need two checks);
                        bool is_continuation = 
                            point_points_coincide(end_x_prev, end_y_prev, start_x_i, start_y_i) ||
                            point_points_coincide(end_x_prev, end_y_prev, end_x_i,   end_y_i);
                        if (is_continuation) {
                            added = true;
                            used[i] = true;
                            sbuff_push_back(&result.data[result.length - 1], dxf->entities[i]);
                            break;
                        }
                    }
                    if (!added) break;
                }
            }
        }
        free(used);
    }

    // copy over from array lists into bare arrays
    // TODO: what would this look like with pointer as the indexing variable?
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
    DXFGroup dxf;
    dxf_load("...", &dxf);

    int num_loops;
    DXFGroup *loops;
    dxf_assemble_sorted_loops(&dxf, &num_loops, &loops);

    // Part part = {};
    // part_boss(&part, loops[0]);
    // part_cut(&part, loops[1]);


    int i = 0;

    Camera2D camera = { 5.0 };
    while (cow_begin_frame()) {
        camera_move(&camera);

        // dxf_draw(&camera, &dxf);
        gui_slider("i", &i, 0, 100, 'j', 'k');
        if (i < num_loops) dxf_draw(&camera, &loops[i]);
        // part_draw(&part);
    }
}
