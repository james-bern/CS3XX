// XXXX: remove option to pass null in stew_draw
// XXXX: remove rounded edges
// NOTE: (stew draw should essentially never be called by the user)
// TODO: per-vertex size
// TODO: per-vertex stipple
// TODO: properly draw meshes ala that one nvidia white paper
////////////////////////////////////////////////////////////////////////////////
// stew ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define STEW_POINTS         GL_POINTS
#define STEW_LINES          GL_LINES
#define STEW_LINE_STRIP     GL_LINE_STRIP
#define STEW_LINE_LOOP      GL_LINE_LOOP
#define STEW_TRIANGLES      GL_TRIANGLES
#define STEW_TRIANGLE_FAN   GL_TRIANGLE_FAN
#define STEW_TRIANGLE_STRIP GL_TRIANGLE_STRIP

// copy vert and frag_TRIANGLES into vert_UBER, frag_UBER
// make uber_shader_program
// draw the triangle with your shader program

// // TASK: per-vertex model matrix
// somehow someway pass a mat4 per vertex
// apply the mat4
// add a mat4 called curr_M to gl, and a giant array of mat4s to be uploaded
// in the vertex shader PV * M

// // TASK: per-vertex primtive
// pass the primitives (verify this worked somehow -- color according to primitive)
// points
// padding

struct {
    char *vert_UBER = R""(#version 330 core
        layout (location = 0) in vec3 vertex;
        layout (location = 1) in vec4 color;
        layout (location = 2) in float size;

        out BLOCK {
            vec4 color;
            float size;
            int primitive_type;
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

    char *geom_UBER = R""(#version 330 core
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

    char *frag_UBER= R""(#version 330 core
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


    // TODO: uber

} stew_source;

struct {
	uint shader_program_UBER;
    uint VAO[1];
    uint VBO[16];
    uint EBO[1];
} stew;

run_before_main {
    uint vert_UBER = shader_compile(stew_source.vert_UBER, GL_VERTEX_SHADER);
    uint geom_UBER = shader_compile(stew_source.geom_UBER, GL_GEOMETRY_SHADER);
	uint frag_UBER = shader_compile(stew_source.frag_UBER, GL_FRAGMENT_SHADER);
	stew.shader_program_UBER = shader_build_program(vert_UBER, geom_UBER, frag_UBER);

    glGenVertexArrays(ARRAY_LENGTH(stew.VAO), stew.VAO);
    glGenBuffers(ARRAY_LENGTH(stew.VBO), stew.VBO);
    glGenBuffers(ARRAY_LENGTH(stew.EBO), stew.EBO);
};

void stew_draw(
        mat4 transform,
        uint num_vertices,
        vec3 *vertex_positions,
        vec4 *vertex_colors,
        real *vertex_sizes,
        uint *vertex_primitives,
        bool force_draw_on_top,
        bool stipple) {
    if (num_vertices == 0) { return; } // NOTE: num_vertices zero is valid input

    glBindVertexArray(stew.VAO[0]);
    uint attrib_index = 0;
    auto upload_vertex_attribute = [&](void *array, uint count, uint dim) {
        ASSERT(array);
        ASSERT(attrib_index <= ARRAY_LENGTH(stew.VBO));
        glDisableVertexAttribArray(attrib_index); {
            uint buffer_size = count * dim * sizeof(real);
            glBindBuffer(GL_ARRAY_BUFFER, stew.VBO[attrib_index]);
            glBufferData(GL_ARRAY_BUFFER, buffer_size, array, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(attrib_index, dim, GL_FLOAT, GL_FALSE, 0, NULL);
        } glEnableVertexAttribArray(attrib_index);
        ++attrib_index;
    };
    upload_vertex_attribute(vertex_positions, num_vertices, 3);
    upload_vertex_attribute(vertex_colors, num_vertices, 4);
    upload_vertex_attribute(vertex_sizes, num_vertices, 1);
    upload_vertex_attribute(vertex_primitives, num_vertices, 1);

    uint shader_program_ID;
    {
		shader_program_ID = stew.shader_program_UBER;
#if 0
        if (STEW_primitive == STEW_POINTS) {
            GL_primitive = GL_POINTS;
            shader_program_ID = stew.shader_program_POINTS;
        } else if (STEW_primitive == STEW_LINES) {
            GL_primitive = GL_LINES;
            shader_program_ID = stew.shader_program_LINES;
        } else if (STEW_primitive == STEW_LINE_STRIP) {
            GL_primitive = GL_LINE_STRIP;
            shader_program_ID = stew.shader_program_LINES;
        } else if (STEW_primitive == STEW_LINE_LOOP) {
            GL_primitive = GL_LINE_LOOP;
            shader_program_ID = stew.shader_program_LINES;
        } else if (STEW_primitive == STEW_TRIANGLES) {
            GL_primitive = GL_TRIANGLES;
            shader_program_ID = stew.shader_program_TRIANGLES;
        } else if (STEW_primitive == STEW_TRIANGLE_FAN) {
            GL_primitive = GL_TRIANGLE_FAN;
            shader_program_ID = stew.shader_program_TRIANGLES;
        } else if (STEW_primitive == STEW_TRIANGLE_STRIP) {
            GL_primitive = GL_TRIANGLE_STRIP;
            shader_program_ID = stew.shader_program_TRIANGLES;
        } else { ASSERT(0);
        }
#endif
    }
    ASSERT(shader_program_ID);
    glUseProgram(shader_program_ID);

    auto LOC = [&](char *name) { return glGetUniformLocation(shader_program_ID, name); };
    vec2 OpenGL_from_Pixel_scale = (2.0f / window_get_size_Pixel());

    glUniform1ui(LOC("stipple"), stipple);
    glUniform1ui(LOC("force_draw_on_top"), force_draw_on_top);
    glUniform2f(LOC("OpenGL_from_Pixel_scale"), OpenGL_from_Pixel_scale.x, OpenGL_from_Pixel_scale.y);
    glUniformMatrix4fv(LOC("transform"), 1, GL_TRUE, transform.data);

    glDrawArrays(GL_TRIANGLES, 0, num_vertices);
}


////////////////////////////////////////////////////////////////////////////////
// gl /////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define GL_MAX_VERTICES 999999

struct {
    bool _called_gl_begin_before_calling_gl_vertex_or_gl_end;

    vec4 current_color;
    real current_size;
    uint current_primitive;

    bool overlay;
    bool stipple;

    mat4 transform;


    uint num_vertices;

    vec3 vertex_positions[GL_MAX_VERTICES];
    vec4 vertex_colors[GL_MAX_VERTICES];
    real vertex_sizes[GL_MAX_VERTICES];
    uint vertex_primitives[GL_MAX_VERTICES];
} gl;

void gl_begin(mat4 transform) {
    ASSERT(!gl._called_gl_begin_before_calling_gl_vertex_or_gl_end);
    gl._called_gl_begin_before_calling_gl_vertex_or_gl_end = true;

    gl.current_color = V4(basic.magenta, 1.0f);
    gl.current_size = 1.5f;

    gl.overlay = false;
    gl.stipple = false;

    gl.transform = transform;

    gl.num_vertices = 0;

}

void gl_end() {
    ASSERT(gl._called_gl_begin_before_calling_gl_vertex_or_gl_end);
    gl._called_gl_begin_before_calling_gl_vertex_or_gl_end = false;
    stew_draw(
            gl.transform,
            gl.num_vertices,
            gl.vertex_positions,
            gl.vertex_colors,
            gl.vertex_sizes,
            gl.vertex_primitives,
            gl.overlay,
            gl.stipple);
}

void gl_primitive(uint primitive) {
    gl.current_primitive = primitive;
}

void gl_overlay(bool overlay) {
    gl.overlay = overlay;
}

void gl_stipple(bool stipple) {
    gl.stipple = stipple;
}

void gl_size(real size) {
    gl.current_size = size;
}

void gl_color(real red, real green, real blue, real alpha) {
    gl.current_color[0] = red;
    gl.current_color[1] = green;
    gl.current_color[2] = blue;
    gl.current_color[3] = alpha;
}

void gl_color(real red, real green, real blue) {
    gl_color(red, green, blue, 1.0f);
}

void gl_color(vec3 rgb) {
    gl_color(rgb[0], rgb[1], rgb[2], 1.0f);
}

void gl_color(vec3 rgb, real alpha) {
    gl_color(rgb[0], rgb[1], rgb[2], alpha);
}

void gl_color(vec4 rgba) {
    gl_color(rgba[0], rgba[1], rgba[2], rgba[3]);
}

void gl_vertex(real x, real y, real z) {
    ASSERT(gl._called_gl_begin_before_calling_gl_vertex_or_gl_end);
    ASSERT(gl.num_vertices < GL_MAX_VERTICES);
    gl.vertex_positions[gl.num_vertices] = { x, y, z };
    gl.vertex_colors[gl.num_vertices] = gl.current_color;
    gl.vertex_sizes[gl.num_vertices] = gl.current_size;
    gl.vertex_primitives[gl.num_vertices] = gl.current_primitive;
    ++gl.num_vertices;
}

void gl_vertex(real x, real y) {
    gl_vertex(x, y, 0.0f);
}


void gl_vertex(vec2 xy) {
    gl_vertex(xy[0], xy[1]);
}

void gl_vertex(vec3 xyz) {
    gl_vertex(xyz[0], xyz[1], xyz[2]);
}


