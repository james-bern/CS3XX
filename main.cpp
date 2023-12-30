// TODO: rename hw00.cpp (make build scripts take argument; fix up your vim to accomodate this)
// TODO: same stuff for STL

#include "cs345.cpp"

// NOTES /////////////////////////////////
// - positions and lengths are in mm
// - angles are in degrees

// STRUCTS ///////////////////////////////

struct Line {
    double start_x;
    double start_y;
    double end_x;
    double end_y;
};

struct Arc {
    double center_x;
    double center_y;
    double radius;
    double start_angle;
    double end_angle;
};

struct DXF {
    int num_lines;
    int num_arcs;
    Line *lines;
    Arc *arcs;
};

// FUNCTIONS /////////////////////////////

// void dxf_export(DXF *dxf, char *filename) {
// 
// }
// 
// DXF dxf_import(char *filename) {
// 
// }

// FORWARD DELCARATIONS //////////////////
// i already implemented these for you ///

void dxf_draw(mat4 transform, DXF *dxf);

// APP ///////////////////////////////////

void dxf_viewer() {
    Camera2D camera = { 200.0 };

    DXF dxf = {};
    dxf.num_lines = 1;
    dxf.num_arcs = 1;
    dxf.lines = (Line *) calloc(dxf.num_lines, sizeof(Line));
    dxf.lines[0] = { 0.0, 0.0, 30.0, 40.0 };
    dxf.arcs = (Arc *) calloc(dxf.num_arcs, sizeof(Arc));
    dxf.arcs[0] = { 30.0, 40.0, 15.0, 0.0, 180.0 };

    while (cow_begin_frame()) {
        camera_move(&camera);
        mat4 transform = camera_get_PV(&camera);
        dxf_draw(transform, &dxf);
    }
}

int main() {
    APPS {
        APP(dxf_viewer);
    }

    return 0;
}

// FORWARD DELCARATIONS //////////////////

void dxf_draw(mat4 transform, DXF *dxf) {
    eso_begin(transform, SOUP_LINES);
    eso_color(monokai.white);
    eso_vertex(0.0, 0.0);
    eso_vertex(30.0, 40.0);
    eso_end();
}
