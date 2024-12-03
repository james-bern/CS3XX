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
    glGenRenderbuffers(1, &GL2.RBO);
    glGenTextures(1, &GL2.TextureID);

    glBindFramebuffer(GL_FRAMEBUFFER, GL2.FBO);
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
        uniform vec4 clipPlane1;
        uniform vec4 clipPlane2;

        void main() {

            {
                vec4 tmp = vec4(position_Model, 1.0);
                tmp = M * tmp;
                vs_out.position_World = vec3(tmp);
                gl_Position = PV * tmp;
                gl_ClipDistance[0] = dot(vec4(vec3(tmp), 1.0), clipPlane1);
                gl_ClipDistance[1] = dot(vec4(vec3(tmp), 1.0), clipPlane2);
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

        uniform int feature_plane_is_active;
        uniform vec3 feature_plane_normal;
        uniform float feature_plane_signed_distance_to_world_origin;

        uniform int mode;

        out vec4 _gl_FragColor;

        void main() {
            vec3 N = normalize(fs_in.normal_World);
            vec3 rgb = vec3(0.0);
            float a = 1.0;


            if (mode == 0) {

                vec3 rgb_gooch3 = vec3(0);
                if (true) { // three way rainbow gooch
                    for (int d = 0; d < 3; ++d) {
                        vec3 warm_color = vec3(0.0);
                        warm_color[d] = 1.0;

                        vec3 cool_color = (vec3(1.0) - warm_color) / 1.5;

                        vec3 L = vec3(0.0);
                        L[d] = 1.0;

                        float LN = dot(L, N);
                        float t = LN; // 0.5 + 0.5 * LN;
                        rgb_gooch3 += 0.3 * mix(vec3(1.0), mix(cool_color, warm_color, t), 0.8);
                    }
                }


                if (false) { // sunlight gooch
                    vec3 warm_color = vec3(1.0, 0.7, 0.3);
                    vec3 cool_color = vec3(0.3, 0.7, 1.0);
                    vec3 L = vec3(0.0, 1.0, 0.0);
                    float LN = dot(L, N);
                    float t = 0.5 + 0.5 * LN;
                    rgb += 0.7 * mix(cool_color, warm_color, t);
                }

                if (false) { // eyelight gooch
                    vec3 warm_color = vec3(1.0, 0.7, 0.3);
                    vec3 cool_color = vec3(0.3, 0.7, 1.0);
                    vec3 L = normalize(eye_World - fs_in.position_World);
                    float LN = dot(L, N);
                    float t = LN;
                    rgb += 0.7 * mix(cool_color, warm_color, t);
                }

                if (true) { // eye light phong fresnel
                    vec3 L = normalize(eye_World - fs_in.position_World);
                    vec3 E = normalize(eye_World - fs_in.position_World);
                    vec3 H = normalize(L + E);
                    float LN = dot(L, N);
                    float F0 = 0.05;
                    float diffuse = max(0.0, LN);
                    float specular = pow(max(0.0, dot(N, H)), 256);
                    float fresnel = F0 + (1 - F0) * pow(1.0 - max(0.0, dot(N, H)), 5);
    rgb += vec3(0.25, 0.35, 0.35);
    rgb += 0.55 * diffuse;
    // rgb += 0.2 * diffuse * rgb_gooch3;
    rgb += 0.3 * specular;
    rgb += 0.3 * fresnel;
    rgb = mix(rgb, vec3(1.0), 0.5f);
    rgb = 0.5 + 0.5 * N;
    // rgb = 0.5 + 0.5 * cos(fs_in.position_World);
}

if (feature_plane_is_active != 0) { // feature plane override
    if (dot(fs_in.normal_World, feature_plane_normal) > 0.99) {
        if (abs(dot(fs_in.position_World, feature_plane_normal) - feature_plane_signed_distance_to_world_origin) < 0.01) {
            rgb = mix(rgb, vec3(1.0), 0.8);
        }
    }
}

// rgb = clamp(vec3(0.0f), vec3(0.9f), rgb);
} else if ((mode == 1) || (mode == 3)) {
    int i = (mode == 1) ? int(fs_in.patch_index) : gl_PrimitiveID;
    rgb.r = (i % 256);
    rgb.g = ((i / 256) % 256);
    rgb.b = ((i / (256 * 256)) % 256);
    rgb /= 255.0;
}

_gl_FragColor = vec4(rgb, a);
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
        uniform float _window_macbook_retina_fixer__VERY_MYSTERIOUS;

        out vec4 _gl_FragColor;
        void main() {
            vec2 TexCoord_from_FragCoord = OpenGL_from_Pixel_scale / 2;
            vec2 texCoord = TexCoord_from_FragCoord * gl_FragCoord.xy / _window_macbook_retina_fixer__VERY_MYSTERIOUS; // ?? TODO: work out where this factor is coming from
            vec3 rgb = texture(TextureID, texCoord).rgb;

            vec3 rgb2; {
                int i = (mode == 4) ? (gl_PrimitiveID / 3 / 2) : int(fs_in.patch_index); // NOTE: /2 is (probably) because the geometry shader emits two triangles per line
                rgb2.r = (i % 256);
                rgb2.g = ((i / 256) % 256);
                rgb2.b = ((i / (256 * 256)) % 256);
                rgb2 /= 255.0;

                // TODO: version that draws lines based on discontinuities in the z buffer
            }
            bool hit = (length(rgb - vec3(1.0)) > 0.0001);
            bool no_match = (length(rgb - rgb2) > 0.0001);
            // if (hit && no_match) discard;
            if (no_match) discard;
            // bool facing_away = (fs_in.triangle_normal.z < 0);
            // if (facing_away) discard;

            vec2 hLx = vec2(0.5 * fs_in.L, 0);
            float d2 = squaredDistancePointLineSegment(fs_in.corner * (hLx + vec2(1.0)), -hLx, hLx);
            float I = 0.18 * exp2(-2 * min(d2, 1.8 * 1.8));
            _gl_FragColor = vec4(vec3(0.0), I);
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

        float squaredLength(vec3 a) {
            return dot(a, a);
        }

        void main() { 
            // _gl_FragColor = texture(screenTexture, TexCoords);

            // sobel
            float o = 0.00073; // TODO: OpenGL_from_Pixel_scale
            vec3 a00 = texture(screenTexture, TexCoords + vec2(-1 * o, -1 * o)).rgb;
            vec3 a01 = texture(screenTexture, TexCoords + vec2(-1 * o,  0 * o)).rgb;
            vec3 a02 = texture(screenTexture, TexCoords + vec2(-1 * o,  1 * o)).rgb;
            vec3 a10 = texture(screenTexture, TexCoords + vec2( 0 * o, -1 * o)).rgb;
            vec3 a11 = texture(screenTexture, TexCoords + vec2( 0 * o,  0 * o)).rgb;
            vec3 a12 = texture(screenTexture, TexCoords + vec2( 0 * o,  1 * o)).rgb;
            vec3 a20 = texture(screenTexture, TexCoords + vec2( 1 * o, -1 * o)).rgb;
            vec3 a21 = texture(screenTexture, TexCoords + vec2( 1 * o,  0 * o)).rgb;
            vec3 a22 = texture(screenTexture, TexCoords + vec2( 1 * o,  1 * o)).rgb;
            float Q = 16777216.0;
            float b00 = min(1.0, Q * squaredLength(a00 - a11));
            float b01 = min(1.0, Q * squaredLength(a01 - a11));
            float b02 = min(1.0, Q * squaredLength(a02 - a11));
            float b10 = min(1.0, Q * squaredLength(a10 - a11));
         // float b11 = min(1.0, Q * squaredLength(a11 - a11));
            float b12 = min(1.0, Q * squaredLength(a12 - a11));
            float b20 = min(1.0, Q * squaredLength(a20 - a11));
            float b21 = min(1.0, Q * squaredLength(a21 - a11));
            float b22 = min(1.0, Q * squaredLength(a22 - a11));
            float Gx = (b00 - b02) + 2 * (b10 - b12) + (b20 - b22);
            float Gy = (b00 - b20) + 2 * (b01 - b21) + (b02 - b22);
            vec2 G = vec2(Gx, Gy);
            float d = dot(G, G);
            // d = min(1.0, 100000 * fwidth(TexCoords).x);
            _gl_FragColor = vec4(vec3(0), d / 5);
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

#define HACK_FRONT_BOTH 0
#define HACK_FRONT_FACES_ONLY 1
#define HACK_BACK_FACES_ONLY 2

uint DRAW_MESH_MODE_LIT            = 0;
uint DRAW_MESH_MODE_PATCH_ID       = 1;
uint DRAW_MESH_MODE_PATCH_EDGES    = 2;
uint DRAW_MESH_MODE_TRIANGLE_ID    = 3;
uint DRAW_MESH_MODE_TRIANGLE_EDGES = 4;
void DRAW_MESH(
        uint mode,
        mat4 P,
        mat4 V,
        mat4 M,
        DrawMesh *mesh,
        vec4 plane_equation1,
        vec4 plane_equation2,
        bool use_mesh_boolean_or_instead_of_and,
        uint HACK_MODE = HACK_FRONT_BOTH
        ) { // default value has no clip plane and uses or clipping

    { // GL; FORNOW FORNOW pushing data to GPU multiple times per mesh every frame (wtf) -- "needed" for prototyping the plane endcaps for mesh tweening
        if (mesh->num_vertices) {
            glBindVertexArray(GL.VAO);
            POOSH(GL.VBO, 0, mesh->num_vertices, mesh->vertex_positions);
            POOSH(GL.VBO, 1, mesh->num_vertices, mesh->vertex_normals);
            POOSH(GL.VBO, 2, mesh->num_vertices, mesh->vertex_patch_indices);
            { // FORNOW: gross explosion of triangle_normal from the work mesh
            }
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL.EBO_faces);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * mesh->num_triangles * sizeof(uint), mesh->triangle_tuples, GL_STATIC_DRAW);
            }
            { // gross explosion from triangles to edges
              // if there is a better way to do this please lmk :(
                uint size = 3 * mesh->num_triangles * sizeof(uint2);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL.EBO_all_edges);
                uint2 *mesh_edge_tuples = (uint2 *) malloc(size);
                defer { free(mesh_edge_tuples); };
                uint k = 0;
                for_(i, mesh->num_triangles) {
                    for_(d, 3) mesh_edge_tuples[k++] = { mesh->triangle_tuples[i][d], mesh->triangle_tuples[i][(d + 1) % 3] };
                }
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, mesh_edge_tuples, GL_STATIC_DRAW);
            }
        }
        {
            // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL.EBO_hard_edges);
            // glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * mesh->num_hard_half_edges * sizeof(uint), mesh->hard_half_edge_tuples, GL_STATIC_DRAW);
        }
    }
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


        GLint clipPlaneLocation1 = glGetUniformLocation(shader_program, "clipPlane1");
        glUniform4fv(clipPlaneLocation1, 1, &plane_equation1[0]);

        if (!use_mesh_boolean_or_instead_of_and) {
            GLint clipPlaneLocation2 = glGetUniformLocation(shader_program, "clipPlane2");
            glUniform4fv(clipPlaneLocation2, 1, &plane_equation2[0]);

        }

        glUniformMatrix4fv(UNIFORM(shader_program, "PV"), 1, GL_TRUE, PV.data);
        glUniformMatrix4fv(UNIFORM(shader_program, "M" ), 1, GL_TRUE, M.data);
        glUniform3f       (UNIFORM(shader_program, "eye_World"), eye_World.x, eye_World.y, eye_World.z);
        glUniform1i(UNIFORM(shader_program, "mode"), mode);

        glActiveTexture(GL_TEXTURE0); // ?
        glBindTexture(GL_TEXTURE_2D, GL2.TextureID);
        glUniform1i(UNIFORM(shader_program, "TextureID"), GL2.TextureID);

        glUniform1i(UNIFORM(shader_program, "feature_plane_is_active"), feature_plane->is_active);
        glUniform3f(UNIFORM(shader_program, "feature_plane_normal"), feature_plane->normal.x, feature_plane->normal.y, feature_plane->normal.z);
        glUniform1f(UNIFORM(shader_program, "feature_plane_signed_distance_to_world_origin"), feature_plane->signed_distance_to_world_origin);

        if (!use_mesh_boolean_or_instead_of_and) {
            glEnable(GL_CLIP_DISTANCE0);
            glEnable(GL_CLIP_DISTANCE1);

            if (HACK_MODE != HACK_FRONT_FACES_ONLY) {
                glCullFace(GL_FRONT);
                glDrawElements(GL_TRIANGLES, num_vertices, GL_UNSIGNED_INT, NULL);
            }

            if (HACK_MODE != HACK_BACK_FACES_ONLY) {
                glCullFace(GL_BACK); 
                glDrawElements(GL_TRIANGLES, num_vertices, GL_UNSIGNED_INT, NULL);
            }

            glDisable(GL_CLIP_DISTANCE0);
            glDisable(GL_CLIP_DISTANCE1);

        } else {
            ASSERT(0); // TODOLATER (not going to bother to HACK this now)
            glEnable(GL_CLIP_DISTANCE0);

            glCullFace(GL_FRONT);
            glDrawElements(GL_TRIANGLES, num_vertices, GL_UNSIGNED_INT, NULL);

            glCullFace(GL_BACK); 
            glDrawElements(GL_TRIANGLES, num_vertices, GL_UNSIGNED_INT, NULL);

            glUniform4fv(clipPlaneLocation1, 1, &plane_equation2[0]);

            glCullFace(GL_FRONT);
            glDrawElements(GL_TRIANGLES, num_vertices, GL_UNSIGNED_INT, NULL);

            glCullFace(GL_BACK); 
            glDrawElements(GL_TRIANGLES, num_vertices, GL_UNSIGNED_INT, NULL);

            glDisable(GL_CLIP_DISTANCE0);
        }


        // glBindTexture(GL_TEXTURE_2D, 0);
        // glActiveTexture(0); // ?
    } else if (
            mode == DRAW_MESH_MODE_TRIANGLE_EDGES
            // || (mode == DRAW_MESH_MODE_PATCH_EDGES)
            ) {

        uint EBO;
        uint num_vertices;
        // if (mode == DRAW_MESH_MODE_TRIANGLE_EDGES) {
        EBO = GL.EBO_all_edges;
        uint mesh_num_half_edges = (3 * mesh->num_triangles);
        num_vertices = (2 * mesh_num_half_edges);
        // }
        // else {
        // EBO = GL.EBO_hard_edges;
        // num_vertices = (2 * mesh->num_hard_half_edges);
        // }

        glBindVertexArray(GL.VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        uint shader_program = edge_shader_program; ASSERT(shader_program); glUseProgram(shader_program);


        glUniformMatrix4fv(UNIFORM(shader_program, "PVM"), 1, GL_TRUE, PVM.data);
        glUniform2f(UNIFORM(shader_program, "OpenGL_from_Pixel_scale"), 2.0f / window_get_width_Pixel(), 2.0f / window_get_height_Pixel());
        glUniform1i(UNIFORM(shader_program, "mode"), mode);
        glUniform1f(UNIFORM(shader_program, "_window_macbook_retina_fixer__VERY_MYSTERIOUS"), _window_macbook_retina_fixer__VERY_MYSTERIOUS);
        glDrawElements(GL_LINES, num_vertices, GL_UNSIGNED_INT, NULL);
    } else {
        ASSERT(0);
    }
}




void fancy_draw(mat4 P, mat4 V, mat4 M, DrawMesh *mesh, vec4 plane_equation1, vec4 plane_equation2, bool use_mesh_boolean_or_instead_of_and) {
    DrawMesh plane_1 = {};
    {
        static vec3  _vertex_positions[] = { { -1, -1, 0 }, { 1, -1, 0 }, { 1, 1, 0 }, { -1, 1, 0 }};
        static uint3 _triangle_tuples[]  = { { 0, 1, 2 }, { 0, 2, 3 } };
        static uint _vertex_patch_indices[]  = { 0, 0, 0, 0 }; // FORNOW 0 TODOLATER  a number larger than the largest vertex index so patch is unique
        static vec3  _vertex_normals[] = { { 0, 0, -1 }, { 0, 0, -1 }, { 0, 0, -1 }, { 0, 0, -1 }}; // wai nate
        plane_1.num_vertices = 4;
        plane_1.num_triangles = 2;
        plane_1.vertex_positions = _vertex_positions;
        plane_1.triangle_tuples = _triangle_tuples;
        plane_1.vertex_patch_indices = _vertex_patch_indices;
        plane_1.vertex_normals = _vertex_normals;
    }
    mat4 T = M4_Translation(0, 0, -plane_equation1.w); // TODO: check negative signs
    mat4 R = M4_RotationFrom(V3(0, 0, 1), _V3(plane_equation1));
    mat4 S = M4_Scaling(100.0f);


    { // set up the stencil buffer
        glEnable(GL_STENCIL_TEST);
        glDisable(GL_DEPTH_TEST);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        {
            glClear(GL_STENCIL_BUFFER_BIT); 

            glStencilFunc(GL_ALWAYS, 0, 0x00);

            // TODO: INCR for BACK faces, decr for FRONT facwes
            // FORNOW: majorly hacked in
            glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
            DRAW_MESH(DRAW_MESH_MODE_LIT, P, V, M, mesh, plane_equation1, plane_equation2, use_mesh_boolean_or_instead_of_and, HACK_BACK_FACES_ONLY);
            glStencilOp(GL_KEEP, GL_DECR, GL_DECR);
            DRAW_MESH(DRAW_MESH_MODE_LIT, P, V, M, mesh, plane_equation1, plane_equation2, use_mesh_boolean_or_instead_of_and, HACK_FRONT_FACES_ONLY);

        }
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_DEPTH_TEST);
    }



    glEnable(GL_STENCIL_TEST);
    {
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc(GL_NOTEQUAL, 0, 0xFF); 
        DRAW_MESH(DRAW_MESH_MODE_LIT, P, V, M * R * T * S, &plane_1, {}, {}, false);
    }
    glDisable(GL_STENCIL_TEST);

    DRAW_MESH(DRAW_MESH_MODE_LIT, P, V, M, mesh, plane_equation1, plane_equation2, use_mesh_boolean_or_instead_of_and);


    #if 0
    { // TODOLATER hard edges
        for_(pass, 2) {
            if ((pass == 1) && !other.show_details) continue;

            glDisable(GL_SCISSOR_TEST);
            glBindFramebuffer(GL_FRAMEBUFFER, GL2.FBO);
            {
                glClearColor(1.0, 1.0, 1.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                DRAW_MESH((pass == 0) ? DRAW_MESH_MODE_PATCH_ID : DRAW_MESH_MODE_TRIANGLE_ID, P, V, M, mesh, plane_equation1, plane_equation2, use_mesh_boolean_or_instead_of_and);
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glEnable(GL_SCISSOR_TEST);

            glDisable(GL_DEPTH_TEST); {
                if (pass == 0) {
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
                } else {
                    DRAW_MESH(DRAW_MESH_MODE_TRIANGLE_EDGES, P, V, M, mesh, plane_equation1, plane_equation2, use_mesh_boolean_or_instead_of_and);
                }
            } glEnable(GL_DEPTH_TEST);
        }
    } 
    #endif
}
