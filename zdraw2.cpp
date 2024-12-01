
















#if 0
struct {
    char *vert = R""(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoords;

        out vec2 TexCoords;

        void main()
        {
            gl_Position = vec4(aPos.x, aPos.y, 0.1, 1.0); // FORNOW: 0.1 (instead of 0.0) z fixes thrashing(?) of gui elements
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
} texture_blit;

run_before_main {
    uint vert = shader_compile(texture_blit_source.vert, GL_VERTEX_SHADER);
    uint frag = shader_compile(texture_blit_source.frag, GL_FRAGMENT_SHADER);
    texture_blit.shader_program = shader_build_program(vert, 0, frag);
};


// TODO: this
struct {
    char *vert = R""(
        #version 330 core
        layout (location = 0) in vec3 position_Model;
        layout (location = 1) in  int    triangle_ID;

        uniform mat4 P;
        uniform mat4 V;
        uniform mat4 M;

        void main() {
            gl_Position = P * V * M * vec4(position_Model, 1.0);
        }
    )"";

    char *frag_ID_BUFFER = R""(
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
} ID_buffer_pass_source;

struct {
} ID_buffer_pass;




struct {
    int shader_program;
    uint VAO;
    uint VBO[2];
    uint FBO;
    uint RBO;
    uint texture;
} all_edge_pass;

run_before_main {
    uint vert = shader_compile(all_edge_pass_source.vert, GL_VERTEX_SHADER);
    // uint frag_ID_BUFFER = shader_compile(all_edge_pass_source.frag_LINES, GL_FRAGMENT_SHADER);
    uint geom_LINES = shader_compile(all_edge_pass_source.geom_LINES, GL_GEOMETRY_SHADER);
    uint frag_LINES = shader_compile(all_edge_pass_source.frag_LINES, GL_FRAGMENT_SHADER);
    all_edge_pass.shader_program = shader_build_program(vert, geom_LINES, frag_LINES);
    glGenVertexArrays(1, &all_edge_pass.VAO);
    glGenBuffers(ARRAY_LENGTH(all_edge_pass.VBO), all_edge_pass.VBO);

    glGenFramebuffers(1, &all_edge_pass.FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, all_edge_pass.FBO);

    glGenTextures(1, &all_edge_pass.texture);

    uint width = _window_macbook_retina_fixer__VERY_MYSTERIOUS * window_get_width_Pixel();
    uint height = _window_macbook_retina_fixer__VERY_MYSTERIOUS * window_get_height_Pixel();

    glActiveTexture(GL_TEXTURE0); // ?
    glBindTexture(GL_TEXTURE_2D, all_edge_pass.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, all_edge_pass.texture, 0);  

    glGenRenderbuffers(1, &all_edge_pass.RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, all_edge_pass.RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, all_edge_pass.RBO);

    ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
};

void _fancy_draw_all_edge_pass(mat4 P, mat4 V, mat4 M, DrawMesh *mesh) {
    { // TODO: render draw the ID buffer
        glDisable(GL_SCISSOR_TEST);
        uint FBO = all_edge_pass.FBO;
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            // TODO: need to draw flat-shaded (soup mesh) colored by triangle index
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_SCISSOR_TEST);
    }

    if (0) {
        uint num_verts = 2 * (3 * mesh->num_triangles);

        uint shader_program = all_edge_pass.shader_program;
        uint  VAO = all_edge_pass.VAO;
        uint *VBO = all_edge_pass.VBO;
        uint  FBO = all_edge_pass.FBO;

        ASSERT(shader_program);
        ASSERT(VAO);
        ASSERT(FBO);

        glUseProgram(shader_program);

        glBindVertexArray(VAO);
        POOSH(VBO, 0, num_verts, mesh->all_edges_vertex_positions);
        POOSH(VBO, 1, num_verts, mesh->all_edges_corresponding_triangle_indices);

        glUniformMatrix4fv(UNIFORM(shader_program, "P"), 1, GL_TRUE, P.data);
        glUniformMatrix4fv(UNIFORM(shader_program, "V"), 1, GL_TRUE, V.data);
        glUniformMatrix4fv(UNIFORM(shader_program, "M"), 1, GL_TRUE, M.data);
        glUniform2f       (UNIFORM(shader_program, "OpenGL_from_Pixel_scale"), 2.0f / window_get_width_Pixel(), 2.0f / window_get_height_Pixel());

        #if 0
        glDrawArrays(GL_LINES, 0, num_verts);
        #else
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        {

            glClearColor(1.0f, 1.0f, 0.0f, 1.0f); 
            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_LINES, 0, num_verts);

        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // glClearColor(1.0f, 0.0f, 1.0f, 1.0f); 
        // glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_SCISSOR_TEST);
        glEnable(GL_DEPTH_TEST);
        #endif
    }

    {
        uint shader_program = texture_blit.shader_program;
        ASSERT(shader_program);
        glUseProgram(shader_program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, all_edge_pass.texture);
        glUniform1i(UNIFORM(shader_program, "screenTexture"), 0);

        glBindVertexArray(blit_quad.VAO);
        glDrawArrays(GL_TRIANGLES, 0, blit_quad.num_vertices);

        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }
}


void _fancy_draw_hard_edges_pass(mat4 P, mat4 V, mat4 M, DrawMesh *mesh) {
    uint hard_edges_num_vertices = (2 * mesh->num_hard_edges);
    eso_begin(P * V * M, SOUP_LINES);
    eso_color(pallete.black);
    eso_size(2.0f);
    for_(k, hard_edges_num_vertices) {
        eso_vertex(mesh->hard_edges_vertex_positions[k]);
    }
    eso_end();
}


#endif
