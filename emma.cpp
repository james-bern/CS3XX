#include "playground.cpp"
#include "easy_mode.cpp"
int main() {

    // wow!

    Camera camera_2D = make_Camera2D(256.0f);
    Camera orbit_camera_3D = make_OrbitCamera3D(0.5f * 256.0f / TAN(RAD(30.0f)), RAD(60.0f));
    Camera first_person_camera_3D = make_FirstPersonCamera3D({ 0.0f, 16.0f, 0.5f * 256.0f / TAN(RAD(30.0f)) }, RAD(60.0f));
    Camera *camera = &camera_2D;
    real time = 0.0f;
    while (begin_frame(camera)) {
        time += 0.0167f;

        { // switch cameras
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
        }

        { // draw ground
            eso_begin(camera->get_PV(), SOUP_QUADS);
            real r = 128.0f;
            eso_color(basic.white);
            eso_vertex(-r, 0.0f, -r);
            eso_vertex(-r, 0.0f,  r);
            eso_vertex( r, 0.0f,  r);
            eso_vertex( r, 0.0f, -r);
            eso_end();
        }

        { // draw gate
            eso_begin(camera->get_PV(), SOUP_QUADS);
            eso_size(5.0f);
            eso_color(color_rainbow_swirl(time / 10.0f), 0.5f);
            real r = 64.0f;
            eso_vertex(-r, -r);
            eso_vertex(-r,  r);
            eso_vertex( r,  r);
            eso_vertex( r, -r);
            eso_end();
        }
    }
}
