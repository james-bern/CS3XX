#if 0

#include "basics.cpp"
#include "arena.cpp"

int main() {
    Arena arena = arena_create();
    void *foo = arena_malloc(&arena, 123);
    void *bar = arena_malloc(&arena, 456);
    void *baz = arena_malloc(&arena, 10001);
    FORNOW_UNUSED(foo);
    FORNOW_UNUSED(bar);
    FORNOW_UNUSED(baz);


    ArenaList<uint> list = { &arena };
    for_(i, 10000) list_push_back(&list, i);

    ArenaMap<uint, uint> map = { &arena };
    map_put(&map, 1U, 5U);
    // printf("%d\n", map_get(1));

    arena_free(&arena);
}


#else

// BETA
// TODO: first and second clicks of FGHI should be hinted (even first!)
// TODO: Pressing enter should spawn a ghost of the pink or blue crosshairs
// TODO: the white box should fade in
// TODO: second click of fillets needs to be previewed in blue; some sort of cool bezier curve interpolation could be nice (or even just lerp between the central arcs?--polygon will be good practice (this could be fun for Nate)
// TODO: cool flash and then fade out with beep (don't actually need beep) for enter with crosshairs
// TODO: use UP and DOWN arrow keys to increase or decrease the field by one if it's a pure number
// TODO: preview fields if mouse not moving
// -- or actually, do both, but have them fade or something if the mouse is/isn't moving (MouseDrawColor, EnterDrawColor)
// -- or actually actually, time_since_current_popup_typed_in (and maybe also a boolean on mouse movement if they change their mind)
// TODO: fillet preview

// TODO: BETA
// - divide shows the point that was divided (animation)

// TODO (Jim): slow mo keybind

// TODO (Jim): cookbook_delete should throw a warning if you're trying to delete the same entity twice (run a O(n) pass on the sorted list)
// TODO (Jim): fillet
// TODO (Jim): power fillet
// TODO (Jim): revolve++ (with same cool animation for partial revolves
// TODO (Jim): dog ear
// TODO (Jim): power dog ear


// TODO (Jim): upgrade test bed
// TODO (Jim): switch everything from radians to turns

// XXXX: stippled lines
// TODO: reset everything in eso on begin

// XXXX: basic 3D grid with lines (have grids on by default)
// ////: camera reset needs to take into account divider position
// XXXX: switch entity_*(...) over to vec2(...) instead of void(..., real *, real *)
// XXXX: consider cookbook_lambdas -> Cookbook cookbook = cookbook(...)
// TODO: (re)write soup.cpp
// TODOFIRST: eso_size(...) (eso_begin doesn't take size)
// TODO: memcmp to see if should record
// TODO: timer to see if should snapshot

#if 0
#define Color0 Green
#define Color1 Red
#define Color2 Pink
#define Color3 Magenta
#define Color4 Purple
#define Color5 Blue
#define Color6 Gray
#define Color7 LightGray
#define Color8 Cyan
#define Color9 Orange
#else
#define Color0 LightGray
#define Color1 Red
#define Color2 Orange
#define Color3 Yellow
#define Color4 Green
#define Color5 Blue
#define Color6 Purple
#define Color7 Brown
#define Color8 DarkGray
#define Color9 XXX
#endif

#include "playground.cpp"
#include "arena.cpp"

char *startup_script = "";

#if 1 // snap
run_before_main {
    startup_script =
        "^."
        "^K"
        "cz10\n"
        "lq<m2d 15 5>"
        "q<m2d -15 5>"
        ;
};
#endif

#if 0 // circle
run_before_main {
    startup_script =
        // "^..cz32\nlq<m2d 10 0>zs<m2d 0 0>\b<esc>le<m2d 10 0>q<m2d -10 0>"
        // "^.cz32\nlq<m2d 10 0>zs<m2d 0 0>\b<esc>"
        // "cz16\n"
        // "s<m2d> 0 0>\b"
        // "lq<m2d 8 0>q<m2d 16 0>"
        // "sc"
        // "^odemo.dxf\n"
        // "bz<m2d 10 10>"
        // "cz<m2d 10 10>"
        // "ysq1[5\n"
        // "^G"
        // "^.^Obuddha.stl\n"
        // "^.^Ocylinder.stl\n"
        // "^odemo.dxf\nsq1y[5\n<m3d 0 10 15 0 -1 0>"
        ;
};
#endif

#if 0 // bug.dxf load
run_before_main {
    startup_script = "^.^obug.dxf\n";
};
#endif

#if 0 // glorbo
run_before_main {
    startup_script = "^.^oglorbo.dxf\n"
        // "^Oglorbo.stl\n"
        ;
};
#endif

#if 0 // Box tweening
run_before_main {
    startup_script = "bz5";
};
#endif

#if 0 // UP DOWN polygon
run_before_main {
    startup_script = "pz";
};
#endif

#if 0 // hotkeys not working bug
run_before_main {
    startup_script = "cx20\n10\nsay]90\t30";
};
#endif

#if 0 // hotkeys not working bug
run_before_main {
    startup_script = "^odemo.dxf\n";
};
#endif

#if 0 // divide
run_before_main {
    startup_script = 
        "cz10\n"
        "s<m2d 0 -10>\b"
        "lzm<m2d 0 10>"
        // "cz10\n"
        // "s<m2d 0 -10>\b"
        // "lzm<m2d 0 10>"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "UUUUUUUUUU"
        ;
    #if 0
    "bz10\t10\n"
        "lm<m2d 5 0>m<m2d 5 10>"
        "lm<m2d 0 5>m<m2d 10 5>"
        "sa\b"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        #endif
};
#endif

#if 0 // multi-popup
run_before_main {
    startup_script = 
        // "[lz"
        // "[10lz"
        // "f123[456"
        "lz12\t34x56\t78[98\t76"
        // "lzx["
        ;
};
#endif

#if 0 // revolve
run_before_main {
    startup_script = \
                     "y"
                     "cz10\n"
                     "sa[10\n"
                     // "Zx-5\n"
                     "y"
                     "samzx5\n"
                     // "Ax5\nx5\t5\n"
                     "sa]\n"
                     ; };
#endif

#if 0 // fillet
run_before_main {
    startup_script = \
                     "Bz40\t20\nf5<m2d 15 10><m2d 20 5>";
};
#endif

#if 0 // dogear
run_before_main {
    startup_script = \
                     "Bz40\t20\ng5<m2d 15 10><m2d 20 5>";
};
#endif

#if 0 // kitchen sink
run_before_main {
    startup_script = 
        #if 1
        "cz10\n" // circle
        "cz\t10\n" // bigger circle
        "bzx30\t30\n" // box
        "ysadc<m2d 0 0>" 
        "[5\t15\n" // extrude
        "sc<m2d 0 30><esc>qs3" 
        "1<m2d 30 15>0<esc>" 
        "sq1sq3me<m2d 40 40>x15\t15\n" 
        "{3\n" 
        "sc<m2d 0 0>Zm<m2d -50 0>" 
        "sc<m2d 0 0>Am<m2d -50 0><m2d -15 5>" 
        "]360\n"
        "^n" 
        "cx30\t30\n3.4\n" 
        "saXzYzXzsa[1\n" 
        #endif
        "^osplash.dxf\nsc<m2d 24 0><m2d 16 0>"
        #if 1
        "[\t10\n" 
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
        // ";" 
        // "^odemo.dxf\n" 
                       // "^signore.stl\ny\n" 
                       // "^oignore.stl\n" 
                       // ".." 
                       // "pz\t5\n" // (Nathan) Polygon
                       // "cz18\nD<m2d 0 9>D<m2d 0 -9>s<m2d 2 -9><m2d -2 9>\b" // (Henok) DivideNearest
                       // "j2<m2d 1 7><m2d -1 -7>\n" //(Henok) Offset
                       // "^N^ob:wug.drawing\nysa"
        #endif
        ;
};
#endif
#ifdef SHIP
run_before_main {
    startup_script = "";
    glfwSetWindowTitle(glfw_window, "Conversation pre-alpha " __DATE__ " " __TIME__);
};
#endif

#include "manifoldc.h"
#include "header.cpp"
#include "header2.cpp"

#define DUMMY_HOTKEY 9999

// (global) state
WorldState_ChangesToThisMustBeRecorded_state state;
ScreenState_ChangesToThisDo_NOT_NeedToBeRecorded_other other;

// convenient pointers to shorten xxx.foo.bar into foo->bar
Drawing *drawing = &state.drawing;
FeaturePlaneState *feature_plane = &state.feature_plane;
Meshes *meshes = &state.meshes;
PopupState *popup = &state.popup;
ToolboxState *toolbox = &state.toolbox;
TwoClickCommandState *two_click_command = &state.two_click_command;
MeshTwoClickCommandState *mesh_two_click_command = &state.mesh_two_click_command;
Camera *camera_drawing = &other.camera_drawing;
Camera *camera_mesh = &other.camera_mesh;
PreviewState *preview = &other.preview;

// FORNOW
Event event_passed_to_popups;
bool already_processed_event_passed_to_popups;

#include "boolean.cpp"
#include "misc.cpp"
#include "draw2.cpp"
#include "draw.cpp"
#include "save_and_load.cpp"
#include "message.cpp"
#include "popup.cpp"
#include "history.cpp"
#include "callbacks.cpp"
#include "bake.cpp"
#include "cookbook.cpp"
#include "button.cpp"
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
    }


    if (0) {
        messagef(pallete.red, "TODO: Center snap should visualize the entity (same for all of them i think)");
        messagef(pallete.red, "TODO: expand scripting to allow SHIFT+SPACE (just use what vimrc does)");
        messagef(pallete.red, "TODO: measure should populate the active Mesh field; this would be really nice");
        messagef(pallete.red, "TODO: popup->linear_copy_num_additional_copies should default to 1 (ZERO_OUT is problem)");
        messagef(pallete.red, "TODO: make negative extrude and revolves work");
        messagef(pallete.red, "TODO: blue move/rotate/copy (certainly) still needs work");
        // messagef(pallete.red, "TODO: move shouldn't snap to entities being moved");
        // messagef(pallete.blue, "TODO: EXCLUDE_SELECTED_ENTITIES_FROM_SECOND_CLICK_SNAP flag");
        messagef(pallete.red, "TODO: rotate about origin bumps the mouse unnecessarily (or like...wrong?)");
        messagef(pallete.yellow, "TODO: CIRCLE type entity");
        messagef(pallete.yellow, "TODO: - Select Connected");
        messagef(pallete.yellow, "TODO: - TwoClickDivide");
        messagef(pallete.green, "Offset broken clicking here *------* ");
        messagef(pallete.green, "TODO (fun): Offset visualization and tweening");


    }

    // messagef(pallete.red, "?TODO: possible to get into a state where Escape no longer cancels Select, Connected (just popups)");


    // messagef(pallete.red, "TODO: rename pallete -> pallete (allow for pallete swaps later)");
    #ifdef SHIP
    // messagef(pallete.light_gray, "press ? for help");
    #endif
    /*
       messagef(pallete.red, "TODO: Intersection snap");
       messagef(pallete.red, "TODO: SHIP should disable all the commands without without without without without without without without buttons");
       messagef(pallete.red, "TODO: Save/Load need buttons");
       messagef(pallete.red, "TODO: Camera clip planes still jacked (including ortho)");
       messagef(pallete.red, "TODO: Camera hotkeys ;, ' need buttons");
       messagef(pallete.red, "TODO: Rezoom camera needs button");
       messagef(pallete.red, "TODO: Beatiful button presses");
       messagef(pallete.red, "TODO: config needs inches vs. mm");
       messagef(pallete.red, "TODO: config needs bool to hide gui");
       messagef(pallete.red, "TODO: Select/deselect snaps");
       messagef(pallete.red, "TODO: Push power fillet to beta");
       messagef(pallete.red, "TODO: Push power offset (shell) to beta");
       messagef(pallete.red, "TODO: Save/Load DXF broken for some arcs if you load\n      and save the dxf in LAYOUT in the middle.");
       */

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
        glClearColor(pallete.black.x, pallete.black.y, pallete.black.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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

                _for_each_entity_ {
                    vec3 target_color = get_color((entity->is_selected) ? ColorCode::Selection : entity->color_code);
                    if (entity->is_selected) target_color = CLAMPED_LERP(3.0f * entity->time_since_is_selected_changed - 0.1f, AVG(pallete.white, target_color), target_color);
                    JUICEIT_EASYTWEEN(&entity->preview_color, target_color, 3.0f);
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

        { // draw
            conversation_draw();
            _messages_draw();
        }

        if (frame++ == 1) glfwShowWindow(glfw_window);
        if (other.stepping_one_frame_while_paused) {
            other.stepping_one_frame_while_paused = false;
            other.paused = true;
        }

    }
}

#endif
