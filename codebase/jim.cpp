#ifndef JIM_CPP
#define JIM_CPP
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

#ifndef COW_CPP
#define ASSERT(b) do { if (!(b)) { \
    *((volatile int *) 0) = 0; \
} } while (0)
#define _COUNT_OF(fixed_size_array) ((sizeof(fixed_size_array)/sizeof(0[fixed_size_array])) / ((size_t)(!(sizeof(fixed_size_array) % sizeof(0[fixed_size_array])))))
#endif

// msvc details switches /Bt /d2cgsummary 

#define real double
#define for_(i, N) for (int i = 0; i < N; ++i)
#define for_line_loop_(i, j, N) for (int i = N - 1, j = 0; j < N; i = j++)
#define for_line_strip_(i, j, N) for (int i = 0, j = 1; j < N; i = j++)
#define for_sign(sign) for (int sign = -1; sign <= 1; sign += 2)

#define NUM_DENm1(f, F) (double(f) / ((F) - 1))
#define NUM_DEN(f, F) (double(f) / (F))

#define SWAP(a, b) do {                   \
    ASSERT(sizeof(a) == sizeof(b));       \
    void *__SWAP_tmp = malloc(sizeof(a)); \
    memcpy(__SWAP_tmp, &a, sizeof(a));    \
    memcpy(&a, &b, sizeof(b));            \
    memcpy(&b, __SWAP_tmp, sizeof(b));    \
} while (0)

// // http://nothings.org/stb_ds/
// *((int*)(stb)-8)
#define STB_DS_IMPLEMENTATION
#include "ext/stb_ds.h"
#undef arrput
#undef arrlen
#undef arrlenu
template <typename T> void arrput(T *&array, T element) { stbds_arrput(array, element); }
#define arrlen(array) int(stbds_arrlen(array))
template <typename T> T *jim_stb2raw(T *dyn) {
    int size = arrlen(dyn) * sizeof(T);
    T *raw = (T *) malloc(size);
    memcpy(raw, dyn, size);
    arrfree(dyn);
    return raw;
}
template <typename T> T *jim_raw2stb(int n, T *raw) {
    T *dyn = 0;
    for_(i, n) arrput(dyn, raw[i]);
    return dyn;
}

// https://handmade.network/forums/t/1273-post_your_c_c++_macro_tricks/3
#define __defer(line) defer_ ## line
#define _defer(line) __defer(line)
#define defer auto _defer(__LINE__) = defer_dummy() + [&]( )
template <typename F> struct Defer { Defer(F f) : f(f) {} ~Defer() { f(); } F f; }; template <typename F> Defer<F> makeDefer( F f ) { return Defer<F>( f ); }; struct defer_dummy {}; template<typename F> Defer<F> operator+( defer_dummy, F&& f ) { return makeDefer<F>( std::forward<F>(f) ); }


// https://en.cppreference.com/w/c/algorithm/qsort
void jim_sort_against(void *base, int nitems, int size, real *corresp_values_to_sort_against) {
    struct qsortHelperStruct {
        int index;
        real value;
    };
    qsortHelperStruct *helperArray = (qsortHelperStruct *) calloc(sizeof(qsortHelperStruct), nitems); {
        for_(i, nitems) helperArray[i] = { i, corresp_values_to_sort_against[i] };
    }

    int(* comp)(qsortHelperStruct *, qsortHelperStruct *) \
        = [](qsortHelperStruct *a, qsortHelperStruct *b) -> int { return (a->value < b->value) ? -1 : 1; }; 

    qsort(helperArray, nitems, sizeof(qsortHelperStruct), (int (*)(const void *, const void *))comp);

    void *tmp_buffer = malloc(nitems * size); { // fornow
        for_(i, nitems) memcpy(\
                ((char *) tmp_buffer) + (i * size), \
                ((char *) base) + (helperArray[i].index * size), \
                size);
        memcpy(base, tmp_buffer, nitems * size);
    } free(tmp_buffer);
}


// if file has been modified (more than a second since the last modification)
// returns the report of fopen(filename, mode)
// otherwise returns NULL
#include <sys/types.h>
#include <sys/stat.h>
#ifdef COW_OS_WINDOWS
#define stat _stat
#else
#include <unistd.h>
#endif
FILE *jim_hot_fopen(char *filename, char *mode = "r") {
    struct Stamp {
        char filename[128];
        time_t mod_time;
    };

    static int num_stamps;
    static Stamp stamps[64];

    time_t mod_time = 0; {
        struct stat result;
        ASSERT(stat(filename, &result) == 0);
        mod_time = result.st_mtime;
    }

    Stamp *stamp = NULL;
    {
        for (Stamp *finger = stamps; finger < stamps + num_stamps; ++finger) {
            if (strcmp(filename, finger->filename) == 0) {
                stamp = finger;
                break;
            }
        }
    }


    if (!stamp) {
        ASSERT(num_stamps < _COUNT_OF(stamps));
        stamp = &stamps[num_stamps++];
        strcpy(stamp->filename, filename);
        stamp->mod_time = mod_time;
        return fopen(filename, mode);
    }

    if (mod_time > stamp->mod_time) {
        stamp->mod_time = mod_time;
        return fopen(filename, mode);
    } 

    return NULL;
}
void eg_hot_fopen() {
    while (true) {
        FILE *file = jim_hot_fopen("scratch.txt");
        if (file) {
            printf("modified!\n");
            fclose(file);
        }
    }
}


#ifdef COW_OS_WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif


// ohno
#define _UNIQUE_ISH_VARIABLE_NAME CONCAT(_VAR_, __COUNTER__)
#define BEGIN_PRE_MAIN static int _UNIQUE_ISH_VARIABLE_NAME = []() {
#define END_PRE_MAIN return 0; }();
#endif

