#include "cs345.cpp"
#include "manifoldc.h"
#include "poe.cpp"
#undef real // ???

bool *key_pressed = globals.key_pressed;
bool *key_toggled = globals.key_toggled;

#include "conversation.h"





void conversation_draw_3D_grid(mat4 P_3D, mat4 V_3D) {
    static IndexedTriangleMesh3D grid_box;
    if (grid_box.num_vertices == 0) {
        static int _grid_box_num_vertices = 24;
        static int _grid_box_num_triangles = 12;
        static int3 _grid_box_triangle_indices[] = {
            { 1, 0, 2},{ 2, 0, 3},
            { 4, 5, 6},{ 4, 6, 7},
            { 8, 9,10},{ 8,10,11},
            {13,12,14},{14,12,15},
            {17,16,18},{18,16,19},
            {20,21,22},{20,22,23},
        };
        static vec3 _grid_box_vertex_positions[] = {
            { 1, 1, 1},{ 1, 1,-1},{ 1,-1,-1},{ 1,-1, 1},
            {-1, 1, 1},{-1, 1,-1},{-1,-1,-1},{-1,-1, 1},
            { 1, 1, 1},{ 1, 1,-1},{-1, 1,-1},{-1, 1, 1},
            { 1,-1, 1},{ 1,-1,-1},{-1,-1,-1},{-1,-1, 1},
            { 1, 1, 1},{ 1,-1, 1},{-1,-1, 1},{-1, 1, 1},
            { 1, 1,-1},{ 1,-1,-1},{-1,-1,-1},{-1, 1,-1},
        };
        static vec3 _grid_box_vertex_colors[] = {
            { 0.8f, 0.8f, 0.8f},{ 0.8f, 0.8f,0.4f},{ 0.8f,0.4f,0.4f},{ 0.8f,0.4f, 0.8f},
            {0.4f, 0.8f, 0.8f},{0.4f, 0.8f,0.4f},{0.4f,0.4f,0.4f},{0.4f,0.4f, 0.8f},
            { 0.8f, 0.8f, 0.8f},{ 0.8f, 0.8f,0.4f},{0.4f, 0.8f,0.4f},{0.4f, 0.8f, 0.8f},
            { 0.8f,0.4f, 0.8f},{ 0.8f,0.4f,0.4f},{0.4f,0.4f,0.4f},{0.4f,0.4f, 0.8f},
            { 0.8f, 0.8f, 0.8f},{ 0.8f,0.4f, 0.8f},{0.4f,0.4f, 0.8f},{0.4f, 0.8f, 0.8f},
            { 0.8f, 0.8f,0.4f},{ 0.8f,0.4f,0.4f},{0.4f,0.4f,0.4f},{0.4f, 0.8f,0.4f},
        };
        static vec2 _grid_box_vertex_texCoords[] = {
            {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
            {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
            {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
            {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
            {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
            {0.00,0.00},{0.00,1.00},{1.00,1.00},{1.00,0.00},
        };
        grid_box = {
            _grid_box_num_vertices,
            _grid_box_num_triangles,
            _grid_box_vertex_positions,
            NULL,
            _grid_box_vertex_colors,
            _grid_box_triangle_indices,
            _grid_box_vertex_texCoords
        };

        u32 texture_side_length = 1024;
        u32 number_of_channels = 4;
        u8 *data = (u8 *) malloc(texture_side_length * texture_side_length * number_of_channels * sizeof(u8));
        u32 o = 9;
        for (u32 j = 0; j < texture_side_length; ++j) {
            for (u32 i = 0; i < texture_side_length; ++i) {
                u32 k = number_of_channels * (j * texture_side_length + i);
                u32 n = u32(texture_side_length / GRID_SIDE_LENGTH * 10);
                u32 t = 2;
                bool32 stripe = (((i + o) % n < t) || ((j + o) % n < t));
                u8 value = 0;
                if (stripe) value = 80;
                if (i < t || j < t || i > texture_side_length - t - 1 || j > texture_side_length - t - 1) value = 160;
                for (u32 d = 0; d < 3; ++d) data[k + d] = value;
                data[k + 3] = 160;
            }
        }
        _mesh_texture_create("procedural grid", texture_side_length, texture_side_length, number_of_channels, data);
    }
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    real32 L = GRID_SIDE_LENGTH;
    grid_box.draw(P_3D, V_3D, M4_Translation(0.0f, L / 2 - 2 * Z_FIGHT_EPS, 0.0f) * M4_Scaling(L / 2), {}, "procedural grid");
    glDisable(GL_CULL_FACE);
}

char conversation_message_buffer[256];
u32 conversation_message_cooldown;
void conversation_messagef(char *format, ...) {
    va_list arg;
    va_start(arg, format);
    vsnprintf(conversation_message_buffer, sizeof(conversation_message_buffer), format, arg);
    va_end(arg);
    conversation_message_cooldown = 300;
}




// TODO: could this take a printf function pointer?
void wrapper_manifold(
        ManifoldManifold **manifold_manifold,
        FancyMesh *fancy_mesh, // dest__NOTE_GETS_OVERWRITTEN,
        u32 num_polygonal_loops,
        u32 *num_vertices_in_polygonal_loops,
        vec2 **polygonal_loops,
        mat4 M_3D_from_2D,
        u32 enter_mode,
        real32 console_param,
        real32 console_param_2) {
    // FORNOW: this function call isn't a no-op
    // history_record_state(history, manifold_manifold, fancy_mesh);
    ASSERT(enter_mode != ENTER_MODE_NONE);

    ManifoldManifold *other_manifold; {
        ManifoldSimplePolygon **simple_polygon_array = (ManifoldSimplePolygon **) malloc(num_polygonal_loops * sizeof(ManifoldSimplePolygon *));
        for (u32 i = 0; i < num_polygonal_loops; ++i) {
            simple_polygon_array[i] = manifold_simple_polygon(malloc(manifold_simple_polygon_size()), (ManifoldVec2 *) polygonal_loops[i], num_vertices_in_polygonal_loops[i]);
        }
        ManifoldPolygons *polygons = manifold_polygons(malloc(manifold_polygons_size()), simple_polygon_array, num_polygonal_loops);
        ManifoldCrossSection *cross_section = manifold_cross_section_of_polygons(malloc(manifold_cross_section_size()), polygons, ManifoldFillRule::MANIFOLD_FILL_RULE_EVEN_ODD);


        { // other_manifold

            if (enter_mode == ENTER_MODE_EXTRUDE_CUT) {
                do_once { printf("[hack] inflating ENTER_MODE_EXTRUDE_CUT\n");};
                console_param += SGN(console_param) * TOLERANCE_DEFAULT;
                console_param_2 += SGN(console_param_2) * TOLERANCE_DEFAULT;
            }

            // NOTE: params are arbitrary sign (and can be same sign)--a typical thing would be like (30, -30)
            //       but we support (30, 40) -- which is equivalent to (40, 0)

            if (enter_mode == ENTER_MODE_EXTRUDE_ADD || enter_mode == ENTER_MODE_EXTRUDE_CUT) {
                real32 min = MIN(0.0f, MIN(console_param, console_param_2));
                real32 max = MAX(0.0f, MAX(console_param, console_param_2));
                real32 length = max - min;

                other_manifold = manifold_extrude(malloc(manifold_manifold_size()), cross_section, length, 0, 0.0f, 1.0f, 1.0f);
                other_manifold = manifold_translate(other_manifold, other_manifold, 0.0f, 0.0f, min);
                // if (console_params_preview_flip_flag) other_manifold = manifold_mirror(other_manifold, other_manifold, 0.0f, 0.0f, 1.0f);

            } else {
                // TODO
                other_manifold = manifold_revolve(malloc(manifold_manifold_size()), cross_section, NUM_SEGMENTS_PER_CIRCLE);
                other_manifold = manifold_rotate(other_manifold, other_manifold, -90.0f, 0.0f, 0.0f);
            }
            other_manifold = manifold_transform(other_manifold, other_manifold,
                    M_3D_from_2D(0, 0), M_3D_from_2D(1, 0), M_3D_from_2D(2, 0),
                    M_3D_from_2D(0, 1), M_3D_from_2D(1, 1), M_3D_from_2D(2, 1),
                    M_3D_from_2D(0, 2), M_3D_from_2D(1, 2), M_3D_from_2D(2, 2),
                    M_3D_from_2D(0, 3), M_3D_from_2D(1, 3), M_3D_from_2D(2, 3));
        }
    }

    // add
    if (!(*manifold_manifold)) {
        ASSERT((enter_mode != ENTER_MODE_EXTRUDE_CUT) && (enter_mode != ENTER_MODE_REVOLVE_CUT));

        *manifold_manifold = other_manifold;
    } else {
        // TODO: ? manifold_delete_manifold(manifold_manifold);
        *manifold_manifold =
            manifold_boolean(
                    malloc(manifold_manifold_size()),
                    *manifold_manifold,
                    other_manifold,
                    ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_ADD)) ? ManifoldOpType::MANIFOLD_ADD : ManifoldOpType::MANIFOLD_SUBTRACT
                    );
    }

    ManifoldMeshGL *meshgl = manifold_get_meshgl(malloc(manifold_meshgl_size()), *manifold_manifold);

    // // NOTE: don't free ANYTHING!--putting the current state on the undo stack
    // XXX fancy_mesh_free(fancy_mesh);
    fancy_mesh->num_vertices = manifold_meshgl_num_vert(meshgl);
    fancy_mesh->num_triangles = manifold_meshgl_num_tri(meshgl);
    fancy_mesh->vertex_positions = manifold_meshgl_vert_properties(malloc(manifold_meshgl_vert_properties_length(meshgl) * sizeof(real32)), meshgl);
    fancy_mesh->triangle_indices = manifold_meshgl_tri_verts(malloc(manifold_meshgl_tri_length(meshgl) * sizeof(u32)), meshgl);
    fancy_mesh_triangle_normals_calculate(fancy_mesh);
    fancy_mesh_cosmetic_edges_calculate(fancy_mesh);
}







////////////////////////////////////////////////////
// GLOBALS (TODO: wrap in struct ConversationState { ... } conversation;
////////////////////////////////////////////////////

Camera2D camera2D;
Camera3D camera3D;

bool32 hide_grid, show_details, show_help;


DXF dxf;
DXFLoopAnalysisResult pick;
BoundingBox *bbox;
BoundingBox bbox_union;
bool32 *dxf_selection_mask;
real32 origin_x;
real32 origin_y;


ManifoldManifold *manifold_manifold;
FancyMesh fancy_mesh;
u32 enter_mode;

char console_buffer[256];
char *console_buffer_write_head;
void conversation_console_buffer_reset() {
    memset(console_buffer, 0, ARRAY_LENGTH(console_buffer) * sizeof(char));
    console_buffer_write_head = console_buffer;
};

real32 console_param;
real32 console_param_2;
bool32 console_params_preview_flip_flag;
real32 console_param_preview;
real32 console_param_2_preview;
void console_params_preview_update() {
    if (console_buffer_write_head == console_buffer) {
        if (enter_mode != ENTER_MODE_MOVE_ORIGIN_TO) {
            console_param_preview = console_param;
            console_param_2_preview = console_param_2;
        } else if (globals.mouse_moved) {
            // FORNOW sloppy recomputation of mouse_position_in_world_coordinates
            _input_get_mouse_position_and_change_in_position_in_world_coordinates(camera_get_PV(&camera2D).data, &console_param_preview, &console_param_2_preview, NULL, NULL);
        }
    } else {
        char buffs[2][64] = {};
        u32 buff_i = 0;
        u32 i = 0;
        for (char *c = console_buffer; (*c) != '\0'; ++c) {
            if (*c == ' ') {
                if (++buff_i == 2) break;
                i = 0;
                continue;
            }
            buffs[buff_i][i++] = (*c);
        }
        real32 sign = (!console_params_preview_flip_flag) ? 1.0f : -1.0f;
        console_param_preview = sign * strtof(buffs[0], NULL);
        console_param_2_preview = sign * strtof(buffs[1], NULL);
        if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
            console_param_2_preview *= -1;
        }
    }
}






u32 hot_pane;
u32 click_mode;
u32 click_modifier; // TODO combine
u32 window_select_click_count;
real32 window_select_x;
real32 window_select_y;


bool32 some_triangle_exists_that_matches_n_selected_and_r_n_selected; // NOTE: if this is false, then a plane is selected
vec3 n_selected;
real32 r_n_selected; // coordinate along n_selected
mat4 M_3D_from_2D;
void conversation_feature_plane_reset() {
    some_triangle_exists_that_matches_n_selected_and_r_n_selected = false;
    r_n_selected = 0.0f;
    n_selected = {};
    M_3D_from_2D = {}; // FORNOW??: implicit nothing selected
}
void conversation_update_M_3D_from_2D() {
    vec3 up = { 0.0f, 1.0f, 0.0f };
    real32 dot_product = dot(n_selected, up);
    vec3 y = (ARE_EQUAL(ABS(dot_product), 1.0f)) ? V3(0.0f,  0.0f, -1.0f * SGN(dot_product)) : up;
    vec3 x = normalized(cross(y, n_selected));
    vec3 z = cross(x, y);

    // FORNOW
    if (ARE_EQUAL(ABS(dot_product), 1.0f) && SGN(dot_product) < 0.0f) {
        y *= -1;
    }

    M_3D_from_2D = M4_xyzo(x, y, z, (r_n_selected) * n_selected) * M4_Translation(-origin_x, -origin_y);
}



// GLOBAL-TOUCHING FUNCTIONS ///////////////////////////

// TODO: see which of these can be easily pulled out into non-global touching

void conversation_load_dxf(char *filename) {
    if (!poe_file_exists(filename)) {
        conversation_messagef("[load] \"%s\" not found", filename);
        return;
    }
    {
        dxf_free(&dxf);
        dxf_loop_analysis_free(&pick);
        free(dxf_selection_mask);
        free(bbox);

        dxf_load(filename, &dxf);
        dxf_selection_mask = (bool32 *) calloc(dxf.num_entities, sizeof(bool32));
        pick = dxf_loop_analysis_create(&dxf);
        bbox = dxf_entity_bounding_boxes_create(&dxf);
        bbox_union = bounding_box_union(dxf.num_entities, bbox);

        camera2D_zoom_to_bounding_box(&camera2D, bbox_union);
        origin_x = 0.0f;
        origin_y = 0.0f;

        conversation_messagef("[load] loaded %s", filename);
    }
}
void conversation_load_stl(char *filename) {
    if (!poe_file_exists(filename)) {
        conversation_messagef("[load] \"%s\" not found", filename);
        return;
    }
    {
        stl_load(filename, &manifold_manifold, &fancy_mesh);
        conversation_messagef("[load] loaded %s", filename);
    }
}
void conversation_load_file(char *filename) {
    if (poe_suffix_match(filename, ".dxf")) {
        conversation_load_dxf(filename);
    } else if (poe_suffix_match(filename, ".stl")) {
        conversation_load_stl(filename);
    } else {
        conversation_messagef("[load] \"%s\" fileype not supported; must be *.dxf or *.stl", filename);
    }
}
void conversation_save_stl(char *filename) {
    // TODO: prompt for overwriting
    if (fancy_mesh_save_stl(&fancy_mesh, filename) ) {
        conversation_messagef("[save] saved %s", filename);
    } else {
        conversation_messagef("[save] could not save open %s for writing.", filename);
    }
}
void conversation_save_file(char *filename) {
    if (poe_suffix_match(filename, ".stl")) {
    } else {
        conversation_messagef("[save] \"%s\" filetype not supported; must be *.stl", filename);
    }
}


char conversation_drop_path[512];
void drop_callback(GLFWwindow *, int count, const char **paths) {
    if (count > 0) {
        char *filename = (char *) paths[0];
        conversation_load_file(filename);
        { // conversation_set_drop_path(filename)
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
        }
    }
}
BEGIN_PRE_MAIN {
    glfwSetDropCallback(COW0._window_glfw_window, drop_callback);
} END_PRE_MAIN;



void conversation_cameras_reset() {
    camera2D = { 0.0f, 0.0, 0.0f, -0.5f, -0.25f };
    camera2D_zoom_to_bounding_box(&camera2D, bbox_union);
    camera3D = { 2.0f * camera2D.screen_height_World, CAMERA_3D_DEFAULT_ANGLE_OF_VIEW, RAD(33.0f), RAD(-44.0f), 0.0f, 0.0f, 0.5f, -0.25f };
}

void conversation_reset() {
    hot_pane = HOT_PANE_NONE;
    click_mode = CLICK_MODE_NONE;
    click_modifier = CLICK_MODIFIER_NONE;
    enter_mode = ENTER_MODE_NONE;


    manifold_manifold = NULL;
    fancy_mesh = {};

    console_param = 0.0f;
    console_param_2 = 0.0f;
    console_params_preview_flip_flag = false;

    hide_grid = false;
    show_details = false;
    show_help = false;

    conversation_feature_plane_reset(); 
    conversation_load_dxf("splash.dxf");
    conversation_cameras_reset();
    conversation_console_buffer_reset();
}


void conversation_draw(mat4 PV_2D, mat4 PV_3D, mat4 P_3D, mat4 V_3D, real32 mouse_x, real32 mouse_y, bool32 dxf_anything_selected, bool32 stl_plane_selected) { // draw
    u32 window_width, window_height; {
        real32 _window_width, _window_height; // FORNOW
        _window_get_size(&_window_width, &_window_height);
        window_width = (u32) _window_width;
        window_height = (u32) _window_height;
    }

    { // panes
        { // draw
          // glDisable(GL_DEPTH_TEST);
          // eso_begin(globals.Identity, SOUP_QUADS);
          // eso_color(0.3f, 0.3f, 0.3f);
          // eso_vertex(0.0f,  1.0f);
          // eso_vertex(0.0f, -1.0f);
          // eso_vertex(1.0f, -1.0f);
          // eso_vertex(1.0f,  1.0f);
          // eso_end();
          // glEnable(GL_DEPTH_TEST);

            eso_begin(globals.Identity, SOUP_LINES, 5.0f, true);
            eso_color(136 / 255.0f, 136 / 255.0f, 136 / 255.0f);
            eso_vertex(0.0f,  1.0f);
            eso_vertex(0.0f, -1.0f);
            eso_end();
        }

        if ((!globals.mouse_left_held && !globals.mouse_right_held) || globals.mouse_left_pressed || globals.mouse_right_pressed) {
            hot_pane = (globals.mouse_position_NDC.x <= 0.0f) ? HOT_PANE_2D : HOT_PANE_3D;
            if ((click_modifier == CLICK_MODIFIER_WINDOW) && (window_select_click_count == 1)) hot_pane = HOT_PANE_2D;// FORNOW
        }
    }

    { // draw 2D draw 2d draw
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, 0, window_width / 2, window_height);
        {
            if (!hide_grid) { // grid 2D grid 2d grid
                eso_begin(PV_2D, SOUP_LINES, 2.0f);
                eso_color(80.0f / 255, 80.0f / 255, 80.0f / 255);
                for (u32 i = 0; i <= u32(GRID_SIDE_LENGTH / GRID_SPACING); ++i) {
                    real32 tmp = i * GRID_SPACING;
                    eso_vertex(tmp, 0.0f);
                    eso_vertex(tmp, GRID_SIDE_LENGTH);
                    eso_vertex(0.0f, tmp);
                    eso_vertex(GRID_SIDE_LENGTH, tmp);
                }
                eso_end();
                eso_begin(PV_2D, SOUP_LINE_LOOP, 2.0f);
                eso_vertex(0.0f, 0.0f);
                eso_vertex(0.0f, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, 0.0f);
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
            { // axes 2D axes 2d axes axis 2D axis 2d axes crosshairs cross hairs origin 2d origin 2D origin
                real32 r = camera2D.screen_height_World / 120.0f;
                mat4 M = M4_Translation(origin_x, origin_y);
                vec3 color = V3(0.8f, 0.8f, 1.0f);
                if (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) {
                    M = M4_Translation(console_param_preview, console_param_2_preview);
                    color = V3(0.0f, 1.0f, 1.0f);
                }
                eso_begin(PV_2D * M, SOUP_LINES, 3.0f);
                eso_color(color);
                eso_vertex(-r*.7f, 0.0f);
                eso_vertex( r, 0.0f);
                eso_vertex(0.0f, -r);
                eso_vertex(0.0f,  r*.7f);
                eso_end();
            }
            if (click_modifier == CLICK_MODIFIER_WINDOW) { // select window
                if (window_select_click_count == 1) {
                    eso_begin(PV_2D, SOUP_LINE_LOOP, 2.0f);
                    eso_color(0.0f, 1.0f, 1.0f);
                    real32 x0 = window_select_x;
                    real32 y0 = window_select_y;
                    real32 x1 = mouse_x;
                    real32 y1 = mouse_y;
                    eso_vertex(x0, y0);
                    eso_vertex(x1, y0);
                    eso_vertex(x1, y1);
                    eso_vertex(x0, y1);
                    eso_end();
                }
            }
        }
        glDisable(GL_SCISSOR_TEST);
    }

    { // 3D draw 3D 3d draw 3d
        glEnable(GL_SCISSOR_TEST);
        glScissor(window_width / 2, 0, window_width / 2, window_height);

        { // selection 2d selection 2D selection (FORNOW: ew)
            u32 color = ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_ADD)) ? DXF_COLOR_TRAVERSE : ((enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) ? DXF_COLOR_QUALITY_1 : ((enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) ? DXF_COLOR_WATER_ONLY : DXF_COLOR_SELECTION;

            u32 NUM_TUBE_STACKS_INCLUSIVE;
            mat4 M;
            mat4 M_incr;
            if (true) {
                if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                    // NOTE: some repetition with wrapper
                    real32 a = MIN(0.0f, MIN(console_param_preview, console_param_2_preview));
                    real32 b = MAX(0.0f, MAX(console_param_preview, console_param_2_preview));
                    real32 length = b - a;
                    NUM_TUBE_STACKS_INCLUSIVE = MIN(64, u32(roundf(length / 2.5f)) + 2);
                    M = M_3D_from_2D * M4_Translation(0.0f, 0.0f, a + Z_FIGHT_EPS);
                    M_incr = M4_Translation(0.0f, 0.0f, (b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                } else if ((enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
                    NUM_TUBE_STACKS_INCLUSIVE = NUM_SEGMENTS_PER_CIRCLE;
                    M = M_3D_from_2D;
                    real32 a = 0.0f;
                    real32 b = TAU;
                    M_incr = M4_RotationAboutYAxis((b - a) / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                } else if (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) {
                    // FORNOW
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * M4_Translation((origin_x - console_param_preview), (origin_y - console_param_2_preview));
                    M_incr = M4_Identity();
                } else if (enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * M4_Translation(0.0f, 0.0f, console_param_preview + Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * M4_Translation(0.0f, 0.0f, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                }
                for (u32 tube_stack_index = 0; tube_stack_index < NUM_TUBE_STACKS_INCLUSIVE; ++tube_stack_index) {
                    {
                        eso_begin(PV_3D * M, SOUP_LINES, 5.0f);
                        for (u32 i = 0; i < dxf.num_entities; ++i) {
                            DXFEntity *entity = &dxf.entities[i];
                            if (dxf_selection_mask[i]) {
                                eso_dxf_entity__SOUP_LINES(entity, color);
                            }
                        }
                        eso_end();
                    }
                    M *= M_incr;
                }
            }
        }


        if (dxf_anything_selected) { // arrow
            if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
                real32 H[2] = { console_param_preview, console_param_2_preview };
                bool32 toggle[2] = { console_params_preview_flip_flag, !console_params_preview_flip_flag };
                mat4 R2 = M4_Identity();
                if ((enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
                    H[0] = 50.0f;
                    H[1] = 0.0f;
                    R2 = M4_RotationAboutXAxis(RAD(-90.0f));
                }
                mat4 R = M4_RotationAboutXAxis(RAD(90.0f));
                vec3 color = ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_ADD)) ? V3(83.0f / 255, 255.0f / 255, 83.0f / 255.0f) : V3(1.0f, 0.0f, 0.0f);
                for (u32 i = 0; i < 2; ++i) {
                    if (!IS_ZERO(H[i])) {
                        real32 total_height = ABS(H[i]);
                        real32 cap_height = (total_height > 10.0f) ? 5.0f : (0.5f * total_height);
                        real32 shaft_height = total_height - cap_height;
                        real32 s = 1.5f;
                        mat4 N = M4_Translation(0.0, 0.0, -Z_FIGHT_EPS);
                        if (toggle[i]) N = M4_Scaling(1.0f, 1.0f, -1.0f) * N;
                        mat4 M_cyl  = M_3D_from_2D * R2 * N * M4_Scaling(s * 1.0f, s * 1.0f, shaft_height) * R;
                        mat4 M_cone = M_3D_from_2D * R2 * N * M4_Translation(0.0f, 0.0f, shaft_height) * M4_Scaling(s * 2.0f, s * 2.0f, cap_height) * R;
                        library.meshes.cylinder.draw(P_3D, V_3D, M_cyl, color);
                        library.meshes.cone.draw(P_3D, V_3D, M_cone, color);
                    }
                }
            }
        }

        if (stl_plane_selected) { // axes 3D axes 3d axes axis 3D axis 3d axis
            real32 r = camera3D.ortho_screen_height_World / 120.0f;
            eso_begin(PV_3D * M_3D_from_2D * M4_Translation(origin_x, origin_y, Z_FIGHT_EPS), SOUP_LINES, 4.0f);
            eso_color(0.8f, 0.8f, 1.0f);
            eso_vertex(-r*0.6f, 0.0f);
            eso_vertex(r, 0.0f);
            eso_vertex(0.0f, -r);
            eso_vertex(0.0f, r*0.6f);
            eso_end();
        }


        { // fancy_mesh; NOTE: includes transparency
            if (fancy_mesh.cosmetic_edges) {
                eso_begin(PV_3D, SOUP_LINES, 3.0f); 
                eso_color(monokai.black);
                // 3 * num_triangles * 2 / 2
                for (u32 k = 0; k < 2 * fancy_mesh.num_cosmetic_edges; ++k) eso_vertex(fancy_mesh.vertex_positions, fancy_mesh.cosmetic_edges[k]);
                eso_end();
            }
            for (u32 pass = 0; pass <= 1; ++pass) {
                eso_begin(PV_3D, (!show_details) ? SOUP_TRIANGLES : SOUP_OUTLINED_TRIANGLES);

                mat3 inv_transpose_V_3D = inverse(transpose(M3(V_3D(0, 0), V_3D(0, 1), V_3D(0, 2), V_3D(1, 0), V_3D(1, 1), V_3D(1, 2), V_3D(2, 0), V_3D(2, 1), V_3D(2, 2))));

                for (u32 i = 0; i < fancy_mesh.num_triangles; ++i) {
                    #if 1
                    vec3 n = get(fancy_mesh.triangle_normals, i);
                    vec3 p[3];
                    real32 x_n;
                    {
                        for (u32 j = 0; j < 3; ++j) p[j] = get(fancy_mesh.vertex_positions, fancy_mesh.triangle_indices[3 * i + j]);
                        x_n = dot(n, p[0]);
                    }
                    vec3 color; 
                    real32 alpha;
                    {
                        vec3 n_camera = inv_transpose_V_3D * n;
                        vec3 color_n = V3(0.5f + 0.5f * n_camera.x, 0.5f + 0.5f * n_camera.y, 1.0f);
                        if (some_triangle_exists_that_matches_n_selected_and_r_n_selected && (dot(n, n_selected) > 0.999f) && (ABS(x_n - r_n_selected) < 0.001f)) {
                            if (pass == 0) continue;
                            color = V3(0.85f, 0.87f, 0.30f);
                            alpha = ((enter_mode == ENTER_MODE_EXTRUDE_ADD || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) && ((console_params_preview_flip_flag) || (console_param_2_preview != 0.0f))) ? 0.7f : 1.0f;
                        } else {
                            if (pass == 1) continue;
                            color = color_n;
                            alpha = 1.0f;
                        }
                    }
                    eso_color(color, alpha);
                    eso_vertex(p[0]);
                    eso_vertex(p[1]);
                    eso_vertex(p[2]);
                    #else
                    eso_color(monokai.green, 1.0f);
                    vec3 p[3];
                    {
                        for (u32 j = 0; j < 3; ++j) p[j] = get(fancy_mesh.vertex_positions, fancy_mesh.triangle_indices[3 * i + j]);
                    }
                    eso_vertex(p[0]);
                    eso_vertex(p[1]);
                    eso_vertex(p[2]);
                    #endif

                }
                eso_end();
            }
        }

        if (!hide_grid) { // grid 3D grid 3d grid
            conversation_draw_3D_grid(P_3D, V_3D);
        }

        { // floating sketch plane; NOTE: transparent
            bool draw = (!some_triangle_exists_that_matches_n_selected_and_r_n_selected);
            mat4 PVM = PV_3D * M_3D_from_2D * M4_Translation(origin_x, origin_y); // FORNOW
            vec3 color = monokai.yellow;
            real32 sign = -1.0f;
            if (enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                PVM *= M4_Translation(0.0f, 0.0f, console_param_preview);
                color = { 0.0f, 1.0f, 1.0f };
                sign = 1.0f;
                draw = true;
            } else if (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) {
                PVM *= M4_Translation(-console_param_preview - origin_x, -console_param_2_preview - origin_y);
                color = { 0.0f, 1.0f, 1.0f };
                sign = 1.0f;
                draw = true;
            }
            real32 r = 30.0f;
            BoundingBox bounding_box = { -r, -r, r, r };
            if (dxf_anything_selected) {
                bounding_box = bounding_box_union(dxf.num_entities, bbox, dxf_selection_mask);
                {
                    bounding_box.min[0] -= origin_x;
                    bounding_box.max[0] -= origin_x;
                    bounding_box.min[1] -= origin_y;
                    bounding_box.max[1] -= origin_y;
                }
                for (u32 d = 0; d < 2; ++d) {
                    bounding_box.min[d] /= GRID_SPACING;
                    bounding_box.min[d] -= 1.0;
                    bounding_box.min[d] = floorf(bounding_box.min[d]);
                    bounding_box.min[d] *= GRID_SPACING;

                    bounding_box.max[d] /= GRID_SPACING;
                    bounding_box.max[d] += 1.0;
                    bounding_box.max[d] = ceilf(bounding_box.max[d]);
                    bounding_box.max[d] *= GRID_SPACING;

                }
            }
            if (draw) {
                eso_begin(PVM, SOUP_OUTLINED_QUADS);
                eso_color(color, 0.35f);
                eso_vertex(bounding_box.min[0], bounding_box.min[1], sign * Z_FIGHT_EPS);
                eso_vertex(bounding_box.min[0], bounding_box.max[1], sign * Z_FIGHT_EPS);
                eso_vertex(bounding_box.max[0], bounding_box.max[1], sign * Z_FIGHT_EPS);
                eso_vertex(bounding_box.max[0], bounding_box.min[1], sign * Z_FIGHT_EPS);
                eso_end();
            }
        }

        glDisable(GL_SCISSOR_TEST);
    }

    { // gui
        gui_printf("[Click] %s %s",
                (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) ? "MOVE_2D_ORIGIN_TO" :
                (click_mode == CLICK_MODE_SELECT) ? "SELECT" :
                (click_mode == CLICK_MODE_DESELECT) ? "DESELECT" :
                "NONE",
                (click_modifier == CLICK_MODE_NONE) ? "" :
                (click_modifier == CLICK_MODIFIER_CONNECTED) ? "CONNECTED" :
                (click_modifier == CLICK_MODIFIER_WINDOW) ? "WINDOW" :
                (click_modifier == CLICK_MODIFIER_CENTER_OF) ? "CENTER_OF" :
                (click_modifier == CLICK_MODIFIER_END_OF) ? "END_OF" :
                (click_modifier == CLICK_MODIFIER_MIDDLE_OF) ? "MIDDLE_OF" :
                "");

        char enter_message[256] = {};
        if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO)) {
            real32 p, p2;
            char glyph, glyph2;
            if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) {
                p      =  console_param_preview;
                p2     = -console_param_2_preview;
                if (console_params_preview_flip_flag) { // ??
                    p *= -1;
                    p2 *= -1;
                }
                if (IS_ZERO(p)) p = 0.0f; // FORNOW makes minus sign go away in hud (not a big deal)
                if (IS_ZERO(p2)) p2 = 0.0f; // FORNOW makes minus sign go away in hud (not a big deal)
                glyph  = (!console_params_preview_flip_flag) ? '^' : 'v';
                glyph2 = (!console_params_preview_flip_flag) ? 'v' : '^';
            } else {
                ASSERT(enter_mode == ENTER_MODE_MOVE_ORIGIN_TO);
                p      = console_param_preview;
                p2     = console_param_2_preview;
                glyph  = 'x';
                glyph2 = 'y';
            }
            sprintf(enter_message, "%c:%gmm %c:%gmm", glyph, p, glyph2, p2);
            if (((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT)) && IS_ZERO(console_param_2_preview)) sprintf(enter_message, "%c:%gmm", glyph, p);
        } else if ((enter_mode == ENTER_MODE_LOAD) || (enter_mode == ENTER_MODE_SAVE)) {
            sprintf(enter_message, "%s%s", conversation_drop_path, console_buffer);
        }

        gui_printf("[Enter] %s %s",
                (enter_mode == ENTER_MODE_EXTRUDE_ADD) ? "EXTRUDE_ADD" :
                (enter_mode == ENTER_MODE_EXTRUDE_CUT) ? "EXTRUDE_CUT" :
                (enter_mode == ENTER_MODE_REVOLVE_ADD) ? "REVOLVE_ADD" :
                (enter_mode == ENTER_MODE_REVOLVE_CUT) ? "REVOLVE_CUT" :
                (enter_mode == ENTER_MODE_LOAD) ? "LOAD" :
                (enter_mode == ENTER_MODE_SAVE) ? "SAVE" :
                (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) ? "MOVE_2D_ORIGIN_TO" :
                (enter_mode == ENTER_MODE_OFFSET_PLANE_BY) ? "OFFSET_PLANE_TO" :
                "NONE",
                enter_message);

        if ((enter_mode == ENTER_MODE_NONE) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
            gui_printf("> %s", console_buffer);
        } else {
            gui_printf("> %s", console_buffer);
        }

        {
            if (conversation_message_cooldown > 0) {
                --conversation_message_cooldown;
            } else {
                conversation_message_buffer[0] = '\0';
            }

            gui_printf("< %s", conversation_message_buffer);
        }

        if (show_details) {
            gui_printf("%d dxf elements", dxf.num_entities);
            gui_printf("%d stl triangles", fancy_mesh.num_triangles);
        }
        if (show_help) {
            { // overlay
                eso_begin(M4_Identity(), SOUP_QUADS, 0.0f, true);
                eso_color(0.0f, 0.0f, 0.0f, 0.7f);
                eso_vertex(-1.0f, -1.0f);
                eso_vertex(-1.0f,  1.0f);
                eso_vertex( 1.0f,  1.0f);
                eso_vertex( 1.0f, -1.0f);
                eso_end();
            }
            gui_printf("(h)elp-show/hide");
            gui_printf("(Escape)-from-current-enter_and_click_modes");
            gui_printf("(s)elect (d)eselect (c)onnected + (a)ll (q)uality + (0-5)");
            gui_printf("(y)-plane (x)-plane");
            gui_printf("(e)trude-add (E)xtrude-cut + (0123456789. ) (f)lip-direction");
            gui_printf("(r)evolve-add (R)evolve-cut");
            gui_printf("(u)ndo (U)-redo");
            gui_printf("(L)oad (S)ave");
            gui_printf("(g)rid (.)-show-details");
            gui_printf("zoom-to-e(X)tents");
            gui_printf("(Tab)-orthographic-perspective-view");
            gui_printf("(m)ove-origin + (c)enter-of (e)nd-of (0123456789.- ) ");
            gui_printf("");
            gui_printf("you can drag and drop dxf's into Conversation");
        }
    }
}



#define USER_INPUT_EVENT_TYPE_KEY_NONE 0
#define USER_INPUT_EVENT_TYPE_KEY_DOWN 1
#define USER_INPUT_EVENT_TYPE_KEY_UP   2
struct UserInputEvent {
    u32 type;
    u32 key;
    real32 mouse_x;
    real32 mouse_y;
};



// / / / / / / / / / / / / / / / / # # # # # # # . . . . . .
// ^                               ^             ^
// |                               |             |
// buffer                          next          one-past-end

UserInputEvent ui_history_buffer[999999];
UserInputEvent *ui_history_next_unprocessed = ui_history_buffer;
UserInputEvent *ui_history_one_past_end = ui_history_buffer;

void ui_history_buffer_push_back(UserInputEvent event) {
    ASSERT(ui_history_one_past_end >= ui_history_buffer);
    ASSERT((unsigned long) (ui_history_one_past_end - ui_history_buffer) < ARRAY_LENGTH(ui_history_buffer));
    *ui_history_one_past_end++ = event;
}

void ui_event_process(UserInputEvent user_input_event) {
    printf("%s %c\n",
            (user_input_event.type == USER_INPUT_EVENT_TYPE_KEY_DOWN) ? "KEY_DOWN" : 
            (user_input_event.type == USER_INPUT_EVENT_TYPE_KEY_UP) ? "KEY_UP" :
            "NONE",
            char(0) + user_input_event.key);
}

void ui_history_process_backlog() {
    for (; ui_history_next_unprocessed < ui_history_one_past_end; ++ui_history_next_unprocessed)
        ui_event_process(*ui_history_next_unprocessed);
}



void callback_key(GLFWwindow *, int key, int, int action, int mods) {
    // ui_history_buffer_push_back({ ... });
}

void callback_cursor_position(GLFWwindow *, double _xpos, double _ypos) {
    ;
}

void callback_mouse_button(GLFWwindow *, int button, int action, int) {
    ;
}

void callback_scroll(GLFWwindow *, double, double _yoffset) {
    ;
}

BEGIN_PRE_MAIN {
    ;
    ;
    ;
    ;
} END_PRE_MAIN;


int main() {
    conversation_messagef("type h for help // pre-alpha " __DATE__ " " __TIME__);
    bool32 reset = true;
    while (cow_begin_frame()) {
        if (reset) {
            reset = false;
            conversation_reset();
        }


        { // user input
            { // record_user_input_event
                ;


                // TODO: populate the ui_history_list list in a callback
                // TODO: attempt to switch over to ui_event_process()
                // TODO: see if the list just works out
                // TODO: we need to stop using cow's key_pressed key_released for this
                // TODO: handle multiple events between frames (can probably punt this for a while)


            }
            { // keyboard and mouse
              // ui_event_process();
              // computed bool32's (FORNOW: sloppy--these change mid-frame)
                bool32 dxf_anything_selected;
                bool32 stl_plane_selected;
                {
                    dxf_anything_selected = false;
                    for (u32 i = 0; i < dxf.num_entities; ++i) {
                        if (dxf_selection_mask[i]) {
                            dxf_anything_selected = true;
                            break;
                        }
                    }

                    stl_plane_selected = !IS_ZERO(squaredNorm(n_selected));
                }

                { // keyboard input
                    if (globals.key_any_key_pressed) {
                        bool32 key_eaten_by_special__NOTE_dealt_with_up_top;
                        {
                            key_eaten_by_special__NOTE_dealt_with_up_top = false;
                            if (click_modifier == CLICK_MODIFIER_QUALITY) {
                                for (u32 color = 0; color < 6; ++color) {
                                    if (key_pressed['0' + color]) {
                                        key_eaten_by_special__NOTE_dealt_with_up_top = true;
                                        for (u32 i = 0; i < dxf.num_entities; ++i) {
                                            if (dxf.entities[i].color == color) {
                                                bool32 value_to_write_to_selection_mask = (click_mode == CLICK_MODE_SELECT);
                                                dxf_selection_mask[i] = value_to_write_to_selection_mask;
                                            }
                                        }
                                        click_modifier = CLICK_MODIFIER_NONE;
                                        break;
                                    }
                                }
                            }
                        }
                        bool32 send_key_to_console;
                        {
                            send_key_to_console = false;
                            send_key_to_console = (send_key_to_console || key_pressed[COW_KEY_BACKSPACE]);
                            if (!key_pressed[COW_KEY_ENTER]) {
                                if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) {
                                    send_key_to_console = (send_key_to_console || key_pressed['.']);
                                    send_key_to_console = (send_key_to_console || key_pressed['-']);
                                    for (u32 i = 0; i < 10; ++i) send_key_to_console = (send_key_to_console || key_pressed['0' + i]);
                                    // note: double negative
                                    if ((enter_mode != ENTER_MODE_OFFSET_PLANE_BY)) {
                                        send_key_to_console = (send_key_to_console || key_pressed[' ']);
                                    }
                                } else if ((enter_mode == ENTER_MODE_LOAD) || (enter_mode == ENTER_MODE_SAVE)) {
                                    send_key_to_console = (send_key_to_console || key_pressed['.']);
                                    send_key_to_console = (send_key_to_console || key_pressed[' ']);
                                    send_key_to_console = (send_key_to_console || key_pressed['-']);
                                    send_key_to_console = (send_key_to_console || key_pressed['/']);
                                    send_key_to_console = (send_key_to_console || key_pressed['\\']);
                                    for (u32 i = 0; i < 10; ++i) send_key_to_console = (send_key_to_console || key_pressed['0' + i]);
                                    for (u32 i = 0; i < 26; ++i) send_key_to_console = (send_key_to_console || key_pressed['a' + i]);
                                    for (u32 i = 0; i < 26; ++i) send_key_to_console = (send_key_to_console || key_pressed['A' + i]);
                                }
                            } else {
                                send_key_to_console = true;
                                {
                                    bool32 extrude = ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT));
                                    bool32 revolve = ((enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT));
                                    bool32 cut = ((enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_CUT));
                                    if (enter_mode == ENTER_MODE_NONE) {
                                        conversation_messagef("[enter] enter mode is none");
                                        send_key_to_console = false;
                                    } else if (extrude || revolve) {
                                        if (!dxf_anything_selected) {
                                            conversation_messagef("[enter] no dxf elements selected");
                                            send_key_to_console = false;
                                        } else if (!stl_plane_selected) { // FORNOW???
                                            conversation_messagef("[enter] no plane selected");
                                            send_key_to_console = false;
                                        } else if (cut && (fancy_mesh.num_triangles == 0)) { // FORNOW
                                            conversation_messagef("[enter] nothing to cut");
                                            send_key_to_console = false;
                                        } else if (extrude) {
                                            if (IS_ZERO(console_param_preview) && IS_ZERO(console_param_2_preview)) {
                                                conversation_messagef("[enter] extrude height is zero");
                                                send_key_to_console = false;
                                            }
                                        } else {
                                            ASSERT(revolve);
                                            ;
                                        }
                                    } else if (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) {
                                        ;
                                    } else if (enter_mode == ENTER_MODE_OFFSET_PLANE_BY) {
                                        ;
                                    }
                                }
                            }
                        }

                        if (key_eaten_by_special__NOTE_dealt_with_up_top) {
                            ;
                        } else if (send_key_to_console) {
                            if (!key_pressed[COW_KEY_ENTER] ) {
                                if (key_pressed[COW_KEY_BACKSPACE]) {
                                    *--console_buffer_write_head = 0;
                                } else {
                                    char c = (char) globals.key_last_key_pressed;
                                    if (globals.key_shift_held && key_pressed['-']) c = '_';
                                    *console_buffer_write_head++ = c;
                                }
                            } else {
                                if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT) || (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) {
                                    if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) || (enter_mode == ENTER_MODE_OFFSET_PLANE_BY)) {
                                        if (console_buffer_write_head != console_buffer) {
                                            console_param = console_param_preview;
                                            console_param_2 = console_param_2_preview;
                                            conversation_console_buffer_reset();
                                        }
                                    }
                                    if ((enter_mode == ENTER_MODE_EXTRUDE_ADD) || (enter_mode == ENTER_MODE_EXTRUDE_CUT) || (enter_mode == ENTER_MODE_REVOLVE_ADD) || (enter_mode == ENTER_MODE_REVOLVE_CUT)) {
                                        CrossSectionEvenOdd cross_section = cross_section_create(&dxf, dxf_selection_mask);
                                        // cross_section_debug_draw(&camera2D, &cross_section);
                                        wrapper_manifold(
                                                &manifold_manifold,
                                                &fancy_mesh,
                                                cross_section.num_polygonal_loops,
                                                cross_section.num_vertices_in_polygonal_loops,
                                                cross_section.polygonal_loops,
                                                M_3D_from_2D,
                                                enter_mode,
                                                console_param,
                                                console_param_2);
                                        // reset state
                                        memset(dxf_selection_mask, 0, dxf.num_entities * sizeof(bool32));
                                        conversation_feature_plane_reset();
                                    } else if (enter_mode == ENTER_MODE_MOVE_ORIGIN_TO) {
                                        origin_x = console_param;
                                        origin_y = console_param_2;
                                        conversation_update_M_3D_from_2D();
                                    } else {
                                        ASSERT(enter_mode == ENTER_MODE_OFFSET_PLANE_BY);
                                        if (!IS_ZERO(console_param)) {
                                            r_n_selected += console_param;
                                            some_triangle_exists_that_matches_n_selected_and_r_n_selected = false; // FORNOW
                                            conversation_update_M_3D_from_2D();
                                        }
                                    }
                                    console_param = 0.0f;
                                    console_param_2 = 0.0f;
                                } else {
                                    ASSERT((enter_mode == ENTER_MODE_LOAD) || (enter_mode == ENTER_MODE_SAVE));
                                    static char full_filename_including_path[512];
                                    sprintf(full_filename_including_path, "%s%s", conversation_drop_path, console_buffer);
                                    if (enter_mode == ENTER_MODE_LOAD) {
                                        conversation_load_file(full_filename_including_path);
                                    } else {
                                        ASSERT(enter_mode == ENTER_MODE_SAVE);
                                        conversation_save_file(full_filename_including_path);
                                    }
                                }
                                conversation_console_buffer_reset();
                                enter_mode = ENTER_MODE_NONE;
                                if (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) {
                                    click_mode = CLICK_MODE_NONE;
                                    click_modifier = CLICK_MODIFIER_NONE;
                                }
                            }
                        } else if (key_pressed[COW_KEY_TAB]) {
                            camera3D.angle_of_view = (IS_ZERO(camera3D.angle_of_view)) ? CAMERA_3D_DEFAULT_ANGLE_OF_VIEW : 0.0f;
                        } else if (key_pressed[COW_KEY_ESCAPE]) {
                            enter_mode = ENTER_MODE_NONE;
                            click_mode = CLICK_MODE_NONE;
                            click_modifier = CLICK_MODIFIER_NONE;
                            conversation_console_buffer_reset();
                            conversation_feature_plane_reset(); // FORNOW
                        } else if (key_pressed['X'] && globals.key_shift_held) {
                            camera2D_zoom_to_bounding_box(&camera2D, bbox_union);
                        } else if (key_pressed['n']) {
                            if (stl_plane_selected) {
                                enter_mode = ENTER_MODE_OFFSET_PLANE_BY;
                                console_params_preview_flip_flag = false;
                                conversation_console_buffer_reset();
                            } else {
                                conversation_messagef("[n] no plane selected");
                            }
                        } else if (key_pressed['m']) {
                            click_mode = CLICK_MODE_MOVE_2D_ORIGIN_TO;
                            click_modifier = CLICK_MODIFIER_NONE;
                            enter_mode = ENTER_MODE_MOVE_ORIGIN_TO;
                            console_params_preview_flip_flag = false;
                            conversation_console_buffer_reset();
                        } else if (key_pressed['S'] && globals.key_shift_held) {
                            enter_mode = ENTER_MODE_SAVE;
                        } else if (key_pressed['g']) {
                            hide_grid = !hide_grid;
                        } else if (key_pressed['h']) {
                            show_help = !show_help;
                        } else if (key_pressed['.'] && !send_key_to_console) {
                            show_details = !show_details;
                        } else if (key_pressed['s']) {
                            click_mode = CLICK_MODE_SELECT;
                            click_modifier = CLICK_MODIFIER_NONE;
                        } else if (key_pressed['w']) {
                            if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                                click_modifier = CLICK_MODIFIER_WINDOW;
                                window_select_click_count = 0;
                            }
                        } else if (key_pressed['d']) {
                            click_mode = CLICK_MODE_DESELECT;
                            click_modifier = CLICK_MODIFIER_NONE;
                        } else if (key_pressed['c']) {
                            if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                                click_modifier = CLICK_MODIFIER_CONNECTED;
                            } else if (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) {
                                click_modifier = CLICK_MODIFIER_CENTER_OF;
                            }
                        } else if (key_pressed['q']) {
                            if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                                click_modifier = CLICK_MODIFIER_QUALITY;
                            }
                        } else if (key_pressed['a']) {
                            if ((click_mode == CLICK_MODE_SELECT) || (click_mode == CLICK_MODE_DESELECT)) {
                                bool32 value_to_write_to_selection_mask = (click_mode == CLICK_MODE_SELECT);
                                for (u32 i = 0; i < dxf.num_entities; ++i) dxf_selection_mask[i] = value_to_write_to_selection_mask;
                            }
                        } else if (key_pressed['x'] || key_pressed['y'] || key_pressed['z']) {
                            some_triangle_exists_that_matches_n_selected_and_r_n_selected = false;
                            r_n_selected = 0.0f;
                            if (key_pressed['x']) n_selected = { 1.0f, 0.0f, 0.0f };
                            if (key_pressed['y']) n_selected = { 0.0f, 1.0f, 0.0f };
                            if (key_pressed['z']) n_selected = { 0.0f, 0.0f, 1.0f };
                            conversation_update_M_3D_from_2D();
                        } else if (key_pressed['E'] && globals.key_shift_held) {
                            enter_mode = ENTER_MODE_EXTRUDE_CUT;
                            console_params_preview_flip_flag = true;
                            conversation_console_buffer_reset();
                        } else if (key_pressed['e']) {
                            if (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) {
                                click_modifier = CLICK_MODIFIER_END_OF;
                            } else {
                                enter_mode = ENTER_MODE_EXTRUDE_ADD;
                                console_params_preview_flip_flag = false;
                                conversation_console_buffer_reset();
                            }
                        } else if (key_pressed['R'] && globals.key_shift_held) {
                            enter_mode = ENTER_MODE_REVOLVE_CUT;
                        } else if (key_pressed['r']) {
                            enter_mode = ENTER_MODE_REVOLVE_ADD;
                        } else if (key_pressed['L'] && globals.key_shift_held) {
                            enter_mode = ENTER_MODE_LOAD;
                        } else if (key_pressed['f']) {
                            console_params_preview_flip_flag = !console_params_preview_flip_flag;
                            console_params_preview_update();
                        } else if (key_pressed['U'] && globals.key_shift_held) {
                            // history_redo(&history, &manifold_manifold, &fancy_mesh);
                        } else if (key_pressed['u']) { // TODO undo

                            // NOTE: discard the undo event itself
                            // NOTE: discard the event we're undoing
                            // NOTE: discard unprocessed tail
                            ui_history_next_unprocessed -= 2;
                            ui_history_one_past_end = ui_history_next_unprocessed;

                            // reset state of Conversation
                            conversation_reset();

                            // replay events back
                            for (   UserInputEvent *event = ui_history_buffer;
                                    event < ui_history_next_unprocessed;
                                    ++event) {
                                ui_event_process(*event);
                            }

                        }
                    }
                    console_params_preview_update();
                }
                { // mouse input
                    { // camera_move
                        if (hot_pane == HOT_PANE_2D) {
                            camera_move(&camera2D);
                        } else if (hot_pane == HOT_PANE_3D) {
                            camera_move(&camera3D);
                        }
                    }
                    { // pick
                        ;
                        // FORNOW camera data
                        mat4 PV_2D = camera_get_PV(&camera2D);
                        real32 mouse_x, mouse_y; { _input_get_mouse_position_and_change_in_position_in_world_coordinates(PV_2D.data, &mouse_x, &mouse_y, NULL, NULL); }
                        mat4 P_3D = camera_get_P(&camera3D);
                        mat4 V_3D = camera_get_V(&camera3D);
                        mat4 PV_3D = P_3D * V_3D;

                        { // pick 2D pick 2d pick
                            if (hot_pane == HOT_PANE_2D) {
                                { // click dxf click dxf_click
                                    if (!globals.mouse_left_held) {
                                    } else if (click_mode == CLICK_MODE_NONE) {
                                    } else if (click_mode == CLICK_MODE_MOVE_2D_ORIGIN_TO) {
                                        if (click_modifier == CLICK_MODE_NONE) {
                                            origin_x = mouse_x;
                                            origin_y = mouse_y;
                                        } else if (click_modifier == CLICK_MODIFIER_CENTER_OF) {
                                            real32 min_squared_distance = HUGE_VAL;
                                            for (DXFEntity *entity = dxf.entities; entity < &dxf.entities[dxf.num_entities]; ++entity) {
                                                if (entity->type == DXF_ENTITY_TYPE_LINE) {
                                                    continue;
                                                } else {
                                                    ASSERT(entity->type == DXF_ENTITY_TYPE_ARC);
                                                    DXFArc *arc = &entity->arc;
                                                    real32 squared_distance = squared_distance_point_dxf_arc(mouse_x, mouse_y, arc);
                                                    if (squared_distance < min_squared_distance) {
                                                        min_squared_distance = squared_distance;
                                                        origin_x = arc->center_x;
                                                        origin_y = arc->center_y;
                                                    }
                                                }
                                            }
                                        } else if (click_modifier == CLICK_MODIFIER_END_OF) {
                                            real32 min_squared_distance = HUGE_VAL;
                                            for (DXFEntity *entity = dxf.entities; entity < &dxf.entities[dxf.num_entities]; ++entity) {
                                                real32 x[2], y[2];
                                                entity_get_start_and_end_points(entity, &x[0], &y[0], &x[1], &y[1]);
                                                for (u32 d = 0; d < 2; ++d) {
                                                    real32 squared_distance = squared_distance_point_point(mouse_x, mouse_y, x[d], y[d]);
                                                    if (squared_distance < min_squared_distance) {
                                                        min_squared_distance = squared_distance;
                                                        origin_x = x[d];
                                                        origin_y = y[d];
                                                    }
                                                }
                                            }
                                        } else if (click_modifier == CLICK_MODIFIER_MIDDLE_OF) {
                                            // TODO
                                        }
                                        click_mode = CLICK_MODE_NONE;
                                        click_modifier = CLICK_MODIFIER_NONE;
                                        enter_mode = ENTER_MODE_NONE;
                                        conversation_update_M_3D_from_2D();
                                    } else {
                                        bool32 value_to_write_to_selection_mask = (click_mode == CLICK_MODE_SELECT);
                                        bool32 modifier_connected = (click_modifier == CLICK_MODIFIER_CONNECTED);
                                        if (click_modifier != CLICK_MODIFIER_WINDOW) {
                                            int hot_entity_index = -1;
                                            double hot_squared_distance = HUGE_VAL;
                                            for (u32 i = 0; i < dxf.num_entities; ++i) {
                                                DXFEntity *entity = &dxf.entities[i];
                                                double squared_distance = squared_distance_point_dxf_entity(mouse_x, mouse_y, entity);
                                                squared_distance /= (camera2D.screen_height_World * camera2D.screen_height_World / 4); // NDC
                                                if (squared_distance < hot_squared_distance) {
                                                    hot_squared_distance = squared_distance;
                                                    hot_entity_index = i;
                                                }
                                            }
                                            if (hot_entity_index != -1) {
                                                if (globals.mouse_left_held) {
                                                    if (!modifier_connected) {
                                                        dxf_selection_mask[hot_entity_index] = value_to_write_to_selection_mask;
                                                    } else {
                                                        u32 loop_index = pick.loop_index_from_entity_index[hot_entity_index];
                                                        DXFEntityIndexAndFlipFlag *loop = pick.loops[loop_index];
                                                        u32 num_entities = pick.num_entities_in_loops[loop_index];
                                                        for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities; ++entity_index_and_flip_flag) {
                                                            dxf_selection_mask[entity_index_and_flip_flag->entity_index] = value_to_write_to_selection_mask;
                                                        }
                                                    }
                                                }
                                            }
                                        } else if (globals.mouse_left_pressed) {
                                            if (globals.mouse_left_pressed) {
                                                if (window_select_click_count == 0) {
                                                    window_select_x = mouse_x;
                                                    window_select_y = mouse_y;

                                                    ++window_select_click_count;
                                                } else {
                                                    BoundingBox window = {
                                                        MIN(window_select_x, mouse_x),
                                                        MIN(window_select_y, mouse_y),
                                                        MAX(window_select_x, mouse_x),
                                                        MAX(window_select_y, mouse_y)
                                                    };
                                                    for (u32 i = 0; i < dxf.num_entities; ++i) {
                                                        if (bounding_box_contains(window, bbox[i])) {
                                                            dxf_selection_mask[i] = value_to_write_to_selection_mask;
                                                        }
                                                    }

                                                    window_select_click_count = 0;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        { // pick 3D pick 3d pick
                            if (hot_pane == HOT_PANE_3D) {
                                if (globals.mouse_left_pressed) {
                                    vec3 o = transformPoint(inverse(PV_3D), V3(globals.mouse_position_NDC, -1.0f));
                                    vec3 dir = normalized(transformPoint(inverse(PV_3D), V3(globals.mouse_position_NDC,  1.0f)) - o);

                                    int32 index_of_first_triangle_hit_by_ray = -1;
                                    {
                                        real32 min_distance = HUGE_VAL;
                                        for (u32 i = 0; i < fancy_mesh.num_triangles; ++i) {
                                            vec3 p[3]; {
                                                for (u32 j = 0; j < 3; ++j) p[j] = get(fancy_mesh.vertex_positions, fancy_mesh.triangle_indices[3 * i + j]);
                                            }
                                            RayTriangleIntersectionResult result = ray_triangle_intersection(o, dir, p[0], p[1], p[2]);
                                            if (result.hit) {
                                                if (result.distance < min_distance) {
                                                    min_distance = result.distance;
                                                    index_of_first_triangle_hit_by_ray = i; // FORNOW
                                                }
                                            }
                                        }
                                    }

                                    if (index_of_first_triangle_hit_by_ray != -1) {
                                        some_triangle_exists_that_matches_n_selected_and_r_n_selected = true;
                                        { // FORNOW (gross) calculateion of n_selected, r_n_selected
                                            n_selected = get(fancy_mesh.triangle_normals, index_of_first_triangle_hit_by_ray);
                                            {
                                                vec3 p_selected[3]; {
                                                    for (u32 j = 0; j < 3; ++j) p_selected[j] = get(fancy_mesh.vertex_positions, fancy_mesh.triangle_indices[3 * index_of_first_triangle_hit_by_ray + j]);
                                                }
                                                r_n_selected = dot(n_selected, p_selected[0]);
                                            }
                                        }
                                        conversation_update_M_3D_from_2D();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        { // draw
            ;
            // FORNOW: repeated computation
            bool32 dxf_anything_selected;
            bool32 stl_plane_selected;
            {
                dxf_anything_selected = false;
                for (u32 i = 0; i < dxf.num_entities; ++i) {
                    if (dxf_selection_mask[i]) {
                        dxf_anything_selected = true;
                        break;
                    }
                }

                stl_plane_selected = !IS_ZERO(squaredNorm(n_selected));
            }
            // FORNOW: repeated computation
            mat4 PV_2D = camera_get_PV(&camera2D);
            real32 mouse_x, mouse_y; { _input_get_mouse_position_and_change_in_position_in_world_coordinates(PV_2D.data, &mouse_x, &mouse_y, NULL, NULL); }
            mat4 P_3D = camera_get_P(&camera3D);
            mat4 V_3D = camera_get_V(&camera3D);
            mat4 PV_3D = P_3D * V_3D;

            conversation_draw(PV_2D, PV_3D, P_3D, V_3D, mouse_x, mouse_y, dxf_anything_selected, stl_plane_selected);
        }
    }
}
