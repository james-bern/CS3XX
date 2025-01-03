#include "playground.cpp"
#include "easy_mode.cpp"
#include "stew.cpp"
int main() {
    printf("%u", GL_POINTS);
    printf("%u", GL_LINES);
    printf("%u", GL_TRIANGLES);
    Camera camera = make_OrbitCamera3D(0.5f * 256.0f / TAN(RAD(30.0f)), RAD(60.0f));
    real time = 0.0f;
    while (begin_frame(&camera)) {
        time += 0.0167f;

        gl_begin();
        gl_primitive(GL_TRIANGLES);
        gl_PV(OpenGL_from_Pixel);
        for_(i, 4) {
            printf("%f ", OpenGL_from_Pixel(0, i));}
        printf("\n");
        for_(i, 4) {
            printf("%f ", OpenGL_from_Pixel(1, i));}
        printf("\n");
        for_(i, 4) {
            printf("%f ", OpenGL_from_Pixel(2, i));}
        printf("\n");
        for_(i, 4) {
            printf("%f ", OpenGL_from_Pixel(3, i));}
        //printf("\n\n\n\n");
        // gl_model_matrix(...);
        gl_color(monokai.red);
        gl_size(5.0f);
        gl_vertex(16.0f, 16.0f);

        gl_size(8.0f + 5.0f * SIN(5 * time));
        gl_color(monokai.green);
        gl_vertex(256.0f, 16.0f);

        gl_size(5.0f);
        gl_color(monokai.blue);
        gl_vertex(0.0f, 256.0f);
        gl_primitive(GL_POINTS);
        gl_vertex(500.0f, 100.0f);
        gl_vertex(400.0f, 0.0f);
        gl_vertex(400.0f, 100.0f);

        gl_primitive(GL_LINES);
        gl_vertex(500.0f, 500.0f);
        gl_vertex(500.0f, 300.0f);
        
        gl_stipple(true);
        gl_vertex(500.0f, 400.0f);
        gl_vertex(400.0f, 400.0f);
        gl_end();


        eso_begin(OpenGL_from_Pixel, GL_LINES);

        eso_size(5.0f);
        eso_stipple(true);
        eso_vertex(500.0f, 100.0f);
        eso_vertex(400.0f, 0.0f);
        eso_end();
    }
}

#if 0
                #if 0
                eso_begin(PV_2D, SOUP_LINES); {
                    if (draw_annotation_line) {
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(mouse);
                        eso_vertex(*first_click);
                    }

                    _for_each_entity_ {
                        if (entity->is_selected && (rotating || moving)) continue;
                        ColorCode color_code = (!entity->is_selected) ? entity->color_code : ColorCode::Selection;
                        eso_color(get_color(color_code));
                        eso_entity__SOUP_LINES(entity);
                    }

                    if (draw_annotation_line) {
                        mat4 M; {
                            if (moving || lcopying) {
                                M = M4_Translation(click_vector);
                            } else { ASSERT(rotating);
                                M = M4_Translation(*first_click) * M4_RotationAboutZAxis(click_theta) * M4_Translation(-*first_click);
                            }
                        }
                        eso_model_matrix(M);
                        eso_color(get_color(ColorCode::Emphasis));
                        _for_each_selected_entity_ eso_entity__SOUP_LINES(entity);
                    }
                } eso_end();


                // current situation: x y z red green blue alpha size

                // full model matrix per vertex +16 floats      |
                // reduced model matrix per vertex +12 floats   |
                // quat and a translation per vertex +7 floats  v

                // entity type per vertex +1 uint (very complicated mega shader program could be fun could be fun)

                // model matrix index per vertex +1 int (+16 uniform mat4s)

                // p
                // .


                // ll
                // --

                // ttt
                // .:.

                // pxxllxttt
                // .  -- .:.
                #endif
#endif
