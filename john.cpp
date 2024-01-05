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

// DXF



#define DXF_ENTITY_TYPE_LINE 0
#define DXF_ENTITY_TYPE_ARC  1
struct DXFEntity {
    int type;
    int color;
    union {
        struct {
            double start_x;
            double start_y;
            double end_x;
            double end_y;
            double _;
        };
        struct {
            double center_x;
            double center_y;
            double radius;
            double start_angle;
            double end_angle;
        };
    };
};

struct DXFGroup {
    int num_entities;
    DXFEntity *entities;
};

void dxf_load(char *filename, DXFGroup *dxf) {
    _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(filename);
    *dxf = {};
    dxf->num_entities = 6;
    dxf->entities = (DXFEntity *) calloc(dxf->num_entities, sizeof(DXFEntity));
    dxf->entities[0] = { DXF_ENTITY_TYPE_LINE, 0, 0.0, 0.0, 1.0, 0.0 };
    dxf->entities[1] = { DXF_ENTITY_TYPE_LINE, 0, 1.0, 0.0, 1.0, 1.0 };
    dxf->entities[2] = { DXF_ENTITY_TYPE_LINE, 0, 0.0, 1.0, 0.0, 0.0 };
    dxf->entities[3] = { DXF_ENTITY_TYPE_ARC,  0, 0.5, 1.0, 0.5,    0.0, 180.0 };
    dxf->entities[4] = { DXF_ENTITY_TYPE_ARC,  1, 0.5, 1.0, 0.25,   0.0, 180.0 };
    dxf->entities[5] = { DXF_ENTITY_TYPE_ARC,  1, 0.5, 1.0, 0.25, 180.0, 360.0 };
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
            _dxf_eso_color(entity->color);
            eso_vertex(entity->start_x, entity->start_y);
            eso_vertex(entity->end_x,   entity->end_y);
        } else if (entity->type == DXF_ENTITY_TYPE_ARC) {
            _dxf_eso_color(entity->color);
            double start_angle = RAD(entity->start_angle);
            double end_angle = RAD(entity->end_angle);
            double delta_angle = end_angle - start_angle;
            int num_segments = (int) (1 + (delta_angle / TAU) * NUM_SEGMENTS_PER_CIRCLE);
            double increment = delta_angle / num_segments;
            double current_angle = start_angle;
            for (int i = 0; i <= num_segments; ++i) {
                eso_vertex(
                        entity->center_x + entity->radius * cos(current_angle),
                        entity->center_y + entity->radius * sin(current_angle));
                current_angle += increment;
                eso_vertex(
                        entity->center_x + entity->radius * cos(current_angle),
                        entity->center_y + entity->radius * sin(current_angle));
            }
        } else {
            ASSERT(0);
        }
    }
    eso_end();
}

void dxf_assemble_sorted_loops(DXFGroup *dxf, int *num_loops, DXFGroup **loops) {
    ASSERT(dxf->num_entities);

    StretchyBuffer<StretchyBuffer<DXFEntity>> result = {};


    // bool *visited = (bool *) calloc(dxf->num_entities, sizeof(bool));
    // StretchyBuffer<int> queue = {};
    // squeue_add(&queue, 0);
    // visited[0] = true;
    // while (queue->length) {
    //     DXFEntity *entity = &dxf->entities[queue[squeue_remove(&queue)]];
    //     // TODO: endpoints
    //     for (int i = 0; i < dxf->num_entities; ++i) {
    //     }
    // }

    // free(visited);

    // copy over from array lists
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

    Camera2D camera = { 5.0 };
    while (cow_begin_frame()) {
        camera_move(&camera);

        // dxf_draw(&camera, &dxf);
        if (num_loops >= 2) {
            if (!globals.key_toggled['a']) {
                dxf_draw(&camera, &loops[0]);
            } else {
                dxf_draw(&camera, &loops[1]);
            }
        }
        // part_draw(&part);
    }
}
