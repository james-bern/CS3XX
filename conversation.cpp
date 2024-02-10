// TODO: conversation_drop_path shows up in line above console when saving
// TODO: messages from app (messagef) for missing path, etc.
// TODO: click on the bottom plane in the box
// TODO: undo

// // Conversation
// This is a little CAD program Jim is making :)
//  It takes in an OMAX DXF and let's you rapidly create a 3D-printable STL using Manifold.

// / make 3D pick work with perspective camera
// / extend cameras and camera_move to have a post-projection NDC offset

// TODO: finish load_stl
// / zoom to extents as part of load (pass camera2D to load)
// app gets its own key_toggled

// TODO: build volume
// TODO: the Hard problem of avoiding the creation of ultra-thin features


#include "cs345.cpp"
#include "poe.cpp"
#undef real // ???


real32 EPSILON_DEFAULT = 0.03f;
real32 Z_FIGHT_EPS = 0.2f;
real32 TOLERANCE_DEFAULT = 1e-5f;
u32 NUM_SEGMENTS_PER_CIRCLE = 64;



////////////////////////////////////////
// 2D //////////////////////////////////
////////////////////////////////////////



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

void dxf_free(DXF *dxf) {
    if (dxf->entities) free(dxf->entities);
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

    FILE *file = (FILE *) fopen(filename, "r");
    if (!file) {
        printf("\"%s\" not found.\n", filename);
        return result;
    }

    List<DXFEntity> stretchy_list = {}; {
        {
            #define DXF_LOAD_MODE_NONE 0
            #define DXF_LOAD_MODE_LINE 1
            #define DXF_LOAD_MODE_ARC  2
            u8 mode = 0;
            int code = 0;
            bool32 code_is_hot = false;
            DXFLine line = {};
            DXFArc arc = {};
            static char buffer[512];
            while (fgets(buffer, ARRAY_LENGTH(buffer), file)) {
                if (mode == DXF_LOAD_MODE_NONE) {
                    if (poe_prefix_match(buffer, "LINE")) {
                        mode = DXF_LOAD_MODE_LINE;
                        code_is_hot = false;
                        line = {};
                    } else if (poe_prefix_match(buffer, "ARC")) {
                        mode = DXF_LOAD_MODE_ARC;
                        code_is_hot = false;
                        arc = {};
                    }
                } else {
                    if (!code_is_hot) {
                        sscanf(buffer, "%d", &code);
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
                            sscanf(buffer, "%d", &value);
                            line.color = arc.color = value; 
                        } else {
                            float value;
                            sscanf(buffer, "%f", &value);
                            if (mode == DXF_LOAD_MODE_LINE) {
                                if (code == 10) {
                                    line.start_x = MM(value);
                                } else if (code == 20) {
                                    line.start_y = MM(value);
                                } else if (code == 11) {
                                    line.end_x = MM(value);
                                } else if (code == 21) {
                                    line.end_y = MM(value);
                                }
                            } else {
                                ASSERT(mode == DXF_LOAD_MODE_ARC);
                                if (code == 10) {
                                    arc.center_x = MM(value);
                                } else if (code == 20) {
                                    arc.center_y = MM(value);
                                } else if (code == 40) {
                                    arc.radius = MM(value);
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
        }
    }

    fclose(file);

    result.num_entities = stretchy_list.length;
    result.entities = (DXFEntity *) calloc(result.num_entities, sizeof(DXFEntity));
    memcpy(result.entities, stretchy_list.data, result.num_entities * sizeof(DXFEntity));
    list_free(&stretchy_list);
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

void dxf_debug_draw(Camera2D *camera2D, DXF *dxf, int32 override_color = DXF_COLOR_DONT_OVERRIDE) {
    eso_begin(camera_get_PV(camera2D), SOUP_LINES);
    for (DXFEntity *entity = dxf->entities; entity < &dxf->entities[dxf->num_entities]; ++entity) {
        eso_dxf_entity__SOUP_LINES(entity, override_color);
    }
    eso_end();
}

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
    return POW(SQRT(squared_distance_point_point(x, y, center_x, center_y)) - radius, 2);
}

real32 squared_distance_point_arc_NOTE_pass_angles_in_radians(real32 x, real32 y, real32 center_x, real32 center_y, real32 radius, real32 start_angle_in_radians, real32 end_angle_in_radians) {
    bool32 point_in_sector = false; {
        real32 v_x = x - center_x;
        real32 v_y = y - center_y;
        // forgive me rygorous :(
        real32 angle = atan2(v_y, v_x);
        while (start_angle_in_radians < -PI) start_angle_in_radians += TAU;
        while (end_angle_in_radians < start_angle_in_radians) end_angle_in_radians += TAU;
        point_in_sector =
            IS_BETWEEN(angle, start_angle_in_radians, end_angle_in_radians)
            || IS_BETWEEN(angle + TAU, start_angle_in_radians, end_angle_in_radians)
            || IS_BETWEEN(angle - TAU, start_angle_in_radians, end_angle_in_radians);
    }
    if (point_in_sector) {
        return squared_distance_point_circle(x, y, center_x, center_y, radius);
    } else {
        real32 start_x, start_y, end_x, end_y;
        get_point_on_circle_NOTE_pass_angle_in_radians(&start_x, &start_y, center_x, center_y, radius, start_angle_in_radians);
        get_point_on_circle_NOTE_pass_angle_in_radians(  &end_x,   &end_y, center_x, center_y, radius, end_angle_in_radians);
        return MIN(squared_distance_point_point(x, y, start_x, start_y), squared_distance_point_point(x, y, end_x, end_y));
    }
}

real32 squared_distance_point_entity(real32 x, real32 y, DXFEntity *entity) {
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLine *line = &entity->line;
        return squared_distance_point_line_segment(x, y, line->start_x, line->start_y, line->end_x, line->end_y);
    } else {
        ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
        DXFArc *arc = &entity->arc;
        return squared_distance_point_arc_NOTE_pass_angles_in_radians(x, y, arc->center_x, arc->center_y, arc->radius, RAD(arc->start_angle_in_degrees), RAD(arc->end_angle_in_degrees));
    }
}

real32 squared_distance_point_dxf(real32 x, real32 y, DXF *dxf) {
    real32 result = HUGE_VAL;
    for (DXFEntity *entity = dxf->entities; entity < &dxf->entities[dxf->num_entities]; ++entity) {
        result = MIN(result, squared_distance_point_entity(x, y, entity));
    }
    return result;
}

struct DXFEntityIndexAndFlipFlag {
    u32 entity_index;
    bool32 flip_flag;
};

struct DXFLoopAnalysisResult {
    u32 num_loops;
    u32 *num_entities_in_loops;
    DXFEntityIndexAndFlipFlag **loops;
    u32 *loop_index_from_entity_index;
};

DXFLoopAnalysisResult dxf_loop_analysis_create(DXF *dxf, bool32 *dxf_selection_mask = NULL) {
    if (dxf->num_entities == 0) {
        DXFLoopAnalysisResult result = {};
        result.num_loops = 0;
        result.num_entities_in_loops = (u32 *) calloc(result.num_loops, sizeof(u32));
        result.loops = (DXFEntityIndexAndFlipFlag **) calloc(result.num_loops, sizeof(DXFEntityIndexAndFlipFlag *));
        result.loop_index_from_entity_index = (u32 *) calloc(dxf->num_entities, sizeof(u32));
        return result;
    }

    DXFLoopAnalysisResult result = {};
    { // num_entities_in_loops, loops
      // populate List's
        List<List<DXFEntityIndexAndFlipFlag>> stretchy_list = {}; {
            bool32 *entity_already_added = (bool32 *) calloc(dxf->num_entities, sizeof(bool32));
            while (true) {
                #define MACRO_CANDIDATE_VALID(i) (!entity_already_added[i] && (!dxf_selection_mask || dxf_selection_mask[i]))
                { // seed loop
                    bool32 added_and_seeded_new_loop = false;
                    for (u32 entity_index = 0; entity_index < dxf->num_entities; ++entity_index) {
                        if (MACRO_CANDIDATE_VALID(entity_index)) {
                            added_and_seeded_new_loop = true;
                            entity_already_added[entity_index] = true;
                            list_push_back(&stretchy_list, {});
                            list_push_back(&stretchy_list.data[stretchy_list.length - 1], { entity_index, false });
                            break;
                        }
                    }
                    if (!added_and_seeded_new_loop) break;
                }
                { // continue and complete
                    real32 tolerance = TOLERANCE_DEFAULT;
                    while (true) {
                        bool32 added_new_entity_to_loop = false;
                        for (u32 entity_index = 0; entity_index < dxf->num_entities; ++entity_index) {
                            if (!MACRO_CANDIDATE_VALID(entity_index)) continue;
                            real32 start_x_prev, start_y_prev, end_x_prev, end_y_prev;
                            real32 start_x_i, start_y_i, end_x_i, end_y_i;
                            DXFEntityIndexAndFlipFlag *prev_entity_index_and_flip_flag = &(stretchy_list.data[stretchy_list.length - 1].data[stretchy_list.data[stretchy_list.length - 1].length - 1]);
                            {
                                entity_get_start_and_end_points(
                                        &dxf->entities[prev_entity_index_and_flip_flag->entity_index],
                                        &start_x_prev, &start_y_prev, &end_x_prev, &end_y_prev);
                                entity_get_start_and_end_points(&dxf->entities[entity_index], &start_x_i, &start_y_i, &end_x_i, &end_y_i);
                            }
                            bool32 is_next_entity = false;
                            bool32 flip_flag = false;
                            if (!prev_entity_index_and_flip_flag->flip_flag) {
                                if (squared_distance_point_point(end_x_prev, end_y_prev, start_x_i, start_y_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = false;
                                } else if (squared_distance_point_point(end_x_prev, end_y_prev, end_x_i, end_y_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = true;
                                }
                            } else {
                                if (squared_distance_point_point(start_x_prev, start_y_prev, start_x_i, start_y_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = false;
                                } else if (squared_distance_point_point(start_x_prev, start_y_prev, end_x_i, end_y_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = true;
                                }
                            }
                            if (is_next_entity) {
                                added_new_entity_to_loop = true;
                                entity_already_added[entity_index] = true;
                                list_push_back(&stretchy_list.data[stretchy_list.length - 1], { entity_index, flip_flag });
                                break;
                            }
                        }
                        if (!added_new_entity_to_loop) break;
                    }
                }

                { // reverse_loop if necessary
                    u32 num_entities_in_loop = stretchy_list.data[stretchy_list.length - 1].length;
                    DXFEntityIndexAndFlipFlag *loop = stretchy_list.data[stretchy_list.length - 1].data;
                    bool32 reverse_loop; {
                        #if 0
                        reverse_loop = false;
                        #else
                        real32 twice_the_signed_area; {
                            twice_the_signed_area = 0.0f;
                            for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities_in_loop; ++entity_index_and_flip_flag) {
                                u32 entity_index = entity_index_and_flip_flag->entity_index;
                                bool32 flip_flag = entity_index_and_flip_flag->flip_flag;
                                DXFEntity *entity = &dxf->entities[entity_index];
                                if (entity->type == DXF_ENTITY_TYPE_LINE) {
                                    DXFLine *line = &entity->line;
                                    // shoelace-type formula
                                    twice_the_signed_area += ((flip_flag) ? -1 : 1) * (line->start_x * line->end_y - line->end_x * line->start_y);
                                } else {
                                    ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                                    DXFArc *arc = &entity->arc;
                                    // "Circular approximation using polygons"
                                    // - n = 2 (area-preserving approximation of arc with two segments)
                                    real32 start_angle, end_angle;
                                    arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, flip_flag);
                                    real32 start_x, start_y, end_x, end_y;
                                    get_point_on_circle_NOTE_pass_angle_in_radians(&start_x, &start_y, arc->center_x, arc->center_y, arc->radius, start_angle);
                                    get_point_on_circle_NOTE_pass_angle_in_radians(  &end_x,   &end_y, arc->center_x, arc->center_y, arc->radius,   end_angle);
                                    real32 mid_angle = (start_angle + end_angle) / 2;
                                    real32 d; {
                                        real32 alpha = ABS(start_angle - end_angle) / 2;
                                        d = arc->radius * alpha / SIN(alpha);
                                    }
                                    real32 mid_x, mid_y;
                                    get_point_on_circle_NOTE_pass_angle_in_radians(&mid_x, &mid_y, arc->center_x, arc->center_y, d, mid_angle);
                                    twice_the_signed_area += mid_x * (end_y - start_y) + mid_y * (start_x - end_x);
                                }
                            }
                        }
                        reverse_loop = (twice_the_signed_area < 0.0f);
                        #endif
                    }
                    if (reverse_loop) {
                        for (u32 i = 0, j = (num_entities_in_loop - 1); i < j; ++i, --j) {
                            DXFEntityIndexAndFlipFlag tmp = loop[i];
                            loop[i] = loop[j];
                            loop[j] = tmp;
                        }
                        for (u32 i = 0; i < num_entities_in_loop; ++i) {
                            loop[i].flip_flag = !loop[i].flip_flag;
                        }
                    }
                }
                #undef MACRO_CANDIDATE_VALID
            }
            free(entity_already_added);
        }

        // copy over from List's
        result.num_loops = stretchy_list.length;
        result.num_entities_in_loops = (u32 *) calloc(result.num_loops, sizeof(u32));
        result.loops = (DXFEntityIndexAndFlipFlag **) calloc(result.num_loops, sizeof(DXFEntityIndexAndFlipFlag *));
        for (u32 i = 0; i < result.num_loops; ++i) {
            result.num_entities_in_loops[i] = stretchy_list.data[i].length;
            result.loops[i] = (DXFEntityIndexAndFlipFlag *) calloc(result.num_entities_in_loops[i], sizeof(DXFEntityIndexAndFlipFlag));
            memcpy(result.loops[i], stretchy_list.data[i].data, result.num_entities_in_loops[i] * sizeof(DXFEntityIndexAndFlipFlag));
        }

        // free List's
        for (int i = 0; i < stretchy_list.length; ++i) list_free(&stretchy_list.data[i]);
        list_free(&stretchy_list);
    }
    // loop_index_from_entity_index (brute force)
    result.loop_index_from_entity_index = (u32 *) calloc(dxf->num_entities, sizeof(u32));
    for (u32 i = 0; i < dxf->num_entities; ++i) {
        for (u32 j = 0; j < result.num_loops; ++j) {
            for (u32 k = 0; k < result.num_entities_in_loops[j]; ++k) {
                if (i == result.loops[j][k].entity_index) {
                    result.loop_index_from_entity_index[i] = j;
                    break;
                }
            }
        }
    }
    return result;
}

void dxf_loop_analysis_free(DXFLoopAnalysisResult *analysis) {
    free(analysis->num_entities_in_loops);
    for (u32 i = 0; i < analysis->num_loops; ++i) {
        free(analysis->loops[i]);
    }
    free(analysis->loop_index_from_entity_index);
}


#define SELECT_MODE_NONE 0
#define SELECT_MODE_SELECT 1
#define SELECT_MODE_DESELECT 2
#define _SELECT_MODE_DEFAULT SELECT_MODE_NONE
#define SELECT_MODIFIER_NONE 0
#define SELECT_MODIFIER_CONNECTED 1
#define SELECT_MODIFIER_QUALITY 2
#define _SELECT_MODIFIER_DEFAULT SELECT_MODIFIER_NONE
void dxf_pick(mat4 PV_2D, real32 camera2D_screen_height_World, DXF *dxf, bool32 *dxf_selection_mask, u32 select_mode, u32 select_modifier, u32 *num_entities_in_pick_loops, DXFEntityIndexAndFlipFlag **pick_loops, u32 *pick_loop_index_from_entity_index, real32 epsilon = EPSILON_DEFAULT) {
    if (!globals.mouse_left_held) return;
    if (select_mode == SELECT_MODE_NONE) return;

    bool32 value_to_write_to_selection_mask = (select_mode == SELECT_MODE_SELECT);
    bool32 modifier_connected = (select_modifier == SELECT_MODIFIER_CONNECTED);

    real32 x, y;
    _input_get_mouse_position_and_change_in_position_in_world_coordinates(PV_2D.data, &x, &y, NULL, NULL);

    int hot_entity_index = -1;
    double hot_squared_distance = HUGE_VAL;
    for (u32 i = 0; i < dxf->num_entities; ++i) {
        DXFEntity *entity = &dxf->entities[i];
        double squared_distance = squared_distance_point_entity(x, y, entity);
        squared_distance /= (camera2D_screen_height_World * camera2D_screen_height_World / 4); // NDC
        if (squared_distance < MIN(epsilon * epsilon, hot_squared_distance)) {
            hot_squared_distance = squared_distance;
            hot_entity_index = i;
        }
    }

    if (hot_entity_index != -1) {
        if (globals.mouse_left_held) {
            if (!modifier_connected) {
                dxf_selection_mask[hot_entity_index] = value_to_write_to_selection_mask;
            } else {
                u32 loop_index = pick_loop_index_from_entity_index[hot_entity_index];
                DXFEntityIndexAndFlipFlag *loop = pick_loops[loop_index];
                u32 num_entities = num_entities_in_pick_loops[loop_index];
                for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities; ++entity_index_and_flip_flag) {
                    dxf_selection_mask[entity_index_and_flip_flag->entity_index] = value_to_write_to_selection_mask;
                }
            }
        }
    }
}

// NOTE: even odd
struct CrossSection {
    u32 num_polygonal_loops;
    u32 *num_vertices_in_polygonal_loops;
    vec2 **polygonal_loops;
};

CrossSection cross_section_create(DXF *dxf, bool32 *dxf_selection_mask) {
    #if 0
    {
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(dxf);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(dxf_selection_mask);
        CrossSection result = {};
        result.num_polygonal_loops = 2;
        result.num_vertices_in_polygonal_loops = (u32 *) calloc(result.num_polygonal_loops, sizeof(u32));
        result.num_vertices_in_polygonal_loops[0] = 4;
        result.num_vertices_in_polygonal_loops[1] = 6;
        result.polygonal_loops = (vec2 **) calloc(result.num_polygonal_loops, sizeof(vec2 *));
        result.polygonal_loops[0] = (vec2 *) calloc(result.num_vertices_in_polygonal_loops[0], sizeof(vec2));
        result.polygonal_loops[1] = (vec2 *) calloc(result.num_vertices_in_polygonal_loops[1], sizeof(vec2));
        result.polygonal_loops[0][0] = { -2.0f, -2.0f };
        result.polygonal_loops[0][1] = {  2.0f, -2.0f };
        result.polygonal_loops[0][2] = {  2.0f,  2.0f };
        result.polygonal_loops[0][3] = { -2.0f,  2.0f };
        result.polygonal_loops[1][0] = { COS(RAD(  0)), SIN(RAD(  0)) };
        result.polygonal_loops[1][1] = { COS(RAD( 60)), SIN(RAD( 60)) };
        result.polygonal_loops[1][2] = { COS(RAD(120)), SIN(RAD(120)) };
        result.polygonal_loops[1][3] = { COS(RAD(180)), SIN(RAD(180)) };
        result.polygonal_loops[1][4] = { COS(RAD(240)), SIN(RAD(240)) };
        result.polygonal_loops[1][5] = { COS(RAD(300)), SIN(RAD(300)) };
        return result;
    }
    #endif
    // populate List's
    List<List<vec2>> stretchy_list = {}; {
        DXFLoopAnalysisResult analysis = dxf_loop_analysis_create(dxf, dxf_selection_mask);
        for (u32 loop_index = 0; loop_index < analysis.num_loops; ++loop_index) {
            u32 num_entities_in_loop = analysis.num_entities_in_loops[loop_index];
            DXFEntityIndexAndFlipFlag *loop = analysis.loops[loop_index];
            list_push_back(&stretchy_list, {});
            for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities_in_loop; ++entity_index_and_flip_flag) {
                u32 entity_index = entity_index_and_flip_flag->entity_index;
                bool32 flip_flag = entity_index_and_flip_flag->flip_flag;
                DXFEntity *entity = &dxf->entities[entity_index];
                if (entity->type == DXF_ENTITY_TYPE_LINE) {
                    DXFLine *line = &entity->line;
                    if (!flip_flag) {
                        list_push_back(&stretchy_list.data[stretchy_list.length - 1], { line->start_x, line->start_y });
                    } else {
                        list_push_back(&stretchy_list.data[stretchy_list.length - 1], { line->end_x, line->end_y });
                    }
                } else {
                    ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                    DXFArc *arc = &entity->arc;
                    real32 start_angle, end_angle;
                    arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, flip_flag);
                    real32 delta_angle = end_angle - start_angle;
                    u32 num_segments = (u32) (2 + ABS(delta_angle) * (NUM_SEGMENTS_PER_CIRCLE / TAU)); // FORNOW (2 + ...)
                    real32 increment = delta_angle / num_segments;
                    real32 current_angle = start_angle;
                    real32 x, y;
                    for (u32 i = 0; i < num_segments; ++i) {
                        get_point_on_circle_NOTE_pass_angle_in_radians(&x, &y, arc->center_x, arc->center_y, arc->radius, current_angle);
                        list_push_back(&stretchy_list.data[stretchy_list.length - 1], { x, y });
                        current_angle += increment;
                    }
                }
            }
        }
        dxf_loop_analysis_free(&analysis);
    }

    // copy over from List's
    CrossSection result = {};
    result.num_polygonal_loops = stretchy_list.length;
    result.num_vertices_in_polygonal_loops = (u32 *) calloc(result.num_polygonal_loops, sizeof(u32));
    result.polygonal_loops = (vec2 **) calloc(result.num_polygonal_loops, sizeof(vec2 *));
    for (u32 i = 0; i < result.num_polygonal_loops; ++i) {
        result.num_vertices_in_polygonal_loops[i] = stretchy_list.data[i].length;
        result.polygonal_loops[i] = (vec2 *) calloc(result.num_vertices_in_polygonal_loops[i], sizeof(vec2));
        memcpy(result.polygonal_loops[i], stretchy_list.data[i].data, result.num_vertices_in_polygonal_loops[i] * sizeof(vec2));
    }

    // free List's
    for (int i = 0; i < stretchy_list.length; ++i) list_free(&stretchy_list.data[i]);
    list_free(&stretchy_list);

    return result;
}

void cross_section_debug_draw(Camera2D *camera2D, CrossSection *cross_section) {
    eso_begin(camera_get_PV(camera2D), SOUP_LINES);
    eso_color(monokai.white);
    for (u32 loop_index = 0; loop_index < cross_section->num_polygonal_loops; ++loop_index) {
        vec2 *polygonal_loop = cross_section->polygonal_loops[loop_index];
        int n = cross_section->num_vertices_in_polygonal_loops[loop_index];
        for (int j = 0, i = n - 1; j < n; i = j++) {
            real32 a_x = polygonal_loop[i].x;
            real32 a_y = polygonal_loop[i].y;
            real32 b_x = polygonal_loop[j].x;
            real32 b_y = polygonal_loop[j].y;
            eso_color(color_rainbow_swirl(real32(i) / (n)));
            eso_vertex(a_x, a_y);
            eso_color(color_rainbow_swirl(real32(i + 1) / (n)));
            eso_vertex(b_x, b_y);

            // draw normal
            real32 c_x = (a_x + b_x) / 2;
            real32 c_y = (a_y + b_y) / 2;
            real32 n_x = b_y - a_y;
            real32 n_y = a_x - b_x;
            real32 norm_n = SQRT(n_x * n_x + n_y * n_y);
            real32 L = 0.013f;
            eso_color(color_rainbow_swirl((i + 0.5f) / (n)));
            eso_vertex(c_x, c_y);
            eso_vertex(c_x + L * n_x / norm_n, c_y + L * n_y / norm_n);
        }
    }
    eso_end();
}




////////////////////////////////////////
// 3D //////////////////////////////////
////////////////////////////////////////




#define ENTER_MODE_NONE             0
#define ENTER_MODE_EXTRUDE_ADD      1
#define ENTER_MODE_EXTRUDE_SUBTRACT 2
#define ENTER_MODE_REVOLVE_ADD      3
#define ENTER_MODE_REVOLVE_SUBTRACT 4
#define ENTER_MODE_LOAD             5
#define ENTER_MODE_SAVE             6
#define _ENTER_MODE_DEFAULT ENTER_MODE_EXTRUDE_ADD




struct ConversationMesh {
    u32 num_vertices;
    u32 num_triangles;
    real32 *vertex_positions;
    u32 *triangle_indices;
    real32 *triangle_normals;

    u32 num_cosmetic_edges;
    u32 *cosmetic_edges;
};

void eso_vertex(real32 *p_j) {
    eso_vertex(p_j[0], p_j[1], p_j[2]);
}

void eso_vertex(real32 *p, u32 j) {
    eso_vertex(p[3 * j + 0], p[3 * j + 1], p[3 * j + 2]);
}

void conversation_mesh_save_stl(ConversationMesh *conversation_mesh, char *filename) {
    FILE *file = fopen(filename, "wb");
    ASSERT(file);

    int num_bytes = 80 + 4 + 50 * conversation_mesh->num_triangles;
    char *buffer = (char *) calloc(num_bytes, 1); {
        int offset = 80;
        memcpy(buffer + offset, &conversation_mesh->num_triangles, 4);
        offset += 4;
        for (u32 i = 0; i < conversation_mesh->num_triangles; ++i) {
            real32 triangle_normal[3];
            {
                // // NOTE: 90-degree rotation about x
                // x <- x
                // y <- -z
                // z <- y
                triangle_normal[0] =  conversation_mesh->triangle_normals[3 * i + 0];
                triangle_normal[1] = -conversation_mesh->triangle_normals[3 * i + 2];
                triangle_normal[2] =  conversation_mesh->triangle_normals[3 * i + 1];
            }
            memcpy(buffer + offset, &triangle_normal, 12);
            offset += 12;
            real32 triangle_vertex_positions[9];
            for (u32 j = 0; j < 3; ++j) {
                // // NOTE: 90-degree rotation about x
                // x <- x
                // y <- -z
                // z <- y
                triangle_vertex_positions[3 * j + 0] =  conversation_mesh->vertex_positions[3 * conversation_mesh->triangle_indices[3 * i + j] + 0];
                triangle_vertex_positions[3 * j + 1] = -conversation_mesh->vertex_positions[3 * conversation_mesh->triangle_indices[3 * i + j] + 2];
                triangle_vertex_positions[3 * j + 2] =  conversation_mesh->vertex_positions[3 * conversation_mesh->triangle_indices[3 * i + j] + 1];
            }
            memcpy(buffer + offset, triangle_vertex_positions, 36);
            offset += 38;
        }
    }
    fwrite(buffer, 1, num_bytes, file);
    free(buffer);

    fclose(file);
}

void conversation_mesh_free(ConversationMesh *conversation_mesh) {
    if (conversation_mesh->vertex_positions) free(conversation_mesh->vertex_positions);
    if (conversation_mesh->triangle_indices) free(conversation_mesh->triangle_indices);
    if (conversation_mesh->triangle_normals) free(conversation_mesh->triangle_normals);
    if (conversation_mesh->cosmetic_edges) free(conversation_mesh->cosmetic_edges);
    *conversation_mesh = {};
}


#include "manifoldc.h"
void wrapper_manifold(
        ManifoldManifold **curr_manifold,
        ConversationMesh *conversation_mesh, // dest__NOTE_GETS_OVERWRITTEN,
        u32 num_polygonal_loops,
        u32 *num_vertices_in_polygonal_loops,
        vec2 **polygonal_loops,
        mat4 M_selected,
        u32 enter_mode,
        bool32 extrude_param_sign_toggle,
        real32 extrude_param,
        real32 extrude_param_2) {
    ASSERT(enter_mode != ENTER_MODE_NONE);
    ManifoldManifold *other_manifold; {
        ManifoldSimplePolygon **simple_polygon_array = (ManifoldSimplePolygon **) malloc(num_polygonal_loops * sizeof(ManifoldSimplePolygon *));
        for (u32 i = 0; i < num_polygonal_loops; ++i) {
            simple_polygon_array[i] = manifold_simple_polygon(malloc(manifold_simple_polygon_size()), (ManifoldVec2 *) polygonal_loops[i], num_vertices_in_polygonal_loops[i]);
        }
        ManifoldPolygons *polygons = manifold_polygons(malloc(manifold_polygons_size()), simple_polygon_array, num_polygonal_loops);
        ManifoldCrossSection *cross_section = manifold_cross_section_of_polygons(malloc(manifold_cross_section_size()), polygons, ManifoldFillRule::MANIFOLD_FILL_RULE_EVEN_ODD);


        { // other_manifold

            // FORNOW: HACK
            if (enter_mode == ENTER_MODE_EXTRUDE_SUBTRACT) {
                do_once { printf("[hack] inflating ENTER_MODE_EXTRUDE_SUBTRACT\n");};
                extrude_param += TOLERANCE_DEFAULT;
                extrude_param_2 += TOLERANCE_DEFAULT;
            }

            if (enter_mode == ENTER_MODE_EXTRUDE_ADD || enter_mode == ENTER_MODE_EXTRUDE_SUBTRACT) {
                other_manifold = manifold_extrude(malloc(manifold_manifold_size()), cross_section, extrude_param + extrude_param_2, 0, 0.0f, 1.0f, 1.0f);
                other_manifold = manifold_translate(other_manifold, other_manifold, 0.0f, 0.0f, -extrude_param_2);
                if (extrude_param_sign_toggle) other_manifold = manifold_mirror(other_manifold, other_manifold, 0.0f, 0.0f, 1.0f);
            } else {
                other_manifold = manifold_revolve(malloc(manifold_manifold_size()), cross_section, NUM_SEGMENTS_PER_CIRCLE);
                other_manifold = manifold_rotate(other_manifold, other_manifold, -90.0f, 0.0f, 0.0f);
            }
            other_manifold = manifold_transform(other_manifold, other_manifold,
                    M_selected(0, 0), M_selected(1, 0), M_selected(2, 0),
                    M_selected(0, 1), M_selected(1, 1), M_selected(2, 1),
                    M_selected(0, 2), M_selected(1, 2), M_selected(2, 2),
                    M_selected(0, 3), M_selected(1, 3), M_selected(2, 3));
        }
    }

    // add
    if (!(*curr_manifold)) {
        if (enter_mode == ENTER_MODE_EXTRUDE_SUBTRACT) { return; } // FORNOW

        *curr_manifold = other_manifold;
    } else {
        // TODO: ? manifold_delete_manifold(curr_manifold);
        *curr_manifold =
            manifold_boolean(
                    malloc(manifold_manifold_size()),
                    *curr_manifold,
                    other_manifold,
                    ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_ADD)) ? ManifoldOpType::MANIFOLD_ADD : ManifoldOpType::MANIFOLD_SUBTRACT
                    );
    }

    ManifoldMeshGL *meshgl = manifold_get_meshgl(malloc(manifold_meshgl_size()), *curr_manifold);

    conversation_mesh_free(conversation_mesh);
    conversation_mesh->num_vertices = manifold_meshgl_num_vert(meshgl);
    conversation_mesh->num_triangles = manifold_meshgl_num_tri(meshgl);
    conversation_mesh->vertex_positions = manifold_meshgl_vert_properties(malloc(manifold_meshgl_vert_properties_length(meshgl) * sizeof(real32)), meshgl);
    conversation_mesh->triangle_indices = manifold_meshgl_tri_verts(malloc(manifold_meshgl_tri_length(meshgl) * sizeof(u32)), meshgl);

    { // triangle_normals
      // FORNOW: uses snail
      // TODO: remove dependency
        conversation_mesh->triangle_normals = (real32 *) malloc(conversation_mesh->num_triangles * 3 * sizeof(real32));
        vec3 p[3];
        for (u32 i = 0; i < conversation_mesh->num_triangles; ++i) {
            for (u32 j = 0; j < 3; ++j) for (u32 d = 0; d < 3; ++d) p[j][d] = conversation_mesh->vertex_positions[3 * conversation_mesh->triangle_indices[3 * i + j] + d];
            vec3 n = normalized(cross(p[1] - p[0], p[2] - p[0]));
            for (u32 d = 0; d < 3; ++d) conversation_mesh->triangle_normals[3 * i + d] = n[d];
        }
    }

    { // cosmetic_edges

        // approach: prep a big array that maps edge -> cwiseProduct of face normals (start it at 1, 1, 1) // (faces that edge is part of)
        //           iterate through all edges detministically (ccw in order, flipping as needed so lower_index->higher_index)
        //           then go back through the array and sum the components; if below some threshold (maybe...0.9?) add that index to a stretchy buffer

        // TODO: a linalg library that operates directly on real32 *'s (like you used in soft_robot.h)

        List<u32> list = {}; {
            Map<Pair<u32, u32>, vec3> map = {}; {
                for (u32 i = 0; i < conversation_mesh->num_triangles; ++i) {
                    vec3 n = { conversation_mesh->triangle_normals[3 * i + 0], conversation_mesh->triangle_normals[3 * i + 1], conversation_mesh->triangle_normals[3 * i + 2] };
                    for (u32 jj0 = 0, jj1 = (3 - 1); jj0 < 3; jj1 = jj0++) {
                        u32 j0 = conversation_mesh->triangle_indices[3 * i + jj0];
                        u32 j1 = conversation_mesh->triangle_indices[3 * i + jj1];
                        if (j0 > j1) {
                            u32 tmp = j0;
                            j0 = j1;
                            j1 = tmp;
                        }
                        Pair<u32, u32> key = { j0, j1 };
                        map_put(&map, key, cwiseProduct(n, map_get(&map, key, { 1.0f, 1.0f, 1.0f })));
                    }
                }
            }
            {
                for (List<Pair<Pair<u32, u32>, vec3>> *bucket = map.buckets; bucket < &map.buckets[map.num_buckets]; ++bucket) {
                    for (Pair<Pair<u32, u32>, vec3> *pair = bucket->data; pair < &bucket->data[bucket->length]; ++pair) {
                        vec3 n2 = pair->value;
                        // pprint(n2);
                        if ((n2.x + n2.y + n2.z) < 0.9f) {
                            list_push_back(&list, pair->key.first); // FORNOW
                            list_push_back(&list, pair->key.second); // FORNOW
                        }
                    }
                }
                // TODO: move this elsewhere
            }
            map_free(&map);
        }
        {
            conversation_mesh->num_cosmetic_edges = list.length / 2;
            conversation_mesh->cosmetic_edges = (u32 *) calloc(2 * conversation_mesh->num_cosmetic_edges, sizeof(u32));
            memcpy(conversation_mesh->cosmetic_edges, list.data, 2 * conversation_mesh->num_cosmetic_edges * sizeof(u32)); 
        }
        list_free(&list);
    }
}


real32 get(real32 *x, u32 i, u32 d) {
    return x[3 * i + d];
}
vec3 get(real32 *x, u32 i) {
    vec3 result;
    for (u32 d = 0; d < 3; ++d) result.data[d] = get(x, i, d);
    return result;
}
void add(real32 *x, u32 i, vec3 v) {
    for (u32 d = 0; d < 3; ++d) x[3 * i + d] += v.data[d];
}

// TODO: more of this
// TODO: cow_unsigned_int
union uzi3 {
    struct {
        u32 i;
        u32 j;
        u32 k;
    };
    u32 data[3];
};

u32 get(u32 *x, u32 i, u32 d) {
    return x[3 * i + d];
}
uzi3 get(u32 *x, u32 i) {
    uzi3 result;
    for (u32 d = 0; d < 3; ++d) result.data[d] = get(x, i, d);
    return result;
}


void stl_load(char *filename, ManifoldManifold **, ConversationMesh *conversation_mesh) {
    // TODO: actually load an STL (currently the rabbit is an obj)

    u32 num_triangles;
    real32 *soup;
    {
        List<real32> _soup = {}; {
            FILE *file = fopen(filename, "r");
            ASSERT(file);
            char buffer[4096];
            while (fgets(buffer, ARRAY_LENGTH(buffer), file) != NULL) {
                cow_real x, y, z;
                sscanf(buffer, "%f %f %f", &x, &y, &z);
                list_push_back(&_soup, 48.0f * x);
                list_push_back(&_soup, 48.0f * y);
                list_push_back(&_soup, 48.0f * z);
            }
            fclose(file);
        }
        num_triangles = _soup.length / 9; // / 3 / 3
        u32 size = _soup.length * sizeof(real32);
        soup = (real32 *) malloc(size);
        memcpy(soup, _soup.data, size);
        list_free(&_soup);
    }

    // unify nearby vertices
    // FORNOW: O(n^2)
    real32 MERGE_THRESHOLD_SQUARED_DISTANCE = powf(0.001f, 2);
    u32 num_vertices;
    real32 *vertex_positions;
    u32 *triangle_indices;
    {
        Map<u32, u32> map = {};
        {
            List<vec3> list = {};
            num_vertices = 0;
            u32 _num_vertices = 3 * num_triangles;
            for (u32 i = 0; i < _num_vertices; ++i) {
                bool32 found = false;
                vec3 p_i = get(soup, i);
                for (u32 h = 0; h < i; ++h) {
                    vec3 p_h = get(soup, h);
                    if (squaredNorm(p_i - p_h) < MERGE_THRESHOLD_SQUARED_DISTANCE) {
                        found = true;
                        map_put(&map, i, map_get(&map, h));
                        break;
                    }
                }
                if (!found) {
                    map_put(&map, i, num_vertices++);
                    list_push_back(&list, p_i);
                }
            }
            {
                u32 size = num_vertices * sizeof(vec3);
                vertex_positions = (real32 *) malloc(size);
                memcpy(vertex_positions, list.data, size);
                list_free(&list);
            }
        }
        triangle_indices = (u32 *) malloc(3 * num_triangles * sizeof(u32));
        for (u32 k = 0; k < 3 * num_triangles; ++k) triangle_indices[k] = map_get(&map, k);
    }



    real32 *triangle_normals; {
        triangle_normals = (real32 *) calloc(3 * num_triangles, sizeof(real32));;
        for (u32 triangle_index = 0; triangle_index < num_triangles; ++triangle_index) {
            vec3 a = get(vertex_positions, get(triangle_indices, triangle_index, 0));
            vec3 b = get(vertex_positions, get(triangle_indices, triangle_index, 1));
            vec3 c = get(vertex_positions, get(triangle_indices, triangle_index, 2));
            add(triangle_normals, triangle_index, normalized(cross(b - a, c - a)));
        }
    }

    conversation_mesh->num_vertices = num_vertices;
    conversation_mesh->num_triangles = num_triangles;
    conversation_mesh->vertex_positions = vertex_positions;
    conversation_mesh->triangle_indices = triangle_indices;
    conversation_mesh->triangle_normals = triangle_normals;
    // TODO (easy--just strip out code you already have into a function): cosmetic edges
    conversation_mesh->num_cosmetic_edges = 0;
    conversation_mesh->cosmetic_edges = NULL;
}










bool *key_pressed = globals.key_pressed;
bool *key_toggled = globals.key_toggled;


// FORNOW M_selected
mat4 get_M_selected(vec3 n_selected, real32 r_n_selected) {
    vec3 up = { 0.0f, 1.0f, 0.0f };
    real32 dot_product = dot(n_selected, up);
    vec3 y = (ARE_EQUAL(ABS(dot_product), 1.0f)) ? V3(0.0f,  0.0f, -1.0f * SGN(dot_product)) : up;
    vec3 x = normalized(cross(y, n_selected));
    vec3 z = cross(x, y);
    return M4_xyzo(x, y, z, r_n_selected * n_selected);
}

void camera2D_zoom_to_dxf_extents(Camera2D *camera2D, DXF *dxf) {
    real32 min[] = {  HUGE_VAL,  HUGE_VAL };
    real32 max[] = { -HUGE_VAL, -HUGE_VAL };
    for (DXFEntity *entity = dxf->entities; entity < &dxf->entities[dxf->num_entities]; ++entity) {
        real32 s[2][2];
        u32 n = 2;
        entity_get_start_and_end_points(entity, &s[0][0], &s[0][1], &s[1][0], &s[1][1]);
        for (u32 i = 0; i < n; ++i) {
            for (u32 d = 0; d < 2; ++d) {
                min[d] = MIN(min[d], s[i][d]);
                max[d] = MAX(max[d], s[i][d]);
            }
        }

        if (entity->type == DXF_ENTITY_TYPE_ARC) {
            DXFArc *arc = &entity->arc;
            // NOTE: endpoints already taken are of
            real32 start_angle = RAD(arc->start_angle_in_degrees);
            real32 end_angle = RAD(arc->end_angle_in_degrees);
            { // FORNOW
                while (start_angle < 0.0f) {
                    start_angle += TAU;
                    end_angle += TAU;
                }
                while (start_angle > TAU) {
                    start_angle -= TAU;
                    end_angle -= TAU;
                }
            }
            if (IS_BETWEEN(         0, start_angle, end_angle)) max[0] = MAX(max[0], arc->center_x + arc->radius);
            if (IS_BETWEEN(    PI / 2, start_angle, end_angle)) max[1] = MAX(max[1], arc->center_y + arc->radius);
            if (IS_BETWEEN(    PI    , start_angle, end_angle)) min[0] = MIN(min[0], arc->center_x - arc->radius);
            if (IS_BETWEEN(3 * PI / 2, start_angle, end_angle)) min[1] = MIN(min[1], arc->center_y - arc->radius);

        }
    }
    real32 new_o_x = AVG(min[0], max[0]);
    real32 new_o_y = AVG(min[1], max[1]);
    real32 new_height = MAX((max[0] - min[0]) * 2 / _window_get_aspect(), (max[1] - min[1])); // factor of 2 since splitscreen
    new_height *= 1.3f; // FORNOW: border
    camera2D->screen_height_World = new_height;
    camera2D->o_x = new_o_x;
    camera2D->o_y = new_o_y;
}




int _grid_box_num_triangles = 12;
int _grid_box_num_vertices = 24;
int3 _grid_box_triangle_indices[] = {
    { 1, 0, 2},{ 2, 0, 3},
    { 4, 5, 6},{ 4, 6, 7},
    { 8, 9,10},{ 8,10,11},
    {13,12,14},{14,12,15},
    {17,16,18},{18,16,19},
    {20,21,22},{20,22,23},
};
vec3 _grid_box_vertex_positions[] = {
    { 1, 1, 1},{ 1, 1,-1},{ 1,-1,-1},{ 1,-1, 1},
    {-1, 1, 1},{-1, 1,-1},{-1,-1,-1},{-1,-1, 1},
    { 1, 1, 1},{ 1, 1,-1},{-1, 1,-1},{-1, 1, 1},
    { 1,-1, 1},{ 1,-1,-1},{-1,-1,-1},{-1,-1, 1},
    { 1, 1, 1},{ 1,-1, 1},{-1,-1, 1},{-1, 1, 1},
    { 1, 1,-1},{ 1,-1,-1},{-1,-1,-1},{-1, 1,-1},
};
vec3 _grid_box_vertex_colors[] = {
    { 0.8f, 0.8f, 0.8f},{ 0.8f, 0.8f,0.4f},{ 0.8f,0.4f,0.4f},{ 0.8f,0.4f, 0.8f},
    {0.4f, 0.8f, 0.8f},{0.4f, 0.8f,0.4f},{0.4f,0.4f,0.4f},{0.4f,0.4f, 0.8f},
    { 0.8f, 0.8f, 0.8f},{ 0.8f, 0.8f,0.4f},{0.4f, 0.8f,0.4f},{0.4f, 0.8f, 0.8f},
    { 0.8f,0.4f, 0.8f},{ 0.8f,0.4f,0.4f},{0.4f,0.4f,0.4f},{0.4f,0.4f, 0.8f},
    { 0.8f, 0.8f, 0.8f},{ 0.8f,0.4f, 0.8f},{0.4f,0.4f, 0.8f},{0.4f, 0.8f, 0.8f},
    { 0.8f, 0.8f,0.4f},{ 0.8f,0.4f,0.4f},{0.4f,0.4f,0.4f},{0.4f, 0.8f,0.4f},
};
vec2 _grid_box_vertex_texCoords[] = {
    {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
    {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
    {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
    {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
    {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
    {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
};

IndexedTriangleMesh3D grid_box = {
    _grid_box_num_vertices,
    _grid_box_num_triangles,
    _grid_box_vertex_positions,
    NULL,
    _grid_box_vertex_colors,
    _grid_box_triangle_indices,
    _grid_box_vertex_texCoords
};


BEGIN_PRE_MAIN {
    u32 width = 1024;
    u32 height = 1024;
    u32 number_of_channels = 3;
    u8 *data = (u8 *) malloc(width * height * 3 * sizeof(u8));
    for (u32 j = 0; j < height; ++j) {
        for (u32 i = 0; i < width; ++i) {
            u32 k = number_of_channels * (j * width + i);
            u32 n = 1024 / 10;
            u32 t = 4;
            bool32 stripe = ((i % n < t) || (j % n < t));
            u8 value = (stripe) ? 80 : 0;
            data[k + 0] = value;
            data[k + 1] = value;
            data[k + 2] = value;
        }
    }
    _mesh_texture_create("procedural grid", width, height, number_of_channels, data);
} END_PRE_MAIN;




#define HOT_PANE_NONE 0
#define HOT_PANE_2D   1
#define HOT_PANE_3D   2
u32 hot_pane;

u32 select_mode;
u32 select_modifier;

bool32 some_triangle_exists_that_matches_n_selected_and_r_n_selected; // NOTE: if this is false, then a plane is selected
vec3 n_selected;
real32 r_n_selected; // coordinate along n_selected
mat4 M_selected;
void selected_reset() {
    some_triangle_exists_that_matches_n_selected_and_r_n_selected = false;
    r_n_selected = 0.0f;
    n_selected = {};
    M_selected = {}; // FORNOW: implicit no selection
}

DXF dxf;
DXFLoopAnalysisResult pick;

bool32 *dxf_selection_mask;

ManifoldManifold *manifold;
ConversationMesh conversation_mesh;

u32 enter_mode;
real32 extrude_param;
real32 extrude_param_2;
bool32 extrude_param_sign_toggle;

char console_buffer[256];
char *console_buffer_write_head;
void console_buffer_reset() {
    memset(console_buffer, 0, ARRAY_LENGTH(console_buffer) * sizeof(char));
    console_buffer_write_head = console_buffer;
};

Camera2D camera2D;
Camera3D camera3D;
real32 CAMERA_3D_DEFAULT_ANGLE_OF_VIEW = RAD(60.0f);


bool32 show_grid, show_details, show_help;

char conversation_drop_path[512];

void conversation_load_file(char *filename) {
    if (poe_suffix_match(filename, ".dxf")) {
        dxf_free(&dxf);
        free(dxf_selection_mask);
        dxf = dxf_load(filename);
        pick = dxf_loop_analysis_create(&dxf);
        dxf_selection_mask = (bool32 *) calloc(dxf.num_entities, sizeof(bool32));
        camera2D_zoom_to_dxf_extents(&camera2D, &dxf);

        for (u32 i = 0; i < strlen(filename); ++i) {
            conversation_drop_path[i] = filename[i];
            if (filename[i] == '.') {
                while (
                        (i != 0) &&
                        (conversation_drop_path[i - 1] != '\\') &&
                        (conversation_drop_path[i - 1] != '/')
                      ) --i;
                conversation_drop_path[i] = '\0';
                break;
            }
        }
    } else if (poe_suffix_match(filename, ".stl")) {
        conversation_mesh_free(&conversation_mesh);
        stl_load(filename, &manifold, &conversation_mesh);
    } else {
        printf("%s filetype not supported.\n", filename);
    }
}

void drop_callback(GLFWwindow *, int count, const char** paths) {
    if (count > 0) {
        conversation_load_file((char *) paths[0]);
    }
}
BEGIN_PRE_MAIN {
    glfwSetDropCallback(COW0._window_glfw_window, drop_callback);
} END_PRE_MAIN;

int main() {

    bool32 reset = true;
    while (cow_begin_frame()) {
        if (reset) {
            reset = false;
            hot_pane = HOT_PANE_NONE;

            select_mode = _SELECT_MODE_DEFAULT;
            select_modifier = _SELECT_MODIFIER_DEFAULT;

            selected_reset(); 
            // some_triangle_exists_that_matches_n_selected_and_r_n_selected = false;
            // n_selected = { 0.0, 1.0, 0.0 };
            // r_n_selected = 0.0f;
            // M_selected = get_M_selected(n_selected, r_n_selected);

            dxf = dxf_load("in.dxf");
            pick = dxf_loop_analysis_create(&dxf);
            dxf_selection_mask = (bool32 *) calloc(dxf.num_entities, sizeof(bool32));

            manifold = NULL;
            conversation_mesh = {};
            #if 0
            {
                conversation_mesh = {};
                conversation_mesh.num_vertices = 4;
                conversation_mesh.num_triangles = 4;
                conversation_mesh.vertex_positions = (real32 *) calloc(3 * conversation_mesh.num_vertices, sizeof(real32));
                conversation_mesh.triangle_indices = (u32 *) calloc(3 * conversation_mesh.num_triangles, sizeof(u32));
                float h = (1.0f + SQRT(3.0f)) / 2;
                {
                    u32 k = 0;
                    conversation_mesh.vertex_positions[k++] = 100.0f * COS(RAD(0.0));
                    conversation_mesh.vertex_positions[k++] = 100.0f * 0.0f;
                    conversation_mesh.vertex_positions[k++] = 100.0f * SIN(RAD(0.0));
                    conversation_mesh.vertex_positions[k++] = 100.0f * COS(RAD(120.0));
                    conversation_mesh.vertex_positions[k++] = 100.0f * 0.0f;
                    conversation_mesh.vertex_positions[k++] = 100.0f * SIN(RAD(120.0));
                    conversation_mesh.vertex_positions[k++] = 100.0f * COS(RAD(240.0));
                    conversation_mesh.vertex_positions[k++] = 100.0f * 0.0f;
                    conversation_mesh.vertex_positions[k++] = 100.0f * SIN(RAD(240.0));
                    conversation_mesh.vertex_positions[k++] = 100.0f * 0.0f;
                    conversation_mesh.vertex_positions[k++] = 100.0f * h;
                    conversation_mesh.vertex_positions[k++] = 100.0f * 0.0f;
                }
                {
                    u32 k = 0;
                    conversation_mesh.triangle_indices[k++] = 0;
                    conversation_mesh.triangle_indices[k++] = 1;
                    conversation_mesh.triangle_indices[k++] = 2;
                    conversation_mesh.triangle_indices[k++] = 1;
                    conversation_mesh.triangle_indices[k++] = 0;
                    conversation_mesh.triangle_indices[k++] = 3;
                    conversation_mesh.triangle_indices[k++] = 2;
                    conversation_mesh.triangle_indices[k++] = 1;
                    conversation_mesh.triangle_indices[k++] = 3;
                    conversation_mesh.triangle_indices[k++] = 0;
                    conversation_mesh.triangle_indices[k++] = 2;
                    conversation_mesh.triangle_indices[k++] = 3;
                }
            }
            #endif

            enter_mode = _ENTER_MODE_DEFAULT;
            extrude_param = 0.0f;
            extrude_param_2 = 0.0f;
            extrude_param_sign_toggle = false;
            console_buffer_reset();

            { // cameras
                real32 height = 128.0f;
                camera2D = { height, 0.0, 0.0f, -0.5f, 0.0f };
                camera2D_zoom_to_dxf_extents(&camera2D, &dxf);
                camera3D = { 500.0, CAMERA_3D_DEFAULT_ANGLE_OF_VIEW, RAD(40), RAD(-30.0f), 0.0f, 70.0f, 0.5f, 0.0f };
            }

            show_grid = true;
            show_details = false;
            show_help = false;
        }

        real32 extrude_param_preview;
        real32 extrude_param_2_preview;
        {
            if (console_buffer_write_head == console_buffer) {
                extrude_param_preview = extrude_param;
                extrude_param_2_preview = extrude_param_2;
            } else {
                char buffs[2][64] = {};
                u32 buff_i = 0;
                u32 i = 0;
                for (char *c = console_buffer; (*c) != '\0'; ++c) {
                    if (*c == ' ') {
                        ++buff_i;
                        i = 0;
                        continue;
                    }
                    buffs[buff_i][i++] = (*c);
                }
                extrude_param_preview = strtof(buffs[0], NULL);
                extrude_param_2_preview = strtof(buffs[1], NULL);
            }
        }

        { // keyboard input keyboard shortuts
            if (globals._input_owner == COW_INPUT_OWNER_NONE) {
                if (key_pressed[COW_KEY_TAB]) {
                    camera3D.angle_of_view = (IS_ZERO(camera3D.angle_of_view)) ? CAMERA_3D_DEFAULT_ANGLE_OF_VIEW : 0.0f;
                } else if (key_pressed[COW_KEY_ESCAPE]) {
                    enter_mode = ENTER_MODE_NONE;
                    console_buffer_reset();

                    selected_reset(); // FORNOW
                } else if ((enter_mode == ENTER_MODE_LOAD) || (enter_mode == ENTER_MODE_SAVE)) {
                    if (key_pressed[COW_KEY_BACKSPACE]) {
                        if (console_buffer_write_head != console_buffer) *--console_buffer_write_head = 0;
                    } else if (key_pressed[COW_KEY_ENTER]) {
                        if (enter_mode == ENTER_MODE_LOAD) {
                            conversation_load_file(console_buffer);
                            console_buffer_reset();
                            enter_mode = _ENTER_MODE_DEFAULT;
                        } else {
                            ASSERT(enter_mode == ENTER_MODE_SAVE);
                            if (poe_suffix_match(console_buffer, ".stl")) {
                                conversation_mesh_save_stl(&conversation_mesh, console_buffer);
                            } else {
                                printf("%s filetype not supported.\n", console_buffer);
                            }
                            console_buffer_reset();
                            enter_mode = _ENTER_MODE_DEFAULT;
                        }
                    } else {
                        bool32 valid_key_pressed; {
                            valid_key_pressed = false;
                            valid_key_pressed = (valid_key_pressed || key_pressed['.']);
                            valid_key_pressed = (valid_key_pressed || key_pressed[' ']);
                            valid_key_pressed = (valid_key_pressed || key_pressed['-']);
                            for (u32 i = 0; i < 10; ++i) valid_key_pressed = (valid_key_pressed || key_pressed['0' + i]);
                            for (u32 i = 0; i < 26; ++i) valid_key_pressed = (valid_key_pressed || key_pressed['a' + i]);
                            for (u32 i = 0; i < 26; ++i) valid_key_pressed = (valid_key_pressed || key_pressed['A' + i]);
                        }
                        if (valid_key_pressed) {
                            char c = (char) globals.key_last_key_pressed;
                            if (globals.key_shift_held && key_pressed['-']) c = '_';
                            *console_buffer_write_head++ = c;
                        }
                    }
                } else if (key_pressed['Z'] && globals.key_shift_held) {
                    camera2D_zoom_to_dxf_extents(&camera2D, &dxf);
                } else if (key_pressed['S'] && globals.key_shift_held) {
                    enter_mode = ENTER_MODE_SAVE;
                } else if (key_pressed['g']) {
                    show_grid = !show_grid;
                } else if (key_pressed['h']) {
                    show_help = !show_help;
                } else if (key_pressed['i']) {
                    show_details = !show_details;
                } else if (key_pressed['s']) {
                    select_mode = SELECT_MODE_SELECT;
                    select_modifier = SELECT_MODIFIER_NONE;
                } else if (key_pressed['d']) {
                    select_mode = SELECT_MODE_DESELECT;
                    select_modifier = SELECT_MODIFIER_NONE;
                } else if (key_pressed['c']) {
                    select_modifier = SELECT_MODIFIER_CONNECTED;
                } else if (key_pressed['q']) {
                    if ((select_mode == SELECT_MODE_SELECT) || (select_mode == SELECT_MODE_DESELECT)) {
                        select_modifier = SELECT_MODIFIER_QUALITY;
                    }
                } else if (key_pressed['a']) {
                    if ((select_mode == SELECT_MODE_SELECT) || (select_mode == SELECT_MODE_DESELECT)) {
                        bool32 value_to_write_to_selection_mask = (select_mode == SELECT_MODE_SELECT);
                        for (u32 i = 0; i < dxf.num_entities; ++i) dxf_selection_mask[i] = value_to_write_to_selection_mask;
                    }
                } else if (key_pressed['x'] || key_pressed['y'] || key_pressed['z']) {
                    some_triangle_exists_that_matches_n_selected_and_r_n_selected = false;
                    r_n_selected = 0.0f;
                    if (key_pressed['x']) n_selected = { 1.0f, 0.0f, 0.0f };
                    if (key_pressed['y']) n_selected = { 0.0f, 1.0f, 0.0f };
                    if (key_pressed['z']) n_selected = { 0.0f, 0.0f, 1.0f };
                    M_selected = get_M_selected(n_selected, r_n_selected);
                } else if (key_pressed['e'] && !globals.key_shift_held) {
                    enter_mode = ENTER_MODE_EXTRUDE_ADD;
                    // console_buffer_reset();
                    extrude_param_sign_toggle = false;
                } else if (key_pressed['E'] && globals.key_shift_held) {
                    enter_mode = ENTER_MODE_EXTRUDE_SUBTRACT;
                    // console_buffer_reset();
                    extrude_param_sign_toggle = true;
                } else if (key_pressed['R'] && globals.key_shift_held) {
                    enter_mode = ENTER_MODE_REVOLVE_SUBTRACT;
                } else if (key_pressed['r']) {
                    enter_mode = ENTER_MODE_REVOLVE_ADD;
                } else if (key_pressed['L'] && globals.key_shift_held) {
                    enter_mode = ENTER_MODE_LOAD;
                } else if (key_pressed['f']) {
                    extrude_param_sign_toggle = !extrude_param_sign_toggle;
                } else if (key_pressed[COW_KEY_ENTER]
                        && (enter_mode != ENTER_MODE_NONE)
                        && (!IS_ZERO(M_selected(3, 3)))
                        && (((enter_mode != ENTER_MODE_EXTRUDE_ADD) && (enter_mode != ENTER_MODE_EXTRUDE_SUBTRACT)) || !IS_ZERO(extrude_param_preview) || !IS_ZERO(extrude_param_2_preview))
                        ) {
                    // enter_mode = ENTER_MODE_NONE;
                    // NOTE: holds over previous
                    if (console_buffer_write_head != console_buffer) {
                        extrude_param = extrude_param_preview;
                        extrude_param_2 = extrude_param_2_preview;
                        console_buffer_reset();
                    }
                    CrossSection cross_section = cross_section_create(&dxf, dxf_selection_mask);
                    // cross_section_debug_draw(&camera2D, &cross_section);


                    {
                        wrapper_manifold(
                                &manifold,
                                &conversation_mesh,
                                cross_section.num_polygonal_loops,
                                cross_section.num_vertices_in_polygonal_loops,
                                cross_section.polygonal_loops,
                                M_selected,
                                enter_mode,
                                extrude_param_sign_toggle,
                                extrude_param,
                                extrude_param_2
                                );
                        memset(dxf_selection_mask, 0, dxf.num_entities * sizeof(bool32));
                        { // some_triangle_exists_that_matches_n_selected_and_r_n_selected
                            ;

                            // FORNOW
                            // r_n_selected += ((!extrude_param_sign_toggle) ? 1.0f : -1.0f) * extrude_param;

                            // { // FORNOW
                            //     some_triangle_exists_that_matches_n_selected_and_r_n_selected = false;
                            //     n_selected = { 0.0, 1.0, 0.0 };
                            //     r_n_selected = 0.0f;
                            //     M_selected = get_M_selected(n_selected, r_n_selected);
                            // }
                            selected_reset();


                            { // FORNOW: this code heavily repeats conversation_mesh_draw
                                some_triangle_exists_that_matches_n_selected_and_r_n_selected = false;
                                for (u32 i = 0; i < conversation_mesh.num_triangles; ++i) {
                                    // FORNOW
                                    vec3 n = { conversation_mesh.triangle_normals[3 * i + 0], conversation_mesh.triangle_normals[3 * i + 1], conversation_mesh.triangle_normals[3 * i + 2] };

                                    // FORNOW
                                    vec3 p[3];
                                    for (u32 j = 0; j < 3; ++j) for (u32 d = 0; d < 3; ++d) p[j][d] = conversation_mesh.vertex_positions[3 * conversation_mesh.triangle_indices[3 * i + j] + d];
                                    real32 x_n = dot(n, p[0]);

                                    if ((dot(n, n_selected) > 0.999f) && (ABS(x_n - r_n_selected) < 0.001f)) {
                                        some_triangle_exists_that_matches_n_selected_and_r_n_selected = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                } else if (globals.key_any_key_pressed) {
                    if (key_pressed[COW_KEY_BACKSPACE]) {
                        if (console_buffer_write_head != console_buffer) *--console_buffer_write_head = 0;
                    } else if (select_modifier == SELECT_MODIFIER_QUALITY) {
                        bool32 value_to_write_to_selection_mask = (select_mode == SELECT_MODE_SELECT);
                        if (select_modifier == SELECT_MODIFIER_QUALITY) {
                            for (u32 color = 0; color < 6; ++color) {
                                if (key_pressed['0' + color]) {
                                    for (u32 i = 0; i < dxf.num_entities; ++i) {
                                        if (dxf.entities[i].line.color == color) { // FORNOW (spooky)
                                            dxf_selection_mask[i] = value_to_write_to_selection_mask;
                                        }
                                    }
                                    select_modifier = SELECT_MODE_NONE;
                                    break;
                                }
                            }
                        }
                    } else if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_SUBTRACT)){
                        bool32 valid_key_pressed; {
                            valid_key_pressed = false;
                            valid_key_pressed = (valid_key_pressed || key_pressed['.']);
                            valid_key_pressed = (valid_key_pressed || key_pressed[' ']);
                            for (u32 i = 0; i < 10; ++i) valid_key_pressed = (valid_key_pressed || key_pressed['0' + i]);
                        }
                        if (valid_key_pressed) {
                            *console_buffer_write_head++ = (char) globals.key_last_key_pressed;
                        }
                    }
                } 


                // FORNOW
                if ((select_modifier == SELECT_MODIFIER_QUALITY) && (globals.key_any_key_pressed) && !key_pressed['q']) select_modifier = SELECT_MODE_NONE;
            }
        }


        if (hot_pane == HOT_PANE_2D) {
            camera_move(&camera2D);
        } else if (hot_pane == HOT_PANE_3D) {
            camera_move(&camera3D);
        }
        mat4 PV_2D = camera_get_PV(&camera2D);
        mat4 P_3D = camera_get_P(&camera3D);
        mat4 V_3D = camera_get_V(&camera3D);
        mat4 PV_3D = P_3D * V_3D;




        { // ui
            if (globals._input_owner == COW_INPUT_OWNER_NONE) {
                { // pick 2D pick 2d pick
                    if (hot_pane == HOT_PANE_2D) {
                        dxf_pick(PV_2D, camera2D.screen_height_World, &dxf, dxf_selection_mask, select_mode, select_modifier, pick.num_entities_in_loops, pick.loops, pick.loop_index_from_entity_index);
                    }
                }

                { // pick 3D pick 3d pick (TODO: allow clicking on grid)
                    if (hot_pane == HOT_PANE_3D) {
                        if (globals.mouse_left_pressed) {
                            int32 selected_triangle_index = -1;
                            {
                                vec3 a = transformPoint(inverse(PV_3D), V3(globals.mouse_position_NDC, -1.0f));
                                vec3 b = transformPoint(inverse(PV_3D), V3(globals.mouse_position_NDC,  1.0f));
                                vec3 o = a;
                                vec3 minus_dir = normalized(a - b);
                                vec3 p[3];
                                real32 min_distance = HUGE_VAL;
                                for (u32 i = 0; i < conversation_mesh.num_triangles; ++i) {
                                    for (u32 j = 0; j < 3; ++j) for (u32 d = 0; d < 3; ++d) p[j][d] = conversation_mesh.vertex_positions[3 * conversation_mesh.triangle_indices[3 * i + j] + d];
                                    vec4 w_t = inverse(hstack(V4(p[0], 1.0f), V4(p[1], 1.0f), V4(p[2], 1.0f), V4(minus_dir, 0.0f))) * V4(o, 1.0f);
                                    if ((w_t.x > 0) && (w_t.y > 0) && (w_t.z > 0)
                                            // && (w_t.w > 0)
                                       ) {
                                        real32 distance = w_t[3];
                                        if (distance < min_distance) {
                                            min_distance = distance;
                                            selected_triangle_index = i; // FORNOW
                                        }
                                    }
                                }
                            }

                            if (selected_triangle_index != -1) {
                                some_triangle_exists_that_matches_n_selected_and_r_n_selected = true;
                                { // FORNOW (gross) calculateion of n_selected, r_n_selected
                                    n_selected = { conversation_mesh.triangle_normals[3 * selected_triangle_index + 0], conversation_mesh.triangle_normals[3 * selected_triangle_index + 1], conversation_mesh.triangle_normals[3 * selected_triangle_index + 2] };
                                    vec3 p_selected[3];
                                    for (u32 j = 0; j < 3; ++j) for (u32 d = 0; d < 3; ++d) p_selected[j][d] = conversation_mesh.vertex_positions[3 * conversation_mesh.triangle_indices[3 * selected_triangle_index + j] + d];
                                    r_n_selected = dot(n_selected, p_selected[0]);
                                }
                                M_selected = get_M_selected(n_selected, r_n_selected);
                            }

                        }
                    }
                    // if (key_pressed[COW_KEY_ESCAPE]) {
                    //     r_n_selected = 0.0f;
                    //     n_selected = {};
                    //     M_selected = {};
                    // }
                }
            }
        }

        { // draw
            u32 window_width, window_height; {
                real32 _window_width, _window_height; // FORNOW
                _window_get_size(&_window_width, &_window_height);
                window_width = (u32) _window_width;
                window_height = (u32) _window_height;
            }

            { // panes
                { // draw
                    glDisable(GL_DEPTH_TEST);
                    eso_begin(globals.Identity, SOUP_QUADS);
                    eso_color(0.3f, 0.3f, 0.3f);
                    eso_vertex(0.0f,  1.0f);
                    eso_vertex(0.0f, -1.0f);
                    eso_vertex(1.0f, -1.0f);
                    eso_vertex(1.0f,  1.0f);
                    eso_end();
                    glEnable(GL_DEPTH_TEST);

                    eso_begin(globals.Identity, SOUP_LINES, 5.0f, true);
                    eso_color(0.0f, 1.0f, 1.0f);
                    eso_vertex(0.0f,  1.0f);
                    eso_vertex(0.0f, -1.0f);
                    eso_end();
                }

                if ((!globals.mouse_left_held && !globals.mouse_right_held) || globals.mouse_left_pressed || globals.mouse_right_pressed) {
                    hot_pane = (globals.mouse_position_NDC.x <= 0.0f) ? HOT_PANE_2D : HOT_PANE_3D;
                }
            }

            { // draw 2D draw 2d draw
                glEnable(GL_SCISSOR_TEST);
                glScissor(0, 0, window_width / 2, window_height);
                {
                    if (show_grid) { // grid 2D grid 2d grid
                        eso_begin(PV_2D, SOUP_LINES, 2.0f);
                        eso_color(0.2f, 0.2f, 0.2f);
                        for (u32 i = 0; i <= 30; ++i) {
                            real32 tmp = i * 10.0f;
                            eso_vertex(tmp,   0.0f);
                            eso_vertex(tmp, 300.0f);
                            eso_vertex(  0.0f, tmp);
                            eso_vertex(300.0f, tmp);
                        }
                        eso_end();
                    }
                    { // entities
                        eso_begin(PV_2D, SOUP_LINES, 3.0f);
                        for (u32 i = 0; i < dxf.num_entities; ++i) {
                            DXFEntity *entity = &dxf.entities[i];
                            int32 color = (dxf_selection_mask[i]) ? DXF_COLOR_SELECTION : DXF_COLOR_DONT_OVERRIDE;
                            eso_dxf_entity__SOUP_LINES(entity, color);
                        }
                        eso_end();
                    }
                    { // dots
                        if (show_details) {
                            eso_begin(camera_get_PV(&camera2D), SOUP_POINTS, 4.0);
                            eso_color(monokai.white);
                            for (DXFEntity *entity = dxf.entities; entity < &dxf.entities[dxf.num_entities]; ++entity) {
                                real32 start_x, start_y, end_x, end_y;
                                entity_get_start_and_end_points(entity, &start_x, &start_y, &end_x, &end_y);
                                eso_vertex(start_x, start_y);
                                eso_vertex(end_x, end_y);
                            }
                            eso_end();
                        }
                    }
                    { // axes 2D axes 2d axes axis 2D axis 2d axes
                        real32 r = camera2D.screen_height_World / 70.0f;
                        eso_begin(PV_2D, SOUP_LINES, 4.0f);
                        eso_color(1.0f, 1.0f, 1.0f);
                        eso_vertex(-r, 0.0f);
                        eso_vertex( r, 0.0f);
                        eso_vertex(0.0f, -r);
                        eso_vertex(0.0f,  r);
                        eso_end();
                    }
                }
                glDisable(GL_SCISSOR_TEST);
            }

            { // 3D draw 3D 3d draw 3d
                bool32 dxf_anything_selected; {
                    dxf_anything_selected = false;
                    for (u32 i = 0; i < dxf.num_entities; ++i) {
                        if (dxf_selection_mask[i]) {
                            dxf_anything_selected = true;
                            break;
                        }
                    }
                }

                glEnable(GL_SCISSOR_TEST);
                glScissor(window_width / 2, 0, window_width / 2, window_height);

                { // 2D selection (FORNOW: ew)
                    u32 color = ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_ADD)) ? DXF_COLOR_TRAVERSE : DXF_COLOR_QUALITY_1;

                    u32 NUM_TUBE_STACKS_INCLUSIVE;
                    mat4 M;
                    mat4 M_incr;
                    if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_SUBTRACT) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_SUBTRACT)) {
                        if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_SUBTRACT)) {
                            real32 sign = (!extrude_param_sign_toggle) ? 1.0f : -1.0f;
                            real32 a = sign * -extrude_param_2_preview;
                            real32 b = sign * extrude_param_preview;
                            real32 total_height = (extrude_param_2_preview + extrude_param_preview);
                            NUM_TUBE_STACKS_INCLUSIVE = MIN(64, u32(roundf(total_height / 2.5f)) + 2);
                            M = M_selected * M4_Translation(0.0f, 0.0f, a + Z_FIGHT_EPS);
                            M_incr = M4_Translation(0.0f, 0.0f, (b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                        } else {
                            ASSERT((enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_SUBTRACT));
                            NUM_TUBE_STACKS_INCLUSIVE = NUM_SEGMENTS_PER_CIRCLE;
                            M = M_selected;
                            real32 a = 0.0f;
                            real32 b = TAU;
                            M_incr = M4_RotationAboutYAxis((b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                        }
                        for (u32 tube_stack_index = 0; tube_stack_index < NUM_TUBE_STACKS_INCLUSIVE; ++tube_stack_index) {
                            {
                                eso_begin(PV_3D * M, SOUP_LINES, 5.0f);
                                for (u32 i = 0; i < dxf.num_entities; ++i) {
                                    DXFEntity *entity = &dxf.entities[i];
                                    if (dxf_selection_mask[i]) eso_dxf_entity__SOUP_LINES(entity, color);
                                }
                                eso_end();
                            }
                            M *= M_incr;
                        }
                    }
                }


                { // arrow
                    {
                        if (dxf_anything_selected) {
                            real32 H[2] = { extrude_param_preview, extrude_param_2_preview };
                            bool32 toggle[2] = { extrude_param_sign_toggle, !extrude_param_sign_toggle };
                            mat4 R2 = M4_Identity();
                            if ((enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_SUBTRACT)) {
                                H[0] = 50.0f;
                                H[1] = 0.0f;
                                R2 = M4_RotationAboutXAxis(RAD(-90.0f));
                            }
                            mat4 R = M4_RotationAboutXAxis(RAD(90.0f));
                            vec3 color = ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_ADD)) ? monokai.green : monokai.red;
                            for (u32 i = 0; i < 2; ++i) {
                                if (!IS_ZERO(H[i])) {
                                    real32 total_height = ABS(H[i]);
                                    real32 cap_height = (total_height > 10.0f) ? 5.0f : (0.5f * total_height);
                                    real32 shaft_height = total_height - cap_height;
                                    real32 s = 1.5f;
                                    mat4 N = M4_Translation(0.0, 0.0, -Z_FIGHT_EPS);
                                    if (toggle[i]) N = M4_Scaling(1.0f, 1.0f, -1.0f) * N;
                                    mat4 M_cyl  = M_selected * R2 * N * M4_Scaling(s * 1.0f, s * 1.0f, shaft_height) * R;
                                    mat4 M_cone = M_selected * R2 * N * M4_Translation(0.0f, 0.0f, shaft_height) * M4_Scaling(s * 2.0f, s * 2.0f, cap_height) * R;
                                    library.meshes.cylinder.draw(P_3D, V_3D, M_cyl, color);
                                    library.meshes.cone.draw(P_3D, V_3D, M_cone, color);
                                }
                            }
                        }
                    }
                }

                { // axes 3D axes 3d axes axis 3D axis 3d axis
                    real32 r = camera3D.ortho_screen_height_World / 30.0f;
                    eso_begin(PV_3D, SOUP_LINES, 4.0f);
                    eso_color(1.0f, 0.0f, 0.0f);
                    eso_vertex(0.0f, 0.0f, 0.0f);
                    eso_vertex(  r, 0.0f, 0.0f);
                    eso_color(0.0f, 1.0f, 0.0f);
                    eso_vertex(0.0f, 0.0f, 0.0f);
                    eso_vertex(0.0f,    r, 0.0f);
                    eso_color(0.0f, 0.0f, 1.0f);
                    eso_vertex(0.0f, 0.0f, 0.0f);
                    eso_vertex(0.0f, 0.0f,    r);
                    eso_end();
                }

                if (show_grid) { // grid 3D grid 3d grid
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_FRONT);
                    real32 r = 256.0f;
                    grid_box.draw(P_3D, V_3D, M4_Translation(0.0f, r / 2 - 2 * Z_FIGHT_EPS, 0.0f) * M4_Scaling(r / 2), {}, "procedural grid");
                    glDisable(GL_CULL_FACE);
                }

                { // conversation_mesh; NOTE: includes transparency
                    if (conversation_mesh.cosmetic_edges) {
                        eso_begin(PV_3D, SOUP_LINES, 3.0f); 
                        eso_color(monokai.black);
                        // 3 * num_triangles * 2 / 2
                        for (u32 k = 0; k < 2 * conversation_mesh.num_cosmetic_edges; ++k) eso_vertex(conversation_mesh.vertex_positions, conversation_mesh.cosmetic_edges[k]);
                        eso_end();
                    }
                    for (u32 pass = 0; pass <= 1; ++pass) {
                        eso_begin(PV_3D, (!show_details) ? SOUP_TRIANGLES : SOUP_OUTLINED_TRIANGLES);
                        for (u32 i = 0; i < conversation_mesh.num_triangles; ++i) {
                            // FORNOW (all this crap)
                            // TODO 

                            vec3 n;
                            for (u32 d = 0; d < 3; ++d) n[d] = conversation_mesh.triangle_normals[3 * i + d];

                            vec3 p[3];
                            real32 x_n;
                            {
                                for (u32 j = 0; j < 3; ++j) for (u32 d = 0; d < 3; ++d) p[j][d] = conversation_mesh.vertex_positions[3 * conversation_mesh.triangle_indices[3 * i + j] + d];
                                x_n = dot(n, p[0]);
                            }

                            vec3 color; 
                            real32 alpha;
                            {
                                vec3 n_camera = transformNormal(V_3D, n);
                                // if (n_camera.z > 0)
                                {
                                    if (some_triangle_exists_that_matches_n_selected_and_r_n_selected && (dot(n, n_selected) > 0.999f) && (ABS(x_n - r_n_selected) < 0.001f)) {
                                        if (pass == 0) continue;
                                        color = LERP(0.9f, V3(0.5f + 0.5f * n_camera.x, 0.5f + 0.5f * n_camera.y, 1.0f), monokai.yellow);
                                        alpha = ((enter_mode == ENTER_MODE_EXTRUDE_ADD || (enter_mode == ENTER_MODE_EXTRUDE_SUBTRACT)) && ((extrude_param_sign_toggle) || (extrude_param_2_preview != 0.0f))) ? 0.7f : 1.0f;
                                    } else {
                                        // if (n_camera.z < 0.0f) n_camera *= -1; // FORNOW
                                        if (pass == 1) continue;
                                        color = V3(0.5f + 0.5f * n_camera.x, 0.5f + 0.5f * n_camera.y, 1.0f);
                                        alpha = 1.0f;
                                    }
                                }
                                // else {
                                //     color = V3(1.0f, 0.0f, 0.0f);
                                //     alpha = 1.0f;
                                // }
                            }
                            eso_color(color, alpha);
                            eso_vertex(p[0]);
                            eso_vertex(p[1]);
                            eso_vertex(p[2]);

                        }
                        eso_end();
                    }
                }


                { // plane; NOTE: transparent
                    if (!some_triangle_exists_that_matches_n_selected_and_r_n_selected) { // planes
                        real32 r = 256.0f / 2;
                        eso_begin(PV_3D * M_selected, SOUP_OUTLINED_QUADS);
                        // eso_color(V3(0.5f) + 0.5 * n_selected, 0.5f);
                        eso_color(monokai.yellow, 0.5f);
                        eso_vertex( r,  r, -Z_FIGHT_EPS);
                        eso_vertex( r, -r, -Z_FIGHT_EPS);
                        eso_vertex(-r, -r, -Z_FIGHT_EPS);
                        eso_vertex(-r,  r, -Z_FIGHT_EPS);
                        eso_end();
                    }
                }

                glDisable(GL_SCISSOR_TEST);
            }

            { // gui
                gui_printf("[Mouse] %s %s", (select_mode == SELECT_MODE_NONE) ? "" : (select_mode == SELECT_MODE_SELECT) ? "SELECT" : "DESELCT", (select_modifier == SELECT_MODE_NONE) ? "" : (select_modifier == SELECT_MODIFIER_CONNECTED) ?  "CONNECTED" : "");


                char extrude_message[256] = {};
                if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_SUBTRACT)) {
                    bool32 show_old_command = (console_buffer_write_head == console_buffer);
                    real32 p = (show_old_command) ? extrude_param : extrude_param_preview;
                    real32 p2 = (show_old_command) ? extrude_param_2 : extrude_param_2_preview;
                    char sign  = (!extrude_param_sign_toggle) ? '^' : 'v';
                    char sign2 =  (extrude_param_sign_toggle) ? '^' : 'v';
                    if (p2 == 0) {
                        sprintf(extrude_message, "%gmm%c", p, sign);
                    } else {
                        sprintf(extrude_message, "%gmm%c %gmm%c", p, sign, p2, sign2);
                    }
                }

                gui_printf("[Enter] %s %s",
                        (enter_mode == ENTER_MODE_EXTRUDE_ADD) ? "EXTRUDE ADD" :
                        (enter_mode == ENTER_MODE_EXTRUDE_SUBTRACT) ? "EXTRUDE SUBTRACT" :
                        (enter_mode == ENTER_MODE_REVOLVE_ADD) ? "REVOLVE ADD" :
                        (enter_mode == ENTER_MODE_REVOLVE_SUBTRACT) ? "REVOLVE SUBTRACT" :
                        (enter_mode == ENTER_MODE_LOAD) ? "LOAD" :
                        (enter_mode == ENTER_MODE_SAVE) ? "SAVE" :
                        "",
                        ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_SUBTRACT)) ? extrude_message : "");
                if ((enter_mode == ENTER_MODE_NONE) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_SUBTRACT)) {
                    gui_printf("> %s", console_buffer);
                } else {
                    gui_printf("`> %s", console_buffer);
                }

                {
                    if (strlen(conversation_drop_path) == 0) {
                        gui_printf("drag and drop dxf...");
                    }
                    else {
                        gui_printf("`%s", conversation_drop_path);
                    }
                }


                if (show_details) {
                    gui_printf("");
                    gui_printf("%d dxf elements", dxf.num_entities);
                    gui_printf("%d stl triangles", conversation_mesh.num_triangles);
                }
                {
                    gui_printf("");
                    gui_printf("(h)elp");
                    if (show_help) {
                        gui_printf("(s)elect (d)eselect (c)onnected + (a)ll (q)uality + (0-5)");
                        gui_printf("(y)-plane (z)-plane (x)-plane");
                        gui_printf("(e)trude-add (E)xtrude-cut + (0-9. ) (f)lip-direction");
                        gui_printf("(r)evolve-add (R)evolve-cut");
                        gui_printf("(L)oad (S)ave // must include file extension");
                        gui_printf("(g)rid (i)nspect");
                        gui_printf("(Z)oom-to-extents");
                        gui_printf("(Tab)-orthographic-perspective-view");
                    }
                }


                // gui_printf("---");
                // if (gui_button("reset")) reset = true;
            }
        }
    }
}
