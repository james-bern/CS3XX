#define _CRT_SECURE_NO_WARNINGS
#define COW_USE_REAL_32
#define COW_NO_SOUND

#include <iostream>
#include <fenv.h>
#include <stdlib.h>
#include <stdarg.h>
#include <utility>
#include <cstdio>
#include <cstring>
#include <cmath>

#include "codebase/snail.cpp"
#include "codebase/cow.cpp"
#include "codebase/jim.cpp"

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

BEGIN_PRE_MAIN {
    _cow_init();
    _cow_reset();
    config.hotkeys_app_quit = 0;
    config.hotkeys_app_menu = 0;
    config.hotkeys_gui_hide = 0;
    config.hotkeys_app_next = 0;
    config.hotkeys_app_prev = 0;
    config.tweaks_size_in_pixels_soup_draw_defaults_to_if_you_pass_0_for_size_in_pixels = 3.0f;
    // config.tweaks_scale_factor_for_everything_involving_pixels_ie_gui_text_soup_NOTE_this_will_init_to_2_on_macbook_retina /= 2;
} END_PRE_MAIN 
