// cow soup/eso should take arbitrary sizes per entity and 

// // Conversation
// NOTE: This is a little CAD program Jim is actively developing.
//       It takes in an OMAX DXF and let's you rapidly create a 3D-printable STL using Manifold.
//       Feel free to ignore.

// TODO: cow_real actually supporting real32 or real64



// roadmap
// / import dxf
// / detect pick_loops
// / 2D picking of pick_loops by clicking on DXF
// / omax-type dxf_selection_mask ('s' + 'c'); ('d' + 'a')
// / multi-select (no shift--follow omax style)
// / turn loop into polygon (rasterize arcs into line segments)
// - cross-section arc discretization quality knobs (the correct knob is ??)
// - compile/link manifold
// - boss ('b') and cut ('c') -- (or 'e'/'E'?)
// - nudging with x, y
// - planar surface picking (and if no 3D geometry yet, your choice of the xy yz and zx planes)
// - cut has depth
// ? you can persist the 2D curves alongside the 3D geometry they generated and then use omax style picking on the 3D part
// ? how are we storing the sequence of operations
// NOTE: for a first cut it could be destructive like omax itself
//       basically just like a conversational mill
//       don't store anything but the STL created so far (and maybe an undo stack eventually)
//       could at least print to the terminal what you've done
//       should preview the cut on the surface before you accept it (eventually) -- and let you nudge it in x, y, and z

#include "cs345.cpp"
#include "poe.cpp"


real32 EPSILON_DEFAULT = 2e-3f;
real32 TOLERANCE_DEFAULT = 1e-5f;
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

void dxf_free(DXF *dxf) {
    ASSERT(dxf->entities);
    free(dxf->entities);
}

DXF dxf_load(char *filename) {
    #if 0
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
    #else
    DXF result = {};

    StretchyBuffer<DXFEntity> stretchy_buffer = {}; {
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
            static char buffer[512];
            while (fgets(buffer, _COUNT_OF(buffer), file)) {
                if (mode == DXF_LOAD_MODE_NONE) {
                    if (poe_matches_prefix(buffer, "LINE")) {
                        mode = DXF_LOAD_MODE_LINE;
                        code_is_hot = false;
                        line = {};
                    } else if (poe_matches_prefix(buffer, "ARC")) {
                        mode = DXF_LOAD_MODE_ARC;
                        code_is_hot = false;
                        arc = {};
                    }
                } else {
                    if (!code_is_hot) {
                        sscanf(buffer, "%d", &code);
                        if (code == 0) {
                            if (mode == DXF_LOAD_MODE_LINE) {
                                sbuff_push_back(&stretchy_buffer, { DXF_ENTITY_TYPE_LINE, line.color, line.start_x, line.start_y, line.end_x, line.end_y });
                            } else {
                                ASSERT(mode == DXF_LOAD_MODE_ARC);
                                sbuff_push_back(&stretchy_buffer, { DXF_ENTITY_TYPE_ARC, arc.color, arc.center_x, arc.center_y, arc.radius, arc.start_angle_in_degrees, arc.end_angle_in_degrees });
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
                                    line.start_x = value;
                                } else if (code == 20) {
                                    line.start_y = value;
                                } else if (code == 11) {
                                    line.end_x = value;
                                } else if (code == 21) {
                                    line.end_y = value;
                                }
                            } else {
                                ASSERT(mode == DXF_LOAD_MODE_ARC);
                                if (code == 10) {
                                    arc.center_x = value;
                                } else if (code == 20) {
                                    arc.center_y = value;
                                } else if (code == 40) {
                                    arc.radius = value;
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

    result.num_entities = stretchy_buffer.length;
    result.entities = (DXFEntity *) calloc(result.num_entities, sizeof(DXFEntity));
    memcpy(result.entities, stretchy_buffer.data, result.num_entities * sizeof(DXFEntity));
    sbuff_free(&stretchy_buffer);
    return result;
    #endif
}

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

void eso_dxf_entity(DXFEntity *entity, int32 override_color = DXF_COLOR_DONT_OVERRIDE) {
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
    for (DXFEntity *entity = dxf->entities; entity < dxf->entities + dxf->num_entities; ++entity) {
        eso_dxf_entity(entity, override_color);
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
    for (DXFEntity *entity = dxf->entities; entity < dxf->entities + dxf->num_entities; ++entity) {
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
        // populate StretchyBuffer's
        StretchyBuffer<StretchyBuffer<DXFEntityIndexAndFlipFlag>> stretchy_buffer = {}; {
            bool32 *entity_already_added = (bool32 *) calloc(dxf->num_entities, sizeof(bool32));
            while (true) {
                #define MACRO_CANDIDATE_VALID(i) (!entity_already_added[i] && (!dxf_selection_mask || dxf_selection_mask[i]))
                { // seed loop
                    bool32 added_and_seeded_new_loop = false;
                    for (u32 entity_index = 0; entity_index < dxf->num_entities; ++entity_index) {
                        if (MACRO_CANDIDATE_VALID(entity_index)) {
                            added_and_seeded_new_loop = true;
                            entity_already_added[entity_index] = true;
                            sbuff_push_back(&stretchy_buffer, {});
                            sbuff_push_back(&stretchy_buffer.data[stretchy_buffer.length - 1], { entity_index, false });
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
                            DXFEntityIndexAndFlipFlag *prev_entity_index_and_flip_flag = &(stretchy_buffer.data[stretchy_buffer.length - 1].data[stretchy_buffer.data[stretchy_buffer.length - 1].length - 1]);
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
                                sbuff_push_back(&stretchy_buffer.data[stretchy_buffer.length - 1], { entity_index, flip_flag });
                                break;
                            }
                        }
                        if (!added_new_entity_to_loop) break;
                    }
                }

                { // reverse_loop if necessary
                    u32 num_entities_in_loop = stretchy_buffer.data[stretchy_buffer.length - 1].length;
                    DXFEntityIndexAndFlipFlag *loop = stretchy_buffer.data[stretchy_buffer.length - 1].data;
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

        // copy over from StretchyBuffer's
        result.num_loops = stretchy_buffer.length;
        result.num_entities_in_loops = (u32 *) calloc(result.num_loops, sizeof(u32));
        result.loops = (DXFEntityIndexAndFlipFlag **) calloc(result.num_loops, sizeof(DXFEntityIndexAndFlipFlag *));
        for (u32 i = 0; i < result.num_loops; ++i) {
            result.num_entities_in_loops[i] = stretchy_buffer.data[i].length;
            result.loops[i] = (DXFEntityIndexAndFlipFlag *) calloc(result.num_entities_in_loops[i], sizeof(DXFEntityIndexAndFlipFlag));
            memcpy(result.loops[i], stretchy_buffer.data[i].data, result.num_entities_in_loops[i] * sizeof(DXFEntityIndexAndFlipFlag));
        }

        // free StretchyBuffer's
        for (int i = 0; i < stretchy_buffer.length; ++i) sbuff_free(&stretchy_buffer.data[i]);
        sbuff_free(&stretchy_buffer);
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


#define TOOL_NONE 0
#define TOOL_SELECT 1
#define TOOL_DESELECT 2
void dxf_pick(Camera2D *camera2D, DXF *dxf, bool32 *dxf_selection_mask, bool deselect, bool tool_connected_modifier, u32 *num_entities_in_pick_loops, DXFEntityIndexAndFlipFlag **pick_loops, u32 *pick_loop_index_from_entity_index, real32 epsilon = EPSILON_DEFAULT) {
    if (!globals.mouse_left_held) return;

    real32 PV[16];
    _camera_get_PV(camera2D, PV);
    real32 x, y;
    _input_get_mouse_position_and_change_in_position_in_world_coordinates(PV, &x, &y, NULL, NULL);

    int hot_entity_index = -1;
    double hot_squared_distance = HUGE_VAL;
    for (u32 i = 0; i < dxf->num_entities; ++i) {
        DXFEntity *entity = &dxf->entities[i];
        double squared_distance = squared_distance_point_entity(x, y, entity);
        if (squared_distance < MIN(epsilon, hot_squared_distance)) {
            hot_squared_distance = squared_distance;
            hot_entity_index = i;
        }
    }

    if (hot_entity_index != -1) {
        if (globals.mouse_left_held) {
            bool32 value = (!deselect);
            if (!tool_connected_modifier) {
                dxf_selection_mask[hot_entity_index] = value;
            } else {
                u32 loop_index = pick_loop_index_from_entity_index[hot_entity_index];
                DXFEntityIndexAndFlipFlag *loop = pick_loops[loop_index];
                u32 num_entities = num_entities_in_pick_loops[loop_index];
                for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities; ++entity_index_and_flip_flag) {
                    dxf_selection_mask[entity_index_and_flip_flag->entity_index] = value;
                }
            }
        }
    }
}


struct Vertex2D {
    real32 x;
    real32 y;
};

// NOTE: even odd
struct CrossSection {
    u32 num_polygonal_loops;
    u32 *num_vertices_in_polygonal_loops;
    Vertex2D **polygonal_loops;
};

CrossSection cross_section_create(DXF *dxf, bool32 *dxf_selection_mask) {
    #if 0
    _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(dxf);
    _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(dxf_selection_mask);
    CrossSection result = {};
    result.num_polygonal_loops = 2;
    result.num_vertices_in_polygonal_loops = (u32 *) calloc(result.num_polygonal_loops, sizeof(u32));
    result.num_vertices_in_polygonal_loops[0] = 4;
    result.num_vertices_in_polygonal_loops[1] = 6;
    result.polygonal_loops = (Vertex2D **) calloc(result.num_polygonal_loops, sizeof(Vertex2D *));
    result.polygonal_loops[0] = (Vertex2D *) calloc(result.num_vertices_in_polygonal_loops[0], sizeof(Vertex2D));
    result.polygonal_loops[1] = (Vertex2D *) calloc(result.num_vertices_in_polygonal_loops[1], sizeof(Vertex2D));
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
    #else


    // populate StretchyBuffer's
    StretchyBuffer<StretchyBuffer<Vertex2D>> stretchy_buffer = {}; {
        DXFLoopAnalysisResult analysis = dxf_loop_analysis_create(dxf, dxf_selection_mask);
        for (u32 loop_index = 0; loop_index < analysis.num_loops; ++loop_index) {
            u32 num_entities_in_loop = analysis.num_entities_in_loops[loop_index];
            DXFEntityIndexAndFlipFlag *loop = analysis.loops[loop_index];
            sbuff_push_back(&stretchy_buffer, {});
            for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities_in_loop; ++entity_index_and_flip_flag) {
                u32 entity_index = entity_index_and_flip_flag->entity_index;
                bool32 flip_flag = entity_index_and_flip_flag->flip_flag;
                DXFEntity *entity = &dxf->entities[entity_index];
                if (entity->type == DXF_ENTITY_TYPE_LINE) {
                    DXFLine *line = &entity->line;
                    if (!flip_flag) {
                        sbuff_push_back(&stretchy_buffer.data[stretchy_buffer.length - 1], { line->start_x, line->start_y });
                    } else {
                        sbuff_push_back(&stretchy_buffer.data[stretchy_buffer.length - 1], { line->end_x, line->end_y });
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
                        sbuff_push_back(&stretchy_buffer.data[stretchy_buffer.length - 1], { x, y });
                        current_angle += increment;
                    }
                }
            }
        }
        dxf_loop_analysis_free(&analysis);
    }

    // copy over from StretchyBuffer's
    CrossSection result = {};
    result.num_polygonal_loops = stretchy_buffer.length;
    result.num_vertices_in_polygonal_loops = (u32 *) calloc(result.num_polygonal_loops, sizeof(u32));
    result.polygonal_loops = (Vertex2D **) calloc(result.num_polygonal_loops, sizeof(Vertex2D *));
    for (u32 i = 0; i < result.num_polygonal_loops; ++i) {
        result.num_vertices_in_polygonal_loops[i] = stretchy_buffer.data[i].length;
        result.polygonal_loops[i] = (Vertex2D *) calloc(result.num_vertices_in_polygonal_loops[i], sizeof(Vertex2D));
        memcpy(result.polygonal_loops[i], stretchy_buffer.data[i].data, result.num_vertices_in_polygonal_loops[i] * sizeof(Vertex2D));
    }

    // free StretchyBuffer's
    for (int i = 0; i < stretchy_buffer.length; ++i) sbuff_free(&stretchy_buffer.data[i]);
    sbuff_free(&stretchy_buffer);

    return result;
    #endif
}

void cross_section_debug_draw(Camera2D *camera2D, CrossSection *cross_section) {
    eso_begin(camera_get_PV(camera2D), SOUP_LINES);
    eso_color(monokai.white);
    for (u32 loop_index = 0; loop_index < cross_section->num_polygonal_loops; ++loop_index) {
        Vertex2D *polygonal_loop = cross_section->polygonal_loops[loop_index];
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





struct STLTriangle {
    real32 v1_x;
    real32 v1_y;
    real32 v1_z;
    real32 v2_x;
    real32 v2_y;
    real32 v2_z;
    real32 v3_x;
    real32 v3_y;
    real32 v3_z;
};

struct STL {
    u32 num_triangles;
    STLTriangle *triangles;
};

void stl_draw(Camera3D *camera, STL *stl) {
    mat4 C_inv = camera_get_V(camera);
    eso_begin(camera_get_PV(camera), SOUP_OUTLINED_TRIANGLES);
    for (STLTriangle *triangle = stl->triangles; triangle < stl->triangles + stl->num_triangles; ++triangle) {
        vec3 v1 = { triangle->v1_x, triangle->v1_y, triangle->v1_z };
        vec3 v2 = { triangle->v2_x, triangle->v2_y, triangle->v2_z };
        vec3 v3 = { triangle->v3_x, triangle->v3_y, triangle->v3_z };
        vec3 n = transformNormal(C_inv, normalized(cross(v2 - v1, v3 - v1)));
        vec3 color = (n.z < 0.0f) ? V3(1.0f, 0.0f, 0.0f) : V3(0.5f + 0.5f * n.x, 0.5f + 0.5f * n.y, 1.0f);
        eso_color(color);
        eso_vertex(v1);
        eso_vertex(v2);
        eso_vertex(v3);
    }
    eso_end();
}

void stl_save_binary(STL *stl, char *filename) {
    FILE *file = fopen(filename, "w");
    ASSERT(file);

    int num_bytes = 80 + 4 + 50 * stl->num_triangles;
    char *buffer = (char *) calloc(num_bytes, 1); {
        int offset = 80;
        memcpy(buffer + offset, &stl->num_triangles, 4);
        offset += 4;
        for (STLTriangle *triangle = stl->triangles; triangle < stl->triangles + stl->num_triangles; ++triangle) {
            offset += 12;
            memcpy(buffer + offset, triangle, 36);
            offset += 38;
        }
    }
    fwrite(buffer, 1, num_bytes, file);
    free(buffer);

    fclose(file);
}

void stl_extrude(STL *stl, bool32 cut, DXF *dxf, bool32 *dxf_selection_mask, STLTriangle *stl_selected_triangle, real32 height) {

}

real32 conversation_get_real32(char *string) {
    return 10.0;
}


int main() {
    DXF dxf = dxf_load("omax.dxf");

    DXFLoopAnalysisResult pick = dxf_loop_analysis_create(&dxf);

    u32 tool = TOOL_NONE;
    bool32 tool_connected_modifier = false;
    bool32 *dxf_selection_mask = (bool32 *) calloc(dxf.num_entities, sizeof(bool32));



    STLTriangle *stl_selected_triangle = NULL;
    STL stl;
    #if 1
    stl = {};
    stl.num_triangles = 4;
    stl.triangles = (STLTriangle *) calloc(stl.num_triangles, sizeof(STLTriangle));
    float h = (1.0f + SQRT(3.0f)) / 2;
    stl.triangles[0] = {
        COS(RAD(  0.0)), 0.0f, SIN(RAD(  0.0)),
        COS(RAD(120.0)), 0.0f, SIN(RAD(120.0)),
        COS(RAD(240.0)), 0.0f, SIN(RAD(240.0)),
    };
    stl.triangles[1] = {
        COS(RAD(120.0)), 0.0f, SIN(RAD(120.0)),
        COS(RAD(  0.0)), 0.0f, SIN(RAD(  0.0)),
        0.0f, h, 0.0f
    };
    stl.triangles[2] = {
        COS(RAD(240.0)), 0.0f, SIN(RAD(240.0)),
        COS(RAD(120.0)), 0.0f, SIN(RAD(120.0)),
        0.0f, h, 0.0f
    };
    stl.triangles[3] = {
        COS(RAD(  0.0)), 0.0f, SIN(RAD(  0.0)),
        COS(RAD(240.0)), 0.0f, SIN(RAD(240.0)),
        0.0f, h, 0.0f
    };
    #else
    #endif



    CrossSection cross_section = {};


    // TODO: simple split screen with simple scissoring (move this into cow)


    Camera2D camera2D = { 5.0, 2.5 };
    Camera3D camera3D = { 5.0, RAD(0.0), RAD(0.0), RAD(0.0), -2.5 };
    while (cow_begin_frame()) {
        camera_move(&camera3D, true, true);
        camera_move(&camera2D);
        { // dxf
            { // pick
                if (globals.key_pressed['s']) {
                    tool = TOOL_SELECT;
                    tool_connected_modifier = false;
                }
                if (globals.key_pressed['d']) {
                    tool = TOOL_DESELECT;
                    tool_connected_modifier = false;
                }
                if (globals.key_pressed['c']) {
                    tool_connected_modifier = true;
                }
                if (globals.key_pressed['a']) {
                    if (tool == TOOL_SELECT) {
                        for (u32 i = 0; i < dxf.num_entities; ++i) dxf_selection_mask[i] = true;
                    } else if (tool == TOOL_DESELECT) {
                        for (u32 i = 0; i < dxf.num_entities; ++i) dxf_selection_mask[i] = false;
                    }
                }
                if (tool == TOOL_SELECT || tool == TOOL_DESELECT) {
                    dxf_pick(&camera2D, &dxf, dxf_selection_mask, tool == TOOL_DESELECT, tool_connected_modifier, pick.num_entities_in_loops, pick.loops, pick.loop_index_from_entity_index);
                }
            }
            { // draw
                if (!globals.key_toggled['h']) {
                    eso_begin(camera_get_PV(&camera2D), SOUP_LINES);
                    for (u32 i = 0; i < dxf.num_entities; ++i) {
                        DXFEntity *entity = &dxf.entities[i];
                        int32 color = (dxf_selection_mask[i]) ? DXF_COLOR_SELECTION : DXF_COLOR_DONT_OVERRIDE;
                        eso_dxf_entity(entity, color);
                    }
                    eso_end();

                    // dots
                    if (globals.key_toggled[COW_KEY_TAB]) {
                        eso_begin(camera_get_PV(&camera2D), SOUP_POINTS, 4.0);
                        eso_color(monokai.white);
                        for (DXFEntity *entity = dxf.entities; entity < dxf.entities + dxf.num_entities; ++entity) {
                            real32 start_x, start_y, end_x, end_y;
                            entity_get_start_and_end_points(entity, &start_x, &start_y, &end_x, &end_y);
                            eso_vertex(start_x, start_y);
                            eso_vertex(end_x, end_y);
                        }
                        eso_end();
                    }
                }
            }
        }
        { // cross_section
            if (globals.key_pressed[COW_KEY_ENTER]) {
                cross_section = cross_section_create(&dxf, dxf_selection_mask);
            }
            cross_section_debug_draw(&camera2D, &cross_section);
        }
        { // stl
            {
                stl_draw(&camera3D, &stl);
            }

            // TODO: we need an actual example(s)

            // // TODO
            // #define EXTRUDE_META_DATA_TYPE_MAJOR_PLANE 0
            // #define EXTRUDE_META_DATA_TYPE_TRIANGLE 1
            // struct ExtrudeMetaData {
            //     bool32 type;
            //     real32 x_offset;
            //     real32 y_offset;
            // }

            if (globals.key_pressed['b'] || globals.key_pressed['c']) {
                // TODO begin operation (can use gui elements to get values from user)
                // then press of enter to finish operation
                // tabbing through the elements would be clutch, but not needed at first
                real32 height = conversation_get_real32("height");
                // TODO 'x', 'y', 'z' or other to select planes
                // TODO-LATER: 'm' to nudge
                stl_extrude(&stl, globals.key_pressed['c'], &dxf, dxf_selection_mask, stl_selected_triangle, height);
            }
            if (globals.key_pressed['r']) {
                // TODO: revolve
                // TODO: revolvution axes

            }
            if (gui_button("save stl", '6')) stl_save_binary(&stl, "out_binary.stl");
        }
    }
}
