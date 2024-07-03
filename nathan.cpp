#include "playground.cpp"
#include "easy_mode.cpp"
#include "stew.cpp"
int main() {
    Camera camera = make_OrbitCamera3D(0.5f * 256.0f / TAN(RAD(30.0f)), RAD(60.0f));
    real time = 0.0f;
    while (begin_frame(&camera)) {
        time += 0.0167f;

        gl_begin(OpenGL_from_Pixel);
        gl_primitive(STEW_LINE_LOOP);
        // gl_model_matrix(...);
        gl_color(monokai.red);
        gl_vertex(16.0f, 16.0f);

        gl_size(8.0f + 5.0f * SIN(5 * time));
        gl_color(monokai.green);
        gl_vertex(256.0f, 16.0f);

        gl_size(5.0f);
        gl_color(monokai.blue);
        gl_vertex(256.0f, 256.0f);

        gl_end();
    }
}
