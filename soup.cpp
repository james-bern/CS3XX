////////////////////////////////////////////////////////////////////////////////
// soup ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// TODO: remove option to pass null in soup_draw
// TODO: remove rounded edges
// (soup draw should essentially never be called by the user)

// TODO: properly draw meshes ala that one nvidia white paper
#define SOUP_POINTS         GL_POINTS
#define SOUP_LINES          GL_LINES
#define SOUP_LINE_STRIP     GL_LINE_STRIP
#define SOUP_LINE_LOOP      GL_LINE_LOOP
#define SOUP_TRIANGLES      GL_TRIANGLES
#define SOUP_TRIANGLE_FAN   GL_TRIANGLE_FAN
#define SOUP_TRIANGLE_STRIP GL_TRIANGLE_STRIP
#define SOUP_QUADS          255

#define _SOUP_XY 2
#define _SOUP_XYZ 3
#define _SOUP_XYZW 4
#define _SOUP_RGB 3
#define _SOUP_RGBA 4

struct {
    int _shader_program_POINTS;
    int _shader_program_LINES;
    int _shader_program_TRIANGLES;
    uint _VAO[3];
    uint _VBO[2];
    uint _EBO[3];

    char *_vert = R""(
        #version 330 core
        layout (location = 0) in vec3 vertex;
        layout (location = 1) in vec4 color;

        out BLOCK {
            vec4 color;
        } vs_out;

        uniform mat4 transform;
        uniform bool force_draw_on_top;
        uniform vec4 color_if_vertex_colors_is_NULL;

        void main() {
            gl_Position = transform * vec4(vertex, 1);
            vs_out.color = color;
            if (force_draw_on_top) {
                gl_Position.z = -.99 * gl_Position.w; // ?
            }
        }
    )"";

    char *_geom_POINTS = R""(
        #version 330 core
        layout (points) in;
        layout (triangle_strip, max_vertices = 4) out;
        uniform float aspect;
        uniform float primitive_radius_OpenGL;

        in BLOCK {
            vec4 color;
        } gs_in[];

        out GS_OUT {
            vec4 color;
            vec2 xy;
        } gs_out;

        vec4 _position;

        void emit(float x, float y) {
            gs_out.xy = vec2(x, y);
            gl_Position = (_position + primitive_radius_OpenGL * vec4(x / aspect, y, 0, 0)) * gl_in[0].gl_Position.w;
            gs_out.color = gs_in[0].color;                                     
            EmitVertex();                                               
        }

        void main() {    
            _position = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
            emit(-1, -1);
            emit(1, -1);
            emit(-1, 1);
            emit(1, 1);
            EndPrimitive();
        }  
    )"";

    char *_frag_POINTS = R""(
        #version 330 core

        in GS_OUT {
            vec4 color;
            vec2 xy;
        } fs_in;

        out vec4 frag_color;

        void main() {
            frag_color = fs_in.color;
            if (length(fs_in.xy) > 1) { discard; }
        }
    )"";

    char *_geom_LINES = R""(
        #version 330 core
        layout (lines) in;
        layout (triangle_strip, max_vertices = 4) out;
        uniform float aspect;
        uniform float primitive_radius_OpenGL;

        in BLOCK {
            vec4 color;
        } gs_in[];

        out BLOCK {
            vec4 color;
        } gs_out;

        void main() {    
            vec4 s = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
            vec4 t = gl_in[1].gl_Position / gl_in[1].gl_Position.w;
            vec4 color_s = gs_in[0].color;
            vec4 color_t = gs_in[1].color;

            vec4 perp = vec4(primitive_radius_OpenGL * vec2(1 / aspect, 1) * normalize(vec2(-1 / aspect, 1) * (t - s).yx), 0, 0);

            gl_Position = (s - perp) * gl_in[0].gl_Position.w;
            gs_out.color = color_s;
            EmitVertex();

            gl_Position = (t - perp) * gl_in[1].gl_Position.w;
            gs_out.color = color_t;
            EmitVertex();

            gl_Position = (s + perp) * gl_in[0].gl_Position.w;
            gs_out.color = color_s;
            EmitVertex();

            gl_Position = (t + perp) * gl_in[1].gl_Position.w;
            gs_out.color = color_t;
            EmitVertex();

            EndPrimitive();
        }  
    )"";

    char *_frag = R""(
        #version 330 core

        in BLOCK {
            vec4 color;
        } fs_in;

        out vec4 frag_color;

        void main() {
            frag_color = fs_in.color;
        }
    )"";
} soup;

run_before_main {
    soup._shader_program_POINTS = _shader_compile_and_build_program(soup._vert, soup._frag_POINTS, soup._geom_POINTS);
    soup._shader_program_LINES = _shader_compile_and_build_program(soup._vert, soup._frag, soup._geom_LINES);
    soup._shader_program_TRIANGLES = _shader_compile_and_build_program(soup._vert, soup._frag);
    glGenVertexArrays(ARRAY_LENGTH(soup._VAO), soup._VAO);
    glGenBuffers(ARRAY_LENGTH(soup._VBO), soup._VBO);
    glGenBuffers(ARRAY_LENGTH(soup._EBO), soup._EBO);
};

void soup_draw(
        mat4 transform,
        uint primitive,
        uint num_vertices,
        vec3 *vertex_positions,
        vec4 *vertex_colors,
        real size_Pixel,
        bool force_draw_on_top) {
    if (num_vertices == 0) { return; } // NOTE: num_vertices zero is valid input

    glBindVertexArray(soup._VAO[0]);
    int i_attrib = 0;
    auto guarded_push = [&](int buffer_size, void *array, int dim) {
        glDisableVertexAttribArray(i_attrib); // fornow
        if (array) {
            glBindBuffer(GL_ARRAY_BUFFER, soup._VBO[i_attrib]);
            glBufferData(GL_ARRAY_BUFFER, buffer_size, array, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(i_attrib, dim, GL_REAL, 0, 0, NULL);
            glEnableVertexAttribArray(i_attrib);
        }
        ++i_attrib;
    };
    int vvv_size = int(num_vertices * sizeof(vec3));
    int ccc_size = int(num_vertices * sizeof(vec4));
    guarded_push(vvv_size, vertex_positions, 3);
    guarded_push(ccc_size, vertex_colors, 4);

    int shader_program_ID = 0; {
        if (primitive == SOUP_POINTS) {
            shader_program_ID = soup._shader_program_POINTS;
        } else if (primitive == SOUP_LINES || primitive == SOUP_LINE_STRIP || primitive == SOUP_LINE_LOOP) {
            shader_program_ID = soup._shader_program_LINES;
        } else { // including SOUP_QUADS
            shader_program_ID = soup._shader_program_TRIANGLES;
        }
    }
    ASSERT(shader_program_ID);
    glUseProgram(shader_program_ID);

    _shader_set_uniform_real(shader_program_ID, "aspect", window_get_aspect());
    _shader_set_uniform_real(shader_program_ID, "primitive_radius_OpenGL", 0.5f * size_Pixel / window_get_size_Pixel().y);
    _shader_set_uniform_bool(shader_program_ID, "force_draw_on_top", force_draw_on_top);
    _shader_set_uniform_mat4(shader_program_ID, "transform", transform.data);

    if (primitive != SOUP_QUADS) {
        glDrawArrays(primitive, 0, num_vertices);
    } else { ASSERT(primitive == SOUP_QUADS);
        // we upload three EBO's _once_               
        // and bind the appropriate one before drawing

        const int MAX_VERTICES = 1000000;
        ASSERT(num_vertices <= MAX_VERTICES);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, soup._EBO[0]);

        {
            primitive = SOUP_TRIANGLES;
            num_vertices = (num_vertices / 4) * 6;
            static GLuint *indices;
            if (!indices) {
                indices = (GLuint *) malloc(MAX_VERTICES / 4 * 6 * sizeof(GLuint));
                int k = 0;
                for (int i = 0; i < MAX_VERTICES / 4; ++i) {
                    indices[k++] = 4 * i + 2;
                    indices[k++] = 4 * i + 1;
                    indices[k++] = 4 * i + 0;
                    indices[k++] = 4 * i + 3;
                    indices[k++] = 4 * i + 2;
                    indices[k++] = 4 * i + 0;
                }
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_VERTICES / 4 * 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);
            }
        }
        glDrawElements(primitive, num_vertices, GL_UNSIGNED_INT, NULL);
    }
}


////////////////////////////////////////////////////////////////////////////////
// eso /////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define ESO_MAX_VERTICES 9999999

struct {
    bool _called_eso_begin_before_calling_eso_vertex_or_eso_end;
    vec4 _current_color;
    vec3 *vertex_positions;
    vec4 *vertex_colors;
    mat4 transform;
    uint primitive;
    uint num_vertices;
    real size_Pixel;
    bool overlay;
} eso;

run_before_main {
    eso.vertex_positions = (vec3 *) calloc(ESO_MAX_VERTICES, sizeof(vec3));
    eso.vertex_colors = (vec4 *) calloc(ESO_MAX_VERTICES, sizeof(vec4));
    eso._current_color = V4(basic.magenta, 1.0f);
};

void eso_begin(mat4 transform, uint primitive, bool force_draw_on_top = false) {
    ASSERT(!eso._called_eso_begin_before_calling_eso_vertex_or_eso_end);
    eso._called_eso_begin_before_calling_eso_vertex_or_eso_end = true;
    eso.primitive = primitive;
    eso.overlay = force_draw_on_top;
    eso.num_vertices = 0;
    eso.transform = transform;
}

void eso_stipple(u8 stipple_code) {
    FORNOW_UNUSED(stipple_code);
}

void eso_size(real size_Pixel) {
    eso.size_Pixel = size_Pixel;
}

void eso_color(real red, real green, real blue, real alpha) {
    eso._current_color[0] = red;
    eso._current_color[1] = green;
    eso._current_color[2] = blue;
    eso._current_color[3] = alpha;
}

void eso_color(real red, real green, real blue) {
    eso_color(red, green, blue, 1.0f);
}

void eso_color(vec3 rgb) {
    eso_color(rgb[0], rgb[1], rgb[2], 1.0f);
}

void eso_color(vec3 rgb, real alpha) {
    eso_color(rgb[0], rgb[1], rgb[2], alpha);
}

void eso_color(vec4 rgba) {
    eso_color(rgba[0], rgba[1], rgba[2], rgba[3]);
}

void eso_vertex(real x, real y, real z) {
    ASSERT(eso._called_eso_begin_before_calling_eso_vertex_or_eso_end);
    ASSERT(eso.num_vertices < ESO_MAX_VERTICES);
    eso.vertex_positions[eso.num_vertices] = { x, y, z };
    eso.vertex_colors[eso.num_vertices] = eso._current_color;
    ++eso.num_vertices;
}

void eso_vertex(real x, real y) {
    eso_vertex(x, y, 0.0f);
}


void eso_vertex(vec2 xy) {
    eso_vertex(xy[0], xy[1]);
}

void eso_vertex(vec3 xyz) {
    eso_vertex(xyz[0], xyz[1], xyz[2]);
}

void eso_end() {
    ASSERT(eso._called_eso_begin_before_calling_eso_vertex_or_eso_end);
    eso._called_eso_begin_before_calling_eso_vertex_or_eso_end = false;
    soup_draw(
            eso.transform,
            eso.primitive,
            eso.num_vertices,
            eso.vertex_positions,
            eso.vertex_colors,
            eso.size_Pixel,
            eso.overlay);
}

