////////////////////////////////////////////////////////////////////////////////
// #include "shader.cpp"////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int _shader_compile(char *source, GLenum type) {
    int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    {
        int success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            { // log
                char infoLog[512];
                glGetShaderInfoLog(shader, 512, NULL, infoLog);
                printf("%s", infoLog);
            }
            ASSERT(0);
        }
    }
    return shader;
};

int _shader_build_program(int vertex_shader, int fragment_shader, int geometry_shader = 0) {
    int shader_program_ID = glCreateProgram();
    glAttachShader(shader_program_ID, vertex_shader);
    glAttachShader(shader_program_ID, fragment_shader);
    if (geometry_shader) glAttachShader(shader_program_ID, geometry_shader);
    glLinkProgram(shader_program_ID);
    {
        int success = 0;
        glGetProgramiv(shader_program_ID, GL_LINK_STATUS, &success);
        if (!success) {
            { // log
                char infoLog[512];
                glGetProgramInfoLog(shader_program_ID, 512, NULL, infoLog);
                printf("%s", infoLog);
            }
            ASSERT(0);
        }
    }
    return shader_program_ID;
};

int _shader_compile_and_build_program(char *vertex_shader_source, char *fragment_shader_source, char *geometry_shader_source = NULL) {
    int vert = _shader_compile(vertex_shader_source, GL_VERTEX_SHADER);
    int frag = _shader_compile(fragment_shader_source, GL_FRAGMENT_SHADER);
    int geom = geometry_shader_source ? _shader_compile(geometry_shader_source, GL_GEOMETRY_SHADER) : 0;
    return _shader_build_program(vert, frag, geom);
}

int _shader_get_uniform_location(int shader_program_ID, char *name) {
    ASSERT(shader_program_ID);
    int location = glGetUniformLocation(shader_program_ID, name);
    return location;
}

void _shader_set_uniform_bool(int shader_program_ID, char *name, bool value) {
    glUniform1ui(_shader_get_uniform_location(shader_program_ID, name), value);
}

void _shader_set_uniform_int(int shader_program_ID, char *name, int value) {
    glUniform1i(_shader_get_uniform_location(shader_program_ID, name), value);
}

void _shader_set_uniform_real(int shader_program_ID, char *name, real value) {
    glUniform1f(_shader_get_uniform_location(shader_program_ID, name), float(value));
}

void _shader_set_uniform_vec2(int shader_program_ID, char *name, real *value) {
    ASSERT(value);
    glUniform2f(_shader_get_uniform_location(shader_program_ID, name), float(value[0]), float(value[1]));
}

void _shader_set_uniform_vec3(int shader_program_ID, char *name, real *value) {
    ASSERT(value);
    glUniform3f(_shader_get_uniform_location(shader_program_ID, name), float(value[0]), float(value[1]), float(value[2]));
}

void _shader_set_uniform_vec4(int shader_program_ID, char *name, real *value) {
    ASSERT(value);
    glUniform4f(_shader_get_uniform_location(shader_program_ID, name), float(value[0]), float(value[1]), float(value[2]), float(value[3]));
}

void _shader_set_uniform_mat4(int shader_program_ID, char *name, real *value) {
    ASSERT(value);
    float tmp[16] = {}; {
        for (int k = 0; k < 16; ++k) tmp[k] = float(value[k]);
    }
    glUniformMatrix4fv(_shader_get_uniform_location(shader_program_ID, name), 1, GL_TRUE, tmp);
}

void _shader_set_uniform_vec3_array(int shader_program_ID, char *name, int count, real *value) {
    ASSERT(value);
    float *tmp = (float *) malloc(count * 3 * sizeof(float)); 
    for (int i = 0; i < count; ++i) {
        for (int d = 0; d < 3; ++d) { 
            tmp[3 * i + d] = float(value[3 * i + d]);
        }
    }
    glUniform3fv(_shader_get_uniform_location(shader_program_ID, name), count, tmp);
    free(tmp);
}

void _shader_set_uniform_vec4_array(int shader_program_ID, char *name, int count, real *value) {
    ASSERT(value);
    float *tmp = (float *) malloc(count * 4 * sizeof(float)); 
    for (int i = 0; i < count; ++i) {
        for (int d = 0; d < 4; ++d) { 
            tmp[4 * i + d] = float(value[4 * i + d]);
        }
    }
    glUniform3fv(_shader_get_uniform_location(shader_program_ID, name), count, tmp);
    free(tmp);
}

void _shader_set_uniform_mat4_array(int shader_program_ID, char *name, int count, real *value) {
    ASSERT(value);
    float *tmp = (float *) malloc(count * 16 * sizeof(float)); 
    for (int i = 0; i < count; ++i) {
        for (int d = 0; d < 16; ++d) { 
            tmp[16 * i + d] = float(value[16 * i + d]);
        }
    }
    glUniformMatrix4fv(_shader_get_uniform_location(shader_program_ID, name), count, GL_TRUE, tmp);
    free(tmp);
}


struct Shader {
    int _program_ID;
    int _attribute_counter;
    GLuint _VAO;
    GLuint _VBO[16];
    GLuint _EBO;
};

Shader shader_create(
        char *vertex_shader_source,
        char *fragment_shader_source,
        char *geometry_shader_source = NULL) {
    ASSERT(vertex_shader_source);
    ASSERT(fragment_shader_source);

    Shader shader = {};
    shader._program_ID = _shader_compile_and_build_program(vertex_shader_source, fragment_shader_source, geometry_shader_source);
    glGenVertexArrays(1, &shader._VAO);
    glGenBuffers(ARRAY_LENGTH(shader._VBO), shader._VBO);
    glGenBuffers(1, &shader._EBO);
    return shader;
};

template <int D> void shader_pass_vertex_attribute(Shader *shader, int num_vertices, Vector<D> *vertex_attribute) {
    ASSERT(shader);
    ASSERT(vertex_attribute);
    glUseProgram(shader->_program_ID);
    glBindVertexArray(shader->_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, shader->_VBO[shader->_attribute_counter]);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * D * sizeof(real), vertex_attribute, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(shader->_attribute_counter, D, GL_REAL, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(shader->_attribute_counter);
    ++shader->_attribute_counter;
}
void shader_pass_vertex_attribute(Shader *shader, int num_vertices, uint4 *vertex_attribute) {
    int D = 4; // FORNOW; TODO: uint2, uint3
    ASSERT(shader);
    ASSERT(vertex_attribute);
    glUseProgram(shader->_program_ID);
    glBindVertexArray(shader->_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, shader->_VBO[shader->_attribute_counter]);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * D * sizeof(int), vertex_attribute, GL_DYNAMIC_DRAW);
    glVertexAttribIPointer(shader->_attribute_counter, D, GL_INT, 0, NULL);
    glEnableVertexAttribArray(shader->_attribute_counter);
    ++shader->_attribute_counter;
}
void shader_draw(Shader *shader, int num_triangles, uint3 *triangle_indices) {
    ASSERT(shader);
    ASSERT(triangle_indices);
    shader->_attribute_counter = 0;

    glUseProgram(shader->_program_ID);

    glEnableVertexAttribArray(0); // ??
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shader->_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * num_triangles * sizeof(int), triangle_indices, GL_DYNAMIC_DRAW);
    glDrawElements(GL_TRIANGLES, 3 * num_triangles, GL_UNSIGNED_INT, NULL);
}

