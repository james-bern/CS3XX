// TODO: debug to same giant build script (just one massive file)


#include "cs345.cpp"


// NOTES /////////////////////////////////
// - for LAYOUT, positions and lengths are in inches
// -- i personally think in mm, so you will see, for example, INCHES(30.0)
// -- this returns (30.0 / 25.4), which is the number of inches in 30mm
// - angles are in degrees
// -- i already think in degrees, so you will see, for example, 180.0
// -- if you would prefer to think in radians, you could use, for example, DEG(PI)
// - color is just an integer
// -- useful #defines for colors are below


// DEFINES ///////////////////////////////

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


// STRUCTS ///////////////////////////////

struct Line {
    double start_x;
    double start_y;
    double end_x;
    double end_y;
    int color;
};

struct Arc {
    double center_x;
    double center_y;
    double radius;
    double start_angle;
    double end_angle;
    int color;
};

struct DXF {
    int num_lines;
    int num_arcs;
    Line *lines;
    Arc *arcs;
};


// FUNCTIONS /////////////////////////////
// you implement these ///////////////////

void dxf_save(DXF *dxf, char *filename) {
    FILE *file = fopen(filename, "w");

    _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(dxf);

    // header
    fprintf(file, "  0\n");
    fprintf(file, "SECTION\n");
    fprintf(file, "  2\n");
    fprintf(file, "ENTITIES\n");
    fprintf(file, "  0\n");

    // lines
    for (Line *line = dxf->lines; line < dxf->lines + dxf->num_lines; ++line) {
        fprintf(file, "LINE\n");
        fprintf(file, "  8\n");
        fprintf(file, "   0\n");
        fprintf(file, "  62\n");
        fprintf(file, "   %d\n", line->color);
        fprintf(file, "  10\n");
        fprintf(file, "   %lf\n", line->start_x);
        fprintf(file, "  20\n");
        fprintf(file, "   %lf\n", line->start_y);
        fprintf(file, "  30\n");
        fprintf(file, "   0.0\n");
        fprintf(file, "  11\n");
        fprintf(file, "   %lf\n", line->end_x);
        fprintf(file, "  21\n");
        fprintf(file, "   %lf\n", line->end_y);
        fprintf(file, "  31\n");
        fprintf(file, "   0.0\n");
        fprintf(file, "  0\n");
    }

    // arcs
    for (Arc *arc = dxf->arcs; arc < dxf->arcs + dxf->num_arcs; ++arc) {
        fprintf(file, "ARC\n");
        fprintf(file, "  8\n");
        fprintf(file, "   0\n");
        fprintf(file, "  62\n");
        fprintf(file, "   %d\n", arc->color);
        fprintf(file, "  10\n");
        fprintf(file, "   %lf\n", arc->center_x);
        fprintf(file, "  20\n");
        fprintf(file, "   %lf\n", arc->center_y);
        fprintf(file, "  30\n");
        fprintf(file, "   0.0\n");
        fprintf(file, "  40\n");
        fprintf(file, "   %lf\n", arc->radius);
        fprintf(file, "  50\n");
        fprintf(file, "   %lf\n", arc->start_angle);
        fprintf(file, "  51\n");
        fprintf(file, "   %lf\n", arc->end_angle);
        fprintf(file, "  0\n");
    }

    // footer
    fprintf(file, "ENDSEC\n");
    fprintf(file, "  0\n");
    fprintf(file, "EOF\n");
    fprintf(file, "\n");

    fclose(file);
}


// FORWARD DELCARATIONS //////////////////
// i already implemented these for you ///

// NOTE: slits drawn as solid lines NOT dotted 
void dxf_draw(Camera2D *camera, DXF *dxf);


// MAIN //////////////////////////////////

int main() {
    DXF dxf = {};
    dxf.num_lines = 3;
    dxf.num_arcs = 2;
    dxf.lines = (Line *) calloc(dxf.num_lines, sizeof(Line));
    dxf.lines[0] = {  INCHES( 0.0),  INCHES( 0.0),  INCHES( 30.0),  INCHES( 0.0), COLOR_QUALITY_5 };
    dxf.lines[1] = {  INCHES(30.0),  INCHES(40.0),  INCHES( 30.0),  INCHES( 0.0), COLOR_QUALITY_5 };
    dxf.lines[2] = {  INCHES(30.0),  INCHES(40.0),  INCHES(  0.0),  INCHES( 0.0), COLOR_QUALITY_5 };
    dxf.arcs = (Arc *) calloc(dxf.num_arcs, sizeof(Arc));
    dxf.arcs[0] = { INCHES(20.0), INCHES(10.0), INCHES(7.0),   0.0, 180.0, COLOR_QUALITY_2 };
    dxf.arcs[1] = { INCHES(20.0), INCHES(10.0), INCHES(7.0), 180.0, 360.0, COLOR_QUALITY_4 };

    Camera2D camera = { INCHES(300.0) };
    while (cow_begin_frame()) {
        camera_move(&camera);

        dxf_draw(&camera, &dxf);

        if (gui_button("save", 's')) {
            dxf_save(&dxf, "out.dxf");
        }
    }
}


// FORWARD DELCARED FUNCTIONS ////////////

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

void dxf_draw(Camera2D *camera, DXF *dxf) {
    eso_begin(camera_get_PV(camera), SOUP_LINES); {
        // lines
        for (Line *line = dxf->lines; line < dxf->lines + dxf->num_lines; ++line) {
            _dxf_eso_color(line->color);
            eso_vertex(line->start_x, line->start_y);
            eso_vertex(line->end_x,   line->end_y);
        }

        // arcs
        int NUM_SEGMENTS_PER_CIRCLE = 64;
        for (Arc *arc = dxf->arcs; arc < dxf->arcs + dxf->num_arcs; ++arc) {
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
    } eso_end();
}

