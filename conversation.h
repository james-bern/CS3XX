////////////////////////////////////////
// Notes ///////////////////////////////
////////////////////////////////////////

// // Conversation
// This is a little CAD program Jim is making :)
// It takes in an OMAX DXF and let's you rapidly create a 3D-printable STL using Manifold.
// Intuitiveness of Sketchup + Precision of SolidWorks

// wonderful undo (including selections)
// - TODO: remove history stack (old undo)
// - TODO: remove dependence on stateful stuff in cow
// - TODO: strip out draw (basically done)
// - TODO: move pure functions and similar non-stateful stuff into conversation.h
// - TODO: copy solidworks stl export params

// typdef bool32 cow_bool;
// TODO: write blow style ability for "game to play[/debug] itself"
// revolves
// - manifold bug(?) where you don't get any geometry if the loop is in left half plane
// (m)iddle snap
// stl layers--stuff only booleans with its own layer (why?)
// IDEA: Translating and Rotating and scaling the (3D) workpiece (like in a mill)
// figuring out how to actually deal with the memory allocation, etc. in the manifoldc bindings
// TODO: color codes instead of ` in gui_printf
// ? TODO: the Hard problem of avoiding the creation of ultra-thin features

// TODO: incorporate console_params_preview_flip_flag fully into console_param_preview's (preview should have it all in there)


////////////////////////////////////////
// Config-Tweaks ///////////////////////
////////////////////////////////////////

real32 Z_FIGHT_EPS = 0.05f;
real32 TOLERANCE_DEFAULT = 1e-5f;
u32 NUM_SEGMENTS_PER_CIRCLE = 64;
real32 GRID_SIDE_LENGTH = 256.0f;
real32 GRID_SPACING = 10.0f;
real32 CAMERA_3D_DEFAULT_ANGLE_OF_VIEW = RAD(60.0f);

////////////////////////////////////////
// Top-Level State Machine /////////////
////////////////////////////////////////

#define ENTER_MODE_NONE              0
#define ENTER_MODE_EXTRUDE_ADD       1
#define ENTER_MODE_EXTRUDE_CUT       2
#define ENTER_MODE_REVOLVE_ADD       3
#define ENTER_MODE_REVOLVE_CUT       4
#define ENTER_MODE_LOAD              5
#define ENTER_MODE_SAVE              6
#define ENTER_MODE_MOVE_ORIGIN_TO    7
#define ENTER_MODE_OFFSET_PLANE_BY   8

#define CLICK_MODE_NONE              0
#define CLICK_MODE_SELECT            1
#define CLICK_MODE_DESELECT          2
#define CLICK_MODE_MOVE_2D_ORIGIN_TO 3

#define CLICK_MODIFIER_NONE      0
#define CLICK_MODIFIER_CONNECTED 1
#define CLICK_MODIFIER_QUALITY   2
#define CLICK_MODIFIER_WINDOW    3
#define CLICK_MODIFIER_CENTER_OF 4
#define CLICK_MODIFIER_END_OF    5
#define CLICK_MODIFIER_MIDDLE_OF 6

#define HOT_PANE_NONE 0
#define HOT_PANE_2D   1
#define HOT_PANE_3D   2

////////////////////////////////////////
// Data-Oriented Snail /////////////////
////////////////////////////////////////

vec3 get(real32 *x, u32 i) {
    vec3 result;
    for (u32 d = 0; d < 3; ++d) result.data[d] = x[3 * i + d];
    return result;
}
void set(real32 *x, u32 i, vec3 v) {
    for (u32 d = 0; d < 3; ++d) x[3 * i + d] = v.data[d];
}
void eso_vertex(real32 *p_j) {
    eso_vertex(p_j[0], p_j[1], p_j[2]);
}
void eso_vertex(real32 *p, u32 j) {
    eso_vertex(p[3 * j + 0], p[3 * j + 1], p[3 * j + 2]);
}

////////////////////////////////////////
// BoundingBox /////////////////////////
////////////////////////////////////////

struct BoundingBox {
    real32 min[2];
    real32 max[2];
};

void pprint(BoundingBox bounding_box) {
    printf("(%f, %f) <-> (%f, %f)\n", bounding_box.min[0], bounding_box.min[1], bounding_box.max[0], bounding_box.max[1]);
}

BoundingBox bounding_box_union(u32 num_bounding_boxes, BoundingBox *bounding_boxes, bool32 *mask = NULL) {
    BoundingBox result = { HUGE_VAL, HUGE_VAL, -HUGE_VAL, -HUGE_VAL };
    for (u32 i = 0; i < num_bounding_boxes; ++i) {
        if ((mask) && (!mask[i])) continue;
        for (u32 d = 0; d < 2; ++d) {
            result.min[d] = MIN(result.min[d], bounding_boxes[i].min[d]);
            result.max[d] = MAX(result.max[d], bounding_boxes[i].max[d]);
        }
    }
    return result;
}

bool32 bounding_box_contains(BoundingBox outer, BoundingBox inner) {
    for (u32 d = 0; d < 2; ++d) {
        if (outer.min[d] > inner.min[d]) return false;
        if (outer.max[d] < inner.max[d]) return false;
    }
    return true;
}

void camera2D_zoom_to_bounding_box(Camera2D *camera2D, BoundingBox bounding_box) {
    real32 new_o_x = AVG(bounding_box.min[0], bounding_box.max[0]);
    real32 new_o_y = AVG(bounding_box.min[1], bounding_box.max[1]);
    real32 new_height = MAX((bounding_box.max[0] - bounding_box.min[0]) * 2 / _window_get_aspect(), (bounding_box.max[1] - bounding_box.min[1])); // factor of 2 since splitscreen
    new_height *= 1.3f; // FORNOW: border
    camera2D->screen_height_World = new_height;
    camera2D->o_x = new_o_x;
    camera2D->o_y = new_o_y;
}

////////////////////////////////////////
// DXF /////////////////////////////////
////////////////////////////////////////

#define DXF_COLOR_TRAVERSE        0
#define DXF_COLOR_QUALITY_1       1
#define DXF_COLOR_QUALITY_2       2
#define DXF_COLOR_QUALITY_3       3
#define DXF_COLOR_QUALITY_4       4
#define DXF_COLOR_QUALITY_5       5
#define DXF_COLOR_ETCH            6
#define DXF_COLOR_WATER_ONLY      8
#define DXF_COLOR_LEAD_IO         9
#define DXF_COLOR_QUALITY_SLIT_1 21
#define DXF_COLOR_QUALITY_SLIT_2 22
#define DXF_COLOR_QUALITY_SLIT_3 23
#define DXF_COLOR_QUALITY_SLIT_4 24
#define DXF_COLOR_QUALITY_SLIT_5 25
#define DXF_COLOR_SELECTION     254
#define DXF_COLOR_DONT_OVERRIDE 255

struct DXFLine {
    real32 start_x;
    real32 start_y;
    real32 end_x;
    real32 end_y;
    real32 _;
};

struct DXFArc {
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
    u32 color;
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
    *dxf = {};
}

void dxf_load(char *filename, DXF *dxf) {
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
    dxf_free(dxf);

    FILE *file = (FILE *) fopen(filename, "r");
    ASSERT(file);

    List<DXFEntity> stretchy_list = {}; {
        {
            #define DXF_LOAD_MODE_NONE 0
            #define DXF_LOAD_MODE_LINE 1
            #define DXF_LOAD_MODE_ARC  2
            u8 mode = 0;
            int code = 0;
            bool32 code_is_hot = false;
            DXFEntity entity = {};
            static char buffer[512];
            while (fgets(buffer, ARRAY_LENGTH(buffer), file)) {
                if (mode == DXF_LOAD_MODE_NONE) {
                    if (poe_prefix_match(buffer, "LINE")) {
                        mode = DXF_LOAD_MODE_LINE;
                        code_is_hot = false;
                        entity = { DXF_ENTITY_TYPE_LINE };
                    } else if (poe_prefix_match(buffer, "ARC")) {
                        mode = DXF_LOAD_MODE_ARC;
                        code_is_hot = false;
                        entity = { DXF_ENTITY_TYPE_ARC };
                    }
                } else {
                    if (!code_is_hot) {
                        sscanf(buffer, "%d", &code);
                        // NOTE this initialization is sketchy but works
                        // probably don't make a habit of it
                        if (code == 0) {
                            list_push_back(&stretchy_list, entity);
                            mode = DXF_LOAD_MODE_NONE;
                            code_is_hot = false;
                        }
                    } else {
                        if (code == 62) {
                            int value;
                            sscanf(buffer, "%d", &value);
                            entity.color = value; 
                        } else {
                            float value;
                            sscanf(buffer, "%f", &value);
                            if (mode == DXF_LOAD_MODE_LINE) {
                                if (code == 10) {
                                    entity.line.start_x = MM(value);
                                } else if (code == 20) {
                                    entity.line.start_y = MM(value);
                                } else if (code == 11) {
                                    entity.line.end_x = MM(value);
                                } else if (code == 21) {
                                    entity.line.end_y = MM(value);
                                }
                            } else {
                                ASSERT(mode == DXF_LOAD_MODE_ARC);
                                if (code == 10) {
                                    entity.arc.center_x = MM(value);
                                } else if (code == 20) {
                                    entity.arc.center_y = MM(value);
                                } else if (code == 40) {
                                    entity.arc.radius = MM(value);
                                } else if (code == 50) {
                                    entity.arc.start_angle_in_degrees = value;
                                } else if (code == 51) {
                                    entity.arc.end_angle_in_degrees = value;
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

    dxf->num_entities = stretchy_list.length;
    dxf->entities = (DXFEntity *) calloc(dxf->num_entities, sizeof(DXFEntity));
    memcpy(dxf->entities, stretchy_list.data, dxf->num_entities * sizeof(DXFEntity));
    list_free(&stretchy_list);
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

void eso_dxf_entity__SOUP_LINES(DXFEntity *entity, int32 override_color = DXF_COLOR_DONT_OVERRIDE) {
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLine *line = &entity->line;
        _dxf_eso_color((override_color != DXF_COLOR_DONT_OVERRIDE) ? override_color : entity->color);
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
        _dxf_eso_color((override_color != DXF_COLOR_DONT_OVERRIDE) ? override_color : entity->color);
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

BoundingBox *dxf_entity_bounding_boxes_create(DXF *dxf) {
    BoundingBox *result = (BoundingBox *) malloc(dxf->num_entities * sizeof(BoundingBox));

    for (u32 entity_index = 0; entity_index < dxf->num_entities; ++entity_index) {
        DXFEntity *entity = &dxf->entities[entity_index];
        result[entity_index] = { HUGE_VAL, HUGE_VAL, -HUGE_VAL, -HUGE_VAL };
        real32 s[2][2];
        u32 n = 2;
        entity_get_start_and_end_points(entity, &s[0][0], &s[0][1], &s[1][0], &s[1][1]);
        for (u32 i = 0; i < n; ++i) {
            for (u32 d = 0; d < 2; ++d) {
                result[entity_index].min[d] = MIN(result[entity_index].min[d], s[i][d]);
                result[entity_index].max[d] = MAX(result[entity_index].max[d], s[i][d]);
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
            if (IS_BETWEEN(         0, start_angle, end_angle)) result[entity_index].max[0] = MAX(result[entity_index].max[0], arc->center_x + arc->radius);
            if (IS_BETWEEN(    PI / 2, start_angle, end_angle)) result[entity_index].max[1] = MAX(result[entity_index].max[1], arc->center_y + arc->radius);
            if (IS_BETWEEN(    PI    , start_angle, end_angle)) result[entity_index].min[0] = MIN(result[entity_index].min[0], arc->center_x - arc->radius);
            if (IS_BETWEEN(3 * PI / 2, start_angle, end_angle)) result[entity_index].min[1] = MIN(result[entity_index].min[1], arc->center_y - arc->radius);
        }
    }

    return result;
}

////////////////////////////////////////
// SquaredDistance /////////////////////
////////////////////////////////////////

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

real32 squared_distance_point_dxf_line(real32 x, real32 y, DXFLine *line) {
    return squared_distance_point_line_segment(x, y, line->start_x, line->start_y, line->end_x, line->end_y);
}

real32 squared_distance_point_dxf_arc(real32 x, real32 y, DXFArc *arc) {
    return squared_distance_point_arc_NOTE_pass_angles_in_radians(x, y, arc->center_x, arc->center_y, arc->radius, RAD(arc->start_angle_in_degrees), RAD(arc->end_angle_in_degrees));
}

real32 squared_distance_point_dxf_entity(real32 x, real32 y, DXFEntity *entity) {
    if (entity->type == DXF_ENTITY_TYPE_LINE) {
        DXFLine *line = &entity->line;
        return squared_distance_point_dxf_line(x, y, line);
    } else {
        ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
        DXFArc *arc = &entity->arc;
        return squared_distance_point_dxf_arc(x, y, arc);
    }
}

real32 squared_distance_point_dxf(real32 x, real32 y, DXF *dxf) {
    real32 result = HUGE_VAL;
    for (DXFEntity *entity = dxf->entities; entity < &dxf->entities[dxf->num_entities]; ++entity) {
        result = MIN(result, squared_distance_point_dxf_entity(x, y, entity));
    }
    return result;
}

////////////////////////////////////////
// LoopAnalysis ////////////////////////
////////////////////////////////////////

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

DXFLoopAnalysisResult dxf_loop_analysis_create(DXF *dxf, bool32 *dxf_selection_mask_NOTE_pass_NULL_for_pick = NULL) {
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
                #define MACRO_CANDIDATE_VALID(i) (!entity_already_added[i] && (!dxf_selection_mask_NOTE_pass_NULL_for_pick || dxf_selection_mask_NOTE_pass_NULL_for_pick[i]))
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
        for (u32 i = 0; i < stretchy_list.length; ++i) list_free(&stretchy_list.data[i]);
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
    if (analysis->num_entities_in_loops) {
        free(analysis->num_entities_in_loops);
    }
    if (analysis->loops) {
        for (u32 i = 0; i < analysis->num_loops; ++i) {
            free(analysis->loops[i]);
        }
        free(analysis->loops);
    }
    if (analysis->loop_index_from_entity_index) {
        free(analysis->loop_index_from_entity_index);
    }
    *analysis = {};
}

////////////////////////////////////////
// LoopAnalysis ////////////////////////
////////////////////////////////////////

struct CrossSectionEvenOdd {
    u32 num_polygonal_loops;
    u32 *num_vertices_in_polygonal_loops;
    vec2 **polygonal_loops;
};

CrossSectionEvenOdd cross_section_create(DXF *dxf, bool32 *dxf_selection_mask) {
    #if 0
    {
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(dxf);
        _SUPPRESS_COMPILER_WARNING_UNUSED_VARIABLE(dxf_selection_mask);
        CrossSectionEvenOdd result = {};
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
    CrossSectionEvenOdd result = {};
    result.num_polygonal_loops = stretchy_list.length;
    result.num_vertices_in_polygonal_loops = (u32 *) calloc(result.num_polygonal_loops, sizeof(u32));
    result.polygonal_loops = (vec2 **) calloc(result.num_polygonal_loops, sizeof(vec2 *));
    for (u32 i = 0; i < result.num_polygonal_loops; ++i) {
        result.num_vertices_in_polygonal_loops[i] = stretchy_list.data[i].length;
        result.polygonal_loops[i] = (vec2 *) calloc(result.num_vertices_in_polygonal_loops[i], sizeof(vec2));
        memcpy(result.polygonal_loops[i], stretchy_list.data[i].data, result.num_vertices_in_polygonal_loops[i] * sizeof(vec2));
    }

    // free List's
    for (u32 i = 0; i < stretchy_list.length; ++i) list_free(&stretchy_list.data[i]);
    list_free(&stretchy_list);

    return result;
}

void cross_section_debug_draw(Camera2D *camera2D, CrossSectionEvenOdd *cross_section) {
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
// FancyMesh, STL //////////////////////
////////////////////////////////////////

struct FancyMesh {
    u32 num_vertices;
    u32 num_triangles;
    real32 *vertex_positions;
    u32 *triangle_indices;
    real32 *triangle_normals;

    u32 num_cosmetic_edges;
    u32 *cosmetic_edges;
};

void fancy_mesh_triangle_normals_calculate(FancyMesh *fancy_mesh) {
    fancy_mesh->triangle_normals = (real32 *) malloc(fancy_mesh->num_triangles * 3 * sizeof(real32));
    vec3 p[3];
    for (u32 i = 0; i < fancy_mesh->num_triangles; ++i) {
        for (u32 j = 0; j < 3; ++j) p[j] = get(fancy_mesh->vertex_positions, fancy_mesh->triangle_indices[3 * i + j]);
        vec3 n = normalized(cross(p[1] - p[0], p[2] - p[0]));
        set(fancy_mesh->triangle_normals, i, n);
    }
}

void fancy_mesh_cosmetic_edges_calculate(FancyMesh *fancy_mesh) {
    // approach: prep a big array that maps edge -> cwiseProduct of face normals (start it at 1, 1, 1) // (faces that edge is part of)
    //           iterate through all edges detministically (ccw in order, flipping as needed so lower_index->higher_index)
    //           then go back and if passes some heuristic add that index to a stretchy buffer
    List<u32> list = {}; {
        Map<Pair<u32, u32>, vec3> map = {}; {
            for (u32 i = 0; i < fancy_mesh->num_triangles; ++i) {
                vec3 n = get(fancy_mesh->triangle_normals, i);
                for (u32 jj0 = 0, jj1 = (3 - 1); jj0 < 3; jj1 = jj0++) {
                    u32 j0 = fancy_mesh->triangle_indices[3 * i + jj0];
                    u32 j1 = fancy_mesh->triangle_indices[3 * i + jj1];
                    if (j0 > j1) {
                        u32 tmp = j0;
                        j0 = j1;
                        j1 = tmp;
                    }
                    Pair<u32, u32> key = { j0, j1 };
                    map_put(&map, key, cwiseProduct(n, map_get(&map, key, V3(1.0f))));
                }
            }
        }
        {
            for (List<Pair<Pair<u32, u32>, vec3>> *bucket = map.buckets; bucket < &map.buckets[map.num_buckets]; ++bucket) {
                for (Pair<Pair<u32, u32>, vec3> *pair = bucket->data; pair < &bucket->data[bucket->length]; ++pair) {
                    vec3 n2 = pair->value;
                    // pprint(n2);
                    if (squaredNorm(n2) < 0.3f) {
                        list_push_back(&list, pair->key.first); // FORNOW
                        list_push_back(&list, pair->key.second); // FORNOW
                    }
                }
            }
        }
        map_free(&map);
    }
    {
        fancy_mesh->num_cosmetic_edges = list.length / 2;
        fancy_mesh->cosmetic_edges = (u32 *) calloc(2 * fancy_mesh->num_cosmetic_edges, sizeof(u32));
        memcpy(fancy_mesh->cosmetic_edges, list.data, 2 * fancy_mesh->num_cosmetic_edges * sizeof(u32)); 
    }
    list_free(&list);
}


bool32 fancy_mesh_save_stl(FancyMesh *fancy_mesh, char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        return false;
    }

    int num_bytes = 80 + 4 + 50 * fancy_mesh->num_triangles;
    char *buffer = (char *) calloc(num_bytes, 1); {
        int offset = 80;
        memcpy(buffer + offset, &fancy_mesh->num_triangles, 4);
        offset += 4;
        for (u32 i = 0; i < fancy_mesh->num_triangles; ++i) {
            real32 triangle_normal[3];
            {
                // 90 degree rotation about x: (x, y, z) <- (x, -z, y)
                triangle_normal[0] =  fancy_mesh->triangle_normals[3 * i + 0];
                triangle_normal[1] = -fancy_mesh->triangle_normals[3 * i + 2];
                triangle_normal[2] =  fancy_mesh->triangle_normals[3 * i + 1];
            }
            memcpy(buffer + offset, &triangle_normal, 12);
            offset += 12;
            real32 triangle_vertex_positions[9];
            for (u32 j = 0; j < 3; ++j) {
                // 90 degree rotation about x: (x, y, z) <- (x, -z, y)
                triangle_vertex_positions[3 * j + 0] =  fancy_mesh->vertex_positions[3 * fancy_mesh->triangle_indices[3 * i + j] + 0];
                triangle_vertex_positions[3 * j + 1] = -fancy_mesh->vertex_positions[3 * fancy_mesh->triangle_indices[3 * i + j] + 2];
                triangle_vertex_positions[3 * j + 2] =  fancy_mesh->vertex_positions[3 * fancy_mesh->triangle_indices[3 * i + j] + 1];
            }
            memcpy(buffer + offset, triangle_vertex_positions, 36);
            offset += 38;
        }
    }
    fwrite(buffer, 1, num_bytes, file);
    free(buffer);

    fclose(file);
    return true;
}

void fancy_mesh_free(FancyMesh *fancy_mesh) {
    if (fancy_mesh->vertex_positions) free(fancy_mesh->vertex_positions);
    if (fancy_mesh->triangle_indices) free(fancy_mesh->triangle_indices);
    if (fancy_mesh->triangle_normals) free(fancy_mesh->triangle_normals);
    if (fancy_mesh->cosmetic_edges)   free(fancy_mesh->cosmetic_edges);
    *fancy_mesh = {};
}

void stl_load(char *filename, ManifoldManifold **manifold_manifold, FancyMesh *fancy_mesh) {
    // history_record_state(history, manifold_manifold, fancy_mesh); // FORNOW

    { // fancy_mesh
        u32 num_triangles;
        real32 *soup;
        {
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

            if (filetype == STL_FILETYPE_ASCII) {
                u8 ascii_scan_dummy[64];
                real32 ascii_scan_p[3];
                List<real32> ascii_data = {};

                FILE *file = (FILE *) fopen(filename, "r");
                while (fgets(line_of_file, ARRAY_LENGTH(line_of_file), file)) {
                    if (poe_prefix_match(line_of_file, "vertex")) {
                        sscanf(line_of_file, "%s %f %f %f", ascii_scan_dummy, &ascii_scan_p[0], &ascii_scan_p[1], &ascii_scan_p[2]);
                        for (u32 d = 0; d < 3; ++d) list_push_back(&ascii_data, ascii_scan_p[d]);
                    }
                }
                fclose(file);
                num_triangles = ascii_data.length / 9;
                u32 size = ascii_data.length * sizeof(real32);
                soup = (real32 *) malloc(size);
                memcpy(soup, ascii_data.data, size);
                list_free(&ascii_data);
            } else {
                ASSERT(filetype == STL_FILETYPE_BINARY);
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
                memcpy(&num_triangles, entire_file + offset, 4);
                offset += 4;
                u32 size = num_triangles * 36;
                soup = (real32 *) calloc(1, size);
                for (u32 i = 0; i < num_triangles; ++i) {
                    offset += 12;
                    memcpy(soup + i * 9, entire_file + offset, 36);
                    offset += 38;
                }
            }
            { // -90 degree rotation about x: (x, y, z) <- (x, z, -y)
                u32 num_vertices = 3 * num_triangles;
                for (u32 i = 0; i < num_vertices; ++i) {
                    real32 tmp = soup[3 * i + 1];
                    soup[3 * i + 1] = soup[3 * i + 2];
                    soup[3 * i + 2] = -tmp;
                }
            }
        }

        u32 num_vertices;
        real32 *vertex_positions;
        u32 *triangle_indices;
        { // merge vertices
            num_vertices = 0;
            Map<vec3, u32> map = {};
            u32 _3_TIMES_num_triangles = 3 * num_triangles;
            {
                List<vec3> list = {};
                for (u32 i = 0; i < _3_TIMES_num_triangles; ++i) {
                    vec3 p = get(soup, i);
                    u32 default_value = _3_TIMES_num_triangles + 1;
                    u32 j = map_get(&map, p, default_value);
                    if (j == default_value) {
                        map_put(&map, p, num_vertices++);
                        list_push_back(&list, p);
                    }
                }
                {
                    u32 size = list.length * sizeof(vec3);
                    vertex_positions = (real32 *) malloc(size);
                    memcpy(vertex_positions, list.data, size);
                }
                list_free(&list);
            }
            triangle_indices = (u32 *) malloc(_3_TIMES_num_triangles * sizeof(u32));
            for (u32 k = 0; k < _3_TIMES_num_triangles; ++k) triangle_indices[k] = map_get(&map, ((vec3 *) soup)[k]);
            map_free(&map);
            free(soup);
        }
        fancy_mesh->num_vertices = num_vertices;
        fancy_mesh->num_triangles = num_triangles;
        fancy_mesh->vertex_positions = vertex_positions;
        fancy_mesh->triangle_indices = triangle_indices;
        fancy_mesh_triangle_normals_calculate(fancy_mesh);
        fancy_mesh_cosmetic_edges_calculate(fancy_mesh);
    }
    { // manifold_manifold
      // FORNOW
        ManifoldMeshGL *meshgl = manifold_meshgl(
                malloc(manifold_meshgl_size()),
                fancy_mesh->vertex_positions,
                fancy_mesh->num_vertices,
                3,
                fancy_mesh->triangle_indices,
                fancy_mesh->num_triangles);
        *manifold_manifold = manifold_of_meshgl(malloc(manifold_manifold_size()), meshgl);
        manifold_delete_meshgl(meshgl);
    }
}