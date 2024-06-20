mat4 OpenGL_from_Pixel;

bool mouse_left_pressed;
bool mouse_left_held;
bool mouse_left_released;

bool mouse_right_pressed;
bool mouse_right_held;
bool mouse_right_released;

vec2 mouse_position_Pixel;
vec2 mouse_position_OpenGL;

real _accumulator_mouse_wheel_offset;
vec2 _accumulator_mouse_change_in_position_Pixel;
vec2 _accumulator_mouse_change_in_position_OpenGL;

vec2 get_mouse_position_World(mat4 PV) {
    return transformPoint(inverse(PV), mouse_position_OpenGL);
}

vec2 _get_mouse_change_in_position_World(mat4 PV) {
    return transformVector(inverse(PV), _accumulator_mouse_change_in_position_OpenGL);
}

#define MAX_NUM_KEYS 512
bool key_pressed[MAX_NUM_KEYS];
bool key_held[MAX_NUM_KEYS];
bool key_released[MAX_NUM_KEYS];
bool key_toggled[MAX_NUM_KEYS];


bool begin_frame() {
    { // clear input before polling
        {
            _accumulator_mouse_wheel_offset = 0.0f;
            _accumulator_mouse_change_in_position_Pixel = {};
            _accumulator_mouse_change_in_position_OpenGL = {};
        }
        {
            memset(key_pressed, 0, sizeof(key_pressed));
            memset(key_released, 0, sizeof(key_released));
        }
    }

    { // gl
        glfwPollEvents();
        glfwSwapBuffers(glfw_window);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }


    OpenGL_from_Pixel = window_get_OpenGL_from_Pixel();

    return (!glfwWindowShouldClose(glfw_window));
}

void easy_callback_key(GLFWwindow *, int key, int, int action, int mods) {
    FORNOW_UNUSED(mods);
    if (key < 0) return;
    if (key >= 512) return;
    if (action == GLFW_PRESS) {
        key_pressed[key] = true;
        key_held[key] = true;
        key_toggled[key] = !key_toggled[key];
    } else if (action == GLFW_RELEASE) {
        key_released[key] = true;
        key_held[key] = false;
    }
}

void easy_callback_cursor_position(GLFWwindow *, double _xpos, double _ypos) {
    vec2 prev_mouse_position_Pixel = mouse_position_Pixel;
    vec2 prev_mouse_position_OpenGL = mouse_position_OpenGL;

    mouse_position_Pixel = { real(_xpos), real(_ypos) };
    mouse_position_OpenGL = transformPoint(OpenGL_from_Pixel, mouse_position_Pixel);

    _accumulator_mouse_change_in_position_Pixel += (mouse_position_Pixel - prev_mouse_position_Pixel);
    _accumulator_mouse_change_in_position_OpenGL += (mouse_position_OpenGL - prev_mouse_position_OpenGL);
}

void easy_callback_mouse_button(GLFWwindow *, int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) { 
            mouse_left_pressed = true;
            mouse_left_held = true;
        } else if (action == GLFW_RELEASE) { 
            mouse_left_released = true;
            mouse_left_held = false;
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) { 
            mouse_right_pressed = true;
            mouse_right_held = true;
        } else if (action == GLFW_RELEASE) { 
            mouse_right_released = true;
            mouse_right_held = false;
        }
    }
}

void easy_callback_scroll(GLFWwindow *, double, double yoffset) {
    _accumulator_mouse_wheel_offset += real(yoffset);
}

void easy_callback_framebuffer_size(GLFWwindow *, int width, int height) {
    glViewport(0, 0, width, height);
}
run_before_main {
    glfwSetFramebufferSizeCallback(glfw_window, easy_callback_framebuffer_size);
    glfwSetKeyCallback(glfw_window, easy_callback_key);
    glfwSetCursorPosCallback(glfw_window, easy_callback_cursor_position);
    glfwSetMouseButtonCallback(glfw_window, easy_callback_mouse_button);
    glfwSetScrollCallback(glfw_window, easy_callback_scroll);
};

void _callback_scroll_helper(Camera *camera_2D, double yoffset) {
    // IDEA: preserve mouse position
    ASSERT(IS_ZERO(camera_2D->angle_of_view));
    ASSERT(IS_ZERO(camera_2D->euler_angles));
    vec2 mouse_position_World_before  = transformPoint(inverse(camera_2D->get_PV()), mouse_position_OpenGL);
    camera_2D->ortho_screen_height_World *= (1.0f - 0.1f * real(yoffset));
    vec2 mouse_position_World_after = transformPoint(inverse(camera_2D->get_PV()), mouse_position_OpenGL);
    camera_2D->pre_nudge_World -= (mouse_position_World_after - mouse_position_World_before);
}

// TODO: include WASD for FirstPersonCamera3D
void Camera::easy_move() {
    if (this->type == CameraType::Camera2D) {
        if (mouse_right_held) {
            this->pre_nudge_World -= _get_mouse_change_in_position_World(this->get_PV());
        }
        if (!IS_ZERO(_accumulator_mouse_wheel_offset)) {
            vec2 mouse_position_World_before  = transformPoint(inverse(this->get_PV()), mouse_position_OpenGL);
            this->ortho_screen_height_World *= (1.0f - 0.1f * real(_accumulator_mouse_wheel_offset));
            vec2 mouse_position_World_after = transformPoint(inverse(this->get_PV()), mouse_position_OpenGL);
            this->pre_nudge_World -= (mouse_position_World_after - mouse_position_World_before);
        }
    } else if (this->type == CameraType::OrbitCamera3D) {
        if (!IS_ZERO(_accumulator_mouse_wheel_offset)) {
            bool is_perspective_camera = (!IS_ZERO(this->angle_of_view));
            Camera tmp_2D = make_EquivalentCamera2D(this);
            _callback_scroll_helper(&tmp_2D, _accumulator_mouse_wheel_offset);
            if (is_perspective_camera) {
                this->persp_distance_to_origin_World = ((0.5f * tmp_2D.ortho_screen_height_World) / TAN(0.5f * this->angle_of_view));
            } else {
                this->ortho_screen_height_World = tmp_2D.ortho_screen_height_World;
            }
            this->pre_nudge_World = tmp_2D.pre_nudge_World;
        }
        if (mouse_left_held) {
            real fac = 2.0f;
            this->euler_angles.y -= fac * _accumulator_mouse_change_in_position_OpenGL.x;
            this->euler_angles.x += fac * _accumulator_mouse_change_in_position_OpenGL.y;
            this->euler_angles.x = CLAMP(this->euler_angles.x, -RAD(90), RAD(90));
        }
        if (mouse_right_held) {
            Camera tmp_2D = make_EquivalentCamera2D(this);
            tmp_2D.pre_nudge_World -= transformVector(inverse(tmp_2D.get_PV()), _accumulator_mouse_change_in_position_OpenGL);
            this->pre_nudge_World = tmp_2D.pre_nudge_World;
        }
    } else {  ASSERT(this->type == CameraType::FirstPersonCamera3D);
        if (key_held['W']) this->pre_nudge_World.y -= 1.0f;
        if (key_held['A']) this->pre_nudge_World.x += 1.0f;
        if (key_held['S']) this->pre_nudge_World.y += 1.0f;
        if (key_held['D']) this->pre_nudge_World.x -= 1.0f;
    }
}

#if 0
void camera_move(Camera2D *camera, bool disable_pan = false, bool disable_zoom = false) {
    real OpenGL_from_World[16] = {};
    _camera_get_PV(camera, OpenGL_from_World);
    if (!disable_pan && globals.mouse_right_held) {
        real dx, dy;
        _input_get_mouse_position_and_change_in_position_in_world_coordinates(OpenGL_from_World, NULL, NULL, &dx, &dy);
        camera->center_World.x -= dx;
        camera->center_World.y -= dy;
    }
    else if (!disable_zoom && !IS_ZERO(globals.mouse_wheel_offset)) {
        camera->height_World *= (1.f - .1f * globals.mouse_wheel_offset);

        // zoom while preserving mouse position                
        //                                                     
        // mouse_World' = mouse_World                          
        // mouse_OpenGL'   = mouse_OpenGL                            
        //                                                     
        // mouse_OpenGL'         = mouse_OpenGL                      
        // P' V' mouse_World' = mouse_OpenGL                     
        // P' V' mouse_World  = mouse_OpenGL                     
        // V' mouse_World     = inv(P') mouse_OpenGL              
        // mouse_World - eye' = inv(P') mouse_OpenGL              
        //               eye' = mouse_World - inv(P') mouse_OpenGL
        //                                    ^----- a -------^

        // TODO: this can (and should) be simplified a lot
        real x, y;
        _input_get_mouse_position_and_change_in_position_in_world_coordinates(OpenGL_from_World, &x, &y, NULL, NULL);

        real mouse_World[4] = { x, y, 0, 1 };
        real a[4] = {};
        real inv_P_prime[16] = {}; {
            Camera2D tmp = *camera;
            _camera_get_P(&tmp, inv_P_prime);
            _linalg_mat4_inverse(inv_P_prime, inv_P_prime);
        }
        real mouse_OpenGL[4] = {}; {
            _linalg_mat4_times_vec4_persp_divide(mouse_OpenGL, OpenGL_from_World, mouse_World); 
        }
        _linalg_mat4_times_vec4_persp_divide(a, inv_P_prime, mouse_OpenGL);
        for (int d = 0; d < 2; ++d) (&camera->center_World.x)[d] = mouse_World[d] - a[d];
    }
}

void camera_move(Camera3D *camera, bool disable_pan = false, bool disable_zoom = false, bool disable_rotate = false) {
    disable_rotate |= (globals._input_owner != COW_INPUT_OWNER_NONE);
    { // 2D transforms
        Camera2D tmp2D = { camera_get_screen_height_World(camera), camera->_center_World.x, camera->_center_World.y, camera->display_nudge_OpenGL.x, camera->display_nudge_OpenGL.y };
        camera_move(&tmp2D, disable_pan, disable_zoom);

        if (IS_ZERO(camera->angle_of_view)) { // ortho
            camera->ortho_screen_height_World = tmp2D.height_World;
        } else { // persp
                 //                   r_y
                 //               -   |  
                 //            -      |  
                 //         -         |  
                 //      - ) theta    |  
                 // eye-------------->o  
                 //         D            
            real r_y = tmp2D.height_World / 2;
            real theta = camera->angle_of_view / 2;
            camera->persp_distance_to_origin = r_y / tan(theta);
        }

        camera->_center_World.x = tmp2D.center_World.x;
        camera->_center_World.y = tmp2D.center_World.y;
    }
    if (!disable_rotate && globals.mouse_left_held) {
        real fac = 2;
        camera->theta -= fac * globals.mouse_change_in_position_OpenGL[0];
        camera->phi += fac * globals.mouse_change_in_position_OpenGL[1];
        camera->phi = CLAMP(camera->phi, -RAD(90), RAD(90));
    }
}
#endif
