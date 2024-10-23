// TODO: an fps camera shouldn't be allowed to be ortho
// TODO: camera should have clip planes as member variables

#ifdef OPERATING_SYSTEM_APPLE
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GL_COREARB
#include <OpenGL/gl3.h>
#include "glfw3.h"
#elif defined(OPERATING_SYSTEM_WINDOWS)
#include "glad.c"
#include "glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include "glfw3native.h"
#endif

GLFWwindow *glfw_window;
real _window_macbook_retina_fixer__VERY_MYSTERIOUS;

run_before_main {
    ASSERT(glfwInit());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 1);

    glfw_window = glfwCreateWindow(960, 720,  __DATE__ " " __TIME__, NULL, NULL);
    if (!glfw_window) {
        printf("Something's gone wonky; if you weren't just messing with init(...) or something, please try restarting your computer and trying again.\n");
        ASSERT(0);
    }
    glfwMakeContextCurrent(glfw_window);
    glfwSetWindowPos(glfw_window, 0, 100);
    glfwSetWindowAttrib(glfw_window, GLFW_FLOATING, false);
    glfwSetWindowAttrib(glfw_window, GLFW_DECORATED, true);

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

    { // _macbook_retina_scale
        int num, den, _;
        glfwGetFramebufferSize(glfw_window, &num, &_);
        glfwGetWindowSize(glfw_window, &den, &_);
        _window_macbook_retina_fixer__VERY_MYSTERIOUS = real(num / den);
    }
};


vec2 window_get_size_Pixel() {
    ASSERT(glfw_window);
    int _width, _height;
    glfwGetFramebufferSize(glfw_window, &_width, &_height);
    real width = real(_width) / _window_macbook_retina_fixer__VERY_MYSTERIOUS;
    real height = real(_height) / _window_macbook_retina_fixer__VERY_MYSTERIOUS;
    return { width, height };
}
uint window_get_width_Pixel() { return window_get_size_Pixel().x; }
uint window_get_height_Pixel() { return window_get_size_Pixel().y; }


void gl_scissor_Pixel(double x, double y, double dx, double dy) {
    // y_Pixel_upper_left -> y_Scissor_upper_left -> y_Scissor_lower_left
    y = window_get_height_Pixel() - y - dy;
    real factor = _window_macbook_retina_fixer__VERY_MYSTERIOUS;
    glScissor(uint(factor * x), uint(factor * y), uint(factor * dx), uint(factor * dy));
}
void gl_scissor_Pixel(bbox2 bbox) {
    gl_scissor_Pixel(bbox.min.x, bbox.min.y, bbox.max.x - bbox.min.x, bbox.max.y - bbox.min.y);
}
#ifdef glScissor
#undef glScissor
#endif
#define glScissor RETINA_BREAKS_THIS_FUNCTION_USE_gl_scissor_WRAPPER



real window_get_aspect() {
    vec2 size = window_get_size_Pixel();
    return size.x / size.y;
}

mat4 window_get_OpenGL_from_Pixel() {
    // OpenGL                         Pixel 
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

mat4 transform_get_P_persp(real angle_of_view, vec2 post_nudge_OpenGL = {}, real near_z_Camera = 0, real far_z_Camera = 0, real aspect = 0) {
    if (IS_ZERO(near_z_Camera)) { near_z_Camera = -0.100f; }
    if (IS_ZERO(far_z_Camera)) { far_z_Camera = -100000.0f; }
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

    // 2) scale film plane by 1 / r_y to yield OpenGL film plane (with height 1) and distance Q_y
    // y' is the projected position of vertex y in OpenGL; i.e., if we can get y', we're done :) 

    //                1 <~ edge of OpenGL film plane
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

    //  OpenGL                    Camera
    //  [x'] = [Q_x   0  0  0] [x] = [ Q_x * x] ~> [-Q_x * (x / z)]
    //  [y'] = [  0 Q_y  0  0] [y] = [ Q_y * y] ~> [-Q_y * (y / z)]
    //  [z'] = [  0   0  a  b] [z] = [  az + b] ~> [      -a - b/z]
    //  [ 1] = [  0   0 -1  0] [1] = [      -z] ~> [             1]

    real angle_y = 0.5f * angle_of_view;
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

    // [1 0 0  t_x_OpenGL] [Q_x   0  0  0]
    // [0 1 0  t_y_OpenGL] [  0 Q_y  0  0]
    // [0 0 1           0] [  0   0  a  b]
    // [0 0 0           1] [  0   0 -1  0]
    result(0, 2) = -post_nudge_OpenGL.x;
    result(1, 2) = -post_nudge_OpenGL.y;

    return result;
}

mat4 transform_get_P_ortho(real height_World, vec2 post_nudge_OpenGL = {}, real near_z_Camera = 0, real far_z_Camera = 0, real aspect = 0) {
    // ASSERT(!IS_ZERO(height_World));
    if (ARE_EQUAL(near_z_Camera, far_z_Camera)) {
        near_z_Camera = 1000000.0f;
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

    // 2) scale everything by 1 / r_y to yield OpenGL film plane (with height 1)

    // 1                                     
    // |                                     
    // |                                     
    // y'----------y / r_y                   
    // |           |                         
    // |           |                         
    // +-----------------> minus_z  direction

    // => y' = y / r_y

    // OpenGL                        Camera
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

    result(0, 3) = post_nudge_OpenGL.x;
    result(1, 3) = post_nudge_OpenGL.y;

    return result;
}

enum class CameraType {
    None,
    Camera2D,
    OrbitCamera3D,
    FirstPersonCamera3D,
};

struct Camera {
    CameraType type;
    real angle_of_view; // NOTE: 0.0f <=> ortho camera
    vec3 euler_angles;
    union {
        struct {
            vec2 pre_nudge_World;
            union {
                real persp_distance_to_origin_World;
                real ortho_screen_height_World;
            };
        };
        vec3 first_person_position_World;
    };
    vec2 post_nudge_OpenGL;

    mat4 get_P();
    mat4 get_V();
    mat4 get_PV();
};

Camera make_Camera2D(real screen_height_World, vec2 center_World = {}, vec2 post_nudge_OpenGL = {}) {
    Camera result = {};
    result.type = CameraType::Camera2D;
    result.ortho_screen_height_World = screen_height_World;
    result.pre_nudge_World = center_World;
    result.post_nudge_OpenGL = post_nudge_OpenGL;
    return result;
}

Camera make_OrbitCamera3D(real distance_to_origin_World, real angle_of_view = RAD(60.0f), vec3 euler_angles = {}, vec2 pre_nudge_World = {}, vec2 post_nudge_OpenGL = {}) {
    Camera result = {};
    result.type = CameraType::OrbitCamera3D;
    result.angle_of_view = angle_of_view;
    result.persp_distance_to_origin_World = distance_to_origin_World;
    result.euler_angles = euler_angles;
    result.pre_nudge_World = pre_nudge_World;
    result.post_nudge_OpenGL = post_nudge_OpenGL;
    return result;
}

Camera make_FirstPersonCamera3D(vec3 first_person_position_World, real angle_of_view = RAD(60.0f), vec3 euler_angles = {}, vec2 post_nudge_OpenGL = {}) {
    Camera result = {};
    result.type = CameraType::FirstPersonCamera3D;
    result.angle_of_view = angle_of_view;
    result.euler_angles = euler_angles;
    result.first_person_position_World = first_person_position_World;
    result.post_nudge_OpenGL = post_nudge_OpenGL;
    return result;
}

Camera make_EquivalentCamera2D(Camera *orbit_camera_3D) {
    ASSERT(orbit_camera_3D->type == CameraType::OrbitCamera3D);
    bool is_perspective_camera = (!IS_ZERO(orbit_camera_3D->angle_of_view));
    Camera result; {
        result = *orbit_camera_3D;
        result.type = CameraType::Camera2D;
        result.angle_of_view = 0.0f;
        result.euler_angles = {};
        if (is_perspective_camera) result.ortho_screen_height_World = 2.0f * (orbit_camera_3D->persp_distance_to_origin_World * TAN(0.5f * orbit_camera_3D->angle_of_view));
    }
    return result;
}


mat4 Camera::get_P() {
    if (IS_ZERO(this->angle_of_view)) {
        return transform_get_P_ortho(this->ortho_screen_height_World, this->post_nudge_OpenGL);
    } else {
        return transform_get_P_persp(this->angle_of_view, this->post_nudge_OpenGL);
    }
}

mat4 Camera::get_V() {
    mat4 C; {
        mat4 T = M4_Translation(this->first_person_position_World);
        mat4 R_x = M4_RotationAboutXAxis(this->euler_angles.x);
        mat4 R_y = M4_RotationAboutYAxis(this->euler_angles.y);
        mat4 R_z = M4_RotationAboutZAxis(this->euler_angles.z);
        mat4 R = (R_y * R_x * R_z);
        C = (this->type != CameraType::FirstPersonCamera3D) ? (R * T) : (T * R);
    }
    return inverse(C);
}

mat4 Camera::get_PV() { return get_P() * get_V(); }


