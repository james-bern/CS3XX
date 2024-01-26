// // Conversation
// NOTE: This is a little CAD program Jim is actively developing.
//       It takes in an OMAX DXF and let's you rapidly create a 3D-printable STL using Manifold.


// TODO: <leader>c comment toggle
// TODO: selecting xy yz and xz planes
// TODO: visualize normal of selected plane (3D arrow)
// TODO: '+'/'-' (cut of +5/-2)
// TODO: bosses on non-y-up geometry
// TODO: loading STL
// TODO: load file dialogue
// TODO: save file dialogue
// TODO: selecting line entity to say extrude height (maybe using 'M')


// TODO: work plane
// TODO: selecting surface for plane
// TODO: planar offsets
// TODO: preview
// TODO: some sort of BVH or grid


// TODO: usage of vec3 is sus af (at least try to keep it contained to like...cross)
// TODO: cow soup/eso should take arbitrary sizes per entity (dots)


// roadmap
// / import dxf
// / detect pick_loops
// / 2D picking of pick_loops by clicking on DXF
// / omax-type dxf_selection_mask ('s' + 'c'); ('d' + 'a')
// / multi-select (no shift--follow omax style)
// / turn loop into polygon (rasterize arcs into line segments)
// / cut has depth
// / compile/link manifold
// / boss ('b') and cut ('c') -- (or 'e'/'E'?)
// / select quality (SELECT_MODIFIER_CONNECTED SELECT_MODIFIER_QUALITY)
// / 2D / 3D ui (2D goes it its own box)
// - cross-section arc discretization quality knobs (the correct knob is ??)
// - planar surface picking (and if no 3D geometry yet, your choice of the xy yz and zx planes)
// ---
// ? nudging with x, y
// ? you can persist the 2D curves alongside the 3D geometry they generated and then use omax style picking on the 3D part
// ? how are we storing the sequence of operations


// NOTE: for a first cut it could be destructive like omax itself
//       basically just like a conversational mill
//       don't store anything but the STL created so far (and maybe an undo stack eventually)
//       could at least print to the terminal what you've done
//       should preview the cut on the surface before you accept it (eventually) -- and let you nudge it in x, y, and z




#include "cs345.cpp"
#include "poe.cpp"
#undef real // ???


real32 EPSILON_DEFAULT = 0.03f;
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
    ASSERT(dxf->entities);
    free(dxf->entities);
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
            static char buffer[512];
            while (fgets(buffer, ARRAY_LENGTH(buffer), file)) {
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
        } fclose(file);
    }

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
#define SELECT_MODIFIER_NONE 0
#define SELECT_MODIFIER_CONNECTED 1
#define SELECT_MODIFIER_QUALITY 2
void dxf_pick(Camera2D *camera2D, DXF *dxf, bool32 *dxf_selection_mask, u32 select_mode, u32 select_modifier, u32 *num_entities_in_pick_loops, DXFEntityIndexAndFlipFlag **pick_loops, u32 *pick_loop_index_from_entity_index, real32 epsilon = EPSILON_DEFAULT) {
    if (!globals.mouse_left_held) return;
    if (select_mode == SELECT_MODE_NONE) return;

    bool32 value_to_write_to_selection_mask = (select_mode == SELECT_MODE_SELECT);
    bool32 modifier_connected = (select_modifier == SELECT_MODIFIER_CONNECTED);

    real32 PV[16];
    _camera_get_PV(camera2D, PV);
    real32 x, y;
    _input_get_mouse_position_and_change_in_position_in_world_coordinates(PV, &x, &y, NULL, NULL);

    int hot_entity_index = -1;
    double hot_squared_distance = HUGE_VAL;
    for (u32 i = 0; i < dxf->num_entities; ++i) {
        DXFEntity *entity = &dxf->entities[i];
        double squared_distance = squared_distance_point_entity(x, y, entity);
        squared_distance /= (camera2D->screen_height_World * camera2D->screen_height_World / 4); // NDC
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
    {
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
    }
    #endif
    // populate List's
    List<List<Vertex2D>> stretchy_list = {}; {
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
    result.polygonal_loops = (Vertex2D **) calloc(result.num_polygonal_loops, sizeof(Vertex2D *));
    for (u32 i = 0; i < result.num_polygonal_loops; ++i) {
        result.num_vertices_in_polygonal_loops[i] = stretchy_list.data[i].length;
        result.polygonal_loops[i] = (Vertex2D *) calloc(result.num_vertices_in_polygonal_loops[i], sizeof(Vertex2D));
        memcpy(result.polygonal_loops[i], stretchy_list.data[i].data, result.num_vertices_in_polygonal_loops[i] * sizeof(Vertex2D));
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




////////////////////////////////////////
// 3D //////////////////////////////////
////////////////////////////////////////




#define FEATURE_MODE_NONE         0
#define FEATURE_MODE_EXTRUDE_BOSS 1
#define FEATURE_MODE_EXTRUDE_CUT  2
#define FEATURE_MODE_REVOLVE      3




struct ConversationMesh {
    u32 num_vertices;
    u32 num_triangles;
    real32 *vertex_positions;
    u32 *triangle_indices;
    real32 *face_normals;

    u32 num_cosmetic_edges;
    u32 *cosmetic_edges;
};

void eso_vertex(real32 *p_j) {
    eso_vertex(p_j[0], p_j[1], p_j[2]);
}

void eso_vertex(real32 *p, u32 j) {
    eso_vertex(p[3 * j + 0], p[3 * j + 1], p[3 * j + 2]);
}

void conversation_mesh_draw(Camera3D *camera, ConversationMesh *mesh, bool32 some_triangle_exists_that_matches_n_selected_and_r_n_selected, vec3 n_selected, real32 r_n_selected) {
    mat4 V = camera_get_V(camera);
    mat4 PV = camera_get_P(camera) * V;


    if (mesh->cosmetic_edges) {
        eso_begin(PV, SOUP_LINES, 3.0f); 
        eso_color(monokai.black);
        // 3 * num_triangles * 2 / 2
        for (u32 k = 0; k < 2 * mesh->num_cosmetic_edges; ++k) eso_vertex(mesh->vertex_positions, mesh->cosmetic_edges[k]);
        eso_end();
    }

    for (u32 pass = 0; pass <= 1; ++pass) {
        eso_begin(PV, (!globals.key_toggled['h']) ? SOUP_TRIANGLES : SOUP_OUTLINED_TRIANGLES);
        for (u32 i = 0; i < mesh->num_triangles; ++i) {
            // FORNOW (all this crap)
            // TODO 

            vec3 n;
            for (u32 d = 0; d < 3; ++d) n[d] = mesh->face_normals[3 * i + d];

            vec3 p[3];
            real32 x_n;
            {
                for (u32 j = 0; j < 3; ++j) for (u32 d = 0; d < 3; ++d) p[j][d] = mesh->vertex_positions[3 * mesh->triangle_indices[3 * i + j] + d];
                x_n = dot(n, p[0]);
            }

            vec3 color; 
            real32 alpha;
            {
                vec3 n_camera = transformNormal(V, n);
                // if (n_camera.z > 0)
                {
                    if (some_triangle_exists_that_matches_n_selected_and_r_n_selected && (dot(n, n_selected) > 0.999f) && (ABS(x_n - r_n_selected) < 0.001f)) {
                        if (pass == 0) continue;
                        color = monokai.yellow;
                        alpha = 0.5f;
                    } else {
                        if (n_camera.z < 0.0f) n_camera *= -1; // FORNOW
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

void mesh_save_stl(ConversationMesh *mesh, char *filename) {
    FILE *file = fopen(filename, "w");
    ASSERT(file);

    int num_bytes = 80 + 4 + 50 * mesh->num_triangles;
    char *buffer = (char *) calloc(num_bytes, 1); {
        int offset = 80;
        memcpy(buffer + offset, &mesh->num_triangles, 4);
        offset += 4;
        for (u32 i = 0; i < mesh->num_triangles; ++i) {
            real32 triangle_normal[3];
            {
                // // NOTE: 90-degree rotation about x
                // x <- x
                // y <- -z
                // z <- y
                triangle_normal[0] =  mesh->face_normals[3 * i + 0];
                triangle_normal[1] = -mesh->face_normals[3 * i + 2];
                triangle_normal[2] =  mesh->face_normals[3 * i + 1];
            }
            memcpy(buffer + offset, &triangle_normal, 12);
            offset += 12;
            real32 triangle_vertex_positions[9];
            for (u32 j = 0; j < 3; ++j) {
                // // NOTE: 90-degree rotation about x
                // x <- x
                // y <- -z
                // z <- y
                triangle_vertex_positions[3 * j + 0] =  mesh->vertex_positions[3 * mesh->triangle_indices[3 * i + j] + 0];
                triangle_vertex_positions[3 * j + 1] = -mesh->vertex_positions[3 * mesh->triangle_indices[3 * i + j] + 2];
                triangle_vertex_positions[3 * j + 2] =  mesh->vertex_positions[3 * mesh->triangle_indices[3 * i + j] + 1];
            }
            memcpy(buffer + offset, triangle_vertex_positions, 36);
            offset += 38;
        }
    }
    fwrite(buffer, 1, num_bytes, file);
    free(buffer);

    fclose(file);
}

void mesh_free(ConversationMesh *mesh) {
    if (mesh->vertex_positions) free(mesh->vertex_positions);
    if (mesh->triangle_indices) free(mesh->triangle_indices);
    if (mesh->face_normals) free(mesh->face_normals);
    if (mesh->cosmetic_edges) free(mesh->cosmetic_edges);
    *mesh = {};
}


#include "manifoldc.h"
void wrapper_manifold(
        ManifoldManifold **curr__NOTE_GETS_UPDATED,
        ConversationMesh *mesh, // dest__NOTE_GETS_OVERWRITTEN,
        u32 num_polygonal_loops,
        u32 *num_vertices_in_polygonal_loops,
        Vertex2D **polygonal_loops,
        mat4 M_selected,
        u32 feature_mode,
        real32 feature_param) {

    ASSERT(feature_mode != FEATURE_MODE_NONE);
    {
        bool32 subtract;
        if (feature_mode == FEATURE_MODE_EXTRUDE_BOSS) {
            subtract = false;
        } else if (feature_mode == FEATURE_MODE_EXTRUDE_CUT) {
            subtract = true;
        } else {
            subtract = false;
        }

        ManifoldManifold *manifold; {
            ManifoldSimplePolygon **simple_polygon_array = (ManifoldSimplePolygon **) malloc(num_polygonal_loops * sizeof(ManifoldSimplePolygon *));
            for (u32 i = 0; i < num_polygonal_loops; ++i) {
                simple_polygon_array[i] = manifold_simple_polygon(malloc(manifold_simple_polygon_size()), (ManifoldVec2 *) polygonal_loops[i], num_vertices_in_polygonal_loops[i]);
            }
            ManifoldPolygons *polygons = manifold_polygons(malloc(manifold_polygons_size()), simple_polygon_array, num_polygonal_loops);
            ManifoldCrossSection *cross_section = manifold_cross_section_of_polygons(malloc(manifold_cross_section_size()), polygons, ManifoldFillRule::MANIFOLD_FILL_RULE_EVEN_ODD);



            if (feature_mode == FEATURE_MODE_EXTRUDE_BOSS || feature_mode == FEATURE_MODE_EXTRUDE_CUT) {
                // if (feature_mode == FEATURE_MODE_EXTRUDE_CUT) feature_param = -feature_param; // FORNOW
                manifold = manifold_extrude(malloc(manifold_manifold_size()), cross_section, ABS(feature_param), 0, 0.0f, 1.0f, 1.0f);
                if (feature_param < 0.0f) manifold = manifold_mirror(manifold, manifold, 0.0, 0.0, 1.0);

                { // TODO transform
                  // manifold = manifold_translate(manifold, manifold, 0.0f, 0.0f, x_n);
                  // manifold = manifold_rotate(manifold, manifold, -90.0f, 0.0f, 0.0f);
                    manifold = manifold_transform(manifold, manifold,
                            M_selected(0, 0), M_selected(1, 0), M_selected(2, 0),
                            M_selected(0, 1), M_selected(1, 1), M_selected(2, 1),
                            M_selected(0, 2), M_selected(1, 2), M_selected(2, 2),
                            M_selected(0, 3), M_selected(1, 3), M_selected(2, 3));
                }

            } else {
                manifold = manifold_revolve(malloc(manifold_manifold_size()), cross_section, NUM_SEGMENTS_PER_CIRCLE);
            }
        }

        // add
        if (!(*curr__NOTE_GETS_UPDATED)) {
            if (feature_mode == FEATURE_MODE_EXTRUDE_CUT) { return; } // FORNOW

            *curr__NOTE_GETS_UPDATED = manifold;
        } else {
            // TODO: ? manifold_delete_manifold(curr__NOTE_GETS_UPDATED);
            *curr__NOTE_GETS_UPDATED =
                manifold_boolean(
                        malloc(manifold_manifold_size()),
                        *curr__NOTE_GETS_UPDATED,
                        manifold,
                        (!subtract) ? ManifoldOpType::MANIFOLD_ADD : ManifoldOpType::MANIFOLD_SUBTRACT
                        );
        }

        ManifoldMeshGL *meshgl = manifold_get_meshgl(malloc(manifold_meshgl_size()), *curr__NOTE_GETS_UPDATED);

        mesh_free(mesh);
        mesh->num_vertices = manifold_meshgl_num_vert(meshgl);
        mesh->num_triangles = manifold_meshgl_num_tri(meshgl);
        mesh->vertex_positions = manifold_meshgl_vert_properties(malloc(manifold_meshgl_vert_properties_length(meshgl) * sizeof(real32)), meshgl);
        mesh->triangle_indices = manifold_meshgl_tri_verts(malloc(manifold_meshgl_tri_length(meshgl) * sizeof(u32)), meshgl);

        { // face_normals

            // FORNOW: uses snail
            // TODO: remove dependency
            mesh->face_normals = (real32 *) malloc(mesh->num_triangles * 3 * sizeof(real32));
            vec3 p[3];
            for (u32 i = 0; i < mesh->num_triangles; ++i) {
                for (u32 j = 0; j < 3; ++j) for (u32 d = 0; d < 3; ++d) p[j][d] = mesh->vertex_positions[3 * mesh->triangle_indices[3 * i + j] + d];
                vec3 n = normalized(cross(p[1] - p[0], p[2] - p[0]));
                for (u32 d = 0; d < 3; ++d) mesh->face_normals[3 * i + d] = n[d];
            }
        }

        { // cosmetic_edges

            // approach: prep a big array that maps edge -> cwiseProduct of face normals (start it at 1, 1, 1) // (faces that edge is part of)
            //           iterate through all edges detministically (ccw in order, flipping as needed so lower_index->higher_index)
            //           then go back through the array and sum the components; if below some threshold (maybe...0.9?) add that index to a stretchy buffer

            // TODO: a linalg library that operates directly on real32 *'s (like you used in soft_robot.h)

            List<u32> list = {}; {
                Map<Pair<u32, u32>, vec3> map = {}; {
                    for (u32 i = 0; i < mesh->num_triangles; ++i) {
                        vec3 n = { mesh->face_normals[3 * i + 0], mesh->face_normals[3 * i + 1], mesh->face_normals[3 * i + 2] };
                        for (u32 jj0 = 0, jj1 = (3 - 1); jj0 < 3; jj1 = jj0++) {
                            u32 j0 = mesh->triangle_indices[3 * i + jj0];
                            u32 j1 = mesh->triangle_indices[3 * i + jj1];
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
                mesh->num_cosmetic_edges = list.length / 2;
                mesh->cosmetic_edges = (u32 *) calloc(2 * mesh->num_cosmetic_edges, sizeof(u32));
                memcpy(mesh->cosmetic_edges, list.data, 2 * mesh->num_cosmetic_edges * sizeof(u32)); 
            }
            list_free(&list);
        }
    }

}


bool *key_pressed = globals.key_pressed;
bool *key_toggled = globals.key_toggled;


// FORNOW
mat4 get_M_selected(vec3 n_selected, real32 r_n_selected) {
    vec3 up = { 0.0f, 1.0f, 0.0f };
    real32 dot_product = dot(n_selected, up);
    vec3 y = (ARE_EQUAL(ABS(dot_product), 1.0)) ? V3(0.0,  0.0, -SGN(dot_product)) : up;
    vec3 x = normalized(cross(y, n_selected));
    vec3 z = cross(x, y);
    return M4_xyzo(x, y, z, r_n_selected * n_selected);
}


int main() {
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

    DXF dxf;
    DXFLoopAnalysisResult pick;

    bool32 *dxf_selection_mask;

    ManifoldManifold *manifold;
    ConversationMesh mesh;

    u32 feature_mode;
    real32 feature_param;
    char feature_param_buffer[256];
    char *feature_param_buffer_write_head;
    auto feature_param_buffer_reset = [&]() {
        memset(feature_param_buffer, 0, ARRAY_LENGTH(feature_param_buffer) * sizeof(char));
        feature_param_buffer_write_head = feature_param_buffer;
    };
    bool32 feature_param_sign_toggle;

    Camera2D camera2D;
    Camera3D camera3D;

    bool32 initialized = false;
    while (cow_begin_frame()) {
        if (!initialized || gui_button("reset")) {
            initialized = true;

            hot_pane = HOT_PANE_NONE;

            select_mode = SELECT_MODE_SELECT;
            select_modifier = SELECT_MODIFIER_CONNECTED;

            some_triangle_exists_that_matches_n_selected_and_r_n_selected = false;
            n_selected = { 0.0, 1.0, 0.0 };
            r_n_selected = 0.0f;
            M_selected = get_M_selected(n_selected, r_n_selected);

            dxf = dxf_load("omax.dxf");
            pick = dxf_loop_analysis_create(&dxf);

            dxf_selection_mask = (bool32 *) calloc(dxf.num_entities, sizeof(bool32));

            manifold = NULL;
            mesh = {};
            #if 0
            {
                mesh = {};
                mesh.num_vertices = 4;
                mesh.num_triangles = 4;
                mesh.vertex_positions = (real32 *) calloc(3 * mesh.num_vertices, sizeof(real32));
                mesh.triangle_indices = (u32 *) calloc(3 * mesh.num_triangles, sizeof(u32));
                float h = (1.0f + SQRT(3.0f)) / 2;
                {
                    u32 k = 0;
                    mesh.vertex_positions[k++] = 100.0f * COS(RAD(0.0));
                    mesh.vertex_positions[k++] = 100.0f * 0.0f;
                    mesh.vertex_positions[k++] = 100.0f * SIN(RAD(0.0));
                    mesh.vertex_positions[k++] = 100.0f * COS(RAD(120.0));
                    mesh.vertex_positions[k++] = 100.0f * 0.0f;
                    mesh.vertex_positions[k++] = 100.0f * SIN(RAD(120.0));
                    mesh.vertex_positions[k++] = 100.0f * COS(RAD(240.0));
                    mesh.vertex_positions[k++] = 100.0f * 0.0f;
                    mesh.vertex_positions[k++] = 100.0f * SIN(RAD(240.0));
                    mesh.vertex_positions[k++] = 100.0f * 0.0f;
                    mesh.vertex_positions[k++] = 100.0f * h;
                    mesh.vertex_positions[k++] = 100.0f * 0.0f;
                }
                {
                    u32 k = 0;
                    mesh.triangle_indices[k++] = 0;
                    mesh.triangle_indices[k++] = 1;
                    mesh.triangle_indices[k++] = 2;
                    mesh.triangle_indices[k++] = 1;
                    mesh.triangle_indices[k++] = 0;
                    mesh.triangle_indices[k++] = 3;
                    mesh.triangle_indices[k++] = 2;
                    mesh.triangle_indices[k++] = 1;
                    mesh.triangle_indices[k++] = 3;
                    mesh.triangle_indices[k++] = 0;
                    mesh.triangle_indices[k++] = 2;
                    mesh.triangle_indices[k++] = 3;
                }
            }
            #endif

            feature_mode = FEATURE_MODE_EXTRUDE_BOSS;
            feature_param = 0.0f;
            feature_param_sign_toggle = false;
            feature_param_buffer_reset();

            camera2D = { 300.0f, 150.0f };
            camera3D = { 300.0f, RAD(0.0f), RAD(15.0f), RAD(-30.0f), -150.0f };
        }
        if (gui_button("save")) mesh_save_stl(&mesh, "out.stl");

        u32 window_width, window_height; {
            real32 _window_width, _window_height; // FORNOW
            _window_get_size(&_window_width, &_window_height);
            window_width = (u32) _window_width;
            window_height = (u32) _window_height;
        }

        { // gui
            glDisable(GL_DEPTH_TEST);
            eso_begin(globals.Identity, SOUP_QUADS);
            eso_color(0.9f, 0.9f, 0.9f);
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

        if (globals.mouse_left_pressed || globals.mouse_right_pressed || (!globals.mouse_left_held && !globals.mouse_right_held)) {
            hot_pane = (globals.mouse_position_NDC.x < 0) ? HOT_PANE_2D : HOT_PANE_3D;
        }

        if (hot_pane == HOT_PANE_2D) {
            camera_move(&camera2D);
        } else if (hot_pane == HOT_PANE_3D) {
            camera_move(&camera3D);
        }

        { // 2D -> 3D
            if (feature_mode != FEATURE_MODE_EXTRUDE_BOSS) {
                if (key_pressed['e'] && !globals.key_shift_held) {
                    feature_mode = FEATURE_MODE_EXTRUDE_BOSS;
                    feature_param_buffer_reset();
                    feature_param_sign_toggle = false;
                }
            }
            if (feature_mode != FEATURE_MODE_EXTRUDE_CUT) {
                if (key_pressed['E'] && globals.key_shift_held) {
                    feature_mode = FEATURE_MODE_EXTRUDE_CUT;
                    feature_param_buffer_reset();
                    feature_param_sign_toggle = true;
                }
            }
            if (feature_mode != FEATURE_MODE_REVOLVE) {
                if (key_pressed['r']) {
                    feature_mode = FEATURE_MODE_REVOLVE;
                    feature_param_buffer_reset();
                }
            } 
            if (feature_mode != FEATURE_MODE_NONE) {
                if (globals.key_any_key_pressed) {
                    if (key_pressed[COW_KEY_BACKSPACE]) {
                        if (feature_param_buffer_write_head != feature_param_buffer) *--feature_param_buffer_write_head = 0;
                    } else if (select_modifier != SELECT_MODIFIER_QUALITY) {
                        bool32 valid_key_pressed; {
                            valid_key_pressed = false;
                            valid_key_pressed |= key_pressed['.'];
                            for (u32 i = 0; i < 10; ++i) valid_key_pressed |= key_pressed['0' + i];
                        }
                        if (valid_key_pressed) {
                            *feature_param_buffer_write_head++ = (char) globals.key_last_key_pressed;
                        }
                    }
                } 
                if (key_pressed[COW_KEY_TAB]) feature_param_sign_toggle = !feature_param_sign_toggle;
                if (key_pressed[COW_KEY_ENTER]) {
                    // feature_mode = FEATURE_MODE_NONE;
                    // NOTE: holds over previous
                    if (feature_param_buffer_write_head != feature_param_buffer) {
                        feature_param = strtof(feature_param_buffer, NULL);
                        feature_param_buffer_reset();
                    }
                    CrossSection cross_section = cross_section_create(&dxf, dxf_selection_mask);
                    // cross_section_debug_draw(&camera2D, &cross_section);
                    {
                        wrapper_manifold(
                                &manifold,
                                &mesh,
                                cross_section.num_polygonal_loops,
                                cross_section.num_vertices_in_polygonal_loops,
                                cross_section.polygonal_loops,
                                M_selected,
                                feature_mode,
                                ((!feature_param_sign_toggle) ? 1.0f : -1.0f) * feature_param
                                );
                        // memset(dxf_selection_mask, 0, dxf.num_entities * sizeof(bool32));
                        { // some_triangle_exists_that_matches_n_selected_and_r_n_selected
                          // FORNOW: this code heavily repeats conversation_mesh_draw
                            some_triangle_exists_that_matches_n_selected_and_r_n_selected = false;
                            for (u32 i = 0; i < mesh.num_triangles; ++i) {
                                // FORNOW
                                vec3 n = { mesh.face_normals[3 * i + 0], mesh.face_normals[3 * i + 1], mesh.face_normals[3 * i + 2] };

                                // FORNOW
                                vec3 p[3];
                                for (u32 j = 0; j < 3; ++j) for (u32 d = 0; d < 3; ++d) p[j][d] = mesh.vertex_positions[3 * mesh.triangle_indices[3 * i + j] + d];
                                real32 x_n = dot(n, p[0]);

                                if ((dot(n, n_selected) > 0.999f) && (ABS(x_n - r_n_selected) < 0.001f)) {
                                    some_triangle_exists_that_matches_n_selected_and_r_n_selected = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        { // 2D
            { // pick
                { // all mouse modes
                    if (key_pressed['s']) {
                        select_mode = SELECT_MODE_SELECT;
                        select_modifier = SELECT_MODIFIER_NONE;
                    }
                    if (key_pressed['d']) {
                        select_mode = SELECT_MODE_DESELECT;
                        select_modifier = SELECT_MODIFIER_NONE;
                    }
                    if (key_pressed['c']) {
                        select_modifier = SELECT_MODIFIER_CONNECTED;
                    }
                    if (select_mode != SELECT_MODE_NONE) {
                        bool32 value_to_write_to_selection_mask = (select_mode == SELECT_MODE_SELECT);

                        if (key_pressed['q']) select_modifier = SELECT_MODIFIER_QUALITY;

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
                        if (key_pressed['a']) {
                            for (u32 i = 0; i < dxf.num_entities; ++i) dxf_selection_mask[i] = value_to_write_to_selection_mask;
                        }
                    }
                }
                if (hot_pane == HOT_PANE_2D) {
                    dxf_pick(&camera2D, &dxf, dxf_selection_mask, select_mode, select_modifier, pick.num_entities_in_loops, pick.loops, pick.loop_index_from_entity_index);
                }
            }
            { // draw
                glEnable(GL_SCISSOR_TEST);
                glScissor(0, 0, window_width / 2, window_height);
                {
                    eso_begin(camera_get_PV(&camera2D), SOUP_LINES);
                    for (u32 i = 0; i < dxf.num_entities; ++i) {
                        DXFEntity *entity = &dxf.entities[i];
                        int32 color = (dxf_selection_mask[i]) ? DXF_COLOR_SELECTION : DXF_COLOR_DONT_OVERRIDE;
                        eso_dxf_entity__SOUP_LINES(entity, color);
                    }
                    eso_end();

                    // dots
                    if (key_toggled['h']) {
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
                glDisable(GL_SCISSOR_TEST);
            }
        }

        real32 feature_param_preview; {
            if (feature_param_buffer_write_head == feature_param_buffer) {
                feature_param_preview = feature_param;
            } else {
                feature_param_preview = strtof(feature_param_buffer, NULL);
            }
            if (feature_param_sign_toggle) feature_param_preview *= -1;
        }


        { // 3D
            { // 3D pick
                {
                    real32 sign;
                    if (key_pressed['x'] || key_pressed['y'] || key_pressed['z']) {
                        some_triangle_exists_that_matches_n_selected_and_r_n_selected = false;
                        r_n_selected = 0.0f;
                        sign = (!globals.key_shift_held) ? 1.0f : -1.0f;
                    }
                    if (key_pressed['x']) n_selected = { sign, 0.0f, 0.0f };
                    if (key_pressed['y']) n_selected = { 0.0f, sign, 0.0f };
                    if (key_pressed['z']) n_selected = { 0.0f, 0.0f, sign };
                    M_selected = get_M_selected(n_selected, r_n_selected);
                }
                if (hot_pane == HOT_PANE_3D) {
                    if (globals.mouse_left_pressed) {

                        int32 selected_triangle_index = -1;
                        {
                            vec3 minus_dir;
                            _camera_get_coordinate_system(&camera3D, NULL, NULL, NULL, (real32 *) &minus_dir, NULL, NULL);
                            vec3 o = transformPoint(inverse(camera_get_PV(&camera3D)), V3(globals.mouse_position_NDC, -0.99f));
                            vec3 p[3];
                            real32 min_distance = HUGE_VAL;
                            for (u32 i = 0; i < mesh.num_triangles; ++i) {
                                for (u32 j = 0; j < 3; ++j) for (u32 d = 0; d < 3; ++d) p[j][d] = mesh.vertex_positions[3 * mesh.triangle_indices[3 * i + j] + d];
                                vec4 w_t = inverse(hstack(V4(p[0], 1.0f), V4(p[1], 1.0f), V4(p[2], 1.0f), V4(minus_dir, 0.0f))) * V4(o, 1.0f);
                                if ((w_t.x > 0) && (w_t.y > 0) && (w_t.z > 0) && (w_t.w > 0)) {
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
                                n_selected = { mesh.face_normals[3 * selected_triangle_index + 0], mesh.face_normals[3 * selected_triangle_index + 1], mesh.face_normals[3 * selected_triangle_index + 2] };
                                vec3 p_selected[3];
                                for (u32 j = 0; j < 3; ++j) for (u32 d = 0; d < 3; ++d) p_selected[j][d] = mesh.vertex_positions[3 * mesh.triangle_indices[3 * selected_triangle_index + j] + d];
                                r_n_selected = dot(n_selected, p_selected[0]);
                            }
                            M_selected = get_M_selected(n_selected, r_n_selected);

                        }

                    }
                }
            }
            { // 3D draw
                mat4 P_3D = camera_get_P(&camera3D);
                mat4 V_3D = camera_get_V(&camera3D);
                mat4 PV_3D = P_3D * V_3D;
                glEnable(GL_SCISSOR_TEST);
                glScissor(window_width / 2, 0, window_width / 2, window_height);
                { // 2D selection
                    for (u32 pass = 0; pass <= 1; ++pass) {
                        u32 color = (feature_mode == FEATURE_MODE_EXTRUDE_BOSS) ? DXF_COLOR_TRAVERSE : DXF_COLOR_QUALITY_1; // FORNOW
                        mat4 M = M_selected;
                        if (pass == 1) {
                            if (feature_mode == FEATURE_MODE_NONE) break;
                            M = M * M4_Translation(0.0f, 0.0f, feature_param_preview);
                        }
                        eso_begin(camera_get_PV(&camera3D) * M, SOUP_LINES, 5.0f);
                        for (u32 i = 0; i < dxf.num_entities; ++i) {
                            DXFEntity *entity = &dxf.entities[i];
                            if (dxf_selection_mask[i]) {
                                eso_dxf_entity__SOUP_LINES(entity, color);
                            }
                        }
                        eso_end();
                    }
                }
                { // arrow
                    if (!IS_ZERO(feature_param_preview)) {
                        real32 total_height = ABS(feature_param_preview);
                        real32 cap_height = (total_height > 10.0) ? 5.0f : (0.5 * total_height);
                        real32 shaft_height = total_height - cap_height;
                        real32 s = (total_height > 10.0f) ? 1.0f : SQRT(total_height / 10.0f);
                        vec3 color = (feature_mode == FEATURE_MODE_EXTRUDE_BOSS) ? monokai.green : monokai.red;
                        mat4 N = (feature_param_preview > 0.0f) ? M4_Identity() : /* M4_Translation(0.0f, 0.0f, total_height) * */ M4_Scaling(1.0f, 1.0f, -1.0f);
                        mat4 R = M4_RotationAboutXAxis(RAD(90));
                        mat4 M_cyl  = M_selected * N * M4_Scaling(s * 1.0f, s * 1.0f, shaft_height) * R;
                        mat4 M_cone = M_selected * N * M4_Translation(0.0f, 0.0f, shaft_height) * M4_Scaling(s * 2.0f, s * 2.0f, cap_height) * R;
                        library.meshes.cylinder.draw(P_3D, V_3D, M_cyl, color);
                        library.meshes.cone.draw(P_3D, V_3D, M_cone, color);
                    }
                }
                { // axes
                    library.soups.axes.draw(PV_3D * M4_Scaling(10.0f));
                }
                { // plane
                    if (!some_triangle_exists_that_matches_n_selected_and_r_n_selected) { // planes
                        real32 r = 50.0f;
                        eso_begin(PV_3D * M_selected, SOUP_OUTLINED_QUADS);
                        eso_color(monokai.yellow, 0.5f);
                        eso_vertex( r,  r, 0.0f);
                        eso_vertex( r, -r, 0.0f);
                        eso_vertex(-r, -r, 0.0f);
                        eso_vertex(-r,  r, 0.0f);
                        eso_end();
                    }
                }

                // NOTE: includes transparency; has to come last
                conversation_mesh_draw(&camera3D, &mesh, some_triangle_exists_that_matches_n_selected_and_r_n_selected, n_selected, r_n_selected);

                glDisable(GL_SCISSOR_TEST);
            }
            { // notes
                ;
                // TODO: we need an actual example(s)

                // // TODO
                // #define EXTRUDE_META_DATA_TYPE_MAJOR_PLANE 0
                // #define EXTRUDE_META_DATA_TYPE_TRIANGLE 1
                // struct ExtrudeMetaData {
                //     bool32 type;
                //     real32 x_offset;
                //     real32 y_offset;
                // }
                if (key_pressed['b'] || key_pressed['c']) {
                    // TODO begin operation (can use gui elements to get values from user)
                    // then press of enter to finish operation
                    // tabbing through the elements would be clutch, but not needed at first
                    // real32 height = conversation_get_real32("height");
                    // TODO 'x', 'y', 'z' or other to select planes
                    // TODO-LATER: 'm' to nudge
                    // stl_extrude(&stl, key_pressed['c'], &dxf, dxf_selection_mask, stl_selected_triangle, height);
                }
                if (key_pressed['z']) {
                    // TODO: revolve
                    // TODO: revolvution axes

                }
            }
        }
        { // gui
            gui_printf("mouse %s %s", (select_mode == SELECT_MODE_NONE) ? "" : (select_mode == SELECT_MODE_SELECT) ? "SELECT" : "DESELCT", (select_modifier == SELECT_MODE_NONE) ? "" : (select_modifier == SELECT_MODIFIER_CONNECTED) ?  "CONNECTED" : "QUALITY");

            char tmp[256]; {
                char *units = (char *) (((feature_mode == FEATURE_MODE_EXTRUDE_BOSS) || (feature_mode == FEATURE_MODE_EXTRUDE_CUT)) ? "mm" : "deg");
                if (feature_param_buffer_write_head == feature_param_buffer) {
                    sprintf(tmp, "%g%s", feature_param, units);
                } else {
                    sprintf(tmp, "`%s%s", feature_param_buffer, units);
                }
            }
            gui_printf("enter %s %s", (feature_mode == FEATURE_MODE_NONE) ? "" : (feature_mode == FEATURE_MODE_EXTRUDE_BOSS) ? "EXTRUDE BOSS" : (feature_mode == FEATURE_MODE_EXTRUDE_CUT) ? "EXTRUDE CUT" : "REVOLVE", (feature_mode == FEATURE_MODE_NONE) ? "" : tmp);

            // gui_printf("some_triangle_exists_that_matches_n_selected_and_r_n_selected %d", some_triangle_exists_that_matches_n_selected_and_r_n_selected);
            // gui_printf("---");
            // gui_printf("---");
            if (key_toggled['h']) gui_printf("NUMBER OF TRIANGLES %d", mesh.num_triangles);
            gui_printf("");
            // gui_printf("n_selected %f %f %f", n_selected.x, n_selected.y, n_selected.z);
        }

    }
}
