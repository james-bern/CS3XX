#include "header.cpp"
#include "jim.cpp"


char viewer_message_buffer[512];
u32 viewer_message_cooldown;
void viewer_messagef(char *format, ...) {
    va_list arg;
    va_start(arg, format);
    vsnprintf(viewer_message_buffer, sizeof(viewer_message_buffer), format, arg);
    va_end(arg);
    viewer_message_cooldown = 600;
}


u32 NUM_SEGMENTS_PER_CIRCLE = 64;

#define DXF_COLOR_TRAVERSE        0
#define DXF_COLOR_QUALITY_1       1
#define DXF_COLOR_QUALITY_2       2
#define DXF_COLOR_QUALITY_3       3
#define DXF_COLOR_QUALITY_4       4
#define DXF_COLOR_QUALITY_5       5
#define DXF_COLOR_ETCH            6
#define DXF_COLOR_LEAD_IO         9
#define DXF_COLOR_QUALITY_SLIT_1 21
#define DXF_COLOR_QUALITY_SLIT_2 22
#define DXF_COLOR_QUALITY_SLIT_3 23
#define DXF_COLOR_QUALITY_SLIT_4 24
#define DXF_COLOR_QUALITY_SLIT_5 25
#define DXF_COLOR_SELECTION     254
#define DXF_COLOR_DONT_OVERRIDE 255


struct DXFLine {
    u32 color;
    real32 start_x;
    real32 start_y;
    real32 end_x;
    real32 end_y;
    real32 _;
};

struct DXFArc {
    u32 color;
    real32 center_x;
    real32 center_y;
    real32 radius;
    real32 start_angle_in_degrees;
    real32 end_angle_in_degrees;
};

#define DXF_ENTITY_TYPE_LINE 0
#define DXF_ENTITY_TYPE_ARC  1
struct DXFEntity {
    u32 type;
    union {
        DXFLine line;
        DXFArc arc;
    };
};

void get_point_on_circle_NOTE_pass_angle_in_radians(real32 *x, real32 *y, real32 center_x, real32 center_y, real32 radius, real32 angle_in_radians) {
    *x = center_x + radius * COS(angle_in_radians);
    *y = center_y + radius * SIN(angle_in_radians);
}

void arc_process_angles_into_lerpable_radians_considering_flip_flag(DXFArc *arc, real32 *start_angle, real32 *end_angle, bool32 flip_flag) {
    // The way the DXF spec works is that start_angle and end_angle define points on the circle
    // which are connected counterclockwise from start to end with an arc
    // (start -ccw-> end)
    //
    // To flip an arc entity, we need to go B -cw-> A
    *start_angle = RAD(arc->start_angle_in_degrees);
    *end_angle = RAD(arc->end_angle_in_degrees);
    if (!flip_flag) {
        // start -ccw-> end
        while (*end_angle < *start_angle) *end_angle += TAU;
    } else {
        // swap
        real32 tmp = *start_angle;
        *start_angle = *end_angle;
        *end_angle = tmp;
        // start -cw-> end
        while (*end_angle > *start_angle) *start_angle += TAU;
    }
}

void entity_get_start_and_end_points(DXFEntity *entity, real32 *start_x, real32 *start_y, real32 *end_x, real32 *end_y) {
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLine *line = &entity->line;
        *start_x = line->start_x;
        *start_y = line->start_y;
        *end_x = line->end_x;
        *end_y = line->end_y;
    } else {
        ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
        DXFArc *arc = &entity->arc;
        get_point_on_circle_NOTE_pass_angle_in_radians(start_x, start_y, arc->center_x, arc->center_y, arc->radius, RAD(arc->start_angle_in_degrees));
        get_point_on_circle_NOTE_pass_angle_in_radians(  end_x,   end_y, arc->center_x, arc->center_y, arc->radius,   RAD(arc->end_angle_in_degrees));
    }
}

struct DXF {
    u32 num_entities;
    DXFEntity *entities;
};

void dxf_free(DXF *drawing) {
    ASSERT(drawing->entities);
    free(drawing->entities);
}

DXF dxf_load(char *filename) {
    #if 0
    {
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(filename);
        return {};
        #elif 0
        DXF result = {};
        result.num_entities = 8;
        result.entities = (DXFEntity *) calloc(result.num_entities, sizeof(DXFEntity));
        result.entities[0] = { DXF_ENTITY_TYPE_LINE, 0, 0.0, 0.0, 1.0, 0.0 };
        result.entities[1] = { DXF_ENTITY_TYPE_LINE, 1, 1.0, 0.0, 1.0, 1.0 };
        result.entities[2] = { DXF_ENTITY_TYPE_LINE, 2, 0.0, 1.0, 0.0, 0.0 };
        result.entities[3] = { DXF_ENTITY_TYPE_ARC,  3, 0.5, 1.0, 0.5,    0.0, 180.0 };
        result.entities[4] = { DXF_ENTITY_TYPE_ARC,  4, 0.5, 1.0, 0.25,   0.0, 180.0 };
        result.entities[5] = { DXF_ENTITY_TYPE_ARC,  5, 0.5, 1.0, 0.25, 180.0, 360.0 };
        result.entities[6] = { DXF_ENTITY_TYPE_ARC,  6, 0.5, 1.0, 0.1,    0.0, 180.0 };
        result.entities[7] = { DXF_ENTITY_TYPE_ARC,  7, 0.5, 1.0, 0.1,  180.0, 360.0 };
        return result;
    }
    #endif
    DXF result = {};

    List<DXFEntity> stretchy_list = {}; {
        FILE *file = (FILE *) fopen(filename, "r");
        ASSERT(file);
        {
            #define DXF_LOAD_MODE_NONE 0
            #define DXF_LOAD_MODE_LINE 1
            #define DXF_LOAD_MODE_ARC  2
            u8 mode = 0;
            int code = 0;
            bool32 code_is_hot = false;
            DXFLine line = {};
            DXFArc arc = {};
            static char line_of_file[512];
            while (fgets(line_of_file, ARRAY_LENGTH(line_of_file), file)) {
                if (mode == DXF_LOAD_MODE_NONE) {
                    if (poe_prefix_match(line_of_file, "LINE")) {
                        mode = DXF_LOAD_MODE_LINE;
                        code_is_hot = false;
                        line = {};
                    } else if (poe_prefix_match(line_of_file, "ARC")) {
                        mode = DXF_LOAD_MODE_ARC;
                        code_is_hot = false;
                        arc = {};
                    }
                } else {
                    if (!code_is_hot) {
                        sscanf(line_of_file, "%d", &code);
                        if (code == 0) {
                            if (mode == DXF_LOAD_MODE_LINE) {
                                list_push_back(&stretchy_list, { DXF_ENTITY_TYPE_LINE, line.color, line.start_x, line.start_y, line.end_x, line.end_y });
                            } else {
                                ASSERT(mode == DXF_LOAD_MODE_ARC);
                                list_push_back(&stretchy_list, { DXF_ENTITY_TYPE_ARC, arc.color, arc.center_x, arc.center_y, arc.radius, arc.start_angle_in_degrees, arc.end_angle_in_degrees });
                            }
                            mode = DXF_LOAD_MODE_NONE;
                            code_is_hot = false;
                        }
                    } else {
                        if (code == 62) {
                            int value;
                            sscanf(line_of_file, "%d", &value);
                            line.color = arc.color = value; 
                        } else {
                            float value;
                            sscanf(line_of_file, "%f", &value);
                            if (mode == DXF_LOAD_MODE_LINE) {
                                if (code == 10) {
                                    line.start_x = (value);
                                } else if (code == 20) {
                                    line.start_y = (value);
                                } else if (code == 11) {
                                    line.end_x = (value);
                                } else if (code == 21) {
                                    line.end_y = (value);
                                }
                            } else {
                                ASSERT(mode == DXF_LOAD_MODE_ARC);
                                if (code == 10) {
                                    arc.center_x = (value);
                                } else if (code == 20) {
                                    arc.center_y = (value);
                                } else if (code == 40) {
                                    arc.radius = (value);
                                } else if (code == 50) {
                                    arc.start_angle_in_degrees = value;
                                } else if (code == 51) {
                                    arc.end_angle_in_degrees = value;
                                }
                            }
                        }
                    }
                    code_is_hot = !code_is_hot;
                }
            }
        } fclose(file);
    }

    result.num_entities = stretchy_list.length;
    result.entities = (DXFEntity *) calloc(result.num_entities, sizeof(DXFEntity));
    memcpy(result.entities, stretchy_list.array, result.num_entities * sizeof(DXFEntity));
    list_free_AND_zero(&stretchy_list);
    return result;
}

// TODO: Port this into a function mutating a real32 *
void _dxf_eso_color(u32 color) {
    if      (color == 0) { eso_color( 83 / 255.0f, 255 / 255.0f,  85 / 255.0f); }
    else if (color == 1) { eso_color(255 / 255.0f,   0 / 255.0f,   0 / 255.0f); }
    else if (color == 2) { eso_color(238 / 255.0f,   0 / 255.0f, 119 / 255.0f); }
    else if (color == 3) { eso_color(255 / 255.0f,   0 / 255.0f, 255 / 255.0f); }
    else if (color == 4) { eso_color(170 / 255.0f,   1 / 255.0f, 255 / 255.0f); }
    else if (color == 5) { eso_color(  0 / 255.0f,  85 / 255.0f, 255 / 255.0f); }
    else if (color == 6) { eso_color(136 / 255.0f, 136 / 255.0f, 136 / 255.0f); }
    else if (color == 7) { eso_color(205 / 255.0f, 205 / 255.0f, 205 / 255.0f); }
    else if (color == 8) { eso_color(  0 / 255.0f, 255 / 255.0f, 255 / 255.0f); }
    else if (color == 9) { eso_color(204 / 255.0f, 136 / 255.0f,   1 / 255.0f); }
    else if (color == DXF_COLOR_SELECTION) { eso_color(1.0f, 1.0f, 0.0f); }
    else {
        // printf("WARNING: slits not implemented\n");
        eso_color(1.0, 1.0, 1.0);
    }
}

void eso_dxf_entity__SOUP_LINES(DXFEntity *entity, int32 override_color = DXF_COLOR_DONT_OVERRIDE) {
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLine *line = &entity->line;
        _dxf_eso_color((override_color != DXF_COLOR_DONT_OVERRIDE) ? override_color : line->color);
        eso_vertex(line->start_x, line->start_y);
        eso_vertex(line->end_x,   line->end_y);
    } else {
        ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
        DXFArc *arc = &entity->arc;
        real32 start_angle, end_angle;
        arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, false);
        real32 delta_angle = end_angle - start_angle;
        u32 num_segments = (u32) (1 + (delta_angle / TAU) * NUM_SEGMENTS_PER_CIRCLE);
        real32 increment = delta_angle / num_segments;
        real32 current_angle = start_angle;
        _dxf_eso_color((override_color != DXF_COLOR_DONT_OVERRIDE) ? override_color : arc->color);
        for (u32 i = 0; i < num_segments; ++i) {
            real32 x, y;
            get_point_on_circle_NOTE_pass_angle_in_radians(&x, &y, arc->center_x, arc->center_y, arc->radius, current_angle);
            eso_vertex(x, y);
            current_angle += increment;
            get_point_on_circle_NOTE_pass_angle_in_radians(&x, &y, arc->center_x, arc->center_y, arc->radius, current_angle);
            eso_vertex(x, y);
        }
    }
}

void dxf_draw(mat4 PV, DXF *drawing, int32 override_color = DXF_COLOR_DONT_OVERRIDE) {
    eso_begin(PV, SOUP_LINES);
    for (DXFEntity *entity = drawing->entities; entity < &drawing->entities[drawing->num_entities]; ++entity) {
        eso_dxf_entity__SOUP_LINES(entity, override_color);
    }
    eso_end();
}



struct STL {
    u32 num_triangles;
    real32 *array;
};


void stl_free(STL *stl) {
    if (stl->array) free(stl->array);
    *stl = {};
}

void stl_draw(mat4 P, mat4 V, mat4 M, STL *stl) {
    eso_begin(P * V * M, SOUP_OUTLINED_TRIANGLES);
    for (real32 *ptr = stl->array; ptr < stl->array + 9 * stl->num_triangles; ptr += 9) {
        vec3 a = { ptr[0], ptr[1], ptr[2] };
        vec3 b = { ptr[3], ptr[4], ptr[5] };
        vec3 c = { ptr[6], ptr[7], ptr[8] };
        vec3 n = transformNormal(V, normalized(cross(b - a, c - a)));
        vec3 color = (n.z < 0) ? V3(1.0f, 0.0f, 0.0f) : V3(0.5f + 0.5f * n.x, 0.5f + 0.5f * n.y, 1.0f);
        eso_color(color);
        eso_vertex(a);
        eso_vertex(b);
        eso_vertex(c);
    }
    eso_end();
}

STL stl_load(char *filename) {
    static char line_of_file[512];

    #define STL_FILETYPE_UNKNOWN 0
    #define STL_FILETYPE_ASCII   1
    #define STL_FILETYPE_BINARY  2
    u32 filetype; {
        FILE *file = (FILE *) fopen(filename, "r");
        fgets(line_of_file, 80, file);
        filetype = (poe_prefix_match(line_of_file, "solid")) ? STL_FILETYPE_ASCII : STL_FILETYPE_BINARY;
        fclose(file);
    }

    STL result = {};
    if (filetype == STL_FILETYPE_ASCII) {
        u8 ascii_scan_dummy[64];
        real32 ascii_scan_p[3];
        List<real32> ascii_data = {};

        FILE *file = (FILE *) fopen(filename, "r");
        while (fgets(line_of_file, ARRAY_LENGTH(line_of_file), file)) {
            if (filetype == STL_FILETYPE_ASCII) {
                if (poe_prefix_match(line_of_file, "vertex")) {
                    sscanf(line_of_file, "%s %f %f %f", ascii_scan_dummy, &ascii_scan_p[0], &ascii_scan_p[1], &ascii_scan_p[2]);
                    for (u32 d = 0; d < 3; ++d) list_push_back(&ascii_data, ascii_scan_p[d]);
                }
            } else if (filetype == STL_FILETYPE_BINARY) {
            }
        }
        fclose(file);

        result.num_triangles = ascii_data.length / 9;
        u32 size = ascii_data.length * sizeof(real32);
        result.array = (real32 *) malloc(size);
        memcpy(result.array, ascii_data.array, size);
        list_free_AND_zero(&ascii_data);
    } else if (filetype == STL_FILETYPE_BINARY) {
        char *entire_file; {
            FILE *file = fopen(filename, "rb");
            fseek(file, 0, SEEK_END);
            long fsize = ftell(file);
            fseek(file, 0, SEEK_SET);
            entire_file = (char *) malloc(fsize + 1);
            fread(entire_file, fsize, 1, file);
            fclose(file);
            entire_file[fsize] = 0;
        }

        u32 offset = 80;
        memcpy(&result.num_triangles, entire_file + offset, 4);
        offset += 4;

        u32 size = result.num_triangles * 36;
        result.array = (real32 *) calloc(1, size);

        for (u32 i = 0; i < result.num_triangles; ++i) {
            offset += 12;
            memcpy(result.array + i * 9, entire_file + offset, 36);
            offset += 38;
        }
    }
    return result;
}


DXF drawing;
STL stl;

#define VIEWER_MODE_NONE 0
#define VIEWER_MODE_DXF  1
#define VIEWER_MODE_STL  2
u32 viewer_mode;

bool part_is_in_inches_not_mm;

void drop_callback(GLFWwindow *, int count, const char** paths) {
    if (count > 0) {
        char *filename = (char *) paths[0]; 
        if (poe_suffix_match(filename, ".drawing")) {
            part_is_in_inches_not_mm = true;
            viewer_mode = VIEWER_MODE_DXF;
            drawing = dxf_load(filename);
            viewer_messagef("loaded %s\n", filename);
        } else if (poe_suffix_match(filename, ".stl")) {
            part_is_in_inches_not_mm = false;
            viewer_mode = VIEWER_MODE_STL;
            stl = stl_load(filename);
            viewer_messagef("loaded %s\n", filename);
        } else {
            viewer_messagef("%s filetype not recognized", filename);
        }
    }
} BEGIN_PRE_MAIN { glfwSetDropCallback(COW0._window_glfw_window, drop_callback); } END_PRE_MAIN;


int main() {
    viewer_messagef("drag and drop *.drawing or *.stl...");
    Camera2D camera2D = { 400.0f, 150.0f, 150.0f };
    Camera3D camera3D = { 256.0f };
    bool show_grid = true;
    while (cow_begin_frame()) {
        if (viewer_mode == VIEWER_MODE_DXF) camera_move(&camera2D);
        if (viewer_mode == VIEWER_MODE_STL) camera_move(&camera3D);

        {
            if (viewer_message_cooldown > 0) {
                --viewer_message_cooldown;
            } else {
                viewer_message_buffer[0] = '\0';
            }
            gui_printf("< %s", viewer_message_buffer);
        }

        if (viewer_mode != VIEWER_MODE_NONE) {
            gui_checkbox("part_is_in_inches_not_mm", &part_is_in_inches_not_mm, 'i');
            if (viewer_mode == VIEWER_MODE_DXF) {
                gui_checkbox("show grid", &show_grid, 'g');
            } else if (viewer_mode == VIEWER_MODE_STL) {
                gui_printf("rotate: left-click and drag");
            }
            gui_printf("pan: right-click and drag");
            gui_printf("zoom: scroll");
        }

        if (show_grid) {
            if (viewer_mode == VIEWER_MODE_DXF) {
                eso_begin(camera_get_PV(&camera2D), SOUP_LINES, 2.0f);
                eso_color(monokai.gray);
                for (u32 i = 0; i <= 30; ++i) {
                    real32 tmp = i * 10.0f;
                    eso_vertex(tmp,   0.0f);
                    eso_vertex(tmp, 300.0f);
                    eso_vertex(  0.0f, tmp);
                    eso_vertex(300.0f, tmp);
                }
                eso_end();
            } else if (viewer_mode == VIEWER_MODE_STL) {
                // TODO: 3D box (LATER)
            }
        }

        if (viewer_mode == VIEWER_MODE_DXF) {
            gui_printf("num_entities %d", drawing.num_entities);
        } else if (viewer_mode == VIEWER_MODE_STL) {
            gui_printf("num_triangles %d", stl.num_triangles);
        }

        if (viewer_mode == VIEWER_MODE_DXF) {
            dxf_draw(camera_get_PV(&camera2D) * M4_Scaling((!part_is_in_inches_not_mm) ? 1.0f : 25.4f), &drawing);
        } else if (viewer_mode == VIEWER_MODE_STL) {
            stl_draw(camera_get_P(&camera3D), camera_get_V(&camera3D), M4_Scaling((!part_is_in_inches_not_mm) ? 1.0f : 25.4f), &stl);
        }
    }
}
