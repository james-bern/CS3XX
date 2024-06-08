
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

// // external defines
GLFWwindow *glfw_window;
#define GL_REAL GL_FLOAT

// // stb
// easy_font
#include "codebase/ext/stb_easy_font.h"
// image
#define STB_IMAGE_IMPLEMENTATION
#include "codebase/ext/stb_image.h"




#define ITRI_MAX_NUM_TEXTURES 32
#define ITRI_MAX_FILENAME_LENGTH 64

// constants
// shaders
struct {
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
} app_shaders;

struct COW0_PersistsAcrossApps_NeverAutomaticallyClearedToZero__ManageItYourself {
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

    real _window_macbook_retina_fixer__VERY_MYSTERIOUS;
};

struct COW1_PersistsAcrossFrames_AutomaticallyClearedToZeroBetweenAppsBycow_reset {
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


COW0_PersistsAcrossApps_NeverAutomaticallyClearedToZero__ManageItYourself COW0;
COW1_PersistsAcrossFrames_AutomaticallyClearedToZeroBetweenAppsBycow_reset COW1;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// core ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


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
void _window_init() {
    ASSERT(glfwInit());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 1);

    // glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    glfw_window = glfwCreateWindow(1000, 1000, "conversation -- " __DATE__ " " __TIME__, NULL, NULL);
    if (!glfw_window) {
        printf("[cow] something's gone wonky; if you weren't just messing with init(...) or something, please try restarting your computer and try again.\n");
        ASSERT(0);
    }


    glfwMakeContextCurrent(glfw_window);

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

    glfwSwapInterval(1);


    glfwSetWindowPos(glfw_window, 0.0f, 0.0f);
    glfwSetWindowSize(glfw_window, 960.0f, 540.0f);
    glfwSetWindowAttrib(glfw_window, GLFW_FLOATING, false);
    glfwSetWindowAttrib(glfw_window, GLFW_DECORATED, true);

    { // _macbook_retina_scale
        int num, den, _;
        glfwGetFramebufferSize(glfw_window, &num, &_);
        glfwGetWindowSize(glfw_window, &den, &_);
        COW0._window_macbook_retina_fixer__VERY_MYSTERIOUS = real(num / den);
    }
}

// // window transforms and camera

vec2 window_get_size_Pixel() {
    ASSERT(glfw_window);
    int _width, _height;
    glfwGetFramebufferSize(glfw_window, &_width, &_height);
    real width = real(_width) / COW0._window_macbook_retina_fixer__VERY_MYSTERIOUS;
    real height = real(_height) / COW0._window_macbook_retina_fixer__VERY_MYSTERIOUS;
    return { width, height };
}

real window_get_aspect() {
    vec2 size = window_get_size_Pixel();
    return size.x / size.y;
}

mat4 window_get_NDC_from_Pixel() {
    // NDC                         Pixel 
    // [x'] = [1/r_x      0   0 -1] [x] = [x/r_x - 1]
    // [y'] = [    0 -1/r_y   0  1] [y] = [1 - y/r_y]
    // [z'] = [    0      0   1  0] [z] = [        z] // so invertible (otherwise, would just have row [ 0 0 0 0 ]
    // [1 ] = [    0      0   0  1] [1] = [        1]
    vec2 r = window_get_size_Pixel() / 2;
    mat4 result = {};
    result(0, 0) = 1.0f / r.x;
    result(1, 1) = -1.0f / r.y;
    result(2, 2) = 1.0f;
    result(0, 3) = -1.0f;
    result(1, 3) = 1.0f;
    result(3, 3) = 1.0f;
    return result;
}

mat4 transform_get_P_persp(real angle_of_view, vec2 post_nudge_NDC = {}, real near_z_Camera = 0, real far_z_Camera = 0, real aspect = 0) {
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

mat4 transform_get_P_ortho(real height_World, vec2 post_nudge_NDC = {}, real near_z_Camera = 0, real far_z_Camera = 0, real aspect = 0) {
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
        return transform_get_P_ortho(camera->ortho_screen_height_World, camera->post_nudge_NDC);
    } else {
        return transform_get_P_persp(camera->angle_of_view, camera->post_nudge_NDC);
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
    COW0._soup_shader_program_POINTS = _shader_compile_and_build_program(app_shaders._soup_vert, app_shaders._soup_frag_POINTS, app_shaders._soup_geom_POINTS);
    COW0._soup_shader_program_LINES = _shader_compile_and_build_program(app_shaders._soup_vert, app_shaders._soup_frag, app_shaders._soup_geom_LINES);
    COW0._soup_shader_program_TRIANGLES = _shader_compile_and_build_program(app_shaders._soup_vert, app_shaders._soup_frag);
    glGenVertexArrays(ARRAY_LENGTH(COW0._soup_VAO), COW0._soup_VAO);
    glGenBuffers(ARRAY_LENGTH(COW0._soup_VBO), COW0._soup_VBO);
    glGenBuffers(ARRAY_LENGTH(COW0._soup_EBO), COW0._soup_EBO);
}

void _soup_draw(
        real *PVM,
        uint primitive,
        uint dimension_of_positions,
        uint dimension_of_colors,
        uint num_vertices,
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

    if (IS_ZERO(size_in_pixels)) { size_in_pixels = 2.0f; }

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

        if (true && (primitive == SOUP_LINES || primitive == SOUP_LINE_STRIP || primitive == SOUP_LINE_LOOP)) {
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

template <uint D_position, uint D_color = 3> void soup_draw(
        mat4 PVM,
        uint primitive,
        uint num_vertices,
        Vector<D_position> *vertex_positions,
        Vector<D_color> *vertex_colors,
        Vector<D_color> color_if_vertex_colors_is_NULL = { 1.0, 0.0, 1.0 },
        real size_in_pixels = 0,
        bool force_draw_on_top = false) {
    STATIC_ASSERT(D_position == 2 || D_position == 3 || D_position == 4);
    STATIC_ASSERT(D_color == 3 || D_color == 4);

    _soup_draw(
            PVM.data,
            primitive,
            D_position,
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

template <uint D_position, uint D_color = 3> void soup_draw(
        mat4 PVM,
        uint primitive,
        uint num_vertices,
        Vector<D_position> *vertex_positions,
        void *vertex_colors = NULL,
        Vector<D_color> color_if_vertex_colors_is_NULL = { 1.0, 0.0, 1.0 },
        real size_in_pixels = 0,
        bool force_draw_on_top = false) {

    ASSERT(vertex_colors == NULL);

    _soup_draw(
            PVM.data,
            primitive,
            D_position,
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

void _eso_begin(real *PVM, uint primitive, real size_in_pixels, bool force_draw_on_top) {
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

void eso_begin(mat4 PVM, uint primitive, real size_in_pixels = 0, bool force_draw_on_top = false) {
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

template <uint D_position, uint D_color> void text_draw(
        mat4 PV,
        char *cstring,
        Vector<D_position> _position_World,
        Vector<D_color> color,
        real font_size_Pixel = 12.0f,
        vec2 nudge_Pixel = {},
        bool force_draw_on_top = true
        ) {
    ASSERT(cstring);

    vec2 *vertex_positions;
    uint num_vertices;
    {
        uint size = 99999 * sizeof(float);
        static void *_vertex_positions = malloc(size);
        vertex_positions = (vec2 *) _vertex_positions;

        num_vertices = 4 * stb_easy_font_print(0, 0, cstring, NULL, _vertex_positions, size);
        { // NOTE: stb stores like this [x:float y:float z:float color:uint8[4]]
            for_(i, num_vertices) {
                ((vec2 *) vertex_positions)[i] = {
                    ((float *) vertex_positions)[4 * i + 0],
                    ((float *) vertex_positions)[4 * i + 1],
                };
            }
        }
    }

    vec3 position_World = { _position_World.x, _position_World.y };
    vec3 position_NDC = transformPoint(PV, position_World);

    if (!IS_BETWEEN(position_NDC.z, -1.0f, 1.0f)) return;

    vec2 position_Pixel = transformPoint(inverse(window_get_NDC_from_Pixel()), _V2(position_NDC));

    mat4 transform = window_get_NDC_from_Pixel()
        * M4_Translation(position_Pixel + nudge_Pixel)
        * M4_Scaling(font_size_Pixel / 12.0f);
    soup_draw(transform, SOUP_QUADS, num_vertices, vertex_positions, NULL, color, 0, force_draw_on_top);
}

////////////////////////////////////////////////////////////////////////////////
// #include "gui.cpp"///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// TODO: color and no color version (see how is done with message)
real FORNOW_gui_printf_red_component = 1.0f;
void gui_printf(const char *format, ...) {
    static char text[256] = {};
    {
        va_list arg;
        va_start(arg, format);
        vsnprintf(text, sizeof(text), format, arg);
        va_end(arg);
    }

    text_draw(window_get_NDC_from_Pixel(), text, V2(COW1._gui_x_curr, COW1._gui_y_curr), V3(FORNOW_gui_printf_red_component, 1.0f, 1.0f));

    COW1._gui_y_curr += 12;
}

////////////////////////////////////////////////////////////////////////////////
// #include "indexed.cpp"///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void _mesh_init() {
    COW0._mesh_shader_program = _shader_compile_and_build_program(app_shaders._mesh_vert, app_shaders._mesh_frag);
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
        glfwGetFramebufferSize(glfw_window, &width, &height);
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
        mat4 C; { // FORNOW
            memcpy(C.data, V, 16 * sizeof(real));
            C = inverse(C);
        }
        real eye_World[4] = { C.data[3], C.data[7], C.data[11], 1 };
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
// #include "mesh.cpp"//////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct Soup3D {
    uint primitive;
    uint num_vertices;
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


