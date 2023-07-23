// #define COW_PATCH_FRAMERATE
// #define COW_PATCH_FRAMERATE_SLEEP
typedef double real;
#define GL_REAL GL_DOUBLE
#define JIM_IS_JIM
#include "include.cpp"

struct FPSCamera {
    vec3 origin;
    double theta;
    double phi;
    double _angle_of_view;
    real y_velocity;
};

mat4 fps_camera_get_C(FPSCamera *human) {
    mat4 C = globals.Identity;

    mat4 trans = M4_Translation(human->origin.x, human->origin.y, human->origin.z);
    mat4 rot_x = M4_RotationAboutXAxis(human->phi);
    mat4 rot_y = M4_RotationAboutYAxis(-1 * human->theta);

    return C * trans * rot_y * rot_x;
}

void fps_camera_move(FPSCamera *human) {

    mat4 C = fps_camera_get_C(human);

    vec3 movement_dir = V3(0, 0, 0);
    real speed = 1;

    vec3 x = transformVector(C, V3(1, 0, 0));
    vec3 z = transformVector(C, V3(0, 0, 1));

    //movement
    {
        if(globals.key_held['w']){
            movement_dir -= z;
        } else  if(globals.key_held['s']){
            movement_dir += z;
        }
        if(globals.key_held['a']){
            movement_dir -= x;
        } else if(globals.key_held['d']){
            movement_dir += x;
        }
        if(globals.key_shift_held){
            speed = 1.75;
        }

        if(human->origin.y > 10){
            human->y_velocity -= 0.035;
        } else if(human->origin.y < 10){
            human->y_velocity = 0;
            human->origin.y = 10;
        } else {
            if(globals.key_pressed[COW_KEY_SPACE]){
                human->y_velocity += 1;
            }
        }
        human->origin.y += human->y_velocity;

    }

    movement_dir.y = 0;
    if(norm(movement_dir) != 0){
        human->origin += speed * normalized(movement_dir);
    }

    if (window_is_pointer_locked()) {
        human->theta += globals.mouse_change_in_position_NDC.x;
        human->phi = CLAMP(human->phi + globals.mouse_change_in_position_NDC.y, -PI / 2, PI / 2);

    }
}


void raspberry() {
    IndexedTriangleMesh3D mesh = library.meshes.sphere;
    real time = 0.0;
    bool draw_wireframe = false;
    bool draw_vertex_positions = true;

    vec3 *X = (vec3 *) malloc(mesh.num_vertices * sizeof(vec3));
    mesh.vertex_positions = (vec3 *) malloc(mesh.num_vertices * sizeof(vec3));
    memcpy(X,                     library.meshes.sphere.vertex_positions, mesh.num_vertices * sizeof(vec3));
    memcpy(mesh.vertex_positions, library.meshes.sphere.vertex_positions, mesh.num_vertices * sizeof(vec3));

    int _FORNOW_mesh_num_vertices = mesh.num_vertices / 4 + 2;
    real t = 1.0;

    int randomSeed = 0;

    // enum BerryType { Raspberry, Blackberry };

    int berry = 0;
    bool matcap = false;
    bool playing = false;
    bool draw_arm = false;
    bool advanced = false;

    Camera3D camera = { 5.0, RAD(45.0) };
    while (cow_begin_frame()) {
        camera_move(&camera);
        mat4 P = camera_get_P(&camera);
        mat4 V = camera_get_V(&camera);
        mat4 R = M4_RotationAboutYAxis(0.4 * time);

        gui_printf("The Virtual Vineyard!");

        gui_slider("t", &t, 0.0, 1.0);
        gui_slider("randomSeed", &randomSeed, 0, 64);

        // todo drop down menu with enum
        gui_slider("berry", &berry, 0, 2, 'j', 'k', true);

        gui_checkbox("playing", &playing, 'p');
        gui_checkbox("matcap", &matcap, 'm');
        gui_checkbox("draw_arm", &draw_arm, 'a');
        gui_checkbox("advanced", &advanced, COW_KEY_TAB);

        if (advanced) {
            gui_checkbox("draw_wireframe", &draw_wireframe, 'w');
            gui_checkbox("draw_vertex_positions", &draw_vertex_positions, 'v');
        }

        // TODO: skybox
        // TODO: fps camera (use functions with same names)

        if (draw_arm) {
            Camera3D arm_camera = { 5.0, RAD(45.0) };
            mat4 P2 = camera_get_P(&arm_camera);
            mat4 V2 = camera_get_V(&arm_camera);
            mat4 M = M4_Translation(2.0, -2.0, 0.0) * M4_RotationAboutXAxis(-RAD(60));
            library.meshes.sphere.draw(P2, V2, M * M4_Scaling(0.55), monokai.blue);
            library.meshes.cylinder.draw(P2, V2, M * M4_Scaling(0.5, 2.0, 0.5), monokai.gray);
            library.meshes.box.draw(    P2, V2, M * M4_Translation(0.5, 2.5, 0.0) * M4_Scaling(0.1, 0.2, 0.2), monokai.orange);
            library.meshes.box.draw(    P2, V2, M * M4_Translation(-0.5, 2.5, 0.0) * M4_Scaling(0.1, 0.2, 0.2), monokai.orange);
        }


        for_(i, mesh.num_vertices) {
            real c = LERP(t, 0.5, 1.0);
            real a = 0.9 + 0.2 * X[i].y * X[i].y + 0.15 * X[i].y;
            mesh.vertex_positions[i] = cwiseProduct(X[i], c * V3(a, (berry == 0) ? 1.0 : 1.2, a));
        }

        if (draw_wireframe) {
            mesh.draw(P, V, R, monokai.gray);
            eso_begin(P * V * R, SOUP_LINES, 4.0); eso_color(monokai.white);
            for (int i = 0; i < mesh.num_triangles; ++i) {
                for (int j = 0; j < 3; ++j) {
                    eso_vertex(mesh.vertex_positions[mesh.triangle_indices[i][j]]);
                    eso_vertex(mesh.vertex_positions[mesh.triangle_indices[i][(j + 1) % 3]]);
                }
            }
            eso_end();
        }

        srand(randomSeed);
        if (draw_vertex_positions) {
            for_(i, _FORNOW_mesh_num_vertices) {
                if (X[i].y > 0.77) { continue; }
                if (!matcap) {
                    mesh.draw(P, V, R * M4_Translation(mesh.vertex_positions[i]) * M4_Scaling(0.17 + 0.04 * random_real(0.0, 1.0)), LERP(t, monokai.green, (berry == 0) ? monokai.red : 0.5 * monokai.purple));
                } else {
                    mesh.draw(P, globals.Identity, V * R * M4_Translation(mesh.vertex_positions[i]) * M4_Scaling(0.17 + 0.04 * random_real(0.0, 1.0)), {}, "raspberry.png");
                }
            }
        }

        if (playing) {
            time += 0.0167;
        }
    }
}

int main() {
    eg_hot_fopen();
    APPS {
        // APP(raspberry);
        APP(eg_hot_fopen);
    }
    return 0;
}





