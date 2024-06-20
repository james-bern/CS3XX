#include "playground.cpp"
#include "easy_mode.cpp"
int main() {
    real t = 0.0f;
    while (begin_frame(NULL)) {
        t += 0.033f;
        eso_begin(OpenGL_from_Pixel, SOUP_LINE_STRIP); {
            eso_size(15.0f + 10.0f * sin(t));
            eso_stipple(0b00000000);
            eso_color(basic.red);
            eso_vertex(50.0f, 50.0f);
            eso_color(basic.green);
            eso_size(15.0f + 10.0f * cos(t));
            eso_vertex(300.0f, 50.0f);

            eso_color(basic.blue);
            eso_stipple(0b11000000);
            eso_size(9.0f);
            eso_vertex(300.0f, 300.0f);
            eso_size(2.0f);
            eso_vertex(50.0f, 300.0f);
        } eso_end();
    }
}

/*
   johnny mnemonic
   keanu reeves, ice t, ...

   movie setting

   what is a computer
   how fast is your computer
   so fast
   but what 

   memes
   */
