// TODO: re-read the paper to see what it says about curvy polygons (maybe they just didn't consider this case)
// -- TODO: can pass corresponding triangle normals and see if that helps (the WorkMesh already has these computed; will have to do some work to hook them up, but not too much)
// TODO: wait, does this even make sense? i don't think so. our triangles aren't actually exploded. we're just using ebos to go over the edges
// there will be some weird geometry shader solution, but it's non-obvious fornow (passing around the third vertex

// // !!!
// NO this bad bad (see problem.stl) -- consider throwing away patch version and just using all edges but turning off some of them (modulating thickness) based on whether they're a patch edge. could pass into for each one to say whether it's a hard half edge or not
// this is going to be an annoying change because i don't think you can pass whether it is a hard edge as a vertex attribute with the current pipeline
// right now we just have all our vertices and we pass a triangle explode EBO as lines
// we probably need to the data actually exploded into triangle soup labeled with "opposing edge is hard" or something like that, which isn't hard, just mildly annoying; or we could just go really inefficient and throw labeled line soup at the problem, which would require minimal modification to the current edge shader pipeline. this is probably the easiest thing to do; it's also a very straight forward idea. just upload a bunch of lines to the GPU, labeled with whether they're hard. no need for a patch ID buffer. and we can definitely single pass shade it. it's bad in terms of space, but that's also probably fine. so we need a massive bool *all_half_edges_soup_per_vertex_is_hard_half_edge; and then we'll also need all_half_edges_soup_vertex_positions; using an EBO with the same data used for smooth-shading the mesh was a cute idea, but I think we need to let it go
// or actually, i think you could just use both textures. if it matches the patch id and it matches the face id, then you're golden

// I think the obvious approach is backface culling (we need to look up when in the pipeline that happens)
// - if it's early, then we rewrite this crap to make the geometry shader emit 3 lines per triangle (just a for loop)--but skip this if the triangle is facing away from us.
// we might be able to condense the hard edges and all edges into a single pass if we do this, but let's not worry about that fornow. priority now is any hard edge solution that doesn't exhibit the problem we have now


// TODO: this doesn't work if you resize the window
// TODO: get rid of STENCIL
// NOTE: VAO and VBO are about mesh data (don't connect them to shaders)

struct {
    uint num_vertices = 6;
    uint VAO;
    uint VBO[2];
    vec2 vertex_positions[6] = { { -1.0f, -1.0f }, {  1.0f, -1.0f }, {  1.0f,  1.0f }, { -1.0f, -1.0f }, {  1.0f,  1.0f }, { -1.0f,  1.0f }, };
    vec2 vertex_texture_coordinates[6] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }, };
} FullScreenQuad;

run_before_main {
    glGenVertexArrays(1, &FullScreenQuad.VAO);
    glGenBuffers(ARRAY_LENGTH(FullScreenQuad.VBO), FullScreenQuad.VBO);
    glBindVertexArray(FullScreenQuad.VAO);
    POOSH(FullScreenQuad.VBO, 0, FullScreenQuad.num_vertices, FullScreenQuad.vertex_positions);
    POOSH(FullScreenQuad.VBO, 1, FullScreenQuad.num_vertices, FullScreenQuad.vertex_texture_coordinates);
};


struct {
    uint FBO;
    uint RBO;
    uint TextureID;
} GL2;

run_before_main {
    glGenFramebuffers(1, &GL2.FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, GL2.FBO);

    glGenTextures(1, &GL2.TextureID);

    uint width = _window_macbook_retina_fixer__VERY_MYSTERIOUS * window_get_width_Pixel();
    uint height = _window_macbook_retina_fixer__VERY_MYSTERIOUS * window_get_height_Pixel();

    glActiveTexture(GL_TEXTURE0); // ?
    glBindTexture(GL_TEXTURE_2D, GL2.TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GL2.TextureID, 0);  

    glGenRenderbuffers(1, &GL2.RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, GL2.RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, GL2.RBO);

    ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
};


struct {
    char *vert = R""(
        #version 330 core
        layout (location = 0) in vec3 position_Model;
        layout (location = 1) in vec3 normal_Model;
        layout (location = 2) in uint patch_index;

        out BLOCK {
            vec3 position_World;
            vec3 normal_World;
            flat uint patch_index;
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
                vs_out.patch_index = patch_index;
            }
        }
    )"";

    char *frag = R""(
        #version 330 core
        in BLOCK {
            vec3 position_World;
            vec3 normal_World;
            flat uint patch_index;
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
                int i = (mode == 1) ? int(fs_in.patch_index) : gl_PrimitiveID;
                rgb.r = (i % 256);
                rgb.g = ((i / 256) % 256);
                rgb.b = ((i / (256 * 256)) % 256);
                rgb /= 255.0;

            }

            _gl_FragColor = vec4(rgb, 1.0);
        }
    )"";
} face_pass_source;



// TODO: distinguish between Clip and NDC

struct {
    char *vert = R""(
        #version 330 core
        layout (location = 0) in vec3 position_Model;
        layout (location = 1) in vec3 normal_Model;
        layout (location = 2) in uint patch_index;

        uniform mat4 PVM;

        out BLOCK {
            flat uint patch_index;
        } vs_out;

        void main() {
            normal_Model;
            vs_out.patch_index = patch_index;
            gl_Position = PVM * vec4(position_Model, 1.0);
        }
    )"";

    char *geom = R""(#version 330 core
        // https://backend.orbit.dtu.dk/ws/portalfiles/portal/3735323/wire-sccg.pdf

        layout (lines) in;
        layout (triangle_strip, max_vertices = 4) out;

        uniform vec2 OpenGL_from_Pixel_scale;
        uniform float _t01;

        // TODO: lines needs to know their patch index


        in BLOCK {
            flat uint patch_index;
        } gs_in[];

        out BLOCK {
            noperspective vec3 color;
            noperspective float L;
            noperspective vec2 corner; // (+/-1, +/-1)
            flat uint patch_index;
        } gs_out;

        float _half_thickness = 0.5f;
        uniform int mode;

        void main() {    
            float half_thickness = (mode == 4) ? _half_thickness : 2 * _half_thickness;

            vec4 v0_NDC = gl_in[0].gl_Position;
            vec4 v1_NDC = gl_in[1].gl_Position;
            float w0 = 1.0 / v0_NDC.w; // NOTE: This is actually how OpenGL works (TODO: link)
            float w1 = 1.0 / v1_NDC.w; // ----------------------------------------------------
            vec4 v0_Clip = v0_NDC * w0;
            vec4 v1_Clip = v1_NDC * w1;

            vec4 S = vec4(OpenGL_from_Pixel_scale, 1, 1);
            vec4 v0 = v0_Clip / S; // _Window (Pixel coordinates with origin at center of screen and no y flip)
            vec4 v1 = v1_Clip / S;

            vec4 a = vec4(normalize(v1.xy - v0.xy), 0, 0); // tangent
            vec4 b = vec4(-a.y, a.x, 0, 0);                // normal

            // a = vec4((32.0 * _t01) * normalize(v1.xy - v0.xy), 0, 0); // TESTING

            float L = (length(v0.xy - v1.xy) / half_thickness);

            gs_out.L = L;
            gs_out.corner = vec2(-1, +1);
            gs_out.color = vec3(1.0, 0.0, 0.0);
            gs_out.patch_index = gs_in[0].patch_index;
            gl_Position = (S * (v0 + half_thickness * (-a + b))) / w0;
            EmitVertex();

            gs_out.L = L;
            gs_out.corner  = vec2(-1, -1);
            gs_out.color = vec3(1.0, 0.0, 0.0);
            gs_out.patch_index = gs_in[0].patch_index;
            gl_Position = (S * (v0 + half_thickness * (-a - b))) / w0;
            EmitVertex();

            gs_out.L = L;
            gs_out.corner = vec2(+1, +1);
            gs_out.color = vec3(1.0, 0.0, 0.0);
            gs_out.patch_index = gs_in[1].patch_index;
            gl_Position = (S * (v1 + half_thickness * (+a + b))) / w1;
            EmitVertex();

            gs_out.L = L;
            gs_out.corner = vec2(+1, -1);
            gs_out.color = vec3(1.0, 0.0, 0.0);
            gs_out.patch_index = gs_in[1].patch_index;
            gl_Position = (S * (v1 + half_thickness * (+a - b))) / w1;
            EmitVertex();

    EndPrimitive();
}  
)"";

char *frag = R""(#version 330 core
        float squaredDistance(vec2 a, vec2 b) {
            vec2 delta = (a - b);
            return dot(delta, delta);
        }

        float squaredDistancePointLineSegment(vec2 p, vec2 start, vec2 end) {
            float d2; {
                float l2 = squaredDistance(start, end);
                if (l2 < 0.0001) { // zero length edge
                    d2 = squaredDistance(p, start);
                } else {
                    float num = dot(p - start, end - start);
                    float t = clamp(num / l2, 0.0, 1.0);
                    vec2 q = mix(start, end, t);
                    d2 = squaredDistance(p, q);
                }
            }
            return d2;
        }

        in BLOCK {
            noperspective vec3 color;
            noperspective float L;
            noperspective vec2 corner;
            flat uint patch_index;
        } fs_in;

        uniform sampler2D TextureID;
        uniform vec2 OpenGL_from_Pixel_scale;
        uniform int mode;

        out vec4 _gl_FragColor;
        void main() {
            vec2 TexCoord_from_FragCoord = OpenGL_from_Pixel_scale / 2;
            vec2 texCoord = TexCoord_from_FragCoord * gl_FragCoord.xy / 2; // ?? TODO: work out where this factor is coming from
            vec3 rgb = texture(TextureID, texCoord).rgb;

            vec3 rgb2; {
                int i = (mode == 4) ? (gl_PrimitiveID / 3 / 2) : int(fs_in.patch_index); // NOTE: /2 is (probably) because the geometry shader emits two triangles per line
                rgb2.r = (i % 256);
                rgb2.g = ((i / 256) % 256);
                rgb2.b = ((i / (256 * 256)) % 256);
                rgb2 /= 255.0;
            }
            bool hit = (length(rgb - vec3(1.0)) > 0.0001);
            bool no_match = (length(rgb - rgb2) > 0.0001);
            // if (hit && no_match) discard;
            if (no_match) discard;
            // bool facing_away = (fs_in.triangle_normal.z < 0);
            // if (facing_away) discard;

            vec2 hLx = vec2(0.5 * fs_in.L, 0);
            float d2 = squaredDistancePointLineSegment(fs_in.corner * (hLx + vec2(1.0)), -hLx, hLx);
            float I = exp2(-2 * min(d2, 1.8 * 1.8));
            _gl_FragColor = vec4(vec3(1.0), I);
            // _gl_FragColor = vec4(mix(vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), I), 1.0); // FORNOW
            // _gl_FragColor.rgb = rgb;
        }
    )"";
} edge_pass_source;

int face_shader_program;
int edge_shader_program;



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
} blit_full_screen_quad_source;


uint blit_full_screen_quad_shader_program;

run_before_main {
    uint vert = shader_compile(blit_full_screen_quad_source.vert, GL_VERTEX_SHADER);
    uint frag = shader_compile(blit_full_screen_quad_source.frag, GL_FRAGMENT_SHADER);
    blit_full_screen_quad_shader_program = shader_build_program(vert, 0, frag);
};


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
            (mode == DRAW_MESH_MODE_LIT) ||
            (mode == DRAW_MESH_MODE_TRIANGLE_ID) ||
            (mode == DRAW_MESH_MODE_PATCH_ID)
       ) {
        uint num_vertices = 3 * mesh->num_triangles;

        glBindVertexArray(GL.VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL.EBO_faces);
        uint shader_program = face_shader_program; ASSERT(shader_program); glUseProgram(shader_program);
        glUniformMatrix4fv(UNIFORM(shader_program, "PV"), 1, GL_TRUE, PV.data);
        glUniformMatrix4fv(UNIFORM(shader_program, "M" ), 1, GL_TRUE, M.data);
        glUniform3f       (UNIFORM(shader_program, "eye_World"), eye_World.x, eye_World.y, eye_World.z);
        glUniform1i(UNIFORM(shader_program, "mode"), mode);

        glActiveTexture(GL_TEXTURE0); // ?
        glBindTexture(GL_TEXTURE_2D, GL2.TextureID);
        glUniform1i(UNIFORM(shader_program, "TextureID"), GL2.TextureID);


        glDrawElements(GL_TRIANGLES, num_vertices, GL_UNSIGNED_INT, NULL);
        // glBindTexture(GL_TEXTURE_2D, 0);
        // glActiveTexture(0); // ?
    } else if (
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
        glUniform2f(UNIFORM(shader_program, "OpenGL_from_Pixel_scale"), 2.0f / window_get_width_Pixel(), 2.0f / window_get_height_Pixel());
        glUniform1i(UNIFORM(shader_program, "mode"), mode);
        glDrawElements(GL_LINES, num_vertices, GL_UNSIGNED_INT, NULL);
    } else {
        ASSERT(0);
    }
}









void fancy_draw(mat4 P, mat4 V, mat4 M, DrawMesh *mesh) {
    // glCullFace(GL_BACK);
    //return;
    DRAW_MESH(DRAW_MESH_MODE_LIT, P, V, M, mesh);


    for_(pass, 2) {
        if (pass == 1) continue;
        if (other.show_details || (pass == 1)) {
            glDisable(GL_SCISSOR_TEST);
            glBindFramebuffer(GL_FRAMEBUFFER, GL2.FBO);
            {
                glClearColor(1.0, 1.0, 1.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                DRAW_MESH((pass == 0) ? DRAW_MESH_MODE_TRIANGLE_ID : DRAW_MESH_MODE_PATCH_ID, P, V, M, mesh);
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glEnable(GL_SCISSOR_TEST);

            glDisable(GL_DEPTH_TEST); {
                DRAW_MESH((pass == 0) ? DRAW_MESH_MODE_TRIANGLE_EDGES : DRAW_MESH_MODE_PATCH_EDGES, P, V, M, mesh);
            } glEnable(GL_DEPTH_TEST);
        }
    }

    if (0) {
        uint shader_program = blit_full_screen_quad_shader_program;
        ASSERT(shader_program);
        glUseProgram(shader_program);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, GL2.TextureID);
        glUniform1i(UNIFORM(shader_program, "screenTexture"), 0);

        glBindVertexArray(FullScreenQuad.VAO);
        glDrawArrays(GL_TRIANGLES, 0, FullScreenQuad.num_vertices);

        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
        return;
    }



    // DRAW_MESH(DRAW_MESH_MODE_PATCH_ID, P, V, M4_Translation( 00.0, 0.0, -60.0) * M, mesh);
    // DRAW_MESH(DRAW_MESH_MODE_PATCH_ID, P, V, M, mesh);
    // DRAW_MESH(DRAW_MESH_MODE_PATCH_EDGES, P, V, M, mesh);

    // _fancy_draw_all_edge_pass(P, V, M, mesh);
    // _fancy_draw_hard_edges_pass(P, V, M, mesh);
    // TODO(?):_fancy_draw_silhouette_edge_pass(P, V, M, mesh);
}



































