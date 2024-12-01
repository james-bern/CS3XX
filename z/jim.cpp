#include "playground.cpp"
#include "easy_mode.cpp"
int main() {
    real t = 0.0f;
    while (begin_frame(NULL)) {
        t += 0.0167;
        eso_begin(OpenGL_from_Pixel, SOUP_LINE_STRIP); {
            eso_stipple(true);
            eso_color(basic.red);
            eso_size(15.0f + 10.0f * SIN(t));
            eso_vertex(50.0f, 50.0f);
            eso_color(basic.green);
            eso_size(15.0f + 10.0f * COS(t));
            eso_vertex(300.0f, 50.0f);
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
