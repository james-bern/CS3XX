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

uint UNIFORM(uint shader_program, char *name) { return glGetUniformLocation(shader_program, name); };
void POOSH(
        uint vertex_attribute_index,
        uint VBO,
        void *array,
        uint num_verts,
        uint dim,
        int sizeof_type,
        int GL_TYPE
        ) {
    glDisableVertexAttribArray(vertex_attribute_index);
    if (array) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, num_verts * dim * sizeof_type, array, GL_DYNAMIC_DRAW);

        if      (GL_TYPE == GL_FLOAT) glVertexAttribPointer (vertex_attribute_index, dim, GL_TYPE, GL_FALSE, 0, NULL);
        else if (GL_TYPE == GL_INT)   glVertexAttribIPointer(vertex_attribute_index, dim, GL_TYPE,           0, NULL);
        else                          ASSERT(0);

        glEnableVertexAttribArray(vertex_attribute_index);
    }
};

void _fancy_draw_face_pass(mat4 P, mat4 V, mat4 M, DrawMesh *mesh) {
    mat4 C = inverse(V);
    vec3 eye_World = { C(0, 3), C(1, 3), C(2, 3) };


    uint shader_program = face_pass.shader_program;
    uint  VAO = face_pass.VAO;
    uint *VBO = face_pass.VBO;
    uint  EBO = face_pass.EBO;


    ASSERT(shader_program);
    glUseProgram(shader_program);

    glBindVertexArray(VAO);
    POOSH(0, VBO[0], mesh->vertex_positions, mesh->num_vertices, 3, sizeof(real), GL_FLOAT);
    POOSH(1, VBO[1], mesh->vertex_normals,   mesh->num_vertices, 3, sizeof(real), GL_FLOAT);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * mesh->num_triangles * sizeof(uint), mesh->triangle_tuples, GL_DYNAMIC_DRAW);

    glUniformMatrix4fv(UNIFORM(shader_program, "P"), 1, GL_TRUE, P.data);
    glUniformMatrix4fv(UNIFORM(shader_program, "V"), 1, GL_TRUE, V.data);
    glUniformMatrix4fv(UNIFORM(shader_program, "M"), 1, GL_TRUE, M.data);
    glUniform3f       (UNIFORM(shader_program, "eye_World"), eye_World.x, eye_World.y, eye_World.z);

    glDrawElements(GL_TRIANGLES, 3 * mesh->num_triangles, GL_UNSIGNED_INT, NULL);
}


struct {
    char *vert = R""(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoords;

        out vec2 TexCoords;

        void main()
        {
            gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
            TexCoords = aTexCoords;
        }  
    )"";

    char *frag = R""(
        #version 330 core
        out vec4 _gl_FragColor;

        in vec2 TexCoords;

        uniform sampler2D screenTexture;

        void main()
        { 
            _gl_FragColor = texture(screenTexture, TexCoords);
        }
    )"";
} texture_blit_source;

struct {
    int shader_program;
    uint VAO;
    uint VBO[2];
} texture_blit;

run_before_main {
    uint vert = shader_compile(texture_blit_source.vert, GL_VERTEX_SHADER);
    uint frag = shader_compile(texture_blit_source.frag, GL_FRAGMENT_SHADER);
    texture_blit.shader_program = shader_build_program(vert, 0, frag);
    glGenVertexArrays(1, &texture_blit.VAO);
    glGenBuffers(ARRAY_LENGTH(texture_blit.VBO), texture_blit.VBO);
};


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

        void main() {    
            vec4 s = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
            vec4 t = gl_in[1].gl_Position / gl_in[1].gl_Position.w;

            float size = 3.0f;

            vec2 perp = OpenGL_from_Pixel_scale * normalize(OpenGL_from_Pixel_scale * vec2(s.y - t.y, t.x - s.x));
            vec4 perp_s = vec4((size / 2) * perp, 0, 0);
            vec4 perp_t = vec4((size / 2) * perp, 0, 0);

            gl_Position = (s - perp_s) * gl_in[0].gl_Position.w; EmitVertex();
            gl_Position = (t - perp_t) * gl_in[1].gl_Position.w; EmitVertex();
            gl_Position = (s + perp_s) * gl_in[0].gl_Position.w; EmitVertex();
            gl_Position = (t + perp_t) * gl_in[1].gl_Position.w; EmitVertex();

            EndPrimitive();
        }  
    )"";

    char *frag = R""(#version 330 core
        out vec4 _gl_FragColor;
        void main() {
            _gl_FragColor = vec4(1.0);
        }
    )"";
} all_edge_pass_source;

struct {
    int shader_program;
    uint VAO;
    uint VBO[2];

    uint FBO;
    uint texture;
} all_edge_pass;

run_before_main {
    uint vert = shader_compile(all_edge_pass_source.vert, GL_VERTEX_SHADER);
    uint geom = shader_compile(all_edge_pass_source.geom, GL_GEOMETRY_SHADER);
    uint frag = shader_compile(all_edge_pass_source.frag, GL_FRAGMENT_SHADER);
    all_edge_pass.shader_program = shader_build_program(vert, geom, frag);
    glGenVertexArrays(1, &all_edge_pass.VAO);
    glGenBuffers(ARRAY_LENGTH(all_edge_pass.VBO), all_edge_pass.VBO);

    glGenFramebuffers(1, &all_edge_pass.FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, all_edge_pass.FBO);

    glGenTextures(1, &all_edge_pass.texture);
    glBindTexture(GL_TEXTURE_2D, all_edge_pass.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, all_edge_pass.texture, 0);  

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
};


void _fancy_draw_all_edge_pass_A(mat4 P, mat4 V, mat4 M, DrawMesh *mesh) {
    uint num_verts = 2 * (3 * mesh->num_triangles);

    uint shader_program = all_edge_pass.shader_program;
    uint  VAO = all_edge_pass.VAO;
    uint *VBO = all_edge_pass.VBO;
    uint  FBO = all_edge_pass.FBO;

    ASSERT(shader_program);
    glUseProgram(shader_program);

    glBindVertexArray(VAO);
    POOSH(0, VBO[0], mesh->all_edges_vertex_positions,               num_verts, 3, sizeof(real), GL_FLOAT);
    POOSH(1, VBO[1], mesh->all_edges_corresponding_triangle_indices, num_verts, 1, sizeof(uint), GL_INT  );

    glUniformMatrix4fv(UNIFORM(shader_program, "P"), 1, GL_TRUE, P.data);
    glUniformMatrix4fv(UNIFORM(shader_program, "V"), 1, GL_TRUE, V.data);
    glUniformMatrix4fv(UNIFORM(shader_program, "M"), 1, GL_TRUE, M.data);
    glUniform2f       (UNIFORM(shader_program, "OpenGL_from_Pixel_scale"), 2.0f / window_get_width_Pixel(), 2.0f / window_get_height_Pixel());

    #if 0
    glDrawArrays(GL_LINES, 0, num_verts);
    #else
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    {

        glClearColor(1.0f, 1.0f, 0.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);
        // glDrawArrays(GL_LINES, 0, num_verts);

    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT);
    #endif
}

void _fancy_draw_all_edge_pass_B() {
    uint num_verts = 6;
    vec2 vertex_positions[] = {
        { -1.0f, -1.0f },
        {  1.0f, -1.0f },
        {  1.0f,  1.0f },
        { -1.0f, -1.0f },
        {  1.0f,  1.0f },
        { -1.0f,  1.0f },
    };
    vec2 vertex_texture_coordinates[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
    };

    uint shader_program = texture_blit.shader_program;
    uint  VAO = texture_blit.VAO;
    uint *VBO = texture_blit.VBO;

    ASSERT(shader_program);
    glUseProgram(shader_program);

    glBindVertexArray(VAO);
    POOSH(0, VBO[0], vertex_positions,           num_verts, 2, sizeof(real), GL_FLOAT);
    POOSH(1, VBO[1], vertex_texture_coordinates, num_verts, 2, sizeof(real), GL_FLOAT);
    glUniform1ui(UNIFORM(shader_program, "screenTexture"), all_edge_pass.texture);

    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, all_edge_pass.texture); // ?
    glDrawArrays(GL_TRIANGLES, 0, num_verts);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
}




void fancy_draw(mat4 P, mat4 V, mat4 M, DrawMesh *mesh) {
    _fancy_draw_face_pass(P, V, M, mesh);
    _fancy_draw_all_edge_pass_A(P, V, M, mesh);
    _fancy_draw_all_edge_pass_B();
    // _fancy_draw_hard_edge_pass(P, V, M, mesh);
    // TODO(?):_fancy_draw_silhouette_edge_pass(P, V, M, mesh);
}
