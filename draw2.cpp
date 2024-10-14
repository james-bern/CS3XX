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
            rgb += 0.5 * (vec3(1.0) - abs(N.zxy)); // normal-ambient
            // rgb += 0.2 * vec3(1.0);

            for (int i = 0; i < 2; ++i) {
                vec3 light_position_World = (i == 0) ? eye_World : vec3(0.0, 1000.0, 0.0);
                vec3 L = normalize(light_position_World - fs_in.position_World);
                vec3 E = normalize(eye_World - fs_in.position_World);
                vec3 H = normalize(L + E);
                float F0 = 0.05;
                float diffuse = max(0.0, dot(N, L));
                float specular = pow(max(0.0, dot(N, H)), 256);
                float fresnel = F0 + (1 - F0) * pow(1.0 - max(0.0, dot(N, H)), 5);

                rgb += 0.3 * diffuse;
                rgb += 0.7 * specular;
                rgb += 0.8 * fresnel;
            }

            _gl_FragColor = vec4(rgb, 1.0);
        }
    )"";
} phong_source;

struct {
    int shader_program;
    uint VAO;
    uint VBO[6];
    uint EBO;
} phong;

run_before_main {
    uint vert = shader_compile(phong_source.vert, GL_VERTEX_SHADER);
    uint frag = shader_compile(phong_source.frag, GL_FRAGMENT_SHADER);
    phong.shader_program = shader_build_program(vert, 0, frag);
    glGenVertexArrays(1, &phong.VAO);
    glGenBuffers(ARRAY_LENGTH(phong.VBO), phong.VBO);
    glGenBuffers(1, &phong.EBO);
};

// TODO: compute vertex normals
void phong_draw(mat4 P, mat4 V, mat4 M, Mesh *) {

    // NOTE moved this up before the pushes; is that okay?
    ASSERT(phong.shader_program);
    glUseProgram(phong.shader_program);

    glBindVertexArray(phong.VAO);
    unsigned int i_attrib = 0;
    auto guarded_push = [&](void *array, int dim, int sizeof_type, int GL_TYPE_VAR) {
        ASSERT(i_attrib < ARRAY_LENGTH(phong.VBO));
        glDisableVertexAttribArray(i_attrib);
        if (array) {
            glBindBuffer(GL_ARRAY_BUFFER, phong.VBO[i_attrib]);
            glBufferData(GL_ARRAY_BUFFER, mesh->num_vertices * dim * sizeof_type, array, GL_DYNAMIC_DRAW);
            if (GL_TYPE_VAR == GL_FLOAT) {
                glVertexAttribPointer(i_attrib, dim, GL_TYPE_VAR, GL_FALSE, 0, NULL);
            } else if (GL_TYPE_VAR == GL_INT) {
                glVertexAttribIPointer(i_attrib, dim, GL_TYPE_VAR, 0, NULL);
            } else {
                ASSERT(0);
            }
            glEnableVertexAttribArray(i_attrib); // FORNOW: after the other stuff?
        }
        ++i_attrib;
    };

    // TODO: assert sizeof's match
    guarded_push(mesh->vertex_positions, 3, sizeof(real), GL_FLOAT);
    guarded_push(mesh->vertex_normals, 3, sizeof(real), GL_FLOAT);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, phong.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * mesh->num_triangles * sizeof(uint), mesh->triangle_indices, GL_DYNAMIC_DRAW);

    auto LOC = [&](char *name) { return glGetUniformLocation(phong.shader_program, name); };
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
