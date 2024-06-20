/*
   johnny mnemonic
   keanu reeves, ice t, ...

   movie setting

   what is a computer
   how fast is your computer
   so fast
   but what 

   memes
   */

#if 1
#include "playground.cpp"
#include "easy_mode.cpp"
int main() {
    #if 0
    real t = 0.0f;
    while (begin_frame(NULL)) {
        t += 0.033f;
        eso_begin(OpenGL_from_Pixel, SOUP_LINE_STRIP); {
            eso_size(15.0f + 10.0f * sin(t));
            eso_stipple(0b00000000);
            eso_color(basic.red);
            eso_vertex(50.0f, 50.0f);
            eso_color(basic.green);
            eso_size(15.0f + 10.0f * cos(t));
            eso_vertex(300.0f, 50.0f);

            eso_color(basic.blue);
            eso_stipple(0b11000000);
            eso_size(9.0f);
            eso_vertex(300.0f, 300.0f);
            eso_size(2.0f);
            eso_vertex(50.0f, 300.0f);
        } eso_end();
    }
    #else
    // TODO: branch off and remake reasonable transform app with plane and things
    real time = 0.0f;
    Camera camera_2D = make_Camera2D(256.0f);
    Camera orbit_camera_3D = make_OrbitCamera3D(0.5f * 256.0f / TAN(RAD(30.0f)), RAD(60.0f));
    Camera first_person_camera_3D = make_FirstPersonCamera3D({ 0.0f, 16.0f, 0.5f * 256.0f / TAN(RAD(30.0f)) }, RAD(60.0f));
    Camera *camera = &camera_2D;
    while (begin_frame(camera)) {
        if (key_pressed['1']) {
            camera = &camera_2D;
            pointer_unlock();
        }
        if (key_pressed['2']) {
            camera = &orbit_camera_3D;
            pointer_unlock();
        }
        if (key_pressed['3']) {
            camera = &first_person_camera_3D;
            pointer_lock();
        }
        // TODO: drawing some simple shapes
        // time += 0.0167f;


        if (0) { // dots
            eso_begin(camera->get_PV(), SOUP_POINTS); {
                for_(i, 2048) {
                    real o = i / 100.0f;
                    real theta = 200 * (24.0f * o - time / 10.0f);
                    eso_color(LERP(.5f + .5f * cos(time / 10), color_rainbow_swirl(theta / TAU / 2 + time / 5), color_rainbow_swirl(o - time / TAU)));
                    // eso_size((o) * (6.0f + 5.0f * sin(o - time)));
                    eso_size(3.0f);
                    real r = 24.0f * o;
                    // real theta = (24.0f * o - time / 10.0f) / o;
                    // real theta = (24.0f * o - time / 10.0f) * o;
                    // eso_size(10.0f + 5.0f * sin(o + time));
                    // real r = 15.0f * o;
                    vec2 p = r * e_theta(SQRT(theta) / 2);
                    eso_vertex(p.x, 0.0f, p.y);
                }
            } eso_end();
        }

        { // ground
            eso_begin(camera->get_PV(), SOUP_QUADS);
            real r = 128.0f;
            eso_color(basic.white);
            eso_vertex(-r, 0.0f, -r);
            eso_vertex(-r, 0.0f,  r);
            eso_vertex( r, 0.0f,  r);
            eso_vertex( r, 0.0f, -r);
            eso_end();
        }

        { // gate
            for_(pass, 2) {
                eso_begin(camera->get_PV(), (pass == 0) ? SOUP_LINE_LOOP : SOUP_QUADS);
                eso_size(5.0f);
                if (pass == 0) {
                    eso_color(basic.red);
                } else {
                    eso_color(basic.red, 0.5f);
                }
                real r = 64.0f;
                eso_vertex(-r, -r);
                eso_vertex(-r,  r);
                eso_vertex( r,  r);
                eso_vertex( r, -r);
                eso_end();
            }
        }
    }
    #endif
}
#else
// XXXX: basic 3D grid with lines (have grids on by default)
// TODO: camera reset needs to take into account divider position
// XXXX: switch entity_*(...) over to vec2(...) instead of void(..., real *, real *)
// XXXX: consider cookbook_lambdas -> Cookbook cookbook = cookbook(...)
// TODO: (re)write soup.cpp
// TODOFIRST: eso_size(...) (eso_begin doesn't take size)
// TODO: memcmp to see if should record
// TODO: timer to see if should snapshot

#include "playground.cpp"

char *startup_script = "";
#if 1
run_before_main {
    startup_script = "cz0123456789";
    startup_script = "^osplash.drawing\nysc<m2d 20 20><m2d 16 16><m2d 16 -16><m2d -16 -16><m2d -16 16>[50\n<m3d 0 100 0 0 -1 0><m2d 0 17.5>{47\nc<m2d 16 -16>\t\t100\nsc<m2d 32 -16><m3d 74 132 113 -0.4 -0.6 -0.7>{60\n^oomax.drawing\nsq0sq1y[3\n";
    startup_script = \
                     "cz10\n"
                     "cz\t10\n"
                     "bzx30\t30\n"
                     "ysadcz"
                     "[5\t15\n"
                     "sc<m2d 0 30>qs3"
                     "1<m2d 30 15>0<esc>"
                     "sq1sq3me<m2d 40 40>x15\t15\n"
                     "{3\n"
                     "sczZm<m2d -50 0>]\n"
                     "^n"
                     "cx30\t30\n3.4\n"
                     "saXzYzXzsa[1\n"
                     "^osplash.dxf\nsc<m2d 24 0><m2d 16 0>[\t10\n"
                     "Ac<m2d 15.3 15.4>c<m2d -16.4 -16.3>sc<m2d -16 16>]\n"
                     "^n"
                     "l<m2d 0 0><m2d 0 10>l<m2d 0 10><m2d 10 0>l<m2d 10 0><m2d 0 0>"
                     "n25\n"
                     "sa[1\n"
                     "n0\n"
                     "^n"
                     "cz8\n"
                     "<m3d 1 100 -1 0 -1 0>"
                     "sa{100\n"
                     ";"
                     "^odemo.dxf\n"
                     "^sz.stl\n"
                     "^oz.stl\n"
                     ".."
                     "cz"
                     // "^N^obug.drawing\nysa"
                     ;
};
#endif

#include "manifoldc.h"
#include "header.cpp"

// (global) state
WorldState_ChangesToThisMustBeRecorded_state state;
ScreenState_ChangesToThisDo_NOT_NeedToBeRecorded_other other;

// convenient pointers to shorten xxx.foo.bar into foo->bar
Drawing *drawing = &state.drawing;
FeaturePlaneState *feature_plane = &state.feature_plane;
Mesh *mesh = &state.mesh;
PopupState *popup = &state.popup;
TwoClickCommandState *two_click_command = &state.two_click_command;
Camera *camera_drawing = &other.camera_drawing;
Camera *camera_mesh = &other.camera_mesh;
PreviewState *preview = &other.preview;

#include "misc.cpp"
#include "draw.cpp"
#include "message.cpp"
#include "popup.cpp"
#include "history.cpp"
#include "callbacks.cpp"
#include "bake.cpp"
#include "cookbook.cpp"
#include "process.cpp"
#include "script.cpp"

int main() {
    { // init
        init_camera_drawing();
        init_camera_mesh();
        script_process(STRING(startup_script));
        { // callbacks
            glfwSetKeyCallback(glfw_window, callback_key);
            glfwSetCursorPosCallback(glfw_window, callback_cursor_position);
            glfwSetMouseButtonCallback(glfw_window, callback_mouse_button);
            glfwSetScrollCallback(glfw_window, callback_scroll);
            glfwSetFramebufferSizeCallback(glfw_window, callback_framebuffer_size);
            glfwSetDropCallback(glfw_window, callback_drop);
            { // NOTE: patch first frame mouse position issue
                other.OpenGL_from_Pixel = window_get_OpenGL_from_Pixel();

                double xpos, ypos;
                glfwGetCursorPos(glfw_window, &xpos, &ypos);
                callback_cursor_position(NULL, xpos, ypos);
            }
        }
        // glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    glfwHideWindow(glfw_window); // to avoid one frame flicker 
    uint64_t frame = 0;
    while (!glfwWindowShouldClose(glfw_window)) {
        glfwPollEvents();
        glfwSwapBuffers(glfw_window);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        other.OpenGL_from_Pixel = window_get_OpenGL_from_Pixel();

        other._please_suppress_drawing_popup_popup = false;

        if (other.stepping_one_frame_while_paused) other.paused = false;
        if (!other.paused) { // update
            { // time_since
                real dt = 0.033f;
                _for_each_entity_ entity->time_since_is_selected_changed += dt;
                other.time_since_cursor_start += dt;
                other.time_since_successful_feature += dt;
                other.time_since_plane_selected += dt;
                // time_since_successful_feature = 1.0f;

                bool going_inside = 0
                    || ((state.enter_mode == EnterMode::ExtrudeAdd) && (popup->extrude_add_in_length > 0.0f))
                    || (state.enter_mode == EnterMode::ExtrudeCut);
                if (!going_inside) {
                    other.time_since_going_inside = 0.0f;
                } else {
                    other.time_since_going_inside += dt;
                }
            }

            { // events
                if (raw_event_queue.length) {
                    while (raw_event_queue.length) {
                        RawEvent raw_event = queue_dequeue(&raw_event_queue);
                        Event freshly_baked_event = bake_event(raw_event);
                        freshly_baked_event_process(freshly_baked_event);
                    }
                } else {
                    // NOTE: this is so we draw the popups
                    freshly_baked_event_process({});
                }
            }

            _messages_update();
        } else {
            // "process" dummy event to draw popups
            freshly_baked_event_process({});
        }

        { // draw
            conversation_draw();
        }

        if (frame++ == 1) glfwShowWindow(glfw_window);
        if (other.stepping_one_frame_while_paused) {
            other.stepping_one_frame_while_paused = false;
            other.paused = true;
        }

    }
}
#endif


