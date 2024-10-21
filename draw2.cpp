struct {
    char *vert = R""(
        #version 330 core
        layout (location = 0) in vec3 position_Model;
        layout (location = 1) in vec3 normal_Model;

        out BLOCK {
            vec3 position_World;
            vec3 normal_World;
        } vs_out;

        uniform mat4 P;
        uniform mat4 V;
        uniform mat4 M;

        void main() {

            {
                vec4 tmp = vec4(position_Model, 1.0);
                tmp = M * tmp;
                vs_out.position_World = vec3(tmp);
                gl_Position = P * V * tmp;
            }

            {
                vec3 tmp = normal_Model;
                tmp = inverse(transpose(mat3(M))) * tmp;
                vs_out.normal_World = tmp;
            }

        }
    )"";

    char *frag = R""(
        #version 330 core
        in BLOCK {
            vec3 position_World;
            vec3 normal_World;
        } fs_in;

        uniform vec3 eye_World;

        uniform bool has_vertex_normals;

        out vec4 _gl_FragColor;

        void main() {
            vec3 N = normalize(fs_in.normal_World);
            vec3 rgb = vec3(0.0);


        vec3 warm_color = vec3(1.0, 1.0, 0.3);
        vec3 cool_color = vec3(0.3, 0.7, 1.0);
        // sunlight gooch
        {
            vec3 L = vec3(0.0, 1.0, 0.0);
            float LN = dot(L, N);
            float t = 0.5 + 0.5 * LN;
            rgb += 0.7 * mix(cool_color, warm_color, t);
        }

        // eye light phong fresnel
        {
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

    _gl_FragColor = vec4(rgb, 1.0);
}
)"";
    } face_pass_source;

struct {
    int shader_program;
    uint VAO;
    uint VBO[6];
    uint EBO;
} face_pass;

run_before_main {
    uint vert = shader_compile(face_pass_source.vert, GL_VERTEX_SHADER);
    uint frag = shader_compile(face_pass_source.frag, GL_FRAGMENT_SHADER);
    face_pass.shader_program = shader_build_program(vert, 0, frag);
    glGenVertexArrays(1, &face_pass.VAO);
    glGenBuffers(ARRAY_LENGTH(face_pass.VBO), face_pass.VBO);
    glGenBuffers(1, &face_pass.EBO);
};

void _fancy_draw_face_pass(mat4 P, mat4 V, mat4 M, DrawMesh *mesh) {
    ASSERT(face_pass.shader_program);
    glUseProgram(face_pass.shader_program);

    glBindVertexArray(face_pass.VAO);
    unsigned int vertex_attribute_index = 0;
    auto POOSH = [&](void *array, int dim, int sizeof_type, int GL_TYPE_VAR) {
        ASSERT(vertex_attribute_index < ARRAY_LENGTH(face_pass.VBO));
        glDisableVertexAttribArray(vertex_attribute_index);
        if (array) {
            glBindBuffer(GL_ARRAY_BUFFER, face_pass.VBO[vertex_attribute_index]);
            glBufferData(GL_ARRAY_BUFFER, mesh->num_vertices * dim * sizeof_type, array, GL_DYNAMIC_DRAW);
            if (GL_TYPE_VAR == GL_FLOAT) {
                glVertexAttribPointer(vertex_attribute_index, dim, GL_TYPE_VAR, GL_FALSE, 0, NULL);
            } else if (GL_TYPE_VAR == GL_INT) {
                glVertexAttribIPointer(vertex_attribute_index, dim, GL_TYPE_VAR, 0, NULL);
            } else {
                ASSERT(0);
            }
            glEnableVertexAttribArray(vertex_attribute_index); // FORNOW: after the other stuff?
        }
        ++vertex_attribute_index;
    };

    POOSH(mesh->vertex_positions, 3, sizeof(real), GL_FLOAT);
    POOSH(mesh->vertex_normals, 3, sizeof(real), GL_FLOAT);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_pass.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * mesh->num_triangles * sizeof(uint), mesh->triangle_tuples, GL_DYNAMIC_DRAW);

    auto LOC = [&](char *name) { return glGetUniformLocation(face_pass.shader_program, name); };
    glUniformMatrix4fv(LOC("P"), 1, GL_TRUE, P.data);
    glUniformMatrix4fv(LOC("V"), 1, GL_TRUE, V.data);
    glUniformMatrix4fv(LOC("M"), 1, GL_TRUE, M.data);
    { // fornow scavenge the camera position from V
        mat4 C = inverse(V);
        vec3 eye_World = { C(0, 3), C(1, 3), C(2, 3) };
        glUniform3f(LOC("eye_World"), eye_World.x, eye_World.y, eye_World.z);
    }

    glDrawElements(GL_TRIANGLES, 3 * mesh->num_triangles, GL_UNSIGNED_INT, NULL);
}




struct {
    char *vert = R""(
        #version 330 core
        layout (location = 0) in vec3 position_Model;
        layout (location = 1) in  int triangle_ID;

        uniform mat4 P;
        uniform mat4 V;
        uniform mat4 M;

        void main() {
            gl_Position = P * V * M * vec4(position_Model, 1.0);
        }
    )"";

    char *geom = R""(#version 330 core
        layout (lines) in;
        layout (triangle_strip, max_vertices = 4) out;

        uniform vec2 OpenGL_from_Pixel_scale;

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
            vec4 color_s = vec4(1.0);
            vec4 color_t = vec4(1.0);

            float size = 3.0f;

            float angle = atan(OpenGL_from_Pixel_scale.x * (t.y - s.y), OpenGL_from_Pixel_scale.y * (t.x - s.x));

            vec2 perp = OpenGL_from_Pixel_scale * normalize(OpenGL_from_Pixel_scale * vec2(s.y - t.y, t.x - s.x));
            vec4 perp_s = vec4((size / 2) * perp, 0, 0);
            vec4 perp_t = vec4((size / 2) * perp, 0, 0);

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

    char *frag = R""(#version 330 core
        in BLOCK {
            vec4 color;
            float size;
            vec2 position_Pixel;
            float angle;
            vec2 starting_point_Pixel;
        } fs_in;

        out vec4 _gl_FragColor;

        void main() {
            _gl_FragColor = fs_in.color;
        }
    )"";
} all_edge_pass_source;

struct {
    int shader_program;
    uint VAO;
    uint VBO[2];
} all_edge_pass;

run_before_main {
    uint vert = shader_compile(all_edge_pass_source.vert, GL_VERTEX_SHADER);
    uint geom = shader_compile(all_edge_pass_source.geom, GL_GEOMETRY_SHADER);
    uint frag = shader_compile(all_edge_pass_source.frag, GL_FRAGMENT_SHADER);
    all_edge_pass.shader_program = shader_build_program(vert, geom, frag);
    glGenVertexArrays(1, &all_edge_pass.VAO);
    glGenBuffers(ARRAY_LENGTH(all_edge_pass.VBO), all_edge_pass.VBO);
};

void _fancy_draw_all_edge_pass(mat4 P, mat4 V, mat4 M, DrawMesh *mesh) {
    #if 1 ////////////////////////////////////////////////////////////////////////////////
    ASSERT(all_edge_pass.shader_program);
    glUseProgram(all_edge_pass.shader_program);
    glBindVertexArray(all_edge_pass.VAO);
    unsigned int vertex_attribute_index = 0;
    auto POOSH = [&](void *array, uint count, uint dim, int sizeof_type, int GL_TYPE_VAR) {
        ASSERT(vertex_attribute_index < ARRAY_LENGTH(all_edge_pass.VBO));
        glDisableVertexAttribArray(vertex_attribute_index);
        if (array) {
            glBindBuffer(GL_ARRAY_BUFFER, all_edge_pass.VBO[vertex_attribute_index]);
            glBufferData(GL_ARRAY_BUFFER, count * dim * sizeof_type, array, GL_DYNAMIC_DRAW);

            if      (GL_TYPE_VAR == GL_FLOAT) glVertexAttribPointer (vertex_attribute_index, dim, GL_TYPE_VAR, GL_FALSE, 0, NULL);
            else if (GL_TYPE_VAR == GL_INT)   glVertexAttribIPointer(vertex_attribute_index, dim, GL_TYPE_VAR,           0, NULL);
            else                              ASSERT(0);

            glEnableVertexAttribArray(vertex_attribute_index);
        }
        ++vertex_attribute_index;
    };
    auto LOC = [&](char *name) { return glGetUniformLocation(all_edge_pass.shader_program, name); };

    uint num_vertices_all_edges = 2 * (3 * mesh->num_triangles);
    POOSH(mesh->all_edges_vertex_positions,               num_vertices_all_edges, 3, sizeof(real), GL_FLOAT);
    POOSH(mesh->all_edges_corresponding_triangle_indices, num_vertices_all_edges, 1, sizeof(uint), GL_INT  );

    glUniformMatrix4fv(LOC("P"), 1, GL_TRUE, P.data);
    glUniformMatrix4fv(LOC("V"), 1, GL_TRUE, V.data);
    glUniformMatrix4fv(LOC("M"), 1, GL_TRUE, M.data);
    glUniform2f(LOC("OpenGL_from_Pixel_scale"), 2.0f / window_get_width_Pixel(), 2.0f / window_get_height_Pixel());

    glDrawArrays(GL_LINES, 0, num_vertices_all_edges);
    #else ////////////////////////////////////////////////////////////////////////////////
    eso_begin(P * V * M, SOUP_LINES);
    eso_color(0.0f, 0.0f, 0.0f);
    uint num_edges = 3 * mesh->num_triangles;
    for_(i, 2 * num_edges) {
        eso_vertex(mesh->all_edges_vertex_positions[i]);
    }
    eso_end();
    #endif ////////////////////////////////////////////////////////////////////////////////
}




void fancy_draw(mat4 P, mat4 V, mat4 M, DrawMesh *mesh) {
    _fancy_draw_face_pass(P, V, M, mesh);
    _fancy_draw_all_edge_pass(P, V, M, mesh);
    // _fancy_draw_hard_edge_pass(P, V, M, mesh);
    // TODO(?):_fancy_draw_silhouette_edge_pass(P, V, M, mesh);
}
