#include "playground.cpp"
#include "arena.cpp"
#include "kitchen.cpp"

#if 0
char *startup_script = "";
#else
// char *startup_script = "^.bz10\t10x-10\t-10";
// char *startup_script = "^.^odemo.dxf\n";
char *startup_script = "^osplash.dxf\n";
#endif

#ifdef SHIP
run_before_main {
    startup_script = "";
    glfwSetWindowTitle(glfw_window, "Conversation pre-alpha " __DATE__ " " __TIME__);
};
#endif

#include "manifoldc.h"
#include "pallete.cpp"
#include "header.cpp"
#include "header2.cpp"

#define DUMMY_HOTKEY 9999

// (global) state
WorldState_ChangesToThisMustBeRecorded_state state;
ScreenState_ChangesToThisDo_NOT_NeedToBeRecorded_other other;

// convenient pointers to shorten xxx.foo.bar into foo->bar
Drawing *drawing = &state.drawing;
FeaturePlaneState *feature_plane = &state.feature_plane;
MeshesReadOnly *meshes = &state.meshes;
PopupState *popup = &state.popup;
ToolboxState *toolbox = &state.toolbox;
TwoClickCommandState *two_click_command = &state.two_click_command;
MeshTwoClickCommandState *mesh_two_click_command = &state.mesh_two_click_command;
Camera *camera_drawing = &other.camera_drawing;
Camera *camera_mesh = &other.camera_mesh;
PreviewState *preview = &other.preview;
Pallete2D *pallete_2D = &other.pallete._2D;
Pallete3D *pallete_3D = &other.pallete._3D;
Pallete *pallete = &other.pallete;
Pallete *target_pallete = &other.target_pallete;


// FORNOW
Event event_passed_to_popups;
bool already_processed_event_passed_to_popups;

#include "boolean.cpp"
#include "misc.cpp"
#include "draw2.cpp"
#include "draw.cpp"

void CLEAR_CANVAS() {
    glClearColor(pallete_2D->background.x, pallete_2D->background.y, pallete_2D->background.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    real x =  get_x_divider_drawing_mesh_Pixel();
    real w = window_get_width_Pixel() - x;
    real h = window_get_height_Pixel();
    glEnable(GL_SCISSOR_TEST);
    gl_scissor_Pixel(x, 0, w, h);

    glClearColor(pallete_3D->background.x, pallete_3D->background.y, pallete_3D->background.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_SCISSOR_TEST);
}

#include "save_and_load.cpp"
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
            glfwSetWindowCloseCallback(glfw_window, callback_window_close);
            { // NOTE: patch first frame mouse position issue
                other.OpenGL_from_Pixel = window_get_OpenGL_from_Pixel();

                { // spoof callback_cursor_position
                    double xpos, ypos;
                    glfwGetCursorPos(glfw_window, &xpos, &ypos);
                    callback_cursor_position(NULL, xpos, ypos);
                }
            }
        }
        // glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        { // cursors_init();
            other.cursors.crosshair = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
            other.cursors.ibeam = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
            other.cursors.hresize = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
            other.cursors.hand = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
        }
        {
            *pallete_2D = _pallete_2D_dark;
            *pallete_3D = _pallete_3D_light;
            *target_pallete = *pallete;
        }
    }




    auto SEND_DUMMY = [&]() {
        // "process" dummy event to draw popups and buttons
        // NOTE: it's a Key;Hotkey event in order to enter that section of the code
        // FORNOW: buttons drawn on EVERY event (no good; TODO fix later)
        Event dummy = {};
        dummy.type = EventType::Key;
        dummy.key_event.subtype = KeyEventSubtype::Hotkey;
        dummy.key_event.key = DUMMY_HOTKEY;
        history_process_event(dummy);
    };

    void _messages_draw(); // forward declaration

    glfwHideWindow(glfw_window); // to avoid one frame flicker 
    uint64_t frame = 0;
    while (!glfwWindowShouldClose(glfw_window)) {
        if (other.slowmo) SLEEP(100);
        glfwSwapBuffers(glfw_window);
        glFinish(); // 69363856
                    // SLEEP(1);

        CLEAR_CANVAS(); // TODO: goes after poll events so that dragging the separator doesn't mess things up with an off by one frame error


        eso_size(1.5f);
        other.OpenGL_from_Pixel = window_get_OpenGL_from_Pixel();
        other._please_suppress_drawing_popup_popup = false;
        other._please_suppress_drawing_toolbox = false;
        memset(popup->a_popup_from_this_group_was_already_called_this_frame, 0, sizeof(popup->a_popup_from_this_group_was_already_called_this_frame));

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
                other.time_since_plane_deselected += dt;
                other.time_since_mouse_moved += dt;
                other.time_since_popup_second_click_not_the_same += dt;
                // time_since_successful_feature = 1.0f;

                bool going_inside = 0
                    || ((state_Mesh_command_is_(ExtrudeAdd)) && (popup->extrude_add_in_length > 0.0f))
                    || (state_Mesh_command_is_(ExtrudeCut));
                if (!going_inside) {
                    other.time_since_going_inside = 0.0f;
                } else {
                    other.time_since_going_inside += dt;
                }
            }


            { // events
                SEND_DUMMY();
                glfwPollEvents();
                if (raw_event_queue.length) {
                    while (raw_event_queue.length) {
                        RawEvent raw_event = queue_dequeue(&raw_event_queue);
                        Event freshly_baked_event = bake_event(raw_event);
                        history_process_event(freshly_baked_event);
                    }
                }
            }

            _messages_update();
        } else {
            SEND_DUMMY();
            glfwPollEvents();
            ;
        }

        // messagef(pallete.white, "%d KB", (drawing->entities.length * sizeof(drawing->entities.array[0])) / 1024);

        { // draw
            conversation_draw();
            if (other.show_console) _messages_draw();
        }

        if (frame++ == 1) glfwShowWindow(glfw_window);
        if (other.stepping_one_frame_while_paused) {
            other.stepping_one_frame_while_paused = false;
            other.paused = true;
        }

    }
}

