
// TODO: They need a simulator app
// TODO: Linear actuator with rack

// TODO: tele-operation from cpp

// 60fps

#include "cs345.cpp"
#include "manifoldc.h"
#include "poe.cpp"
#include "conversation.h"
#undef real // ???
#define u32 DO_NOT_USE_u32_USE_uint32_INSTEAD

#define NUM_LINKS 2
real32 L[] = { 50.0f, 50.0f };
real32 u[] = { 0.0f, 0.0f };

void solve_ik(real32 x_target, real32 y_target, real32 *u_0, real32 *u_1) {
    real32 L20 = L[0] * L[0];
    real32 L21 = L[1] * L[1];
    real32 r2 = (x_target * x_target) + (y_target * y_target);
    real32 r = SQRT(r2);
    real32 A = atan2(y_target, x_target);
    real32 B = acos((r2 + L20 - L21) / (2 * L[0] * r));
    *u_0 = A - B;
    *u_1 = acos((r2 - L20 - L21) / (2 * L[0] * L[1]));
}

real32 x_target = 100.0f;
real32 y_target = 0.0f;

#define TRACE_MAX_NUM_POINTS 512
Queue<vec2> trace;
DXF dxf;
List<vec2> target;

uint32 mode;


int main() {
    Camera2D camera_2D = { 128.0f };
    bool toggle_ik = true;
    bool toggle_traj = false;
    bool toggle_show_target = true;
    uint32 i_target = 0;
    dxf_load("jim4.dxf", &dxf);
    {
        DXFLoopAnalysisResult analysis = dxf_loop_analysis_create_FORNOW_QUADRATIC(&dxf);
        ASSERT(analysis.num_loops == 1);
        uint32 num_entities = analysis.num_entities_in_loops[0];
        DXFEntityIndexAndFlipFlag *loop = analysis.loops[0];
        real32 x, y;
        for (uint32 i = 0; i < num_entities; ++i) {
            DXFEntity *entity = &dxf.entities[loop[i].entity_index];
            bool32 flip_flag =  loop[i].flip_flag;
            real32 t = 0.0f;
            real32 dt = 0.8f / entity_length(entity);
            while (t < 1.0f - TINY_VAL) {
                entity_lerp_considering_flip_flag(entity, t, &x, &y, flip_flag);
                list_push_back(&target, { x, y });
                t += dt;
            }
        }
    }
    #if 1
    {
        FILE *file = (FILE *) fopen("ik.txt", "w");
        ASSERT(file);
        real32 u_0;
        real32 u_1;
        fprintf(file, "const int num_frames = %d;\n", target.length);
        fprintf(file, "int target[2 * num_frames] = {");
        for (uint32 i = 0; i < target.length; ++i) {
            solve_ik(target.data[i].x, target.data[i].y, &u_0, &u_1);
            fprintf(file, "%d,", int(round(LINEAR_REMAP(u_0, -PI / 2,  PI / 2, 500.0f, 2500.0f))));
            fprintf(file, "%d,", int(round(LINEAR_REMAP(u_1,  PI / 2, -PI / 2, 500.0f, 2500.0f))));
        }
        fprintf(file, "};\n");
        fclose(file);
    }
    #endif
    while (cow_begin_frame()) {
        camera_move(&camera_2D);
        mat4 PV = camera_get_PV(&camera_2D);


        gui_slider("L_0", &L[0], 0.0f, 100.0f);
        gui_slider("L_1", &L[1], 0.0f, 100.0f);
        gui_slider("theta_0", &u[0], -PI, PI, true);
        gui_slider("theta_1", &u[1], -PI, PI, true);
        gui_checkbox("toggle_ik", &toggle_ik, 'i');
        gui_checkbox("toggle_traj", &toggle_traj, 'j');
        gui_checkbox("toggle_show_target", &toggle_show_target, 'k');

        if (toggle_traj) {
            vec2 s_target = target.data[i_target];
            i_target = (i_target + 1) % target.length;
            x_target = s_target.x;
            y_target = s_target.y;
        } else if ((globals._input_owner == COW_INPUT_OWNER_NONE) && globals.mouse_left_held) {
            vec2 mouse_s = mouse_get_position(PV);
            x_target = mouse_s.x;
            y_target = mouse_s.y;
        }


        // IK
        if (toggle_ik) {
            solve_ik(x_target, y_target, &u[0], &u[1]);
            #if 0 // servo.write(...);
            u[0] = LINEAR_REMAP(round(LINEAR_REMAP(u[0], -PI / 2,  PI / 2, 0.0f, 180.0f)), 0.0, 180.0f, -PI / 2,  PI / 2);
            u[1] = LINEAR_REMAP(round(LINEAR_REMAP(u[1],  PI / 2, -PI / 2, 0.0f, 180.0f)), 0.0, 180.0f,  PI / 2, -PI / 2);
            #endif
            #if 1 // servo.writeMicroseconds(...);
            u[0] = LINEAR_REMAP(int(round(LINEAR_REMAP(u[0], -PI / 2,  PI / 2, 1000.0f, 2000.0f))), 1000.0f, 2000.0f, -PI / 2,  PI / 2);
            u[1] = LINEAR_REMAP(int(round(LINEAR_REMAP(u[1],  PI / 2, -PI / 2, 1000.0f, 2000.0f))), 1000.0f, 2000.0f,  PI / 2, -PI / 2);
            #endif
            #if 1
            u[0] = CLAMP(u[0], -PI / 2, PI / 2);
            u[1] = CLAMP(u[1], -PI / 2, PI / 2);
            #endif
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

        if (toggle_show_target) dxf_debug_draw(&camera_2D, &dxf);

        // eso_begin(PV, SOUP_POINTS, 12.0f);
        // for (uint32 i = 0; i < target.length; ++i) {
        //     eso_color(color_kelly(i));
        //     eso_vertex(target.data[i]);
        // }
        // eso_end();

        eso_begin(PV, SOUP_LINE_STRIP);
        for (uint32 i = 0; i < trace.length; ++i) {
            eso_color(color_plasma(1.0f - NUM_DENm1(i, TRACE_MAX_NUM_POINTS)));
            eso_vertex(trace.data[i]);
        }
        eso_end();

        eso_begin(PV, SOUP_LINES, 48.0f);
        eso_color(monokai.gray);
        eso_vertex(x_0, y_0);
        eso_vertex(x_1, y_1);
        eso_vertex(x_1, y_1);
        eso_vertex(x_2, y_2);
        eso_end();

        eso_begin(PV, SOUP_POINTS, 24.0f);
        eso_color(monokai.yellow);
        eso_vertex(x_target, y_target);
        eso_end();
    }
}
