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

// GL_TRIANGLES = 4
// GL_LINES = 1
// GL_POINTS = 0

struct {
    char *vert_UBER = R""(#version 330 core
        layout (location = 0) in vec3 vertex;
        layout (location = 1) in vec4 color;
        layout (location = 2) in float size;
        layout (location = 3) in uint primitive_type;
        layout (location = 4) in uint stipple;
        layout (location = 5) in vec3 PV_c0; 
        layout (location = 6) in vec3 PV_c1; 
        layout (location = 7) in vec3 PV_c2; 
        layout (location = 8) in vec3 PV_c3; 
        layout (location = 9) in vec3 transform_c0; // you might ask why we pass 4 vec 3s when all we need are the first 3 rows
        layout (location = 10) in vec3 transform_c1; // this is because glsl is the best language ever and is column major
        layout (location = 11) in vec3 transform_c2; // despite it being a c like languange
        layout (location = 12) in vec3 transform_c3; // this resulted in 0 hours of lost time and no frustration whatsoever

        out BLOCK {
            vec4 color;
            float size;
            flat uint primitive_type;
            flat uint stipple;
        } vs_out;

        uniform bool force_draw_on_top;
        uniform mat4 test_transform;

        void main() {
        
            mat4 PV = mat4(
                vec4(PV_c0, 0.0),
                vec4(PV_c1, 0.0),
                vec4(PV_c2, 0.0),
                vec4(PV_c3, 1.0)
            );

            mat4 transform = mat4(
                vec4(transform_c0, 0.0),
                vec4(transform_c1, 0.0),
                vec4(transform_c2, 0.0),
                vec4(transform_c3, 1.0)
            );

            gl_Position = PV * transform * vec4(vertex, 1);
            if (force_draw_on_top) {
                gl_Position.z = -.99 * gl_Position.w; // ?
            }
            vs_out.color = color; 
            vs_out.size = size;
            vs_out.primitive_type = primitive_type;
            vs_out.stipple = stipple;
        }
    )"";

    char *geom_UBER = R""(#version 330 core
        layout (triangles) in;
        layout (triangle_strip, max_vertices = 4) out;
        uniform vec2 OpenGL_from_Pixel_scale;

        in BLOCK {
            vec4 color;
            float size;
            flat uint primitive_type;
            flat uint stipple;
        } gs_in[];

        out BLOCK {
            vec4 color;
            float size;
            vec2 position_Pixel; // NOTE: y flipped sorry
            float angle;
            vec2 starting_point_Pixel;
            vec2 xy;
            flat uint primitive_type;
            flat uint stipple;
        } gs_out;

        void emit_point(vec4 p, float x, float y) {
            vec2 radius = (gs_in[0].size / 2) * OpenGL_from_Pixel_scale;
            gs_out.color = gs_in[0].color;                                     
            gs_out.xy = vec2(x, y);
            gl_Position = (p + vec4(radius * gs_out.xy, 0, 0)) * gl_in[0].gl_Position.w;
            gs_out.primitive_type = gs_in[0].primitive_type;
            EmitVertex();                                               
        }
        

        void main() {
            if (gs_in[0].primitive_type == 0u) { // GL_POINTS == 0
                vec4 p = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
                emit_point(p, -1, -1);
                emit_point(p, 1, -1);
                emit_point(p, -1, 1);
                emit_point(p, 1, 1);
            } else if (gs_in[0].primitive_type == 1u) { // GL_LINES == 1
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
                gs_out.primitive_type = gs_in[0].primitive_type;
                gs_out.stipple = gs_in[0].stipple;
                EmitVertex();

                gl_Position = (t - perp_t) * gl_in[1].gl_Position.w;
                gs_out.position_Pixel = (vec2(1.0f) + gl_Position.xy) / OpenGL_from_Pixel_scale;
                gs_out.color = color_t;
                gs_out.angle = angle;
                gs_out.starting_point_Pixel = (vec2(1.0f) + s.xy * gl_in[0].gl_Position.w) / OpenGL_from_Pixel_scale;
                gs_out.primitive_type = gs_in[0].primitive_type;
                gs_out.stipple = gs_in[0].stipple;
                EmitVertex();

                gl_Position = (s + perp_s) * gl_in[0].gl_Position.w;
                gs_out.position_Pixel = (vec2(1.0f) + gl_Position.xy) / OpenGL_from_Pixel_scale;
                gs_out.color = color_s;
                gs_out.angle = angle;
                gs_out.starting_point_Pixel = (vec2(1.0f) + s.xy * gl_in[0].gl_Position.w) / OpenGL_from_Pixel_scale;
                gs_out.primitive_type = gs_in[0].primitive_type;
                gs_out.stipple = gs_in[0].stipple;
                EmitVertex();

                gl_Position = (t + perp_t) * gl_in[1].gl_Position.w;
                gs_out.position_Pixel = (vec2(1.0f) + gl_Position.xy) / OpenGL_from_Pixel_scale;
                gs_out.color = color_t;
                gs_out.angle = angle;
                gs_out.starting_point_Pixel = (vec2(1.0f) + s.xy * gl_in[0].gl_Position.w) / OpenGL_from_Pixel_scale;
                gs_out.primitive_type = gs_in[0].primitive_type;
                gs_out.stipple = gs_in[0].stipple;
                EmitVertex();
            } else if (gs_in[0].primitive_type == 4u) { // GL_TRIANGLES == 4
                for(int i = 0; i < 3; i++) {
                    gl_Position = gl_in[i].gl_Position;
                    gs_out.color = gs_in[i].color;
                    gs_out.primitive_type = gs_in[0].primitive_type;
                    gs_out.stipple = gs_in[0].stipple;
                    // If you have vertex attributes, pass them through
                    // outNormal = inNormal[i];
                    // outTexCoord = inTexCoord[i];

                    EmitVertex();
                }
            }
            EndPrimitive();
        }  
    )"";

    char *frag_UBER= R""(#version 330 core

        in BLOCK {
            vec4 color;
            float size;
            vec2 position_Pixel;
            float angle;
            vec2 starting_point_Pixel;
            vec2 xy;
            flat uint primitive_type;
            flat uint stipple;
        } fs_in;

        out vec4 frag_color;

        void main() {
            if (fs_in.primitive_type == 0u) { // GL_POINTS == 0
                frag_color = fs_in.color;
                if (length(fs_in.xy) > 1) { discard; }
            } else if (fs_in.primitive_type == 1u) { // GL_LINES == 1;
                frag_color = fs_in.color; // TODO: add stipple
                if (fs_in.stipple == 1u) {
                        vec2 xy = fs_in.position_Pixel;
                        // rotate by -angle
                        float s = sin(fs_in.angle);
                        float c = cos(fs_in.angle);
                        mat2 Rinv = mat2(c, -s, s, c);
                        vec2 uv = Rinv * (xy - fs_in.starting_point_Pixel);

                        if (int(uv.x + 99999) % 10 > 5) discard; // FORNOW
                }
            } else if (fs_in.primitive_type == 4u) {
                frag_color = fs_in.color;
            }
        }

    )"";

        } stew_source;

        struct {
            uint shader_program_UBER;
            uint VAO[1];
            uint VBO[32];
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
        uint num_vertices,
        vec3 *vertex_positions,
        vec4 *vertex_colors,
        real *vertex_sizes,
        uint *vertex_primitives,
        uint *vertex_stipple,
        vec3 *vertex_PV_c0,
        vec3 *vertex_PV_c1,
        vec3 *vertex_PV_c2,
        vec3 *vertex_PV_c3,
        vec3 *vertex_transform_c0,
        vec3 *vertex_transform_c1,
        vec3 *vertex_transform_c2,
        vec3 *vertex_transform_c3,
        bool force_draw_on_top,
        mat4 current_transform) {
    if (num_vertices == 0) { return; } // NOTE: num_vertices zero is valid input

    glBindVertexArray(stew.VAO[0]);
    uint attrib_index = 0;
    auto upload_vertex_attribute = [&](void *array, uint count, uint dim, bool isInt) {
        ASSERT(array);
        ASSERT(attrib_index <= ARRAY_LENGTH(stew.VBO));
        glDisableVertexAttribArray(attrib_index); {
            uint buffer_size = count * dim * (isInt ? sizeof(uint) : sizeof(real));
            glBindBuffer(GL_ARRAY_BUFFER, stew.VBO[attrib_index]);
            glBufferData(GL_ARRAY_BUFFER, buffer_size, array, GL_DYNAMIC_DRAW);
            if (isInt) { // note special fucntion with 'I' because opengl says s
                glVertexAttribIPointer(attrib_index, dim, GL_UNSIGNED_INT, 0, NULL);
            } else { 
                glVertexAttribPointer(attrib_index, dim, GL_FLOAT, GL_FALSE, 0, NULL);
            }
        } glEnableVertexAttribArray(attrib_index);
        ++attrib_index;
    };
    upload_vertex_attribute(vertex_positions, num_vertices, 3, false);
    upload_vertex_attribute(vertex_colors, num_vertices, 4, false);
    upload_vertex_attribute(vertex_sizes, num_vertices, 1, false);
    upload_vertex_attribute(vertex_primitives, num_vertices, 1, true);
    upload_vertex_attribute(vertex_stipple, num_vertices, 1, true);
    upload_vertex_attribute(vertex_PV_c0, num_vertices, 3, false);
    upload_vertex_attribute(vertex_PV_c1, num_vertices, 3, false);
    upload_vertex_attribute(vertex_PV_c2, num_vertices, 3, false);
    upload_vertex_attribute(vertex_PV_c3, num_vertices, 3, false);
    upload_vertex_attribute(vertex_transform_c0, num_vertices, 3, false);
    upload_vertex_attribute(vertex_transform_c1, num_vertices, 3, false);
    upload_vertex_attribute(vertex_transform_c2, num_vertices, 3, false);
    upload_vertex_attribute(vertex_transform_c3, num_vertices, 3, false);

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

    glUniform1ui(LOC("force_draw_on_top"), force_draw_on_top);
    glUniform2f(LOC("OpenGL_from_Pixel_scale"), OpenGL_from_Pixel_scale.x, OpenGL_from_Pixel_scale.y);
    glUniformMatrix4fv(LOC("test_transform"), 1, GL_TRUE, current_transform.data);

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

    mat4 current_transform;
    mat4 current_PV;


    uint num_vertices;

    vec3 vertex_positions[GL_MAX_VERTICES];
    vec4 vertex_colors[GL_MAX_VERTICES];
    real vertex_sizes[GL_MAX_VERTICES];
    uint vertex_primitives[GL_MAX_VERTICES];
    uint vertex_stipple[GL_MAX_VERTICES];
    vec3 vertex_PV_c0[GL_MAX_VERTICES];
    vec3 vertex_PV_c1[GL_MAX_VERTICES];
    vec3 vertex_PV_c2[GL_MAX_VERTICES];
    vec3 vertex_PV_c3[GL_MAX_VERTICES];
    vec3 vertex_transforms_c0[GL_MAX_VERTICES];
    vec3 vertex_transforms_c1[GL_MAX_VERTICES];
    vec3 vertex_transforms_c2[GL_MAX_VERTICES];
    vec3 vertex_transforms_c3[GL_MAX_VERTICES];
} gl;

void gl_begin() {
    ASSERT(!gl._called_gl_begin_before_calling_gl_vertex_or_gl_end);
    gl._called_gl_begin_before_calling_gl_vertex_or_gl_end = true;

    gl.current_color = V4(basic.magenta, 1.0f);
    gl.current_size = 1.5f;

    gl.overlay = false;
    gl.stipple = false;

    gl.num_vertices = 0;

    gl.current_transform = _Identity4x4;
}

void gl_end() {
    ASSERT(gl._called_gl_begin_before_calling_gl_vertex_or_gl_end);
    gl._called_gl_begin_before_calling_gl_vertex_or_gl_end = false;
    stew_draw(
            gl.num_vertices,
            gl.vertex_positions,
            gl.vertex_colors,
            gl.vertex_sizes,
            gl.vertex_primitives,
            gl.vertex_stipple,
            gl.vertex_PV_c0,
            gl.vertex_PV_c1,
            gl.vertex_PV_c2,
            gl.vertex_PV_c3,
            gl.vertex_transforms_c0,
            gl.vertex_transforms_c1,
            gl.vertex_transforms_c2,
            gl.vertex_transforms_c3,
            gl.overlay,
            gl.current_transform);
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

void gl_PV(mat4 PV) {
    gl.current_PV = PV;
}

void gl_transform(mat4 transform) {
    gl.current_transform = transform;
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
    gl.vertex_stipple[gl.num_vertices] = gl.stipple;
    for_(i, 3) {
        gl.vertex_PV_c0[gl.num_vertices][i] = gl.current_PV(i, 0); 
        gl.vertex_PV_c1[gl.num_vertices][i] = gl.current_PV(i, 1); 
        gl.vertex_PV_c2[gl.num_vertices][i] = gl.current_PV(i, 2); 
        gl.vertex_PV_c3[gl.num_vertices][i] = gl.current_PV(i, 3); 
        gl.vertex_transforms_c0[gl.num_vertices][i] = gl.current_transform(i, 0); 
        gl.vertex_transforms_c1[gl.num_vertices][i] = gl.current_transform(i, 1); 
        gl.vertex_transforms_c2[gl.num_vertices][i] = gl.current_transform(i, 2); 
        gl.vertex_transforms_c3[gl.num_vertices][i] = gl.current_transform(i, 3); 
        //printf("%f ", gl.vertex_transforms_c0[gl.num_vertices][i]);
    }
    printf("\n");
    for_(i, 3) {
        //printf("%f ", transform(1, i));
        //printf("%f ", gl.vertex_transforms_c1[gl.num_vertices][i]);
    }
    printf("\n");
    for_(i, 3) {
        //printf("%f ", gl.vertex_transforms_c2[gl.num_vertices][i]);
        //printf("%f ", transform(2, i));
    }
    printf("\n");
    for_(i, 3) {
        //printf("%f ", gl.vertex_transforms_c3[gl.num_vertices][i]);
        //printf("%f ", transform(2, i));
    }
    //printf("\n\n\n\n");
    if (gl.current_primitive == GL_POINTS) {
        gl.num_vertices += 3;
    } else if (gl.current_primitive == GL_LINES) {
        if (gl.num_vertices % 3 == 1) {
            gl.num_vertices += 2;
        } else {
        gl.num_vertices += 1;
        }
    } else { ASSERT(gl.current_primitive == GL_TRIANGLES); // TODO: make better
        gl.num_vertices += 1;
    }
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


