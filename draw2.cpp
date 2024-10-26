// TODO: VAO and VBO are about mesh data (don't connect them to shaders)

struct {
    uint num_vertices = 6;
    uint VAO;
    uint VBO[2];
    vec2 vertex_positions[6] = { { -1.0f, -1.0f }, {  1.0f, -1.0f }, {  1.0f,  1.0f }, { -1.0f, -1.0f }, {  1.0f,  1.0f }, { -1.0f,  1.0f }, };
    vec2 vertex_texture_coordinates[6] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }, };
} blit_quad;

run_before_main {
    glGenVertexArrays(1, &blit_quad.VAO);
    glGenBuffers(ARRAY_LENGTH(blit_quad.VBO), blit_quad.VBO);
    glBindVertexArray(blit_quad.VAO);
    POOSH(blit_quad.VBO, 0, blit_quad.num_vertices, blit_quad.vertex_positions);
    POOSH(blit_quad.VBO, 1, blit_quad.num_vertices, blit_quad.vertex_texture_coordinates);
};



struct {
    char *vert = R""(
        #version 330 core
        layout (location = 0) in vec3 position_Model;
        layout (location = 1) in vec3 normal_Model;
        layout (location = 2) in uint vertex_patch_index;

        out BLOCK {
            vec3 position_World;
            vec3 normal_World;
            flat uint vertex_patch_index;
        } vs_out;

        uniform mat4 PV;
        uniform mat4 M;

        void main() {

            {
                vec4 tmp = vec4(position_Model, 1.0);
                tmp = M * tmp;
                vs_out.position_World = vec3(tmp);
                gl_Position = PV * tmp;
            }

            {
                vec3 tmp = normal_Model;
                tmp = inverse(transpose(mat3(M))) * tmp;
                vs_out.normal_World = tmp;
            }

            {
                vs_out.vertex_patch_index = vertex_patch_index;
            }
        }
    )"";

    char *frag = R""(
        #version 330 core
        in BLOCK {
            vec3 position_World;
            vec3 normal_World;
            flat uint vertex_patch_index;
        } fs_in;

        uniform vec3 eye_World;

        uniform int mode;

        out vec4 _gl_FragColor;

        void main() {
            vec3 N = normalize(fs_in.normal_World);
            vec3 rgb = vec3(0.0);

            if (mode == 0) {
                vec3 warm_color = vec3(1.0, 1.0, 0.3);
                vec3 cool_color = vec3(0.3, 0.7, 1.0);

                { // sunlight gooch
                    vec3 L = vec3(0.0, 1.0, 0.0);
                    float LN = dot(L, N);
                    float t = 0.5 + 0.5 * LN;
                    rgb += 0.7 * mix(cool_color, warm_color, t);
                }

                { // eye light phong fresnel
                    vec3 L = normalize(eye_World - fs_in.position_World);
                    vec3 E = normalize(eye_World - fs_in.position_World);
                    vec3 H = normalize(L + E);
                    float F0 = 0.05;
                    float diffuse = max(0.0, dot(N, L));
                    float specular = pow(max(0.0, dot(N, H)), 256);
                    float fresnel = F0 + (1 - F0) * pow(1.0 - max(0.0, dot(N, H)), 5);
                    rgb += 0.3 * diffuse;
                    rgb += 0.2 * specular;
                    rgb += 0.3 * fresnel;
                }
            } else if ((mode == 1) || (mode == 3)) {
                int id = (mode == 1) ? int(fs_in.vertex_patch_index) : gl_PrimitiveID;
                int r = (id)             % 256;
                int g = (id / 256)       % 256;
                int b = (id / 256 / 256) % 256;
                rgb = vec3(r / 255.0, g / 255.0, b / 255.0);
            }

            _gl_FragColor = vec4(rgb, 1.0);
        }
    )"";
} face_pass_source;


struct {
    char *vert = R""(
        #version 330 core
        layout (location = 0) in vec3 position_Model;

        uniform mat4 PVM;

        void main() {
            gl_Position = PVM * vec4(position_Model, 1.0);
        }
    )"";

    char *geom = R""(#version 330 core
        layout (lines) in;
        layout (triangle_strip, max_vertices = 4) out;

        uniform vec2 OpenGL_from_Pixel_scale;
        uniform float _t01;

        out vec4 color;

        void main() {    
            vec4 s = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
            vec4 t = gl_in[1].gl_Position / gl_in[1].gl_Position.w;

            float size = 16.0f;

            vec2 _tangent = OpenGL_from_Pixel_scale * normalize(OpenGL_from_Pixel_scale * vec2(t.xy - s.xy));
            vec4 tangent = 3 * _t01 * vec4((size / 2) * _tangent, 0, 0);

            vec2 _perp = OpenGL_from_Pixel_scale * normalize(OpenGL_from_Pixel_scale * vec2(s.y - t.y, t.x - s.x));
            vec4 perp = vec4((size / 2) * _perp, 0, 0);

            color = vec4(1.0); color.r = 0;
            gl_Position = (s - perp - tangent) * gl_in[0].gl_Position.w;
            EmitVertex();

            color = vec4(1.0); color.r = 0;
            gl_Position = (s + perp - tangent) * gl_in[0].gl_Position.w;
            EmitVertex();

            color = vec4(1.0); color.g = 0;
            gl_Position = (t - perp + tangent) * gl_in[1].gl_Position.w;
            EmitVertex();

            color = vec4(1.0); color.g = 0;
            gl_Position = (t + perp + tangent) * gl_in[1].gl_Position.w;
            EmitVertex();

            EndPrimitive();
        }  
    )"";

    char *frag = R""(#version 330 core
        // TODO: sample the ID buffer and drop selectively
        in vec4 color;
        out vec4 _gl_FragColor;
        void main() {
            _gl_FragColor = color;
        }
    )"";
} edge_pass_source;

int face_shader_program;
int edge_shader_program;


run_before_main {
    {
        uint face_vert = shader_compile(face_pass_source.vert, GL_VERTEX_SHADER);
        uint face_frag = shader_compile(face_pass_source.frag, GL_FRAGMENT_SHADER);
        face_shader_program = shader_build_program(face_vert, 0, face_frag);
    }

    {
        uint edge_vert = shader_compile(edge_pass_source.vert, GL_VERTEX_SHADER  );
        uint edge_geom = shader_compile(edge_pass_source.geom, GL_GEOMETRY_SHADER);
        uint edge_frag = shader_compile(edge_pass_source.frag, GL_FRAGMENT_SHADER);
        edge_shader_program = shader_build_program(edge_vert, edge_geom, edge_frag);
    }
};


uint DRAW_MESH_MODE_LIT            = 0;
uint DRAW_MESH_MODE_PATCH_ID       = 1;
uint DRAW_MESH_MODE_PATCH_EDGES    = 2;
uint DRAW_MESH_MODE_TRIANGLE_ID    = 3;
uint DRAW_MESH_MODE_TRIANGLE_EDGES = 4;
void DRAW_MESH(uint mode, mat4 P, mat4 V, mat4 M, DrawMesh *mesh) {
    mat4 C = inverse(V);
    vec3 eye_World = { C(0, 3), C(1, 3), C(2, 3) };
    mat4 PV = P * V;
    mat4 PVM = PV * M;

    // NOTE: glPolygonMode doesn't work here (it does NOT change the primitive)
    // TODO: custom element buffer object for lines

    if (
            (mode == DRAW_MESH_MODE_TRIANGLE_EDGES) ||
            (mode == DRAW_MESH_MODE_PATCH_EDGES)
       ) {

        uint EBO;
        uint num_vertices;
        if (mode == DRAW_MESH_MODE_TRIANGLE_EDGES) {
            EBO = GL.EBO_all_edges;
            uint mesh_num_half_edges = (3 * mesh->num_triangles);
            num_vertices = (2 * mesh_num_half_edges);
        } else {
            EBO = GL.EBO_hard_edges;
            num_vertices = (2 * mesh->num_hard_half_edges);
        }

        glBindVertexArray(GL.VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        uint shader_program = edge_shader_program; ASSERT(shader_program); glUseProgram(shader_program);

        static real __t01;
        static real _t01;
        __t01 += 0.0167f;
        _t01 = 0.5 - 0.5 * COS(__t01);
        glUniform1f(UNIFORM(shader_program, "_t01"), _t01);

        glUniformMatrix4fv(UNIFORM(shader_program, "PVM"), 1, GL_TRUE, PVM.data);
        glUniform2f       (UNIFORM(shader_program, "OpenGL_from_Pixel_scale"), 2.0f / window_get_width_Pixel(), 2.0f / window_get_height_Pixel());
        glDrawElements(GL_LINES, num_vertices, GL_UNSIGNED_INT, NULL);
    } else if (
            (mode == DRAW_MESH_MODE_LIT) ||
            (mode == DRAW_MESH_MODE_TRIANGLE_ID) ||
            (mode == DRAW_MESH_MODE_PATCH_ID)
            ) {
        glBindVertexArray(GL.VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL.EBO_faces);
        uint shader_program = face_shader_program; ASSERT(shader_program); glUseProgram(shader_program);
        glUniformMatrix4fv(UNIFORM(shader_program, "PV"), 1, GL_TRUE, PV.data);
        glUniformMatrix4fv(UNIFORM(shader_program, "M" ), 1, GL_TRUE, M.data);
        glUniform3f       (UNIFORM(shader_program, "eye_World"), eye_World.x, eye_World.y, eye_World.z);
        glUniform1i(UNIFORM(shader_program, "mode"), mode);
        glDrawElements(GL_TRIANGLES, 3 * mesh->num_triangles, GL_UNSIGNED_INT, NULL);
    } else {
        ASSERT(0);
    }


}


void fancy_draw(mat4 P, mat4 V, mat4 M, DrawMesh *mesh) {
    // DRAW_MESH(DRAW_MESH_MODE_LIT,            P, V, M4_Translation(-00.0, 0.0,  00.0) * M, mesh);



    // TODO: bring back FBO, RBO, etc.
    DRAW_MESH(DRAW_MESH_MODE_PATCH_ID,       P, V, M4_Translation( 00.0, 0.0, -60.0) * M, mesh);
    DRAW_MESH(DRAW_MESH_MODE_PATCH_EDGES,    P, V, M4_Translation( 00.0, 0.0,  00.0) * M, mesh);

    DRAW_MESH(DRAW_MESH_MODE_PATCH_ID,       P, V, M4_Translation( 00.0, 0.0, -60.0) * M, mesh);
    DRAW_MESH(DRAW_MESH_MODE_TRIANGLE_ID,    P, V, M4_Translation( 80.0, 0.0, -60.0) * M, mesh);
    DRAW_MESH(DRAW_MESH_MODE_TRIANGLE_EDGES, P, V, M4_Translation( 80.0, 0.0,  00.0) * M, mesh);

    // _fancy_draw_all_edge_pass(P, V, M, mesh);
    // _fancy_draw_hard_edges_pass(P, V, M, mesh);
    // TODO(?):_fancy_draw_silhouette_edge_pass(P, V, M, mesh);
}



































