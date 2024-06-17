uint shader_compile(char *source, GLenum type) {
    uint shader = glCreateShader(type);
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

uint shader_build_program(uint vertex_shader, uint fragment_shader, uint geometry_shader = 0) {
    uint shader_program_ID = glCreateProgram();
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

uint shader_compile_and_build_program(char *vertex_shader_source, char *fragment_shader_source, char *geometry_shader_source = NULL) {
    uint vert = shader_compile(vertex_shader_source, GL_VERTEX_SHADER);
    uint frag = shader_compile(fragment_shader_source, GL_FRAGMENT_SHADER);
    uint geom = geometry_shader_source ? shader_compile(geometry_shader_source, GL_GEOMETRY_SHADER) : 0;
    return shader_build_program(vert, frag, geom);
}

