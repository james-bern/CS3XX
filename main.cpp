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
#if 0
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
                     "^signore.stl\n"
                     "^oignore.stl\n"
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
        glClearColor(omax.black.x, omax.black.y, omax.black.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        eso_size(1.5f);

        other.OpenGL_from_Pixel = window_get_OpenGL_from_Pixel();

        other._please_suppress_drawing_popup_popup = false;

        if (other.stepping_one_frame_while_paused) other.paused = false;
        if (!other.paused) { // update
            { // time_since
                // FORNOW: HAAAAAACK TODO: time frames and actually increment time properly
                real dt = 0.0167f;
                #ifdef OPERATING_SYSTEM_WINDOWS
                dt *= 2;
                #endif
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

