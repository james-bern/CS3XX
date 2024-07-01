// XXXX: remove option to pass null in soup_draw
// XXXX: remove rounded edges
// NOTE: (soup draw should essentially never be called by the user)
// TODO: per-vertex size
// TODO: per-vertex stipple
// TODO: properly draw meshes ala that one nvidia white paper

////////////////////////////////////////////////////////////////////////////////
// soup ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define SOUP_POINTS         GL_POINTS
#define SOUP_LINES          GL_LINES
#define SOUP_LINE_STRIP     GL_LINE_STRIP
#define SOUP_LINE_LOOP      GL_LINE_LOOP
#define SOUP_TRIANGLES      GL_TRIANGLES
#define SOUP_TRIANGLE_FAN   GL_TRIANGLE_FAN
#define SOUP_TRIANGLE_STRIP GL_TRIANGLE_STRIP
#define SOUP_QUADS          255
#define SOUP_TRI_MESH       254 // TODO

struct {
    char *vert = R""(#version 330 core
        layout (location = 0) in vec3 vertex;
        layout (location = 1) in vec4 color;
        layout (location = 2) in float size;

        out BLOCK {
            vec4 color;
            float size;
        } vs_out;

        uniform mat4 transform;
        uniform bool force_draw_on_top;

        void main() {
            gl_Position = transform * vec4(vertex, 1);
            if (force_draw_on_top) {
                gl_Position.z = -.99 * gl_Position.w; // ?
            }
            vs_out.color = color;
            vs_out.size = size;
        }
    )"";

    char *geom_POINTS = R""(#version 330 core
        layout (points) in;
        layout (triangle_strip, max_vertices = 4) out;
        uniform vec2 OpenGL_from_Pixel_scale;

        in BLOCK {
            vec4 color;
            float size;
        } gs_in[];

        out GS_OUT {
            vec4 color;
            vec2 xy;
        } gs_out;

        void emit(vec4 p, float x, float y) {
            vec2 radius = (gs_in[0].size / 2) * OpenGL_from_Pixel_scale;
            gs_out.color = gs_in[0].color;                                     
            gs_out.xy = vec2(x, y);
            gl_Position = (p + vec4(radius * gs_out.xy, 0, 0)) * gl_in[0].gl_Position.w;
            EmitVertex();                                               
        }

        void main() {    
            vec4 p = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
            emit(p, -1, -1);
            emit(p, 1, -1);
            emit(p, -1, 1);
            emit(p, 1, 1);
            EndPrimitive();
        }  
    )"";

    char *frag_POINTS = R""(#version 330 core
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

    char *geom_LINES = R""(#version 330 core
        layout (lines) in;
        layout (triangle_strip, max_vertices = 4) out;
        uniform vec2 OpenGL_from_Pixel_scale;

        in BLOCK {
            vec4 color;
            float size;
        } gs_in[];

        out BLOCK {
            vec4 color;
            float size;
            vec2 position_Pixel; // NOTE: y flipped sorry
            float angle;
            vec2 starting_point_Pixel;
        } gs_out;

        void main() {    
            vec4 s = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
            vec4 t = gl_in[1].gl_Position / gl_in[1].gl_Position.w;
            vec4 color_s = gs_in[0].color;
            vec4 color_t = gs_in[1].color;

            float angle = atan(OpenGL_from_Pixel_scale.x * (t.y - s.y), OpenGL_from_Pixel_scale.y * (t.x - s.x));

            vec2 perp = OpenGL_from_Pixel_scale * normalize(OpenGL_from_Pixel_scale * vec2(s.y - t.y, t.x - s.x));
            vec4 perp_s = vec4((gs_in[0].size / 2) * perp, 0, 0);
            vec4 perp_t = vec4((gs_in[1].size / 2) * perp, 0, 0);

            gl_Position = (s - perp_s) * gl_in[0].gl_Position.w;
            gs_out.position_Pixel = (vec2(1.0f) + gl_Position.xy) / OpenGL_from_Pixel_scale;
            gs_out.color = color_s;
            gs_out.angle = angle;
            gs_out.starting_point_Pixel = (vec2(1.0f) + s.xy * gl_in[0].gl_Position.w) / OpenGL_from_Pixel_scale;
            EmitVertex();

            gl_Position = (t - perp_t) * gl_in[1].gl_Position.w;
            gs_out.position_Pixel = (vec2(1.0f) + gl_Position.xy) / OpenGL_from_Pixel_scale;
            gs_out.color = color_t;
            gs_out.angle = angle;
            gs_out.starting_point_Pixel = (vec2(1.0f) + s.xy * gl_in[0].gl_Position.w) / OpenGL_from_Pixel_scale;
            EmitVertex();

            gl_Position = (s + perp_s) * gl_in[0].gl_Position.w;
            gs_out.position_Pixel = (vec2(1.0f) + gl_Position.xy) / OpenGL_from_Pixel_scale;
            gs_out.color = color_s;
            gs_out.angle = angle;
            gs_out.starting_point_Pixel = (vec2(1.0f) + s.xy * gl_in[0].gl_Position.w) / OpenGL_from_Pixel_scale;
            EmitVertex();

            gl_Position = (t + perp_t) * gl_in[1].gl_Position.w;
            gs_out.position_Pixel = (vec2(1.0f) + gl_Position.xy) / OpenGL_from_Pixel_scale;
            gs_out.color = color_t;
            gs_out.angle = angle;
            gs_out.starting_point_Pixel = (vec2(1.0f) + s.xy * gl_in[0].gl_Position.w) / OpenGL_from_Pixel_scale;
            EmitVertex();

            EndPrimitive();
        }  
    )"";

    char *frag_LINES = R""(#version 330 core
        uniform bool stipple;

        in BLOCK {
            vec4 color;
            float size;
            vec2 position_Pixel;
            float angle;
            vec2 starting_point_Pixel;
        } fs_in;

        out vec4 frag_color;

        void main() {
            frag_color = fs_in.color;
            if (stipple) {
                vec2 xy = fs_in.position_Pixel;
                // rotate by -angle
                float s = sin(fs_in.angle);
                float c = cos(fs_in.angle);
                mat2 Rinv = mat2(c, -s, s, c);
                vec2 uv = Rinv * (xy - fs_in.starting_point_Pixel);

                if (int(uv.x + 99999) % 10 > 5) discard; // FORNOW
            }
        }
    )"";

    char *frag_TRIANGLES = R""(#version 330 core
        in BLOCK {
            vec4 color;
            float size;
        } fs_in;

        out vec4 frag_color;

        void main() {
            frag_color = fs_in.color;
        }
    )"";

    char *geom_TRI_MESH = R""(#version 330 core
        layout (triangles) in;
        layout (triangle_strip, max_vertices = 3) out;

        uniform vec2 OpenGL_from_Pixel_scale;

        in BLOCK {
            vec4 color;
            float size;
        } gs_in[];

        out GS_OUT {
            vec4 color;
            noperspective vec3 heights;
            noperspective vec3 sizes;
        } gs_out;

        float point_line_dist(vec2 p, vec2 a, vec2 b) {
           vec2 line = b - a;
           vec2 n = vec2(line.y, -line.x);
           vec2 v = p - a;
           return abs(dot(v, n)) / length(n);
        }

        void main() {    
            vec3 sizes = vec3(gs_in[0].size, gs_in[1].size, gs_in[2].size);

            for (int d = 0; d < 3; ++d) {
                gl_Position = gl_in[d].gl_Position / gl_in[d].gl_Position.w;
                gs_out.color = gs_in[d].color;

                int e = (d + 1) % 3;
                int f = (d + 2) % 3;
                vec2 p = gl_in[d].gl_Position.xy / gl_in[d].gl_Position.w / OpenGL_from_Pixel_scale;
                vec2 q = gl_in[e].gl_Position.xy / gl_in[e].gl_Position.w / OpenGL_from_Pixel_scale;
                vec2 r = gl_in[f].gl_Position.xy / gl_in[f].gl_Position.w / OpenGL_from_Pixel_scale;
                vec3 heights = vec3(0);
                heights[d] = point_line_dist(p, q, r);
                vec3 bary = vec3(0);
                bary[d] = 1;

                gs_out.heights = heights;
                gs_out.sizes = sizes;
                EmitVertex();                                               
            }

            EndPrimitive();
        }  
    )"";

    char *frag_TRI_MESH = R""(#version 330 core
        in GS_OUT {
            vec4 color;
            noperspective vec3 heights;
            noperspective vec3 sizes;
        } fs_in;

        out vec4 frag_color;

        // TODO: eso_size should go in here, and you can remove the z-fight-y pass ...
        // NOTE: passing 0 for size should be NO edges
        void main() {
            int i = 0;
            if (fs_in.heights[1] < fs_in.heights[i]) i = 1;
            if (fs_in.heights[2] < fs_in.heights[i]) i = 2;

            if (fs_in.sizes[i] < 0.01) frag_color = fs_in.color;
            else {
                vec3 h = fs_in.heights / fs_in.sizes;
                float height = min(min(h.x, h.y), h.z);
                frag_color = mix(mix(vec4(0,0,0,1), fs_in.color, 0.5), fs_in.color, smoothstep(0.5, 1.0, height));
            }
        }
    )"";


} soup_source;

struct {
    uint shader_program_POINTS;
    uint shader_program_LINES;
    uint shader_program_TRIANGLES;
    uint shader_program_TRI_MESH;
    uint VAO[1];
    uint VBO[16];
    uint EBO[1];
} soup;

run_before_main {
    uint vert = shader_compile(soup_source.vert, GL_VERTEX_SHADER);
    uint geom_POINTS = shader_compile(soup_source.geom_POINTS, GL_GEOMETRY_SHADER);
    uint geom_LINES = shader_compile(soup_source.geom_LINES, GL_GEOMETRY_SHADER);
    uint geom_TRI_MESH = shader_compile(soup_source.geom_TRI_MESH, GL_GEOMETRY_SHADER);
    uint frag_POINTS = shader_compile(soup_source.frag_POINTS, GL_FRAGMENT_SHADER);
    uint frag_LINES = shader_compile(soup_source.frag_LINES, GL_FRAGMENT_SHADER);
    uint frag_TRIANGLES = shader_compile(soup_source.frag_TRIANGLES, GL_FRAGMENT_SHADER);
    uint frag_TRI_MESH = shader_compile(soup_source.frag_TRI_MESH, GL_FRAGMENT_SHADER);
    soup.shader_program_POINTS = shader_build_program(vert, geom_POINTS, frag_POINTS);
    soup.shader_program_LINES = shader_build_program(vert, geom_LINES, frag_LINES);
    soup.shader_program_TRIANGLES = shader_build_program(vert, 0, frag_TRIANGLES);
    soup.shader_program_TRI_MESH = shader_build_program(vert, geom_TRI_MESH, frag_TRI_MESH);
    glGenVertexArrays(ARRAY_LENGTH(soup.VAO), soup.VAO);
    glGenBuffers(ARRAY_LENGTH(soup.VBO), soup.VBO);
    glGenBuffers(ARRAY_LENGTH(soup.EBO), soup.EBO);
};

void soup_draw(
        mat4 transform,
        uint SOUP_primitive,
        uint num_vertices,
        vec3 *vertex_positions,
        vec4 *vertex_colors,
        real *vertex_sizes,
        bool force_draw_on_top,
        bool stipple) {
    if (num_vertices == 0) { return; } // NOTE: num_vertices zero is valid input

    glBindVertexArray(soup.VAO[0]);
    uint attrib_index = 0;
    auto upload_vertex_attribute = [&](void *array, uint count, uint dim) {
        ASSERT(array);
        ASSERT(attrib_index <= ARRAY_LENGTH(soup.VBO));
        glDisableVertexAttribArray(attrib_index); {
            uint buffer_size = count * dim * sizeof(real);
            glBindBuffer(GL_ARRAY_BUFFER, soup.VBO[attrib_index]);
            glBufferData(GL_ARRAY_BUFFER, buffer_size, array, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(attrib_index, dim, GL_FLOAT, GL_FALSE, 0, NULL);
        } glEnableVertexAttribArray(attrib_index);
        ++attrib_index;
    };
    upload_vertex_attribute(vertex_positions, num_vertices, 3);
    upload_vertex_attribute(vertex_colors, num_vertices, 4);
    upload_vertex_attribute(vertex_sizes, num_vertices, 1);

    uint GL_primitive;
    uint shader_program_ID;
    {
        if (SOUP_primitive == SOUP_POINTS) {
            GL_primitive = GL_POINTS;
            shader_program_ID = soup.shader_program_POINTS;
        } else if (SOUP_primitive == SOUP_LINES) {
            GL_primitive = GL_LINES;
            shader_program_ID = soup.shader_program_LINES;
        } else if (SOUP_primitive == SOUP_LINE_STRIP) {
            GL_primitive = GL_LINE_STRIP;
            shader_program_ID = soup.shader_program_LINES;
        } else if (SOUP_primitive == SOUP_LINE_LOOP) {
            GL_primitive = GL_LINE_LOOP;
            shader_program_ID = soup.shader_program_LINES;
        } else if (SOUP_primitive == SOUP_TRIANGLES) {
            GL_primitive = GL_TRIANGLES;
            shader_program_ID = soup.shader_program_TRIANGLES;
        } else if (SOUP_primitive == SOUP_TRIANGLE_FAN) {
            GL_primitive = GL_TRIANGLE_FAN;
            shader_program_ID = soup.shader_program_TRIANGLES;
        } else if (SOUP_primitive == SOUP_TRIANGLE_STRIP) {
            GL_primitive = GL_TRIANGLE_STRIP;
            shader_program_ID = soup.shader_program_TRIANGLES;
        } else if (SOUP_primitive == SOUP_QUADS) {
            GL_primitive = GL_TRIANGLES;
            shader_program_ID = soup.shader_program_TRIANGLES;
        } else { ASSERT(SOUP_primitive == SOUP_TRI_MESH);
            GL_primitive = SOUP_TRIANGLES;
            shader_program_ID = soup.shader_program_TRI_MESH;
        }
    }
    ASSERT(shader_program_ID);
    glUseProgram(shader_program_ID);

    auto LOC = [&](char *name) { return glGetUniformLocation(shader_program_ID, name); };
    vec2 OpenGL_from_Pixel_scale = (2.0f / window_get_size_Pixel());

    glUniform1ui(LOC("stipple"), stipple);
    glUniform1ui(LOC("force_draw_on_top"), force_draw_on_top);
    glUniform2f(LOC("OpenGL_from_Pixel_scale"), OpenGL_from_Pixel_scale.x, OpenGL_from_Pixel_scale.y);
    glUniformMatrix4fv(LOC("transform"), 1, GL_TRUE, transform.data);

    if (SOUP_primitive != SOUP_QUADS) {
        glDrawArrays(GL_primitive, 0, num_vertices);
    } else { ASSERT(SOUP_primitive == SOUP_QUADS);
        const int MAX_VERTICES = 1000000;
        ASSERT(num_vertices <= MAX_VERTICES);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, soup.EBO[0]);
        {
            GL_primitive = GL_TRIANGLES;
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
        glDrawElements(GL_primitive, num_vertices, GL_UNSIGNED_INT, NULL);
    }
}


////////////////////////////////////////////////////////////////////////////////
// eso /////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define ESO_MAX_VERTICES 9999999

struct {
    bool _called_eso_begin_before_calling_eso_vertex_or_eso_end;

    vec4 current_color;
    real current_size;

    bool overlay;
    bool stipple;

    mat4 transform;
    uint primitive;

    uint num_vertices;

    vec3 vertex_positions[ESO_MAX_VERTICES];
    vec4 vertex_colors[ESO_MAX_VERTICES];
    real vertex_sizes[ESO_MAX_VERTICES];
} eso;

void eso_begin(mat4 transform, uint primitive) {
    ASSERT(!eso._called_eso_begin_before_calling_eso_vertex_or_eso_end);
    eso._called_eso_begin_before_calling_eso_vertex_or_eso_end = true;

    eso.current_color = V4(basic.magenta, 1.0f);
    eso.current_size = 1.5f;

    eso.overlay = false;
    eso.stipple = false;

    eso.transform = transform;
    eso.primitive = primitive;

    eso.num_vertices = 0;
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
            eso.vertex_sizes,
            eso.overlay,
            eso.stipple);
}

void eso_overlay(bool overlay) {
    eso.overlay = overlay;
}

void eso_stipple(bool stipple) {
    eso.stipple = stipple;
}

void eso_size(real size) {
    eso.current_size = size;
}

void eso_color(real red, real green, real blue, real alpha) {
    eso.current_color[0] = red;
    eso.current_color[1] = green;
    eso.current_color[2] = blue;
    eso.current_color[3] = alpha;
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
    eso.vertex_colors[eso.num_vertices] = eso.current_color;
    eso.vertex_sizes[eso.num_vertices] = eso.current_size;
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


