// [why are you going?]
//
// we're going [to the defense] to support charlie, i think
// pretty much nobody knows anything about the stuff he's doing except he himself


// TODO: They need a simulator app
// TODO: Linear actuator with rack

// TODO: tele-operation from cpp

// 60fps

#include "cs345.cpp"
#include "manifoldc.h"
#include "poe.cpp"
#undef real // ???
#define u32 DO_NOT_USE_u32_USE_uint32_INSTEAD

#define NUM_LINKS 2
real32 L[] = { 100.0f, 50.0f };
real32 u[] = { 0.0f, 0.0f };

real32 x_target = 100.0f;
real32 y_target = 0.0f;

#define TRACE_MAX_NUM_POINTS 100024
Queue<vec2> trace;

uint32 mode;

int main() {
    Camera2D camera_2D = { 256.0f };
    bool ik = false;
    while (cow_begin_frame()) {
        mat4 PV = camera_get_PV(&camera_2D);
        camera_move(&camera_2D);


        gui_slider("L_0", &L[0], 0.0f, 100.0f);
        gui_slider("L_1", &L[1], 0.0f, 100.0f);
        gui_slider("theta_0", &u[0], -PI, PI, true);
        gui_slider("theta_1", &u[1], -PI, PI, true);
        gui_checkbox("ik", &ik, 'i');

        if ((globals._input_owner == COW_INPUT_OWNER_NONE) && globals.mouse_left_held) {
            vec2 mouse_s = mouse_get_position(PV);
            x_target = mouse_s.x;
            y_target = mouse_s.y;
        }

            static real32 time;
        // IK
        if (ik) {
            real32 L20 = L[0] * L[0];
            real32 L21 = L[1] * L[1];
            real32 r2 = (x_target * x_target) + (y_target * y_target);
            real32 r = SQRT(r2);
            real32 A = atan2(y_target, x_target);
            real32 B = acos((r2 + L20 - L21) / (2 * L[0] * r));
            u[0] = A - B;
            u[1] = acos((r2 - L20 - L21) / (2 * L[0] * L[1]));
        } else {
            u[0] = 2 * time;
            u[1] = (PI + sin(time)) * time;
            // u[0] = sin(1.33 * time) * RAD(90);
            // u[1] = sin(4.7 * time) * RAD(90);
            time += 0.01;
        }

        // FK
        real32 x_0 = 0.0f;
        real32 y_0 = 0.0f;
        real32 x_1 = x_0 + L[0] * COS(u[0]);
        real32 y_1 = y_0 + L[0] * SIN(u[0]);
        real32 x_2 = x_1 + L[1] * COS(u[0] + u[1]);
        real32 y_2 = y_1 + L[1] * SIN(u[0] + u[1]);

        queue_enqueue(&trace, { x_2, y_2 });
        if (trace.length > TRACE_MAX_NUM_POINTS) queue_dequeue(&trace);

        eso_begin(PV, SOUP_LINE_STRIP);
        eso_color(monokai.white);
        for (uint32 i = 0; i < trace.length; ++i) {
            eso_color(color_plasma(0.5 + 0.5 * sin(i / 10. - 5.0 *
                            time)));
            eso_vertex(trace.array[i]);
        }
        eso_end();

        eso_begin(PV, SOUP_LINES, 12.0f);
        eso_color(color_kelly(0));
        eso_vertex(x_0, y_0);
        eso_vertex(x_1, y_1);
        eso_color(color_kelly(1));
        eso_vertex(x_1, y_1);
        eso_vertex(x_2, y_2);
        eso_end();

        eso_begin(PV, SOUP_POINTS, 12.0f);
        eso_color(monokai.green);
        eso_vertex(x_target, y_target);
        eso_end();
    }
}
