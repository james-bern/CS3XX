// #define COW_PATCH_FRAMERATE
// #define COW_PATCH_FRAMERATE_SLEEP
#define JIM_IS_JIM
#include "include.cpp"

void raspberry() {
    Camera3D camera = { 5.0, RAD(45.0) };
    IndexedTriangleMesh3D mesh = library.meshes.sphere;
    real time = 0.0;
    bool draw_wireframe = true;
    bool draw_vertex_positions = true;

    vec3 *X = (vec3 *) malloc(mesh.num_vertices * sizeof(vec3));
    memcpy(X, mesh.vertex_positions, mesh.num_vertices * sizeof(vec3));

    int _FORNOW_mesh_num_vertices = mesh.num_vertices / 4 + 2;
    real t = 1.0;

    int randomSeed = 0;

    // enum BerryType { Raspberry, Blackberry };

    int berry = 0;
    bool matcap = false;

    while (cow_begin_frame()) {
        camera_move(&camera);
        mat4 P = camera_get_P(&camera);
        mat4 V = camera_get_V(&camera);
        mat4 M = M4_RotationAboutYAxis(0.4 * time);

        gui_printf("The Virtual Vineyard!");

        gui_slider("t", &t, 0.0, 1.0);
        gui_slider("randomSeed", &randomSeed, 0, 64);

        // todo drop down menu with enum
        gui_slider("berry", &berry, 0, 2, 'j', 'k', true);

        gui_checkbox("matcap", &matcap, 'm');
        static bool advanced;
        gui_checkbox("advanced", &advanced, COW_KEY_TAB);
        if (advanced) {
            gui_checkbox("draw_wireframe", &draw_wireframe, 'w');
            gui_checkbox("draw_vertex_positions", &draw_vertex_positions, 'v');
        }




        for_(i, mesh.num_vertices) {
            real c = LERP(t, 0.5, 1.0);
            real a = 0.9 + 0.2 * X[i].y * X[i].y + 0.15 * X[i].y;
            mesh.vertex_positions[i] = cwiseProduct(X[i], c * V3(a, (berry == 0) ? 1.0 : 1.2, a));
        }

            if (draw_wireframe) {
                mesh.draw(P, V, M, monokai.gray);
                eso_begin(P * V * M, SOUP_LINES, 4.0);
                eso_color(monokai.white);
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
                    mesh.draw(P, V, M * M4_Translation(mesh.vertex_positions[i]) * M4_Scaling(0.17 + 0.04 * random_real(0.0, 1.0)), LERP(t, monokai.green, (berry == 0) ? monokai.red : 0.5 * monokai.purple));
                } else {
                    mesh.draw(P, globals.Identity, V * M * M4_Translation(mesh.vertex_positions[i]) * M4_Scaling(0.17 + 0.04 * random_real(0.0, 1.0)), {}, "raspberry.png");
                }
            }
        }

        time += 0.0167;
    }
}

int main() {
    APPS {
        APP(raspberry);
    }
    return 0;
}





