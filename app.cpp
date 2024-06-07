// // GLFW
#ifdef COW_OS_UBUNTU
#define GL_GLEXT_PROTOTYPES
#include "codebase/ext/glfw3.h"
#elif defined(OPERATING_SYSTEM_APPLE)
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GL_COREARB
#include <OpenGL/gl3.h>
#include "codebase/ext/glfw3.h"
#else
#include "codebase/ext/glad.c"
#include "codebase/ext/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include "codebase/ext/glfw3native.h"
#endif

#define GL_REAL GL_FLOAT

// // stb
// easy_font
#include "codebase/ext/stb_easy_font.h"
// image
#define STB_IMAGE_IMPLEMENTATION
#include "codebase/ext/stb_image.h"




#define ITRI_MAX_NUM_TEXTURES 32
#define ITRI_MAX_FILENAME_LENGTH 64

struct CW_USER_FACING_CONFIG {
    bool tweaks_soup_draw_with_rounded_corners_for_all_line_primitives = true;
    bool tweaks_record_raw_then_encode_everything_WARNING_USES_A_LOT_OF_DISK_SPACE = false;
    real tweaks_size_in_pixels_soup_draw_defaults_to_if_you_pass_0_for_size_in_pixels = 2.0;
};

struct C2_READONLY_USER_FACING_DATA {
    mat4 NDC_from_Screen;
};

// constants
// shaders
struct CX_INTERNAL_CONSTANTS {
    char *_soup_vert = R""(
        #version 330 core
        layout (location = 0) in vec3 vertex;
        layout (location = 1) in vec4 color;

        out BLOCK {
            vec4 color;
        } vs_out;

        uniform mat4 PVM;
        uniform bool force_draw_on_top;
        uniform bool has_vertex_colors;
        uniform vec4 color_if_vertex_colors_is_NULL;

        void main() {
            gl_Position = PVM * vec4(vertex, 1);
            vs_out.color = color_if_vertex_colors_is_NULL;

            if (has_vertex_colors) {
                vs_out.color = color;
            }

            if (force_draw_on_top) {
                gl_Position.z = -.99 * gl_Position.w; // ?
            }
        }
    )"";

    char *_soup_geom_POINTS = R""(
        #version 330 core
        layout (points) in;
        layout (triangle_strip, max_vertices = 4) out;
        uniform float aspect;
        uniform float primitive_radius_NDC;

        in BLOCK {
            vec4 color;
        } gs_in[];

        out GS_OUT {
            vec4 color;
            vec2 xy;
        } gs_out;

        vec4 _position;

        void emit(float x, float y) {
            gs_out.xy = vec2(x, y);
            gl_Position = (_position + primitive_radius_NDC * vec4(x / aspect, y, 0, 0)) * gl_in[0].gl_Position.w;
            gs_out.color = gs_in[0].color;                                     
            EmitVertex();                                               
        }

        void main() {    
            _position = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
            emit(-1, -1);
            emit(1, -1);
            emit(-1, 1);
            emit(1, 1);
            EndPrimitive();
        }  
    )"";

    char *_soup_frag_POINTS = R""(
        #version 330 core

        in GS_OUT {
            vec4 color;
            vec2 xy;
        } fs_in;

        out vec4 frag_color;

        void main() {
            frag_color = fs_in.color;
            if (length(fs_in.xy) > 1) { discard; }
        }
    )"";

    char *_soup_geom_LINES = R""(
        #version 330 core
        layout (lines) in;
        layout (triangle_strip, max_vertices = 4) out;
        uniform float aspect;
        uniform float primitive_radius_NDC;

        in BLOCK {
            vec4 color;
        } gs_in[];

        out BLOCK {
            vec4 color;
        } gs_out;

        void main() {    
            vec4 s = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
            vec4 t = gl_in[1].gl_Position / gl_in[1].gl_Position.w;
            vec4 color_s = gs_in[0].color;
            vec4 color_t = gs_in[1].color;

            vec4 perp = vec4(primitive_radius_NDC * vec2(1 / aspect, 1) * normalize(vec2(-1 / aspect, 1) * (t - s).yx), 0, 0);

            gl_Position = (s - perp) * gl_in[0].gl_Position.w;
            gs_out.color = color_s;
            EmitVertex();

            gl_Position = (t - perp) * gl_in[1].gl_Position.w;
            gs_out.color = color_t;
            EmitVertex();

            gl_Position = (s + perp) * gl_in[0].gl_Position.w;
            gs_out.color = color_s;
            EmitVertex();

            gl_Position = (t + perp) * gl_in[1].gl_Position.w;
            gs_out.color = color_t;
            EmitVertex();

            EndPrimitive();
        }  
    )"";

    char *_soup_frag = R""(
        #version 330 core

        in BLOCK {
            vec4 color;
        } fs_in;

        out vec4 frag_color;

        void main() {
            frag_color = fs_in.color;
        }
    )"";

    char *_mesh_vert = R""(
        #version 330 core
        layout (location = 0) in  vec3 vertex;
        layout (location = 1) in  vec3 normal;
        layout (location = 2) in  vec3 color;
        layout (location = 3) in  vec2 texCoord;
        layout (location = 4) in ivec4 boneIndices;
        layout (location = 5) in  vec4 boneWeights;

        out BLOCK {
            vec3 position_World;
            vec3 normal_World;
            vec3 color;
            vec2 texCoord;
        } vs_out;

        uniform mat4 P, V, M;

        uniform bool has_vertex_colors;
        uniform vec3 color_if_vertex_colors_is_NULL;

        uniform bool has_vertex_texture_coordinates;

        uniform bool has_bones;
        uniform mat4 bones[64];

        void main() {

            {
                vec4 tmp_position = vec4(vertex, 1.0);
                if (has_bones) {
                    tmp_position = boneWeights.x * (bones[boneIndices.x] * tmp_position)
                                 + boneWeights.y * (bones[boneIndices.y] * tmp_position)
                                 + boneWeights.z * (bones[boneIndices.z] * tmp_position)
                                 + boneWeights.w * (bones[boneIndices.w] * tmp_position);
                }
                tmp_position = M * tmp_position;
                vs_out.position_World = vec3(tmp_position);
                gl_Position = P * V * tmp_position;
            }

            {
                vec3 tmp_normal = normal;
                if (has_bones) {
                    tmp_normal = boneWeights.x * (inverse(transpose(mat3(bones[boneIndices.x]))) * tmp_normal)
                               + boneWeights.y * (inverse(transpose(mat3(bones[boneIndices.y]))) * tmp_normal)
                               + boneWeights.z * (inverse(transpose(mat3(bones[boneIndices.z]))) * tmp_normal)
                               + boneWeights.w * (inverse(transpose(mat3(bones[boneIndices.w]))) * tmp_normal);
                }
                tmp_normal = inverse(transpose(mat3(M))) * tmp_normal;
                vs_out.normal_World = tmp_normal;
            }

            vs_out.color = has_vertex_colors ? color : color_if_vertex_colors_is_NULL;
            vs_out.texCoord = has_vertex_texture_coordinates ? texCoord : vec2(0., 0.);

        }
    )"";

    char *_mesh_frag = R""(
        #version 330 core

        in BLOCK {
            vec3 position_World;
            vec3 normal_World;
            vec3 color;
            vec2 texCoord;
        } fs_in;
        uniform sampler2D i_texture;

        uniform vec4 eye_World;

        uniform bool has_vertex_normals;
        uniform bool has_vertex_colors;
        uniform bool has_vertex_texture_coordinates;
        uniform bool has_texture;

        out vec4 frag_color;

        void main() {
            vec3 world_to_eye = vec3(eye_World) - fs_in.position_World;
            vec3 N = normalize(fs_in.normal_World);
            vec3 E = normalize(world_to_eye);

            vec3 rgb = fs_in.color;
            float a = 1.;
            if (has_texture) {
                vec2 texture_coordinates = (has_vertex_texture_coordinates) ? fs_in.texCoord : (.5 + .5 * N).xy;
                vec4 rgba = texture(i_texture, texture_coordinates);
                rgb = rgba.rgb;
                a = rgba.a;
            } else if (has_vertex_normals) {
                vec3 L = E;
                vec3 H = normalize(L + E);
                float F0 = .05;
                float diffuse = max(0, dot(N, L));
                float specular = pow(max(0, dot(N, H)), 100);
                float fresnel = F0 + (1 - F0) * pow(1 - max(0, dot(N, H)), 5);
                rgb += .3 * (-1.0 + 2.0 * diffuse);
                rgb += .4 * specular;
                rgb += .6 * (-.3 + 1.3 * fresnel);
            }
            frag_color = vec4(rgb, a);
        }
    )"";
};

struct C0_PersistsAcrossApps_NeverAutomaticallyClearedToZero__ManageItYourself {
    real *_eso_vertex_positions;
    real *_eso_vertex_colors;

    int _mesh_shader_program;
    uint _mesh_VAO;
    uint _mesh_VBO[6];
    uint _mesh_EBO;

    int _soup_shader_program_POINTS;
    int _soup_shader_program_LINES;
    int _soup_shader_program_TRIANGLES;
    uint _soup_VAO[3];
    uint _soup_VBO[2];
    uint _soup_EBO[3];

    GLFWwindow *_window_glfw_window;
    void *_window_hwnd__note_this_is_NULL_if_not_on_Windows;
    real _window_macbook_retina_fixer__VERY_MYSTERIOUS;
};

struct C1_PersistsAcrossFrames_AutomaticallyClearedToZeroBetweenAppsBycow_reset {
    bool _eso_called_eso_begin_before_calling_eso_vertex_or_eso_end;
    real _eso_current_color[4];
    real _eso_PVM[16];
    int _eso_primitive;
    int _eso_num_vertices;
    real _eso_size_in_pixels;
    bool _eso_overlay;

    char _mesh_texture_filenames[ITRI_MAX_NUM_TEXTURES][ITRI_MAX_FILENAME_LENGTH];
    uint  _mesh_textures[ITRI_MAX_NUM_TEXTURES];
    int  _mesh_num_textures;

    real _gui_x_curr;
    real _gui_y_curr;
};


CW_USER_FACING_CONFIG config;
C2_READONLY_USER_FACING_DATA globals; // COW2
CX_INTERNAL_CONSTANTS COWX;
C0_PersistsAcrossApps_NeverAutomaticallyClearedToZero__ManageItYourself COW0;
C1_PersistsAcrossFrames_AutomaticallyClearedToZeroBetweenAppsBycow_reset COW1;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// core ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////




// TODO: remove this next
////////////////////////////////////////////////////////////////////////////////
// #include "_linalg.cpp"///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define _LINALG_4X4(A, i, j) A[4 * (i) + (j)]



void _linalg_mat4_times_mat4(real *C, real *A, real *B) { // C = A B
    ASSERT(C);
    ASSERT(A);
    ASSERT(B);

    real tmp[16] = {}; { // allows for e.g. A <- A B;
        for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) for (int k = 0; k < 4; ++k) _LINALG_4X4(tmp, i, j) += _LINALG_4X4(A, i, k) * _LINALG_4X4(B, k, j);
    }
    memcpy(C, tmp, sizeof(tmp));
}

void _linalg_mat4_times_vec4_persp_divide(real *b, real *A, real *x) { // b = A x
    ASSERT(b);
    ASSERT(A);
    ASSERT(x);

    real tmp[4] = {}; { // allows for x <- A x
        for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) tmp[i] += _LINALG_4X4(A, i, j) * x[j];
        if (!IS_ZERO(tmp[3])) {
            // ASSERT(!IS_ZERO(x[3]));
            for (int i = 0; i < 4; ++i) tmp[i] /= tmp[3]; 
        }
    }
    memcpy(b, tmp, sizeof(tmp));
}

real _linalg_mat4_determinant(real *A) {
    ASSERT(A);
    real A2323 = _LINALG_4X4(A, 2, 2) * _LINALG_4X4(A, 3, 3) - _LINALG_4X4(A, 2, 3) * _LINALG_4X4(A, 3, 2);
    real A1323 = _LINALG_4X4(A, 2, 1) * _LINALG_4X4(A, 3, 3) - _LINALG_4X4(A, 2, 3) * _LINALG_4X4(A, 3, 1);
    real A1223 = _LINALG_4X4(A, 2, 1) * _LINALG_4X4(A, 3, 2) - _LINALG_4X4(A, 2, 2) * _LINALG_4X4(A, 3, 1);
    real A0323 = _LINALG_4X4(A, 2, 0) * _LINALG_4X4(A, 3, 3) - _LINALG_4X4(A, 2, 3) * _LINALG_4X4(A, 3, 0);
    real A0223 = _LINALG_4X4(A, 2, 0) * _LINALG_4X4(A, 3, 2) - _LINALG_4X4(A, 2, 2) * _LINALG_4X4(A, 3, 0);
    real A0123 = _LINALG_4X4(A, 2, 0) * _LINALG_4X4(A, 3, 1) - _LINALG_4X4(A, 2, 1) * _LINALG_4X4(A, 3, 0);
    return _LINALG_4X4(A, 0, 0) * (_LINALG_4X4(A, 1, 1) * A2323 - _LINALG_4X4(A, 1, 2) * A1323 + _LINALG_4X4(A, 1, 3) * A1223) 
        -  _LINALG_4X4(A, 0, 1) * (_LINALG_4X4(A, 1, 0) * A2323 - _LINALG_4X4(A, 1, 2) * A0323 + _LINALG_4X4(A, 1, 3) * A0223) 
        +  _LINALG_4X4(A, 0, 2) * (_LINALG_4X4(A, 1, 0) * A1323 - _LINALG_4X4(A, 1, 1) * A0323 + _LINALG_4X4(A, 1, 3) * A0123) 
        -  _LINALG_4X4(A, 0, 3) * (_LINALG_4X4(A, 1, 0) * A1223 - _LINALG_4X4(A, 1, 1) * A0223 + _LINALG_4X4(A, 1, 2) * A0123);
}

void _linalg_mat4_inverse(real *invA, real *A) {
    ASSERT(invA);
    ASSERT(A);
    real one_over_det = 1 / _linalg_mat4_determinant(A);
    real tmp[16] = {}; { // allows for A <- inv(A)
        real A2323 = _LINALG_4X4(A, 2, 2) * _LINALG_4X4(A, 3, 3) - _LINALG_4X4(A, 2, 3) * _LINALG_4X4(A, 3, 2);
        real A1323 = _LINALG_4X4(A, 2, 1) * _LINALG_4X4(A, 3, 3) - _LINALG_4X4(A, 2, 3) * _LINALG_4X4(A, 3, 1);
        real A1223 = _LINALG_4X4(A, 2, 1) * _LINALG_4X4(A, 3, 2) - _LINALG_4X4(A, 2, 2) * _LINALG_4X4(A, 3, 1);
        real A0323 = _LINALG_4X4(A, 2, 0) * _LINALG_4X4(A, 3, 3) - _LINALG_4X4(A, 2, 3) * _LINALG_4X4(A, 3, 0);
        real A0223 = _LINALG_4X4(A, 2, 0) * _LINALG_4X4(A, 3, 2) - _LINALG_4X4(A, 2, 2) * _LINALG_4X4(A, 3, 0);
        real A0123 = _LINALG_4X4(A, 2, 0) * _LINALG_4X4(A, 3, 1) - _LINALG_4X4(A, 2, 1) * _LINALG_4X4(A, 3, 0);
        real A2313 = _LINALG_4X4(A, 1, 2) * _LINALG_4X4(A, 3, 3) - _LINALG_4X4(A, 1, 3) * _LINALG_4X4(A, 3, 2);
        real A1313 = _LINALG_4X4(A, 1, 1) * _LINALG_4X4(A, 3, 3) - _LINALG_4X4(A, 1, 3) * _LINALG_4X4(A, 3, 1);
        real A1213 = _LINALG_4X4(A, 1, 1) * _LINALG_4X4(A, 3, 2) - _LINALG_4X4(A, 1, 2) * _LINALG_4X4(A, 3, 1);
        real A2312 = _LINALG_4X4(A, 1, 2) * _LINALG_4X4(A, 2, 3) - _LINALG_4X4(A, 1, 3) * _LINALG_4X4(A, 2, 2);
        real A1312 = _LINALG_4X4(A, 1, 1) * _LINALG_4X4(A, 2, 3) - _LINALG_4X4(A, 1, 3) * _LINALG_4X4(A, 2, 1);
        real A1212 = _LINALG_4X4(A, 1, 1) * _LINALG_4X4(A, 2, 2) - _LINALG_4X4(A, 1, 2) * _LINALG_4X4(A, 2, 1);
        real A0313 = _LINALG_4X4(A, 1, 0) * _LINALG_4X4(A, 3, 3) - _LINALG_4X4(A, 1, 3) * _LINALG_4X4(A, 3, 0);
        real A0213 = _LINALG_4X4(A, 1, 0) * _LINALG_4X4(A, 3, 2) - _LINALG_4X4(A, 1, 2) * _LINALG_4X4(A, 3, 0);
        real A0312 = _LINALG_4X4(A, 1, 0) * _LINALG_4X4(A, 2, 3) - _LINALG_4X4(A, 1, 3) * _LINALG_4X4(A, 2, 0);
        real A0212 = _LINALG_4X4(A, 1, 0) * _LINALG_4X4(A, 2, 2) - _LINALG_4X4(A, 1, 2) * _LINALG_4X4(A, 2, 0);
        real A0113 = _LINALG_4X4(A, 1, 0) * _LINALG_4X4(A, 3, 1) - _LINALG_4X4(A, 1, 1) * _LINALG_4X4(A, 3, 0);
        real A0112 = _LINALG_4X4(A, 1, 0) * _LINALG_4X4(A, 2, 1) - _LINALG_4X4(A, 1, 1) * _LINALG_4X4(A, 2, 0);

        int i = 0;
        tmp[i++] =  one_over_det * (_LINALG_4X4(A, 1, 1) * A2323 - _LINALG_4X4(A, 1, 2) * A1323 + _LINALG_4X4(A, 1, 3) * A1223);
        tmp[i++] = -one_over_det * (_LINALG_4X4(A, 0, 1) * A2323 - _LINALG_4X4(A, 0, 2) * A1323 + _LINALG_4X4(A, 0, 3) * A1223);
        tmp[i++] =  one_over_det * (_LINALG_4X4(A, 0, 1) * A2313 - _LINALG_4X4(A, 0, 2) * A1313 + _LINALG_4X4(A, 0, 3) * A1213);
        tmp[i++] = -one_over_det * (_LINALG_4X4(A, 0, 1) * A2312 - _LINALG_4X4(A, 0, 2) * A1312 + _LINALG_4X4(A, 0, 3) * A1212);
        tmp[i++] = -one_over_det * (_LINALG_4X4(A, 1, 0) * A2323 - _LINALG_4X4(A, 1, 2) * A0323 + _LINALG_4X4(A, 1, 3) * A0223);
        tmp[i++] =  one_over_det * (_LINALG_4X4(A, 0, 0) * A2323 - _LINALG_4X4(A, 0, 2) * A0323 + _LINALG_4X4(A, 0, 3) * A0223);
        tmp[i++] = -one_over_det * (_LINALG_4X4(A, 0, 0) * A2313 - _LINALG_4X4(A, 0, 2) * A0313 + _LINALG_4X4(A, 0, 3) * A0213);
        tmp[i++] =  one_over_det * (_LINALG_4X4(A, 0, 0) * A2312 - _LINALG_4X4(A, 0, 2) * A0312 + _LINALG_4X4(A, 0, 3) * A0212);
        tmp[i++] =  one_over_det * (_LINALG_4X4(A, 1, 0) * A1323 - _LINALG_4X4(A, 1, 1) * A0323 + _LINALG_4X4(A, 1, 3) * A0123);
        tmp[i++] = -one_over_det * (_LINALG_4X4(A, 0, 0) * A1323 - _LINALG_4X4(A, 0, 1) * A0323 + _LINALG_4X4(A, 0, 3) * A0123);
        tmp[i++] =  one_over_det * (_LINALG_4X4(A, 0, 0) * A1313 - _LINALG_4X4(A, 0, 1) * A0313 + _LINALG_4X4(A, 0, 3) * A0113);
        tmp[i++] = -one_over_det * (_LINALG_4X4(A, 0, 0) * A1312 - _LINALG_4X4(A, 0, 1) * A0312 + _LINALG_4X4(A, 0, 3) * A0112);
        tmp[i++] = -one_over_det * (_LINALG_4X4(A, 1, 0) * A1223 - _LINALG_4X4(A, 1, 1) * A0223 + _LINALG_4X4(A, 1, 2) * A0123);
        tmp[i++] =  one_over_det * (_LINALG_4X4(A, 0, 0) * A1223 - _LINALG_4X4(A, 0, 1) * A0223 + _LINALG_4X4(A, 0, 2) * A0123);
        tmp[i++] = -one_over_det * (_LINALG_4X4(A, 0, 0) * A1213 - _LINALG_4X4(A, 0, 1) * A0213 + _LINALG_4X4(A, 0, 2) * A0113);
        tmp[i++] =  one_over_det * (_LINALG_4X4(A, 0, 0) * A1212 - _LINALG_4X4(A, 0, 1) * A0212 + _LINALG_4X4(A, 0, 2) * A0112);
    }
    memcpy(invA, tmp, sizeof(tmp));
}

void _linalg_mat4_transpose(real *AT, real *A) { // AT = A^T
    ASSERT(AT);
    ASSERT(A);
    real tmp[16] = {}; { // allows for A <- transpose(A)
        for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) _LINALG_4X4(tmp, i, j) = _LINALG_4X4(A, j, i);
    }
    memcpy(AT, tmp, 16 * sizeof(real));
}

////////////////////////////////////////////////////////////////////////////////
// #include "window.cpp"////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// scissor
void gl_scissor_TODO_CHECK_ARGS(real x, real y, real dx, real dy) {
    real factor = COW0._window_macbook_retina_fixer__VERY_MYSTERIOUS;
    glScissor(factor * x, factor * y, factor * dx, factor * dy);
}
#ifdef glScissor
#undef glScissor
#endif
#define glScissor RETINA_BREAKS_THIS_FUNCTION_USE_gl_scissor_WRAPPER

void _callback_set_callbacks();
void _callback_cursor_position(GLFWwindow *, double _xpos, double _ypos);
void _window_init() {
    ASSERT(glfwInit());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 1);

    // glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    COW0._window_glfw_window = glfwCreateWindow(1000, 1000, "conversation -- " __DATE__ " " __TIME__, NULL, NULL);
    if (!COW0._window_glfw_window) {
        printf("[cow] something's gone wonky; if you weren't just messing with init(...) or something, please try restarting your computer and try again.\n");
        ASSERT(0);
    }


    glfwMakeContextCurrent(COW0._window_glfw_window);

    #ifdef OPERATING_SYSTEM_WINDOWS
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    #endif

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDisable(GL_CULL_FACE);

    glfwSwapInterval(1);

    _callback_set_callbacks();

    #ifdef OPERATING_SYSTEM_WINDOWS
    COW0._window_hwnd__note_this_is_NULL_if_not_on_Windows = glfwGetWin32Window(COW0._window_glfw_window);
    #endif

    { // _macbook_retina_scale
        int num, den, _;
        glfwGetFramebufferSize(COW0._window_glfw_window, &num, &_);
        glfwGetWindowSize(COW0._window_glfw_window, &den, &_);
        COW0._window_macbook_retina_fixer__VERY_MYSTERIOUS = real(num / den);
    }

    glfwSetWindowPos(COW0._window_glfw_window, 0.0f, 0.0f);
    glfwSetWindowSize(COW0._window_glfw_window, 960.0f, 540.0f);
    glfwSetWindowAttrib(COW0._window_glfw_window, GLFW_FLOATING, false);
    glfwSetWindowAttrib(COW0._window_glfw_window, GLFW_DECORATED, true);

}




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

vec2 window_get_size_Pixel() {
    ASSERT(COW0._window_glfw_window);
    int _width, _height;
    glfwGetFramebufferSize(COW0._window_glfw_window, &_width, &_height);
    real width = real(_width) / COW0._window_macbook_retina_fixer__VERY_MYSTERIOUS;
    real height = real(_height) / COW0._window_macbook_retina_fixer__VERY_MYSTERIOUS;
    return { width, height };
}

real window_get_aspect() {
    vec2 size = window_get_size_Pixel();
    return size.x / size.y;
}

mat4 window_get_NDC_from_Screen() {
    // NDC                         Screen
    // [x'] = [1/r_x      0   0 -1] [x] = [x/r_x - 1]
    // [y'] = [    0 -1/r_y   0  1] [y] = [1 - y/r_y]
    // [z'] = [    0      0   0  0] [z] = [        0]
    // [1 ] = [    0      0   0  1] [1] = [        1]
    vec2 r = window_get_size_Pixel() / 2;
    mat4 result = {};
    result(0, 0) = 1.0f / r.x;
    result(1, 1) = -1.0f / r.y;
    result(0, 3) = -1.0f;
    result(1, 3) = 1.0f;
    result(3, 3) = 1.0f;
    return result;
}

mat4 get_P_persp(real angle_of_view, vec2 post_nudge_NDC = {}, real near_z_Camera = 0, real far_z_Camera = 0, real aspect = 0) {
    if (IS_ZERO(near_z_Camera)) { near_z_Camera = -10.0f; }
    if (IS_ZERO(far_z_Camera)) { far_z_Camera = -10000.0f; }
    if (IS_ZERO(aspect)) { aspect = window_get_aspect(); }
    ASSERT(near_z_Camera < 0.0f);
    ASSERT(far_z_Camera < 0.0f);

    // consider a point with coordinates (x, y, -z) in the camera's coordinate system
    //                                                                   where z < 0*
    //                              *recall that the camera's z-axis points backwards

    // 1) imagine projecting the point onto some film plane with height r_y and distance D

    //                r_y                               
    //               -|                                 
    //              - |                                 
    //  angle_y    -  |           y <~ vertex           
    //         \  -   |       -   |                     
    //          |-    |   -       |                     
    //          v     +           |                     
    //         -) -   |           |                     
    //        0-------+-----------+----->               
    //                D          -z                     

    // 2) scale film plane by 1 / r_y to yield NDC film plane (with height 1) and distance Q_y
    // y' is the projected position of vertex y in NDC; i.e., if we can get y', we're done :) 

    //                1 <~ edge of NDC film plane
    //               -|                          
    //              - |                          
    //  angle_y    -  |           y              
    //         \  -   |       -   |              
    //          |-    |   -       |              
    //          v     y'          |              
    //         -) -   |           |              
    //        0-------+-----------+----->        
    //              D / r_y      -z              
    //                ^                          
    //                |                          
    //                cot(angle_y) := Q_y        

    // similar triangles has y' / Q_y = y / -z                     
    //                          => y' = -Q_y * (y / z) (Equation 1)

    // we can repeat this procedure in x      
    // the only difference is Q_x vs. Q_y     
    // -------------------------------------- 
    // cot(angle_x) = D / r_x                 
    // cot(angle_y) = D / r_y                 
    // => r_x cot(angle_x) = r_y cot(angle_y) 
    // recall: aspect := r_x / r_y            
    //  => aspect cot(angle_x) = cot(angle_y) 
    //                  => Q_x = Q_y / aspect.

    // encode Equation 1 (and the variant for x) into a homogeneous matrix equation
    // the third row is a   typical clip plane mapping                             

    //  NDC                    Camera
    //  [x'] = [Q_x   0  0  0] [x] = [ Q_x * x] ~> [-Q_x * (x / z)]
    //  [y'] = [  0 Q_y  0  0] [y] = [ Q_y * y] ~> [-Q_y * (y / z)]
    //  [z'] = [  0   0  a  b] [z] = [  az + b] ~> [      -a - b/z]
    //  [ 1] = [  0   0 -1  0] [1] = [      -z] ~> [             1]

    real angle_y = angle_of_view / 2;
    real Q_y = 1 / TAN(angle_y);
    real Q_x = Q_y / aspect;

    mat4 result = {};
    result(0, 0) = Q_x;
    result(1, 1) = Q_y;
    result(3, 2) = -1;

    // z'(z) = [-a - b/z]              
    // we want to map [n, f] -> [-1, 1]
    // z'(n) = -a - b/n := -1          
    // z'(f) = -a - b/f :=  1          
    //                                 
    // => a + b/n =  1                 
    //    a + b/f = -1                 
    // => b/n - b/f = 2                
    //                                 
    // => b * (f - n) / (n * f) = 2    
    // => b = (2 * n * f) / (f - n)    
    //                                 
    // => a + (2 * f) / (f - n) = 1    
    // => a = -(n + f) / (f - n)       
    //       = (n + f) / (n - f)       
    result(2, 2) = (near_z_Camera + far_z_Camera) / (near_z_Camera - far_z_Camera);
    result(2, 3) = (2 * near_z_Camera * far_z_Camera) / (far_z_Camera - near_z_Camera);

    // [1 0 0  t_x_NDC] [Q_x   0  0  0]
    // [0 1 0  t_y_NDC] [  0 Q_y  0  0]
    // [0 0 1        0] [  0   0  a  b]
    // [0 0 0        1] [  0   0 -1  0]
    result(0, 2) = -post_nudge_NDC.x;
    result(1, 2) = -post_nudge_NDC.y;

    return result;
}

mat4 get_P_ortho(real height_World, vec2 post_nudge_NDC = {}, real near_z_Camera = 0, real far_z_Camera = 0, real aspect = 0) {
    // ASSERT(!IS_ZERO(height_World));
    if (ARE_EQUAL(near_z_Camera, far_z_Camera)) {
        near_z_Camera = 10000.0f;
        far_z_Camera = -near_z_Camera;
    }
    if (IS_ZERO(aspect)) { aspect = window_get_aspect(); }

    // consider a point with coordinates (x, y, z) in the camera's coordinate system

    // 1) imagine projecting the point onto some film plane with height r_y

    // r_y                                  
    // |                                    
    // |                                    
    // +-----------y                        
    // |           |                        
    // |           |                        
    // +-----------------> minus_z direction

    // 2) scale everything by 1 / r_y to yield NDC film plane (with height 1)

    // 1                                     
    // |                                     
    // |                                     
    // y'----------y / r_y                   
    // |           |                         
    // |           |                         
    // +-----------------> minus_z  direction

    // => y' = y / r_y

    // NDC                        Camera
    // [x'] = [1/r_x      0   0  0] [x] = [ x/r_x]
    // [y'] = [    0  1/r_y   0  0] [y] = [ y/r_y]
    // [z'] = [    0      0   a  b] [z] = [az + b]
    // [1 ] = [    0      0   0  1] [1] = [     1]

    // z'(z) = [az + b]                
    // we want to map [n, f] -> [-1, 1]
    // z'(n) = an + b := -1            
    // z'(f) = af + b :=  1            
    //                                 
    // => a * (f - n) = 2              
    //    a = 2 / (f - n)              
    //                                 
    // (2 * f) / (f - n) + b = 1       
    // => b = (n + f) / (n - f)        

    real r_y = height_World / 2;
    real r_x = window_get_aspect() * r_y;
    real a = 2.0f / (far_z_Camera - near_z_Camera);
    real b = (near_z_Camera + far_z_Camera) / (near_z_Camera - far_z_Camera);

    mat4 result = {};
    result(0, 0) = 1.0f / r_x;
    result(1, 1) = 1.0f / r_y;
    result(2, 2) = a;
    result(2, 3) = b;
    result(3, 3) = 1.0f;

    // [1 0 0  t_x] [1/r_x      0   0  0]
    // [0 1 0  t_y] [    0  1/r_y   0  0]
    // [0 0 1    0] [    0      0   a  b]
    // [0 0 0    1] [    0      0   0  1]

    result(0, 3) = post_nudge_NDC.x;
    result(1, 3) = post_nudge_NDC.y;

    return result;
}


////////////////////////////////////////////////////////////////////////////////
// #include "camera.cpp"////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// NOTE: this can be a 3D orbit camera (persp or ortho) or a 2D camera
struct Camera {
    real angle_of_view; // NOTE: 0.0f <=> ortho camera
                        // NOTE: this is the full (double) angle of view, NOT half
    union {
        real persp_distance_to_origin_World;
        real ortho_screen_height_World;
    };
    vec3 euler_angles;
    vec2 pre_nudge_World;
    vec2 post_nudge_NDC;
};

Camera make_Camera2D(real screen_height_World, vec2 center_World, vec2 post_nudge_NDC) {
    Camera result = {};
    result.ortho_screen_height_World = screen_height_World;
    result.pre_nudge_World = center_World;
    result.post_nudge_NDC = post_nudge_NDC;
    return result;
}

Camera make_OrbitCamera3D(real angle_of_view, real distance_to_origin_World, vec3 euler_angles, vec2 pre_nudge_World, vec2 post_nudge_NDC) {
    Camera result = {};
    result.angle_of_view = angle_of_view;
    result.persp_distance_to_origin_World = distance_to_origin_World;
    result.euler_angles = euler_angles;
    result.pre_nudge_World = pre_nudge_World;
    result.post_nudge_NDC = post_nudge_NDC;
    return result;
}

Camera make_EquivalentCamera2D(Camera *camera_3D) {
    bool is_perspective_camera = (!IS_ZERO(camera_3D->angle_of_view));
    Camera result; {
        result = *camera_3D;
        result.angle_of_view = 0.0f;
        result.euler_angles = {};
        if (is_perspective_camera) result.ortho_screen_height_World = 2.0f * (camera_3D->persp_distance_to_origin_World * TAN(0.5f * camera_3D->angle_of_view));
    }
    return result;
}

mat4 camera_get_P(Camera *camera) {
    if (IS_ZERO(camera->angle_of_view)) {
        return get_P_ortho(camera->ortho_screen_height_World, camera->post_nudge_NDC);
    } else {
        return get_P_persp(camera->angle_of_view, camera->post_nudge_NDC);
    }
}

mat4 camera_get_V(Camera *camera) {
    mat4 C; {
        mat4 T = M4_Translation(camera->pre_nudge_World.x, camera->pre_nudge_World.y, camera->persp_distance_to_origin_World);
        mat4 R_x = M4_RotationAboutXAxis(camera->euler_angles.x);
        mat4 R_y = M4_RotationAboutYAxis(camera->euler_angles.y);
        mat4 R_z = M4_RotationAboutZAxis(camera->euler_angles.z);
        C = R_y * R_x * R_z * T;
    }
    return inverse(C);
}

mat4 camera_get_PV(Camera *camera) { return camera_get_P(camera) * camera_get_V(camera); }




////////////////////////////////////////////////////////////////////////////////
// #include "input_and_callback.cpp"////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void _callback_framebuffer_size(GLFWwindow *, int width, int height) { glViewport(0, 0, width, height); }
void _callback_set_callbacks() { glfwSetFramebufferSizeCallback(COW0._window_glfw_window, _callback_framebuffer_size); }

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

////////////////////////////////////////////////////////////////////////////////
// #include "soup.cpp"//////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define SOUP_POINTS         GL_POINTS
#define SOUP_LINES          GL_LINES
#define SOUP_LINE_STRIP     GL_LINE_STRIP
#define SOUP_LINE_LOOP      GL_LINE_LOOP
#define SOUP_TRIANGLES      GL_TRIANGLES
#define SOUP_TRIANGLE_FAN   GL_TRIANGLE_FAN
#define SOUP_TRIANGLE_STRIP GL_TRIANGLE_STRIP
#define SOUP_OUTLINED_TRIANGLES  254
#define SOUP_QUADS          255
#define SOUP_OUTLINED_QUADS      253


#define _SOUP_XY 2
#define _SOUP_XYZ 3
#define _SOUP_XYZW 4
#define _SOUP_RGB 3
#define _SOUP_RGBA 4

void _soup_init() {
    COW0._soup_shader_program_POINTS = _shader_compile_and_build_program(COWX._soup_vert, COWX._soup_frag_POINTS, COWX._soup_geom_POINTS);
    COW0._soup_shader_program_LINES = _shader_compile_and_build_program(COWX._soup_vert, COWX._soup_frag, COWX._soup_geom_LINES);
    COW0._soup_shader_program_TRIANGLES = _shader_compile_and_build_program(COWX._soup_vert, COWX._soup_frag);
    glGenVertexArrays(ARRAY_LENGTH(COW0._soup_VAO), COW0._soup_VAO);
    glGenBuffers(ARRAY_LENGTH(COW0._soup_VBO), COW0._soup_VBO);
    glGenBuffers(ARRAY_LENGTH(COW0._soup_EBO), COW0._soup_EBO);
}

void _soup_draw(
        real *PVM,
        int primitive,
        int dimension_of_positions,
        int dimension_of_colors,
        int num_vertices,
        real *vertex_positions,
        real *vertex_colors,
        real r_if_vertex_colors_is_NULL,
        real g_if_vertex_colors_is_NULL,
        real b_if_vertex_colors_is_NULL,
        real a_if_vertex_colors_is_NULL,
        real size_in_pixels,
        bool force_draw_on_top) {

    if (num_vertices == 0) { return; } // NOTE: num_vertices zero is valid input

    ASSERT(PVM);
    ASSERT(dimension_of_positions >= 1 && dimension_of_positions <= 4);
    if (vertex_colors) ASSERT(dimension_of_colors == 3 || dimension_of_colors == 4);
    ASSERT(dimension_of_colors >= 0);
    ASSERT(vertex_positions);



    int mesh_special_case = 0;
    { // recursive calls
        {
            if (primitive == SOUP_OUTLINED_TRIANGLES || primitive == SOUP_OUTLINED_QUADS) {
                _soup_draw(
                        PVM, primitive == SOUP_OUTLINED_TRIANGLES ? SOUP_TRIANGLES : SOUP_QUADS,
                        dimension_of_positions,
                        dimension_of_colors,
                        num_vertices,
                        vertex_positions,
                        vertex_colors,
                        r_if_vertex_colors_is_NULL,
                        g_if_vertex_colors_is_NULL,
                        b_if_vertex_colors_is_NULL,
                        a_if_vertex_colors_is_NULL,
                        size_in_pixels,
                        force_draw_on_top);

                if (primitive == SOUP_OUTLINED_TRIANGLES) {
                    mesh_special_case = 1;
                } else {
                    mesh_special_case = 2;
                }

                primitive = SOUP_LINES;
                vertex_colors = NULL;

                // FORNOW
                r_if_vertex_colors_is_NULL = 1.0;
                g_if_vertex_colors_is_NULL = 1.0;
                b_if_vertex_colors_is_NULL = 1.0;
                a_if_vertex_colors_is_NULL = 1.0;
            }
        }

        if (config.tweaks_soup_draw_with_rounded_corners_for_all_line_primitives && (primitive == SOUP_LINES || primitive == SOUP_LINE_STRIP || primitive == SOUP_LINE_LOOP)) {
            _soup_draw(
                    PVM,
                    SOUP_POINTS,
                    dimension_of_positions,
                    dimension_of_colors,
                    num_vertices,
                    vertex_positions,
                    vertex_colors,
                    r_if_vertex_colors_is_NULL,
                    g_if_vertex_colors_is_NULL,
                    b_if_vertex_colors_is_NULL,
                    a_if_vertex_colors_is_NULL,
                    size_in_pixels,
                    force_draw_on_top);
        }
    }


    if (IS_ZERO(size_in_pixels)) { size_in_pixels = config.tweaks_size_in_pixels_soup_draw_defaults_to_if_you_pass_0_for_size_in_pixels; }


    real color_if_vertex_colors_is_NULL[4] = { r_if_vertex_colors_is_NULL, g_if_vertex_colors_is_NULL, b_if_vertex_colors_is_NULL, a_if_vertex_colors_is_NULL };

    glBindVertexArray(COW0._soup_VAO[mesh_special_case]);
    int i_attrib = 0;
    auto guarded_push = [&](int buffer_size, void *array, int dim) {
        glDisableVertexAttribArray(i_attrib); // fornow
        if (array) {
            glBindBuffer(GL_ARRAY_BUFFER, COW0._soup_VBO[i_attrib]);
            glBufferData(GL_ARRAY_BUFFER, buffer_size, array, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(i_attrib, dim, GL_REAL, 0, 0, NULL);
            glEnableVertexAttribArray(i_attrib);
        }
        ++i_attrib;
    };
    int vvv_size = int(num_vertices * dimension_of_positions * sizeof(real));
    int ccc_size = int(num_vertices * dimension_of_colors * sizeof(real));
    guarded_push(vvv_size, vertex_positions, dimension_of_positions);
    guarded_push(ccc_size, vertex_colors, dimension_of_colors);

    int shader_program_ID = 0; {
        if (primitive == SOUP_POINTS) {
            shader_program_ID = COW0._soup_shader_program_POINTS;
        } else if (primitive == SOUP_LINES || primitive == SOUP_LINE_STRIP || primitive == SOUP_LINE_LOOP) {
            shader_program_ID = COW0._soup_shader_program_LINES;
        } else { // including SOUP_QUADS
            shader_program_ID = COW0._soup_shader_program_TRIANGLES;
        }
    }
    ASSERT(shader_program_ID);
    glUseProgram(shader_program_ID);

    _shader_set_uniform_real(shader_program_ID, "aspect", window_get_aspect());
    _shader_set_uniform_real(shader_program_ID, "primitive_radius_NDC", 0.5f * size_in_pixels / window_get_size_Pixel().y);
    _shader_set_uniform_bool(shader_program_ID, "has_vertex_colors", vertex_colors != NULL);
    _shader_set_uniform_bool(shader_program_ID, "force_draw_on_top", force_draw_on_top);
    _shader_set_uniform_mat4(shader_program_ID, "PVM", PVM);
    _shader_set_uniform_vec4(shader_program_ID, "color_if_vertex_colors_is_NULL", color_if_vertex_colors_is_NULL);

    if (primitive != SOUP_QUADS && !mesh_special_case) {
        glDrawArrays(primitive, 0, num_vertices);
    } else {
        // we upload three EBO's _once_               
        // and bind the appropriate one before drawing

        ASSERT(primitive == SOUP_QUADS || mesh_special_case != 0);

        const int MAX_VERTICES = 1000000;
        ASSERT(num_vertices <= MAX_VERTICES);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, COW0._soup_EBO[mesh_special_case]);

        if (primitive == SOUP_QUADS) {
            primitive = SOUP_TRIANGLES;
            num_vertices = (num_vertices / 4) * 6;
            static GLuint *indices;
            if (!indices) {
                indices = (GLuint *) malloc(MAX_VERTICES / 4 * 6 * sizeof(GLuint));
                int k = 0;
                for (int i = 0; i < MAX_VERTICES / 4; ++i) {
                    indices[k++] = 4 * i + 2;
                    indices[k++] = 4 * i + 1;
                    indices[k++] = 4 * i + 0;
                    indices[k++] = 4 * i + 3;
                    indices[k++] = 4 * i + 2;
                    indices[k++] = 4 * i + 0;
                }
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_VERTICES / 4 * 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);
            }
        } else {
            if (mesh_special_case == 1) {
                num_vertices = (num_vertices / 3) * 6;
                static GLuint *indices;
                if (!indices) {
                    indices = (GLuint *) malloc(MAX_VERTICES / 3 * 6 * sizeof(GLuint));
                    int k = 0;
                    for (int i = 0; i < MAX_VERTICES / 3; ++i) {
                        indices[k++] = 3 * i + 0;
                        indices[k++] = 3 * i + 1;
                        indices[k++] = 3 * i + 1;
                        indices[k++] = 3 * i + 2;
                        indices[k++] = 3 * i + 2;
                        indices[k++] = 3 * i + 0;
                    }
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_VERTICES / 3 * 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);
                }
            } else {
                ASSERT(mesh_special_case == 2);
                num_vertices = (num_vertices / 4) * 8;
                static GLuint *indices;
                if (!indices) {
                    indices = (GLuint *) malloc(MAX_VERTICES / 4 * 8 * sizeof(GLuint));
                    int k = 0;
                    for (int i = 0; i < MAX_VERTICES / 4; ++i) {
                        indices[k++] = 4 * i + 0;
                        indices[k++] = 4 * i + 1;
                        indices[k++] = 4 * i + 1;
                        indices[k++] = 4 * i + 2;
                        indices[k++] = 4 * i + 2;
                        indices[k++] = 4 * i + 3;
                        indices[k++] = 4 * i + 3;
                        indices[k++] = 4 * i + 0;
                    }
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_VERTICES / 4 * 8 * sizeof(GLuint), indices, GL_STATIC_DRAW);
                }
            }
        }
        glDrawElements(primitive, num_vertices, GL_UNSIGNED_INT, NULL);
    }

}

template <uint D_pos, uint D_color = 3> void soup_draw(
        mat4 PVM,
        int primitive,
        int num_vertices,
        Vector<D_pos> *vertex_positions,
        Vector<D_color> *vertex_colors,
        Vector<D_color> color_if_vertex_colors_is_NULL = { 1.0, 0.0, 1.0 },
        real size_in_pixels = 0,
        bool force_draw_on_top = false) {
    STATIC_ASSERT(D_pos == 2 || D_pos == 3 || D_pos == 4);
    STATIC_ASSERT(D_color == 3 || D_color == 4);

    _soup_draw(
            PVM.data,
            primitive,
            D_pos,
            D_color,
            num_vertices,
            (real *) vertex_positions,
            (real *) vertex_colors,
            color_if_vertex_colors_is_NULL[0],
            color_if_vertex_colors_is_NULL[1],
            color_if_vertex_colors_is_NULL[2],
            (D_color == 4) ? color_if_vertex_colors_is_NULL[3] : 1,
            size_in_pixels,
            force_draw_on_top
            );
}

template <uint D_pos, uint D_color = 3> void soup_draw(
        mat4 PVM,
        int primitive,
        int num_vertices,
        Vector<D_pos> *vertex_positions,
        void *vertex_colors = NULL,
        Vector<D_color> color_if_vertex_colors_is_NULL = { 1.0, 0.0, 1.0 },
        real size_in_pixels = 0,
        bool force_draw_on_top = false) {

    ASSERT(vertex_colors == NULL);

    _soup_draw(
            PVM.data,
            primitive,
            D_pos,
            D_color,
            num_vertices,
            (real *) vertex_positions,
            (real *) vertex_colors,
            color_if_vertex_colors_is_NULL[0],
            color_if_vertex_colors_is_NULL[1],
            color_if_vertex_colors_is_NULL[2],
            (D_color == 4) ? color_if_vertex_colors_is_NULL[3] : 1,
            size_in_pixels,
            force_draw_on_top
            );
}

////////////////////////////////////////////////////////////////////////////////
// #include "easy_soup.cpp"/////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define ESO_MAX_VERTICES 9999999

void _eso_reset() {
    COW1._eso_current_color[0] = 1.0;
    COW1._eso_current_color[1] = 0.0;
    COW1._eso_current_color[2] = 1.0;
    COW1._eso_current_color[3] = 1.0;
}

void _eso_init() {
    COW0._eso_vertex_positions = (real *) calloc(ESO_MAX_VERTICES, 3 * sizeof(real));
    COW0._eso_vertex_colors = (real *) calloc(ESO_MAX_VERTICES, 4 * sizeof(real));
    _eso_reset();
}

void _eso_begin(real *PVM, int primitive, real size_in_pixels, bool force_draw_on_top) {
    ASSERT(!COW1._eso_called_eso_begin_before_calling_eso_vertex_or_eso_end);
    COW1._eso_called_eso_begin_before_calling_eso_vertex_or_eso_end = true;
    COW1._eso_primitive = primitive;
    COW1._eso_size_in_pixels = size_in_pixels;
    COW1._eso_overlay = force_draw_on_top;
    COW1._eso_num_vertices = 0;
    ASSERT(PVM);
    memcpy(COW1._eso_PVM, PVM, 16 * sizeof(real));
}

void eso_end() {
    ASSERT(COW1._eso_called_eso_begin_before_calling_eso_vertex_or_eso_end);
    COW1._eso_called_eso_begin_before_calling_eso_vertex_or_eso_end = false;
    _soup_draw(
            COW1._eso_PVM,
            COW1._eso_primitive,
            _SOUP_XYZ,
            _SOUP_RGBA,
            COW1._eso_num_vertices,
            COW0._eso_vertex_positions,
            COW0._eso_vertex_colors,
            0.0,
            0.0,
            0.0,
            0.0,
            COW1._eso_size_in_pixels,
            COW1._eso_overlay
            );
}

void eso_vertex(real x, real y, real z) {
    ASSERT(COW1._eso_called_eso_begin_before_calling_eso_vertex_or_eso_end);
    ASSERT(COW1._eso_num_vertices < ESO_MAX_VERTICES);
    real p[3] = { x, y, z };
    memcpy(COW0._eso_vertex_positions + 3 * COW1._eso_num_vertices, p, 3 * sizeof(real));
    memcpy(COW0._eso_vertex_colors + 4 * COW1._eso_num_vertices, COW1._eso_current_color, 4 * sizeof(real));
    ++COW1._eso_num_vertices;
}

void eso_vertex(real x, real y) { eso_vertex(x, y, 0.0f); }

void eso_color(real r, real g, real b, real a = 1.0) {
    COW1._eso_current_color[0] = r;
    COW1._eso_current_color[1] = g;
    COW1._eso_current_color[2] = b;
    COW1._eso_current_color[3] = a;
}

void eso_begin(mat4 PVM, int primitive, real size_in_pixels = 0, bool force_draw_on_top = false) {
    _eso_begin(PVM.data, primitive, size_in_pixels, force_draw_on_top);
}

void eso_vertex(vec2 xy) {
    eso_vertex(xy[0], xy[1]);
}

void eso_vertex(vec3 xyz) {
    eso_vertex(xyz[0], xyz[1], xyz[2]);
}

void eso_color(vec3 rgb, real a = 1.0) {
    eso_color(rgb[0], rgb[1], rgb[2], a);
}

////////////////////////////////////////////////////////////////////////////////
// #include "text.cpp"//////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// TODO no_billboard

void _text_draw(
        real *PV,
        char *text,
        real x_world,
        real y_world,
        real z_world,
        real r,
        real g,
        real b,
        real a,
        real font_size_in_pixels,
        real dx_in_pixels,
        real dy_in_pixels,
        bool force_draw_on_top
        ) {
    ASSERT(PV);
    ASSERT(text);

    real window_width_in_pixels, window_height_in_pixels;
    {
        vec2 size = window_get_size_Pixel();
        window_width_in_pixels = size.x;
        window_height_in_pixels = size.y;
    }

    if (IS_ZERO(font_size_in_pixels)) { font_size_in_pixels = 12; }
    // font_size_in_pixels *= config.tweaks_scale_factor_for_everything_involving_pixels_ie_gui_text_soup_NOTE_this_will_init_to_2_on_macbook_retina;

    static char buffer[99999]; // ~500 chars
    int num_quads = stb_easy_font_print(0, 0, text, NULL, buffer, sizeof(buffer));
    int num_vertices = 4 * num_quads;
    static real vertex_positions[99999];

    char *read_head = buffer;
    for (int k = 0; k < num_vertices; ++k) {
        for (int d = 0; d < 2; ++d) {
            vertex_positions[2 * k + d] = ((float *) read_head)[d];
        }
        read_head += (3 * sizeof(float) + 4);
    }

    real s_NDC[4] = {}; {
        real s_World[4] = { x_world, y_world, z_world, 1 };
        _linalg_mat4_times_vec4_persp_divide(s_NDC, PV, s_World);
    }

    if (IS_BETWEEN(s_NDC[2], -1, 1)) {
        real transform[16] = {}; {
            // M4_Translation(s_NDC) * app_NDC_from_Screen() * M4_Translation(ds_Screen + dims / 2) * M4_Scaling(size, size);

            real TS[16] = {
                font_size_in_pixels / 12, 0, 0, dx_in_pixels + window_width_in_pixels / 2,
                0, font_size_in_pixels / 12, 0, dy_in_pixels + window_height_in_pixels / 2,
                0, 0, 1, 0,
                0, 0, 0, 1,
            };

            _linalg_mat4_times_mat4(transform, (real *) &globals.NDC_from_Screen, TS);
            for (int d = 0; d < 3; ++d) transform[4 * d + 3] += s_NDC[d];
        }

        _soup_draw(
                transform,
                SOUP_QUADS,
                _SOUP_XY,
                _SOUP_RGBA,
                num_vertices,
                vertex_positions,
                NULL,
                r,
                g,
                b,
                a,
                0,
                force_draw_on_top
                );
    }
}

////////////////////////////////////////////////////////////////////////////////
// #include "gui.cpp"///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


real FORNOW_gui_printf_red_component = 1.0f;
void gui_printf(const char *format, ...) {
    static char _text[256] = {};
    {
        va_list arg;
        va_start(arg, format);
        vsnprintf(_text, sizeof(_text), format, arg);
        va_end(arg);
    }

    char *text = _text;
    char *sep = strchr(text, '`'); // fornow hacking in two color text
    if (!sep) {
        _text_draw((real *) &globals.NDC_from_Screen, text, COW1._gui_x_curr, COW1._gui_y_curr, 0.0,
                FORNOW_gui_printf_red_component,
                1.0,
                1.0,
                1.0, 0, 0.0, 0.0, true);
    } else {
        real tmp = COW1._gui_x_curr; {
            *sep = 0;
            _text_draw((real *) &globals.NDC_from_Screen, text, COW1._gui_x_curr, COW1._gui_y_curr, 0.0,
                    1.0,
                    1.0,
                    1.0,
                    1.0, 0, 0.0, 0.0, true);
            COW1._gui_x_curr += 2 * stb_easy_font_width(text);
            text = sep + 1;
            _text_draw((real *) &globals.NDC_from_Screen, text, COW1._gui_x_curr, COW1._gui_y_curr, 0.0, 0.0, 1.0, 1.0, 1.0, 0, 0.0, 0.0, true);
        } COW1._gui_x_curr = tmp;
    }

    COW1._gui_y_curr += 12;
}

////////////////////////////////////////////////////////////////////////////////
// #include "indexed.cpp"///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void _mesh_init() {
    COW0._mesh_shader_program = _shader_compile_and_build_program(COWX._mesh_vert, COWX._mesh_frag);
    glGenVertexArrays(1, &COW0._mesh_VAO);
    glGenBuffers(ARRAY_LENGTH(COW0._mesh_VBO), COW0._mesh_VBO);
    glGenBuffers(1, &COW0._mesh_EBO);
    stbi_set_flip_vertically_on_load(true);
}

bool _mesh_texture_find(int *i_texture, char *texture_filename) { // fornow O(n)
    bool already_loaded = false;
    for ((*i_texture) = 0; (*i_texture) < COW1._mesh_num_textures; (*i_texture)++) {
        if (strcmp(texture_filename, COW1._mesh_texture_filenames[*i_texture]) == 0) {
            already_loaded = true;
            break;
        }
    }
    return already_loaded;
}

int _mesh_texture_get_format(int number_of_channels) {
    ASSERT(number_of_channels == 1 || number_of_channels == 3 || number_of_channels == 4);
    return (number_of_channels == 1) ? GL_RED : (number_of_channels == 3) ? GL_RGB : GL_RGBA;
}

uint _mesh_texture_create(char *texture_filename, int width, int height, int number_of_channels, u8 *data) {
    ASSERT(texture_filename);
    ASSERT(COW1._mesh_num_textures < ITRI_MAX_NUM_TEXTURES);
    ASSERT(strlen(texture_filename) < ITRI_MAX_FILENAME_LENGTH);
    ASSERT(width > 0);
    ASSERT(height > 0);
    ASSERT(number_of_channels == 1 || number_of_channels == 3 || number_of_channels == 4);
    ASSERT(data);

    int format = _mesh_texture_get_format(number_of_channels);

    int i_texture = COW1._mesh_num_textures++;
    strcpy(COW1._mesh_texture_filenames[i_texture], texture_filename);

    glGenTextures(1, COW1._mesh_textures + i_texture);
    glActiveTexture(GL_TEXTURE0 + i_texture);
    glBindTexture(GL_TEXTURE_2D, COW1._mesh_textures[i_texture]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return COW1._mesh_textures[i_texture];
}

void _mesh_texture_sync_to_GPU(char *texture_filename, int width, int height, int number_of_channels, u8 *data) {
    ASSERT(texture_filename);
    ASSERT(width > 0);
    ASSERT(height > 0);
    ASSERT(number_of_channels == 1 || number_of_channels == 3 || number_of_channels == 4);
    ASSERT(data);

    int format = _mesh_texture_get_format(number_of_channels);

    int i_texture;
    ASSERT(_mesh_texture_find(&i_texture, texture_filename));
    glActiveTexture(GL_TEXTURE0 + i_texture);
    glBindTexture(GL_TEXTURE_2D, COW1._mesh_textures[i_texture]);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
}

uint _mesh_texture_load(char *texture_filename) {
    ASSERT(texture_filename);
    int width, height, number_of_channels;
    u8 *data = stbi_load(texture_filename, &width, &height, &number_of_channels, 0);
    ASSERT(data);
    ASSERT(width > 0);
    ASSERT(height > 0);
    ASSERT(number_of_channels == 3 || number_of_channels == 4);
    uint result = _mesh_texture_create(texture_filename, width, height, number_of_channels, data);
    stbi_image_free(data);
    return result;
}

struct Texture {
    char *name;
    int width;
    int height;
    int number_of_channels;
    u8 *data;
    uint _texture_GLuint;
};

Texture texture_create(char *texture_name, int width = 0, int height = 0, int number_of_channels = 3) {
    ASSERT(texture_name);
    if (width == 0 || height == 0) { // FORNOW
        glfwGetFramebufferSize(COW0._window_glfw_window, &width, &height);
    }
    ASSERT(number_of_channels == 1 || number_of_channels == 3 || number_of_channels == 4);
    Texture texture = {};
    texture.name = texture_name;
    texture.width = width;
    texture.height = height;
    texture.number_of_channels = number_of_channels;
    texture.data = (u8 *) calloc(width * height * number_of_channels, sizeof(u8));
    texture._texture_GLuint = _mesh_texture_create(texture.name, texture.width, texture.height, texture.number_of_channels, texture.data);
    return texture;
}

Texture texture_load(char *texture_filename) {
    ASSERT(texture_filename);
    Texture texture = {};
    texture.name = texture_filename;
    texture.data = stbi_load(texture.name, &texture.width, &texture.height, &texture.number_of_channels, 0);
    ASSERT(texture.data);
    ASSERT(texture.width > 0);
    ASSERT(texture.height > 0);
    ASSERT(texture.number_of_channels == 3 || texture.number_of_channels == 4);
    texture._texture_GLuint = _mesh_texture_create(texture.name, texture.width, texture.height, texture.number_of_channels, texture.data);
    return texture;
}

void texture_set_pixel(Texture *texture, int i, int j, real r, real g = 0.0, real b = 0.0, real a = 1.0) {
    #define REAL2U8(r) (u8(CLAMP(r, 0.0, 1.0) * 255.0))
    int pixel = i * texture->width + j;
    texture->data[pixel * texture->number_of_channels + 0]  = REAL2U8(r);
    if (texture->number_of_channels > 1) {
        texture->data[pixel * texture->number_of_channels + 1]  = REAL2U8(g);
        texture->data[pixel * texture->number_of_channels + 2]  = REAL2U8(b);
    }
    if (texture->number_of_channels > 3) {
        texture->data[pixel * texture->number_of_channels + 3]  = REAL2U8(a);
    }
    #undef REAL2U8
}

void texture_get_pixel(Texture *texture, int i, int j, real *r, real *g = NULL, real *b = NULL, real *a = NULL) {
    ASSERT(r != NULL);

    #define U82COW_REAL(r) real((r) / 255.0f)
    int pixel = i * texture->width + j;
    *r = U82COW_REAL(texture->data[pixel * texture->number_of_channels + 0]);
    if (texture->number_of_channels > 1) {
        if (g != NULL) {
            *g = U82COW_REAL(texture->data[pixel * texture->number_of_channels + 1]);
        }
        if (b != NULL) {
            *b = U82COW_REAL(texture->data[pixel * texture->number_of_channels + 2]);
        }
    }
    if (texture->number_of_channels > 3) {
        if (a != NULL) {
            *a = U82COW_REAL(texture->data[pixel * texture->number_of_channels + 3]);
        }
    }
    #undef REAL2U8
}

void texture_set_pixel(Texture *texture, int i, int j, vec3 rgb, real a = 1.0) {
    texture_set_pixel(texture, i, j, rgb.x, rgb.y, rgb.z, a);
}

void texture_sync_to_GPU(Texture *texture) {
    _mesh_texture_sync_to_GPU(texture->name, texture->width, texture->height, texture->number_of_channels, texture->data);
}

void _mesh_draw(
        real *P,
        real *V,
        real *M,
        int num_triangles,
        int *triangle_indices,
        int num_vertices,
        real *vertex_positions,
        real *vertex_normals = NULL,
        real *vertex_colors = NULL,
        real r_if_vertex_colors_is_NULL = 1,
        real g_if_vertex_colors_is_NULL = 1,
        real b_if_vertex_colors_is_NULL = 1,
        real *vertex_texture_coordinates = NULL,
        char *texture_filename = NULL,
        int num_bones = 0,
        real *bones__NUM_BONES_MAT4S = NULL,
        int *bone_indices__INT4_PER_VERTEX = NULL,
        real *bone_weights__VEC4_PER_VERTEX = NULL
        ) {
    if (num_triangles == 0) { return; } // NOTE: num_triangles zero is now valid input
    ASSERT(P);
    ASSERT(V);
    ASSERT(M);
    ASSERT(vertex_positions);
    real color_if_vertex_colors_is_NULL[3] = { r_if_vertex_colors_is_NULL, g_if_vertex_colors_is_NULL, b_if_vertex_colors_is_NULL };

    int i_texture = -1;
    if (texture_filename) {
        if (!_mesh_texture_find(&i_texture, texture_filename)) {
            _mesh_texture_load(texture_filename);
            _mesh_texture_find(&i_texture, texture_filename);
        }
    }

    // NOTE moved this up before the pushes; is that okay?
    ASSERT(COW0._mesh_shader_program);
    glUseProgram(COW0._mesh_shader_program);

    glBindVertexArray(COW0._mesh_VAO);
    unsigned int i_attrib = 0;
    auto guarded_push = [&](void *array, int dim, int sizeof_type, int GL_TYPE_VAR) {
        ASSERT(i_attrib < ARRAY_LENGTH(COW0._mesh_VBO));
        glDisableVertexAttribArray(i_attrib);
        if (array) {
            glBindBuffer(GL_ARRAY_BUFFER, COW0._mesh_VBO[i_attrib]);
            glBufferData(GL_ARRAY_BUFFER, num_vertices * dim * sizeof_type, array, GL_DYNAMIC_DRAW);
            if (GL_TYPE_VAR == GL_REAL) {
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
    guarded_push(vertex_positions,              3, sizeof(real), GL_REAL);
    guarded_push(vertex_normals,                3, sizeof(real), GL_REAL);
    guarded_push(vertex_colors,                 3, sizeof(real), GL_REAL);
    guarded_push(vertex_texture_coordinates,    2, sizeof(real), GL_REAL);
    guarded_push(bone_indices__INT4_PER_VERTEX, 4, sizeof(int ), GL_INT);
    guarded_push(bone_weights__VEC4_PER_VERTEX, 4, sizeof(real), GL_REAL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, COW0._mesh_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * num_triangles * sizeof(uint), triangle_indices, GL_DYNAMIC_DRAW);

    _shader_set_uniform_mat4(COW0._mesh_shader_program, "P", P);
    _shader_set_uniform_mat4(COW0._mesh_shader_program, "V", V);
    _shader_set_uniform_mat4(COW0._mesh_shader_program, "M", M);
    { // fornow scavenge the camera position from V
        real C[16];
        _linalg_mat4_inverse(C, V);
        real eye_World[4] = { C[3], C[7], C[11], 1 };
        _shader_set_uniform_vec4(COW0._mesh_shader_program, "eye_World", eye_World);
    }
    _shader_set_uniform_bool(COW0._mesh_shader_program, "has_vertex_colors", vertex_colors != NULL);
    _shader_set_uniform_bool(COW0._mesh_shader_program, "has_vertex_normals", vertex_normals != NULL);
    _shader_set_uniform_bool(COW0._mesh_shader_program, "has_vertex_texture_coordinates", vertex_texture_coordinates != NULL);
    _shader_set_uniform_bool(COW0._mesh_shader_program, "has_bones", num_bones != 0);
    _shader_set_uniform_bool(COW0._mesh_shader_program, "has_texture", texture_filename != NULL);
    _shader_set_uniform_int (COW0._mesh_shader_program, "i_texture", MAX(0, i_texture));
    _shader_set_uniform_vec3(COW0._mesh_shader_program, "color_if_vertex_colors_is_NULL", color_if_vertex_colors_is_NULL);

    if (num_bones != 0) {
        _shader_set_uniform_mat4_array(COW0._mesh_shader_program, "bones", num_bones, bones__NUM_BONES_MAT4S);
    }

    glDrawElements(GL_TRIANGLES, 3 * num_triangles, GL_UNSIGNED_INT, NULL);
}

void mesh_draw(
        mat4 P,
        mat4 V,
        mat4 M,
        int num_triangles,
        uint3 *triangle_indices,
        int num_vertices,
        vec3 *vertex_positions,
        vec3 *vertex_normals,
        vec3 *vertex_colors,
        vec3 color_if_vertex_colors_is_NULL = { 1.0, 1.0, 1.0 },
        vec2 *vertex_texture_coordinates = NULL,
        char *texture_filename = NULL,
        int num_bones = 0,
        mat4 *bones = NULL,
        uint4 *bone_indices = NULL,
        vec4 *bone_weights = NULL
        ) {
    _mesh_draw(
            P.data,
            V.data,
            M.data,
            num_triangles,
            (int *) triangle_indices,
            num_vertices,
            (real *) vertex_positions,
            (real *) vertex_normals,
            (real *) vertex_colors,
            color_if_vertex_colors_is_NULL[0],
            color_if_vertex_colors_is_NULL[1],
            color_if_vertex_colors_is_NULL[2],
            (real *) vertex_texture_coordinates,
            texture_filename,
            num_bones,
            (real *) bones,
            (int *) bone_indices,
            (real *) bone_weights
            );
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// extras //////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


vec3 color_kelly(int i) {
    static vec3 _kelly_colors[]={{255.f/255,179.f/255,0.f/255},{128.f/255,62.f/255,117.f/255},{255.f/255,104.f/255,0.f/255},{166.f/255,189.f/255,215.f/255},{193.f/255,0.f/255,32.f/255},{206.f/255,162.f/255,98.f/255},{129.f/255,112.f/255,102.f/255},{0.f/255,125.f/255,52.f/255},{246.f/255,118.f/255,142.f/255},{0.f/255,83.f/255,138.f/255},{255.f/255,122.f/255,92.f/255},{83.f/255,55.f/255,122.f/255},{255.f/255,142.f/255,0.f/255},{179.f/255,40.f/255,81.f/255},{244.f/255,200.f/255,0.f/255},{127.f/255,24.f/255,13.f/255},{147.f/255,170.f/255,0.f/255},{89.f/255,51.f/255,21.f/255},{241.f/255,58.f/255,19.f/255},{35.f/255,44.f/255,22.f/255}};
    return _kelly_colors[MODULO(i, ARRAY_LENGTH(_kelly_colors))];
}

vec3 color_plasma(real t) {
    t = CLAMP(t, 0.0f, 1.0f);
    const vec3 c0 = { 0.058732343923997f, 0.023336708925656f, 0.543340182674875f };
    const vec3 c1 = { 2.176514634195958f, 0.238383417126018f, 0.753960459978403f };
    const vec3 c2 = { -2.68946047645803f, -7.45585113573890f, 3.110799939717086f };
    const vec3 c3 = { 6.130348345893603f, 42.34618814772270f, -28.5188546533215f };
    const vec3 c4 = { -11.1074361906227f, -82.6663110942804f, 60.13984767418263f };
    const vec3 c5 = { 10.02306557647065f, 71.41361770095349f, -54.0721865556006f };
    const vec3 c6 = { -3.65871384277778f, -22.9315346546114f, 18.19190778539828f };
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}

vec3 color_rainbow_swirl(real t) {
    #define Q(o) (.5f + .5f * COS(6.28f * ((o) - t)))
    return V3(Q(0.0f), Q(.33f), Q(-.33f));
    #undef Q
}

////////////////////////////////////////////////////////////////////////////////
// #include "mesh.cpp"//////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct Soup3D {
    int primitive;
    int num_vertices;
    vec3 *vertex_positions;
    vec3 *vertex_colors;

    void draw(
            mat4 PVM,
            vec3 color_if_vertex_colors_is_NULL,
            real size_in_pixels,
            bool force_draw_on_top);

    // fornow
    void draw(
            mat4 PVM,
            vec4 color_if_vertex_colors_is_NULL,
            real size_in_pixels,
            bool force_draw_on_top);

    void _dump_for_library(char *filename, char *name);
};

struct IndexedTriangleMesh3D {
    int num_vertices;
    int num_triangles;
    vec3 *vertex_positions;
    vec3 *vertex_normals;
    vec3 *vertex_colors;
    uint3 *triangle_indices;
    vec2 *vertex_texture_coordinates;
    char *texture_filename;
    int num_bones;
    mat4 *bones;
    uint4 *bone_indices;
    vec4 *bone_weights;

    static IndexedTriangleMesh3D combine(int num_meshes, IndexedTriangleMesh3D *meshes) {
        // TODO: check that meshes are "compatible"--i.e., e.g., that they ALL have vertex colors
        IndexedTriangleMesh3D result = {};
        for (int meshIndex = 0; meshIndex < num_meshes; ++meshIndex) {
            IndexedTriangleMesh3D &mesh = meshes[meshIndex];
            result.num_vertices  += mesh.num_vertices;
            result.num_triangles += mesh.num_triangles;
            result.num_bones     += mesh.num_bones;
            ASSERT(!mesh.vertex_texture_coordinates);
            ASSERT(!mesh.texture_filename);
        }
        result.vertex_positions = (vec3 *) malloc(result.num_vertices  * sizeof(vec3));
        result.vertex_normals   = (vec3 *) malloc(result.num_vertices  * sizeof(vec3));
        result.vertex_colors    = (vec3 *) malloc(result.num_vertices  * sizeof(vec3));
        result.triangle_indices = (uint3 *) malloc(result.num_triangles * sizeof(uint3));
        result.bones            = (mat4 *) malloc(result.num_bones     * sizeof(mat4));
        result.bone_indices     = (uint4 *) malloc(result.num_vertices  * sizeof(uint4));
        result.bone_weights     = (vec4 *) malloc(result.num_vertices  * sizeof(vec4));
        int num_vertices = 0;
        int num_triangles = 0;
        int num_bones = 0;
        for (int meshIndex = 0; meshIndex < num_meshes; ++meshIndex) {
            IndexedTriangleMesh3D &mesh = meshes[meshIndex];
            memcpy(result.vertex_positions + num_vertices , mesh.vertex_positions, mesh.num_vertices  * sizeof(vec3));
            memcpy(result.vertex_normals   + num_vertices , mesh.vertex_normals  , mesh.num_vertices  * sizeof(vec3));
            memcpy(result.vertex_colors    + num_vertices , mesh.vertex_colors   , mesh.num_vertices  * sizeof(vec3));
            memcpy(result.triangle_indices + num_triangles, mesh.triangle_indices, mesh.num_triangles * sizeof(uint3));
            if (result.num_bones != 0) {
                memcpy(result.bones            + num_bones    , mesh.bones           , mesh.num_bones     * sizeof(mat4));
                memcpy(result.bone_indices     + num_vertices , mesh.bone_indices    , mesh.num_vertices  * sizeof(uint4));
                memcpy(result.bone_weights     + num_vertices , mesh.bone_weights    , mesh.num_vertices  * sizeof(vec4));
            }

            // patch up triangle_indices and bone_indices
            for (int triangleIndex = 0; triangleIndex < mesh.num_triangles; ++triangleIndex) {
                for (int d = 0; d < 3; ++d) result.triangle_indices[num_triangles + triangleIndex][d] += num_vertices;
            }
            if (result.num_bones != 0) {
                for (int vertexIndex = 0; vertexIndex < mesh.num_vertices; ++vertexIndex) {
                    for (int d = 0; d < 4; ++d) result.bone_indices[num_vertices + vertexIndex][d] += num_bones;
                }
            }

            num_vertices += mesh.num_vertices;
            num_triangles += mesh.num_triangles;
            num_bones += mesh.num_bones;
        }
        return result;
    }

    void draw(
            mat4 P,
            mat4 V,
            mat4 M,
            vec3 color_if_vertex_colors_is_NULL,
            char *texture_filename_if_texture_filename_is_NULL
            );

    void _dump_for_library(char *filename, char *name);

    void _applyTransform(mat4 Model_from_Whatever) {
        // useful for (desctructively) converting from zBrush -> cow
        for (int vertex_i = 0; vertex_i < num_vertices; ++vertex_i) {
            vertex_positions[vertex_i] = transformPoint(Model_from_Whatever, vertex_positions[vertex_i]);
            if (vertex_normals) vertex_normals[vertex_i] = transformNormal(Model_from_Whatever, vertex_normals[vertex_i]);
        }
    }

    vec3 _skin(int i) {
        vec3 result = {};
        for (int d = 0; d < 4; ++d) result += bone_weights[i][d] * transformPoint(bones[bone_indices[i][d]], vertex_positions[i]);
        return result;
    }

    vec3 _skin(uint3 tri, vec3 w) {
        vec3 result = {};
        for (int d = 0; d < 3; ++d) result += w[d] * _skin(tri[d]);
        return result;
    }

};

void Soup3D::draw(
        mat4 PVM,
        vec3 color_if_vertex_colors_is_NULL = { 1.0, 0.0, 1.0 },
        real size_in_pixels = 0,
        bool force_draw_on_top = false) {
    soup_draw(
            PVM,
            primitive,
            num_vertices,
            vertex_positions,
            vertex_colors,
            color_if_vertex_colors_is_NULL,
            size_in_pixels,
            force_draw_on_top);
}

void Soup3D::draw(
        mat4 PVM,
        vec4 color_if_vertex_colors_is_NULL,
        real size_in_pixels = 0,
        bool force_draw_on_top = false) {
    soup_draw(
            PVM,
            primitive,
            num_vertices,
            vertex_positions,
            vertex_colors,
            color_if_vertex_colors_is_NULL,
            size_in_pixels,
            force_draw_on_top);
}

void IndexedTriangleMesh3D::draw(
        mat4 P,
        mat4 V,
        mat4 M,
        vec3 color_if_vertex_colors_is_NULL = { 1.0, 0.0, 1.0 },
        char *texture_filename_if_texture_filename_is_NULL = NULL) {
    mesh_draw(
            P,
            V,
            M,
            num_triangles,
            triangle_indices,
            num_vertices,
            vertex_positions,
            vertex_normals,
            vertex_colors,
            color_if_vertex_colors_is_NULL,
            vertex_texture_coordinates,
            (texture_filename) ? texture_filename : texture_filename_if_texture_filename_is_NULL,
            num_bones,
            bones,
            bone_indices,
            bone_weights
            );
}


