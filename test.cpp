#include "cs345.cpp"

int main() {

    while (cow_begin_frame()) {
        mat4 NDC_from_Screen = _window_get_NDC_from_Screen();

        vec2 window_size = window_get_size();

        glEnable(GL_SCISSOR_TEST);
        glScissor(0, 0, window_size.x / 2, window_size.y / 2);
        eso_begin(NDC_from_Screen, SOUP_QUADS, 10.0f);
        eso_color(0.0f, 1.0f, 1.0f);
        real32 eps = 16.0f;
        eso_vertex( eps + 0.0f,           eps + 0.0f);
        eso_vertex( eps + 0.0f,          -eps + window_size.y);
        eso_vertex(-eps + window_size.x, -eps + window_size.y);
        eso_vertex(-eps + window_size.x,  eps + 0.0f);
        eso_end();
        glDisable(GL_SCISSOR_TEST);

        eso_begin(NDC_from_Screen, SOUP_POINTS, 10.0f);
        eso_color(1.0f, 0.0f, 1.0f);
        eso_vertex(globals.mouse_position_Screen);
        eso_end();
    }

    return 1;
}
