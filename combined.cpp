
// BETA
// TODO: first and second clicks of FGHI should be hinted (even first!)
// TODO: Pressing enter should spawn a ghost of the pink or blue crosshairs
// TODO: the white box should fade in
// TODO: second click of fillets needs to be previewed in blue; some sort of cool bezier curve interpolation could be nice (or even just lerp between the central arcs?--polygon will be good practice (this could be fun for Nate)
// TODO: cool flash and then fade out with beep (don't actually need beep) for enter with crosshairs
// TODO: use UP and DOWN arrow keys to increase or decrease the field by one if it's a pure number
// TODO: preview fields if mouse not moving
// -- or actually, do both, but have them fade or something if the mouse is/isn't moving (MouseDrawColor, EnterDrawColor)
// -- or actually actually, time_since_current_popup_typed_in (and maybe also a boolean on mouse movement if they change their mind)
// TODO: fillet preview

// TODO: BETA
// - divide shows the point that was divided (animation)

// TODO (Jim): slow mo keybind

// TODO (Jim): cookbook_delete should throw a warning if you're trying to delete the same entity twice (run a O(n) pass on the sorted list)
// TODO (Jim): fillet
// TODO (Jim): power fillet
// TODO (Jim): revolve++ (with same cool animation for partial revolves
// TODO (Jim): dog ear
// TODO (Jim): power dog ear


// TODO (Jim): upgrade test bed
// TODO (Jim): switch everything from radians to turns

// XXXX: stippled lines
// TODO: reset everything in eso on begin

// XXXX: basic 3D grid with lines (have grids on by default)
// ////: camera reset needs to take into account divider position
// XXXX: switch entity_*(...) over to vec2(...) instead of void(..., real *, real *)
// XXXX: consider cookbook_lambdas -> Cookbook cookbook = cookbook(...)
// TODO: (re)write soup.cpp
// TODOFIRST: eso_size(...) (eso_begin doesn't take size)
// TODO: memcmp to see if should record
// TODO: timer to see if should snapshot

#if 0
#define Color0 Green
#define Color1 Red
#define Color2 Pink
#define Color3 Magenta
#define Color4 Purple
#define Color5 Blue
#define Color6 Gray
#define Color7 LightGray
#define Color8 Cyan
#define Color9 Orange
#else
#define Color0 LightGray
#define Color1 Red
#define Color2 Orange
#define Color3 Yellow
#define Color4 Green
#define Color5 Blue
#define Color6 Purple
#define Color7 Brown
#define Color8 DarkGray
#define Color9 XXX
#endif

// <!> Begin playground.cpp <!> 
// <!> Begin basics.cpp <!> 
#define _CRT_SECURE_NO_WARNINGS
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <utility>
#include <time.h>
#include <chrono>

// // types
// basic types
typedef uint32_t uint;
typedef float real;
typedef uint8_t u8;
typedef uint64_t u64;
// vecD
template <uint D> struct Vector;
typedef Vector<2> vec2;
typedef Vector<3> vec3;
typedef Vector<4> vec4;
#define vecD Vector<D>
#define tuD  template <uint D>
#define tuDv tuD vecD
// matD
template <uint D> struct Matrix;
typedef Matrix<2> mat2;
typedef Matrix<3> mat3;
typedef Matrix<4> mat4;
#define matD Matrix<D>
#define tuDm tuD matD

// // standard macros
// ASSERT
#define ASSERT(b) do { if (!(b)) { \
    printf("ASSERT("); \
    printf(STR(b)); \
    printf("); <- "); \
    printf("Line %d in %s\n", __LINE__, __FILE__); \
    *((volatile int *) 0) = 0; \
} } while (0)
// STATIC_ASSERT
#define STATIC_ASSERT(cond) static_assert(cond, "STATIC_ASSERT");
// FORNOW_UNUSED
// - (suppresses compiler warning about unused variable)
#define FORNOW_UNUSED(expr) do { (void)(expr); } while (0)
#define _IS_INDEXABLE(arg) (sizeof(arg[0]))
#define _IS_ARRAY(arg) (_IS_INDEXABLE(arg) && (((void *) &arg) == ((void *) arg)))
#define ARRAY_LENGTH(arr) (_IS_ARRAY(arr) ? (sizeof(arr) / sizeof(arr[0])) : 0)

// // unorthodox macros
// for_
// - makes general-purpose for loops more readable (to me)
#define for_(i, N) for (uint i = 0; i < N; ++i)
// do_once
// - code inside do_once { ... } will run the first time it's hit, then never again
#define STR(foo) #foo
#define XSTR(foo) STR(foo)
#define CONCAT_(a, b) a ## b
#define CONCAT(a, b) CONCAT_(a, b)
#define do_once static bool CONCAT(_do_once_, __LINE__) = false; bool CONCAT(_prev_do_once_, __LINE__) = CONCAT(_do_once_, __LINE__); CONCAT(_do_once_, __LINE__) = true; if (!CONCAT(_prev_do_once_, __LINE__) && CONCAT(_do_once_, __LINE__))
// defer
// - code inside defer { ... }; will run when we leave the defer's enclosing scope
// - https://handmade.network/forums/t/1273-post_your_c_c++_macro_tricks/3
template <typename F> struct Defer {
    F f;
    Defer(F _f) : f(_f) { }
    ~Defer() { f(); }
};
template <typename F> Defer<F> makeDefer(F f) { return Defer<F>(f); };
struct defer_dummy {};
template <typename F> Defer<F> operator + (defer_dummy, F &&f) { return makeDefer<F>(std::forward<F>(f)); }
#define defer auto CONCAT(defer_, __COUNTER__) = defer_dummy() + [&]()
// run_before_main
// - code inside run_before_main { ... }; runs before main
struct run_before_main_dummy {};
template <typename F> bool operator + (run_before_main_dummy, F &&f) { f(); return true; }
#define run_before_main static bool CONCAT(run_before_main_, __COUNTER__) = run_before_main_dummy() + []()

// // math
// constants
#define TINY_VAL real(1e-5)
#undef HUGE_VAL
#define HUGE_VAL real(1e6)
#define PI real(3.14159265359)
#define TAU (2 * PI)
// conversions
real RAD(real degrees) { return (PI / 180 * (degrees)); }
real DEG(real radians) { return (180 / PI * (radians)); }
real INCHES(real mm) { return ((mm) / real(25.4)); }
real MM(real inches) { return ((inches) * real(25.4)); }
// trig
#define SIN sinf
#define COS cosf
#define TAN tanf
#define ATAN2 atan2f
// POW, SQRT
#define POW powf
#define SQRT sqrtf
#define ROUND roundf
tuDv ROUND(vecD a) { for_(d, D) a[d] = ROUND(a[d]); return a; }

// SGN
int SGN(  int a) { return (a < 0) ? -1 : 1; }
int SGN(float a) { return (a < 0) ? -1 : 1; }
// ABS
int  ABS( int a) { return (a < 0) ? -a : a; }
real ABS(real a) { return (a < 0) ? -a : a; }
// MIN
// int  MIN( int a,  int b) { return (a < b) ? a : b; } // TODO: do we ever use this?
uint MIN(uint a, uint b) { return (a < b) ? a : b; }
real MIN(real a, real b) { return (a < b) ? a : b; }
// MAX
// int  MAX( int a,  int b) { return (a > b) ? a : b; } // TODO: do we ever use this?
uint MAX(uint a, uint b) { return (a > b) ? a : b; }
real MAX(real a, real b) { return (a > b) ? a : b; }
// floating-poiut comparisons
bool IS_ZERO(real a) { return (ABS(a) < TINY_VAL); }
tuD bool IS_ZERO(vecD a) { for_(d, D) if (!IS_ZERO(a[d])) return false; return true; }
bool ARE_EQUAL(real a, real b) { return IS_ZERO(ABS(a - b)); }
tuD bool ARE_EQUAL(vecD a, vecD b) { return IS_ZERO((a - b)); }
bool IS_BETWEEN_LOOSE(real p, real a, real b) { return (((a - TINY_VAL) < p) && (p < (b + TINY_VAL))); }
bool IS_BETWEEN_TIGHT(real p, real a, real b) { return (((a + TINY_VAL) < p) && (p < (b - TINY_VAL))); }
// CLAMP
real CLAMP(real t, real a, real b) { return MIN(MAX(t, a), b); }
real MAG_CLAMP(real t, real a) {
    ASSERT(a > 0.0f);
    return CLAMP(t, -ABS(a), ABS(a));
}
// LERP
real LERP(real t, real a, real b) { return ((1.0f - t) * a) + (t * b); }
tuDv LERP(real t, vecD a, vecD b) { return ((1.0f - t) * a) + (t * b); }
real AVG(real a, real b) { return LERP(0.5f, a, b); }
tuD real AVG(vecD a) { real tmp = 0.0f; for_(d, D) tmp += a[d]; return tmp/ D; }
tuDv AVG(vecD a, vecD b) { return LERP(0.5f, a, b); }
real INVERSE_LERP(real p, real a, real b) { return (p - a) / (b - a); }
real LINEAR_REMAP(real p, real a, real b, real c, real d) { return LERP(INVERSE_LERP(p, a, b), c, d); }
// CLAMPED_LERP
real CLAMPED_LERP(real t, real a, real b) { return LERP(CLAMP(t, 0.0f, 1.0f), a, b); }
tuDv CLAMPED_LERP(real t, vecD a, vecD b) { return LERP(CLAMP(t, 0.0f, 1.0f), a, b); }
real CLAMPED_INVERSE_LERP(real p, real a, real b) { return CLAMP(INVERSE_LERP(p, a, b), 0.0f, 1.0f); }
real CLAMPED_LINEAR_REMAP(real p, real a, real b, real c, real d) { return LERP(CLAMPED_INVERSE_LERP(p, a, b), c, d); }
tuDv CLAMPED_LINEAR_REMAP(real p, real a, real b, vecD c, vecD d) { return LERP(CLAMPED_INVERSE_LERP(p, a, b), c, d); }
// MODULO
// - works for negative N
int MODULO(int x, int N) { return ((x % N) + N) % N; }

// // OS-specific
// detect operating system
#if defined(__APPLE__) || defined(__MACH__)
#define OPERATING_SYSTEM_APPLE
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#define OPERATING_SYSTEM_WINDOWS
#else
#pragma message("ERROR: INVALID OPERATING SYSTEM")
#endif
// DEBUGGER
#ifdef OPERATING_SYSTEM_APPLE
#include <signal.h>
#define DEBUGBREAK() raise(SIGTRAP)
#elif defined(OPERATING_SYSTEM_WINDOWS)
#define DEBUGBREAK() __debugbreak()
#endif
// SLEEP
#ifdef OPERATING_SYSTEM_APPLE
#include <unistd.h>
#define SLEEP(x) usleep((x)*1000)
#elif defined(OPERATING_SYSTEM_WINDOWS)
#define SLEEP Sleep
#endif
// IS_NAN
#ifdef OPERATING_SYSTEM_APPLE
#include <unistd.h>
#define IS_NAN _isnan
#elif defined(OPERATING_SYSTEM_WINDOWS)
#include <windows.h>
#define IS_NAN(x) 
#endif
// SWAP
template <typename T> void SWAP(T *a, T *b) {
    T tmp = *a;
    *a = *b;
    *b = tmp;
}
// MILLIS
long MILLIS() {
    using namespace std::chrono;
    milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return (long) ms.count();
}
// don't buffer printf
run_before_main { setvbuf(stdout, NULL, _IONBF, 0); };
// seed random number generator
run_before_main { srand((unsigned int) time(NULL)); };
// <!> End basics.cpp <!>
#include <cstdint>
#include <cstring>
#include <stdarg.h>
// <!> Begin string.cpp <!> 
struct String {
    char *data;
    uint length;
};

bool string_pointer_is_valid(String string, char *pointer) {
    return (string.data <= pointer) && (pointer < string.data + string.length);
}

#define STRING(cstring_literal) { (char *)(cstring_literal), uint(strlen(cstring_literal)) }

#define _STRING_CALLOC(name, length) String name = { (char *) calloc(1, length) }

String _string_from_cstring(char *cstring) {
    return { (char *)(cstring), uint(strlen(cstring)) };
}

bool string_matches_prefix(String string, String prefix) {
    if (string.length < prefix.length) return false;
    return (memcmp(string.data, prefix.data, prefix.length) == 0);
}

bool string_matches_prefix(String string, char *prefix) {
    return string_matches_prefix(string, STRING(prefix));
}

bool string_matches_suffix(String string, String suffix) {
    if (string.length < suffix.length) return false;
    return (memcmp(&string.data[string.length - suffix.length], &suffix.data[suffix.length - suffix.length], suffix.length) == 0);
}

bool string_matches_suffix(String string, char *prefix) {
    return string_matches_suffix(string, STRING(prefix));
}

bool string_equal(String string1, String string2) {
    if (string1.length != string2.length) return false;

    for_(i, string1.length) {
        if (string1.data[i] != string2.data[i]) return false;
    }
    return true;
}

real strtof(String string) { // FORNOW
    static char cstring[4096];
    memset(cstring, 0, sizeof(cstring));
    ASSERT(string.length < sizeof(cstring));
    memcpy(cstring, string.data, string.length);
    return strtof(cstring, NULL);
}

bool string_read_line_from_file(String *string, uint max_line_length, FILE *file) {
    bool result = fgets(string->data, max_line_length, file);
    if (result) string->length = uint(strlen(string->data));
    return result;
}

FILE *FILE_OPEN(String filename, char *code, bool skip_assert = false) { // FORNOW
    static char cstring[4096];
    memset(cstring, 0, sizeof(cstring));
    ASSERT(filename.length < sizeof(cstring));
    memcpy(cstring, filename.data, filename.length);
    FILE *result = fopen(cstring, code);
    if (!skip_assert) ASSERT(result);
    return result;
}

bool FILE_EXISTS(String filename) {
    FILE *file = FILE_OPEN(filename, "r", true);
    if (!file) {
        return false;
    }
    fclose(file);
    return true;
}

bool FGETS(String *line, uint LINE_MAX_LENGTH, FILE *file) {
    ASSERT(line);
    ASSERT(file);
    bool result = fgets(line->data, LINE_MAX_LENGTH, file);
    line->length = uint(strlen(line->data));
    return result;
}
// <!> End string.cpp <!>
// <!> Begin linalg.cpp <!> 
////////////////////////////////////////////////////////////////////////////////
// vectors and matrices ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

tuD struct Vector {
    // real data[D];
    // real &operator [](uint index) { return data[index]; }
};

template <> struct Vector<2> {
    struct { real x, y; };
    real &operator [](uint index) {
        ASSERT(index < 2);
        if (index == 0) return x;
        return y;
    }
};

template <> struct Vector<3> {
    struct { real x, y, z; };
    real &operator [](uint index) {
        ASSERT(index < 3);
        if (index == 0) return x;
        if (index == 1) return y;
        return z;
    }
};

template <> struct Vector<4> {
    struct { real x, y, z, w; };
    real &operator [](uint index) {
        ASSERT(index < 4);
        if (index == 0) return x;
        if (index == 1) return y;
        if (index == 2) return z;
        return w;
    }
};


tuD struct Matrix {
    real data[D * D];
    real &operator ()(uint row, uint col) {
        ASSERT(row < D);
        ASSERT(col < D);
        return data[D * row + col];
    }
    const real &operator ()(uint row, uint col) const {
        ASSERT(row < D);
        ASSERT(col < D);
        return data[D * row + col];
    }
};



tuD union SnailTupleOfUnsignedInts {
    uint data[D];
    uint &operator [](uint index) { return data[index]; }
};

template <> union SnailTupleOfUnsignedInts<2> {
    uint data[2];
    struct { uint i, j; };
    uint &operator [](uint index) { return data[index]; }
};

template <> union SnailTupleOfUnsignedInts<3> {
    uint data[3];
    struct { uint i, j, k; };
    uint &operator [](uint index) { return data[index]; }
};

typedef SnailTupleOfUnsignedInts<2> uint2;
typedef SnailTupleOfUnsignedInts<3> uint3;
typedef SnailTupleOfUnsignedInts<4> uint4;

// "constructors" //////////////////////////////////////////////////////////////

vec2 V2(real x, real y) { return { x, y }; }
vec3 V3(real x, real y, real z) { return { x, y, z }; }
vec4 V4(real x, real y, real z, real w) { return { x, y, z, w }; }
vec3 V3(vec2 xy, real z) { return { xy.x, xy.y, z }; }
vec4 V4(vec3 xyz, real w) { return { xyz.x, xyz.y, xyz.z, w }; }
vec2 V2(real x) { return { x, x }; }
vec3 V3(real x) { return { x, x, x }; }
vec4 V4(real x) { return { x, x, x, x }; }
vec2 _V2(vec3 xyz) { return { xyz.x, xyz.y }; }
vec3 _V3(vec4 xyzw) { return { xyzw.x, xyzw.y, xyzw.z }; }

mat2 M2(real a0, real a1, real a2, real a3) {
    return { a0, a1, a2, a3 };
}
mat3 M3(real a0, real a1, real a2, real a3, real a4, real a5, real a6, real a7, real a8) {
    return { a0, a1, a2, a3, a4, a5, a6, a7, a8 };
}
mat4 M4(real a0, real a1, real a2, real a3, real a4, real a5, real a6, real a7, real a8, real a9, real a10, real a11, real a12, real a13, real a14, real a15) {
    return { a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15 };
}
mat3 _M3(mat4 M) {
    return { M(0, 0), M(0, 1), M(0, 2), M(1, 0), M(1, 1), M(1, 2), M(2, 0), M(2, 1), M(2, 2) };
}

mat2 hstack(vec2 col0, vec2 col1) { return { col0.x, col1.x, col0.y, col1.y }; }
mat3 hstack(vec3 col0, vec3 col1, vec3 col2) { return { col0.x, col1.x, col2.x, col0.y, col1.y, col2.y, col0.z, col1.z, col2.z }; }
mat4 hstack(vec4 col0, vec4 col1, vec4 col2, vec4 col3) { return { col0.x, col1.x, col2.x, col3.x, col0.y, col1.y, col2.y, col3.y, col0.z, col1.z, col2.z, col3.z, col0.w, col1.w, col2.w, col3.w }; }

// arithmetic operators ////////////////////////////////////////////////////////

// vectors
tuDv  operator +  (vecD A, vecD B) {
    vecD result;
    for_(i, D) {
        result[i] = A[i] + B[i];
    }
    return result;
}
tuDv &operator += (vecD &A, vecD B) {
    A = A + B;
    return A;
}

tuDv  operator -  (vecD A, vecD B) {
    vecD result;
    for_(i, D) {
        result[i] = A[i] - B[i];
    }
    return result;
}
tuDv &operator -= (vecD &A, vecD B) {
    A = A - B;
    return A;
}

tuDv  operator *  (real scalar, vecD A) {
    vecD result;
    for_(i, D) {
        result[i]  = scalar * A[i];
    }
    return result;
}
tuDv  operator *  (vecD A, real scalar) {
    vecD result = scalar * A;
    return result;
}
tuDv &operator *= (vecD &A, real scalar) {
    A = scalar * A;
    return A;
}
tuDv  operator -  (vecD A) {
    return -1 * A;
}

tuDv  operator /  (vecD A, real scalar) {
    vecD result;
    for_(i, D) {
        result[i]  = A[i] / scalar;
    }
    return result;
}
tuDv  operator /  (real scalar, vecD A) {
    vecD result;
    for_(i, D) {
        result[i]  = scalar / A[i];
    }
    return result;
}
tuDv &operator /= (vecD &v, real scalar) {
    v = v / scalar;
    return v;
}

// matrices
tuDm  operator +  (matD A, matD B) {
    matD result = {};
    for_(k, D * D) {
        result.data[k] = A.data[k] + B.data[k];
    }
    return result;
}
tuDm &operator += (matD &A, matD B) {
    A = A + B;
    return A;
}

tuDm  operator -  (matD A, matD B) {
    matD result = {};
    for_(i, D * D) {
        result.data[i] = A.data[i] - B.data[i];
    }
    return result;
}
tuDm &operator -= (matD &A, matD B) {
    A = A + B;
    return A;
}

tuDm  operator *  (matD A, matD B) {
    matD result = {};
    for_(row, D) {
        for_(col, D) {
            for_(i, D) {
                result(row, col) += A(row, i) * B(i, col);
            }
        }
    }
    return result;
}
tuDm &operator *= (matD &A, matD B) {
    A = A * B;
    return A;
}
tuDv  operator *  (matD A, vecD b) { // A b
    vecD result = {};
    for_(row, D) {
        for_(col, D) {
            result[row] += A(row, col) * b[col];
        }
    }
    return result;
}
tuDv  operator *  (vecD b, matD A) { // b^D A
    vecD result = {};
    for_(row, D) {
        for_(col, D) {
            result[row] += A(col, row) * b[col];
        }
    }
    return result;
}
tuDm  operator *  (real scalar, matD M) {
    matD result = {};
    for_(k, D * D) {
        result.data[k] = scalar * M.data[k];
    }
    return result;
}
tuDm  operator *  (matD M, real scalar) {
    return scalar * M;
}
tuDm &operator *= (matD &M, real scalar) {
    M = scalar * M;
    return M;
}
tuDm  operator -  (matD M) {
    return -1 * M;
}

tuDm  operator /  (matD M, real scalar) {
    return (1 / scalar) * M;
}
tuDm &operator /= (matD &M, real scalar) {
    M = M / scalar;
    return M;
}

// important vector functions //////////////////////////////////////////////////

tuD real dot(vecD A, vecD B) {
    real result = 0.0f;
    for_(i, D) {
        result += A[i] * B[i];
    }
    return result;
}
tuDm outer(vecD A, vecD B) {
    matD result = {};
    for_(row, D) {
        for_(col, D) {
            result(row, col) = A[row] * B[col];
        }
    }
    return result;
}

real cross(vec2 A, vec2 B) {
    return A.x * B.y - A.y * B.x;
}
vec3 cross(vec3 A, vec3 B) {
    return { A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x };
}

tuD real squaredNorm(vecD A) {
    return dot(A, A);
}
tuD real norm(vecD A) {
    return sqrt(squaredNorm(A));
}
tuD real sum(vecD A) {
    real result = 0.0;
    for_(i, D) result += A[i];
    return result;
}
tuDv normalized(vecD A) {
    real norm_A = norm(A);
    // ASSERT(fabs(norm_v) > 1e-7);
    return (1 / norm_A) * A;
}
tuD real squaredDistance(vecD A, vecD B) {
    return squaredNorm(A - B);
}
tuD real distance(vecD A, vecD B) {
    return norm(A - B);
}

real ATAN2(vec2); // FORNOW: forward declaration
real angle_from_0_TAU(vec2 A, vec2 B) {
    real result = ATAN2(B - A);
     if (result < 0.0f) result += TAU;
     return result;
}

// ALIASES
// tuD real length(vecD v) { return norm(v); }
// tuD real squared_length(vecD v) { return squaredNorm(v); }

// important matrix functions //////////////////////////////////////////////////

tuDm transpose(matD M) {
    matD result = {};
    for_(row, D) {
        for_(col, D) {
            result(row, col) = M(col, row);
        }
    }
    return result;
}

real determinant(mat2 M) {
    return M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
}
real determinant(mat3 M) {
    return M(0, 0) * (M(1, 1) * M(2, 2) - M(2, 1) * M(1, 2))
        - M(0, 1) * (M(1, 0) * M(2, 2) - M(1, 2) * M(2, 0))
        + M(0, 2) * (M(1, 0) * M(2, 1) - M(1, 1) * M(2, 0));
}
real determinant(mat4 M) {
    real A2323 = M(2, 2) * M(3, 3) - M(2, 3) * M(3, 2);
    real A1323 = M(2, 1) * M(3, 3) - M(2, 3) * M(3, 1);
    real A1223 = M(2, 1) * M(3, 2) - M(2, 2) * M(3, 1);
    real A0323 = M(2, 0) * M(3, 3) - M(2, 3) * M(3, 0);
    real A0223 = M(2, 0) * M(3, 2) - M(2, 2) * M(3, 0);
    real A0123 = M(2, 0) * M(3, 1) - M(2, 1) * M(3, 0);
    return M(0, 0) * ( M(1, 1) * A2323 - M(1, 2) * A1323 + M(1, 3) * A1223 ) 
        - M(0, 1) * ( M(1, 0) * A2323 - M(1, 2) * A0323 + M(1, 3) * A0223 ) 
        + M(0, 2) * ( M(1, 0) * A1323 - M(1, 1) * A0323 + M(1, 3) * A0123 ) 
        - M(0, 3) * ( M(1, 0) * A1223 - M(1, 1) * A0223 + M(1, 2) * A0123 ) ;
}

mat2 inverse(mat2 M) {
    real invdet = 1 / determinant(M);
    return { invdet * M(1, 1), 
        invdet * -M(0, 1), 
        invdet * -M(1, 0), 
        invdet * M(0, 0) };
}
mat3 inverse(mat3 M) {
    real invdet = 1 / determinant(M);
    return { invdet * (M(1, 1) * M(2, 2) - M(2, 1) * M(1, 2)),
        invdet * (M(0, 2) * M(2, 1) - M(0, 1) * M(2, 2)),
        invdet * (M(0, 1) * M(1, 2) - M(0, 2) * M(1, 1)),
        invdet * (M(1, 2) * M(2, 0) - M(1, 0) * M(2, 2)),
        invdet * (M(0, 0) * M(2, 2) - M(0, 2) * M(2, 0)),
        invdet * (M(1, 0) * M(0, 2) - M(0, 0) * M(1, 2)),
        invdet * (M(1, 0) * M(2, 1) - M(2, 0) * M(1, 1)),
        invdet * (M(2, 0) * M(0, 1) - M(0, 0) * M(2, 1)),
        invdet * (M(0, 0) * M(1, 1) - M(1, 0) * M(0, 1)) };
}
mat4 inverse(mat4 M) {
    real invdet = 1 / determinant(M);
    real A2323 = M(2, 2) * M(3, 3) - M(2, 3) * M(3, 2) ;
    real A1323 = M(2, 1) * M(3, 3) - M(2, 3) * M(3, 1) ;
    real A1223 = M(2, 1) * M(3, 2) - M(2, 2) * M(3, 1) ;
    real A0323 = M(2, 0) * M(3, 3) - M(2, 3) * M(3, 0) ;
    real A0223 = M(2, 0) * M(3, 2) - M(2, 2) * M(3, 0) ;
    real A0123 = M(2, 0) * M(3, 1) - M(2, 1) * M(3, 0) ;
    real A2313 = M(1, 2) * M(3, 3) - M(1, 3) * M(3, 2) ;
    real A1313 = M(1, 1) * M(3, 3) - M(1, 3) * M(3, 1) ;
    real A1213 = M(1, 1) * M(3, 2) - M(1, 2) * M(3, 1) ;
    real A2312 = M(1, 2) * M(2, 3) - M(1, 3) * M(2, 2) ;
    real A1312 = M(1, 1) * M(2, 3) - M(1, 3) * M(2, 1) ;
    real A1212 = M(1, 1) * M(2, 2) - M(1, 2) * M(2, 1) ;
    real A0313 = M(1, 0) * M(3, 3) - M(1, 3) * M(3, 0) ;
    real A0213 = M(1, 0) * M(3, 2) - M(1, 2) * M(3, 0) ;
    real A0312 = M(1, 0) * M(2, 3) - M(1, 3) * M(2, 0) ;
    real A0212 = M(1, 0) * M(2, 2) - M(1, 2) * M(2, 0) ;
    real A0113 = M(1, 0) * M(3, 1) - M(1, 1) * M(3, 0) ;
    real A0112 = M(1, 0) * M(2, 1) - M(1, 1) * M(2, 0) ;
    return { invdet * ( M(1, 1) * A2323 - M(1, 2) * A1323 + M(1, 3) * A1223 ),
        invdet * - ( M(0, 1) * A2323 - M(0, 2) * A1323 + M(0, 3) * A1223 ),
        invdet *   ( M(0, 1) * A2313 - M(0, 2) * A1313 + M(0, 3) * A1213 ),
        invdet * - ( M(0, 1) * A2312 - M(0, 2) * A1312 + M(0, 3) * A1212 ),
        invdet * - ( M(1, 0) * A2323 - M(1, 2) * A0323 + M(1, 3) * A0223 ),
        invdet *   ( M(0, 0) * A2323 - M(0, 2) * A0323 + M(0, 3) * A0223 ),
        invdet * - ( M(0, 0) * A2313 - M(0, 2) * A0313 + M(0, 3) * A0213 ),
        invdet *   ( M(0, 0) * A2312 - M(0, 2) * A0312 + M(0, 3) * A0212 ),
        invdet *   ( M(1, 0) * A1323 - M(1, 1) * A0323 + M(1, 3) * A0123 ),
        invdet * - ( M(0, 0) * A1323 - M(0, 1) * A0323 + M(0, 3) * A0123 ),
        invdet *   ( M(0, 0) * A1313 - M(0, 1) * A0313 + M(0, 3) * A0113 ),
        invdet * - ( M(0, 0) * A1312 - M(0, 1) * A0312 + M(0, 3) * A0112 ),
        invdet * - ( M(1, 0) * A1223 - M(1, 1) * A0223 + M(1, 2) * A0123 ),
        invdet *   ( M(0, 0) * A1223 - M(0, 1) * A0223 + M(0, 2) * A0123 ),
        invdet * - ( M(0, 0) * A1213 - M(0, 1) * A0213 + M(0, 2) * A0113 ),
        invdet *   ( M(0, 0) * A1212 - M(0, 1) * A0212 + M(0, 2) * A0112 ) };
}

// using 4x4 transforms ////////////////////////////////////////////////////////

tuDv transformPoint(const mat4 &M, vecD p) {
    vec4 p_hom = {};
    memcpy(&p_hom, &p, D * sizeof(real));
    p_hom.w = 1;
    vec4 ret_hom = M * p_hom;
    ret_hom /= ret_hom.w;
    vecD result = {};
    memcpy(&result, &ret_hom, D * sizeof(real));
    return result;
}
tuDv transformVector(const mat4 &M, vecD v) {
    vec3 v_3D = {};
    memcpy(&v_3D, &v, D * sizeof(real));
    vec3 ret_hom = _M3(M) * v_3D;
    vecD result;
    memcpy(&result, &ret_hom, D * sizeof(real));
    return result;
}
tuDv transformNormal(const mat4 &M, vecD n) {
    vec3 ret_hom = inverse(transpose(_M3(M))) * n;
    vecD result;
    memcpy(&result, &ret_hom, D * sizeof(real));
    return result;
}

// 4x4 transform cookbook //////////////////////////////////////////////////////

tuDm identityMatrix() {
    matD result = {};
    for_(i, D) {
        result(i, i) = 1;
    }
    return result;
}
const mat4 _Identity4x4 = identityMatrix<4>();

mat4 M4_Identity() {
    return _Identity4x4; // FORNOW
}

mat4 M4_Translation(real x, real y, real z = 0) {
    mat4 result = _Identity4x4;
    result(0, 3) = x;
    result(1, 3) = y;
    result(2, 3) = z;
    return result;
}
mat4 M4_Translation(vec2 xy) {
    return M4_Translation(xy.x, xy.y);
}
mat4 M4_Translation(vec3 xyz) {
    return M4_Translation(xyz.x, xyz.y, xyz.z);
}
mat4 M4_Scaling(real x, real y, real z = 1) {
    mat4 result = {};
    result(0, 0) = x;
    result(1, 1) = y;
    result(2, 2) = z;
    result(3, 3) = 1;
    return result;
}
mat4 M4_Scaling(real s) {
    return M4_Scaling(s, s, s);
}
mat4 M4_Scaling(vec2 xy) {
    return M4_Scaling(xy.x, xy.y);
}
mat4 M4_Scaling(vec3 xyz) {
    return M4_Scaling(xyz.x, xyz.y, xyz.z);
}
mat4 M4_RotationAboutXAxis(real t) {
    mat4 result = _Identity4x4;
    result(1, 1) = COS(t); result(1, 2) = -SIN(t);
    result(2, 1) = SIN(t); result(2, 2) =  COS(t);
    return result;
}
mat4 M4_RotationAboutYAxis(real t) {
    mat4 result = _Identity4x4;
    result(0, 0) =  COS(t); result(0, 2) = SIN(t);
    result(2, 0) = -SIN(t); result(2, 2) = COS(t);
    return result;
}
mat4 M4_RotationAboutZAxis(real t) {
    mat4 result = _Identity4x4;
    result(0, 0) = COS(t); result(0, 1) = -SIN(t);
    result(1, 0) = SIN(t); result(1, 1) =  COS(t);
    return result;
}

mat4 M4_RotationAbout(vec3 axis, real angle) {
    real x = axis.x;
    real y = axis.y;
    real z = axis.z;
    real x2 = x * x;
    real y2 = y * y;
    real z2 = z * z;
    real xy = x * y;
    real xz = x * z;
    real yz = y * z;
    real col = COS(angle);
    real s = SIN(angle);
    real d = 1-col;
    return { col+x2*d, xy*d-z*s, xz*d+y*s, 0,
        xy*d+z*s, col+y2*d, yz*d-x*s, 0,
        xz*d-y*s, yz*d+x*s, col+z2*d, 0,
        0, 0, 0, 1
    };
}

mat4 M4_RotationFrom(vec3 a, vec3 b) {
    // NOTE: twist dof is whatever
    // https://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d

    // FORNOW
    a = normalized(a);
    b = normalized(b);

    vec3 v = cross(a, b);
    real col = dot(a, b);
    if (ABS(col + 1.0f) < 1e-5f) return M4_Identity();
    mat3 v_x = { 0.0, -v.z, v.y, v.z, 0.0, -v.x, -v.y, v.x, 0.0 };
    mat3 R = identityMatrix<3>() + v_x + v_x * v_x / (1 + col);
    return {
        R.data[0], R.data[1], R.data[2], 0.0,
            R.data[3], R.data[4], R.data[5], 0.0,
            R.data[6], R.data[7], R.data[8], 0.0,
            0.0,       0.0,       0.0, 1.0 };
}

// optimization stuff //////////////////////////////////////////////////////////

tuDm firstDerivativeofUnitVector(vecD v) {
    vecD tmp = normalized(v);
    return (1 / norm(v)) * (identityMatrix<D>() - outer(tmp, tmp));
}
#define firstDerivativeOfNorm normalized
#define secondDerivativeOfNorm firstDerivativeofUnitVector

tuD real squaredNorm(matD M) {
    real result = 0;
    for(uint i = 0; i < D * D; ++i) {
        result += M[i] * M[i];
    }
    return result;
}

// misc functions //////////////////////////////////////////////////////////////

tuD real minComponent(vecD A) {
    real result = HUGE_VAL;
    for(uint i = 0; i < D; ++i) result = MIN(result, A[i]);
    return result;
}

tuD real maxComponent(vecD A) {
    real result = -HUGE_VAL;
    for(uint i = 0; i < D; ++i) result = MAX(result, A[i]);
    return result;
}

tuDv cwiseAbs(vecD A) {
    for(uint i = 0; i < D; ++i) A[i] = abs(A[i]);
    return A;
}
tuDv cwiseMin(vecD A, vecD B) {
    vecD result = {};
    for(uint i = 0; i < D; ++i) result[i] = (A[i] < B[i]) ? A[i] : B[i];
    return result;
}
tuDv cwiseMax(vecD A, vecD B) {
    vecD result = {};
    for(uint i = 0; i < D; ++i) result[i] = (A[i] > B[i]) ? A[i] : B[i];
    return result;
}
tuDv cwiseProduct(vecD a, vecD b) {
    vecD result = {};
    for(uint i = 0; i < D; ++i) result[i] = a[i] * b[i];
    return result;
}
vec2 e_theta(real theta) {
    return { COS(theta), SIN(theta) };
}
real ATAN2(vec2 a) {
    return ATAN2(a.y, a.x);
}
vec2 rotated(vec2 a, real theta) {
    return { COS(theta) * a.x - SIN(theta) * a.y, SIN(theta) * a.x + COS(theta) * a.y };
}
vec2 rotated_about(vec2 a, vec2 o, real theta) {
    return rotated(a - o, theta) + o;
}
vec2 scaled_about(vec2 a, vec2 o, real scale) {
    return scale * (a - o) + o;
}
mat2 R_theta_2x2(real theta) {
    return { COS(theta), -SIN(theta), SIN(theta), COS(theta) };
}
vec2 perpendicularTo(vec2 v) {
    return { v.y, -v.x };
}

mat4 xyzo2mat4(vec3 x, vec3 y, vec3 z, vec3 o) {
    return {
        x[0], y[0], z[0], o[0],
        x[1], y[1], z[1], o[1],
        x[2], y[2], z[2], o[2],
        0, 0, 0, 1
    };
}
#define M4_xyzo xyzo2mat4
tuDv magClamped(vecD a, real col) {
    real norm_a = norm(a);
    if (ABS(norm_a) < col) { return a; }
    return a / norm_a * MAG_CLAMP(norm_a, col);
}

// utility /////////////////////////////////////////////////////////////////////

tuD void pprint(vecD A) {
    printf("V%d(", D);
    for_(i, D) {
        printf("%lf", A[i]);
        if (i != D - 1) printf(", ");
    }
    printf(")\n");
}
tuD void pprint(matD M) {
    for_(row, D) {
        printf("| ");
        for_(col, D) {
            printf("%lf", M(row, col));
            if (col != D - 1) printf(", ");
        }
        printf(" |\n");
    }
}

// math math ///////////////////////////////////////////////////////////////////

struct RayTriangleIntersectionResult {
    bool hit;
    real distance;
    vec3 pos;
};
RayTriangleIntersectionResult ray_triangle_intersection(vec3 o, vec3 dir, vec3 a, vec3 b, vec3 c) {
    RayTriangleIntersectionResult result = {};
    vec4 w_t = inverse(M4(
                a[0], b[0], c[0], -dir[0],
                a[1], b[1], c[1], -dir[1],
                a[2], b[2], c[2], -dir[2],
                1.0f, 1.0f, 1.0f,     0.0))
        * V4(o, 1.0f);
    result.hit = ((w_t.x > 0) && (w_t.y > 0) && (w_t.z > 0) && (w_t.w > 0));
    result.distance = w_t.w;
    result.pos = o + dir * result.distance;
    return result;
}


// <!> End linalg.cpp <!>
// <!> Begin color.cpp <!> 
struct {
    vec3 red = { 1.0f, 0.0f, 0.0f };
    vec3 green = { 0.0f, 1.0f, 0.0f };
    vec3 blue = { 0.0f, 0.0f, 1.0f };
    vec3 yellow = { 1.0f, 1.0f, 0.0f };
    vec3 cyan = { 0.0f, 1.0f, 1.0f };
    vec3 magenta = { 1.0f, 0.0f, 1.0f };
    vec3 white = { 1.0f, 1.0f, 1.0f };
    vec3 light_gray = { 0.75f, 0.75f, 0.75f };
    vec3 gray = { 0.5f, 0.5f, 0.5f };
    vec3 dark_gray = { 0.25f, 0.25f, 0.25f };
    vec3 black = { 0.0f, 0.0f, 0.0f };
    // TODO: add orange, brown, purple,
} basic;

vec3 RGB255(uint r, uint g, uint b) {
    return V3(real(r), real(g), real(b)) / 255.0f;
}

struct {
    vec3 red        = RGB255(249,  38, 114);
    vec3 orange     = RGB255(253, 151,  31);
    // not actual monokai yellow cause i don't like it
    vec3 yellow     = RGB255(255, 255,  50);
    vec3 green      = RGB255(166, 226,  46);
    vec3 blue       = RGB255(102, 217, 239);
    vec3 purple     = RGB255(174, 129, 255);
    vec3 white      = RGB255(255, 255, 255); // *shrug*
    vec3 light_gray = RGB255(192, 192, 192); // *shrug*
    vec3 gray       = RGB255(127, 127, 127); // *shrug*
    vec3 dark_gray  = RGB255( 64,  64,  64); // *shrug*
    vec3 black      = RGB255(  0,   0,   0); // *shrug*
    vec3 brown      = RGB255(123,  63,   0); // no actual brown
} monokai;

vec3 get_kelly_color(int i) {
    static vec3 _kelly_colors[]={{255.f/255,179.f/255,0.f/255},{128.f/255,62.f/255,117.f/255},{255.f/255,104.f/255,0.f/255},{166.f/255,189.f/255,215.f/255},{193.f/255,0.f/255,32.f/255},{206.f/255,162.f/255,98.f/255},{129.f/255,112.f/255,102.f/255},{0.f/255,125.f/255,52.f/255},{246.f/255,118.f/255,142.f/255},{0.f/255,83.f/255,138.f/255},{255.f/255,122.f/255,92.f/255},{83.f/255,55.f/255,122.f/255},{255.f/255,142.f/255,0.f/255},{179.f/255,40.f/255,81.f/255},{244.f/255,200.f/255,0.f/255},{127.f/255,24.f/255,13.f/255},{147.f/255,170.f/255,0.f/255},{89.f/255,51.f/255,21.f/255},{241.f/255,58.f/255,19.f/255},{35.f/255,44.f/255,22.f/255}};
    return _kelly_colors[MODULO(i, ARRAY_LENGTH(_kelly_colors))];
}

vec3 color_rainbow_swirl(real t) {
    return {
        (0.5f + 0.5f * COS(TAU * ( 0.000f - t))),
        (0.5f + 0.5f * COS(TAU * ( 0.333f - t))),
        (0.5f + 0.5f * COS(TAU * (-0.333f - t)))
    };
}

// <!> End color.cpp <!>
// <!> Begin bbox.cpp <!> 
template <uint D> struct BoundingBox {
    vecD min;
    vecD max;
};

typedef BoundingBox<2> bbox2;
typedef BoundingBox<3> bbox3;
#define bboxD BoundingBox<D>
#define tuDb tuD bboxD

tuDb BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA() {
    BoundingBox<D> result;
    for_(d, D) {
        result.min[d] = HUGE_VAL;
        result.max[d] = -HUGE_VAL;
    }
    return result;
}

tuD bool bbox_contains(bboxD A, vecD point) {
    for_(d, D) {
        if (!IS_BETWEEN_LOOSE(point[d], A.min[d], A.max[d])) return false;
    }
    return true;
}

tuD bool bbox_contains(bboxD A, bboxD other) {
    for_(d, D) {
        if (A.min[d] > other.min[d]) return false;
        if (A.max[d] < other.max[d]) return false;
    }
    return true;
}

tuD bboxD bbox_inflate(bboxD A, real epsilon) {
    for_(d, D) {
        A.min[d] -= epsilon;
        A.max[d] += epsilon;
    }
    return A;
}

tuD bboxD bbox_inflate(bboxD A, vecD epsilon) {
    for_(d, D) {
        A.min[d] -= epsilon[d];
        A.max[d] += epsilon[d];
    }
    return A;
}

// // add
tuDb &operator += (bboxD &A, vecD p) {
    for_(d, D) {
        A.min[d] = MIN(A.min[d], p[d]);
        A.max[d] = MAX(A.max[d], p[d]);
    }
    return A;
}

// // union
// +
tuDb  operator +  (bboxD A, bboxD B) {
    bboxD result;
    for_(d, D) {
        result.min[d] = MIN(A.min[d], B.min[d]);
        result.max[d] = MAX(A.max[d], B.max[d]);
    }
    return result;
}
// +=
tuDb &operator += (bboxD &A, bboxD B) {
    A = A + B;
    return A;
}

void eso_vertex(real x, real y);
void eso_bbox_SOUP_QUADS(bbox2 A) {
    eso_vertex(A.min.x, A.min.y);
    eso_vertex(A.min.x, A.max.y);
    eso_vertex(A.max.x, A.max.y);
    eso_vertex(A.max.x, A.min.y);
}
// <!> End bbox.cpp <!>
// <!> Begin containers.cpp <!>  // TODO: implement better Map
template <typename T> struct List {
    uint length;
    uint _capacity;
    T *array;
    // T &operator [](int index) { return data[index]; }
};

template <typename T> void list_push_back(List<T> *list, T element) {
    if (list->_capacity == 0) {
        ASSERT(!list->array);
        ASSERT(list->length == 0);
        list->_capacity = 16;
        list->array = (T *) malloc(list->_capacity * sizeof(T));
    }
    if (list->length == list->_capacity) {
        list->_capacity *= 2;
        list->array = (T *) realloc(list->array, list->_capacity * sizeof(T));
    }
    list->array[list->length++] = element;
}

template <typename T> void list_free_AND_zero(List<T> *list) {
    if (list->array) free(list->array);
    *list = {};
}

template <typename T> void list_calloc_NOT_reserve(List<T> *list, unsigned int num_slots, unsigned int num_bytes_per_slot) {
    ASSERT(!list->array);
    list_free_AND_zero(list);
    list->_capacity = num_slots;
    list->length = num_slots;
    list->array = (T *) calloc(num_slots, num_bytes_per_slot);
}

template <typename T> void list_memset(List<T> *list, char byte_to_write, unsigned int num_bytes_to_write) {
    ASSERT(byte_to_write == 0); // NOTE: jim only ever calls memset(..., 0, ...); this check is just to catch his silly mistakes
    memset(list->array, byte_to_write, num_bytes_to_write);
}

template <typename T> void list_clone(List<T> *destination, List<T> *source) {
    list_free_AND_zero(destination);
    destination->length = source->length;
    destination->_capacity = source->_capacity;
    if (destination->_capacity != 0) {
        int num_bytes = destination->_capacity * sizeof(T);
        destination->array = (T *) malloc(num_bytes);
        memcpy(destination->array, source->array, num_bytes);
    }
}


template <typename T> void list_insert(List<T> *list, uint i, T element) {
    ASSERT(i <= list->length);
    list_push_back(list, {});
    memmove(&list->array[i + 1], &list->array[i], (list->length - i - 1) * sizeof(T));
    list->array[i] = element;
}

template <typename T> T list_delete_at(List<T> *list, uint i) {
    ASSERT(i >= 0);
    ASSERT(i < list->length);
    T result = list->array[i];
    memmove(&list->array[i], &list->array[i + 1], (list->length - i - 1) * sizeof(T));
    --list->length;
    return result;
}

template <typename T> void list_push_front(List<T> *list, T element) {
    list_insert(list, 0, element);
}


template <typename T> T list_pop_back(List<T> *list) {
    ASSERT(list->length != 0);
    return list_delete_at(list, list->length - 1);
}

template <typename T> T list_pop_front(List<T> *list) {
    ASSERT(list->length != 0);
    return list_delete_at(list, 0);
}

#define Queue List
#define queue_enqueue list_push_back
#define queue_dequeue list_pop_front
#define queue_free_AND_zero list_free_AND_zero
#define Stack List
#define stack_push list_push_back
#define stack_pop list_pop_back
#define stack_free_AND_zero list_free_AND_zero



template <typename Key, typename Value> struct Pair {
    union {
        struct {
            Key key;
            Value value;
        };
        struct {
            Key first;
            Key second;
        };
    };
};

template <typename Key, typename Value> struct Map {
    uint num_buckets;
    List<Pair<Key, Value>> *buckets;
    // T &operator [](int index) { return data[index]; }
};

// http://www.azillionmonkeys.com/qed/hash.html
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
    || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif
#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
        +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif
uint32_t paul_hsieh_SuperFastHash(void *_data, int len) {
    char *data = (char *) _data;

    uint32_t hash = len, tmp;
    int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

template <typename Key, typename Value> void map_put(Map<Key, Value> *map, Key key, Value value) {
    if (!map->buckets) {
        map->num_buckets = 10001; //100003;
        map->buckets = (List<Pair<Key, Value>> *) calloc(map->num_buckets, sizeof(List<Pair<Key, Value>>));
    }
    { // TODO resizing; load factor; ...
    }
    List<Pair<Key, Value>> *bucket = &map->buckets[paul_hsieh_SuperFastHash(&key, sizeof(Key)) % map->num_buckets];
    for (Pair<Key, Value> *pair = bucket->array; pair < &bucket->array[bucket->length]; ++pair) {
        if (memcmp(&pair->key, &key, sizeof(Key)) == 0) {
            pair->value = value;
            return;
        }
    }
    list_push_back(bucket, { key, value });
}

template <typename Key, typename Value> Value *_map_get_pointer(Map<Key, Value> *map, Key key) {
    if (map->num_buckets == 0) return NULL;
    ASSERT(map->buckets);
    List<Pair<Key, Value>> *bucket = &map->buckets[paul_hsieh_SuperFastHash(&key, sizeof(Key)) % map->num_buckets];
    for (Pair<Key, Value> *pair = bucket->array; pair < &bucket->array[bucket->length]; ++pair) {
        if (memcmp(&pair->key, &key, sizeof(Key)) == 0) {
            return &pair->value;
        }
    }
    return NULL;
}

template <typename Key, typename Value> Value map_get(Map<Key, Value> *map, Key key, Value default_value = {}) {
    if (map->num_buckets == 0) return default_value;
    ASSERT(map->buckets);
    List<Pair<Key, Value>> *bucket = &map->buckets[paul_hsieh_SuperFastHash(&key, sizeof(Key)) % map->num_buckets];
    for (Pair<Key, Value> *pair = bucket->array; pair < &bucket->array[bucket->length]; ++pair) {
        if (memcmp(&pair->key, &key, sizeof(Key)) == 0) {
            return pair->value;
        }
    }
    return default_value;
}


template <typename Key, typename Value> void map_free_and_zero(Map<Key, Value> *map) {
    for (List<Pair<Key, Value>> *bucket = map->buckets; bucket < &map->buckets[map->num_buckets]; ++bucket) list_free_AND_zero(bucket);
    if (map->num_buckets) free(map->buckets);
    *map = {};
}

// <!> End containers.cpp <!>
// <!> Begin elephant.cpp <!> 
// TODO: consider different ElephantStack internals

template <typename T> struct ElephantStack {
    Stack<T> _undo_stack;
    Stack<T> _redo_stack;
};

template <typename T> uint elephant_length_undo(ElephantStack<T> *elephant) {
    return elephant->_undo_stack.length;
}

template <typename T> uint elephant_length_redo(ElephantStack<T> *elephant) {
    return elephant->_redo_stack.length;
}

template <typename T> bool elephant_is_empty_undo(ElephantStack<T> *elephant) {
    return (elephant_length_undo(elephant) == 0);
}

template <typename T> bool elephant_is_empty_redo(ElephantStack<T> *elephant) {
    return (elephant_length_redo(elephant) == 0);
}

template <typename T> T *elephant_undo_ptr_begin(ElephantStack<T> *elephant) {
    return elephant->_undo_stack.array;
}

template <typename T> T *elephant_undo_ptr_one_past_end(ElephantStack<T> *elephant) {
    return elephant->_undo_stack.array + elephant->_undo_stack.length;
}

template <typename T> T *elephant_redo_ptr_one_past_end(ElephantStack<T> *elephant) {
    return elephant->_redo_stack.array + elephant->_redo_stack.length;
}

template <typename T> T *elephant_peek_undo(ElephantStack<T> *elephant) {
    ASSERT(elephant->_undo_stack.length);
    return elephant_undo_ptr_one_past_end(elephant) - 1;
}

template <typename T> T *elephant_peek_redo(ElephantStack<T> *elephant) {
    ASSERT(elephant->_redo_stack.length);
    return elephant_redo_ptr_one_past_end(elephant) - 1;
}

template <typename T> T *elephant_pop_undo_onto_redo(ElephantStack<T> *elephant) {
    ASSERT(elephant->_undo_stack.length);
    stack_push(&elephant->_redo_stack, stack_pop(&elephant->_undo_stack));
    return elephant_peek_redo(elephant);
}

template <typename T> T *elephant_pop_redo_onto_undo(ElephantStack<T> *elephant) {
    ASSERT(elephant->_redo_stack.length);
    stack_push(&elephant->_undo_stack, stack_pop(&elephant->_redo_stack));
    return elephant_peek_undo(elephant);
}

template <typename T> void elephant_clear_redo(ElephantStack<T> *elephant) {
    stack_free_AND_zero(&elephant->_redo_stack);
}

template <typename T> void elephant_push_undo_clear_redo(ElephantStack<T> *elephant, T item) {
    stack_push(&elephant->_undo_stack, item);
    elephant_clear_redo(elephant);
}

// <!> End elephant.cpp <!>
// <!> Begin window.cpp <!> 
// TODO: an fps camera shouldn't be allowed to be ortho
// TODO: camera should have clip planes as member variables

#ifdef OPERATING_SYSTEM_APPLE
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GL_COREARB
#include <OpenGL/gl3.h>
#include "glfw3.h"
#elif defined(OPERATING_SYSTEM_WINDOWS)
#include "glad.c"
#include "glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include "glfw3native.h"
#endif

GLFWwindow *glfw_window;
real _window_macbook_retina_fixer__VERY_MYSTERIOUS;

run_before_main {
    ASSERT(glfwInit());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 1);

    glfw_window = glfwCreateWindow(960, 720,  __DATE__ " " __TIME__, NULL, NULL);
    if (!glfw_window) {
        printf("Something's gone wonky; if you weren't just messing with init(...) or something, please try restarting your computer and trying again.\n");
        ASSERT(0);
    }
    glfwMakeContextCurrent(glfw_window);
    glfwSetWindowPos(glfw_window, 0, 100);
    glfwSetWindowAttrib(glfw_window, GLFW_FLOATING, false);
    glfwSetWindowAttrib(glfw_window, GLFW_DECORATED, true);

    #ifdef OPERATING_SYSTEM_WINDOWS
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    #endif

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glfwSwapInterval(1);

    { // _macbook_retina_scale
        int num, den, _;
        glfwGetFramebufferSize(glfw_window, &num, &_);
        glfwGetWindowSize(glfw_window, &den, &_);
        _window_macbook_retina_fixer__VERY_MYSTERIOUS = real(num / den);
    }
};


vec2 window_get_size_Pixel() {
    ASSERT(glfw_window);
    int _width, _height;
    glfwGetFramebufferSize(glfw_window, &_width, &_height);
    real width = real(_width) / _window_macbook_retina_fixer__VERY_MYSTERIOUS;
    real height = real(_height) / _window_macbook_retina_fixer__VERY_MYSTERIOUS;
    return { width, height };
}
real window_get_width_Pixel() { return window_get_size_Pixel().x; }
real window_get_height_Pixel() { return window_get_size_Pixel().y; }


void gl_scissor_Pixel(double x, double y, double dx, double dy) {
    // y_Pixel_upper_left -> y_Scissor_upper_left -> y_Scissor_lower_left
    y = window_get_height_Pixel() - y - dy;
    real factor = _window_macbook_retina_fixer__VERY_MYSTERIOUS;
    glScissor(uint(factor * x), uint(factor * y), uint(factor * dx), uint(factor * dy));
}
void gl_scissor_Pixel(bbox2 bbox) {
    gl_scissor_Pixel(bbox.min.x, bbox.min.y, bbox.max.x - bbox.min.x, bbox.max.y - bbox.min.y);
}
#ifdef glScissor
#undef glScissor
#endif
#define glScissor RETINA_BREAKS_THIS_FUNCTION_USE_gl_scissor_WRAPPER



real window_get_aspect() {
    vec2 size = window_get_size_Pixel();
    return size.x / size.y;
}

mat4 window_get_OpenGL_from_Pixel() {
    // OpenGL                         Pixel 
    // [x'] = [1/r_x      0   0 -1] [x] = [x/r_x - 1]
    // [y'] = [    0 -1/r_y   0  1] [y] = [1 - y/r_y]
    // [z'] = [    0      0   1  0] [z] = [        z] // so invertible (otherwise, would just have row [ 0 0 0 0 ]
    // [1 ] = [    0      0   0  1] [1] = [        1]
    vec2 r = window_get_size_Pixel() / 2;
    mat4 result = {};
    result(0, 0) = 1.0f / r.x;
    result(1, 1) = -1.0f / r.y;
    result(2, 2) = 1.0f;
    result(0, 3) = -1.0f;
    result(1, 3) = 1.0f;
    result(3, 3) = 1.0f;
    return result;
}

mat4 transform_get_P_persp(real angle_of_view, vec2 post_nudge_OpenGL = {}, real near_z_Camera = 0, real far_z_Camera = 0, real aspect = 0) {
    if (IS_ZERO(near_z_Camera)) { near_z_Camera = -0.100f; }
    if (IS_ZERO(far_z_Camera)) { far_z_Camera = -100000.0f; }
    if (IS_ZERO(aspect)) { aspect = window_get_aspect(); }
    ASSERT(near_z_Camera < 0.0f);
    ASSERT(far_z_Camera < 0.0f);

    // consider a point with coordinates (x, y, -z) in the camera's coordinate system
    //                                                                   where z < 0*
    //                              *recall that the camera's z-axis points backwards

    // 1) imagine projecting the point onto some film plane with height r_y and distance D

    //                r_y                               
    //               -|                                 
    //              - |                                 
    //  angle_y    -  |           y <~ vertex           
    //         \  -   |       -   |                     
    //          |-    |   -       |                     
    //          v     +           |                     
    //         -) -   |           |                     
    //        0-------+-----------+----->               
    //                D          -z                     

    // 2) scale film plane by 1 / r_y to yield OpenGL film plane (with height 1) and distance Q_y
    // y' is the projected position of vertex y in OpenGL; i.e., if we can get y', we're done :) 

    //                1 <~ edge of OpenGL film plane
    //               -|                          
    //              - |                          
    //  angle_y    -  |           y              
    //         \  -   |       -   |              
    //          |-    |   -       |              
    //          v     y'          |              
    //         -) -   |           |              
    //        0-------+-----------+----->        
    //              D / r_y      -z              
    //                ^                          
    //                |                          
    //                cot(angle_y) := Q_y        

    // similar triangles has y' / Q_y = y / -z                     
    //                          => y' = -Q_y * (y / z) (Equation 1)

    // we can repeat this procedure in x      
    // the only difference is Q_x vs. Q_y     
    // -------------------------------------- 
    // cot(angle_x) = D / r_x                 
    // cot(angle_y) = D / r_y                 
    // => r_x cot(angle_x) = r_y cot(angle_y) 
    // recall: aspect := r_x / r_y            
    //  => aspect cot(angle_x) = cot(angle_y) 
    //                  => Q_x = Q_y / aspect.

    // encode Equation 1 (and the variant for x) into a homogeneous matrix equation
    // the third row is a   typical clip plane mapping                             

    //  OpenGL                    Camera
    //  [x'] = [Q_x   0  0  0] [x] = [ Q_x * x] ~> [-Q_x * (x / z)]
    //  [y'] = [  0 Q_y  0  0] [y] = [ Q_y * y] ~> [-Q_y * (y / z)]
    //  [z'] = [  0   0  a  b] [z] = [  az + b] ~> [      -a - b/z]
    //  [ 1] = [  0   0 -1  0] [1] = [      -z] ~> [             1]

    real angle_y = 0.5f * angle_of_view;
    real Q_y = 1 / TAN(angle_y);
    real Q_x = Q_y / aspect;

    mat4 result = {};
    result(0, 0) = Q_x;
    result(1, 1) = Q_y;
    result(3, 2) = -1;

    // z'(z) = [-a - b/z]              
    // we want to map [n, f] -> [-1, 1]
    // z'(n) = -a - b/n := -1          
    // z'(f) = -a - b/f :=  1          
    //                                 
    // => a + b/n =  1                 
    //    a + b/f = -1                 
    // => b/n - b/f = 2                
    //                                 
    // => b * (f - n) / (n * f) = 2    
    // => b = (2 * n * f) / (f - n)    
    //                                 
    // => a + (2 * f) / (f - n) = 1    
    // => a = -(n + f) / (f - n)       
    //       = (n + f) / (n - f)       
    result(2, 2) = (near_z_Camera + far_z_Camera) / (near_z_Camera - far_z_Camera);
    result(2, 3) = (2 * near_z_Camera * far_z_Camera) / (far_z_Camera - near_z_Camera);

    // [1 0 0  t_x_OpenGL] [Q_x   0  0  0]
    // [0 1 0  t_y_OpenGL] [  0 Q_y  0  0]
    // [0 0 1           0] [  0   0  a  b]
    // [0 0 0           1] [  0   0 -1  0]
    result(0, 2) = -post_nudge_OpenGL.x;
    result(1, 2) = -post_nudge_OpenGL.y;

    return result;
}

mat4 transform_get_P_ortho(real height_World, vec2 post_nudge_OpenGL = {}, real near_z_Camera = 0, real far_z_Camera = 0, real aspect = 0) {
    // ASSERT(!IS_ZERO(height_World));
    if (ARE_EQUAL(near_z_Camera, far_z_Camera)) {
        near_z_Camera = 1000000.0f;
        far_z_Camera = -near_z_Camera;
    }
    if (IS_ZERO(aspect)) { aspect = window_get_aspect(); }

    // consider a point with coordinates (x, y, z) in the camera's coordinate system

    // 1) imagine projecting the point onto some film plane with height r_y

    // r_y                                  
    // |                                    
    // |                                    
    // +-----------y                        
    // |           |                        
    // |           |                        
    // +-----------------> minus_z direction

    // 2) scale everything by 1 / r_y to yield OpenGL film plane (with height 1)

    // 1                                     
    // |                                     
    // |                                     
    // y'----------y / r_y                   
    // |           |                         
    // |           |                         
    // +-----------------> minus_z  direction

    // => y' = y / r_y

    // OpenGL                        Camera
    // [x'] = [1/r_x      0   0  0] [x] = [ x/r_x]
    // [y'] = [    0  1/r_y   0  0] [y] = [ y/r_y]
    // [z'] = [    0      0   a  b] [z] = [az + b]
    // [1 ] = [    0      0   0  1] [1] = [     1]

    // z'(z) = [az + b]                
    // we want to map [n, f] -> [-1, 1]
    // z'(n) = an + b := -1            
    // z'(f) = af + b :=  1            
    //                                 
    // => a * (f - n) = 2              
    //    a = 2 / (f - n)              
    //                                 
    // (2 * f) / (f - n) + b = 1       
    // => b = (n + f) / (n - f)        

    real r_y = height_World / 2;
    real r_x = window_get_aspect() * r_y;
    real a = 2.0f / (far_z_Camera - near_z_Camera);
    real b = (near_z_Camera + far_z_Camera) / (near_z_Camera - far_z_Camera);

    mat4 result = {};
    result(0, 0) = 1.0f / r_x;
    result(1, 1) = 1.0f / r_y;
    result(2, 2) = a;
    result(2, 3) = b;
    result(3, 3) = 1.0f;

    // [1 0 0  t_x] [1/r_x      0   0  0]
    // [0 1 0  t_y] [    0  1/r_y   0  0]
    // [0 0 1    0] [    0      0   a  b]
    // [0 0 0    1] [    0      0   0  1]

    result(0, 3) = post_nudge_OpenGL.x;
    result(1, 3) = post_nudge_OpenGL.y;

    return result;
}

enum class CameraType {
    None,
    Camera2D,
    OrbitCamera3D,
    FirstPersonCamera3D,
};

struct Camera {
    CameraType type;
    real angle_of_view; // NOTE: 0.0f <=> ortho camera
    vec3 euler_angles;
    union {
        struct {
            vec2 pre_nudge_World;
            union {
                real persp_distance_to_origin_World;
                real ortho_screen_height_World;
            };
        };
        vec3 first_person_position_World;
    };
    vec2 post_nudge_OpenGL;

    mat4 get_P();
    mat4 get_V();
    mat4 get_PV();
};

Camera make_Camera2D(real screen_height_World, vec2 center_World = {}, vec2 post_nudge_OpenGL = {}) {
    Camera result = {};
    result.type = CameraType::Camera2D;
    result.ortho_screen_height_World = screen_height_World;
    result.pre_nudge_World = center_World;
    result.post_nudge_OpenGL = post_nudge_OpenGL;
    return result;
}

Camera make_OrbitCamera3D(real distance_to_origin_World, real angle_of_view = RAD(60.0f), vec3 euler_angles = {}, vec2 pre_nudge_World = {}, vec2 post_nudge_OpenGL = {}) {
    Camera result = {};
    result.type = CameraType::OrbitCamera3D;
    result.angle_of_view = angle_of_view;
    result.persp_distance_to_origin_World = distance_to_origin_World;
    result.euler_angles = euler_angles;
    result.pre_nudge_World = pre_nudge_World;
    result.post_nudge_OpenGL = post_nudge_OpenGL;
    return result;
}

Camera make_FirstPersonCamera3D(vec3 first_person_position_World, real angle_of_view = RAD(60.0f), vec3 euler_angles = {}, vec2 post_nudge_OpenGL = {}) {
    Camera result = {};
    result.type = CameraType::FirstPersonCamera3D;
    result.angle_of_view = angle_of_view;
    result.euler_angles = euler_angles;
    result.first_person_position_World = first_person_position_World;
    result.post_nudge_OpenGL = post_nudge_OpenGL;
    return result;
}

Camera make_EquivalentCamera2D(Camera *orbit_camera_3D) {
    ASSERT(orbit_camera_3D->type == CameraType::OrbitCamera3D);
    bool is_perspective_camera = (!IS_ZERO(orbit_camera_3D->angle_of_view));
    Camera result; {
        result = *orbit_camera_3D;
        result.type = CameraType::Camera2D;
        result.angle_of_view = 0.0f;
        result.euler_angles = {};
        if (is_perspective_camera) result.ortho_screen_height_World = 2.0f * (orbit_camera_3D->persp_distance_to_origin_World * TAN(0.5f * orbit_camera_3D->angle_of_view));
    }
    return result;
}


mat4 Camera::get_P() {
    if (IS_ZERO(this->angle_of_view)) {
        return transform_get_P_ortho(this->ortho_screen_height_World, this->post_nudge_OpenGL);
    } else {
        return transform_get_P_persp(this->angle_of_view, this->post_nudge_OpenGL);
    }
}

mat4 Camera::get_V() {
    mat4 C; {
        mat4 T = M4_Translation(this->first_person_position_World);
        mat4 R_x = M4_RotationAboutXAxis(this->euler_angles.x);
        mat4 R_y = M4_RotationAboutYAxis(this->euler_angles.y);
        mat4 R_z = M4_RotationAboutZAxis(this->euler_angles.z);
        mat4 R = (R_y * R_x * R_z);
        C = (this->type != CameraType::FirstPersonCamera3D) ? (R * T) : (T * R);
    }
    return inverse(C);
}

mat4 Camera::get_PV() { return get_P() * get_V(); }


// <!> End window.cpp <!>
// <!> Begin shader.cpp <!> 
uint shader_compile(char *source, GLenum type) {
    uint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    {
        int success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            { // log
                char infoLog[512];
                glGetShaderInfoLog(shader, 512, NULL, infoLog);
                printf("%s\n", source);
                printf("%s", infoLog);
            }
            ASSERT(0);
        }
    }
    ASSERT(shader);
    return shader;
};

uint shader_build_program(uint vertex_shader, uint geometry_shader, uint fragment_shader) {
    uint shader_program_ID = glCreateProgram();
    ASSERT(shader_program_ID);
    ASSERT(vertex_shader); glAttachShader(shader_program_ID, vertex_shader);
    if (geometry_shader) glAttachShader(shader_program_ID, geometry_shader);
    ASSERT(fragment_shader); glAttachShader(shader_program_ID, fragment_shader);
    glLinkProgram(shader_program_ID);
    {
        int success = 0;
        glGetProgramiv(shader_program_ID, GL_LINK_STATUS, &success);
        if (!success) {
            { // log
                char infoLog[512];
                glGetProgramInfoLog(shader_program_ID, 512, NULL, infoLog);
                printf("%s", infoLog);
            }
            ASSERT(0);
        }
    }
    return shader_program_ID;
};

uint shader_compile_and_build_program(char *vertex_shader_source, char *fragment_shader_source, char *geometry_shader_source = NULL) {
    uint vert = shader_compile(vertex_shader_source, GL_VERTEX_SHADER);
    uint frag = shader_compile(fragment_shader_source, GL_FRAGMENT_SHADER);
    uint geom = geometry_shader_source ? shader_compile(geometry_shader_source, GL_GEOMETRY_SHADER) : 0;
    return shader_build_program(vert, frag, geom);
}

// <!> End shader.cpp <!>
// <!> Begin soup.cpp <!> 
// XXXX: remove option to pass null in soup_draw
// XXXX: remove rounded edges
// NOTE: (soup draw should essentially never be called by the user)
// TODO: per-vertex size
// TODO: per-vertex stipple
// TODO: properly draw meshes ala that one nvidia white paper

////////////////////////////////////////////////////////////////////////////////
// soup ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define SOUP_POINTS         GL_POINTS
#define SOUP_LINES          GL_LINES
#define SOUP_LINE_STRIP     GL_LINE_STRIP
#define SOUP_LINE_LOOP      GL_LINE_LOOP
#define SOUP_TRIANGLES      GL_TRIANGLES
#define SOUP_TRIANGLE_FAN   GL_TRIANGLE_FAN
#define SOUP_TRIANGLE_STRIP GL_TRIANGLE_STRIP
#define SOUP_QUADS          255
#define SOUP_TRI_MESH       254 // TODO

struct {
    char *vert = R""(#version 330 core
        layout (location = 0) in vec3 vertex;
        layout (location = 1) in vec4 color;
        layout (location = 2) in float size;

        out BLOCK {
            vec4 color;
            float size;
        } vs_out;

        uniform mat4 transform;
        uniform bool force_draw_on_top;

        void main() {
            gl_Position = transform * vec4(vertex, 1);
            if (force_draw_on_top) {
                gl_Position.z = -.99 * gl_Position.w; // ?
            }
            vs_out.color = color;
            vs_out.size = size;
        }
    )"";

    char *geom_POINTS = R""(#version 330 core
        layout (points) in;
        layout (triangle_strip, max_vertices = 4) out;
        uniform vec2 OpenGL_from_Pixel_scale;

        in BLOCK {
            vec4 color;
            float size;
        } gs_in[];

        out GS_OUT {
            vec4 color;
            vec2 xy;
        } gs_out;

        void emit(vec4 p, float x, float y) {
            vec2 radius = (gs_in[0].size / 2) * OpenGL_from_Pixel_scale;
            gs_out.color = gs_in[0].color;                                     
            gs_out.xy = vec2(x, y);
            gl_Position = (p + vec4(radius * gs_out.xy, 0, 0)) * gl_in[0].gl_Position.w;
            EmitVertex();                                               
        }

        void main() {    
            vec4 p = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
            emit(p, -1, -1);
            emit(p, 1, -1);
            emit(p, -1, 1);
            emit(p, 1, 1);
            EndPrimitive();
        }  
    )"";

    char *frag_POINTS = R""(#version 330 core
        in GS_OUT {
            vec4 color;
            vec2 xy;
        } fs_in;

        out vec4 frag_color;

        void main() {
            frag_color = fs_in.color;
            if (length(fs_in.xy) > 1) { discard; }
        }
    )"";

    char *geom_LINES = R""(#version 330 core
        layout (lines) in;
        layout (triangle_strip, max_vertices = 4) out;
        uniform vec2 OpenGL_from_Pixel_scale;

        in BLOCK {
            vec4 color;
            float size;
        } gs_in[];

        out BLOCK {
            vec4 color;
            float size;
            vec2 position_Pixel; // NOTE: y flipped sorry
            float angle;
            vec2 starting_point_Pixel;
        } gs_out;

        void main() {    
            vec4 s = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
            vec4 t = gl_in[1].gl_Position / gl_in[1].gl_Position.w;
            vec4 color_s = gs_in[0].color;
            vec4 color_t = gs_in[1].color;

            float angle = atan(OpenGL_from_Pixel_scale.x * (t.y - s.y), OpenGL_from_Pixel_scale.y * (t.x - s.x));

            vec2 perp = OpenGL_from_Pixel_scale * normalize(OpenGL_from_Pixel_scale * vec2(s.y - t.y, t.x - s.x));
            vec4 perp_s = vec4((gs_in[0].size / 2) * perp, 0, 0);
            vec4 perp_t = vec4((gs_in[1].size / 2) * perp, 0, 0);

            gl_Position = (s - perp_s) * gl_in[0].gl_Position.w;
            gs_out.position_Pixel = (vec2(1.0f) + gl_Position.xy) / OpenGL_from_Pixel_scale;
            gs_out.color = color_s;
            gs_out.angle = angle;
            gs_out.starting_point_Pixel = (vec2(1.0f) + s.xy * gl_in[0].gl_Position.w) / OpenGL_from_Pixel_scale;
            EmitVertex();

            gl_Position = (t - perp_t) * gl_in[1].gl_Position.w;
            gs_out.position_Pixel = (vec2(1.0f) + gl_Position.xy) / OpenGL_from_Pixel_scale;
            gs_out.color = color_t;
            gs_out.angle = angle;
            gs_out.starting_point_Pixel = (vec2(1.0f) + s.xy * gl_in[0].gl_Position.w) / OpenGL_from_Pixel_scale;
            EmitVertex();

            gl_Position = (s + perp_s) * gl_in[0].gl_Position.w;
            gs_out.position_Pixel = (vec2(1.0f) + gl_Position.xy) / OpenGL_from_Pixel_scale;
            gs_out.color = color_s;
            gs_out.angle = angle;
            gs_out.starting_point_Pixel = (vec2(1.0f) + s.xy * gl_in[0].gl_Position.w) / OpenGL_from_Pixel_scale;
            EmitVertex();

            gl_Position = (t + perp_t) * gl_in[1].gl_Position.w;
            gs_out.position_Pixel = (vec2(1.0f) + gl_Position.xy) / OpenGL_from_Pixel_scale;
            gs_out.color = color_t;
            gs_out.angle = angle;
            gs_out.starting_point_Pixel = (vec2(1.0f) + s.xy * gl_in[0].gl_Position.w) / OpenGL_from_Pixel_scale;
            EmitVertex();

            EndPrimitive();
        }  
    )"";

    char *frag_LINES = R""(#version 330 core
        uniform bool stipple;

        in BLOCK {
            vec4 color;
            float size;
            vec2 position_Pixel;
            float angle;
            vec2 starting_point_Pixel;
        } fs_in;

        out vec4 frag_color;

        void main() {
            frag_color = fs_in.color;
            if (stipple) {
                vec2 xy = fs_in.position_Pixel;
                // rotate by -angle
                float s = sin(fs_in.angle);
                float c = cos(fs_in.angle);
                mat2 Rinv = mat2(c, -s, s, c);
                vec2 uv = Rinv * (xy - fs_in.starting_point_Pixel);

                if (int(uv.x + 99999) % 10 > 5) discard; // FORNOW
            }
        }
    )"";

    char *frag_TRIANGLES = R""(#version 330 core
        in BLOCK {
            vec4 color;
            float size;
        } fs_in;

        out vec4 frag_color;

        void main() {
            frag_color = fs_in.color;
        }
    )"";

    char *geom_TRI_MESH = R""(#version 330 core
        layout (triangles) in;
        layout (triangle_strip, max_vertices = 3) out;

        uniform vec2 OpenGL_from_Pixel_scale;

        in BLOCK {
            vec4 color;
            float size;
        } gs_in[];

        out GS_OUT {
            vec4 color;
            noperspective vec3 heights;
            noperspective vec3 sizes;
        } gs_out;

        float point_line_dist(vec2 p, vec2 a, vec2 b) {
           vec2 line = b - a;
           vec2 n = vec2(line.y, -line.x);
           vec2 v = p - a;
           return abs(dot(v, n)) / length(n);
        }

        void main() {    
            vec3 sizes = vec3(gs_in[0].size, gs_in[1].size, gs_in[2].size);

            for (int d = 0; d < 3; ++d) {
                gl_Position = gl_in[d].gl_Position / gl_in[d].gl_Position.w;
                gs_out.color = gs_in[d].color;

                int e = (d + 1) % 3;
                int f = (d + 2) % 3;
                vec2 p = gl_in[d].gl_Position.xy / gl_in[d].gl_Position.w / OpenGL_from_Pixel_scale;
                vec2 q = gl_in[e].gl_Position.xy / gl_in[e].gl_Position.w / OpenGL_from_Pixel_scale;
                vec2 r = gl_in[f].gl_Position.xy / gl_in[f].gl_Position.w / OpenGL_from_Pixel_scale;
                vec3 heights = vec3(0);
                heights[d] = point_line_dist(p, q, r);
                vec3 bary = vec3(0);
                bary[d] = 1;

                gs_out.heights = heights;
                gs_out.sizes = sizes;
                EmitVertex();                                               
            }

            EndPrimitive();
        }  
    )"";

    char *frag_TRI_MESH = R""(#version 330 core
        in GS_OUT {
            vec4 color;
            noperspective vec3 heights;
            noperspective vec3 sizes;
        } fs_in;

        out vec4 frag_color;

        // TODO: eso_size should go in here, and you can remove the z-fight-y pass ...
        // NOTE: passing 0 for size should be NO edges
        void main() {
            int i = 0;
            if (fs_in.heights[1] < fs_in.heights[i]) i = 1;
            if (fs_in.heights[2] < fs_in.heights[i]) i = 2;

            if (fs_in.sizes[i] < 0.01) frag_color = fs_in.color;
            else {
                vec3 h = fs_in.heights / fs_in.sizes;
                float height = min(min(h.x, h.y), h.z);
                frag_color = mix(mix(vec4(0,0,0,1), fs_in.color, 0.5), fs_in.color, smoothstep(0.5, 1.0, height));
            }
        }
    )"";


} soup_source;

struct {
    uint shader_program_POINTS;
    uint shader_program_LINES;
    uint shader_program_TRIANGLES;
    uint shader_program_TRI_MESH;
    uint VAO[1];
    uint VBO[16];
    uint EBO[1];
} soup;

run_before_main {
    uint vert = shader_compile(soup_source.vert, GL_VERTEX_SHADER);
    uint geom_POINTS = shader_compile(soup_source.geom_POINTS, GL_GEOMETRY_SHADER);
    uint geom_LINES = shader_compile(soup_source.geom_LINES, GL_GEOMETRY_SHADER);
    uint geom_TRI_MESH = shader_compile(soup_source.geom_TRI_MESH, GL_GEOMETRY_SHADER);
    uint frag_POINTS = shader_compile(soup_source.frag_POINTS, GL_FRAGMENT_SHADER);
    uint frag_LINES = shader_compile(soup_source.frag_LINES, GL_FRAGMENT_SHADER);
    uint frag_TRIANGLES = shader_compile(soup_source.frag_TRIANGLES, GL_FRAGMENT_SHADER);
    uint frag_TRI_MESH = shader_compile(soup_source.frag_TRI_MESH, GL_FRAGMENT_SHADER);
    soup.shader_program_POINTS = shader_build_program(vert, geom_POINTS, frag_POINTS);
    soup.shader_program_LINES = shader_build_program(vert, geom_LINES, frag_LINES);
    soup.shader_program_TRIANGLES = shader_build_program(vert, 0, frag_TRIANGLES);
    soup.shader_program_TRI_MESH = shader_build_program(vert, geom_TRI_MESH, frag_TRI_MESH);
    glGenVertexArrays(ARRAY_LENGTH(soup.VAO), soup.VAO);
    glGenBuffers(ARRAY_LENGTH(soup.VBO), soup.VBO);
    glGenBuffers(ARRAY_LENGTH(soup.EBO), soup.EBO);
};

void soup_draw(
        mat4 transform,
        uint SOUP_primitive,
        uint num_vertices,
        vec3 *vertex_positions,
        vec4 *vertex_colors,
        real *vertex_sizes,
        bool force_draw_on_top,
        bool stipple) {
    if (num_vertices == 0) { return; } // NOTE: num_vertices zero is valid input

    glBindVertexArray(soup.VAO[0]);
    uint attrib_index = 0;
    auto upload_vertex_attribute = [&](void *array, uint count, uint dim) {
        ASSERT(array);
        ASSERT(attrib_index <= ARRAY_LENGTH(soup.VBO));
        glDisableVertexAttribArray(attrib_index); {
            uint buffer_size = count * dim * sizeof(real);
            glBindBuffer(GL_ARRAY_BUFFER, soup.VBO[attrib_index]);
            glBufferData(GL_ARRAY_BUFFER, buffer_size, array, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(attrib_index, dim, GL_FLOAT, GL_FALSE, 0, NULL);
        } glEnableVertexAttribArray(attrib_index);
        ++attrib_index;
    };
    upload_vertex_attribute(vertex_positions, num_vertices, 3);
    upload_vertex_attribute(vertex_colors, num_vertices, 4);
    upload_vertex_attribute(vertex_sizes, num_vertices, 1);

    uint GL_primitive;
    uint shader_program_ID;
    {
        if (SOUP_primitive == SOUP_POINTS) {
            GL_primitive = GL_POINTS;
            shader_program_ID = soup.shader_program_POINTS;
        } else if (SOUP_primitive == SOUP_LINES) {
            GL_primitive = GL_LINES;
            shader_program_ID = soup.shader_program_LINES;
        } else if (SOUP_primitive == SOUP_LINE_STRIP) {
            GL_primitive = GL_LINE_STRIP;
            shader_program_ID = soup.shader_program_LINES;
        } else if (SOUP_primitive == SOUP_LINE_LOOP) {
            GL_primitive = GL_LINE_LOOP;
            shader_program_ID = soup.shader_program_LINES;
        } else if (SOUP_primitive == SOUP_TRIANGLES) {
            GL_primitive = GL_TRIANGLES;
            shader_program_ID = soup.shader_program_TRIANGLES;
        } else if (SOUP_primitive == SOUP_TRIANGLE_FAN) {
            GL_primitive = GL_TRIANGLE_FAN;
            shader_program_ID = soup.shader_program_TRIANGLES;
        } else if (SOUP_primitive == SOUP_TRIANGLE_STRIP) {
            GL_primitive = GL_TRIANGLE_STRIP;
            shader_program_ID = soup.shader_program_TRIANGLES;
        } else if (SOUP_primitive == SOUP_QUADS) {
            GL_primitive = GL_TRIANGLES;
            shader_program_ID = soup.shader_program_TRIANGLES;
        } else { ASSERT(SOUP_primitive == SOUP_TRI_MESH);
            GL_primitive = SOUP_TRIANGLES;
            shader_program_ID = soup.shader_program_TRI_MESH;
        }
    }
    ASSERT(shader_program_ID);
    glUseProgram(shader_program_ID);

    auto LOC = [&](char *name) { return glGetUniformLocation(shader_program_ID, name); };
    vec2 OpenGL_from_Pixel_scale = (2.0f / window_get_size_Pixel());

    glUniform1ui(LOC("stipple"), stipple);
    glUniform1ui(LOC("force_draw_on_top"), force_draw_on_top);
    glUniform2f(LOC("OpenGL_from_Pixel_scale"), OpenGL_from_Pixel_scale.x, OpenGL_from_Pixel_scale.y);
    glUniformMatrix4fv(LOC("transform"), 1, GL_TRUE, transform.data);

    if (SOUP_primitive != SOUP_QUADS) {
        glDrawArrays(GL_primitive, 0, num_vertices);
    } else { ASSERT(SOUP_primitive == SOUP_QUADS);
        const int MAX_VERTICES = 1000000;
        ASSERT(num_vertices <= MAX_VERTICES);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, soup.EBO[0]);
        {
            GL_primitive = GL_TRIANGLES;
            num_vertices = (num_vertices / 4) * 6;
            static GLuint *indices;
            if (!indices) {
                indices = (GLuint *) malloc(MAX_VERTICES / 4 * 6 * sizeof(GLuint));
                int k = 0;
                for (int i = 0; i < MAX_VERTICES / 4; ++i) {
                    indices[k++] = 4 * i + 2;
                    indices[k++] = 4 * i + 1;
                    indices[k++] = 4 * i + 0;
                    indices[k++] = 4 * i + 3;
                    indices[k++] = 4 * i + 2;
                    indices[k++] = 4 * i + 0;
                }
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_VERTICES / 4 * 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);
            }
        }
        glDrawElements(GL_primitive, num_vertices, GL_UNSIGNED_INT, NULL);
    }
}


////////////////////////////////////////////////////////////////////////////////
// eso /////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define ESO_MAX_VERTICES 9999999

struct {
    bool _called_eso_begin_before_calling_eso_vertex_or_eso_end;

    vec4 current_color;
    real current_size;

    bool overlay;
    bool stipple;

    mat4 transform;
    uint primitive;

    uint num_vertices;

    vec3 vertex_positions[ESO_MAX_VERTICES];
    vec4 vertex_colors[ESO_MAX_VERTICES];
    real vertex_sizes[ESO_MAX_VERTICES];
} eso;

void eso_begin(mat4 transform, uint primitive) {
    ASSERT(!eso._called_eso_begin_before_calling_eso_vertex_or_eso_end);
    eso._called_eso_begin_before_calling_eso_vertex_or_eso_end = true;

    eso.current_color = V4(basic.magenta, 1.0f);
    eso.current_size = 1.5f;

    eso.overlay = false;
    eso.stipple = false;

    eso.transform = transform;
    eso.primitive = primitive;

    eso.num_vertices = 0;
}

void eso_end() {
    ASSERT(eso._called_eso_begin_before_calling_eso_vertex_or_eso_end);
    eso._called_eso_begin_before_calling_eso_vertex_or_eso_end = false;
    soup_draw(
            eso.transform,
            eso.primitive,
            eso.num_vertices,
            eso.vertex_positions,
            eso.vertex_colors,
            eso.vertex_sizes,
            eso.overlay,
            eso.stipple);
}

void eso_overlay(bool overlay) {
    eso.overlay = overlay;
}

void eso_stipple(bool stipple) {
    eso.stipple = stipple;
}

void eso_size(real size) {
    eso.current_size = size;
}

void eso_color(real red, real green, real blue, real alpha) {
    eso.current_color[0] = red;
    eso.current_color[1] = green;
    eso.current_color[2] = blue;
    eso.current_color[3] = alpha;
}

void eso_color(real red, real green, real blue) {
    eso_color(red, green, blue, 1.0f);
}

void eso_color(vec3 rgb) {
    eso_color(rgb[0], rgb[1], rgb[2], 1.0f);
}

void eso_color(vec3 rgb, real alpha) {
    eso_color(rgb[0], rgb[1], rgb[2], alpha);
}

void eso_color(vec4 rgba) {
    eso_color(rgba[0], rgba[1], rgba[2], rgba[3]);
}

void eso_vertex(real x, real y, real z) {
    ASSERT(eso._called_eso_begin_before_calling_eso_vertex_or_eso_end);
    ASSERT(eso.num_vertices < ESO_MAX_VERTICES);
    eso.vertex_positions[eso.num_vertices] = { x, y, z };
    eso.vertex_colors[eso.num_vertices] = eso.current_color;
    eso.vertex_sizes[eso.num_vertices] = eso.current_size;
    ++eso.num_vertices;
}

void eso_vertex(real x, real y) {
    eso_vertex(x, y, 0.0f);
}


void eso_vertex(vec2 xy) {
    eso_vertex(xy[0], xy[1]);
}

void eso_vertex(vec3 xyz) {
    eso_vertex(xyz[0], xyz[1], xyz[2]);
}


// <!> End soup.cpp <!>
// <!> Begin text.cpp <!> 
// NOTE: this is a (slightly) modified version of stb_easy_font with a wrapper

static struct stb_easy_font_info_struct {
    unsigned char advance;
    unsigned char h_seg;
    unsigned char v_seg;
} stb_easy_font_charinfo[96] = {
    {  6,  0,  0 },  {  3,  0,  0 },  {  5,  1,  1 },  {  7,  1,  4 },
    {  7,  3,  7 },  {  7,  6, 12 },  {  7,  8, 19 },  {  4, 16, 21 },
    {  4, 17, 22 },  {  4, 19, 23 },  { 23, 21, 24 },  { 23, 22, 31 },
    { 20, 23, 34 },  { 22, 23, 36 },  { 19, 24, 36 },  { 21, 25, 36 },
    {  6, 25, 39 },  {  6, 27, 43 },  {  6, 28, 45 },  {  6, 30, 49 },
    {  6, 33, 53 },  {  6, 34, 57 },  {  6, 40, 58 },  {  6, 46, 59 },
    {  6, 47, 62 },  {  6, 55, 64 },  { 19, 57, 68 },  { 20, 59, 68 },
    { 21, 61, 69 },  { 22, 66, 69 },  { 21, 68, 69 },  {  7, 73, 69 },
    {  9, 75, 74 },  {  6, 78, 81 },  {  6, 80, 85 },  {  6, 83, 90 },
    {  6, 85, 91 },  {  6, 87, 95 },  {  6, 90, 96 },  {  7, 92, 97 },
    {  6, 96,102 },  {  5, 97,106 },  {  6, 99,107 },  {  6,100,110 },
    {  6,100,115 },  {  7,101,116 },  {  6,101,121 },  {  6,101,125 },
    {  6,102,129 },  {  7,103,133 },  {  6,104,140 },  {  6,105,145 },
    {  7,107,149 },  {  6,108,151 },  {  7,109,155 },  {  7,109,160 },
    {  7,109,165 },  {  7,118,167 },  {  6,118,172 },  {  4,120,176 },
    {  6,122,177 },  {  4,122,181 },  { 23,124,182 },  { 22,129,182 },
    {  4,130,182 },  { 22,131,183 },  {  6,133,187 },  { 22,135,191 },
    {  6,137,192 },  { 22,139,196 },  {  6,144,197 },  { 22,147,198 },
    {  6,150,202 },  { 19,151,206 },  { 21,152,207 },  {  6,155,209 },
    {  3,160,210 },  { 23,160,211 },  { 22,164,216 },  { 22,165,220 },
    { 22,167,224 },  { 22,169,228 },  { 21,171,232 },  { 21,173,233 },
    {  5,178,233 },  { 22,179,234 },  { 23,180,238 },  { 23,180,243 },
    { 23,180,248 },  { 22,189,248 },  { 22,191,252 },  {  5,196,252 },
    {  3,203,252 },  {  5,203,253 },  { 22,210,253 },  {  0,214,253 },
};

static unsigned char stb_easy_font_hseg[214] = {
    97,37,69,84,28,51,2,18,10,49,98,41,65,25,81,105,33,9,97,1,97,37,37,36,81,10,98,107,3,100,3,99,58,51,4,99,58,8,73,81,10,50,98,8,73,81,4,10,50,98,8,25,33,65,81,10,50,17,65,97,25,33,25,49,9,65,20,68,1,65,25,49,41,11,105,13,101,76,10,50,10,50,98,11,99,10,98,11,50,99,11,50,11,99,8,57,58,3,99,99,107,10,10,11,10,99,11,5,100,41,65,57,41,65,9,17,81,97,3,107,9,97,1,97,33,25,9,25,41,100,41,26,82,42,98,27,83,42,98,26,51,82,8,41, 35,8,10,26,82,114,42,1,114,8,9,73,57,81,41,97,18,8,8,25,26,26,82,26,82,26,82,41,25,33,82,26,49,73,35,90,17,81,41,65,57,41,65,25,81,90,114,20,84,73,57,41,49,25,33,65,81,9,97,1,97,25,33,65,81,57,33,25,41,25,
};

static unsigned char stb_easy_font_vseg[253] = {
    4,2,8,10,15,8,15,33,8,15,8,73,82,73,57,41,82,10,82,18,66,10,21,29,1,65, 27,8,27,9,65,8,10,50,97,74,66,42,10,21,57,41,29,25,14,81,73,57,26,8,8, 26,66,3,8,8,15,19,21,90,58,26,18,66,18,105,89,28,74,17,8,73,57,26,21, 8,42,41,42,8,28,22,8,8,30,7,8,8,26,66,21,7,8,8,29,7,7,21,8,8,8,59,7,8, 8,15,29,8,8,14,7,57,43,10,82,7,7,25,42,25,15,7,25,41,15,21,105,105,29, 7,57,57,26,21,105,73,97,89,28,97,7,57,58,26,82,18,57,57,74,8,30,6,8,8, 14,3,58,90,58,11,7,74,43,74,15,2,82,2,42,75,42,10,67,57,41,10,7,2,42, 74,106,15,2,35,8,8,29,7,8,8,59,35,51,8,8,15,35,30,35,8,8,30,7,8,8,60, 36,8,45,7,7,36,8,43,8,44,21,8,8,44,35,8,8,43,23,8,8,43,35,8,8,31,21,15, 20,8,8,28,18,58,89,58,26,21,89,73,89,29,20,8,8,30,7,
};

typedef struct {
    unsigned char c[4];
} stb_easy_font_color;

static int stb_easy_font_draw_segs(float x, float y, unsigned char *segs, int num_segs, int vertical, stb_easy_font_color c, char *vbuf, int vbuf_size, int offset) {
    int i,j;
    for (i=0; i < num_segs; ++i) {
        int len = segs[i] & 7;
        x += (float) ((segs[i] >> 3) & 1);
        if (len && offset+64 <= vbuf_size) {
            float y0 = y + (float) (segs[i]>>4);
            for (j=0; j < 4; ++j) {
                * (float *) (vbuf+offset+0) = x  + (j==1 || j==2 ? (vertical ? 1 : len) : 0);
                * (float *) (vbuf+offset+4) = y0 + (    j >= 2   ? (vertical ? len : 1) : 0);
                * (float *) (vbuf+offset+8) = 0.f;
                * (stb_easy_font_color *) (vbuf+offset+12) = c;
                offset += 16;
            }
        }
    }
    return offset;
}

static float stb_easy_font_spacing_val = 0;

// static void stb_easy_font_spacing(float spacing) {
//    stb_easy_font_spacing_val = spacing;
// }

static int stb_easy_font_print(float x, float y, String string, unsigned char color[4], void *vertex_buffer, int vbuf_size) {
    char *vbuf = (char *) vertex_buffer;
    float start_x = x;
    int offset = 0;

    stb_easy_font_color c = { 255,255,255,255 }; // use structure copying to avoid needing depending on memcpy()
    if (color) { c.c[0] = color[0]; c.c[1] = color[1]; c.c[2] = color[2]; c.c[3] = color[3]; }

    char *text = string.data;
    while (string_pointer_is_valid(string, text) && (offset < vbuf_size)) {
        if (*text == '\n') {
            y += 12;
            x = start_x;
        } else {
            unsigned char advance = stb_easy_font_charinfo[*text-32].advance;
            float y_ch = advance & 16 ? y+1 : y;
            int h_seg, v_seg, num_h, num_v;
            h_seg = stb_easy_font_charinfo[*text-32  ].h_seg;
            v_seg = stb_easy_font_charinfo[*text-32  ].v_seg;
            num_h = stb_easy_font_charinfo[*text-32+1].h_seg - h_seg;
            num_v = stb_easy_font_charinfo[*text-32+1].v_seg - v_seg;
            offset = stb_easy_font_draw_segs(x, y_ch, &stb_easy_font_hseg[h_seg], num_h, 0, c, vbuf, vbuf_size, offset);
            offset = stb_easy_font_draw_segs(x, y_ch, &stb_easy_font_vseg[v_seg], num_v, 1, c, vbuf, vbuf_size, offset);
            x += advance & 15;
            x += stb_easy_font_spacing_val;
        }
        ++text;
    }
    return (unsigned) offset/64;
}

static int stb_easy_font_travel_x(String string) {
    float len = 0;
    float max_len = 0;
    char *text = string.data;
    while (string_pointer_is_valid(string, text)) {
        if (*text == '\n') {
            if (len > max_len) max_len = len;
            len = 0;
        } else {
            len += stb_easy_font_charinfo[*text-32].advance & 15;
            len += stb_easy_font_spacing_val;
        }
        ++text;
    }
    if (len > max_len) max_len = len;
    return (int) ceil(max_len);
}

static int stb_easy_font_travel_y(String string) {
    int count = 0;
    char *text = string.data;
    while (string_pointer_is_valid(string, text)) {
        if (*text == '\n') ++count;
        ++text;
    }
    return count * 12;
}

static vec2 stb_easy_font_travel(String string) {
    return V2(real(stb_easy_font_travel_x(string)), real(stb_easy_font_travel_y(string)));
}

////////////////////////////////////////
// text_draw ///////////////////////////
////////////////////////////////////////

vec2 text_travel(String string, real font_height_Pixel) {
    return (font_height_Pixel / 12.0f) * stb_easy_font_travel(string);
}

// TODO: consider text_drawf
template <uint D_position, uint D_color> vec2 text_draw(
        mat4 PV,
        String string,
        Vector<D_position> position_World,
        Vector<D_color> color,
        real font_height_Pixel = 12.0f,
        vec2 nudge_Pixel = {},
        bool overlay = true
        ) {
    STATIC_ASSERT((D_position == 2) || (D_position == 3));
    STATIC_ASSERT((D_color == 3) || (D_color == 4));

    vec2 *vertex_positions;
    uint num_vertices;
    {
        uint size = 99999 * sizeof(float);
        static void *_vertex_positions = malloc(size);
        vertex_positions = (vec2 *) _vertex_positions;

        num_vertices = 4 * stb_easy_font_print(0, 0, string, NULL, _vertex_positions, size);
        { // NOTE: stb stores like this [x:float y:float z:float color:uint8[4]]
            for_(i, num_vertices) {
                ((vec2 *) vertex_positions)[i] = {
                    ((float *) vertex_positions)[4 * i + 0],
                    ((float *) vertex_positions)[4 * i + 1],
                };
            }
        }
    }

    vec3 position_World3; {
        position_World3.z = 0;
        for_(d, D_position) position_World3[d] = position_World[d];
    }
    vec3 position_OpenGL = transformPoint(PV, position_World3);

    if (!IS_BETWEEN_LOOSE(position_OpenGL.z, -1.0f, 1.0f)) return {};

    vec2 position_Pixel = transformPoint(inverse(window_get_OpenGL_from_Pixel()), _V2(position_OpenGL));

    mat4 transform = window_get_OpenGL_from_Pixel()
        * M4_Translation(position_Pixel + nudge_Pixel)
        * M4_Scaling(font_height_Pixel / 12.0f);
    eso_begin(transform, SOUP_QUADS);
    eso_overlay(overlay);
    eso_color(color);
    for_(i, num_vertices) eso_vertex(vertex_positions[i]);
    eso_end();

    return text_travel(string, font_height_Pixel);
}

////////////////////////////////////////
// easy_text_draw ///////////////////////////
////////////////////////////////////////

struct EasyTextPen {
    vec2 origin;
    real font_height_Pixel;
    vec3 color;
    bool automatically_append_newline;

    real one_minus_alpha;
    vec2 offset_Pixel;

    bool ghost_write; // does all the math and updates, just doesn't draw

    vec2 get_position_Pixel() {
        return this->origin + this->offset_Pixel;
    }
    real get_x_Pixel() { return this->origin.x + this->offset_Pixel.x; }
    real get_y_Pixel() { return this->origin.y + this->offset_Pixel.y; }
};

void easy_text_draw(EasyTextPen *pen, String string) {
    vec2 travel;
    if (!pen->ghost_write) {
        travel = text_draw(window_get_OpenGL_from_Pixel(), string, pen->get_position_Pixel(), V4(pen->color, 1.0f - pen->one_minus_alpha), pen->font_height_Pixel);
    } else {
        travel = text_travel(string, pen->font_height_Pixel);
    }

    if (IS_ZERO(travel.y) && (!pen->automatically_append_newline)) {
        pen->offset_Pixel.x += travel.x;
    } else {
        pen->offset_Pixel.x = 0.0f;
        pen->offset_Pixel.y += travel.y;
        if (pen->automatically_append_newline) pen->offset_Pixel.y += pen->font_height_Pixel;
    }
}

void easy_text_drawf(EasyTextPen *pen, const char *format, ...) {
    #define EASY_TEXT_MAX_LENGTH 4096
    static _STRING_CALLOC(string, EASY_TEXT_MAX_LENGTH); {
        va_list arg;
        va_start(arg, format);
        string.length = vsnprintf(string.data, EASY_TEXT_MAX_LENGTH, format, arg);
        va_end(arg);
    }
    easy_text_draw(pen, string);
}

real _easy_text_dx(EasyTextPen *pen, String string) {
    return text_travel(string, pen->font_height_Pixel).x;
}

real _easy_text_dx(EasyTextPen *pen, char *cstring) {
    return _easy_text_dx(pen, STRING(cstring));
}
// <!> End text.cpp <!>
#define sin use_SIN_instead_of_sin
#define cos use_COS_instead_of_cos
#define tan use_TAN_instead_of_tan
#define atan2 use_ATAN2_instead_of_atan2
#define pow use_POW_instead_of_pow
#define sqrt use_SQRT_instead_of_sqrt
#define atan use_atan2_instead_of_atan

// <!> End playground.cpp <!>

char *startup_script = "";

#if 1 // circle
run_before_main {
    startup_script =
        // "^..cz32\nlq<m2d 10 0>zs<m2d 0 0>\b<esc>le<m2d 10 0>q<m2d -10 0>"
        "^.cz32\nlq<m2d 10 0>zs<m2d 0 0>\b<esc>"
        // "cz16\n"
        // "s<m2d> 0 0>\b"
        // "lq<m2d 8 0>q<m2d 16 0>"
        // "sc"
        "^odemo.dxf\n"
        ;
};
#endif

#if 0 // bug.dxf load
run_before_main {
    startup_script = "^.^obug.dxf\n";
};
#endif

#if 0 // glorbo
run_before_main {
    startup_script = "^.^oglorbo.dxf\n"
        // "^Oglorbo.stl\n"
        ;
};
#endif

#if 0 // Box tweening
run_before_main {
    startup_script = "bz5";
};
#endif

#if 0 // UP DOWN polygon
run_before_main {
    startup_script = "pz";
};
#endif

#if 0 // hotkeys not working bug
run_before_main {
    startup_script = "cx20\n10\nsay]90\t30";
};
#endif

#if 0 // hotkeys not working bug
run_before_main {
    startup_script = "^odemo.dxf\n";
};
#endif

#if 0 // divide
run_before_main {
    startup_script = 
        "cz10\n"
        "s<m2d 0 -10>\b"
        "lzm<m2d 0 10>"
        // "cz10\n"
        // "s<m2d 0 -10>\b"
        // "lzm<m2d 0 10>"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "UUUUUUUUUU"
        ;
    #if 0
    "bz10\t10\n"
        "lm<m2d 5 0>m<m2d 5 10>"
        "lm<m2d 0 5>m<m2d 10 5>"
        "sa\b"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        #endif
};
#endif

#if 0 // multi-popup
run_before_main {
    startup_script = 
        // "[lz"
        // "[10lz"
        // "f123[456"
        "lz12\t34x56\t78[98\t76"
        // "lzx["
        ;
};
#endif

#if 0 // revolve
run_before_main {
    startup_script = \
                     "y"
                     "cz10\n"
                     "sa[10\n"
                     // "Zx-5\n"
                     "y"
                     "samzx5\n"
                     // "Ax5\nx5\t5\n"
                     "sa]\n"
                     ; };
#endif

#if 0 // fillet
run_before_main {
    startup_script = \
                     "Bz40\t20\nf5<m2d 15 10><m2d 20 5>";
};
#endif

#if 0 // dogear
run_before_main {
    startup_script = \
                     "Bz40\t20\ng5<m2d 15 10><m2d 20 5>";
};
#endif

#if 0 // kitchen sink
run_before_main {
    startup_script = 
        #if 1
        "cz10\n" // circle
        "cz\t10\n" // bigger circle
        "bzx30\t30\n" // box
        "ysadc<m2d 0 0>" // TODO: comment
        "[5\t15\n" // extrude
        "sc<m2d 0 30><esc>qs3" // TODO: comment
        "1<m2d 30 15>0<esc>" // TODO: comment
        "sq1sq3me<m2d 40 40>x15\t15\n" // TODO: comment
        "{3\n" // TODO: comment
        "sc<m2d 0 0>Zm<m2d -50 0>" // TODO: comment
        "sc<m2d 0 0>Am<m2d -50 0><m2d -15 5>" // TODO: comment
        "]360\n"
        "^n" // TODO: comment
        "cx30\t30\n3.4\n" // TODO: comment
        "saXzYzXzsa[1\n" // TODO: comment
        #endif
        "^osplash.dxf\nsc<m2d 24 0><m2d 16 0>"
        #if 1
        "[\t10\n" // TODO: comment
        "Ac<m2d 15.3 15.4>c<m2d -16.4 -16.3>sc<m2d -16 16>]\n" // TODO: comment
        "^n" // TODO: comment
        "l<m2d 0 0><m2d 0 10>l<m2d 0 10><m2d 10 0>l<m2d 10 0><m2d 0 0>" // TODO: comment
        "n25\n" // TODO: comment
        "sa[1\n" // TODO: comment
        "n0\n" // TODO: comment
        "^n" // TODO: comment
        "cz8\n" // TODO: comment
        "<m3d 1 100 -1 0 -1 0>" // TODO: comment
        "sa{100\n" // TODO: comment
        ";" // TODO: comment
        "^odemo.dxf\n" // TODO: comment
                       // "^signore.stl\ny\n" // TODO: comment
                       // "^oignore.stl\n" // TODO: comment
                       // ".." // TODO: comment
                       // "pz\t5\n" // (Nathan) Polygon
                       // "cz18\nD<m2d 0 9>D<m2d 0 -9>s<m2d 2 -9><m2d -2 9>\b" // (Henok) DivideNearest
                       // "j2<m2d 1 7><m2d -1 -7>\n" //(Henok) Offset
                       // "^N^ob:wug.drawing\nysa"
        #endif
        ;
};
#endif
#ifdef SHIP
run_before_main {
    startup_script = "";
    glfwSetWindowTitle(glfw_window, "Conversation pre-alpha " __DATE__ " " __TIME__);
};
#endif

#include "manifoldc.h"
// <!> Begin header.cpp <!> 
////////////////////////////////////////
// Forward-Declarations ////////////////
////////////////////////////////////////

void messagef(vec3 color, char *format, ...);
template <typename T> void JUICEIT_EASYTWEEN(T *a, T b, real multiplier = 1.0f);
// TODO: take entire transform (same used for draw) for wrapper_manifold--strip out incremental nature into function



///////////


enum class ToolboxGroup {
    None,
    Draw,
    Snap,
    Mesh,
    Xsel, // Select
    Colo, // SetColor
    Both, // DrawingAndMesh
    NUMBER_OF,
};

struct Shortcut {
    uint key;
    u8 mods;
};

#define CMD_FLAG_ (1 << 00)
struct Command {
    ToolboxGroup group;
    bool is_mode;
    u64 flags;
    String name;
    #define COMMAND_MAX_NUM_SHORTCUTS 4
    Shortcut shortcuts[COMMAND_MAX_NUM_SHORTCUTS];
};

bool command_equals(Command A, Command B) {
    return (A.name.data == B.name.data);
}

// FORNOW ew
#define state_Draw_command_is_(Name) command_equals(state.Draw_command, commands.Name)
#define state_Mesh_command_is_(Name) command_equals(state.Mesh_command, commands.Name)
#define state_Snap_command_is_(Name) command_equals(state.Snap_command, commands.Name)
#define state_Xsel_command_is_(Name) command_equals(state.Xsel_command, commands.Name)
#define state_Colo_command_is_(Name) command_equals(state.Colo_command, commands.Name)

// FORNOW ew
#define set_state_Draw_command(Name) do { /*ASSERT(command_equals(commands.Name, commands.None) || (commands.Name.group == ToolboxGroup::Draw));*/ state.Draw_command = commands.Name; } while (0)
#define set_state_Mesh_command(Name) do { /*ASSERT(command_equals(commands.Name, commands.None) || (commands.Name.group == ToolboxGroup::Mesh));*/ state.Mesh_command = commands.Name; } while (0)
#define set_state_Snap_command(Name) do { /*ASSERT(command_equals(commands.Name, commands.None) || (commands.Name.group == ToolboxGroup::Snap));*/ state.Snap_command = commands.Name; } while (0)
#define set_state_Xsel_command(Name) do { /*ASSERT(command_equals(commands.Name, commands.None) || (commands.Name.group == ToolboxGroup::Xsel));*/ state.Xsel_command = commands.Name; } while (0)
#define set_state_Colo_command(Name) do { /*ASSERT(command_equals(commands.Name, commands.None) || (commands.Name.group == ToolboxGroup::Colo));*/ state.Colo_command = commands.Name; } while (0)


// <!> Begin commands.cpp <!> 
// NOTE: FORNOW: FOCUS_THIEF is really the same thing as having a popup (except for two-click commands like Line where the popup doesn't show initially)
#define TWO_CLICK     (1 << 0)
#define FOCUS_THIEF   (1 << 1)
#define SNAPPER       (1 << 2)
#define SHIFT_15      (1 << 3)
#define NO_RECORD     (1 << 4)
#define EXCLUDE_SELECTED_FROM_SECOND_CLICK_SNAP (1 << 5)
#define HIDE_FEATURE_PLANE (1 << 6)
#define _UNSUED_FLAG7 (1 << 7)
#define _UNSUED_FLAG8 (1 << 8)
#define _UNSUED_FLAG9 (1 << 9)


#define COMMANDS_OUTER \
    COMMANDS_INNER(None, 0, 0, None, 0, 0); \
    COMMANDS_INNER(Escape, GLFW_KEY_ESCAPE, 0b000, Both, 0, 0); \
    \
    COMMANDS_INNER(Undo,            'U', 0b000, Both, 0, 0 | NO_RECORD, 'Z', 0b010); \
    COMMANDS_INNER(Redo,            'U', 0b001, Both, 0, 0 | NO_RECORD, 'Y', 0b010, 'Z', 0b011 ); \
    COMMANDS_INNER(ToggleDetails,   '.', 0b000, Both, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(ToggleGUI,       '.', 0b010, Both, 0, 0 | NO_RECORD);  \
    \
    COMMANDS_INNER(Center,          'C', 0b000, Snap, 1, 0); \
    COMMANDS_INNER(End,             'E', 0b000, Snap, 1, 0); \
    COMMANDS_INNER(Intersect,       'I', 0b000, Snap, 1, 0); \
    COMMANDS_INNER(Middle,          'M', 0b000, Snap, 1, 0); \
    COMMANDS_INNER(Perp,            'P', 0b000, Snap, 1, 0); \
    COMMANDS_INNER(Quad,            'Q', 0b000, Snap, 1, 0); \
    /*COMMANDS_INNER(Tangent,         'T', 0b000, Snap, 1, 0); */\
    COMMANDS_INNER(XY,              'X', 0b000, Snap, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(Zero,            'Z', 0b000, Snap, 0, 0); \
    COMMANDS_INNER(ClearSnap,         0, 0b000, Snap, 0, 0); \
    \
    COMMANDS_INNER(SetAxis,         'A', 0b001, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    COMMANDS_INNER(Box,             'B', 0b000, Draw, 1, 0 | TWO_CLICK | SNAPPER); \
    COMMANDS_INNER(SetColor,        'Q', 0b000, Draw, 1, 0); \
    COMMANDS_INNER(CenterBox,       'B', 0b001, Draw, 1, 0 | TWO_CLICK | SNAPPER); \
    COMMANDS_INNER(CenterLine,      'L', 0b001, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    COMMANDS_INNER(Circle,          'C', 0b000, Draw, 1, 0 | TWO_CLICK | SNAPPER); \
    COMMANDS_INNER(Copy,            'O', 0b000, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15 | EXCLUDE_SELECTED_FROM_SECOND_CLICK_SNAP); \
    COMMANDS_INNER(Deselect,        'D', 0b000, Draw, 1, 0); \
    COMMANDS_INNER(DiamCircle,      'C', 0b010, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    COMMANDS_INNER(Divide2,         'I', 0b000, Draw, 1, 0 | TWO_CLICK); \
    COMMANDS_INNER(Fillet,          'F', 0b000, Draw, 1, 0 | TWO_CLICK | FOCUS_THIEF); \
    COMMANDS_INNER(DogEar,          'G', 0b000, Draw, 1, 0 | TWO_CLICK | FOCUS_THIEF); \
    COMMANDS_INNER(Line,            'L', 0b000, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    COMMANDS_INNER(Measure,         'M', 0b001, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    COMMANDS_INNER(Mirror2,         'M', 0b011, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    COMMANDS_INNER(Move,            'M', 0b000, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15 | EXCLUDE_SELECTED_FROM_SECOND_CLICK_SNAP); \
    COMMANDS_INNER(Drag,            'D', 0b001, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15 | EXCLUDE_SELECTED_FROM_SECOND_CLICK_SNAP); \
    COMMANDS_INNER(Offset,          'H', 0b000, Draw, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(OpenDXF,         'O', 0b010, Draw, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(SetOrigin,       'Z', 0b001, Draw, 1, 0 | SNAPPER); \
    COMMANDS_INNER(Polygon,         'P', 0b000, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    COMMANDS_INNER(RCopy,           'R', 0b001, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15); \
    COMMANDS_INNER(Rotate,          'R', 0b000, Draw, 1, 0 | TWO_CLICK | SNAPPER | SHIFT_15 | EXCLUDE_SELECTED_FROM_SECOND_CLICK_SNAP); \
    COMMANDS_INNER(SaveDXF,         'S', 0b010, Draw, 1, 0 | FOCUS_THIEF | NO_RECORD); \
    COMMANDS_INNER(Scale,           'S', 0b001, Draw, 1, 0 | NO_RECORD); \
    COMMANDS_INNER(Select,          'S', 0b000, Draw, 1, 0); \
    COMMANDS_INNER(MirrorX,         'X', 0b001, Draw, 1, 0 | SNAPPER); \
    COMMANDS_INNER(MirrorY,         'Y', 0b001, Draw, 1, 0 | SNAPPER); \
    COMMANDS_INNER(ClearDrawing,    'N', 0b010, Draw, 0, 0); \
    COMMANDS_INNER(ZoomDrawing,       0, 0b000, Draw, 0, 0 | NO_RECORD); \
    \
    COMMANDS_INNER(Delete,               GLFW_KEY_DELETE, 0b000, Draw, 0, 0, GLFW_KEY_BACKSPACE, 0b000);  \
    COMMANDS_INNER(OverwriteDXF,                       0, 0b000, Draw, 0, 0); \
    COMMANDS_INNER(OverwriteSTL,                       0, 0b000, Mesh, 0, 0); \
    \
    COMMANDS_INNER(ExtrudeAdd,      '[', 0b000, Mesh, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(ExtrudeCut,      '[', 0b001, Mesh, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(NudgePlane,      'N', 0b000, Mesh, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(OpenSTL,         'O', 0b011, Mesh, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(CyclePlane,      'Y', 0b000, Mesh, 0, 0); \
    COMMANDS_INNER(HidePlane,       ';', 0b000, Mesh, 0, 0);  \
    COMMANDS_INNER(MirrorPlaneX,     0,  0b000, Mesh, 0, 0);  \
    COMMANDS_INNER(MirrorPlaneY,     0,  0b000, Mesh, 0, 0);  \
    \
    COMMANDS_INNER(RevolveAdd,      ']', 0b000, Mesh, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(RevolveCut,      ']', 0b001, Mesh, 1, 0 | FOCUS_THIEF); \
    COMMANDS_INNER(SaveSTL,         'S', 0b011, Mesh, 1, 0 | FOCUS_THIEF | NO_RECORD); \
    COMMANDS_INNER(ClearMesh,       'N', 0b011, Mesh, 0, 0); \
    COMMANDS_INNER(ZoomMesh,          0, 0b000, Mesh, 0, 0 | NO_RECORD); \
    COMMANDS_INNER(ZoomPlane,         0, 0b000, Mesh, 0, 0 | NO_RECORD); \
    \
    COMMANDS_INNER(Measure3D,       'M', 0b011, Mesh, 1, 0 | TWO_CLICK | SNAPPER | HIDE_FEATURE_PLANE); \
    \
    \
    COMMANDS_INNER(All,             'A', 0b000, Xsel, 0, 0); \
    COMMANDS_INNER(Connected,       'C', 0b000, Xsel, 1, 0); \
    COMMANDS_INNER(Window,          'W', 0b000, Xsel, 1, 0 | TWO_CLICK); \
    COMMANDS_INNER(ByColor,         'Q', 0b000, Xsel, 1, 0); \
    \
    COMMANDS_INNER(OfSelection,        'S', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color0,          '0', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color1,          '1', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color2,          '2', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color3,          '3', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color4,          '4', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color5,          '5', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color6,          '6', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color7,          '7', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color8,          '8', 0b000, Colo, 1, 0); \
    COMMANDS_INNER(Color9,          '9', 0b000, Colo, 1, 0); \
    \
    \
    \
    \
    \
    \
    \
    \
    \
    \
    \
    \
    \
    \
    COMMANDS_INNER(TOGGLE_BUTTONS,          GLFW_KEY_TAB, 0b001, None, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(TOGGLE_EVENT_STACK,               'K', 0b011, None, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(TOGGLE_GRID,                      'G', 0b000, None, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(TOGGLE_LIGHT_MODE,                'L', 0b011, None, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(PREVIOUS_HOT_KEY_2D,              ' ', 0b000, None, 0, 0);  \
    COMMANDS_INNER(PREVIOUS_HOT_KEY_3D,              ' ', 0b001, None, 0, 0);  \
    COMMANDS_INNER(PRINT_HISTORY,                    'H', 0b010, None, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(PowerFillet,                      'F', 0b001, None, 0, 0);  \
    COMMANDS_INNER(HELP_MENU,                        '/', 0b010, None, 0, 0 | NO_RECORD);  \
    COMMANDS_INNER(DivideNearest,                    'X', 0b000, None, 0, 0);  \
    // COMMANDS_INNER(NEXT_POPUP_BAR,                  TAB,  0b000, None, 0, 0);/* secretly supported but scary */ \




    // TODO: tag each command with an ID?--have them part of an enum?
    //       (checking equality based on name.data feels dangerous)


struct {
    #define COMMANDS_INNER(NAME, CHAR, CODE, GROUP, IS_MODE, FLAGS, ...) \
    Command NAME = { ToolboxGroup::GROUP, IS_MODE, FLAGS, STRING(STR(NAME)), { CHAR, CODE, __VA_ARGS__ } };

    COMMANDS_OUTER;

    #undef COMMANDS_INNER
} commands;


#define CONFIG_OUTER \
    CONFIG_INNER(HIDE_GUI, false);  \
    CONFIG_INNER(usingInches, false);


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct {
    #define CONFIG_INNER(NAME, VALUE) \
    real NAME = VALUE

    CONFIG_OUTER;

    #undef CONFIG_INNER
} config;





real parse_config(String str) {
    if (0) {
    } else if (string_matches_prefix(str, "F")) {
        return 0.0f;
    } else if (string_matches_prefix(str, "T")) {
        return 1.0f;
    } else {
        return real(strtol(str.data, NULL, 0)); 
    }
}

uint parse_key(String str) {
    if (0) {
    } else if (string_matches_prefix(str, "ESCAPE")) {
        return GLFW_KEY_ESCAPE;
    } else if (string_matches_prefix(str, "TAB")) {
        return GLFW_KEY_TAB;
    } else if (string_matches_prefix(str, "DELETE")) {
        return GLFW_KEY_DELETE;
    } else if (string_matches_prefix(str, "BACKSPACE")) {
        return GLFW_KEY_BACKSPACE;
    } else {
        return (uint)str.data[0];
    }
}

#if 0
Command parse_command(String str) {
    char *start = str.data;
    Command command = {};
    while (str.data - start < str.length) {
        if (string_matches_prefix(str, "SHIFT+")) {
            command.shortcut.mods |= MOD_SHIFT;
            str.data += 6;
        } else if (string_matches_prefix(str, "CTRL+")) { 
            command.shortcut.mods |= MOD_CTRL;
            str.data += 5;
        } else if (string_matches_prefix(str, "ALT+")) {
            command.shortcut.mods |= MOD_ALT;
            str.data += 4;
        } else {
            command.shortcut.key = parse_key(STRING(str.data));
            break;
        }
    }
    return command;
}

Command COMMAND(uint command_name, unsigned char modifier) {
    return { command_name, modifier };
}

run_before_main {

    FILE *file = fopen("conversation.cfg", "r");
    if (!file) {
        // messagef(pallete.red, "Failed to open commands file");
        return;
    }
    defer { fclose(file); };

    #define LINE_MAX_LENGTH 256
    static _STRING_CALLOC(line, LINE_MAX_LENGTH);
    while (FGETS(&line, LINE_MAX_LENGTH, file)) {
        if (line.length == 0) {
        } else if (line.data[0] == '#') {
        } else {
            bool is_valid;
            String command_name;
            String command_string;
            {
                uint index_of_equals_sign; {
                    index_of_equals_sign = 0;
                    while ((line.data[index_of_equals_sign] != '=') && (line.data[index_of_equals_sign])) ++index_of_equals_sign; 
                }
                is_valid = (line.data[index_of_equals_sign] != '\0' || index_of_equals_sign < line.length);
                command_string = { &line.data[index_of_equals_sign + 1], line.length - index_of_equals_sign - 1 - 1 };
                command_name = { line.data, index_of_equals_sign };
                //DEBUGBREAK();
            }
            if (is_valid) {
                #define COMMANDS_INNER(NAME, _CHAR, _CODE, _GROUP, _FLAGS, _IS_MODE) \
                else if (string_matches_prefix(command_name, STR(NAME))) commands.NAME = parse_command(command_string)

                if (0);
                COMMANDS_OUTER;

                #undef INNER


                #define CONFIG_INNER(NAME, _VALUE) \
                else if (string_matches_prefix(command_name, STR(NAME))) config.NAME = parse_config(command_string)

                if (0);
                CONFIG_OUTER;

                #undef CONFIG_OUTER
            }
        }
    }


    // config stuff
    {
        // other.hide_toolbox = config.HIDE_GUI;
    }
};
#undef OUTER
#endif
// <!> End commands.cpp <!>

Command commands_Color[] = { 
    commands.Color0,
    commands.Color1,
    commands.Color2,
    commands.Color3,
    commands.Color4,
    commands.Color5,
    commands.Color6,
    commands.Color7,
    commands.Color8,
    commands.Color9
};


enum class EntityType {
    Arc,
    Line,
    Circle,
};

enum class Pane {
    None,
    Drawing,
    Mesh,
    Popup,
    Separator,
    Toolbox,
};

enum class CellType {
    None,
    Real,
    String,
    Uint,
};

enum class EventType {
    None,
    Key,
    Mouse,
};

enum class KeyEventSubtype {
    None,
    Hotkey,
    Popup,
};

enum class MouseEventSubtype {
    None,
    Drawing,
    Mesh,
    Popup,
    ToolboxButton,
};

enum class ColorCode {
    Traverse,
    Quality1,
    Quality2,
    Quality3,
    Quality4,
    Quality5,
    Etch,
    Unknown,
    _WaterOnly,
    LeadIO,
    QualitySlit1 = 21,
    QualitySlit2,
    QualitySlit3,
    QualitySlit4,
    QualitySlit5,
    Selection = 255,
    Emphasis = 254,
};

/////////////////


////////////////////////////////////////
// structs /////////////////////////////
////////////////////////////////////////

struct LineEntity {
    vec2 start;
    vec2 end;
};

struct ArcEntity {
    vec2 center;
    real radius;
    real start_angle_in_degrees;
    real end_angle_in_degrees;
};

struct CircleEntity {
    vec2 center;
    real radius;
    bool has_pseudo_point;
    real pseudo_point_angle;

    vec2 get_pseudo_point() {
        vec2 get_point_on_circle_NOTE_pass_angle_in_radians(vec2, real, real);
        return get_point_on_circle_NOTE_pass_angle_in_radians(this->center, this->radius, this->pseudo_point_angle);
    }

    void set_pseudo_point(vec2 pseudo_point) {
        ASSERT(!ARE_EQUAL(this->center, pseudo_point));
        this->pseudo_point_angle = ATAN2(pseudo_point - this->center);
    }
};


struct Entity {
    EntityType type;

    ColorCode color_code;
    bool is_selected;
    vec3 preview_color;
    real time_since_is_selected_changed;

    LineEntity line;
    ArcEntity arc;
    CircleEntity circle;
};

struct Mesh {
    uint num_vertices;
    uint num_triangles;
    vec3 *vertex_positions;
    uint3 *triangle_indices;
    vec3 *triangle_normals;

    // ??
    uint num_cosmetic_edges;
    uint2 *cosmetic_edges;

    bbox3 bbox;
};

struct RawKeyEvent {
    uint key;
    bool control;
    bool shift;
    bool alt;
};

struct RawMouseEvent {
    Pane pane;
    vec2 mouse_Pixel;
    bool mouse_held;
    bool mouse_double_click;
    bool mouse_double_click_held;
};

struct RawEvent {
    EventType type;

    RawKeyEvent raw_key_event;
    RawMouseEvent raw_mouse_event;
};

struct MagicSnapResult {
    vec2 mouse_position;
    bool snapped;
    uint entity_index_snapped_to;
    uint entity_index_intersect;
    uint entity_index_tangent_2;
    bool split_intersect;
    bool split_tangent_2;
};

struct MagicSnapResult3D {
    vec3 mouse_position;
    uint triangle_index;

    bool snapped;
    bool hit_mesh;
};

struct MouseEventDrawing {
    MagicSnapResult snap_result;
};

struct MouseEventMesh {
    vec3 mouse_ray_origin;
    vec3 mouse_ray_direction;
};

struct MouseEventPopup {
    uint cell_index;
    uint cursor;
};

struct MouseEventToolboxButton {
    char *name;
};

struct MouseEvent {
    MouseEventSubtype subtype;

    vec2 mouse_Pixel;
    bool mouse_held;
    bool mouse_double_click;
    bool mouse_double_click_held;

    MouseEventDrawing mouse_event_drawing;
    MouseEventMesh mouse_event_mesh;
    MouseEventPopup mouse_event_popup;
    MouseEventToolboxButton mouse_event_toolbox_button;
};

struct KeyEvent {
    KeyEventSubtype subtype;

    uint key;
    bool control;
    bool shift;
    bool alt;
    char *_name_of_spoofing_button;
};

struct Event {
    EventType type;

    // ew
    bool record_me;
    bool checkpoint_me;
    bool snapshot_me;

    KeyEvent key_event;
    MouseEvent mouse_event;
};

struct Drawing {
    List<Entity> entities;
    vec2 origin;
    vec2 axis_base_point;
    real axis_angle_from_y;
};

struct FeaturePlaneState {
    bool is_active;
    vec3 normal;
    real signed_distance_to_world_origin;
};

struct TwoClickCommandState {
    bool awaiting_second_click;
    vec2 first_click;
    Entity *entity_closest_to_first_click;
    bool tangent_first_click; // first thing that came to mind might be more elegant solution
};

struct MeshTwoClickCommandState {
    bool awaiting_second_click;
    vec3 first_click;
};

struct PopupManager {
    char *tags[uint(ToolboxGroup::NUMBER_OF)];
    //
    char *get_tag(ToolboxGroup group) { return tags[uint(group)]; }
    void set_tag(ToolboxGroup group, char *_name0) { tags[uint(group)] = _name0; }

    bool _popup_popup_called_this_process[uint(ToolboxGroup::NUMBER_OF)];
    bool focus_group_was_set_manually;
    ToolboxGroup focus_group;

    //////////

    // TODO: problem is here (calling being_process interacts poorly with the recursion)
    void begin_process() {
        // // NOTE: end of previous call to process
        // -- (so we don't also need an end_process())
        bool any_active; {
            any_active = false;
            for (uint i = 1; i < uint(ToolboxGroup::NUMBER_OF); ++i) {
                if (!_popup_popup_called_this_process[i]) {
                    tags[i] = NULL;
                } else {
                    any_active = true;
                }
            }
        }
        if (!any_active) focus_group = ToolboxGroup::None;
        // // NOTE: beginning of this call to process
        focus_group_was_set_manually = false;
        memset(_popup_popup_called_this_process, 0, sizeof(_popup_popup_called_this_process));
    }

    // void end_process() {
    //     bool tag_corresponding_to_focus_group_became_NULL = (focus_group != ToolboxGroup::None) && (get_tag(focus_group) == NULL);
    //     if (tag_corresponding_to_focus_group_became_NULL) focus_group = ToolboxGroup::None;
    // }

    void manually_set_focus_group(ToolboxGroup new_focus_group) {
        // ASSERT(get_tag(new_focus_group)); // TODO: really important to get this assert working
        focus_group_was_set_manually = true;
        focus_group = new_focus_group;
    }

    void register_call_to_popup_popup(ToolboxGroup group) {
        _popup_popup_called_this_process[uint(group)] = true;
    }
};

#define POPUP_MAX_NUM_CELLS 5
#define POPUP_CELL_LENGTH 256
struct PopupState {
    _STRING_CALLOC(active_cell_buffer, POPUP_CELL_LENGTH);

    PopupManager manager;
    bool a_popup_from_this_group_was_already_called_this_frame[uint(ToolboxGroup::NUMBER_OF)];

    uint active_cell_index;
    uint cursor;
    uint selection_cursor;

    CellType _type_of_active_cell;

    bool _FORNOW_info_mouse_is_hovering;

    real extrude_add_out_length;
    real extrude_add_in_length;
    real extrude_cut_in_length;
    real extrude_cut_out_length;
    real circle_diameter;
    real circle_radius;
    real circle_circumference;
    real fillet_radius;
    real dogear_radius;
    real box_width;
    real box_height;
    real xy_x_coordinate;
    real xy_y_coordinate;
    real feature_plane_nudge;
    real line_length;
    real line_angle;
    real line_run;
    real line_rise;
    real drag_length;
    real drag_angle;
    real drag_run;
    uint drag_extend_line; // TODO: THIS SHOULD BE BOOL
    real drag_rise;
    real move_length;
    real move_angle;
    real move_run;
    real move_rise;
    real linear_copy_length;
    real linear_copy_angle;
    real linear_copy_run;
    real linear_copy_rise;
    uint linear_copy_num_additional_copies;
    real offset_distance;
    uint polygon_num_sides = 6;
    real polygon_distance_to_side;
    real polygon_distance_to_corner;
    real polygon_side_length;
    real revolve_add_in_angle;
    real revolve_add_out_angle;
    real revolve_cut_in_angle;
    real revolve_cut_out_angle;
    real rotate_angle;
    uint rotate_copy_num_total_copies;
    real rotate_copy_angle;
    real scale_factor;
    _STRING_CALLOC(open_dxf_filename, POPUP_CELL_LENGTH);
    _STRING_CALLOC(save_dxf_filename, POPUP_CELL_LENGTH);
    _STRING_CALLOC(overwrite_dxf_yn_buffer, POPUP_CELL_LENGTH);
    _STRING_CALLOC(open_stl_filename, POPUP_CELL_LENGTH);
    _STRING_CALLOC(save_stl_filename, POPUP_CELL_LENGTH);
    _STRING_CALLOC(overwrite_stl_yn_buffer, POPUP_CELL_LENGTH);
};

struct ToolboxState {
    char *hot_name;
};

struct WorldState_ChangesToThisMustBeRecorded_state {
    Mesh mesh;
    Drawing drawing;
    FeaturePlaneState feature_plane;
    TwoClickCommandState two_click_command;
    MeshTwoClickCommandState mesh_two_click_command;
    PopupState popup;
    ToolboxState toolbox;

    Command Draw_command = commands.None; // FORNOW (command_equals)
    Command Mesh_command = commands.None; // FORNOW
    Command Snap_command = commands.None; // FORNOW
    Command Xsel_command = commands.None; // FORNOW
    Command Colo_command = commands.None; // FORNOW

    Event space_bar_event;
    Event shift_space_bar_event;
};

struct PreviewState {
    bbox2 feature_plane;
    real extrude_in_length;
    real extrude_out_length;
    real revolve_in_angle;
    real revolve_out_angle;
    vec3 tubes_color;
    vec3 feature_plane_color;
    real feature_plane_offset;
    vec2 drawing_origin;
    vec2 mouse;
    real cursor_subtext_alpha;

    vec2 popup_second_click;
    vec2 xy_xy;
    vec2 mouse_snap;
    real polygon_num_sides;
    vec3 color_mouse;

    vec2 offset_entity_start;
    vec2 offset_entity_end;
    vec2 offset_entity_middle;
    vec2 offset_entity_opposite;
};

struct Cursors {
    // pass NULL to glfwSetCursor to go back to the arrow
    GLFWcursor *curr;
    GLFWcursor *crosshair;
    GLFWcursor *ibeam;
    GLFWcursor *hresize;
    GLFWcursor *hand;
};

struct ScreenState_ChangesToThisDo_NOT_NeedToBeRecorded_other {
    mat4 OpenGL_from_Pixel;
    mat4 transform_Identity = M4_Identity();

    Cursors cursors;

    Camera camera_drawing;
    Camera camera_mesh;

    bool hide_grid;
    bool show_details;
    bool show_help;
    bool show_event_stack;
    bool hide_toolbox;
    bool mirror_3D_plane_X = false;
    bool mirror_3D_plane_Y = false;

    Pane hot_pane;
    real x_divider_drawing_mesh_OpenGL = 0.15f;
    Pane mouse_left_drag_pane;
    Pane mouse_right_drag_pane;

    long timestamp_mouse_left_click;
    bool mouse_double_left_click_held;

    bool shift_held;
    vec2 mouse_OpenGL;
    vec2 mouse_Pixel;

    bool _please_suppress_drawing_popup_popup; // NOTE: THIS IS A TERRRRRIBLE VARIABLE NAME
    bool please_suppress_messagef;
    bool _please_suppress_drawing_toolbox;
    bool _please_suppress_drawing_toolbox_snaps;

    bool slowmo;
    bool paused;
    bool stepping_one_frame_while_paused;

    real time_since_cursor_start;
    real time_since_successful_feature;
    real time_since_plane_selected;
    real time_since_plane_deselected;
    real time_since_going_inside;
    real time_since_mouse_moved;
    real time_since_popup_second_click_not_the_same;


    vec2 snap_divide_dot;
    real size_snap_divide_dot;

    PreviewState preview;

};

struct StandardEventProcessResult {
    bool record_me;
    bool checkpoint_me;
    bool snapshot_me;
};

//////////////////////////////////

////////////////////////////////////////
// colors //////////////////////////////
////////////////////////////////////////

struct {
    #if 0
    vec3 red = RGB255(255, 0, 0);
    vec3 orange = RGB255(204, 136, 1);
    vec3 green = RGB255(83, 255,  85);
    vec3 blue = RGB255(0, 85, 255);
    vec3 purple = RGB255(170, 1, 255);
    vec3 pink = RGB255(238, 0, 119);
    #else
    vec3 red = monokai.red;
    vec3 orange = monokai.orange;
    vec3 green = monokai.green;
    vec3 blue = monokai.blue;
    vec3 purple = monokai.purple;
    vec3 brown = monokai.brown;
    #endif
    vec3 cyan = RGB255(0, 255, 255);
    vec3 magenta = RGB255(255, 0, 255);
    vec3 yellow = RGB255(255, 255, 0);
    vec3 black = RGB255(0, 0, 0);

    vec3 white = RGB255(255, 255, 255);
    vec3 light_gray = RGB255(160, 160, 160);
    vec3 gray = RGB255(115, 115, 115);
    vec3 dark_gray = RGB255(70, 70, 70);
    vec3 darker_gray = RGB255(20, 20, 20);

    vec3 dark_yellow = RGB255(200, 200, 0);
} pallete;

vec3 Q_pallete[10] = {
    #if 0
    pallete.green,
    pallete.red,
    pallete.pink,
    pallete.magenta,
    pallete.purple,
    pallete.blue,
    pallete.gray,
    pallete.light_gray, // TODO: what is this
    pallete.cyan,
    pallete.orange,
    #else
    pallete.light_gray,
    pallete.red,
    pallete.orange,
    pallete.yellow,
    pallete.green,
    pallete.blue,
    pallete.purple,
    pallete.brown,
    pallete.dark_gray,
    #endif
};

vec3 get_accent_color(ToolboxGroup group) {
    vec3 result;
    if (group == ToolboxGroup::Draw) {
        result = V3(0.5f, 1.0f, 1.0f);
    } else if (group == ToolboxGroup::Both) {
        result = V3(0.75f, 1.0f, 0.75f);
    } else if (group == ToolboxGroup::Mesh) {
        result = V3(1.0f, 1.0f, 0.5f);
    } else if (group == ToolboxGroup::Snap) {
        result = V3(1.0f, 0.5f, 1.0f);
    } else if (group == ToolboxGroup::Xsel) {
        result = V3(0.75f, 0.75f, 1.0f);
    } else if (group == ToolboxGroup::Colo) {
        result = pallete.white;
    } else { ASSERT(group == ToolboxGroup::None);
        result = {};
    }
    return result;
}


////////////////////////////////////////
// Config-Tweaks ///////////////////////
////////////////////////////////////////

real Z_FIGHT_EPS = 0.05f;
real TOLERANCE_DEFAULT = 5e-4f;
uint NUM_SEGMENTS_PER_CIRCLE = 64;
real GRID_SIDE_LENGTH = 240.0f;
real GRID_SPACING = 10.0f;
real CAMERA_3D_PERSPECTIVE_ANGLE_OF_VIEW = RAD(45.0f);

////////////////////////////////////////
// Cow Additions ///////////////////////
////////////////////////////////////////

real WRAP_TO_0_TAU_INTERVAL(real theta) {
    theta = fmod(theta, TAU);
    if (theta < 0.0) theta += TAU;
    return theta;
}

real _WRAP_TO_0_360_INTERVAL(real theta_in_degrees) {
    theta_in_degrees = fmod(theta_in_degrees, 360.0f);
    if (theta_in_degrees < 0.0) theta_in_degrees += 360.0f;
    return theta_in_degrees;
}

bool ANGLE_IS_BETWEEN_CCW(real t, real a, real b) {
    return (WRAP_TO_0_TAU_INTERVAL(t - a) < WRAP_TO_0_TAU_INTERVAL(t - b));
}

bool ANGLE_IS_BETWEEN_CCW_DEGREES(real t, real a, real b) {
    return (WRAP_TO_0_TAU_INTERVAL(RAD(t) - RAD(a)) < WRAP_TO_0_TAU_INTERVAL(RAD(t) - RAD(b)));
}

bool ANGLE_IS_BETWEEN_CCW_DEGREES_TIGHT(real t, real a, real b) {
    return ANGLE_IS_BETWEEN_CCW_DEGREES(t, a + 100 * TINY_VAL, b - 100 * TINY_VAL);
}
////////////////////////////////////////
// List<Entity> /////////////////////////////////
////////////////////////////////////////

vec2 get_point_on_circle_NOTE_pass_angle_in_radians(vec2 center, real radius, real angle_in_radians) {
    return center + radius * V2(COS(angle_in_radians), SIN(angle_in_radians));
}

// NOTE: this is real gross
void arc_process_angles_into_lerpable_radians_considering_flip_flag(ArcEntity *arc, real *start_angle, real *end_angle, bool flip_flag) {
    // The way the List<Entity> spec works is that start_angle and end_angle define points on the circle
    // which are connected counterclockwise from start to end with an arc
    // (start -ccw-> end)
    //
    // To flip an arc entity, we need to go B -cw-> A
    *start_angle = WRAP_TO_0_TAU_INTERVAL(RAD(arc->start_angle_in_degrees));
    *end_angle = WRAP_TO_0_TAU_INTERVAL(RAD(arc->end_angle_in_degrees));
    if (*end_angle < *start_angle) *end_angle += TAU;
    if (flip_flag) { // swap
        real tmp = *start_angle;
        *start_angle = *end_angle;
        *end_angle = tmp;
    }
}

real entity_length(Entity *entity) {
    if (entity->type == EntityType::Line) {
        LineEntity *line = &entity->line;
        return norm(line->start - line->end);
    } else if (entity->type == EntityType::Arc) {
        ArcEntity *arc = &entity->arc;
        real start_angle;
        real end_angle;
        arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, false);
        return ABS(start_angle - end_angle) * arc->radius;
    } else { ASSERT(entity->type == EntityType::Circle);
        return PI * entity->circle.radius * 2;
    }
}

vec2 entity_get_start_point(Entity *entity) {
    if (entity->type == EntityType::Line) {
        LineEntity *line = &entity->line;
        return line->start;
    } else { ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc = &entity->arc;
        return get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, RAD(arc->start_angle_in_degrees));
    }
}

vec2 entity_get_end_point(Entity *entity) {
    if (entity->type == EntityType::Line) {
        LineEntity *line = &entity->line;
        return line->end;
    } else { ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc = &entity->arc;
        return get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, RAD(arc->end_angle_in_degrees));
    }
}

void entity_get_start_and_end_points(Entity *entity, vec2 *start, vec2 *end) {
    *start = entity_get_start_point(entity);
    *end = entity_get_end_point(entity);
}

vec2 entity_lerp_considering_flip_flag(Entity *entity, real t, bool flip_flag) {
    ASSERT(IS_BETWEEN_LOOSE(t, 0.0f, 1.0f));
    if (entity->type == EntityType::Line) {
        LineEntity *line = &entity->line;
        if (flip_flag) t = 1.0f - t; // FORNOW
        return LERP(t, line->start, line->end);
    } else {
        ASSERT(entity->type == EntityType::Arc);
        ArcEntity *arc = &entity->arc;
        real angle; {
            real start_angle, end_angle;
            arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, flip_flag); // FORNOW
            angle = LERP(t, start_angle, end_angle);
        }
        return get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, angle);
    }
}

vec2 entity_get_middle(Entity *entity) {
    return entity_lerp_considering_flip_flag(entity, 0.5f, false);
}

// struct List<Entity> {
//     uint num_entities;
//     Entity *entities;
// };


vec3 get_color(ColorCode color_code) {
    uint i = uint(color_code);
    if (0 <= i && i <= 9) {
        return Q_pallete[i];
    } else if (20 <= i && i <= 29) {
        do_once { messagef(pallete.orange, "WARNING: slits not implemented"); };
        return Q_pallete[i - 20];
    } else if (color_code == ColorCode::Selection) {
        return pallete.white;
    } else if (color_code == ColorCode::Emphasis) {
        return pallete.white;
    } else {
        ASSERT(false);
        return {};
    }
}

void eso_entity__SOUP_LINES(Entity *entity) {
    if (entity->type == EntityType::Line) {
        LineEntity *line = &entity->line;
        eso_vertex(line->start);
        eso_vertex(line->end);
    } else if (entity->type == EntityType::Arc) {
        ArcEntity *arc = &entity->arc;
        real start_angle, end_angle;
        arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, false);
        real delta_angle = end_angle - start_angle;
        uint num_segments = uint(1 + (delta_angle / TAU) * 64); // FORNOW: TODO: make dependent on zoom
        real increment = delta_angle / num_segments;
        real current_angle = start_angle;
        for_(i, num_segments) {
            eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, current_angle));
            current_angle += increment;
            eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, current_angle));
        }
    } else { ASSERT(entity->type == EntityType::Circle);
        CircleEntity *circle = &entity->circle;
        uint num_segments = 64;
        real current_angle = 0.0;
        real increment = TAU / num_segments;
        for_(i, num_segments) {
            eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, current_angle));
            current_angle += increment;
            eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, current_angle));
        }
    }
}


void entities_debug_draw(Camera *camera_drawing, List<Entity> *entities) {
    eso_begin(camera_drawing->get_PV(), SOUP_LINES);
    for (Entity *entity = entities->array; entity < &entities->array[entities->length]; ++entity) {
        eso_entity__SOUP_LINES(entity);
    }
    eso_end();
}

bbox2 entity_get_bbox(Entity *entity) {
    // special case
    if (entity->type == EntityType::Circle) {
        CircleEntity *circle = &entity->circle;
        return
        {
            circle->center - V2(circle->radius),
                circle->center + V2(circle->radius)
        };
    }


    bbox2 result = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
    vec2 s[2];
    uint n = 2;
    entity_get_start_and_end_points(entity, &s[0], &s[1]);
    for_(i, n) {
        for_(d, 2) {
            result.min[d] = MIN(result.min[d], s[i][d]);
            result.max[d] = MAX(result.max[d], s[i][d]);
        }
    }
    if (entity->type == EntityType::Arc) {
        ArcEntity *arc = &entity->arc;
        // NOTE: endpoints already taken are of; we just have to deal with the quads (if they exist)
        // TODO: angle_is_between_counter_clockwise (TODO TODO TODO)
        real start_angle = RAD(arc->start_angle_in_degrees);
        real end_angle = RAD(arc->end_angle_in_degrees);
        if (ANGLE_IS_BETWEEN_CCW(RAD(  0.0f), start_angle, end_angle)) result.max[0] = MAX(result.max[0], arc->center.x + arc->radius);
        if (ANGLE_IS_BETWEEN_CCW(RAD( 90.0f), start_angle, end_angle)) result.max[1] = MAX(result.max[1], arc->center.y + arc->radius);
        if (ANGLE_IS_BETWEEN_CCW(RAD(180.0f), start_angle, end_angle)) result.min[0] = MIN(result.min[0], arc->center.x - arc->radius);
        if (ANGLE_IS_BETWEEN_CCW(RAD(270.0f), start_angle, end_angle)) result.min[1] = MIN(result.min[1], arc->center.y - arc->radius);
    }
    return result;
}

bbox2 entities_get_bbox(List<Entity> *entities, bool only_consider_selected_entities = false) {
    bbox2 result = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
    for_(i, entities->length) {
        if ((only_consider_selected_entities) && (!entities->array[i].is_selected)) continue;
        bbox2 bbox = entity_get_bbox(&entities->array[i]);
        result += bbox;
    }
    return result;
}

////////////////////////////////////////
// SquaredDistance /////////////////////
////////////////////////////////////////

real squared_distance_point_line_segment(vec2 p, vec2 start, vec2 end) {
    real l2 = squaredDistance(start, end);
    if (l2 < TINY_VAL) return squaredDistance(p, start);
    real num = dot(p - start, end - start);
    vec2 q = CLAMPED_LERP(num / l2, start, end);//
    return squaredDistance(p, q);
}

real squared_distance_point_circle(vec2 p, vec2 center, real radius) {
    return POW(distance(p, center) - radius, 2);
}

real squared_distance_point_arc_NOTE_pass_angles_in_radians(vec2 p, vec2 center, real radius, real start_angle_in_radians, real end_angle_in_radians) {
    bool point_in_sector = ANGLE_IS_BETWEEN_CCW(angle_from_0_TAU(center, p), start_angle_in_radians, end_angle_in_radians);
    if (point_in_sector) {
        return squared_distance_point_circle(p, center, radius);
    } else {
        vec2 start = get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, start_angle_in_radians);
        vec2 end = get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, end_angle_in_radians);
        return MIN(squaredDistance(p, start), squaredDistance(p, end));
    }
}

real squared_distance_point_dxf_line_entity(vec2 p, LineEntity *line) {
    return squared_distance_point_line_segment(p, line->start, line->end);
}

real squared_distance_point_dxf_arc_entity(vec2 p, ArcEntity *arc) {
    return squared_distance_point_arc_NOTE_pass_angles_in_radians(p, arc->center, arc->radius, RAD(arc->start_angle_in_degrees), RAD(arc->end_angle_in_degrees));
}

real squared_distance_point_dxf_circle_entity(vec2 p, CircleEntity *circle) {
    return squared_distance_point_circle(p, circle->center, circle->radius);
}

real squared_distance_point_entity(vec2 p, Entity *entity) {
    if (entity->type == EntityType::Line) {
        LineEntity *line = &entity->line;
        return squared_distance_point_dxf_line_entity(p, line);
    } else if (entity->type == EntityType::Arc) {
        ArcEntity *arc = &entity->arc;
        return squared_distance_point_dxf_arc_entity(p, arc);
    } else { ASSERT(entity->type == EntityType::Circle);
        CircleEntity *circle = &entity->circle;
        return squared_distance_point_dxf_circle_entity(p, circle);
    }
}

real squared_distance_point_dxf(vec2 p, List<Entity> *entities) {
    real result = HUGE_VAL;
    for (Entity *entity = entities->array; entity < &entities->array[entities->length]; ++entity) {
        result = MIN(result, squared_distance_point_entity(p, entity));
    }
    return result;
}

struct DXFFindClosestEntityResult {
    bool success;
    // uint index;
    Entity *closest_entity;
    vec2 line_nearest_point;
    real arc_nearest_angle_in_degrees;
    // TODO: t
};
DXFFindClosestEntityResult dxf_find_closest_entity(List<Entity> *entities, vec2 p) {
    DXFFindClosestEntityResult result = {};
    double hot_squared_distance = HUGE_VAL;
    for (Entity *entity = entities->array; entity < entities->array + entities->length; ++entity) {
        double squared_distance = squared_distance_point_entity(p, entity);
        if (squared_distance < hot_squared_distance) {
            hot_squared_distance = squared_distance;
            result.success = true;
            result.closest_entity = entity;
            if (result.closest_entity->type == EntityType::Line) {
                LineEntity *line = &result.closest_entity->line;
                real l2 = squaredDistance(line->start, line->end);
                if (l2 < TINY_VAL) {
                    result.line_nearest_point = line->start;
                } else {
                    real num = dot(p - line->start, line->end - line->start);
                    result.line_nearest_point = CLAMPED_LERP(num / l2, line->start, line->end);
                }
            } else if (result.closest_entity->type == EntityType::Arc) {
                ArcEntity *arc = &result.closest_entity->arc;
                result.arc_nearest_angle_in_degrees = DEG(ATAN2(p - arc->center));
            } else { ASSERT(result.closest_entity->type == EntityType::Circle);
                // CircleEntity *circle = &result.closest_entity->circle;
                // result.arc_nearest_angle_in_degrees = DEG(ATAN2(p - circle->center));
            }
        }
    }
    return result;
}


////////////////////////////////////////
// LoopAnalysis ////////////////////////
////////////////////////////////////////

struct DXFEntityIndexAndFlipFlag {
    uint entity_index;
    bool flip_flag;
};

struct DXFLoopAnalysisResult {
    uint num_loops;
    uint *num_entities_in_loops;
    DXFEntityIndexAndFlipFlag **loops;
    uint *loop_index_from_entity_index;
};

DXFLoopAnalysisResult dxf_loop_analysis_create_FORNOW_QUADRATIC(List<Entity> *entities, bool only_consider_selected_entities) {
    if (entities->length == 0) {
        DXFLoopAnalysisResult result = {};
        result.num_loops = 0;
        result.num_entities_in_loops = (uint *) calloc(result.num_loops, sizeof(uint));
        result.loops = (DXFEntityIndexAndFlipFlag **) calloc(result.num_loops, sizeof(DXFEntityIndexAndFlipFlag *));
        result.loop_index_from_entity_index = (uint *) calloc(entities->length, sizeof(uint));
        return result;
    }

    DXFLoopAnalysisResult result = {};
    { // num_entities_in_loops, loops
      // populate List's
        List<List<DXFEntityIndexAndFlipFlag>> stretchy_list = {}; {
            bool *entity_already_added = (bool *) calloc(entities->length, sizeof(bool));
            while (true) {
                #define MACRO_CANDIDATE_VALID(i) (!entity_already_added[i] && (!only_consider_selected_entities || entities->array[i].is_selected))
                { // seed loop
                    bool added_and_seeded_new_loop = false;
                    for_(entity_index, entities->length) {
                        if (MACRO_CANDIDATE_VALID(entity_index)) {
                            added_and_seeded_new_loop = true;
                            entity_already_added[entity_index] = true;
                            list_push_back(&stretchy_list, {});
                            list_push_back(&stretchy_list.array[stretchy_list.length - 1], { entity_index, false });
                            break;
                        }
                    }
                    if (!added_and_seeded_new_loop) break;
                }

                DXFEntityIndexAndFlipFlag *FORNOW_seed = &stretchy_list.array[stretchy_list.length - 1].array[stretchy_list.array[stretchy_list.length - 1].length - 1];

                if (entities->array[FORNOW_seed->entity_index].type != EntityType::Circle) { // continue and complete
                    real tolerance = TOLERANCE_DEFAULT;
                    while (true) {
                        bool added_new_entity_to_loop = false;
                        for_(entity_index, entities->length) {
                            if (entities->array[entity_index].type == EntityType::Circle) continue;
                            if (!MACRO_CANDIDATE_VALID(entity_index)) continue;
                            vec2 start_prev;
                            vec2 end_prev;
                            vec2 start_i;
                            vec2 end_i;
                            DXFEntityIndexAndFlipFlag *prev_entity_index_and_flip_flag = &(stretchy_list.array[stretchy_list.length - 1].array[stretchy_list.array[stretchy_list.length - 1].length - 1]);
                            {
                                entity_get_start_and_end_points(&entities->array[prev_entity_index_and_flip_flag->entity_index], &start_prev, &end_prev);
                                entity_get_start_and_end_points(&entities->array[entity_index], &start_i, &end_i);
                            }
                            bool is_next_entity = false;
                            bool flip_flag = false;
                            if (!prev_entity_index_and_flip_flag->flip_flag) {
                                if (squaredDistance(end_prev, start_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = false;
                                } else if (squaredDistance(end_prev, end_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = true;
                                }
                            } else {
                                if (squaredDistance(start_prev, start_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = false;
                                } else if (squaredDistance(start_prev, end_i) < tolerance) {
                                    is_next_entity = true;
                                    flip_flag = true;
                                }
                            }
                            if (is_next_entity) {
                                added_new_entity_to_loop = true;
                                entity_already_added[entity_index] = true;
                                list_push_back(&stretchy_list.array[stretchy_list.length - 1], { entity_index, flip_flag });
                                break;
                            }
                        }
                        if (!added_new_entity_to_loop) break;
                    }

                    { // reverse_loop if necessary
                        uint num_entities_in_loop = stretchy_list.array[stretchy_list.length - 1].length;
                        DXFEntityIndexAndFlipFlag *loop = stretchy_list.array[stretchy_list.length - 1].array;
                        bool reverse_loop; {
                            #if 0
                            reverse_loop = false;
                            #else
                            real twice_the_signed_area; {
                                twice_the_signed_area = 0.0f;
                                for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities_in_loop; ++entity_index_and_flip_flag) {
                                    uint entity_index = entity_index_and_flip_flag->entity_index;
                                    bool flip_flag = entity_index_and_flip_flag->flip_flag;
                                    Entity *entity = &entities->array[entity_index];
                                    if (entity->type == EntityType::Line) {
                                        LineEntity *line = &entity->line;
                                        // shoelace-type formula
                                        twice_the_signed_area += ((flip_flag) ? -1 : 1) * (line->start.x * line->end.y - line->end.x * line->start.y);
                                    } else {
                                        ASSERT(entity->type == EntityType::Arc);
                                        ArcEntity *arc = &entity->arc;
                                        // "Circular approximation using polygons"
                                        // - n = 2 (area-preserving approximation of arc with two segments)
                                        real start_angle, end_angle;
                                        arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, flip_flag);
                                        vec2 start = get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, start_angle);
                                        vec2 end = get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, end_angle);
                                        real mid_angle = (start_angle + end_angle) / 2;
                                        real d; {
                                            real alpha = ABS(start_angle - end_angle) / 2;
                                            d = arc->radius * alpha / SIN(alpha);
                                        }
                                        vec2 mid = get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, d, mid_angle);
                                        twice_the_signed_area += mid.x * (end.y - start.y) + mid.y * (start.x - end.x); // TODO cross(...)
                                    }
                                }
                            }
                            reverse_loop = (twice_the_signed_area < 0.0f);
                            #endif
                        }
                        if (reverse_loop) {
                            for (uint i = 0, j = (num_entities_in_loop - 1); i < j; ++i, --j) {
                                DXFEntityIndexAndFlipFlag tmp = loop[i];
                                loop[i] = loop[j];
                                loop[j] = tmp;
                            }
                            for_(i, num_entities_in_loop) {
                                loop[i].flip_flag = !loop[i].flip_flag;
                            }
                        }
                    }
                }
                #undef MACRO_CANDIDATE_VALID
            }
            free(entity_already_added);
        }

        // copy over from List's
        result.num_loops = stretchy_list.length;
        result.num_entities_in_loops = (uint *) calloc(result.num_loops, sizeof(uint));
        result.loops = (DXFEntityIndexAndFlipFlag **) calloc(result.num_loops, sizeof(DXFEntityIndexAndFlipFlag *));
        for_(i, result.num_loops) {
            result.num_entities_in_loops[i] = stretchy_list.array[i].length;
            result.loops[i] = (DXFEntityIndexAndFlipFlag *) calloc(result.num_entities_in_loops[i], sizeof(DXFEntityIndexAndFlipFlag));
            memcpy(result.loops[i], stretchy_list.array[i].array, result.num_entities_in_loops[i] * sizeof(DXFEntityIndexAndFlipFlag));
        }

        // free List's
        for_(i, stretchy_list.length) list_free_AND_zero(&stretchy_list.array[i]);
        list_free_AND_zero(&stretchy_list);
    }
    // loop_index_from_entity_index (brute force)
    result.loop_index_from_entity_index = (uint *) calloc(entities->length, sizeof(uint));
    for_(i, entities->length) {
        for_(j, result.num_loops) {
            for_(k, result.num_entities_in_loops[j]) {
                if (i == result.loops[j][k].entity_index) {
                    result.loop_index_from_entity_index[i] = j;
                    break;
                }
            }
        }
    }
    return result;
}

void dxf_loop_analysis_free(DXFLoopAnalysisResult *analysis) {
    if (analysis->num_entities_in_loops) {
        free(analysis->num_entities_in_loops);
    }
    if (analysis->loops) {
        for_(i, analysis->num_loops) {
            free(analysis->loops[i]);
        }
        free(analysis->loops);
    }
    if (analysis->loop_index_from_entity_index) {
        free(analysis->loop_index_from_entity_index);
    }
    *analysis = {};
}

////////////////////////////////////////
// LoopAnalysis ////////////////////////
////////////////////////////////////////

struct CrossSectionEvenOdd {
    uint num_polygonal_loops;
    uint *num_vertices_in_polygonal_loops;
    vec2 **polygonal_loops;
};

CrossSectionEvenOdd cross_section_create_FORNOW_QUADRATIC(List<Entity> *entities, bool only_consider_selected_entities) {
    #if 0
    {
        FORNOW_UNUSED(entities);
        FORNOW_UNUSED(include);
        CrossSectionEvenOdd result = {};
        result.num_polygonal_loops = 2;
        result.num_vertices_in_polygonal_loops = (uint *) calloc(result.num_polygonal_loops, sizeof(uint));
        result.num_vertices_in_polygonal_loops[0] = 4;
        result.num_vertices_in_polygonal_loops[1] = 6;
        result.polygonal_loops = (vec2 **) calloc(result.num_polygonal_loops, sizeof(vec2 *));
        result.polygonal_loops[0] = (vec2 *) calloc(result.num_vertices_in_polygonal_loops[0], sizeof(vec2));
        result.polygonal_loops[1] = (vec2 *) calloc(result.num_vertices_in_polygonal_loops[1], sizeof(vec2));
        result.polygonal_loops[0][0] = { -2.0f, -2.0f };
        result.polygonal_loops[0][1] = {  2.0f, -2.0f };
        result.polygonal_loops[0][2] = {  2.0f,  2.0f };
        result.polygonal_loops[0][3] = { -2.0f,  2.0f };
        result.polygonal_loops[1][0] = { COS(RAD(  0)), SIN(RAD(  0)) };
        result.polygonal_loops[1][1] = { COS(RAD( 60)), SIN(RAD( 60)) };
        result.polygonal_loops[1][2] = { COS(RAD(120)), SIN(RAD(120)) };
        result.polygonal_loops[1][3] = { COS(RAD(180)), SIN(RAD(180)) };
        result.polygonal_loops[1][4] = { COS(RAD(240)), SIN(RAD(240)) };
        result.polygonal_loops[1][5] = { COS(RAD(300)), SIN(RAD(300)) };
        return result;
    }
    #endif
    // populate List's
    List<List<vec2>> stretchy_list = {}; {
        DXFLoopAnalysisResult analysis = dxf_loop_analysis_create_FORNOW_QUADRATIC(entities, only_consider_selected_entities);
        for_(loop_index, analysis.num_loops) {
            uint num_entities_in_loop = analysis.num_entities_in_loops[loop_index];
            DXFEntityIndexAndFlipFlag *loop = analysis.loops[loop_index];
            list_push_back(&stretchy_list, {});
            for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < loop + num_entities_in_loop; ++entity_index_and_flip_flag) {
                uint entity_index = entity_index_and_flip_flag->entity_index;
                bool flip_flag = entity_index_and_flip_flag->flip_flag;
                Entity *entity = &entities->array[entity_index];
                if (entity->type == EntityType::Line) {
                    LineEntity *line = &entity->line;
                    if (!flip_flag) {
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], { line->start.x, line->start.y });
                    } else {
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], { line->end.x, line->end.y });
                    }
                } else if (entity->type == EntityType::Arc) {
                    ArcEntity *arc = &entity->arc;
                    real start_angle, end_angle;
                    arc_process_angles_into_lerpable_radians_considering_flip_flag(arc, &start_angle, &end_angle, flip_flag);
                    real delta_angle = end_angle - start_angle;
                    uint num_segments = uint(2 + ABS(delta_angle) * (NUM_SEGMENTS_PER_CIRCLE / TAU)); // FORNOW (2 + ...)
                    real increment = delta_angle / num_segments;
                    real current_angle = start_angle;
                    vec2 p;
                    for_(i, num_segments) {
                        p = get_point_on_circle_NOTE_pass_angle_in_radians(arc->center, arc->radius, current_angle);
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], p);
                        current_angle += increment;
                    }
                } else { ASSERT(entity->type == EntityType::Circle);
                    CircleEntity *circle = &entity->circle;
                    uint num_segments = NUM_SEGMENTS_PER_CIRCLE;
                    for_(i, num_segments) {
                        real angle = real(i) / num_segments * TAU;
                        vec2 p = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle);
                        list_push_back(&stretchy_list.array[stretchy_list.length - 1], p);
                    }
                }
            }
        }
        dxf_loop_analysis_free(&analysis);
    }

    // copy over from List's
    CrossSectionEvenOdd result = {};
    result.num_polygonal_loops = stretchy_list.length;
    result.num_vertices_in_polygonal_loops = (uint *) calloc(result.num_polygonal_loops, sizeof(uint));
    result.polygonal_loops = (vec2 **) calloc(result.num_polygonal_loops, sizeof(vec2 *));
    for_(i, result.num_polygonal_loops) {
        result.num_vertices_in_polygonal_loops[i] = stretchy_list.array[i].length;
        result.polygonal_loops[i] = (vec2 *) calloc(result.num_vertices_in_polygonal_loops[i], sizeof(vec2));
        memcpy(result.polygonal_loops[i], stretchy_list.array[i].array, result.num_vertices_in_polygonal_loops[i] * sizeof(vec2));
    }

    // free List's
    for_(i, stretchy_list.length) list_free_AND_zero(&stretchy_list.array[i]);
    list_free_AND_zero(&stretchy_list);

    return result;
}

void cross_section_debug_draw(Camera *camera_drawing, CrossSectionEvenOdd *cross_section) {
    eso_begin(camera_drawing->get_PV(), SOUP_LINES);
    eso_color(pallete.white);
    for_(loop_index, cross_section->num_polygonal_loops) {
        vec2 *polygonal_loop = cross_section->polygonal_loops[loop_index];
        int n = cross_section->num_vertices_in_polygonal_loops[loop_index];
        for (int j = 0, i = n - 1; j < n; i = j++) {
            real a_x = polygonal_loop[i].x;
            real a_y = polygonal_loop[i].y;
            real b_x = polygonal_loop[j].x;
            real b_y = polygonal_loop[j].y;
            eso_color(color_rainbow_swirl(real(i) / (n)));
            eso_vertex(a_x, a_y);
            eso_color(color_rainbow_swirl(real(i + 1) / (n)));
            eso_vertex(b_x, b_y);

            // draw normal
            real c_x = (a_x + b_x) / 2;
            real c_y = (a_y + b_y) / 2;
            real n_x = b_y - a_y;
            real n_y = a_x - b_x;
            real norm_n = SQRT(n_x * n_x + n_y * n_y);
            real L = 0.013f;
            eso_color(color_rainbow_swirl((i + 0.5f) / (n)));
            eso_vertex(c_x, c_y);
            eso_vertex(c_x + L * n_x / norm_n, c_y + L * n_y / norm_n);
        }
    }
    eso_end();
}

void cross_section_free(CrossSectionEvenOdd *cross_section) {
    free(cross_section->num_vertices_in_polygonal_loops);
    for_(i, cross_section->num_polygonal_loops) free(cross_section->polygonal_loops[i]);
    free(cross_section->polygonal_loops);
}

////////////////////////////////////////
// Mesh, Mesh //////////////////////
////////////////////////////////////////


void mesh_triangle_normals_calculate(Mesh *mesh) {
    mesh->triangle_normals = (vec3 *) malloc(mesh->num_triangles * sizeof(vec3));
    vec3 p[3];
    for_(i, mesh->num_triangles) {
        for_(d, 3) p[d] = mesh->vertex_positions[mesh->triangle_indices[i][d]];
        vec3 n = normalized(cross(p[1] - p[0], p[2] - p[0]));
        mesh->triangle_normals[i] = n;
    }
}

void mesh_cosmetic_edges_calculate(Mesh *mesh) {
    // approach: prep a big array that maps edge -> cwiseProduct of face normals (start it at 1, 1, 1) // (faces that edge is part of)
    //           iterate through all edges detministically (ccw in order, flipping as needed so lower_index->higher_index)
    //           then go back and if passes some heuristic add that index to a stretchy buffer
    List<uint2> list = {}; {
        Map<uint2, vec3> map = {}; {
            for_(i, mesh->num_triangles) {
                vec3 n = mesh->triangle_normals[i];
                for (uint jj0 = 0, jj1 = (3 - 1); jj0 < 3; jj1 = jj0++) {
                    uint j0 = mesh->triangle_indices[i][jj0];
                    uint j1 = mesh->triangle_indices[i][jj1];
                    if (j0 > j1) {
                        uint tmp = j0;
                        j0 = j1;
                        j1 = tmp;
                    }
                    uint2 key = { j0, j1 };
                    map_put(&map, key, cwiseProduct(n, map_get(&map, key, V3(1.0f))));
                }
            }
        }
        {
            for (List<Pair<uint2, vec3>> *bucket = map.buckets; bucket < &map.buckets[map.num_buckets]; ++bucket) {
                for (Pair<uint2, vec3> *pair = bucket->array; pair < &bucket->array[bucket->length]; ++pair) {
                    vec3 n2 = pair->value;
                    // pprint(n2);
                    real angle = DEG(acos(n2.x + n2.y + n2.z)); // [0.0f, 180.0f]
                    if (angle > 30.0f) {
                        list_push_back(&list, pair->key); // FORNOW
                    }
                }
            }
        }
        map_free_and_zero(&map);
    }
    {
        mesh->num_cosmetic_edges = list.length;
        mesh->cosmetic_edges = (uint2 *) calloc(mesh->num_cosmetic_edges, sizeof(uint2));
        memcpy(mesh->cosmetic_edges, list.array, mesh->num_cosmetic_edges * sizeof(uint2)); 
    }
    list_free_AND_zero(&list);
}

void mesh_bbox_calculate(Mesh *mesh) {
    mesh->bbox = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<3>();
    for_(i, mesh->num_vertices) {
        mesh->bbox += mesh->vertex_positions[i];
    }
}

void mesh_free_AND_zero(Mesh *mesh) {
    if (mesh->vertex_positions) free(mesh->vertex_positions);
    if (mesh->triangle_indices) free(mesh->triangle_indices);
    if (mesh->triangle_normals) free(mesh->triangle_normals);
    if (mesh->cosmetic_edges)   free(mesh->cosmetic_edges);
    *mesh = {};
}

void mesh_deep_copy(Mesh *dst, Mesh *src) {
    *dst = *src;
    if (src->vertex_positions) {
        uint size = src->num_vertices * sizeof(vec3);
        dst->vertex_positions = (vec3 *) malloc(size);
        memcpy(dst->vertex_positions, src->vertex_positions, size);
    }
    if (src->triangle_indices) {
        uint size = src->num_triangles * sizeof(uint3);
        dst->triangle_indices = (uint3 *) malloc(size);
        memcpy(dst->triangle_indices, src->triangle_indices, size);
    }
    if (src->triangle_normals) {
        uint size = src->num_triangles * sizeof(vec3); 
        dst->triangle_normals = (vec3 *) malloc(size);
        memcpy(dst->triangle_normals, src->triangle_normals, size);
    }
    if (src->cosmetic_edges) {
        uint size = src->num_cosmetic_edges * sizeof(uint2);
        dst->cosmetic_edges = (uint2 *) malloc(size);
        memcpy(dst->cosmetic_edges, src->cosmetic_edges, size);
    }
}

////////////////////////////////////////////////////////////////////////////////
// key_lambda //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool _key_lambda(KeyEvent *key_event, uint key, bool control = false, bool shift = false, bool alt = false) {
    ASSERT(!(('a' <= key) && (key <= 'z')));
    bool key_match = (key_event->key == key);
    bool super_match = ((key_event->control && control) || (!key_event->control && !control)); // * bool
    bool shift_match = ((key_event->shift && shift) || (!key_event->shift && !shift)); // * bool
    bool alt_match = ((key_event->alt && alt) || (!key_event->alt && !alt)); // * bool
    return (key_match && super_match && shift_match && alt_match);
};

////////////////////////////////////////////////////////////////////////////////
// world_state /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void world_state_deep_copy(WorldState_ChangesToThisMustBeRecorded_state *dst, WorldState_ChangesToThisMustBeRecorded_state *src) {
    *dst = *src;
    dst->drawing.entities = {};
    list_clone(&dst->drawing.entities,    &src->drawing.entities   );
    mesh_deep_copy(&dst->mesh, &src->mesh);
}

void world_state_free_AND_zero(WorldState_ChangesToThisMustBeRecorded_state *world_state) {
    mesh_free_AND_zero(&world_state->mesh);
    list_free_AND_zero(&world_state->drawing.entities);
}

////////////////////////////////////////////////////////////////////////////////
// uh oh ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// TODO: don't overwrite fancy mesh, let the calling code do what it will
// TODO: could this take a printf function pointer?
Mesh wrapper_manifold(
        Mesh *mesh, // dest__NOTE_GETS_OVERWRITTEN,
        uint num_polygonal_loops,
        uint *num_vertices_in_polygonal_loops,
        vec2 **polygonal_loops,
        mat4 M_3D_from_2D,
        Command Mesh_command,
        real out_quantity,
        real in_quantity,
        vec2 dxf_origin,
        vec2 dxf_axis_base_point,
        real dxf_axis_angle_from_y
        ) {


    bool add = (command_equals(Mesh_command, commands.ExtrudeAdd)) || (command_equals(Mesh_command, commands.RevolveAdd));
    bool cut = (command_equals(Mesh_command, commands.ExtrudeCut)) || (command_equals(Mesh_command, commands.RevolveCut));
    bool extrude = (command_equals(Mesh_command, commands.ExtrudeAdd)) || (command_equals(Mesh_command, commands.ExtrudeCut));
    bool revolve = (command_equals(Mesh_command, commands.RevolveAdd)) || (command_equals(Mesh_command, commands.RevolveCut));
    ASSERT(add || cut);
    ASSERT(extrude || revolve);

    ManifoldManifold *manifold_A; {
        if (mesh->num_vertices == 0) {
            manifold_A = NULL;
        } else { // manifold <- mesh
            ManifoldMeshGL *meshgl = manifold_meshgl(
                    malloc(manifold_meshgl_size()),
                    (real *) mesh->vertex_positions,
                    mesh->num_vertices,
                    3,
                    (uint *) mesh->triangle_indices,
                    mesh->num_triangles);

            manifold_A = manifold_of_meshgl(malloc(manifold_manifold_size()), meshgl);

            manifold_delete_meshgl(meshgl);
        }
    }

    ManifoldManifold *manifold_B; {
        ManifoldSimplePolygon **simple_polygon_array; {
            simple_polygon_array = (ManifoldSimplePolygon **) malloc(num_polygonal_loops * sizeof(ManifoldSimplePolygon *));
            for_(i, num_polygonal_loops) {
                simple_polygon_array[i] = manifold_simple_polygon(malloc(manifold_simple_polygon_size()), (ManifoldVec2 *) polygonal_loops[i], num_vertices_in_polygonal_loops[i]);
            }
        } 

        ManifoldPolygons *_polygons; {
            _polygons = manifold_polygons(malloc(manifold_polygons_size()), simple_polygon_array, num_polygonal_loops);
        }

        ManifoldCrossSection *cross_section; {
            cross_section = manifold_cross_section_of_polygons(malloc(manifold_cross_section_size()), _polygons, ManifoldFillRule::MANIFOLD_FILL_RULE_EVEN_ODD);
            // cross_section = manifold_cross_section_translate(cross_section, cross_section, -dxf_origin.x, -dxf_origin.y);

            if (revolve) {
                manifold_cross_section_translate(cross_section, cross_section, -dxf_axis_base_point.x, -dxf_axis_base_point.y);
                manifold_cross_section_rotate(cross_section, cross_section, DEG(-dxf_axis_angle_from_y)); // * has both the 90 y-up correction and the angle
            }
        }

        ManifoldPolygons *polygons = manifold_cross_section_to_polygons(malloc(manifold_polygons_size()), cross_section);



        { // manifold_B
            if (command_equals(Mesh_command, commands.ExtrudeCut)) {
                do_once { messagef(pallete.red, "FORNOW ExtrudeCut: Inflating as naive solution to avoid thin geometry."); };
                in_quantity += SGN(in_quantity) * TOLERANCE_DEFAULT;
                out_quantity += SGN(out_quantity) * TOLERANCE_DEFAULT;
            }

            // NOTE: params are arbitrary sign (and can be same sign)--a typical thing would be like (30, -30)
            //       but we support (30, 40) -- which is equivalent to (40, 0)

            if (extrude) {
                real length = in_quantity + out_quantity;
                manifold_B = manifold_extrude(malloc(manifold_manifold_size()), polygons, length, 0, 0.0f, 1.0f, 1.0f);
                manifold_B = manifold_translate(manifold_B, manifold_B, 0.0f, 0.0f, -in_quantity);
            } else { ASSERT(revolve);
                // TODO: M_3D_from_2D 
                real angle_in_degrees = in_quantity + out_quantity;
                manifold_B = manifold_revolve(malloc(manifold_manifold_size()), polygons, NUM_SEGMENTS_PER_CIRCLE, angle_in_degrees);
                manifold_B = manifold_rotate(manifold_B, manifold_B, 0.0, 0.0, -out_quantity); // *
                manifold_B = manifold_rotate(manifold_B, manifold_B, 0.0, DEG(-dxf_axis_angle_from_y), 0.0f); // *
                manifold_B = manifold_rotate(manifold_B, manifold_B, -90.0f, 0.0f, 0.0f);
                manifold_B = manifold_translate(manifold_B, manifold_B, dxf_axis_base_point.x, dxf_axis_base_point.y, 0.0f);
            }
            manifold_B = manifold_translate(manifold_B, manifold_B, -dxf_origin.x, -dxf_origin.y, 0.0f);
            manifold_B = manifold_transform(manifold_B, manifold_B,
                    M_3D_from_2D(0, 0), M_3D_from_2D(1, 0), M_3D_from_2D(2, 0),
                    M_3D_from_2D(0, 1), M_3D_from_2D(1, 1), M_3D_from_2D(2, 1),
                    M_3D_from_2D(0, 2), M_3D_from_2D(1, 2), M_3D_from_2D(2, 2),
                    M_3D_from_2D(0, 3), M_3D_from_2D(1, 3), M_3D_from_2D(2, 3));
        }

        { // free(simple_polygon_array)
            for_(i, num_polygonal_loops) manifold_delete_simple_polygon(simple_polygon_array[i]);
            free(simple_polygon_array);
        }
        manifold_delete_polygons(polygons);
        manifold_delete_cross_section(cross_section);
        manifold_delete_polygons(_polygons);
    }

    Mesh result; { // C <- f(A, B)
        ManifoldMeshGL *meshgl; {
            ManifoldManifold *manifold_C;
            if (manifold_A == NULL) {
                ASSERT(!cut);
                manifold_C = manifold_B;
            } else {
                // TODO: ? manifold_delete_manifold(manifold_A);
                manifold_C =
                    manifold_boolean(
                            malloc(manifold_manifold_size()),
                            manifold_A,
                            manifold_B,
                            (add) ? ManifoldOpType::MANIFOLD_ADD : ManifoldOpType::MANIFOLD_SUBTRACT
                            );
                manifold_delete_manifold(manifold_A);
                manifold_delete_manifold(manifold_B);
            }

            meshgl = manifold_get_meshgl(malloc(manifold_meshgl_size()), manifold_C);
        }

        { // result <- meshgl
            result = {};
            result.num_vertices = manifold_meshgl_num_vert(meshgl);
            result.num_triangles = manifold_meshgl_num_tri(meshgl);
            result.vertex_positions = (vec3 *) manifold_meshgl_vert_properties(malloc(manifold_meshgl_vert_properties_length(meshgl) * sizeof(real)), meshgl);
            result.triangle_indices = (uint3 *) manifold_meshgl_tri_verts(malloc(manifold_meshgl_tri_length(meshgl) * sizeof(uint)), meshgl);
            mesh_triangle_normals_calculate(&result);
            mesh_cosmetic_edges_calculate(&result);
            mesh_bbox_calculate(&result);
        }

        manifold_delete_meshgl(meshgl);
    }

    return result;
}

char *key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(KeyEvent *key_event) { // inline
    static char buffer[256];

    char *_ctrl_plus; {
        if (!key_event->control) {
            _ctrl_plus = "";
        } else {
            _ctrl_plus = "CTRL+";
        }
    }

    char *_shift_plus; {
        if (!key_event->shift) {
            _shift_plus = "";
        } else {
            _shift_plus = "SHIFT+";
        }
    }

    char *_alt_plus; {
        if (!key_event->alt) {
            _alt_plus = "";
        } else {
            _alt_plus = "ALT+";
        }
    }

    char _key_buffer[2];
    char *_key; {
        if (0) ;
        else if (key_event->key == '\0') _key = "";
        else if (key_event->key == GLFW_KEY_BACKSPACE) _key = "BACKSPACE";
        else if (key_event->key == GLFW_KEY_DELETE) _key = "DELETE";
        else if (key_event->key == GLFW_KEY_ENTER) _key = "ENTER";
        else if (key_event->key == GLFW_KEY_ESCAPE) _key = "ESCAPE";
        else if (key_event->key == GLFW_KEY_LEFT) _key = "LEFT";
        else if (key_event->key == GLFW_KEY_RIGHT) _key = "RIGHT";
        else if (key_event->key == GLFW_KEY_SPACE) _key = "SPACE";
        else if (key_event->key == GLFW_KEY_TAB) _key = "TAB";
        else if (key_event->key == GLFW_KEY_UP) _key = "UP";
        else if (key_event->key == GLFW_KEY_DOWN) _key = "DOWN";
        else if (key_event->key == GLFW_KEY_PAGE_UP) _key = "PAGE_UP";
        else if (key_event->key == GLFW_KEY_PAGE_DOWN) _key = "PAGE_DOWN";
        else if (key_event->key == GLFW_KEY_HOME) _key = "HOME";
        else if (key_event->key == GLFW_KEY_END) _key = "END";
        else {
            _key_buffer[0] = (char) key_event->key;
            _key_buffer[1] = '\0';
            _key = _key_buffer;
        }
    }

    sprintf(buffer, "%s%s%s%s", _ctrl_plus, _shift_plus, _alt_plus, _key);
    return buffer;
}

////////////////////////////////////////
// intersection ////////////////////////
////////////////////////////////////////


struct LineLineXResult {
    vec2 point;
    real t_ab;
    real t_cd;
    bool point_is_on_segment_ab;
    bool point_is_on_segment_cd;
    bool lines_are_parallel;
};


LineLineXResult line_line_intersection(//vec2 a, vec2 b, vec2 c, vec2 d) {
vec2 p, vec2 p_plus_r, vec2 q, vec2 q_plus_s) {
    // https://stackoverflow.com/a/565282
    vec2 r = p_plus_r - p;
    vec2 s = q_plus_s - q;
    real r_cross_s = cross(r, s);

    LineLineXResult result = {};
    result.lines_are_parallel = ABS(r_cross_s) < 0.0001;
    if (result.lines_are_parallel) {
    } else {
        vec2 q_minus_p = q - p;
        result.t_ab = cross(q_minus_p, s) / r_cross_s;
        result.t_cd = cross(q_minus_p, r) / r_cross_s;
        result.point = p + result.t_ab * r;
        result.point_is_on_segment_ab = IS_BETWEEN_TIGHT(result.t_ab, 0.0f, 1.0f);
        result.point_is_on_segment_cd = IS_BETWEEN_TIGHT(result.t_cd, 0.0f, 1.0f);
    }
    return result;
}

LineLineXResult line_line_intersection(LineEntity *a, LineEntity *b) {
    return line_line_intersection(a->start, a->end, b->start, b->end);
}

struct ArcArcXResult {
    vec2 point1;
    vec2 point2;
    real theta_1a;
    real theta_1b;
    real theta_2a;
    real theta_2b;
    bool point1_is_on_arc_a;
    bool point1_is_on_arc_b;
    bool point2_is_on_arc_a;
    bool point2_is_on_arc_b;
    bool no_possible_intersection;
};

// burkardt is amazing, he even uses our arc struct
ArcArcXResult arc_arc_intersection(ArcEntity *arc_a, ArcEntity *arc_b) {

    ArcArcXResult result = {};

    float d = distance(arc_a->center, arc_b->center);

    //TODO: find fucntion that checks to see if they are close enough for floats
    if (d > arc_a->radius + arc_b->radius) {                // non intersecting
        result.no_possible_intersection = true;
    } else if (d < abs(arc_a->radius-arc_b->radius)) {      // One circle within other
        result.no_possible_intersection = true;
    } else if (d == 0 && arc_a->radius == arc_b->radius) {  // coincident circles
        result.no_possible_intersection = true;
    } else {
        real a = (POW(arc_a->radius, 2) - POW(arc_b->radius, 2) + POW(d, 2)) / (2 * d);
        real h = SQRT(POW(arc_a->radius, 2) - POW(a, 2));

        vec2 v = arc_a->center + a * (arc_b->center - arc_a->center) / d; 

        result.point1 = { v.x + h * (arc_b->center.y - arc_a->center.y) / d, v.y - h * (arc_b->center.x - arc_a->center.x) / d };
        result.point2 = { v.x - h * (arc_b->center.y - arc_a->center.y) / d, v.y + h * (arc_b->center.x - arc_a->center.x) / d };

        result.theta_1a = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(result.point1 - arc_a->center)));
        result.theta_2a = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(result.point2 - arc_a->center)));
        result.theta_1b = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(result.point1 - arc_b->center)));
        result.theta_2b = DEG(WRAP_TO_0_TAU_INTERVAL(ATAN2(result.point2 - arc_b->center)));


        result.point1_is_on_arc_a = ANGLE_IS_BETWEEN_CCW_DEGREES_TIGHT(result.theta_1a, arc_a->start_angle_in_degrees + TINY_VAL, arc_a->end_angle_in_degrees - TINY_VAL);
        result.point1_is_on_arc_b = ANGLE_IS_BETWEEN_CCW_DEGREES_TIGHT(result.theta_1b, arc_b->start_angle_in_degrees, arc_b->end_angle_in_degrees);
        result.point2_is_on_arc_a = ANGLE_IS_BETWEEN_CCW_DEGREES_TIGHT(result.theta_2a, arc_a->start_angle_in_degrees, arc_a->end_angle_in_degrees);
        result.point2_is_on_arc_b = ANGLE_IS_BETWEEN_CCW_DEGREES_TIGHT(result.theta_2b, arc_b->start_angle_in_degrees, arc_b->end_angle_in_degrees);

        result.no_possible_intersection = false;
    }

    return result;
}

struct LineArcXResult {
    vec2 point1;
    vec2 point2;
    real theta_1;
    real theta_2;
    real t1;
    real t2;
    bool point1_is_on_arc;
    bool point1_is_on_line_segment;
    bool point2_is_on_arc;
    bool point2_is_on_line_segment;
    bool no_possible_intersection;
};

LineArcXResult line_arc_intersection(LineEntity *line, ArcEntity *arc) {
    // using determinant to find num intersects https://www.nagwa.com/en/explainers/987161873194/#:~:text=the%20discriminant%20%ce%94%20%3d%20%f0%9d%90%b5%20%e2%88%92%204,and%20the%20circle%20are%20disjoint.
    LineArcXResult result = {};

    vec2 v1 = line->end - line->start;
    vec2 v2 = line->start - arc->center;

    float a = dot(v1, v1);
    float b = 2 * dot(v1, v2);
    float c = dot(v2, v2) - POW(arc->radius, 2);
    float d = POW(b, 2) - 4 * a * c;


    if (d < 0) {                // no intersect
        result.no_possible_intersection = true; // can we exit early???
    } else {                    // two intersects
        result.t1 = (-b + SQRT(d)) / (2 * a); 
        result.t2 = (-b - SQRT(d)) / (2 * a); 

        result.point1 = line->start + result.t1 * v1;
        result.point2 = line->start + result.t2 * v1;

        result.theta_1 = DEG(angle_from_0_TAU(arc->center, result.point1));
        result.theta_2 = DEG(angle_from_0_TAU(arc->center, result.point2));
        result.point1_is_on_arc = ANGLE_IS_BETWEEN_CCW_DEGREES_TIGHT(result.theta_1, arc->start_angle_in_degrees, arc->end_angle_in_degrees);
        result.point2_is_on_arc = ANGLE_IS_BETWEEN_CCW_DEGREES_TIGHT(result.theta_2, arc->start_angle_in_degrees, arc->end_angle_in_degrees);

        result.point1_is_on_line_segment = IS_BETWEEN_TIGHT(result.t1, 0.0f, 1.0f);
        result.point2_is_on_line_segment = IS_BETWEEN_TIGHT(result.t2, 0.0f, 1.0f);

    }

    return result;
}

struct ArcArcXClosestResult {
    vec2 point;
    real theta_a;
    real theta_b;
    bool point_is_on_arc_a;
    bool point_is_on_arc_b;
    bool no_possible_intersection;
};

ArcArcXClosestResult arc_arc_intersection_closest(ArcEntity *arc_a, ArcEntity *arc_b, vec2 point) {
    ArcArcXClosestResult result;
    ArcArcXResult two_point_result = arc_arc_intersection(arc_a, arc_b);
    if (distance(point, two_point_result.point1) < distance(point, two_point_result.point2)) {
        result.point = two_point_result.point1;
        result.theta_a = two_point_result.theta_1a;
        result.theta_b = two_point_result.theta_1b;
        result.point_is_on_arc_a = two_point_result.point1_is_on_arc_a;
        result.point_is_on_arc_b = two_point_result.point1_is_on_arc_b;
    } else {
        result.point = two_point_result.point2;
        result.theta_a = two_point_result.theta_2a;
        result.theta_b = two_point_result.theta_2b;
        result.point_is_on_arc_a = two_point_result.point2_is_on_arc_a;
        result.point_is_on_arc_b = two_point_result.point2_is_on_arc_b;
    }
    result.no_possible_intersection = two_point_result.no_possible_intersection;
    return result;
}

struct LineArcXClosestResult {
    vec2 point;
    real theta;
    real t;
    bool point_is_on_arc;
    bool point_is_on_line_segment;
    bool no_possible_intersection;
};

LineArcXClosestResult line_arc_intersection_closest(LineEntity *line, ArcEntity *arc, vec2 point) {
    LineArcXClosestResult result;
    LineArcXResult two_point_result = line_arc_intersection(line, arc);
    if (distance(point, two_point_result.point1) < distance(point, two_point_result.point2)) {
        result.point = two_point_result.point1;
        result.theta = two_point_result.theta_1;
        result.t = two_point_result.t1;
        result.point_is_on_arc = two_point_result.point1_is_on_arc;
        result.point_is_on_line_segment = two_point_result.point1_is_on_line_segment;
    } else {
        result.point = two_point_result.point2;
        result.theta = two_point_result.theta_2;
        result.t = two_point_result.t2;
        result.point_is_on_arc = two_point_result.point2_is_on_arc;
        result.point_is_on_line_segment = two_point_result.point2_is_on_line_segment;
    }
    result.no_possible_intersection = two_point_result.no_possible_intersection;
    return result;
}

struct ClosestIntersectionResult {
    vec2 point;
    bool no_possible_intersection;
};

ClosestIntersectionResult closest_intersection(Entity *A, Entity *B, vec2 point) {
    if (A->type == EntityType::Line && B->type == EntityType::Line) {
        LineLineXResult res = line_line_intersection(&A->line, &B->line);
        return { res.point, res.lines_are_parallel}; 
    } else if (A->type == EntityType::Arc && B->type == EntityType::Arc) {
        ArcArcXClosestResult res = arc_arc_intersection_closest(&A->arc, &B->arc, point);
        return { res.point, res.no_possible_intersection };
    } else {
        LineArcXClosestResult res; 
        if (A->type == EntityType::Line) res = line_arc_intersection_closest(&A->line, &B->arc, point);
        else res = line_arc_intersection_closest(&B->line, &A->arc, point);
        return { res.point, res.no_possible_intersection };
    }
}

real get_three_point_angle(vec2 p, vec2 center, vec2 q) {
    real theta_p = angle_from_0_TAU(center, p);
    real theta_q = angle_from_0_TAU(center, q);
    real result = theta_q - theta_p;
    if (result < 0.0f) result += TAU;
    return result;
}
// <!> End header.cpp <!>
// <!> Begin header2.cpp <!> 
// TODO: move make's out of cookbook and into here
// TODO: entity_offseted (reference point)

Entity entity_make_three_point_arc_or_line(vec2 a, vec2 b, vec2 c) {
    Entity result = {};
    if (false) { // full circle
        result.type = EntityType::Circle;
        result.circle.center = AVG(a, b);
        result.circle.radius = distance(a, result.circle.center);
    } else {
        vec2 p1 = AVG(a, b);
        vec2 p2 = AVG(b, c);
        vec2 n1 = normalized(perpendicularTo(b - a));
        vec2 n2 = normalized(perpendicularTo(c - b));
        LineLineXResult intersection_result = line_line_intersection(p1, p1 + n1, p2, p2 + n2);
        if (intersection_result.lines_are_parallel) { // Line
            result.type = EntityType::Line;
            result.line.start = a;
            result.line.end   = c;
        } else {
            result.type = EntityType::Arc;
            result.arc.center = intersection_result.point;;
            result.arc.radius = distance(a, result.arc.center);
            result.arc.start_angle_in_degrees = DEG(ATAN2(c - result.arc.center));
            result.arc.end_angle_in_degrees   = DEG(ATAN2(a - result.arc.center));
            if (intersection_result.t_ab < 0.0) {
                SWAP(&result.arc.start_angle_in_degrees, &result.arc.end_angle_in_degrees);
            }
        }
    }
    return result;
}

Entity entity_translated(const Entity *_entity, vec2 translation_vector) {
    Entity result = *_entity;
    if (result.type == EntityType::Line) {
        LineEntity *line = &result.line;
        line->start += translation_vector;
        line->end   += translation_vector;
    } else if (result.type == EntityType::Arc) {
        ArcEntity *arc = &result.arc;
        arc->center += translation_vector;
    } else { ASSERT(result.type == EntityType::Circle);
        CircleEntity *circle = &result.circle;
        circle->center += translation_vector;
    }
    return result;
}

Entity entity_rotated(const Entity *_entity, vec2 center, real theta) {
    Entity result = *_entity;
    if (result.type == EntityType::Line) {
        LineEntity *line = &result.line;
        line->start = rotated_about(line->start, center, theta);
        line->end   = rotated_about(line->end,   center, theta);
    } else if (result.type == EntityType::Arc) {
        ArcEntity *arc = &result.arc;
        arc->center = rotated_about(arc->center, center, theta);
        arc->start_angle_in_degrees += DEG(theta);
        arc->end_angle_in_degrees   += DEG(theta);
    } else { ASSERT(result.type == EntityType::Circle);
        CircleEntity *circle = &result.circle;
        circle->center = rotated_about(circle->center, center, theta);
        circle->pseudo_point_angle += theta;
    }
    return result;
}

Entity entity_offsetted(const Entity *_entity, real offset_distance, vec2 reference_point) {
    Entity result = *_entity;
    if (result.type == EntityType::Line) {
        LineEntity *line = &result.line;
        vec2 normal = normalized(perpendicularTo(line->end - line->start));
        real sign = (dot(normal, reference_point - line->start) < 0.0) ? -1 : 1;
        vec2 offset = sign * offset_distance * normal;
        line->start += offset;
        line->end += offset;
    } else if (result.type == EntityType::Arc) {
        ArcEntity *arc = &result.arc;
        bool in_circle = (distance(arc->center, reference_point) < arc->radius);
        bool in_sector = false;
        if (!in_circle) {
            // TODO: comment (diagram)
            vec2 start_point = entity_get_start_point(&result);
            vec2 end_point = entity_get_end_point(&result);
            vec2 perp_end = perpendicularTo(end_point - arc->center);
            vec2 perp_start = perpendicularTo(start_point - arc->center);
            vec2 end_to_mouse = reference_point - end_point;
            vec2 start_to_mouse = reference_point - start_point;
            real end_cross_p = cross(end_to_mouse, perp_end);
            real start_cross_p = cross(start_to_mouse, perp_start);
            real diam_cross_p = cross(end_to_mouse, start_point - end_point);
            in_sector = (end_cross_p > 0) && (start_cross_p > 0) && (diam_cross_p > 0);
        }
        int sign = (in_circle || in_sector) ? -1 : 1;
        arc->radius += sign * offset_distance;
    } else { ASSERT(result.type == EntityType::Circle);
        CircleEntity *circle = &result.circle;
        bool in_circle = distance(circle->center, reference_point) < circle->radius;
        int sign = (!in_circle) ? 1 : -1;
        circle->radius += sign * offset_distance;
    }
    return result;
}

#if 0
Entity entity_mirrored(const Entity *result, vec2 origin, real axis_angle) {
    return {};
}


#endif
// <!> End header2.cpp <!>

#define DUMMY_HOTKEY 9999

// (global) state
WorldState_ChangesToThisMustBeRecorded_state state;
ScreenState_ChangesToThisDo_NOT_NeedToBeRecorded_other other;

// convenient pointers to shorten xxx.foo.bar into foo->bar
Drawing *drawing = &state.drawing;
FeaturePlaneState *feature_plane = &state.feature_plane;
Mesh *mesh = &state.mesh;
PopupState *popup = &state.popup;
ToolboxState *toolbox = &state.toolbox;
TwoClickCommandState *two_click_command = &state.two_click_command;
MeshTwoClickCommandState *mesh_two_click_command = &state.mesh_two_click_command;
Camera *camera_drawing = &other.camera_drawing;
Camera *camera_mesh = &other.camera_mesh;
PreviewState *preview = &other.preview;

// FORNOW
Event event_passed_to_popups;
bool already_processed_event_passed_to_popups;

// <!> Begin boolean.cpp <!> 
bool click_mode_SPACE_BAR_REPEAT_ELIGIBLE() {
    return 0
        || (state_Draw_command_is_(Box))
        || (state_Draw_command_is_(Circle))
        || (state_Draw_command_is_(Fillet))
        || (state_Draw_command_is_(Line))
        || (state_Draw_command_is_(Measure))
        || (state_Draw_command_is_(Mirror2))
        || (state_Draw_command_is_(MirrorX))
        || (state_Draw_command_is_(MirrorY))
        || (state_Draw_command_is_(Move))
        || (state_Draw_command_is_(SetOrigin))
        || (state_Draw_command_is_(Polygon))
        || (state_Draw_command_is_(Rotate))
        || (state_Draw_command_is_(RCopy))
        || (state_Draw_command_is_(Divide2))
        || (state_Draw_command_is_(DiamCircle))
        ;
}

bool SELECT_OR_DESELECT() {
    return ((state_Draw_command_is_(Select)) || (state_Draw_command_is_(Deselect)));
}

bool WINDOW_SELECT_OR_WINDOW_DESELECT() {
    return (SELECT_OR_DESELECT() && (state_Xsel_command_is_(Window)));
}

bool _non_WINDOW__SELECT_DESELECT___OR___SET_COLOR() {
    return (
            ((!state_Xsel_command_is_(Window)) && SELECT_OR_DESELECT())
            || (state_Draw_command_is_(SetColor)));
}

bool _SELECT_OR_DESELECT_COLOR() {
    bool A = SELECT_OR_DESELECT();
    bool B = (state_Draw_command_is_(SetColor));
    return A && B;
}


bool first_click_must_acquire_entity() {
    return 0 ||
        (state_Draw_command_is_(DogEar)) ||
        (state_Draw_command_is_(Fillet)) ||
        (state_Draw_command_is_(Divide2))
        ;
}

bool enter_mode_SHIFT_SPACE_BAR_REPEAT_ELIGIBLE() {
    return 0
        || (state_Mesh_command_is_(ExtrudeAdd))
        || (state_Mesh_command_is_(ExtrudeCut))
        || (state_Mesh_command_is_(RevolveAdd))
        || (state_Mesh_command_is_(RevolveCut))
        || (state_Mesh_command_is_(NudgePlane))
        ;
}

// <!> End boolean.cpp <!>
// <!> Begin misc.cpp <!> 
#define _for_each_entity_ for (\
        Entity *entity = drawing->entities.array;\
        entity < &drawing->entities.array[drawing->entities.length];\
        ++entity)

#define _for_each_selected_entity_ _for_each_entity_ if (entity->is_selected) 

template <typename T> void JUICEIT_EASYTWEEN(T *a, T b, real multiplier) {
    real f = multiplier * 0.1f;
    if (IS_ZERO(multiplier)) f = 1.0f;
    if (!other.paused) *a += f * (b - *a);
}

real _JUICEIT_EASYTWEEN(real t) { return 0.287f * log(t) + 1.172f; }


real get_x_divider_drawing_mesh_Pixel() {
    return LINEAR_REMAP(other.x_divider_drawing_mesh_OpenGL, -1.0f, 1.0f, 0.0f, window_get_size_Pixel().x);
}

#define __snap_for__ _for_each_entity_ if (!( \
            1 \
            && (state.Draw_command.flags & EXCLUDE_SELECTED_FROM_SECOND_CLICK_SNAP) \
            && (two_click_command->awaiting_second_click) \
            && (entity->is_selected) \
            ))

MagicSnapResult magic_snap(vec2 before, bool calling_this_function_for_drawing_preview = false) {
    MagicSnapResult result = {};
    result.mouse_position = before;
    {
        if (1
                && (state.Draw_command.flags & SHIFT_15)
                && (two_click_command->awaiting_second_click)
                && (other.shift_held)) {
            vec2 a = two_click_command->first_click;
            vec2 b = before;
            vec2 r = b - a; 
            real norm_r = norm(r);
            real factor = 360 / 15 / TAU;
            real theta = roundf(ATAN2(r) * factor) / factor;
            result.mouse_position = a + norm_r * e_theta(theta);
        } else if (
                (state_Draw_command_is_(Box))
                && (two_click_command->awaiting_second_click)
                && (other.shift_held)) { // TODO (Felipe): snap square
            result.mouse_position = before;
        } else if (!calling_this_function_for_drawing_preview) { // NOTE: this else does, in fact, match LAYOUT's behavior
            DXFFindClosestEntityResult closest_entity_info = {};

            // TODO: need to filter End and Middle as well to ignore circles
            // (this all needs to be cleaned up)
            if (0
                    || state_Snap_command_is_(Center)
                    || state_Snap_command_is_(Quad)
                    //|| state_Snap_command_is_(Tangent)
               ) {
                real min_squared_distance = HUGE_VAL;
                Entity *temp_entity = NULL;
                __snap_for__ {
                    if (entity->type == EntityType::Line) {
                        continue;
                    } else if (entity->type == EntityType::Arc) {
                        ArcEntity *arc = &entity->arc;
                        real squared_distance = squared_distance_point_dxf_arc_entity(before, arc);
                        if (squared_distance < min_squared_distance) {
                            min_squared_distance = squared_distance;
                            temp_entity = entity;
                        }
                    } else { ASSERT(entity->type == EntityType::Circle); 
                        CircleEntity *circle = &entity->circle;
                        real squared_distance = squared_distance_point_dxf_circle_entity(before, circle);
                        if (squared_distance < min_squared_distance) {
                            min_squared_distance = squared_distance;
                            temp_entity = entity;
                        }
                    }
                }
                if (!ARE_EQUAL(min_squared_distance, HUGE_VAL)) {
                    closest_entity_info.success = true;
                    closest_entity_info.closest_entity = temp_entity;
                } else {
                    closest_entity_info.success = false;
                }
            } else { // NOTE (Jim): ? else if (!state_Snap_command_is_(None))
                closest_entity_info = dxf_find_closest_entity(&drawing->entities, before);
            }

            if (closest_entity_info.success) {
                Entity *closest_entity = closest_entity_info.closest_entity;
                result.entity_index_snapped_to = uint(closest_entity_info.closest_entity - drawing->entities.array); //TODO TODO TODO
                if (state_Snap_command_is_(Center)) {
                    result.snapped = true;
                    if (closest_entity->type == EntityType::Arc) {
                        ArcEntity *arc = &closest_entity->arc;
                        result.mouse_position = arc->center;
                    } else { ASSERT(closest_entity->type == EntityType::Circle);
                        CircleEntity *circle = &closest_entity->circle;
                        result.mouse_position = circle->center;
                    }
                } else if (state_Snap_command_is_(Quad)) {
                    result.snapped = true;
                    vec2 center;
                    real radius;
                    real angle;
                    {
                        if (closest_entity->type == EntityType::Arc) {
                            ArcEntity *arc = &closest_entity->arc;
                            center = arc->center;
                            radius = arc->radius;
                        } else { ASSERT(closest_entity->type == EntityType::Circle);
                            CircleEntity *circle = &closest_entity->circle;
                            center = circle->center;
                            radius = circle->radius;
                        }
                        { // angle
                            angle = LINEAR_REMAP(angle_from_0_TAU(center, before), 0.0f, TAU, 0.0f, 4.0f);
                            angle = (ROUND(angle) / 4.0f) * TAU;
                        }
                    }
                    result.mouse_position = get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, angle);
                } else if (state_Snap_command_is_(Middle)) {
                    if (closest_entity->type != EntityType::Circle) {
                        result.mouse_position = entity_get_middle(closest_entity);
                        result.snapped = true;
                    }
                } else if (state_Snap_command_is_(End)) { // this one is a little custom
                    real min_squared_distance = HUGE_VAL;
                    __snap_for__ {
                        uint count = 0;
                        vec2 p[2] = {};
                        if (entity->type == EntityType::Circle) {
                            CircleEntity *circle = &entity->circle;
                            if (circle->has_pseudo_point) {
                                p[count++] = circle->get_pseudo_point();
                            }
                        } else {
                            entity_get_start_and_end_points(entity, &p[0], &p[1]);
                            count = 2;
                        }
                        for_(d, count) {
                            real squared_distance = squaredDistance(before, p[d]);
                            if (squared_distance < min_squared_distance) {
                                min_squared_distance = squared_distance;
                                result.mouse_position = p[d];
                                result.entity_index_snapped_to = uint(entity - drawing->entities.array);
                                result.snapped = true;
                            }
                        }
                    }
                } else if (state_Snap_command_is_(Intersect)) { // this one is a little custom
                                                                // TODO Circle
                    real min_squared_distance = HUGE_VAL;
                    Entity *temp_entity = NULL;
                    __snap_for__ {
                        real squared_distance = squared_distance_point_entity(before, entity);
                        if (squared_distance < min_squared_distance && entity != closest_entity) {
                            min_squared_distance = squared_distance;
                            temp_entity = entity;
                        }
                    }
                    if (temp_entity != NULL) {
                        ClosestIntersectionResult res = closest_intersection(closest_entity, temp_entity, before);
                        if (!res.no_possible_intersection) {
                            result.mouse_position = res.point;
                            result.entity_index_snapped_to = uint(closest_entity_info.closest_entity - drawing->entities.array); //TODO TODO TODO
                            result.entity_index_intersect = uint(temp_entity - drawing->entities.array);
                            result.snapped = true;
                            result.split_intersect = true;
                        }
                        // else messagef(pallete.orange, "no intersection found");
                    }
                    // else messagef(pallete.orange, "no intersection found");
                } else if (state_Snap_command_is_(Perp)) { 
                    vec2 click_one = two_click_command->awaiting_second_click ? two_click_command->first_click : before;
                    if (closest_entity->type == EntityType::Line) {
                        vec2 a_to_b = closest_entity->line.end - closest_entity->line.start;
                        vec2 a_to_p = click_one - closest_entity->line.start;
                        real t = dot(a_to_p, a_to_b) / dot(a_to_b, a_to_b);
                        result.mouse_position = closest_entity->line.start + t * a_to_b; 
                        result.snapped = true;
                    } else if (closest_entity->type == EntityType::Arc) { // layout pretends the arc is a full circle for perp
                        vec2 normalized_in_direction = normalized(click_one - closest_entity->arc.center);
                        vec2 perp_one = closest_entity->arc.center + closest_entity->arc.radius * normalized_in_direction;
                        vec2 perp_two = closest_entity->arc.center - closest_entity->arc.radius * normalized_in_direction;

                        result.mouse_position = distance(perp_one, before) < distance(perp_two, before) ? perp_one : perp_two;
                        result.snapped = true;
                    } else { ASSERT(closest_entity->type == EntityType::Circle);
                        vec2 normalized_in_direction = normalized(click_one - closest_entity->circle.center);
                        vec2 perp_one = closest_entity->circle.center + closest_entity->circle.radius * normalized_in_direction;
                        vec2 perp_two = closest_entity->circle.center - closest_entity->circle.radius * normalized_in_direction;

                        result.mouse_position = distance(perp_one, before) < distance(perp_two, before) ? perp_one : perp_two;
                        result.snapped = true;
                    }
                } /*else if (state_Snap_command_is_(Tangent)) {
                // TODO TODO TODO
                vec2 mouse = before;

                if (two_click_command->awaiting_second_click) {
                mouse = two_click_command->first_click;
                }
                if (two_click_command->awaiting_second_click && two_click_command->tangent_first_click) {
                mouse = two_click_command->first_click;

                ArcEntity c2 = closest_entity->arc;
                ArcEntity c1 = two_click_command->entity_closest_to_first_click->arc;

                vec2 center_diff = c2.center - c1.center;
                real dist = distance(c1.center, c2.center);
                real angle = ATAN2(center_diff);

                real phi1 = acos((c1.radius - c2.radius) / dist);
                real phi2 = acos((c1.radius + c2.radius) / dist);

                real theta1a = angle + phi1;
                real theta1b = angle - phi1;
                real theta2a = angle + phi2;
                real theta2b = angle - phi2;

                vec2 p1a1 = c1.center + V2(c1.radius * COS(theta1a), c1.radius * SIN(theta1a));
                vec2 p1a2 = c1.center + V2(c1.radius * COS(theta1b), c1.radius * SIN(theta1b));
                if (distance(mouse, p1a1) > distance(mouse, p1a1)) {
                p1a1 = p1a2;
                theta1a = theta1b;
                }
                vec2 p1b = c2.center + V2(c2.radius * COS(theta1a), c2.radius * SIN(theta1a));

                vec2 p2a1 = c1.center + V2(c1.radius * COS(theta2a), c1.radius * SIN(theta2a));
                vec2 p2a2 = c1.center + V2(c1.radius * COS(theta2b), c1.radius * SIN(theta2b));
                if (distance(mouse, p2a1) > distance(mouse, p2a2)) {
                p2a1 = p2a2;
                theta2a = theta2b;
                }
                vec2 p2b = c2.center - V2(c2.radius * COS(theta2a), c2.radius * SIN(theta2a));

                if (distance(before, p1b) > distance(before, p2b)) {
                two_click_command->first_click = p2a1;
                result.mouse_position = p2b;
                } else {
                two_click_command->first_click = p1a1;
                result.mouse_position = p1b;
                }

                two_click_command->tangent_first_click = false;
                result.snapped = true;
                result.split_tangent_2 = true;
                result.entity_index_tangent_2 = uint(two_click_command->entity_closest_to_first_click - drawing->entities.array);

                } else if (two_click_command->awaiting_second_click) {
                vec2 center = closest_entity->arc.center;
                real radius = closest_entity->arc.radius;
                real d = distance(center, mouse);

                if (d > radius) {
                real t1 = ATAN2(mouse - center);
                real t2 = acos(radius / d);
                real theta1 = t1 + t2;
                real theta2 = t1 - t2;
                vec2 tan1 = { center.x + radius * COS(theta1), center.y + radius * SIN(theta1) };
                vec2 tan2 = { center.x + radius * COS(theta2), center.y + radius * SIN(theta2) };
                result.mouse_position = distance(before, tan1) < distance(before, tan2) ? tan1 : tan2;
                result.snapped = true;
                }
                } else {
                messagef(pallete.light_gray, "wowowwowowo");
                two_click_command->tangent_first_click = true; 
                two_click_command->entity_closest_to_first_click = closest_entity;
                messagef(pallete.red, "%f %f", closest_entity->arc.center.x, closest_entity->arc.center.y);
            }
            }*/
            }
        }
    }


    return result;
}

MagicSnapResult3D magic_snap_3d() {
    MagicSnapResult3D result{};

    mat4 World_3D_from_OpenGL = inverse(camera_mesh->get_PV());
    vec3 ray_origin = transformPoint(World_3D_from_OpenGL, V3(other.mouse_OpenGL, -1.0f));
    vec3 ray_end = transformPoint(World_3D_from_OpenGL, V3(other.mouse_OpenGL,  1.0f));
    vec3 ray_direction = normalized(ray_end - ray_origin);

    int index_of_first_triangle_hit_by_ray = -1;
    vec3 exact_hit_pos;
    {
        real min_distance = HUGE_VAL;
        for_(i, mesh->num_triangles) {
            vec3 p[3]; {
                for_(j, 3) p[j] = mesh->vertex_positions[mesh->triangle_indices[i][j]];
            }
            RayTriangleIntersectionResult ray_triangle_intersection_result = ray_triangle_intersection(ray_origin, ray_direction, p[0], p[1], p[2]);
            if (ray_triangle_intersection_result.hit) {
                if (ray_triangle_intersection_result.distance < min_distance) {
                    min_distance = ray_triangle_intersection_result.distance;
                    exact_hit_pos = ray_triangle_intersection_result.pos;
                    index_of_first_triangle_hit_by_ray = i; // FORNOW
                }
            }
        }
    }

    if (index_of_first_triangle_hit_by_ray != -1) { // something hit
        result.triangle_index = index_of_first_triangle_hit_by_ray;
        result.hit_mesh = true;
        result.mouse_position = exact_hit_pos;

        if (!state_Snap_command_is_(None)) { // TODO: Change to 3D specific snap type?
            real min_distance = HUGE_VAL;
            for_(i, 3) {
                vec3 vertex_pos = mesh->vertex_positions[mesh->triangle_indices[index_of_first_triangle_hit_by_ray][i]];
                real dist = squaredDistance(exact_hit_pos, vertex_pos);
                if (dist < min_distance) {
                    min_distance = dist;
                    result.mouse_position = vertex_pos;
                    result.snapped = true;
                }
            }
        }
    }

    return result;
}

void init_camera_drawing() {
    *camera_drawing = make_Camera2D(100.0f, {}, { AVG(-1.0f, other.x_divider_drawing_mesh_OpenGL), 0.0f });
    if (drawing->entities.length) {
        bbox2 bbox = entities_get_bbox(&drawing->entities);
        real eps = 150.0f;
        real f = ((get_x_divider_drawing_mesh_Pixel() - eps) / window_get_width_Pixel());
        vec2 L = (bbox.max - bbox.min);
        camera_drawing->ortho_screen_height_World = MAX((L.x / f) / window_get_aspect(), L.y);
        camera_drawing->ortho_screen_height_World += 96.0f * (camera_drawing->ortho_screen_height_World / window_get_height_Pixel());
        camera_drawing->pre_nudge_World = V2(-(eps / 2) * (camera_drawing->ortho_screen_height_World / window_get_height_Pixel()), 0.0f) + AVG(bbox.min, bbox.max);
    }
}
void init_camera_mesh() {
    *camera_mesh = make_OrbitCamera3D(
            200.0f,
            CAMERA_3D_PERSPECTIVE_ANGLE_OF_VIEW,
            { RAD(-44.0f), RAD(33.0f) },
            {},
            { AVG(other.x_divider_drawing_mesh_OpenGL, 1.0f), 0.0f }
            );
    // // TODO: rasterize the bounding box
    // mat4 PV = camera_get_PV(camera_mesh);
    // bbox2 B = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
    // B += _V2(transformPoint(PV, V3(mesh->bbox.min.x, mesh->bbox.min.y, mesh->bbox.min.z)));
    // B += _V2(transformPoint(PV, V3(mesh->bbox.max.x, mesh->bbox.min.y, mesh->bbox.min.z)));
    // B += _V2(transformPoint(PV, V3(mesh->bbox.min.x, mesh->bbox.max.y, mesh->bbox.min.z)));
    // B += _V2(transformPoint(PV, V3(mesh->bbox.max.x, mesh->bbox.max.y, mesh->bbox.min.z)));
    // B += _V2(transformPoint(PV, V3(mesh->bbox.min.x, mesh->bbox.min.y, mesh->bbox.max.z)));
    // B += _V2(transformPoint(PV, V3(mesh->bbox.max.x, mesh->bbox.min.y, mesh->bbox.max.z)));
    // B += _V2(transformPoint(PV, V3(mesh->bbox.min.x, mesh->bbox.max.y, mesh->bbox.max.z)));
    // B += _V2(transformPoint(PV, V3(mesh->bbox.max.x, mesh->bbox.max.y, mesh->bbox.max.z)));
    // real f = 1.0f - (get_x_divider_Pixel() / window_get_width_Pixel());
    // vec2 L_OpenGL = (B.max - B.min);
    // real fac = camera_mesh->persp_distance_to_origin_World * TAN(camera_mesh->angle_of_view);;
    // vec2 L = L_OpenGL * fac;
    // camera_mesh->persp_distance_to_origin_World = (0.5f * MAX((L.x / f) / window_get_aspect(), L.y)) / TAN(camera_mesh->angle_of_view);
    // camera_mesh->pre_nudge_World = fac * (V2(0.5f) + 0.5f * AVG(B.min, B.max));
}




// <!> End misc.cpp <!>
// <!> Begin draw.cpp <!> 
// // TODO: (Jim) stuff for alpha
// TODO: fix in/out relationship (right now they just seem to add)
// TODO: tubes
// TODO: manifold_wrapper
// XXX: fix origin axis relationship with revolve
// XXX: - tubes
// XXX: - manifold_wrapper
// TODO: draw axis on RHS when revolving
// TODO: 3D-picking is broken for non xyz planes
// TODO: revisit extruded cut on the botton of box with name (why did the students need to flip their names)





mat4 get_M_3D_from_2D() {
    vec3 up = { 0.0f, 1.0f, 0.0f };
    real dot_product = dot(feature_plane->normal, up);
    // OLD VERSION:
    // vec3 y = (ARE_EQUAL(ABS(dot_product), 1.0f)) ? V3(0.0f,  0.0f, -1.0f * SGN(dot_product)) : up;
    // vec3 x = normalized(cross(y, feature_plane->normal));
    // vec3 z = cross(x, y);
    vec3 down = (ARE_EQUAL(ABS(dot_product), 1.0f)) ? V3(0.0f, 0.0f, 1.0f * SGN(dot_product)) : V3(0.0f, -1.0f, 0.0f);
    vec3 z = feature_plane->normal;
    vec3 x = normalized(cross(z, down));
    vec3 y = cross(z, x);

    if (other.mirror_3D_plane_X)
        x *= -1;
    if (other.mirror_3D_plane_Y)
        y *= -1;

    return M4_xyzo(x, y, z, (feature_plane->signed_distance_to_world_origin) * feature_plane->normal);
}

bbox2 mesh_draw(mat4 P_3D, mat4 V_3D, mat4 M_3D) {
    bbox2 face_selection_bbox = BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA<2>();
    mat4 inv_M_3D_from_2D = inverse(get_M_3D_from_2D());

    mat4 PVM_3D = P_3D * V_3D * M_3D;

    if (!other.show_details) {
        if (mesh->cosmetic_edges) {
            eso_begin(PVM_3D, SOUP_LINES); 
            // eso_color(CLAMPED_LERP(2 * time_since_successful_feature, pallete.white, pallete.black));
            eso_color(0,0,0);
            eso_size(1.0f);
            for_(i, mesh->num_cosmetic_edges) {
                for_(d, 2) {
                    eso_vertex(mesh->vertex_positions[mesh->cosmetic_edges[i][d]]);
                }
            }
            eso_end();
        }
    }

    for_(pass, 2) {
        eso_begin(PVM_3D, (!other.show_details) ? SOUP_TRIANGLES : SOUP_TRI_MESH);
        eso_size(0.5f);

        // mat3 inv_transpose_V_3D = inverse(transpose(_M3(V_3D)));


        for_(i, mesh->num_triangles) {
            vec3 n = mesh->triangle_normals[i];
            vec3 p[3];
            real x_n;
            {
                for_(j, 3) p[j] = mesh->vertex_positions[mesh->triangle_indices[i][j]];
                x_n = dot(n, p[0]);
            }

            vec3 eye; {
                mat4 C = inverse(V_3D); // Is this recalculating the inverse for every triangle!?
                for_(d, 3) eye[d] = C(d, 3);
            }

            vec3 v = normalized(eye - p[0]);

            vec3 color; 
            real alpha;
            {
                // vec3 n_Camera = inv_transpose_V_3D * n;
                // vec3 color_n = V3(V2(0.66f) + 0.33f * _V2(n_Camera), 1.0f);
                // n_Camera = V3(0.5f) + 0.5f * n_Camera;
                // vec3 color_n = (n_Camera.x * monokai.red + n_Camera.y * monokai.blue + n_Camera.z * monokai.purple) / 2;
                vec3 color_n = V3(0.4f + 0.3f * MAX(0.0f, dot(n, v)));
                if ((true || feature_plane->is_active) && (dot(n, feature_plane->normal) > 0.99f) && (ABS(x_n - feature_plane->signed_distance_to_world_origin) < 0.01f)) {
                    if (pass == 0) continue;

                    // TODO:
                    if (feature_plane->is_active) {
                        // color = CLAMPED_LERP(_JUICEIT_EASYTWEEN(other.time_since_plane_selected), pallete.white, V3(0.65f, 0.67f, 0.10f));// CLAMPED_LERP(2.0f * time_since_plane_selected - 0.5f, pallete.yellow, V3(0.85f, 0.87f, 0.30f));
                        color = CLAMPED_LERP(_JUICEIT_EASYTWEEN(other.time_since_plane_selected), pallete.white, pallete.light_gray);// CLAMPED_LERP(2.0f * time_since_plane_selected - 0.5f, pallete.yellow, V3(0.85f, 0.87f, 0.30f));
                    } else color = color_n;

                    // if (2.0f * time_since_plane_selected < 0.3f) color = pallete.white; // FORNOW

                    alpha = CLAMPED_LERP(_JUICEIT_EASYTWEEN(other.time_since_going_inside), 1.0f, 0.7f);

                    face_selection_bbox += _V2(transformPoint(inv_M_3D_from_2D, p[0]));
                    face_selection_bbox += _V2(transformPoint(inv_M_3D_from_2D, p[1]));
                    face_selection_bbox += _V2(transformPoint(inv_M_3D_from_2D, p[2]));


                } else {
                    if (pass == 1) continue;
                    color = color_n;
                    alpha = 1.0f;
                }
            }
            real mask = CLAMP(1.2f * other.time_since_successful_feature, 0.0f, 2.0f);
            // color = CLAMPED_LINEAR_REMAP(time_since_successful_feature, -0.5f, 0.5f, pallete.white, color);
            eso_color(color, alpha);
            for_(d, 3) {
                eso_color(CLAMPED_LERP(mask + SIN(CLAMPED_INVERSE_LERP(p[d].y, mesh->bbox.max.y, mesh->bbox.min.y) + 0.5f * other.time_since_successful_feature), pallete.white, color), alpha);

                eso_vertex(p[d]);
            }
        }
        eso_end();
    }

    return face_selection_bbox;
}

void conversation_draw() {
    mat4 P_2D = camera_drawing->get_P();
    mat4 V_2D = camera_drawing->get_V();
    mat4 PV_2D = P_2D * V_2D;
    mat4 inv_PV_2D = inverse(PV_2D);
    vec2 mouse_World_2D = transformPoint(inv_PV_2D, other.mouse_OpenGL);
    mat4 M_3D_from_2D = get_M_3D_from_2D();



    bool extruding = ((state_Mesh_command_is_(ExtrudeAdd)) || (state_Mesh_command_is_(ExtrudeCut)));
    bool revolving = ((state_Mesh_command_is_(RevolveAdd)) || (state_Mesh_command_is_(RevolveCut)));
    bool adding     = ((state_Mesh_command_is_(ExtrudeAdd)) || (state_Mesh_command_is_(RevolveAdd)));
    bool cutting     = ((state_Mesh_command_is_(ExtrudeCut)) || (state_Mesh_command_is_(RevolveCut)));

    { // preview->extrude_in_length
        real target = (adding) ? popup->extrude_add_in_length : popup->extrude_cut_in_length;
        JUICEIT_EASYTWEEN(&preview->extrude_in_length, target);
    }
    { // preview->extrude_out_length
        real target = (adding) ? popup->extrude_add_out_length : popup->extrude_cut_out_length;
        JUICEIT_EASYTWEEN(&preview->extrude_out_length, target);
    }
    { // preview->revolve_in_angle
        real target = (adding) ? popup->revolve_add_in_angle : popup->revolve_cut_in_angle;
        JUICEIT_EASYTWEEN(&preview->revolve_in_angle, target);
    }
    { // preview->revolve_out_angle
        real target = (adding) ? popup->revolve_add_out_angle : popup->revolve_cut_out_angle;
        JUICEIT_EASYTWEEN(&preview->revolve_out_angle, target);
    }

    // TODO
    { // preview_feature_plane_offset
        real target = (state_Mesh_command_is_(NudgePlane)) ? popup->feature_plane_nudge : 0.0f;
        JUICEIT_EASYTWEEN(&preview->feature_plane_offset, target);
    }

    // preview
    vec2 mouse = magic_snap(mouse_World_2D, true).mouse_position; // this isn't really snapped per se (probably a bad name) -- just has the 15 deg stuff and similar

    if (two_click_command->awaiting_second_click && two_click_command->tangent_first_click) {
        //messagef(pallete.red, "wowowo");
        vec2 before = mouse;
        Entity *closest_entity = two_click_command->entity_closest_to_first_click;
        messagef(pallete.orange, "%f %f", closest_entity->arc.center.x, closest_entity->arc.center.y);
        vec2 center = closest_entity->arc.center;
        real radius = closest_entity->arc.radius;
        real d = distance(center, before);

        if (d > radius) {
            real t1 = acos(radius / d);
            real t2 = ATAN2(before - center);
            real theta = t1 + t2;
            two_click_command->first_click = { center.x + radius * COS(theta), center.y + radius * SIN(theta) };
        }
    }
    {
        vec2 target_preview_mouse = mouse;
        JUICEIT_EASYTWEEN(&preview->mouse, target_preview_mouse);
    }

    vec2 target_preview_drawing_origin = (!state_Draw_command_is_(SetOrigin)) ? drawing->origin : mouse;
    {
        JUICEIT_EASYTWEEN(&preview->drawing_origin, target_preview_drawing_origin);
    }

    // TODO: lerp
    vec2 preview_dxf_axis_base_point;
    real preview_dxf_axis_angle_from_y;
    {
        if (!state_Draw_command_is_(SetAxis)) {
            preview_dxf_axis_base_point = drawing->axis_base_point;
            preview_dxf_axis_angle_from_y = drawing->axis_angle_from_y;
        } else if (!two_click_command->awaiting_second_click) {
            preview_dxf_axis_base_point = mouse;
            preview_dxf_axis_angle_from_y = drawing->axis_angle_from_y;
        } else {
            preview_dxf_axis_base_point = two_click_command->first_click;
            preview_dxf_axis_angle_from_y = ATAN2(mouse - preview_dxf_axis_base_point) - PI / 2;
        }
    }


    mat4 P_3D = camera_mesh->get_P();
    mat4 V_3D = camera_mesh->get_V();
    mat4 PV_3D = P_3D * V_3D;

    uint window_width, window_height; {
        vec2 _window_size = window_get_size_Pixel();
        window_width = uint(_window_size.x);
        window_height = uint(_window_size.y);
    }

    { // drawing mesh panes
        bool dragging = (other.mouse_left_drag_pane == Pane::Separator);
        bool hovering = ((other.mouse_left_drag_pane == Pane::None) && (other.hot_pane == Pane::Separator));
        eso_begin(M4_Identity(), SOUP_LINES);
        // eso_overlay(true);
        eso_size(dragging ? 1.0f
                : hovering ? 2.0f
                : 1.5f);
        eso_color(
                dragging ? pallete.white
                : hovering ? pallete.light_gray
                : pallete.gray);
        eso_vertex(other.x_divider_drawing_mesh_OpenGL, -1.0f);
        eso_vertex(other.x_divider_drawing_mesh_OpenGL,  1.0f);
        eso_end();
    }

    real x_divider_drawing_mesh_Pixel = get_x_divider_drawing_mesh_Pixel();

    bool moving_selected_entities = (
            (two_click_command->awaiting_second_click)
            && (0 
                || (state_Draw_command_is_(Move))
                || (state_Draw_command_is_(Rotate))
                || (state_Draw_command_is_(Copy)))
            ); // TODO: loft up

    { // draw 2D draw 2d draw
        auto DRAW_CROSSHAIR = [&](vec2 o, vec3 color) {
            real funky_OpenGL_factor = other.camera_drawing.ortho_screen_height_World / 120.0f;
            eso_begin(PV_2D, SOUP_LINES);
            eso_color(pallete.black);
            real r = 1.3 * funky_OpenGL_factor;
            eso_size(2.0f);
            eso_vertex(o - V2(r, 0));
            eso_vertex(o + V2(r, 0));
            eso_vertex(o - V2(0, r));
            eso_vertex(o + V2(0, r));
            eso_color(color);
            r = 1.2 * funky_OpenGL_factor;
            eso_size(1.0f);
            eso_vertex(o - V2(r, 0));
            eso_vertex(o + V2(r, 0));
            eso_vertex(o - V2(0, r));
            eso_vertex(o + V2(0, r));
            eso_end();
        };

        auto DRAW_BOX = [&](vec2 click_1, vec2 click_2, vec3 color) {
            eso_begin(PV_2D, SOUP_LINE_LOOP);
            eso_color(color);
            eso_vertex(click_1);
            eso_vertex(click_1.x, click_2.y);
            eso_vertex(click_2);
            eso_vertex(click_2.x, click_1.y);
            eso_end();
        };

        auto DRAW_CIRCLE = [&](vec2 click_1, vec2 click_2, vec3 color) {
            vec2 center = click_1;
            real radius = distance(click_1, click_2);
            eso_begin(PV_2D, SOUP_LINE_LOOP);
            eso_color(color);
            for_(i, NUM_SEGMENTS_PER_CIRCLE) {
                real theta = (real(i) / NUM_SEGMENTS_PER_CIRCLE) * TAU;
                eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta));
            }
            eso_end();
        };

        auto DRAW_LINE = [&](vec2 click_1, vec2 click_2, vec3 color) {
            eso_begin(PV_2D, SOUP_LINES);
            eso_color(color);
            eso_vertex(click_1);
            eso_vertex(click_2);
            eso_end();
        };

        auto DRAW_CENTERLINE = [&](vec2 click_1, vec2 click_2, vec3 color) {
            eso_begin(PV_2D, SOUP_LINES);
            eso_color(color);
            eso_vertex(click_1 + (click_1 - click_2));
            eso_vertex(click_2);
            eso_end();
        };

        auto DRAW_POLYGON = [&](vec2 click_1, vec2 click_2, vec3 color) {
            // TODO: JUICEIT_EASYTWEEN polygon_num_sides
            uint polygon_num_sides = uint(preview->polygon_num_sides);
            real delta_theta = -TAU / preview->polygon_num_sides;
            vec2 center = click_1;
            vec2 vertex_0 = click_2;
            real radius = distance(center, vertex_0);
            real theta_0 = ATAN2(vertex_0 - center);
            {
                eso_begin(PV_2D, SOUP_LINES);
                eso_stipple(true);
                eso_color(color);
                eso_vertex(center);
                eso_vertex(vertex_0);
                eso_end();
            }
            {
                eso_begin(PV_2D, SOUP_LINE_LOOP);
                eso_color(color);
                for_(i, polygon_num_sides) {
                    real theta_i = theta_0 + (i * delta_theta);
                    real theta_ip1 = theta_i + delta_theta;
                    eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta_i));
                    eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta_ip1));
                }
                eso_end();
            }
        };

        bool moving = (two_click_command->awaiting_second_click) && (state_Draw_command_is_(Move));
        bool linear_copying = (two_click_command->awaiting_second_click) && (state_Draw_command_is_(Copy));
        bool rotating = (two_click_command->awaiting_second_click) && (state_Draw_command_is_(Rotate));
        // bool moving_linear_copying_or_rotating = (moving || rotating || linear_copying);

        auto DRAW_ENTITIES_BEING_MOVED_LINEAR_COPIED_OR_ROTATED = [&](vec2 click_1, vec2 click_2, vec3 color) {
            // TODO: do this like crosshairs where they disappear more immediatelly
            if (IS_ZERO(squaredNorm(click_1 - click_2))) return; // NOTE: you want this (we're attempthing this call three times!)
            vec2 click_vector_12 = click_2 - click_1;
            real click_theta_12 = ATAN2(click_vector_12);
            mat4 M; {
                if (moving || linear_copying) {
                    M = M4_Translation(click_vector_12);
                } else { ASSERT(rotating);
                    M = M4_Translation(click_1) * M4_RotationAboutZAxis(click_theta_12) * M4_Translation(-click_1);
                }
            }
            eso_begin(PV_2D * M, SOUP_LINES);
            eso_color(color);
            _for_each_selected_entity_ eso_entity__SOUP_LINES(entity);
            eso_end();
        };



        glEnable(GL_SCISSOR_TEST);
        gl_scissor_Pixel(0, 0, x_divider_drawing_mesh_Pixel, window_height);
        {
            if (!other.hide_grid) { // grid 2D grid 2d grid // jim wtf are these supposed to mean
                mat4 PVM = PV_2D * M4_Translation(-GRID_SIDE_LENGTH / 2, -GRID_SIDE_LENGTH / 2);
                eso_begin(PVM, SOUP_LINES);
                eso_color(pallete.darker_gray);
                for (uint i = 0; i <= uint(GRID_SIDE_LENGTH / GRID_SPACING); ++i) {
                    real tmp = i * GRID_SPACING;
                    eso_vertex(tmp, 0.0f);
                    eso_vertex(tmp, GRID_SIDE_LENGTH);
                    eso_vertex(0.0f, tmp);
                    eso_vertex(GRID_SIDE_LENGTH, tmp);
                }
                eso_end();
                eso_begin(PVM, SOUP_LINE_LOOP);
                eso_color(pallete.dark_gray);
                eso_vertex(0.0f, 0.0f);
                eso_vertex(0.0f, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, GRID_SIDE_LENGTH);
                eso_vertex(GRID_SIDE_LENGTH, 0.0f);
                eso_end();
            }
            if (1) { // axes 2D axes 2d axes axis 2D axis 2d axes crosshairs cross hairs origin 2d origin 2D origin
                real funky_OpenGL_factor = other.camera_drawing.ortho_screen_height_World / 120.0f;
                real r = 3 * funky_OpenGL_factor;
                // real LL = 1000 * funky_OpenGL_factor;

                // eso_begin(PV_2D, SOUP_LINES); {
                //     // axis
                //     eso_stipple(true);
                //     eso_color(pallete.dark_gray);
                //     if (state_Draw_command_is_(SetAxis)) {
                //         eso_color(get_color(ColorCode::Emphasis));
                //     } else if (state_Mesh_command_is_(RevolveAdd)) {
                //         eso_color(AVG(pallete.dark_gray, get_color(ColorCode::Emphasis)));
                //     } else if (state_Mesh_command_is_(RevolveCut)) {
                //         eso_color(AVG(pallete.dark_gray, get_color(ColorCode::Emphasis)));
                //     } else {
                //     }
                //     vec2 v = LL * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                //     eso_vertex(preview_dxf_axis_base_point + v);
                //     eso_vertex(preview_dxf_axis_base_point - v);
                // } eso_end();
                eso_begin(PV_2D, SOUP_POINTS); {
                    eso_overlay(true);
                    eso_color(pallete.white);
                    eso_size(6.0f);
                    eso_vertex(target_preview_drawing_origin - V2(0, 0));
                } eso_end();
                vec2 v = r * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                eso_begin(PV_2D, SOUP_LINES); {
                    eso_overlay(true);
                    eso_color(pallete.white);
                    eso_size(3.0f);
                    eso_vertex(preview_dxf_axis_base_point);
                    eso_size(0.0f);
                    eso_vertex(preview_dxf_axis_base_point + v);
                } eso_end();
                // eso_begin(PV_2D, SOUP_TRIANGLES); {
                //     eso_color(pallete.white);
                //     real eps = r / 5;
                //     eso_vertex(target_preview_drawing_origin + V2(r + 2 * eps, 0));
                //     eso_vertex(target_preview_drawing_origin + V2(r,         eps));
                //     eso_vertex(target_preview_drawing_origin + V2(r,        -eps));
                //     eso_vertex(target_preview_drawing_origin + V2(0, r + 2 * eps));
                //     eso_vertex(target_preview_drawing_origin + V2( eps, r));
                //     eso_vertex(target_preview_drawing_origin + V2(-eps, r));
                // } eso_end();
            }



            vec2 *first_click = &two_click_command->first_click;




            vec2 Draw_Enter; {
                Draw_Enter = *first_click;
                if (popup->manager.focus_group == ToolboxGroup::Draw) {
                    if (state_Draw_command_is_(Box)) Draw_Enter += V2(popup->box_width, popup->box_height);
                    if (state_Draw_command_is_(Circle)) Draw_Enter += V2(popup->circle_radius, 0.0f);
                    if (state_Draw_command_is_(Polygon)) Draw_Enter += V2(popup->polygon_distance_to_corner, 0.0f);
                    if (state_Draw_command_is_(Line)) Draw_Enter += V2(popup->line_run, popup->line_rise);
                    if (state_Draw_command_is_(Move)) Draw_Enter += V2(popup->move_run, popup->move_rise);
                    if (state_Draw_command_is_(Copy)) Draw_Enter += V2(popup->linear_copy_run, popup->linear_copy_rise);
                    if (state_Draw_command_is_(Rotate)) {
                        if (!IS_ZERO(popup->rotate_angle)) { // FORNOW
                            Draw_Enter += 10.0f * e_theta(RAD(popup->rotate_angle));
                        }
                    }
                }
            }
            vec2 Snap_Enter; {
                Snap_Enter = *first_click;
                if (popup->manager.focus_group == ToolboxGroup::Snap) {
                    if (state_Snap_command_is_(XY)) {
                        Snap_Enter = V2(popup->xy_x_coordinate, popup->xy_y_coordinate);
                    }
                }
            }

            MagicSnapResult true_snap_result = magic_snap(mouse);

            {
                JUICEIT_EASYTWEEN(&preview->popup_second_click, Draw_Enter);
                JUICEIT_EASYTWEEN(&preview->xy_xy, Snap_Enter);

                JUICEIT_EASYTWEEN(&preview->mouse_snap, true_snap_result.mouse_position, 1.0f);

                JUICEIT_EASYTWEEN(&preview->polygon_num_sides, real(popup->polygon_num_sides));
            }
            bool Snap_eating_mouse = !(state_Snap_command_is_(None) || state_Snap_command_is_(XY));
            vec2 position_mouse; {
                position_mouse = (!Snap_eating_mouse) ? mouse : preview->mouse_snap; // FORNOW
            }
            bool Draw_eating_Enter = ((popup->manager.focus_group == ToolboxGroup::Draw) &&
                    (!two_click_command->awaiting_second_click || !ARE_EQUAL(*first_click, Draw_Enter)));
            bool Snap_eating_Enter = ((popup->manager.focus_group == ToolboxGroup::Snap) && state_Snap_command_is_(XY) &&
                    (!two_click_command->awaiting_second_click || !ARE_EQUAL(*first_click, Snap_Enter)));
            vec3 target_color_mouse; {
                target_color_mouse = get_color(ColorCode::Emphasis);
                // if (Snap_eating_mouse) target_color_mouse = AVG(get_color(ColorCode::Emphasis), get_accent_color(ToolboxGroup::Snap));
                if (Snap_eating_mouse) target_color_mouse = get_accent_color(ToolboxGroup::Snap);
                if  (Draw_eating_Enter) {
                    target_color_mouse = CLAMPED_LERP(_JUICEIT_EASYTWEEN(-0.7f + 1.3f * 
                                MIN(other.time_since_popup_second_click_not_the_same, other.time_since_mouse_moved)
                                ), target_color_mouse, 0.2f * target_color_mouse);
                }
                if  (Snap_eating_Enter) {
                    target_color_mouse = CLAMPED_LERP(_JUICEIT_EASYTWEEN(-0.7f + 1.3f * 
                                MIN(other.time_since_popup_second_click_not_the_same, other.time_since_mouse_moved)
                                ), target_color_mouse, 0.2f * target_color_mouse);
                }
            }
            JUICEIT_EASYTWEEN(&preview->color_mouse, target_color_mouse);



            // vec2 click_vector = (position_mouse - *first_click);
            // real click_theta = ATAN2(click_vector);

            { // entities
                eso_begin(PV_2D, SOUP_LINES); {
                    // entities 2D entities 2d entities drawing 2D drawing 2d drawing
                    _for_each_entity_ {
                        if (entity->is_selected && (rotating || moving)) continue;
                        eso_color((entity->is_selected) ? get_color(ColorCode::Selection) : get_color(entity->color_code));
                        eso_size(1.5f);
                        eso_entity__SOUP_LINES(entity);
                    }
                } eso_end();


                { // dots snap_divide_dot
                    if (other.show_details) { // dots
                        eso_begin(PV_2D, SOUP_POINTS);
                        eso_size(3.0f);
                        eso_color(pallete.white);
                        _for_each_entity_ {
                            if (entity->is_selected && (rotating || moving)) continue;
                            if (entity->type == EntityType::Circle) {
                                CircleEntity *circle = &entity->circle;
                                if (circle->has_pseudo_point) eso_vertex(circle->get_pseudo_point());
                                continue;
                            }
                            eso_vertex(entity_get_start_point(entity));
                            eso_vertex(entity_get_end_point(entity));
                        }
                        eso_end();
                    }

                    { // snap_divide_dot
                        eso_begin(PV_2D, SOUP_POINTS);
                        eso_color(pallete.light_gray);
                        JUICEIT_EASYTWEEN(&other.size_snap_divide_dot, 0.0f, 0.5f);
                        eso_size(other.size_snap_divide_dot);
                        eso_vertex(other.snap_divide_dot);
                        eso_end();
                    }
                }
            }

            { // annotations

                // new-style annotations
                // FORNOW (this is sloppy and bad)
                #define ANNOTATION(Name, NAME) \
                do { \
                    if (state_Draw_command_is_(Name)) { \
                        DRAW_##NAME(*first_click, position_mouse, preview->color_mouse); \
                        DRAW_##NAME(*first_click, preview->popup_second_click, get_accent_color(ToolboxGroup::Draw)); \
                        DRAW_##NAME(*first_click, preview->xy_xy, get_accent_color(ToolboxGroup::Snap)); \
                    } \
                } while (0)

                if (two_click_command->awaiting_second_click) {
                    ANNOTATION(Line, LINE);
                    ANNOTATION(CenterLine, CENTERLINE);
                    ANNOTATION(Box, BOX);
                    ANNOTATION(Circle, CIRCLE);
                    ANNOTATION(Polygon, POLYGON);

                    ANNOTATION(Move, LINE);
                    ANNOTATION(Copy, LINE);
                    ANNOTATION(Rotate, LINE);
                    // NOTE: this is still kinda broken
                    ANNOTATION(Move, ENTITIES_BEING_MOVED_LINEAR_COPIED_OR_ROTATED);
                    ANNOTATION(Copy, ENTITIES_BEING_MOVED_LINEAR_COPIED_OR_ROTATED);
                    ANNOTATION(Rotate, ENTITIES_BEING_MOVED_LINEAR_COPIED_OR_ROTATED);
                }

                { // entity snapped to
                    // TODO: Intersect
                    if (true_snap_result.snapped) {
                        Entity *entity_snapped_to = &drawing->entities.array[true_snap_result.entity_index_snapped_to];
                        eso_begin(PV_2D, SOUP_LINES);
                        // eso_overlay(true);
                        eso_color(get_accent_color(ToolboxGroup::Snap));
                        eso_entity__SOUP_LINES(entity_snapped_to);
                        eso_end();
                    }
                }

                { // crosshairs
                    if (state_Snap_command_is_(XY)) {
                        if (popup->manager.focus_group == ToolboxGroup::Snap) {
                            if (!Snap_eating_Enter) other.time_since_popup_second_click_not_the_same = 0.0f;
                        }
                        if (Snap_eating_Enter) DRAW_CROSSHAIR(preview->xy_xy, get_accent_color(ToolboxGroup::Snap));
                    } else if (!state_Snap_command_is_(None)) {
                        DRAW_CROSSHAIR(preview->mouse_snap, get_accent_color(ToolboxGroup::Snap));
                    }

                    if (two_click_command->awaiting_second_click && !state_Draw_command_is_(None)) {
                        if (popup->manager.focus_group == ToolboxGroup::Draw) {
                            if (!Draw_eating_Enter) other.time_since_popup_second_click_not_the_same = 0.0f;
                        }
                        if (Draw_eating_Enter) DRAW_CROSSHAIR(preview->popup_second_click, pallete.cyan);
                    }
                }

                { // experimental preview part B
                  // NOTE: circle <-> circle is wonky
                    if (state_Draw_command_is_(Offset)) {
                        DXFFindClosestEntityResult closest_result = dxf_find_closest_entity(&drawing->entities, mouse);
                        if (closest_result.success) {
                            Entity *_closest_entity = closest_result.closest_entity;
                            Entity target_entity = entity_offsetted(_closest_entity, popup->offset_distance, mouse);
                            vec2 target_start, target_end, target_middle, target_opposite;
                            if (target_entity.type != EntityType::Circle) {
                                entity_get_start_and_end_points(&target_entity, &target_start, &target_end);
                                target_middle = entity_get_middle(&target_entity);
                                target_opposite = target_middle;
                            } else { ASSERT(target_entity.type == EntityType::Circle);
                                CircleEntity *circle = &target_entity.circle;
                                real angle; {
                                    if (ARE_EQUAL(preview->offset_entity_end, preview->offset_entity_start)) {
                                        angle = 0.0;
                                    } else {
                                        angle = (PI / 2) - ATAN2(normalized(preview->offset_entity_end - preview->offset_entity_start));
                                    }
                                }
                                // real angle = ATAN2(preview->offset_entity_middle - circle->center);
                                // real angle = ATAN2(mouse - circle->center);
                                // TODO: something else?
                                target_middle   = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle);
                                target_start    = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle - PI / 2);
                                target_end      = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle + PI / 2);
                                target_opposite = get_point_on_circle_NOTE_pass_angle_in_radians(circle->center, circle->radius, angle + PI);

                            }

                            // reasonable line <-> arc behavior
                            if (1) { // heuristic (FORNOW: minimize max distance)
                                real D2na = squaredDistance(preview->offset_entity_start, target_start);
                                real D2nb = squaredDistance(preview->offset_entity_end, target_end);
                                real D2ya = squaredDistance(preview->offset_entity_start, target_end);
                                real D2yb = squaredDistance(preview->offset_entity_end, target_start);
                                real max_D2_no_swap = MAX(D2na, D2nb);
                                real max_D2_yes_swap = MAX(D2ya, D2yb);
                                if (max_D2_no_swap > max_D2_yes_swap) {
                                    SWAP(&target_start, &target_end);
                                }
                            }

                            JUICEIT_EASYTWEEN(&preview->offset_entity_start, target_start);
                            JUICEIT_EASYTWEEN(&preview->offset_entity_end, target_end);
                            JUICEIT_EASYTWEEN(&preview->offset_entity_middle, target_middle);
                            JUICEIT_EASYTWEEN(&preview->offset_entity_opposite, target_opposite);

                            // TODO: could try a crescent moon kind of a situation
                            // TODO: just need a three point arc lambda
                            //       (and could in theory fillet the arcs)

                            vec2 a = preview->offset_entity_start;
                            vec2 b = preview->offset_entity_middle;
                            vec2 c = preview->offset_entity_end;
                            vec2 d = preview->offset_entity_opposite;
                            Entity dummy = entity_make_three_point_arc_or_line(a, b, c);
                            Entity dummy2 = entity_make_three_point_arc_or_line(a, d, c);
                            eso_begin(PV_2D, SOUP_LINES);
                            { // eso_vertex
                              // eso_color(1.0f, 0.0f, 1.0f);
                              // eso_vertex(a);
                              // eso_vertex(b);
                              // eso_vertex(b);
                              // eso_vertex(c);
                              // eso_color((distance(b, d) / distance(a, c)) * get_color(ColorCode::Emphasis));
                                eso_color(get_color(ColorCode::Emphasis));
                                eso_entity__SOUP_LINES(&dummy2);
                                eso_color(get_color(ColorCode::Emphasis));
                                eso_entity__SOUP_LINES(&dummy);
                            }
                            eso_end();
                        }
                    }
                }

                // FORNOW: old-style annotations
                if (!two_click_command->awaiting_second_click) {
                } else {
                    if (state_Xsel_command_is_(Window)) {
                        eso_begin(PV_2D, SOUP_LINE_LOOP);
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(first_click->x, first_click->y);
                        eso_vertex(mouse.x, first_click->y);
                        eso_vertex(mouse.x, mouse.y);
                        eso_vertex(first_click->x, mouse.y);
                        eso_end();
                    }

                    if (state_Draw_command_is_(CenterBox)) {                
                        vec2 one_corner = mouse;
                        vec2 center = *first_click;
                        real other_y = 2 * center.y - one_corner.y;
                        real other_x = 2 * center.x - one_corner.x;
                        eso_begin(PV_2D, SOUP_LINE_LOOP);
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(one_corner);
                        eso_vertex(V2(one_corner.x, other_y));
                        eso_vertex(V2(other_x, other_y));
                        if (two_click_command->tangent_first_click) {
                            two_click_command->tangent_first_click = false;
                        }
                        eso_vertex(V2(other_x, one_corner.y));
                        eso_end();
                    }
                    if (state_Draw_command_is_(Measure)) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(two_click_command->first_click);
                        eso_vertex(mouse);
                        eso_end();
                    }
                    if (state_Draw_command_is_(Mirror2)) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(two_click_command->first_click);
                        eso_vertex(mouse);
                        eso_end();
                    }
                    if (state_Draw_command_is_(Rotate)) {
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(two_click_command->first_click);
                        eso_vertex(mouse);
                        eso_end();
                    }
                    if (state_Draw_command_is_(DiamCircle)) {
                        vec2 edge_one = two_click_command->first_click;
                        vec2 edge_two = mouse;
                        vec2 center = (edge_one + edge_two) / 2;
                        real radius = norm(edge_one - center);
                        eso_begin(PV_2D, SOUP_LINE_LOOP);
                        eso_color(get_color(ColorCode::Emphasis));
                        for_(i, NUM_SEGMENTS_PER_CIRCLE) {
                            real theta = (real(i) / NUM_SEGMENTS_PER_CIRCLE) * TAU;
                            eso_vertex(get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta));
                        }
                        eso_end();
                        eso_begin(PV_2D, SOUP_LINES);
                        eso_stipple(true);
                        eso_color(get_color(ColorCode::Emphasis));
                        eso_vertex(edge_one);
                        eso_vertex(edge_two);
                        eso_end();
                    }
                    if (state_Draw_command_is_(Divide2)) {
                        if (two_click_command->awaiting_second_click) {
                            eso_begin(PV_2D, SOUP_LINES);
                            eso_color(get_color(ColorCode::Emphasis));
                            eso_entity__SOUP_LINES(two_click_command->entity_closest_to_first_click);
                            eso_end();
                        }
                    }
                    if (state_Draw_command_is_(Fillet)) {
                        if (two_click_command->awaiting_second_click) {
                            eso_begin(PV_2D, SOUP_LINES);
                            eso_color(get_color(ColorCode::Emphasis));
                            eso_entity__SOUP_LINES(two_click_command->entity_closest_to_first_click);
                            eso_end();
                        }
                    }
                    if (state_Draw_command_is_(DogEar)) {
                        if (two_click_command->awaiting_second_click) {
                            eso_begin(PV_2D, SOUP_LINES);
                            eso_color(get_color(ColorCode::Emphasis));
                            eso_entity__SOUP_LINES(two_click_command->entity_closest_to_first_click);
                            eso_end();
                        }
                    }
                }
            }
        }
        glDisable(GL_SCISSOR_TEST);
    }





    { // 3D draw 3D 3d draw 3d
        {
            glEnable(GL_SCISSOR_TEST);
            gl_scissor_Pixel(x_divider_drawing_mesh_Pixel, 0, window_width - x_divider_drawing_mesh_Pixel, window_height);
        }


        mat4 inv_T_o = M4_Translation(-preview->drawing_origin);
        if (feature_plane->is_active) { // selection 2d selection 2D selection tube tubes slice slices stack stacks wire wireframe wires frame (FORNOW: ew)
            ;
            // FORNOW
            bool moving_stuff = ((state_Draw_command_is_(SetOrigin)) || (state_Mesh_command_is_(NudgePlane)));
            vec3 target_preview_tubes_color = (0) ? V3(0)
                : (moving_selected_entities) ? get_color(ColorCode::Emphasis)
                : (adding) ? pallete.green
                : (cutting) ? pallete.red
                : (moving_stuff) ? get_color(ColorCode::Emphasis)
                : get_color(ColorCode::Selection);
            JUICEIT_EASYTWEEN(&preview->tubes_color, target_preview_tubes_color);

            uint NUM_TUBE_STACKS_INCLUSIVE;
            mat4 M;
            mat4 M_incr;
            {
                // mat4 T_o = M4_Translation(preview->drawing_origin);
                if (extruding) {
                    real a = -preview->extrude_in_length;
                    real L = preview->extrude_out_length + preview->extrude_in_length;
                    NUM_TUBE_STACKS_INCLUSIVE = MIN(64U, uint(ROUND(L / 2.5f)) + 2);
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0.0f, 0.0f, a + Z_FIGHT_EPS);
                    M_incr = M4_Translation(0.0f, 0.0f, L / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                } else if (revolving) {
                    real a = -RAD(preview->revolve_out_angle);
                    real b = RAD(preview->revolve_in_angle);
                    real L = b - a;
                    NUM_TUBE_STACKS_INCLUSIVE = MIN(64U, uint(ROUND(L / 0.1f)) + 2);
                    vec3 axis = V3(e_theta(PI / 2 + preview_dxf_axis_angle_from_y), 0.0f);
                    mat4 R_0 = M4_RotationAbout(axis, a);
                    mat4 R_inc = M4_RotationAbout(axis, L / (NUM_TUBE_STACKS_INCLUSIVE - 1));
                    mat4 T_a = M4_Translation(V3(preview_dxf_axis_base_point, 0.0f));
                    mat4 inv_T_a = inverse(T_a);
                    // M_incr = T_o * T_a * R_a * inv_T_a * inv_T_o;
                    M_incr = T_a * R_inc * inv_T_a;
                    M = M_3D_from_2D * inv_T_o * T_a * R_0 * inv_T_a;
                } else if (state_Draw_command_is_(SetOrigin)) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0, 0, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else if (state_Mesh_command_is_(NudgePlane)) {
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0.0f, 0.0f, preview->feature_plane_offset + Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                } else { // default
                    NUM_TUBE_STACKS_INCLUSIVE = 1;
                    M = M_3D_from_2D * inv_T_o * M4_Translation(0, 0, Z_FIGHT_EPS);
                    M_incr = M4_Identity();
                }

                mat4 T_Move;
                if (moving_selected_entities) {
                    T_Move = M4_Translation(preview->mouse - two_click_command->first_click);
                } else {
                    T_Move = M4_Identity();
                }
                for_(tube_stack_index, NUM_TUBE_STACKS_INCLUSIVE) {
                    eso_begin(PV_3D * M * T_Move, SOUP_LINES); {
                        _for_each_selected_entity_ {
                            real alpha;
                            vec3 color;
                            // if (entity->is_selected) {
                            alpha = CLAMP(-0.2f + 3.0f * MIN(entity->time_since_is_selected_changed, other.time_since_plane_selected), 0.0f, 1.0f);
                            color = CLAMPED_LERP(-0.5f + SQRT(2.0f * entity->time_since_is_selected_changed), pallete.white, preview->tubes_color);
                            // } else {
                            //     alpha = CLAMPED_LERP(5.0f * entity->time_since_is_selected_changed, 1.0f, 0.0f);
                            //     color = get_color(color);
                            // }
                            eso_color(color, alpha);
                            eso_entity__SOUP_LINES(entity);
                        }
                    } eso_end();
                    M *= M_incr;
                }
            }
        }

        if (feature_plane->is_active) { // axes 3D axes 3d axes axis 3D axis 3d axis
            real r = other.camera_mesh.ortho_screen_height_World / 100.0f;
            eso_begin(PV_3D * M_3D_from_2D * M4_Translation(0.0f, 0.0f, Z_FIGHT_EPS), SOUP_LINES);
            eso_color(pallete.white);
            eso_vertex(0, 0.0f);
            eso_vertex( r, 0.0f);
            eso_vertex(0.0f, 0);
            eso_vertex(0.0f,  r);
            if (revolving) {
                // TODO: clip this to the feature_plane
                real LL = 100.0f;
                vec2 v = LL * e_theta(PI / 2 + preview_dxf_axis_angle_from_y);
                vec2 a = preview_dxf_axis_base_point + v;
                vec2 b = preview_dxf_axis_base_point - v;
                eso_color(get_color(ColorCode::Emphasis));
                eso_vertex(-preview->drawing_origin + a);
                eso_vertex(-preview->drawing_origin + b); // FORNOW
            }
            eso_end();
        }

        if (!other.hide_grid) { // grid 3D grid 3d grid
            for_(k, 6) {
                real r = 0.5f * GRID_SIDE_LENGTH;
                mat4 M0 = M4_Translation(-r, -r, r);
                mat4 M1; {
                    M1 = {}; // FORNOW compiler warning
                    if (k == 0) M1 = M4_Identity();
                    if (k == 1) M1 = M4_RotationAboutYAxis(PI);
                    if (k == 2) M1 = M4_RotationAboutXAxis( PI / 2);
                    if (k == 3) M1 = M4_RotationAboutXAxis(-PI / 2);
                    if (k == 4) M1 = M4_RotationAboutYAxis( PI / 2);
                    if (k == 5) M1 = M4_RotationAboutYAxis(-PI / 2);
                }
                mat4 PVM1 = PV_3D * M1;
                { // backface culling (check sign of rasterized triangle)
                    vec2 a = _V2(transformPoint(PVM1, V3(0.0f, 0.0f, r)));
                    vec2 b = _V2(transformPoint(PVM1, V3(1.0f, 0.0f, r)));
                    vec2 c = _V2(transformPoint(PVM1, V3(0.0f, 1.0f, r)));
                    if (cross(b - a , c - a) > 0.0f) continue;
                }
                mat4 transform = PVM1 * M0;
                eso_begin(transform, SOUP_LINES);
                eso_color(pallete.darker_gray);
                eso_size(2.0f);
                for (uint i = 0; i <= uint(GRID_SIDE_LENGTH / GRID_SPACING); ++i) {
                    real tmp = i * GRID_SPACING;
                    eso_vertex(tmp, 0.0f);
                    eso_vertex(tmp, GRID_SIDE_LENGTH);
                    eso_vertex(0.0f, tmp);
                    eso_vertex(GRID_SIDE_LENGTH, tmp);
                }
                eso_end();
                // eso_size(4.0f);
                // eso_begin(transform, SOUP_LINE_LOOP);
                // eso_color(pallete.dark_gray);
                // eso_vertex(0.0f, 0.0f);
                // eso_vertex(0.0f, GRID_SIDE_LENGTH);
                // eso_vertex(GRID_SIDE_LENGTH, GRID_SIDE_LENGTH);
                // eso_vertex(GRID_SIDE_LENGTH, 0.0f);
                // eso_end();
            }
        }

        { // feature plane feature-plane feature_plane // floating sketch plane; selection plane NOTE: transparent
            {
                bbox2 face_selection_bbox; {
                    face_selection_bbox = mesh_draw(P_3D, V_3D, M4_Identity());
                }
                bbox2 dxf_selection_bbox; {
                    dxf_selection_bbox = entities_get_bbox(&drawing->entities, true);

                    // TODO: this should incorporate a preview of the fact that some entities are moving
                    if (moving_selected_entities) {
                        vec2 T = (preview->mouse - two_click_command->first_click);
                        dxf_selection_bbox.min += T;
                        dxf_selection_bbox.max += T;
                    }

                    dxf_selection_bbox.min -= target_preview_drawing_origin;
                    dxf_selection_bbox.max -= target_preview_drawing_origin;
                }
                bbox2 target_bbox; {
                    target_bbox = face_selection_bbox + dxf_selection_bbox;
                    for_(d, 2) {
                        if (target_bbox.min[d] > target_bbox.max[d]) {
                            target_bbox.min[d] = 0.0f;
                            target_bbox.max[d] = 0.0f;
                        }
                    }
                    {
                        real eps = 10.0f;
                        target_bbox.min[0] -= eps;
                        target_bbox.max[0] += eps;
                        target_bbox.min[1] -= eps;
                        target_bbox.max[1] += eps;
                    }
                }
                if (!feature_plane->is_active) {
                    target_bbox.min -= V2(10.0f);
                    target_bbox.max += V2(10.0f);
                }
                JUICEIT_EASYTWEEN(&preview->feature_plane.min, target_bbox.min);
                JUICEIT_EASYTWEEN(&preview->feature_plane.max, target_bbox.max);
                // if (other.time_since_plane_selected == 0.0f) { // FORNOW
                //     preview->feature_plane = target_bbox;
                // }
            }

            {
                mat4 PVM = PV_3D * M_3D_from_2D;
                vec3 target_feature_plane_color = get_color(ColorCode::Selection);
                {
                    if (state_Mesh_command_is_(NudgePlane)) {
                        PVM *= M4_Translation(0.0f, 0.0f, preview->feature_plane_offset);
                        target_feature_plane_color = get_color(ColorCode::Emphasis); 
                    } else if (state_Draw_command_is_(SetOrigin)) {
                        target_feature_plane_color = get_color(ColorCode::Emphasis); 
                    } else if (moving_selected_entities) {
                        target_feature_plane_color = get_color(ColorCode::Emphasis); 
                    }
                }

                JUICEIT_EASYTWEEN(&preview->feature_plane_color, target_feature_plane_color);

                {
                    real f = CLAMPED_LERP(SQRT(other.time_since_plane_deselected), 1.0f, 0.0f);
                    if (feature_plane->is_active) f = CLAMPED_LERP(SQRT(3.0f * other.time_since_plane_selected), f, 1.0f);
                    // vec2 center = (preview->feature_plane.max + preview->feature_plane.min) / 2.0f;
                    // mat4 scaling_about_center = M4_Translation(center) * M4_Scaling(f) * M4_Translation(-center);
                    eso_begin(PVM * M4_Translation(0.0f, 0.0f, Z_FIGHT_EPS)/* * scaling_about_center*/, SOUP_QUADS);
                    eso_color(preview->feature_plane_color, f * 0.4f);
                    eso_bbox_SOUP_QUADS(preview->feature_plane);
                    eso_end();
                }
            }
        }

        MagicSnapResult3D snap_result = magic_snap_3d();
        if (snap_result.hit_mesh) {
            eso_begin(PV_3D, SOUP_POINTS);
            eso_size(20);
            eso_color(get_color(ColorCode::Emphasis));
            eso_vertex(snap_result.mouse_position);
            eso_end();
        }

        if (!mesh_two_click_command->awaiting_second_click) {

        } else if (state_Mesh_command_is_(Measure3D)) {
            eso_begin(PV_3D, SOUP_POINTS);
            eso_size(20);
            eso_color(get_color(ColorCode::Emphasis));
            eso_vertex(mesh_two_click_command->first_click);
            eso_end();

            if (snap_result.hit_mesh) {
                eso_begin(PV_3D, SOUP_LINES);
                eso_color(get_color(ColorCode::Emphasis));
                eso_vertex(mesh_two_click_command->first_click);
                eso_vertex(snap_result.mouse_position);
                eso_end();
            }
        }

        glDisable(GL_SCISSOR_TEST);
    }


    { // cursor

        bool drag_none = (other.mouse_left_drag_pane == Pane::None);
        bool drag_drawing = (other.mouse_left_drag_pane == Pane::Drawing);
        bool drag_popup = (other.mouse_left_drag_pane == Pane::Popup);
        bool drag_separator = (other.mouse_left_drag_pane == Pane::Separator);
        bool drag_toolbox = (other.mouse_left_drag_pane == Pane::Toolbox);
        bool hot_popup = (other.hot_pane == Pane::Popup);
        bool hot_drawing = (other.hot_pane == Pane::Drawing);
        bool hot_separator = (other.hot_pane == Pane::Separator);
        bool hot_toolbox = (other.hot_pane == Pane::Toolbox);
        bool drag_none_and_hot_popup = (drag_none && hot_popup);
        bool drag_none_and_hot_separator = (drag_none && hot_separator);
        bool drag_none_and_hot_drawing = (drag_none && hot_drawing);
        bool drag_none_and_hot_toolbox = (drag_none && hot_toolbox);

        {
            GLFWcursor *next; {
                if (drag_none_and_hot_popup || drag_popup) {
                    next = other.cursors.ibeam;
                } else if (drag_none_and_hot_separator || drag_separator) {
                    next = other.cursors.hresize;
                } else if (drag_none_and_hot_drawing || drag_drawing) {
                    if (state.Draw_command.flags & SNAPPER) {
                        next = other.cursors.crosshair;
                    } else {
                        next = NULL;
                    }
                } else if (drag_none_and_hot_toolbox || drag_toolbox) {
                    next = other.cursors.hand;
                } else {
                    next = NULL;
                }
            }
            if (other.cursors.curr != next) {
                other.cursors.curr = next;
                glfwSetCursor(glfw_window, next);
            }
        }

        {
            real target = (drag_none_and_hot_drawing || drag_drawing) ? 1.0f : 0.0f;
            JUICEIT_EASYTWEEN(&preview->cursor_subtext_alpha, target, 2.0f);
        }
        vec3 color = pallete.white;
        // {
        //     color = pallete.white;
        //     if ((state_Draw_command_is_(SetColor)) && (state.click_modifier != ClickModifier::OfSelection)) {
        //         color = get_color(state.click_color_code);
        //     }
        // }

        // TODO: somehow macro this

        String STRING_EMPTY_STRING = {};
        String Top_string = (state_Draw_command_is_(None)) ? STRING_EMPTY_STRING : state.Draw_command.name;
        String Bot_string; {
            if (0) ;
            else if (!state_Snap_command_is_(None)) Bot_string = state.Snap_command.name;
            else if (!state_Xsel_command_is_(None)) Bot_string = state.Xsel_command.name;
            else if (!state_Colo_command_is_(None)) Bot_string = state.Colo_command.name;
            else Bot_string = STRING("");
        }

        #if 0
        { // spoof callback_cursor_position
            double xpos, ypos;
            glfwGetCursorPos(glfw_window, &xpos, &ypos);
            void callback_cursor_position(GLFWwindow *, double xpos, double ypos);
            callback_cursor_position(NULL, xpos, ypos);
        }
        #endif

        EasyTextPen pen = { other.mouse_Pixel + V2(12.0f, 16.0f), 12.0f, color, true, 1.0f - preview->cursor_subtext_alpha };
        easy_text_draw(&pen, Top_string);
        easy_text_draw(&pen, Bot_string);
    }

    void history_debug_draw(); // forward declaration


#if 0
    if (other.show_help) {
        eso_begin(M4_Identity(), SOUP_QUADS); {
            eso_overlay(true);
            eso_color(pallete.black, 0.7f);
            eso_vertex(-1.0f, -1.0f);
            eso_vertex(-1.0f,  1.0f);
            eso_vertex( 1.0f,  1.0f);
            eso_vertex( 1.0f, -1.0f);
        } eso_end();

        auto command_to_string = [](Command command) -> char* {

            bool control = command.mods & MOD_CTRL;
            bool shift = command.mods & MOD_SHIFT;
            bool alt = command.mods & MOD_ALT;
            KeyEvent tmp = { {}, command.key, control, shift, alt }; 

            return key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(&tmp);
        };
        EasyTextPen pen1 = { V2(25.0f, 16.0f), 16.0f, pallete.white, true}; // FORNOW
        #define PRINT_COMMAND(PEN, NAME) \
        easy_text_drawf(PEN, "  %s: %s", #NAME, \
                command_to_string(commands.NAME));
        EasyTextPen pen2 = pen1;
        pen2.origin.x += 450.0f;


        //////////////////////////////////////////
        //////  SNAP COMMANDS  ///////////////////
        //////////////////////////////////////////

        easy_text_drawf(&pen1, "SNAP COMMANDS\n");
        PRINT_COMMAND(&pen1, Center);
        PRINT_COMMAND(&pen1, End);
        PRINT_COMMAND(&pen1, Middle);
        PRINT_COMMAND(&pen1, Perp);
        PRINT_COMMAND(&pen1, Quad);
        PRINT_COMMAND(&pen1, XY);
        PRINT_COMMAND(&pen1, Zero);


        //////////////////////////////////////////
        //////  Select COMMANDS  /////////////////
        //////////////////////////////////////////

        easy_text_drawf(&pen1, "\nSELECT COMMANDS\n");
        PRINT_COMMAND(&pen1, SetColor);
        PRINT_COMMAND(&pen1, Connected);
        PRINT_COMMAND(&pen1, All);
        PRINT_COMMAND(&pen1, Window);


        //////////////////////////////////////////
        //////  OTHER COMMANDS  //////////////////
        //////////////////////////////////////////

        easy_text_drawf(&pen1, "\nOTHER COMMANDS\n");
        PRINT_COMMAND(&pen1, SetAxis);
        PRINT_COMMAND(&pen1, Box);
        PRINT_COMMAND(&pen1, SetOrigin);
        PRINT_COMMAND(&pen1, Circle);
        PRINT_COMMAND(&pen1, ClearDrawing);
        PRINT_COMMAND(&pen1, ClearMesh);
        PRINT_COMMAND(&pen1, CyclePlane);
        PRINT_COMMAND(&pen1, DELETE_SELECTED); // TODO
        PRINT_COMMAND(&pen1, DELETE_SELECTED_ALTERNATE); //TODO
        PRINT_COMMAND(&pen1, Deselect);
        PRINT_COMMAND(&pen1, DIVIDE_NEAREST);
        PRINT_COMMAND(&pen1, ZoomDrawing);
        PRINT_COMMAND(&pen1, Escape); // TODO
        PRINT_COMMAND(&pen1, ExtrudeAdd);
        PRINT_COMMAND(&pen1, ExtrudeCut);
        PRINT_COMMAND(&pen1, Fillet);
        PRINT_COMMAND(&pen1, HELP_MENU);
        PRINT_COMMAND(&pen1, Line);
        PRINT_COMMAND(&pen1, Copy);
        PRINT_COMMAND(&pen1, Measure);
        PRINT_COMMAND(&pen1, Mirror2);
        PRINT_COMMAND(&pen1, MirrorX);
        PRINT_COMMAND(&pen1, MirrorY);
        PRINT_COMMAND(&pen2, Move);
        PRINT_COMMAND(&pen2, NEXT_POPUP_BAR);
        PRINT_COMMAND(&pen2, NudgePlane);
        PRINT_COMMAND(&pen2, Offset);
        PRINT_COMMAND(&pen1, OpenDXF);
        PRINT_COMMAND(&pen1, OpenSTL);
        PRINT_COMMAND(&pen2, Polygon);
        PRINT_COMMAND(&pen2, POWER_FILLET);
        PRINT_COMMAND(&pen2, PREVIOUS_HOT_KEY_2D);
        PRINT_COMMAND(&pen2, PREVIOUS_HOT_KEY_3D);
        PRINT_COMMAND(&pen2, PRINT_HISTORY);
        PRINT_COMMAND(&pen2, Color0);
        PRINT_COMMAND(&pen2, Color1);
        PRINT_COMMAND(&pen2, Color2);
        PRINT_COMMAND(&pen2, Color3);
        PRINT_COMMAND(&pen2, Color4);
        PRINT_COMMAND(&pen2, Color5);
        PRINT_COMMAND(&pen2, Color6);
        PRINT_COMMAND(&pen2, Color7);
        PRINT_COMMAND(&pen2, Color8);
        PRINT_COMMAND(&pen2, Color9);
        PRINT_COMMAND(&pen2, Redo);
        PRINT_COMMAND(&pen2, REDO_ALTERNATE);
        PRINT_COMMAND(&pen2, Scale);
        PRINT_COMMAND(&pen2, RevolveAdd);
        PRINT_COMMAND(&pen2, RevolveCut);
        PRINT_COMMAND(&pen2, Rotate);
        PRINT_COMMAND(&pen2, RCopy);
        PRINT_COMMAND(&pen2, SaveDXF);
        PRINT_COMMAND(&pen2, SaveSTL);
        PRINT_COMMAND(&pen2, Select);
        PRINT_COMMAND(&pen2, TOGGLE_BUTTONS);
        PRINT_COMMAND(&pen2, TOGGLE_DRAWING_DETAILS);
        PRINT_COMMAND(&pen2, TOGGLE_EVENT_STACK);
        PRINT_COMMAND(&pen2, HidePlane);
        PRINT_COMMAND(&pen2, TOGGLE_GRID);
        PRINT_COMMAND(&pen2, TOGGLE_LIGHT_MODE);
        PRINT_COMMAND(&pen2, Divide2);
        PRINT_COMMAND(&pen2, DiamCircle);
        PRINT_COMMAND(&pen2, Undo);
        PRINT_COMMAND(&pen2, UNDO_ALTERNATE);
        PRINT_COMMAND(&pen2, ZoomMesh);
    }
#endif

    if (other.show_event_stack) history_debug_draw();

    { // paused; slowmo
        real x = 12.0f;
        real y = window_get_height_Pixel() - 12.0f;
        real w = 6.0f;
        real h = 2.5f * w;
        if (other.paused) {
            eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
            eso_overlay(true);
            eso_color(pallete.green);
            for_(i, 2) {
                real o = i * (1.7f * w);
                eso_vertex(x     + o, y    );
                eso_vertex(x     + o, y - h);
                eso_vertex(x + w + o, y - h);
                eso_vertex(x + w + o, y    );
            }
            eso_end();
        }
        if (other.slowmo) {
            eso_begin(other.OpenGL_from_Pixel, SOUP_TRIANGLES);
            eso_overlay(true);
            eso_color(pallete.yellow);
            {
                eso_vertex(x    , y - h);
                eso_vertex(x    , y    );
                eso_vertex(x + h, y    );
            }
            eso_end();
        }
    }

    { // details
        uint num_lines;
        uint num_arcs;
        uint num_circles;
        {
            num_lines = 0;
            num_arcs = 0;
            num_circles = 0;
            _for_each_entity_ {
                if (entity->type == EntityType::Line) {
                    ++num_lines;
                } else if (entity->type == EntityType::Arc) {
                    ++num_arcs;
                } else { ASSERT(entity->type == EntityType::Circle);
                    ++num_circles;
                }
            }
        }

        real height = 12.0f;
        EasyTextPen pen = { V2(96.0f, window_get_height_Pixel() - 13.0f), height, 0.5f * get_accent_color(ToolboxGroup::Draw) };
        easy_text_drawf(&pen, "%d lines %d arcs %d circles", num_lines, num_arcs, num_circles);
        pen = { V2(get_x_divider_drawing_mesh_Pixel() + 7.0f, window_get_height_Pixel() - 13.0f), height, 0.5f * get_accent_color(ToolboxGroup::Mesh) };
        easy_text_drawf(&pen, "%d triangles", mesh->num_triangles);
    }

}

// <!> End draw.cpp <!>
// <!> Begin save_and_load.cpp <!> 
void entities_load(String filename, List<Entity> *entities) {
    #if 0
    {
        FORNOW_UNUSED(filename);
        return {};
        #elif 0
        List<Entity> result = {};
        result.num_entities = 8;
        result.entities = (Entity *) calloc(result.num_entities, sizeof(Entity));
        result.entities[0] = { EntityType::Line, 0, 0.0, 0.0, 1.0, 0.0 };
        result.entities[1] = { EntityType::Line, 1, 1.0, 0.0, 1.0, 1.0 };
        result.entities[2] = { EntityType::Line, 2, 0.0, 1.0, 0.0, 0.0 };
        result.entities[3] = { EntityType::Arc,  3, 0.5, 1.0, 0.5,    0.0, 180.0 };
        result.entities[4] = { EntityType::Arc,  4, 0.5, 1.0, 0.25,   0.0, 180.0 };
        result.entities[5] = { EntityType::Arc,  5, 0.5, 1.0, 0.25, 180.0, 360.0 };
        result.entities[6] = { EntityType::Arc,  6, 0.5, 1.0, 0.1,    0.0, 180.0 };
        result.entities[7] = { EntityType::Arc,  7, 0.5, 1.0, 0.1,  180.0, 360.0 };
        return result;
    }
    #endif
    list_free_AND_zero(entities);

    FILE *file = (FILE *) FILE_OPEN(filename, "r");
    ASSERT(file);

    bool inches = true; // if none read will assume inches
    
    *entities = {}; {
        #define PARSE_NONE   0
        #define PARSE_LINE   1
        #define PARSE_ARC    2
        #define PARSE_CIRCLE 3
        #define PARSE_UNITS  4
        uint mode = 0;

        auto convert = [&](real value) {
            if (inches && !config.usingInches) return MM(value);
            else if (!inches && config.usingInches) return INCHES(value);
            return value;
        };
        
        int code = 0;
        bool code_is_hot = false;
        Entity entity = {};
        #define MAX_LINE_LENGTH 1024
        static _STRING_CALLOC(line_from_file, MAX_LINE_LENGTH);
        while (string_read_line_from_file(&line_from_file, MAX_LINE_LENGTH, file)) {
            if (mode == PARSE_NONE) {
                if (string_matches_prefix(line_from_file, STRING("$INSUNITS"))) {
                    mode = PARSE_UNITS;
                    code_is_hot = false;
                } else if (string_matches_prefix(line_from_file, STRING("LINE"))) {
                    mode = PARSE_LINE;
                    code_is_hot = false;
                    entity = {};
                    entity.type = EntityType::Line;
                } else if (string_matches_prefix(line_from_file, STRING("ARC"))) {
                    mode = PARSE_ARC;
                    code_is_hot = false;
                    entity = {};
                    entity.type = EntityType::Arc;
                } else if (string_matches_prefix(line_from_file, STRING("CIRCLE"))) {
                    mode = PARSE_CIRCLE;
                    code_is_hot = false;
                    entity = {};
                    entity.type = EntityType::Circle;
                }
            } else {
                if (!code_is_hot) {
                    sscanf(line_from_file.data, "%d", &code);
                    // NOTE this initialization is sketchy but works
                    // probably don't make a habit of it
                    if (code == 0) {
                        list_push_back(entities, entity);
                        mode = PARSE_NONE;
                        code_is_hot = false;
                    }
                } else {
                    if (code == 62) {
                        int value;
                        sscanf(line_from_file.data, "%d", &value);
                        entity.color_code = (ColorCode) value; 
                    } else {
                        float value;
                        sscanf(line_from_file.data, "%f", &value);
                        if (mode == PARSE_LINE) {
                            if (code == 10) {
                                entity.line.start.x = convert(value);
                            } else if (code == 20) {
                                entity.line.start.y = convert(value);
                            } else if (code == 11) {
                                entity.line.end.x = convert(value);
                            } else if (code == 21) {
                                entity.line.end.y = convert(value);
                            }
                        } else if (mode == PARSE_ARC) {
                            if (code == 10) {
                                entity.arc.center.x = convert(value);
                            } else if (code == 20) {
                                entity.arc.center.y = convert(value);
                            } else if (code == 40) {
                                entity.arc.radius = convert(value);
                            } else if (code == 50) {
                                entity.arc.start_angle_in_degrees = value;
                            } else if (code == 51) {
                                entity.arc.end_angle_in_degrees = value;
                            }
                        } else if (mode == PARSE_CIRCLE) {
                            if (code == 10) {
                                entity.circle.center.x = convert(value);
                            } else if (code == 20) {
                                entity.circle.center.y = convert(value);
                            } else if (code == 40) {
                                entity.circle.radius = convert(value);
                            }
                        } else {
                            ASSERT(mode == PARSE_UNITS);
                            inches = (value == 1);
                            mode = PARSE_NONE;
                        }
                    }
                }
                code_is_hot = !code_is_hot;
            }
        }
    }

    fclose(file);
}

bool drawing_save_dxf(Drawing *drawing_to_save, String filename) {
    List<Entity> *entities = &drawing_to_save->entities;

    FILE* file = (FILE *) FILE_OPEN(filename, "w");
    if (!file) {
        messagef(pallete.red, "Error opening file for writing: %s\n", filename);
        return false;
    }

    // Write DXF header
    fprintf(file, "999\nFile generated by Conversation\n");
    fprintf(file, "0\nSECTION\n2\nHEADER\n");

    fprintf(file, "0\nSECTION\n2\nHEADER\n");
    fprintf(file, "9\n$ACADVER\n1\nAC1021\n");
    fprintf(file, "9\n$INSBASE\n10\n0.0\n20\n0.0\n30\n0.0\n");
    fprintf(file, "9\n$INSUNITS\n70\n%d\n", config.usingInches ? 1 : 4);
    fprintf(file, "9\n$EXTMIN\n10\n0.0\n20\n0.0\n30\n0.0\n");
    fprintf(file, "9\n$EXTMAX\n10\n1000.0\n20\n1000.0\n30\n0.0\n");
    fprintf(file, "0\nENDSEC\n");
    // Write TABLES section
    fprintf(file, "0\nSECTION\n2\nTABLES\n");

    // LTYPE table
    fprintf(file, "0\nTABLE\n2\nLTYPE\n70\n1\n0\nLTYPE\n2\nCONTINUOUS\n70\n64\n3\nSolid line\n72\n65\n73\n0\n40\n0.0\n0\nENDTAB\n");

    // LAYER table
    fprintf(file, "0\nTABLE\n2\nLAYER\n70\n1\n0\nLAYER\n2\n0\n70\n64\n62\n7\n6\nCONTINUOUS\n0\nENDTAB\n");

    // STYLE table
    fprintf(file, "0\nTABLE\n2\nSTYLE\n70\n1\n0\nSTYLE\n2\nSTANDARD\n70\n0\n40\n0.0\n41\n1.0\n50\n0.0\n71\n0\n42\n0.2\n3\ntxt\n4\n\n0\nENDTAB\n");

    fprintf(file, "0\nENDSEC\n");

    // Write ENTITIES section
    fprintf(file, "0\nSECTION\n2\nENTITIES\n");
    for (Entity *entity = entities->array; entity < &entities->array[entities->length]; ++entity) {

        if (entity->type == EntityType::Line) {
            fprintf(file, "0\nLINE\n");
            fprintf(file, "8\n0\n");  // Layer
            fprintf(file, "62\n%d\n", entity->color_code);
            fprintf(file, "10\n%.6f\n", entity->line.start.x);
            fprintf(file, "20\n%.6f\n", entity->line.start.y);
            fprintf(file, "30\n0.0\n");  // Z coordinate (2D)
            fprintf(file, "11\n%.6f\n", entity->line.end.x);
            fprintf(file, "21\n%.6f\n", entity->line.end.y);
            fprintf(file, "31\n0.0\n");  // Z coordinate (2D)
        } else if (entity->type == EntityType::Arc) {
            fprintf(file, "0\nARC\n");
            fprintf(file, "8\n0\n");  // Layer
            fprintf(file, "62\n%d\n", entity->color_code);
            fprintf(file, "10\n%.6f\n", entity->arc.center.x);
            fprintf(file, "20\n%.6f\n", entity->arc.center.y);
            fprintf(file, "30\n0.0\n");  // Z coordinate (2D)
            fprintf(file, "40\n%.6f\n", entity->arc.radius);
            fprintf(file, "50\n%.6f\n", _WRAP_TO_0_360_INTERVAL(entity->arc.start_angle_in_degrees));
            fprintf(file, "51\n%.6f\n", _WRAP_TO_0_360_INTERVAL(entity->arc.end_angle_in_degrees));
        } else if (entity->type == EntityType::Circle) {
            fprintf(file, "0\nCIRCLE\n");
            fprintf(file, "8\n0\n");  // Layer
            fprintf(file, "62\n%d\n", entity->color_code);
            fprintf(file, "10\n%.6f\n", entity->circle.center.x);
            fprintf(file, "20\n%.6f\n", entity->circle.center.y);
            fprintf(file, "30\n0.0\n");  // Z coordinate (2D)
            fprintf(file, "40\n%.6f\n", entity->circle.radius);
        }
    }

    fprintf(file, "0\nENDSEC\n");

    // Write EOF
    fprintf(file, "0\nEOF\n");

    fclose(file);
    return true;
}

bool mesh_save_stl(Mesh *mesh_to_save, String filename) {
    FILE *file = FILE_OPEN(filename, "wb");
    if (!file) {
        return false;
    }

    int num_bytes = 80 + 4 + 50 * mesh_to_save->num_triangles;
    char *buffer = (char *) calloc(num_bytes, 1); {
        int offset = 80;
        memcpy(buffer + offset, &mesh_to_save->num_triangles, 4);
        offset += 4;
        for_(i, mesh_to_save->num_triangles) {
            vec3 triangle_normal; {
                triangle_normal = mesh_to_save->triangle_normals[i];
                // 90 degree rotation about x: (x, y, z) <- (x, -z, y)
                triangle_normal = { triangle_normal.x, -triangle_normal.z, triangle_normal.y };
            }
            memcpy(buffer + offset, &triangle_normal, 12);
            offset += 12;
            vec3 triangle_vertex_positions[3];
            for_(j, 3) {
                triangle_vertex_positions[j] = mesh_to_save->vertex_positions[mesh_to_save->triangle_indices[i][j]];
                // 90 degree rotation about x: (x, y, z) <- (x, -z, y)
                triangle_vertex_positions[j] = { triangle_vertex_positions[j].x, -triangle_vertex_positions[j].z, triangle_vertex_positions[j].y };

            }
            memcpy(buffer + offset, triangle_vertex_positions, 36);
            offset += 38;
        }
    }
    fwrite(buffer, 1, num_bytes, file);
    free(buffer);

    fclose(file);
    return true;
}

void stl_load(String filename, Mesh *mesh_to_load) {
    // history_record_state(history, manifold_manifold, mesh); // FORNOW

    { // mesh
        uint num_triangles;
        vec3 *triangle_soup;
        {
            #define MAX_LINE_LENGTH 1024
            static _STRING_CALLOC(line_of_file, MAX_LINE_LENGTH);

            #define STL_FILETYPE_UNKNOWN 0
            #define STL_FILETYPE_ASCII   1
            #define STL_FILETYPE_BINARY  2
            uint filetype; {
                FILE *file = FILE_OPEN(filename, "r");
                string_read_line_from_file(&line_of_file, 80, file);
                filetype = (string_matches_prefix(line_of_file, STRING("solid"))) ? STL_FILETYPE_ASCII : STL_FILETYPE_BINARY;
                fclose(file);
            }

            if (filetype == STL_FILETYPE_ASCII) {
                char ascii_scan_dummy[64];
                real ascii_scan_p[3];
                List<real> ascii_data = {};

                FILE *file = FILE_OPEN(filename, "r");
                while (string_read_line_from_file(&line_of_file, MAX_LINE_LENGTH, file)) {
                    if (string_matches_prefix(line_of_file, STRING("vertex"))) {
                        sscanf(line_of_file.data, "%s %f %f %f", ascii_scan_dummy, &ascii_scan_p[0], &ascii_scan_p[1], &ascii_scan_p[2]);
                        for_(d, 3) list_push_back(&ascii_data, ascii_scan_p[d]);
                    }
                }
                fclose(file);
                num_triangles = ascii_data.length / 9;
                uint size = ascii_data.length * sizeof(real);
                triangle_soup = (vec3 *) malloc(size);
                memcpy(triangle_soup, ascii_data.array, size);
                list_free_AND_zero(&ascii_data);
            } else {
                ASSERT(filetype == STL_FILETYPE_BINARY);
                char *entire_file; {
                    FILE *file = FILE_OPEN(filename, "rb");
                    fseek(file, 0, SEEK_END);
                    long fsize = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    entire_file = (char *) malloc(fsize + 1);
                    fread(entire_file, fsize, 1, file);
                    fclose(file);
                    entire_file[fsize] = 0;
                }
                uint offset = 80;
                memcpy(&num_triangles, entire_file + offset, 4);
                offset += 4;
                uint size = num_triangles * 36;
                triangle_soup = (vec3 *) calloc(1, size);
                for_(i, num_triangles) {
                    offset += 12;
                    memcpy(&triangle_soup[3 * i], entire_file + offset, 36);
                    offset += 38;
                }
            }
            { // -90 degree rotation about x: (x, y, z) <- (x, z, -y)
                uint num_vertices = 3 * num_triangles;
                for_(i, num_vertices) {
                    triangle_soup[i] = { triangle_soup[i].x, triangle_soup[i].z, -triangle_soup[i].y };
                }
            }
        }

        uint num_vertices;
        vec3 *vertex_positions;
        uint3 *triangle_indices;
        { // merge vertices (NOTE: only merges vertices that overlap exactly)
            num_vertices = 0;
            Map<vec3, uint> map = {};
            uint _3__times__num_triangles = 3 * num_triangles;
            uint default_value = _3__times__num_triangles + 1;
            {
                List<vec3> list = {};
                for_(i, _3__times__num_triangles) {
                    vec3 p = triangle_soup[i];
                    uint j = map_get(&map, p, default_value);
                    if (j == default_value) {
                        map_put(&map, p, num_vertices++);
                        list_push_back(&list, p);
                    }
                }
                {
                    uint size = list.length * sizeof(vec3);
                    vertex_positions = (vec3 *) malloc(size);
                    memcpy(vertex_positions, list.array, size);
                }
                list_free_AND_zero(&list);
            }
            triangle_indices = (uint3 *) malloc(num_triangles * sizeof(uint3));
            for_(k, _3__times__num_triangles) triangle_indices[k / 3][k % 3] = map_get(&map, triangle_soup[k]);
            map_free_and_zero(&map);
        }

        free(triangle_soup);

        mesh_to_load->num_vertices = num_vertices;
        mesh_to_load->num_triangles = num_triangles;
        mesh_to_load->vertex_positions = vertex_positions;
        mesh_to_load->triangle_indices = triangle_indices;
        mesh_triangle_normals_calculate(mesh_to_load);
        mesh_cosmetic_edges_calculate(mesh_to_load);
        mesh_bbox_calculate(mesh_to_load);
    }
}

// <!> End save_and_load.cpp <!>
// <!> Begin message.cpp <!> 
#define MESSAGE_MAX_LENGTH 256
#define MESSAGE_MAX_NUM_MESSAGES 64
#define MESSAGE_MAX_TIME 16.0f

struct Message {
    _STRING_CALLOC(string, MESSAGE_MAX_LENGTH);
    real time_remaining;
    real y;
    vec3 base_color;
    uint height;
};

Message messages[MESSAGE_MAX_NUM_MESSAGES];

uint _message_index;


void _messagef(vec3 color, char *format, va_list arg) {
    if (other.please_suppress_messagef) return;
    Message *message = &messages[_message_index];
    message->string.length = vsnprintf(message->string.data, MESSAGE_MAX_LENGTH, format, arg);
    va_end(arg);

    message->base_color = color;
    message->time_remaining = MESSAGE_MAX_TIME;
    _message_index = (_message_index + 1) % MESSAGE_MAX_NUM_MESSAGES;
    message->y = 0.0f;//-16.0f; // FORNOW

    // Count the number of newlines in the message
    message->height = 1;
    for (uint i = 0; i < message->string.length; i++) {
        if (message->string.data[i] == '\n') {
            message->height++;
        }
    }

    // printf("%s\n", message->buffer); // FORNOW print to terminal as well
}

void messagef(vec3 color, char *format, ...) {
    va_list args;
    va_start(args, format);
    _messagef(color, format, args);
    va_end(args);
}

void messagef(char *format, ...) {
    va_list args;
    va_start(args, format);
    _messagef(pallete.red, format, args);
    va_end(args);
}

void _messages_update() {
    for_(i, MESSAGE_MAX_NUM_MESSAGES) {
        Message *message = &messages[i];
        if (message->time_remaining > 0) {
            message->time_remaining -= 0.0167f;;
        } else {
            message->time_remaining = 0.0f;
        }
    }
}

void _messages_draw() {
    real font_height_Pixel = 12.0f;
    uint i_0 = (_message_index == 0) ? (MESSAGE_MAX_NUM_MESSAGES - 1) : _message_index - 1;

    real epsilon = font_height_Pixel;
    real x_left, y_top, x_right, y_bottom;
    bbox2 bbox;
    {
        x_left = get_x_divider_drawing_mesh_Pixel() + epsilon;
        x_right = window_get_width_Pixel() - epsilon;
        y_bottom = window_get_height_Pixel() - 2 * epsilon;
        y_top = y_bottom - 96.0f;
        bbox = { x_left, y_top, x_right, y_bottom };
    }

    uint lines_drawn = 0;
    auto draw_lambda = [&](uint message_index) {
        Message *message = &messages[message_index];

        real FADE_IN_TIME = 0.33f;
        real FADE_OUT_TIME = 2.0f;

        real alpha; { // ramp on ramp off
            alpha = 0
                + CLAMPED_LINEAR_REMAP(message->time_remaining, MESSAGE_MAX_TIME, MESSAGE_MAX_TIME - FADE_IN_TIME, 0.0f, 1.0f)
                - CLAMPED_LINEAR_REMAP(message->time_remaining, FADE_OUT_TIME, 0.0f, 0.0f, 1.0f);
        }

        vec3 color = CLAMPED_LINEAR_REMAP(message->time_remaining, MESSAGE_MAX_TIME + FADE_IN_TIME, MESSAGE_MAX_TIME - 2.5f * FADE_IN_TIME, pallete.white, message->base_color);
        color = CLAMPED_LINEAR_REMAP(message->time_remaining, FADE_OUT_TIME, 0.0f, color, V3((color.x + color.y + color.z) / 3));

        real y_target = lines_drawn * font_height_Pixel;
        lines_drawn += message->height;
        // if (message->time_remaining < FADE_OUT_TIME) y_target += CLAMPED_LINEAR_REMAP(message->time_remaining, FADE_OUT_TIME, 0.0f, 0.0f, 12.0f);

        JUICEIT_EASYTWEEN(&message->y, y_target);
        if (message->time_remaining > 0) {
            text_draw(other.OpenGL_from_Pixel, message->string, ROUND(V2(x_left, y_top + message->y)), V4(color, alpha), font_height_Pixel);
        }
    };

    glEnable(GL_SCISSOR_TEST);
    gl_scissor_Pixel(bbox);
    { // this is pretty gross
        uint i = i_0;
        while (true) {
            draw_lambda(i);

            if (i > 0) --i;
            else if (i == 0) i = MESSAGE_MAX_NUM_MESSAGES - 1;

            if (i == i_0) break;
        }
    }
    glDisable(GL_SCISSOR_TEST);
    
    // TODO: stencil test to the transition in beautiful
    bbox2 inflated_bbox = bbox_inflate(bbox, epsilon / 2); 
    eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
    eso_color(pallete.black, 0.3f);
    eso_bbox_SOUP_QUADS(inflated_bbox);
    eso_end();
    eso_begin(other.OpenGL_from_Pixel, SOUP_LINE_LOOP);
    eso_color(pallete.dark_gray);
    eso_bbox_SOUP_QUADS(inflated_bbox);
    eso_end();
}

// <!> End message.cpp <!>
// <!> Begin popup.cpp <!> 
// TODO: drawing before load up is bad bad very bad (we must load up first)
// TODO: ?? LOAD UP MUST HAPPEN FIRST

// BUG: somehow possible sometimes to type 'b' into ExtrudeAdd
// BUG: somehow possible sometimes to type 'b' into ExtrudeAdd

// TODO: consider adding type-checking (NOTE: maybe hard?)

void _POPUP_MEASURE_HOOK(real value) {
    // NOTE: _type_of_active_cell isn't getting cleared to None like it should be
    if (popup->_type_of_active_cell != CellType::Real) return;

    popup->active_cell_buffer.length = sprintf(popup->active_cell_buffer.data, "%g", value);
    popup->cursor = popup->active_cell_buffer.length;
    popup->selection_cursor = 0;
}

void POPUP(
        Command command,
        bool zero_on_load_up,
        CellType _cell_type0,                  String _name0,      void *_value0,
        CellType _cell_type1 = CellType::None, String _name1 = {}, void *_value1 = NULL,
        CellType _cell_type2 = CellType::None, String _name2 = {}, void *_value2 = NULL,
        CellType _cell_type3 = CellType::None, String _name3 = {}, void *_value3 = NULL,
        CellType _cell_type4 = CellType::None, String _name4 = {}, void *_value4 = NULL
        ) {
    String title = command.name;
    ToolboxGroup group = command.group;

    popup->manager.register_call_to_popup_popup(group);

    CellType popup_cell_type[POPUP_MAX_NUM_CELLS];
    String popup_name[POPUP_MAX_NUM_CELLS];
    void *popup_value[POPUP_MAX_NUM_CELLS];
    uint popup_num_cells;
    {
        popup_cell_type[0] = _cell_type0;
        popup_cell_type[1] = _cell_type1;
        popup_cell_type[2] = _cell_type2;
        popup_cell_type[3] = _cell_type3;
        popup_cell_type[4] = _cell_type4;
        popup_name[0] = _name0;
        popup_name[1] = _name1;
        popup_name[2] = _name2;
        popup_name[3] = _name3;
        popup_name[4] = _name4;
        popup_value[0] = _value0;
        popup_value[1] = _value1;
        popup_value[2] = _value2;
        popup_value[3] = _value3;
        popup_value[4] = _value4;

        { // popup_num_cells
            popup_num_cells = 0;
            for_(d, POPUP_MAX_NUM_CELLS) if (popup_name[d].data) ++popup_num_cells;
            ASSERT(popup_num_cells);
        }
    }


    auto POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER = [&]() -> void {
        uint d = popup->active_cell_index;
        if (popup_cell_type[d] == CellType::Real) {
            real *value_d_as_real_ptr = (real *)(popup_value[d]);
            popup->active_cell_buffer.length = sprintf(popup->active_cell_buffer.data, "%g", *value_d_as_real_ptr);
        } else if (popup_cell_type[d] == CellType::Uint) {
            uint *value_d_as_uint_ptr = (uint *)(popup_value[d]);
            popup->active_cell_buffer.length = sprintf(popup->active_cell_buffer.data, "%u", *value_d_as_uint_ptr);
        } else { ASSERT(popup_cell_type[d] == CellType::String);
            String *value_d_as_String_ptr = (String *)(popup_value[d]);
            memcpy(popup->active_cell_buffer.data, value_d_as_String_ptr->data, value_d_as_String_ptr->length);
            popup->active_cell_buffer.length = value_d_as_String_ptr->length;
            popup->active_cell_buffer.data[popup->active_cell_buffer.length] = '\0'; // FORNOW: ease of reading in debugger
        }
    };
    auto POPUP_CLEAR_ALL_VALUES_TO_ZERO = [&]() -> void {
        for_(d, popup_num_cells) {
            if (!popup_name[d].data) continue;
            if (popup_cell_type[d] == CellType::Real) {
                real *value_d_as_real_ptr = (real *)(popup_value[d]);
                *value_d_as_real_ptr = 0.0f;
            } else if (popup_cell_type[d] == CellType::Uint) {
                uint *value_d_as_uint_ptr = (uint *)(popup_value[d]);
                *value_d_as_uint_ptr = 0;
            } else { ASSERT(popup_cell_type[d] == CellType::String);
                String *value_d_as_String_ptr = (String *)(popup_value[d]);
                value_d_as_String_ptr->length = 0;
                value_d_as_String_ptr->data[value_d_as_String_ptr->length] = '\0'; // FORNOW: ease of reading in debugger
            }
        }
    };


    auto POPUP_SYNC_ACTIVE_CELL_BUFFER = [&]() -> void {
        uint d = popup->active_cell_index;
        if (popup_cell_type[d] == CellType::Real) {
            real *value_d_as_real_ptr = (real *)(popup_value[d]);
            // FORNOW: null-terminating and calling strto*
            popup->active_cell_buffer.data[popup->active_cell_buffer.length] = '\0';
            *value_d_as_real_ptr = strtof(popup->active_cell_buffer);
        } else if (popup_cell_type[d] == CellType::Uint) {
            uint *value_d_as_uint_ptr = (uint *)(popup_value[d]);
            // FORNOW: null-terminating and calling strto*
            popup->active_cell_buffer.data[popup->active_cell_buffer.length] = '\0';
            *value_d_as_uint_ptr = uint(strtol(popup->active_cell_buffer.data, NULL, 10));
        } else { ASSERT(popup_cell_type[d] == CellType::String);
            String *value_d_as_String_ptr = (String *)(popup_value[d]);
            memcpy(value_d_as_String_ptr->data, popup->active_cell_buffer.data, popup->active_cell_buffer.length);
            value_d_as_String_ptr->length = popup->active_cell_buffer.length;

            // FORNOW: keeping null-termination around for messagef?
            value_d_as_String_ptr->data[value_d_as_String_ptr->length] = '\0';
        }
    };


    auto POPUP_SELECTION_NOT_ACTIVE = [&]() -> bool {
        return (popup->selection_cursor == popup->cursor);
    };

    auto POPUP_SET_ACTIVE_CELL_INDEX = [&](int new_active_cell_index) -> void {
        popup->active_cell_index = new_active_cell_index;
        POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER();
        popup->cursor = popup->active_cell_buffer.length;
        popup->selection_cursor = 0; // select whole cell
        popup->_type_of_active_cell = popup_cell_type[popup->active_cell_index];
    };




    struct SpecialCaseClickOnInactivePopup {
        bool special_case_clicked_in;
        uint popup_info_hover_cell_index;
        uint popup_info_hover_cell_cursor;
    };
    auto LOADUP_LOGIC = [&](SpecialCaseClickOnInactivePopup special_case = {}) { // load up
        bool tag_corresponding_to_this_group_was_changed = (popup->manager.get_tag(group) != command.name.data);
        bool tag_corresponding_to_focus_group_became_NULL = (popup->manager.focus_group != ToolboxGroup::None) && (popup->manager.get_tag(popup->manager.focus_group) == NULL);
        bool focus_group_was_manually_set_to_this_group = (popup->manager.focus_group_was_set_manually && (group == popup->manager.focus_group));
        bool focus_group_is_None = (popup->manager.focus_group == ToolboxGroup::None); // FORNOW
        bool common = (0
                || tag_corresponding_to_this_group_was_changed
                || tag_corresponding_to_focus_group_became_NULL
                || focus_group_was_manually_set_to_this_group
                || focus_group_is_None
                );

        if (tag_corresponding_to_this_group_was_changed) {
            popup->manager.set_tag(group, command.name.data);
            if (zero_on_load_up) POPUP_CLEAR_ALL_VALUES_TO_ZERO();
        }

        if (common) {
            popup->manager.focus_group = group;
            popup->active_cell_index = 0;
            if (special_case.special_case_clicked_in) popup->active_cell_index = special_case.popup_info_hover_cell_index;
            POPUP_LOAD_CORRESPONDING_VALUE_INTO_ACTIVE_CELL_BUFFER();
            popup->cursor = popup->active_cell_buffer.length;
            popup->selection_cursor = 0;
            if (special_case.special_case_clicked_in) { popup->cursor = special_case.popup_info_hover_cell_cursor; popup->selection_cursor = popup->cursor; }
            popup->_type_of_active_cell = popup_cell_type[popup->active_cell_index];
        }
    };

    auto IS_FOCUSED = [&]() { return (group == popup->manager.focus_group); };

    LOADUP_LOGIC();
    if (IS_FOCUSED()) POPUP_SYNC_ACTIVE_CELL_BUFFER();


    bool dont_draw_because_already_called = popup->a_popup_from_this_group_was_already_called_this_frame[uint(group)]; // this is for dragging the mouse and not having the transparent rectangles flicker
    popup->a_popup_from_this_group_was_already_called_this_frame[uint(group)] = true;
    vec3 raw_accent_color;
    vec3 accent_color;
    vec3 lighter_gray;
    vec3 darker_gray;
    EasyTextPen pen; 
    {
        {
            raw_accent_color = get_accent_color(group);
            if (IS_FOCUSED()) {
                accent_color = raw_accent_color;
                lighter_gray = pallete.light_gray;
                darker_gray = pallete.gray;
                lighter_gray = LERP(0.2f, lighter_gray, raw_accent_color);
                darker_gray = LERP(0.1f, darker_gray, raw_accent_color);
            } else {
                accent_color = LERP(0.1f, pallete.gray, raw_accent_color);
                lighter_gray = pallete.dark_gray;
                darker_gray = pallete.dark_gray;
                lighter_gray = LERP(0.1f, lighter_gray, raw_accent_color);
                darker_gray = LERP(0.05f, darker_gray, raw_accent_color);
            }
        }
        {
            pen = { V2(170.0f, 12.0f), 18.0f, AVG(lighter_gray, accent_color) };
            if (group == ToolboxGroup::Mesh) {
                pen.origin.x += get_x_divider_drawing_mesh_Pixel() - 109.0f;
            } else if (group == ToolboxGroup::Snap) {
                // pen.origin.x = get_x_divider_drawing_mesh_Pixel() - 128.0f
                pen.origin.y += 196.0f;
            }
            pen.ghost_write = (dont_draw_because_already_called || other._please_suppress_drawing_popup_popup); // NOTE: _please_suppress_drawing_popup_popup is for undo / redo
            easy_text_draw(&pen, title);
            pen.origin.x += pen.offset_Pixel.x + 8.0f;
            pen.offset_Pixel.x = 0.0f;
            // pen.origin.y += 2.5f; // FORNOW
            pen.font_height_Pixel = 18.0f;
        }
    }

    // FORNOW: HACK: i'm computing all of these based on the current other.mouse_Pixel
    //               (i am NOT actually looking at where the mouse was when the event happened)
    //               ((this is probably usually fine, and can be switched over to the "proper" approach without too much trouble"))
    bool popup_info_is_hovering = false;
    uint popup_info_hover_cell_index = uint(-1);
    uint popup_info_hover_cell_cursor = uint(-1);
    uint popup_info_active_cell_cursor = uint(-1);
    for_(d, popup_num_cells) {
        bool d_is_active_cell_index; {
            d_is_active_cell_index = true;
            d_is_active_cell_index &= IS_FOCUSED();
            d_is_active_cell_index &= (d == popup->active_cell_index);
        }

        real y_top;
        real y_bottom;
        real x_field_left;
        real x_field_right;
        String field; 
        bbox2 field_bbox;
        { // easy_text_draw
            pen.color = (d_is_active_cell_index) ? accent_color : lighter_gray;
            y_top = pen.get_y_Pixel();
            y_bottom = y_top + (0.8f * pen.font_height_Pixel);

            easy_text_draw(&pen, popup_name[d]);
            easy_text_drawf(&pen, ": ");

            x_field_left = pen.get_x_Pixel() - (pen.font_height_Pixel / 12.0f);

            { // field 
                if (d_is_active_cell_index) {
                    field = popup->active_cell_buffer;
                } else {
                    if (popup_cell_type[d] == CellType::Real) {
                        real *value_d_as_real_ptr = (real *)(popup_value[d]);
                        static _STRING_CALLOC(scratch, POPUP_CELL_LENGTH);
                        scratch.length = snprintf(scratch.data, POPUP_CELL_LENGTH, "%g", *value_d_as_real_ptr);
                        field = scratch;
                    } else if (popup_cell_type[d] == CellType::Uint) {
                        uint *value_d_as_uint_ptr = (uint *)(popup_value[d]);
                        static _STRING_CALLOC(scratch, POPUP_CELL_LENGTH);
                        scratch.length = snprintf(scratch.data, POPUP_CELL_LENGTH, "%u", *value_d_as_uint_ptr);
                        field = scratch;
                    } else { ASSERT(popup_cell_type[d] == CellType::String); 
                        String *value_d_as_String_ptr = (String *)(popup_value[d]);
                        field = *value_d_as_String_ptr;
                    }
                }
            }

            easy_text_draw(&pen, field);

            x_field_right = pen.get_x_Pixel();

            easy_text_drawf(&pen, "\n");

            field_bbox = { x_field_left, y_top, x_field_right, y_bottom };
        }

        {
            { // *_cell_cursor (where the cursor is / _will_ be)
                uint d_cell_cursor; {
                    // FORNOW: O(n)
                    d_cell_cursor = 0;
                    String slice = field;
                    slice.length = 1;
                    real x_char_middle = x_field_left;
                    real half_char_width_prev = 0.0f;
                    for_(i, field.length) {
                        x_char_middle += half_char_width_prev;
                        {
                            half_char_width_prev = 0.5f * _easy_text_dx(&pen, slice);
                            ++slice.data;
                        }
                        x_char_middle += half_char_width_prev;

                        real x_mouse = other.mouse_Pixel.x;
                        if (x_mouse > x_char_middle) d_cell_cursor = i + 1;
                    }
                }

                { // popup_info_hover_cell_*
                    if (bbox_contains(field_bbox, other.mouse_Pixel)) {
                        popup->_FORNOW_info_mouse_is_hovering = true; // FORNOW
                        popup_info_is_hovering = true; // FORNOW
                        popup_info_hover_cell_index = d;
                        popup_info_hover_cell_cursor = d_cell_cursor;
                    }
                }

                { // popup_info_active_cell_cursor
                    if (d == popup->active_cell_index) {
                        popup_info_active_cell_cursor = d_cell_cursor;
                    }
                }
            }

            if (!pen.ghost_write) { // draw cursor selection_bbox hover_bbox
                if (d_is_active_cell_index) { // draw cursor selection_bbox
                    if (POPUP_SELECTION_NOT_ACTIVE()) { // draw cursor
                        real x_cursor; {
                            String slice = field;
                            slice.length = popup->cursor;
                            x_cursor = x_field_left + _easy_text_dx(&pen, slice);
                        }
                        real sint = SIN(other.time_since_cursor_start * 7);
                        real alpha = 0.5f + 0.5f * sint;
                        eso_begin(other.OpenGL_from_Pixel, SOUP_LINES);
                        eso_size(1.5f + 1.5f * sint);
                        SIN(other.time_since_cursor_start * 7);
                        eso_color(accent_color, alpha);
                        eso_vertex(x_cursor, y_top);
                        eso_vertex(x_cursor, y_bottom);
                        eso_end();
                    } else { // draw selection_bbox
                        real x_selection_left;
                        real x_selection_right;
                        {
                            uint left_cursor = MIN(popup->cursor, popup->selection_cursor);
                            uint right_cursor = MAX(popup->cursor, popup->selection_cursor);

                            String slice = field;
                            slice.length = left_cursor;
                            x_selection_left = (x_field_left + _easy_text_dx(&pen, slice));
                            slice.data += left_cursor;
                            slice.length = (right_cursor - left_cursor);
                            x_selection_right = (x_selection_left + _easy_text_dx(&pen, slice));
                        }
                        bbox2 selection_bbox = { x_selection_left, y_top, x_selection_right, y_bottom };
                        eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
                        eso_color(accent_color, 0.4f);
                        eso_bbox_SOUP_QUADS(selection_bbox);
                        eso_end();
                    }
                } else if (popup_info_is_hovering) { // draw hover_bbox
                    bool d_is_hover_cell_index = (d == popup_info_hover_cell_index);
                    bool draw_hover_bbox = ((d_is_hover_cell_index) && (other.mouse_left_drag_pane == Pane::None));
                    if (draw_hover_bbox) {
                        eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
                        eso_overlay(true);
                        eso_color(raw_accent_color, 0.4f);
                        eso_bbox_SOUP_QUADS(field_bbox);
                        eso_end();
                    }
                }
            }
        }
    }

    bool special_case_clicked_in = false;
    bool special_case_xxx_control_tabbed_in = false;
    if (!already_processed_event_passed_to_popups) { // event handling
        Event *event = &event_passed_to_popups;

        if (event->type == EventType::Key) {
            if (IS_FOCUSED()) {
                KeyEvent *key_event = &event->key_event;
                if (key_event->subtype == KeyEventSubtype::Popup) {

                    uint key = key_event->key;
                    bool shift = key_event->shift;
                    bool control = key_event->control;

                    bool _tab_hack_so_aliases_not_introduced_too_far_up = false;
                    if (key == GLFW_KEY_TAB) {
                        _tab_hack_so_aliases_not_introduced_too_far_up = true;
                        if (!control) {
                            already_processed_event_passed_to_popups = true; // FORNOW; TODO: CTRL+TAB should be handled by the next popup in the series

                            uint new_active_cell_index; {
                                // FORNOW
                                if (!shift) {
                                    new_active_cell_index = (popup->active_cell_index + 1) % popup_num_cells;
                                } else {
                                    if (popup->active_cell_index != 0) {
                                        new_active_cell_index = popup->active_cell_index - 1;
                                    } else {
                                        new_active_cell_index = popup_num_cells - 1;
                                    }
                                }
                            }
                            POPUP_SET_ACTIVE_CELL_INDEX(new_active_cell_index);
                        }
                    }

                    uint left_cursor = MIN(popup->cursor, popup->selection_cursor);
                    uint right_cursor = MAX(popup->cursor, popup->selection_cursor);

                    if (_tab_hack_so_aliases_not_introduced_too_far_up) {
                    } else if (control && (key == 'A')) {
                        popup->cursor = popup->active_cell_buffer.length;
                        popup->selection_cursor = 0;
                    } else if (key == GLFW_KEY_LEFT) {
                        if (!shift && !control) {
                            if (POPUP_SELECTION_NOT_ACTIVE()) {
                                if (popup->cursor > 0) --popup->cursor;
                            } else {
                                popup->cursor = left_cursor;
                            }
                            popup->selection_cursor = popup->cursor;
                        } else if (shift && !control) {
                            if (POPUP_SELECTION_NOT_ACTIVE()) popup->selection_cursor = popup->cursor;
                            if (popup->cursor > 0) --popup->cursor;
                        } else if (control && !shift) {
                            popup->selection_cursor = popup->cursor = 0;
                        } else { ASSERT(shift && control);
                            popup->selection_cursor = 0;
                        }
                    } else if (key == GLFW_KEY_RIGHT) {
                        if (!shift && !control) {
                            if (POPUP_SELECTION_NOT_ACTIVE()) {
                                if (popup->cursor < popup->active_cell_buffer.length) ++popup->cursor;
                            } else {
                                popup->cursor = MAX(popup->cursor, popup->selection_cursor);
                            }
                            popup->selection_cursor = popup->cursor;
                        } else if (shift && !control) {
                            if (POPUP_SELECTION_NOT_ACTIVE()) popup->selection_cursor = popup->cursor;
                            if (popup->cursor < popup->active_cell_buffer.length) ++popup->cursor;
                        } else if (control && !shift) {
                            popup->selection_cursor = popup->cursor = popup->active_cell_buffer.length;
                        } else { ASSERT(shift && control);
                            popup->selection_cursor = popup->active_cell_buffer.length;
                        }
                    } else if (key == GLFW_KEY_BACKSPACE) {
                        // * * * *|* * * * 
                        if (POPUP_SELECTION_NOT_ACTIVE()) {
                            if (popup->cursor > 0) {
                                memmove(&popup->active_cell_buffer.data[popup->cursor - 1], &popup->active_cell_buffer.data[popup->cursor], POPUP_CELL_LENGTH - popup->cursor);
                                --popup->active_cell_buffer.length;
                                --popup->cursor;
                            }
                        } else {
                            // * * * * * * * * * * * * * * * *
                            // * * * * * * * - - - - - - - - -
                            //    L                 R 

                            // * * * * * * * * * * * * * * * *
                            // * * * * * * * * * * * * - - - -
                            //    L       R                   
                            memmove(&popup->active_cell_buffer.data[left_cursor], &popup->active_cell_buffer.data[right_cursor], POPUP_CELL_LENGTH - right_cursor);
                            popup->active_cell_buffer.length -= (right_cursor - left_cursor);
                            popup->cursor = left_cursor;
                        }
                        popup->selection_cursor = popup->cursor;
                    } else if (key == GLFW_KEY_ENTER) {
                        ;
                    } else {
                        // TODO: strip char_equivalent into function

                        bool key_is_alpha = ('A' <= key) && (key <= 'Z');

                        char char_equivalent; {
                            char_equivalent = (char) key;
                            if (!shift && key_is_alpha) {
                                char_equivalent = 'a' + (char_equivalent - 'A');
                            }
                        }
                        if (POPUP_SELECTION_NOT_ACTIVE()) {
                            if (popup->cursor < POPUP_CELL_LENGTH) {
                                memmove(&popup->active_cell_buffer.data[popup->cursor + 1], &popup->active_cell_buffer.data[popup->cursor], POPUP_CELL_LENGTH - popup->cursor - 1);
                                popup->active_cell_buffer.data[popup->cursor] = char_equivalent;
                                ++popup->cursor;
                                ++popup->active_cell_buffer.length;
                            }
                        } else {
                            memmove(&popup->active_cell_buffer.data[left_cursor + 1], &popup->active_cell_buffer.data[right_cursor], POPUP_CELL_LENGTH - right_cursor);
                            popup->active_cell_buffer.length -= (right_cursor - left_cursor);
                            popup->cursor = left_cursor;
                            popup->active_cell_buffer.data[popup->cursor] = char_equivalent;
                            ++popup->cursor;
                            ++popup->active_cell_buffer.length;
                        }
                        popup->selection_cursor = popup->cursor;
                    }

                    // FORNOW: keeping null-termination around for messagef?
                    popup->active_cell_buffer.data[popup->active_cell_buffer.length] = '\0';
                }
            } else { // CTRL+TAB
                KeyEvent *key_event = &event->key_event;
                if (key_event->subtype == KeyEventSubtype::Popup) {
                    uint key = key_event->key;
                    bool control = key_event->control;
                    bool shift = key_event->shift;
                    if (control && (key == GLFW_KEY_TAB)) {
                        ToolboxGroup next_group = popup->manager.focus_group;
                        do {
                            if (!shift) {
                                next_group = ToolboxGroup(uint(next_group) - 1);
                                if (next_group == ToolboxGroup::None) {
                                    next_group = ToolboxGroup(uint(ToolboxGroup::NUMBER_OF) - 1);
                                }
                            } else {
                                next_group = ToolboxGroup(uint(next_group) + 1);
                                if (next_group == ToolboxGroup::NUMBER_OF) {
                                    next_group = ToolboxGroup(uint(ToolboxGroup::None) + 1);
                                }
                            }
                        } while (popup->manager.get_tag(next_group) == NULL);

                        if (next_group == group) {
                            already_processed_event_passed_to_popups = true;
                            popup->manager.manually_set_focus_group(group);
                            special_case_xxx_control_tabbed_in = true;
                        }
                    }
                }
            }
        }


        if (event->type == EventType::Mouse) {
            MouseEvent *mouse_event = &event->mouse_event;
            if (mouse_event->subtype == MouseEventSubtype::Popup) {

                MouseEventPopup *mouse_event_popup = &mouse_event->mouse_event_popup;
                FORNOW_UNUSED(mouse_event_popup); // FORNOW we're just using other.mouse_Pixel

                if (!mouse_event->mouse_held) { // press
                    if (popup_info_is_hovering) {
                        already_processed_event_passed_to_popups = true; // NOTE: does NOT require focus!

                        if (IS_FOCUSED()) {
                            if (popup->active_cell_index == popup_info_hover_cell_index) { // same cell
                                if (mouse_event->mouse_double_click) { // select all (double click)
                                    popup->cursor = popup->active_cell_buffer.length;
                                    popup->selection_cursor = 0;
                                } else { // move
                                    popup->cursor = popup_info_hover_cell_cursor;
                                    popup->selection_cursor = popup->cursor;
                                }
                            } else { // switch cell
                                POPUP_SET_ACTIVE_CELL_INDEX(popup_info_hover_cell_index);
                                popup->cursor = popup_info_hover_cell_cursor;
                                popup->selection_cursor = popup->cursor;
                            }
                        } else {
                            special_case_clicked_in = true;
                            popup->manager.manually_set_focus_group(group);
                        }
                    }
                } else if (!mouse_event->mouse_double_click_held) { // drag
                    if (IS_FOCUSED()) {
                        already_processed_event_passed_to_popups = true;
                        popup->selection_cursor = popup_info_active_cell_cursor;
                    }
                }
            }
        }
    }

    if (special_case_clicked_in) {
        LOADUP_LOGIC({ true, popup_info_hover_cell_index, popup_info_hover_cell_cursor });
    }
    if (special_case_xxx_control_tabbed_in) {
        LOADUP_LOGIC();
    }

    if (IS_FOCUSED()) POPUP_SYNC_ACTIVE_CELL_BUFFER();

};
// <!> End popup.cpp <!>
// <!> Begin history.cpp <!> 
StandardEventProcessResult standard_event_process(Event event); // forward declaration

#ifdef DEBUG_HISTORY_DISABLE_HISTORY_ENTIRELY
//
void history_process_event(Event standard_event) { _standard_event_process_NOTE_RECURSIVE(standard_event); }
void history_undo() { messagef("[DEBUG] history disabled"); }
void history_redo() { messagef("[DEBUG] history disabled"); }
void history_debug_draw() { gui_printf("[DEBUG] history disabled"); }
//
#else
//
struct {
    ElephantStack<Event> recorded_user_events;
    ElephantStack<WorldState_ChangesToThisMustBeRecorded_state> snapshotted_world_states;
} history;

struct StackPointers {
    Event *user_event;
    WorldState_ChangesToThisMustBeRecorded_state *world_state;
};

StackPointers POP_UNDO_ONTO_REDO() {
    StackPointers result = {};
    result.user_event = elephant_pop_undo_onto_redo(&history.recorded_user_events);
    if (result.user_event->snapshot_me) result.world_state = elephant_pop_undo_onto_redo(&history.snapshotted_world_states);
    return result;
};

StackPointers POP_REDO_ONTO_UNDO() {
    StackPointers result = {};
    result.user_event = elephant_pop_redo_onto_undo(&history.recorded_user_events);
    if (result.user_event->snapshot_me) result.world_state = elephant_pop_redo_onto_undo(&history.snapshotted_world_states);
    return result;
};

StackPointers PEEK_UNDO() {
    Event *user_event = elephant_peek_undo(&history.recorded_user_events);
    WorldState_ChangesToThisMustBeRecorded_state *world_state = elephant_is_empty_undo(&history.snapshotted_world_states) ? NULL : elephant_peek_undo(&history.snapshotted_world_states);
    return { user_event, world_state };
}

bool EVENT_UNDO_NONEMPTY() {
    return !elephant_is_empty_undo(&history.recorded_user_events);
}

bool EVENT_REDO_NONEMPTY() {
    return !elephant_is_empty_redo(&history.recorded_user_events);
}

void PUSH_UNDO_CLEAR_REDO(Event standard_event) {
    elephant_push_undo_clear_redo(&history.recorded_user_events, standard_event);
    { // clear the world_state redo stack
        for (////
                WorldState_ChangesToThisMustBeRecorded_state *world_state = history.snapshotted_world_states._redo_stack.array;
                world_state < history.snapshotted_world_states._redo_stack.array + history.snapshotted_world_states._redo_stack.length;
                ++world_state
            ) {//
            world_state_free_AND_zero(world_state);
        }
        elephant_clear_redo(&history.snapshotted_world_states); // TODO ?
    }
    if (standard_event.snapshot_me) {
        WorldState_ChangesToThisMustBeRecorded_state snapshot;
        world_state_deep_copy(&snapshot, &state);
        elephant_push_undo_clear_redo(&history.snapshotted_world_states, snapshot); // TODO: clear is unnecessary here
    }
}

void history_process_event(Event standard_event) {
    StandardEventProcessResult tmp = standard_event_process(standard_event);
    standard_event.record_me = tmp.record_me;
    standard_event.checkpoint_me = tmp.checkpoint_me;
    #ifndef DEBUG_HISTORY_DISABLE_SNAPSHOTTING
    standard_event.snapshot_me = tmp.snapshot_me;
    #endif
    if (standard_event.record_me) PUSH_UNDO_CLEAR_REDO(standard_event);
}

void history_undo() {
    if (elephant_is_empty_undo(&history.recorded_user_events)) {
        messagef(pallete.orange, "Undo: nothing to undo");
        return;
    }

    { // // manipulate stacks (undo -> redo)
      // 1) pop through a first checkpoint 
      // 2) pop up to a second checkpoint
        while (EVENT_UNDO_NONEMPTY()) { if (POP_UNDO_ONTO_REDO().user_event->checkpoint_me) break; }
        while (EVENT_UNDO_NONEMPTY()) {
            if (PEEK_UNDO().user_event->checkpoint_me) break;
            POP_UNDO_ONTO_REDO();
        }
    }
    Event *one_past_end = elephant_undo_ptr_one_past_end(&history.recorded_user_events);
    Event *begin;
    { // // find beginning
      // 1) walk back to snapshot event (or end of stack)
      // TODO: this feels kind of sloppy still
        begin = one_past_end - 1; // ?
        world_state_free_AND_zero(&state);
        while (true) {
            if (begin <= elephant_undo_ptr_begin(&history.recorded_user_events)) {
                begin =  elephant_undo_ptr_begin(&history.recorded_user_events); // !
                state = {};
                break;
            }
            if (begin->snapshot_me) {
                world_state_deep_copy(&state, PEEK_UNDO().world_state);
                break;
            }
            --begin;
        }
    }
    other.please_suppress_messagef = true; {
        for (Event *event = begin; event < one_past_end; ++event) standard_event_process(*event);
    } other.please_suppress_messagef = false;

    messagef(pallete.light_gray, "Undo");
}

void history_redo() {
    if (elephant_is_empty_redo(&history.recorded_user_events)) {
        messagef(pallete.orange, "Redo: nothing to redo");
        return;
    }

    other.please_suppress_messagef = true; {
        while (EVENT_REDO_NONEMPTY()) { // // manipulate stacks (undo <- redo)
            StackPointers popped = POP_REDO_ONTO_UNDO();
            Event *user_event = popped.user_event;
            WorldState_ChangesToThisMustBeRecorded_state *world_state = popped.world_state;

            standard_event_process(*user_event);
            if (world_state) {
                world_state_free_AND_zero(&state);
                world_state_deep_copy(&state, world_state);
            }

            if (user_event->checkpoint_me) break;
        }
    } other.please_suppress_messagef = false;

    messagef(pallete.light_gray, "Redo");
}

void history_printf_script() {
    List<char> _script = {};
    for (////
            Event *event = history.recorded_user_events._undo_stack.array;
            event < history.recorded_user_events._undo_stack.array + history.recorded_user_events._undo_stack.length;
            ++event
        ) {//
        if (event->type == EventType::Key) {
            KeyEvent key_event = event->key_event;
            if (key_event.control) list_push_back(&_script, '^');
            if (key_event.key == GLFW_KEY_ENTER) {
                list_push_back(&_script, '\\');
                list_push_back(&_script, 'n');
            } else {
                char char_equivalent = (char) key_event.key;
                if ((('A' <= key_event.key) && (key_event.key <= 'Z')) && !key_event.shift) char_equivalent = 'a' + (char_equivalent - 'A');
                list_push_back(&_script, (char) char_equivalent);
            }
        }
    }
    printf("%.*s\n", _script.length, _script.array);
    list_free_AND_zero(&_script);
}

void history_debug_draw() {
    { // FORNOW
        eso_begin(M4_Identity(), SOUP_QUADS);
        eso_overlay(true);
        eso_color(pallete.black, 0.7f);
        BoundingBox<2> bbox_OpenGL = { -1.0f, -1.0f, 1.0f, 1.0f };
        eso_bbox_SOUP_QUADS(bbox_OpenGL);
        eso_end();
    }
    EasyTextPen pen;

    auto _history_user_event_draw_helper = [&](Event event) {
        char message[256]; {
            // TODO: handle shift and control with the special characters
            if (event.type == EventType::Key) {
                KeyEvent *key_event = &event.key_event;
                char *boxed; {
                    if (key_event->subtype == KeyEventSubtype::Hotkey) {
                        boxed = "[KEY_HOTKEY]";
                    } else { ASSERT(key_event->subtype == KeyEventSubtype::Popup);
                        boxed = "[KEY_POPUP]";
                    }
                }
                if (!key_event->_name_of_spoofing_button) {
                    sprintf(message, "%s %s", boxed, key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(key_event));
                } else {
                    boxed = "[BUTTON]"; // FORNOW
                    sprintf(message, "%s %s", boxed, key_event->_name_of_spoofing_button);
                }
            } else { ASSERT(event.type == EventType::Mouse);
                MouseEvent *mouse_event = &event.mouse_event;
                if (mouse_event->subtype == MouseEventSubtype::Drawing) {
                    MouseEventDrawing *mouse_event_drawing = &mouse_event->mouse_event_drawing;
                    sprintf(message, "[MOUSE_DRAWING] %g %g", mouse_event_drawing->snap_result.mouse_position.x, mouse_event_drawing->snap_result.mouse_position.y);
                } else if (mouse_event->subtype == MouseEventSubtype::Mesh) {
                    MouseEventMesh *mouse_event_mesh = &mouse_event->mouse_event_mesh;
                    sprintf(message, "[MOUSE_MESH] %g %g %g %g %g %g", mouse_event_mesh->mouse_ray_origin.x, mouse_event_mesh->mouse_ray_origin.y, mouse_event_mesh->mouse_ray_origin.z, mouse_event_mesh->mouse_ray_direction.x, mouse_event_mesh->mouse_ray_direction.y, mouse_event_mesh->mouse_ray_direction.z);
                } else if (mouse_event->subtype == MouseEventSubtype::Popup) {
                    MouseEventPopup *mouse_event_popup = &mouse_event->mouse_event_popup;
                    sprintf(message, "[MOUSE_POPUP] %d %d", mouse_event_popup->cell_index, mouse_event_popup->cursor);
                } else { ASSERT(mouse_event->subtype == MouseEventSubtype::ToolboxButton);
                    MouseEventToolboxButton *mouse_event_toolbox_button = &mouse_event->mouse_event_toolbox_button;
                    sprintf(message, "[MOUSE_TOOLBOX_BUTTON] %s", mouse_event_toolbox_button->name);
                }
            }
        }
        easy_text_drawf(&pen, "%c%c %s",
                (event.checkpoint_me)   ? 'C' : ' ',
                (event.snapshot_me)     ? 'S' : ' ',
                message);
    };

    auto _history_world_state_draw_helper = [&](WorldState_ChangesToThisMustBeRecorded_state *world_state) {
        easy_text_drawf(&pen, "%d elements  %d triangles", world_state->drawing.entities.length, world_state->mesh.num_triangles);
    };

    pen = { V2(12.0f, 12.0f), 12.0f, pallete.white, true };

    { // recorded_user_events
        if (history.recorded_user_events._redo_stack.length) {
            for (////
                    Event *event = history.recorded_user_events._redo_stack.array;
                    event < history.recorded_user_events._redo_stack.array + history.recorded_user_events._redo_stack.length;
                    ++event
                ) {//
                _history_user_event_draw_helper(*event);
            }
            pen.color = pallete.cyan; {
                easy_text_drawf(&pen, "^ redo (%d)", elephant_length_redo(&history.recorded_user_events));
            } pen.color = pallete.white;
        }
        if ((history.recorded_user_events._redo_stack.length) || (history.recorded_user_events._undo_stack.length)) {
            pen.color = pallete.cyan; {
                easy_text_drawf(&pen, "  RECORDED_EVENTS");
            } pen.color = pallete.white;
        } else {
            pen.color = pallete.cyan; {
                easy_text_drawf(&pen, "--- no history ---");
            } pen.color = pallete.white;
        }
        if (history.recorded_user_events._undo_stack.length) {
            pen.color = pallete.cyan; {
                easy_text_drawf(&pen, "v undo (%d)", elephant_length_undo(&history.recorded_user_events));
            } pen.color = pallete.white;
            for (////
                    Event *event = history.recorded_user_events._undo_stack.array + (history.recorded_user_events._undo_stack.length - 1);
                    event >= history.recorded_user_events._undo_stack.array;
                    --event
                ) {//
                _history_user_event_draw_helper(*event);
            }
        }
    }

    pen.origin.x += get_x_divider_drawing_mesh_Pixel() + 12.0f;
    pen.offset_Pixel = {};

    { // snapshotted_world_states
        if (history.snapshotted_world_states._redo_stack.length) {
            for (////
                    WorldState_ChangesToThisMustBeRecorded_state *world_state = history.snapshotted_world_states._redo_stack.array;
                    world_state < history.snapshotted_world_states._redo_stack.array + history.snapshotted_world_states._redo_stack.length;
                    ++world_state
                ) {//
                _history_world_state_draw_helper(world_state);
            }
            pen.color = pallete.cyan; {
                easy_text_drawf(&pen, "^ redo (%d)", elephant_length_redo(&history.snapshotted_world_states));
            } pen.color = pallete.white;
        }
        if ((history.snapshotted_world_states._redo_stack.length) || (history.snapshotted_world_states._undo_stack.length)) {
            pen.color = pallete.cyan; {
                easy_text_drawf(&pen, "  SNAPSHOTTED_WORLD_STATES");
            } pen.color = pallete.white;
        }
        if (history.snapshotted_world_states._undo_stack.length) {
            pen.color = pallete.cyan; {
                easy_text_drawf(&pen, "v undo (%d)", elephant_length_undo(&history.snapshotted_world_states));
            } pen.color = pallete.white;
            for (////
                    WorldState_ChangesToThisMustBeRecorded_state *world_state = history.snapshotted_world_states._undo_stack.array + (history.snapshotted_world_states._undo_stack.length - 1);
                    world_state >= history.snapshotted_world_states._undo_stack.array;
                    --world_state
                ) {//
                _history_world_state_draw_helper(world_state);
            }
        }
    }
}
//
#endif
// <!> End history.cpp <!>
// <!> Begin callbacks.cpp <!> 
Queue<RawEvent> raw_event_queue;

void callback_key(GLFWwindow *, int key, int, int action, int mods) {
    bool control = (mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER));
    bool shift = (mods & GLFW_MOD_SHIFT);
    bool alt = (mods & GLFW_MOD_ALT);

    if (key == GLFW_KEY_LEFT_SHIFT) {
        if (action == GLFW_PRESS) {
            other.shift_held = true;
        } else if (action == GLFW_RELEASE) {
            other.shift_held = false;
        }
    }
    if (key == GLFW_KEY_LEFT_SHIFT) return;
    if (key == GLFW_KEY_RIGHT_SHIFT) return;
    if (key == GLFW_KEY_LEFT_ALT) return;
    if (key == GLFW_KEY_RIGHT_ALT) return;
    if (key == GLFW_KEY_LEFT_CONTROL) return;
    if (key == GLFW_KEY_RIGHT_CONTROL) return;
    if (key == GLFW_KEY_LEFT_SUPER) return;
    if (key == GLFW_KEY_RIGHT_SUPER) return;
    if (action == GLFW_PRESS || (action == GLFW_REPEAT)) {
        // FORNOW: i guess okay to handle these here?
        bool toggle_pause = ((key == 'P') && (control) && (!shift));
        bool toggle_slowmo = ((key == 'P') && (control) && (shift));
        bool step = (other.paused) && ((key == '.') && (!control) && (!shift));
        bool quit = ((key == 'Q') && (control) && (!shift));
        if (toggle_pause) {
            other.paused = !other.paused;
        } else if (toggle_slowmo) {
            other.slowmo = !other.slowmo;
        } else if (step) {
            other.stepping_one_frame_while_paused = true;
        } else if (quit) {
            glfwSetWindowShouldClose(glfw_window, true);
        } else {
            RawEvent raw_event = {}; {
                raw_event.type = EventType::Key;
                RawKeyEvent *raw_key_event = &raw_event.raw_key_event;
                raw_key_event->key = key;
                raw_key_event->control = control;
                raw_key_event->shift = shift;
                raw_key_event->alt = alt;
            }
            queue_enqueue(&raw_event_queue, raw_event);
        }
    }
}

void callback_cursor_position(GLFWwindow *, double xpos, double ypos) {
    other.time_since_mouse_moved = 0.0f;

    // mouse_*
    vec2 delta_mouse_OpenGL;
    vec2 delta_mouse_World_2D;
    {
        vec2 prev_mouse_OpenGL = other.mouse_OpenGL;
        other.mouse_Pixel = { real(xpos), real(ypos) };
        other.mouse_OpenGL = transformPoint(other.OpenGL_from_Pixel, other.mouse_Pixel);
        delta_mouse_OpenGL = other.mouse_OpenGL - prev_mouse_OpenGL;
        delta_mouse_World_2D = transformVector(inverse(camera_drawing->get_PV()), delta_mouse_OpenGL);
    }

    { // hot_pane

        // by definition x_divider_stamp_drawing < x_divider_stamp_drawing
        // unless we want to implement something where we can hide windows
        real x_divider_drawing_mesh_Pixel = get_x_divider_drawing_mesh_Pixel();
        real eps = 6.0f;
        real x_mouse_Pixel = other.mouse_Pixel.x;
        if (
                1
                && (toolbox->hot_name)
           ) {
            other.hot_pane = Pane::Toolbox;
        } else if (
                1
                && (popup->manager.focus_group != ToolboxGroup::None)
                && (popup->_FORNOW_info_mouse_is_hovering)
                ) {
            other.hot_pane = Pane::Popup;
        } else if (x_mouse_Pixel < x_divider_drawing_mesh_Pixel - eps) {
            other.hot_pane = Pane::Drawing;
        } else if (x_mouse_Pixel < x_divider_drawing_mesh_Pixel + eps) {
            other.hot_pane = Pane::Separator;
        } else {
            other.hot_pane = Pane::Mesh;
        }
    }

    { // special draggin mouse_held EventType::Mouse
        if (0
                || (other.mouse_left_drag_pane == Pane::Drawing)
                || (other.mouse_left_drag_pane == Pane::Popup)
           ) {
            RawEvent raw_event; {
                raw_event = {};
                raw_event.type = EventType::Mouse;
                RawMouseEvent *raw_mouse_event = &raw_event.raw_mouse_event;
                raw_mouse_event->pane = other.mouse_left_drag_pane;
                raw_mouse_event->mouse_Pixel = other.mouse_Pixel;
                raw_mouse_event->mouse_held = true;
                raw_mouse_event->mouse_double_click_held = other.mouse_double_left_click_held;
            }
            queue_enqueue(&raw_event_queue, raw_event);
        }
    }

    { // dragging drawing mesh divider
        if (other.mouse_left_drag_pane == Pane::Separator) {
            real prev_x_divider_drawing_mesh_OpenGL = other.x_divider_drawing_mesh_OpenGL;
            real prev_x_divider_drawing_mesh_Pixel = get_x_divider_drawing_mesh_Pixel();
            other.x_divider_drawing_mesh_OpenGL = LINEAR_REMAP(CLAMP(real(xpos), 199.0f, window_get_width_Pixel() - 131.0f), 0.0f, window_get_width_Pixel(), -1.0f, 1.0f); // *
            real x_divider_drawing_mesh_Pixel = get_x_divider_drawing_mesh_Pixel();

            real dx_divider_drawing_mesh_OpenGL = 0.5f * (other.x_divider_drawing_mesh_OpenGL - prev_x_divider_drawing_mesh_OpenGL);
            camera_drawing->post_nudge_OpenGL.x += dx_divider_drawing_mesh_OpenGL;
            camera_mesh->post_nudge_OpenGL.x += dx_divider_drawing_mesh_OpenGL;

            real width_ratio_drawing = (x_divider_drawing_mesh_Pixel / prev_x_divider_drawing_mesh_Pixel);
            camera_drawing->ortho_screen_height_World /= width_ratio_drawing;
            real _window_width_Pixel = window_get_width_Pixel();
            real width_ratio_mesh = (_window_width_Pixel - x_divider_drawing_mesh_Pixel) / (_window_width_Pixel - prev_x_divider_drawing_mesh_Pixel);
            { // camera_mesh->*
                bool is_perspective_camera = (!IS_ZERO(camera_mesh->angle_of_view));
                Camera tmp_2D = make_EquivalentCamera2D(camera_mesh);
                tmp_2D.ortho_screen_height_World /= width_ratio_mesh;
                if (is_perspective_camera) {
                    camera_mesh->persp_distance_to_origin_World = ((0.5f * tmp_2D.ortho_screen_height_World) / TAN(0.5f * camera_mesh->angle_of_view));
                } else {
                    camera_mesh->ortho_screen_height_World = tmp_2D.ortho_screen_height_World;
                }
            }
        }
    }

    { // moving cameras
      // mouse_left_drag_pane
        if (other.mouse_left_drag_pane == Pane::Mesh) {
            real fac = 2.0f;
            camera_mesh->euler_angles.y -= fac * delta_mouse_OpenGL.x;
            camera_mesh->euler_angles.x += fac * delta_mouse_OpenGL.y;
            camera_mesh->euler_angles.x = CLAMP(camera_mesh->euler_angles.x, -RAD(90), RAD(90));
        }

        // mouse_right_drag_pane
        if (other.mouse_right_drag_pane == Pane::Drawing) {
            camera_drawing->pre_nudge_World -= delta_mouse_World_2D;
        } else if (other.mouse_right_drag_pane == Pane::Mesh) {
            Camera tmp_2D = make_EquivalentCamera2D(camera_mesh);
            tmp_2D.pre_nudge_World -= transformVector(inverse(tmp_2D.get_PV()), delta_mouse_OpenGL);
            camera_mesh->pre_nudge_World = tmp_2D.pre_nudge_World;
        }
    }
}

void callback_mouse_button(GLFWwindow *, int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {

        bool double_left_click;
        { // double_left_click, other.mouse_double_left_click_held
            double_left_click = false;
            long new_timestamp = MILLIS();
            long delta = new_timestamp - other.timestamp_mouse_left_click;
            // messagef(pallete.blue, "%ld - %ld = %ld", new_timestamp, other.timestamp_mouse_left_click, delta);
            if (delta < 128.0L) {
                other.mouse_double_left_click_held = true;
                double_left_click = true;
            }
            other.timestamp_mouse_left_click = new_timestamp;
        }

        // NOTE: GLFW_HELD (not a real thing) is in callback_cursor_position
        if (action == GLFW_PRESS) {
            other.mouse_left_drag_pane = other.hot_pane;
            RawEvent raw_event; {
                raw_event = {};
                raw_event.type = EventType::Mouse;
                RawMouseEvent *raw_mouse_event = &raw_event.raw_mouse_event;
                raw_mouse_event->pane = other.hot_pane;
                raw_mouse_event->mouse_Pixel = other.mouse_Pixel;
                raw_mouse_event->mouse_double_click = double_left_click;
            }
            queue_enqueue(&raw_event_queue, raw_event);
        } else { ASSERT(action == GLFW_RELEASE);
            other.mouse_left_drag_pane = Pane::None;
            other.mouse_double_left_click_held = false;
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            other.mouse_right_drag_pane = other.hot_pane;
        } else { ASSERT(action == GLFW_RELEASE);
            other.mouse_right_drag_pane = Pane::None;
        }
    }
}

void _callback_scroll_helper(Camera *camera_2D, double yoffset) {
    // IDEA: preserve mouse position
    ASSERT(IS_ZERO(camera_2D->angle_of_view));
    ASSERT(IS_ZERO(camera_2D->euler_angles));
    vec2 mouse_position_World_before  = transformPoint(inverse(camera_2D->get_PV()), other.mouse_OpenGL);
    camera_2D->ortho_screen_height_World *= (1.0f - 0.1f * real(yoffset));
    vec2 mouse_position_World_after = transformPoint(inverse(camera_2D->get_PV()), other.mouse_OpenGL);
    camera_2D->pre_nudge_World -= (mouse_position_World_after - mouse_position_World_before);
}

void callback_scroll(GLFWwindow *, double, double yoffset) {
    if (other.hot_pane == Pane::Drawing) {
        _callback_scroll_helper(camera_drawing, yoffset);
    } else if (other.hot_pane == Pane::Mesh) {
        // IDEA: preserve mouse position in the rasterized image
        bool is_perspective_camera = (!IS_ZERO(camera_mesh->angle_of_view));
        Camera tmp_2D = make_EquivalentCamera2D(camera_mesh);
        _callback_scroll_helper(&tmp_2D, yoffset);
        if (is_perspective_camera) {
            camera_mesh->persp_distance_to_origin_World = ((0.5f * tmp_2D.ortho_screen_height_World) / TAN(0.5f * camera_mesh->angle_of_view));
        } else {
            camera_mesh->ortho_screen_height_World = tmp_2D.ortho_screen_height_World;
        }
        camera_mesh->pre_nudge_World = tmp_2D.pre_nudge_World;
    }
}

void callback_framebuffer_size(GLFWwindow *, int width, int height) {
    glViewport(0, 0, width, height);
}

void callback_drop(GLFWwindow *, int count, const char **paths) {
    if (count > 0) {
        void script_process(String);

        char *filename = (char *) paths[0];
        String string_filename = STRING(filename);
        script_process(STRING("\033"));
        script_process(STRING("^o"));
        script_process(string_filename);
        script_process(STRING("\n"));
    }
}

// <!> End callbacks.cpp <!>
// <!> Begin bake.cpp <!> 
KeyEventSubtype classify_baked_subtype_of_raw_key_event(RawKeyEvent *raw_key_event) {
    if (popup->manager.focus_group == ToolboxGroup::None) return KeyEventSubtype::Hotkey;

    uint key = raw_key_event->key;
    bool control = raw_key_event->control;
    // bool shift = raw_key_event->shift;

    // TODO: these need to take control into account
    bool key_is_digit = ('0' <= key) && (key <= '9');
    bool key_is_period = (key == '.');
    bool key_is_hyphen = (key == '-');
    bool key_is_alpha = ('A' <= key) && (key <= 'Z');
    bool key_is_delete = (key == GLFW_KEY_BACKSPACE) || (key == GLFW_KEY_DELETE);
    bool key_is_enter = (key == GLFW_KEY_ENTER);
    bool key_is_nav = (key == GLFW_KEY_TAB) || (key == GLFW_KEY_LEFT) || (key == GLFW_KEY_RIGHT);
    bool key_is_ctrl_a = (key == 'A') && (control);
    bool key_is_slash = (key == '/') || (key == '\\');
    bool key_is_colon = (key == ':');
    bool key_is_space = (key == ' ');
    bool key_is_underscore = (key == '_');

    bool is_consumable_by_popup; {
        is_consumable_by_popup = false;
        if (!_SELECT_OR_DESELECT_COLOR()) is_consumable_by_popup |= key_is_digit;
        is_consumable_by_popup |= key_is_delete;
        is_consumable_by_popup |= key_is_enter;
        is_consumable_by_popup |= key_is_nav;
        is_consumable_by_popup |= key_is_ctrl_a;
        if (popup->_type_of_active_cell == CellType::Real) {
            is_consumable_by_popup |= key_is_hyphen;
            is_consumable_by_popup |= key_is_period;
        } else if (popup->_type_of_active_cell == CellType::Uint) {
            ;
        } else if (popup->_type_of_active_cell == CellType::String) {
            is_consumable_by_popup |= key_is_alpha;
            is_consumable_by_popup |= key_is_colon;
            is_consumable_by_popup |= key_is_hyphen;
            is_consumable_by_popup |= key_is_period;
            is_consumable_by_popup |= key_is_slash;
            is_consumable_by_popup |= key_is_space;
            is_consumable_by_popup |= key_is_underscore;
        } else {
            ASSERT(false);
        }
    }
    if (is_consumable_by_popup) return KeyEventSubtype::Popup;
    return KeyEventSubtype::Hotkey;
}

// NOTE: this function does state-dependent stuff (magic-snapping)
// NOTE: a lot of stuff is happening at once here:
//       pixel coords -> pre-snapped world coords -> snapped world-coords
// NOTE: this function can "drop" raw_event's by returning the null event.
//       this smells a bit (should probs fail earlier, but I like the previous layer not knowing this stuff)
Event bake_event(RawEvent raw_event) {
    Event event = {};
    if (raw_event.type == EventType::Key) {
        RawKeyEvent *raw_key_event = &raw_event.raw_key_event;

        event.type = EventType::Key;
        KeyEvent *key_event = &event.key_event;
        key_event->key = raw_key_event->key;
        key_event->control = raw_key_event->control;
        key_event->shift = raw_key_event->shift;
        key_event->alt = raw_key_event->alt;
        key_event->subtype = classify_baked_subtype_of_raw_key_event(raw_key_event); // NOTE: must come last
    } else { ASSERT(raw_event.type == EventType::Mouse);
        RawMouseEvent *raw_mouse_event = &raw_event.raw_mouse_event;

        event.type = EventType::Mouse;
        MouseEvent *mouse_event = &event.mouse_event;
        mouse_event->mouse_Pixel = raw_mouse_event->mouse_Pixel;
        mouse_event->mouse_held = raw_mouse_event->mouse_held;
        mouse_event->mouse_double_click = raw_mouse_event->mouse_double_click;
        mouse_event->mouse_double_click_held = raw_mouse_event->mouse_double_click_held;
        {
            if (raw_mouse_event->pane == Pane::Drawing) {
                mat4 World_2D_from_OpenGL = inverse(camera_drawing->get_PV());
                vec2 mouse_World_2D = transformPoint(World_2D_from_OpenGL, other.mouse_OpenGL);

                mouse_event->subtype = MouseEventSubtype::Drawing;

                MouseEventDrawing *mouse_event_drawing = &mouse_event->mouse_event_drawing;
                mouse_event_drawing->snap_result = magic_snap(mouse_World_2D);
            } else if (raw_mouse_event->pane == Pane::Mesh) {
                mat4 World_3D_from_OpenGL = inverse(camera_mesh->get_PV());
                vec3 point_a = transformPoint(World_3D_from_OpenGL, V3(other.mouse_OpenGL, -1.0f));
                vec3 point_b = transformPoint(World_3D_from_OpenGL, V3(other.mouse_OpenGL,  1.0f));

                mouse_event->subtype = MouseEventSubtype::Mesh;

                MouseEventMesh *mouse_event_mesh = &mouse_event->mouse_event_mesh;
                mouse_event_mesh->mouse_ray_origin = point_a;
                mouse_event_mesh->mouse_ray_direction = normalized(point_b - point_a);
            } else if (raw_mouse_event->pane == Pane::Popup) {
                mouse_event->subtype = MouseEventSubtype::Popup;
                MouseEventPopup *mouse_event_popup = &mouse_event->mouse_event_popup;
                FORNOW_UNUSED(mouse_event_popup);
            } else if (raw_mouse_event->pane == Pane::Toolbox) {
                mouse_event->subtype = MouseEventSubtype::ToolboxButton;
                MouseEventToolboxButton *mouse_event_toolbox_button = &mouse_event->mouse_event_toolbox_button;
                mouse_event_toolbox_button->name = toolbox->hot_name;
            } else { ASSERT(raw_mouse_event->pane == Pane::Separator);
                event = {};
            }
        }
    }
    return event;
}

////////////////////////////////////////////////////////////////////////////////

Event make_mouse_event_2D(vec2 mouse_position) {
    Event event = {};
    event.type = EventType::Mouse;
    MouseEvent *mouse_event = &event.mouse_event;
    mouse_event->subtype = MouseEventSubtype::Drawing;
    MouseEventDrawing *mouse_event_drawing = &mouse_event->mouse_event_drawing;
    mouse_event_drawing->snap_result.mouse_position = mouse_position;
    return event;
}
Event make_mouse_event_2D(real x, real y) { return make_mouse_event_2D({ x, y }); }

Event make_mouse_event_3D(vec3 mouse_ray_origin, vec3 mouse_ray_direction) {
    Event event = {};
    event.type = EventType::Mouse;
    MouseEvent *mouse_event = &event.mouse_event;
    mouse_event->subtype = MouseEventSubtype::Mesh;
    MouseEventMesh *mouse_event_mesh = &mouse_event->mouse_event_mesh;
    mouse_event_mesh->mouse_ray_origin = mouse_ray_origin;
    mouse_event_mesh->mouse_ray_direction = mouse_ray_direction;
    return event;
}

// <!> End bake.cpp <!>
// <!> Begin cookbook.cpp <!> 


struct Cookbook {
    Event event;
    StandardEventProcessResult *result;
    bool skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack;

    List<Entity> _add_buffer;
    List<uint> _delete_buffer;

    Entity _make_line(vec2 start, vec2 end, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = {};
        entity.type = EntityType::Line;
        entity.preview_color = get_color(ColorCode::Emphasis);
        LineEntity *line = &entity.line;
        line->start = start;
        line->end = end;
        entity.is_selected = is_selected;
        entity.color_code = color_code;
        return entity;
    };

    Entity _make_arc(vec2 center, real radius, real start_angle_in_degrees, real end_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = {};
        entity.type = EntityType::Arc;
        entity.preview_color = get_color(ColorCode::Emphasis);
        ArcEntity *arc = &entity.arc;
        arc->center = center;
        arc->radius = radius;
        arc->start_angle_in_degrees = start_angle_in_degrees;
        arc->end_angle_in_degrees = end_angle_in_degrees;
        entity.is_selected = is_selected;
        entity.color_code = color_code;
        return entity;
    };

    Entity _make_circle(vec2 center, real radius, bool has_pseudo_point, real pseudo_point_angle, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = {};
        entity.type = EntityType::Circle;
        entity.preview_color = get_color(ColorCode::Emphasis);
        CircleEntity *circle = &entity.circle;
        circle->center = center;
        circle->radius = radius;
        circle->has_pseudo_point = has_pseudo_point;
        circle->pseudo_point_angle = pseudo_point_angle;
        entity.is_selected = is_selected;
        entity.color_code = color_code;
        return entity;
    };

    void _add_entity(Entity entity) {
        ASSERT(_delete_buffer.length == 0);
        list_push_back(&drawing->entities, entity);
    };

    void _add_line(vec2 start, vec2 end, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = _make_line(start, end, is_selected, color_code);
        _add_entity(entity);
    };

    void _add_arc(vec2 center, real radius, real start_angle_in_degrees, real end_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = _make_arc(center, radius, start_angle_in_degrees, end_angle_in_degrees, is_selected, color_code);
        _add_entity(entity);
    };

    void _add_circle(vec2 center, real radius, bool has_pseudo_point, real pseudo_point_angle, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = _make_circle(center, radius, has_pseudo_point, pseudo_point_angle, is_selected, color_code);
        _add_entity(entity);
    };

    void _buffer_add_entity(Entity entity) {
        float LENGTH_CUTOFF = 0.003f;
        if (entity_length(&entity) < LENGTH_CUTOFF) { // TODO: define glorbal const for min len
            messagef("zero length entity not created");
        } else if (entity.type == EntityType::Arc && entity.arc.radius < LENGTH_CUTOFF) {
            messagef("zero length entity not created");
        } else if (entity.type == EntityType::Circle && entity.circle.radius < LENGTH_CUTOFF) {
            messagef("zero length entity not created");
        } else {
            list_push_back(&_add_buffer, entity);
        }
    };

    void buffer_add_line(vec2 start, vec2 end, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = _make_line(start, end, is_selected, color_code);
        _buffer_add_entity(entity);
    };

    void buffer_add_arc(vec2 center, real radius, real start_angle_in_degrees, real end_angle_in_degrees, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = _make_arc(center, radius, start_angle_in_degrees, end_angle_in_degrees, is_selected, color_code);
        _buffer_add_entity(entity);
    };

    void buffer_add_circle(vec2 center, real radius, bool has_pseudo_point, real pseudo_point_angle, bool is_selected = false, ColorCode color_code = ColorCode::Traverse) {
        Entity entity = _make_circle(center, radius, has_pseudo_point, pseudo_point_angle, is_selected, color_code);
        _buffer_add_entity(entity);
    };

    // void consume_buffered_adds() {
    // };


    void _delete_entity(uint i) {
        list_delete_at(&drawing->entities, i);
    };

    void _buffer_delete_entity_DEPRECATED_INDEX_VERSION(uint i) {
        list_push_back(&_delete_buffer, i);
    };

    void buffer_delete_entity(Entity *entity) {
        ASSERT(entity >= drawing->entities.array);
        uint i = uint(entity - drawing->entities.array);
        _buffer_delete_entity_DEPRECATED_INDEX_VERSION(i);
    };

    static int _compare_delete_buffer(const void *_a, const void *_b) {
        uint a = *((uint *)(_a));
        uint b = *((uint *)(_b));

        if (a > b) return -1;
        if (a < b) return 1;
        return 0;
    }

    void end_frame() {
        { // _delete_buffer
            qsort(
                    _delete_buffer.array,
                    _delete_buffer.length,
                    sizeof(_delete_buffer.array[0]),
                    _compare_delete_buffer
                 );
            for_(i, _delete_buffer.length) _delete_entity(_delete_buffer.array[i]);
            list_free_AND_zero(&_delete_buffer);
        }

        { // _add_buffer
            for_(i, _add_buffer.length) _add_entity(_add_buffer.array[i]);
            list_free_AND_zero(&_add_buffer);
        }
    };

    void entity_set_is_selected(Entity *entity, bool is_selected) {
        if (entity->is_selected != is_selected) {
            result->record_me = true;
            result->checkpoint_me = !((event.type == EventType::Mouse) && (event.mouse_event.mouse_held)); // FORNOW
            entity->is_selected = is_selected;
            entity->time_since_is_selected_changed = 0.0f;
        }
    };

    void set_is_selected_for_all_entities(bool is_selected) {
        _for_each_entity_ entity_set_is_selected(entity, is_selected);
    };

    void entity_set_color(Entity *entity, ColorCode color_code) {
        if (entity->color_code != color_code) {
            result->record_me = true;
            result->checkpoint_me = true;
            entity->color_code = color_code;
        }
    };

    // DOES NOT EXTEND Line
    void attempt_divide_entity_at_point(uint entity_index, vec2 point) {
        Entity *entity = &drawing->entities.array[entity_index];
        bool delete_flag = false;
        if (entity->type == EntityType::Line) {
            LineEntity *line = &entity->line;
            bool point_is_on_line = 0.001 > ABS(distance(point, line->start) + distance(point, line->end) - distance(line->start, line->end)); // FORNOW
            if (point_is_on_line) {
                if (distance(line->start, point) > TINY_VAL) {
                    buffer_add_line(line->start, point, false, entity->color_code);
                    delete_flag = true;
                }
                if (distance(point, line->end) > TINY_VAL) {
                    buffer_add_line(point, line->end, false, entity->color_code);
                    delete_flag = true;
                }
            }
        } else if (entity->type == EntityType::Arc) {
            ArcEntity *arc = &entity->arc;
            real angle = DEG(ATAN2(point - arc->center));
            if (abs(distance(point, arc->center) - arc->radius) < 0.001 && ANGLE_IS_BETWEEN_CCW_DEGREES(angle, arc->start_angle_in_degrees, arc->end_angle_in_degrees)) {
                if (!ARE_EQUAL(arc->start_angle_in_degrees, angle)) {
                    buffer_add_arc(arc->center, arc->radius, arc->start_angle_in_degrees, angle, false, entity->color_code);
                    delete_flag = true;
                }
                if (!ARE_EQUAL(arc->end_angle_in_degrees, angle)) {
                    buffer_add_arc(arc->center, arc->radius, angle, arc->end_angle_in_degrees, false, entity->color_code);
                    delete_flag = true;
                }

            }
        } else { ASSERT(entity->type == EntityType::Circle);
            CircleEntity *circle = &entity->circle;
            if (squared_distance_point_dxf_circle_entity(point, circle) < TINY_VAL) {
                if (!circle->has_pseudo_point) {
                    circle->has_pseudo_point = true;
                    circle->set_pseudo_point(point);
                } else {
                    if (ARE_EQUAL(circle->get_pseudo_point(), point)) {
                        ;
                    } else {
                        delete_flag = true;
                        real radius = distance(circle->center, point);
                        real angle1_in_degrees = DEG(circle->pseudo_point_angle);
                        real angle2_in_degrees = DEG(ATAN2(point - circle->center));
                        buffer_add_arc(circle->center, radius, angle1_in_degrees, angle2_in_degrees, false, entity->color_code);
                        buffer_add_arc(circle->center, radius, angle2_in_degrees, angle1_in_degrees, false, entity->color_code);
                    }
                }
            }
        }
        if (delete_flag) {
            _buffer_delete_entity_DEPRECATED_INDEX_VERSION(entity_index);
        }
    }

    void attempt_fillet_ENTITIES_GET_DELETED_AT_END_OF_FRAME(const Entity *E, const Entity *F, vec2 reference_point, real radius) {
        if (E == F) {
            messagef(pallete.orange, "Fillet: clicked same entity twice");
            return;
        }

        bool is_line_line = (E->type == EntityType::Line) && (F->type == EntityType::Line);
        bool is_line_arc_or_arc_line = (E->type == EntityType::Line && F->type == EntityType::Arc) || (E->type == EntityType::Arc && F->type == EntityType::Line);
        bool is_arc_arc = (E->type == EntityType::Arc && F->type == EntityType::Arc);


        if (is_line_line) {
            if (distance(E->line.start, E->line.end) < radius) {
                messagef(pallete.orange, "Fillet: first line too short for given radius");
                return;
            }
            if (distance(F->line.start, F->line.end) < radius) {
                messagef(pallete.orange, "Fillet: second line too short for given radius");
                return;
            }
        }

        if (is_line_line) {
            //  a -- b   x          a -- B-.  
            //                           |  - 
            //           d    =>         X - D
            //    p      |            p      |
            //           c                   c

            vec2 p = reference_point;
            vec2 a;
            vec2 b;
            vec2 c;
            vec2 d;
            vec2 x;
            vec2 e_ab;
            vec2 e_cd;
            {
                a = E->line.start;
                b = E->line.end;
                c = F->line.start;
                d = F->line.end;

                LineLineXResult _x = line_line_intersection(a, b, c, d);
                if (_x.lines_are_parallel) {
                    messagef(pallete.orange, "Fillet: lines are parallel");
                    return;
                }
                x = _x.point;

                e_ab = normalized(b - a);
                e_cd = normalized(d - c);

                bool swap_ab, swap_cd; {
                    vec2 v_xp_in_edge_basis = inverse(hstack(e_ab, e_cd)) * (p - x);
                    swap_ab = (v_xp_in_edge_basis.x > 0.0f);
                    swap_cd = (v_xp_in_edge_basis.y > 0.0f);
                }

                if (swap_ab) {
                    {
                        a = b;
                        b = E->line.start;
                    }
                    e_ab *= -1;
                }

                if (swap_cd) {
                    {
                        c = d;
                        d = F->line.start;
                    }
                    e_cd *= -1;
                }
            }

            { // add new lines and remove old ones
                real L; {
                    real full_angle = get_three_point_angle(a, x, c);
                    if (full_angle > PI) full_angle = TAU - full_angle;
                    L = radius / TAN(full_angle / 2);
                }
                b = x - (L * e_ab);
                d = x - (L * e_cd);
                Entity new_E = _make_line(a, b, E->is_selected, E->color_code);
                Entity new_F = _make_line(c, d, F->is_selected, F->color_code);

                // lowkey no idea what this does but copied for consistency 
                new_E.preview_color = get_color(ColorCode::Emphasis);
                new_F.preview_color = get_color(ColorCode::Emphasis);


                _buffer_add_entity(new_E);
                _buffer_add_entity(new_F);

                //buffer_delete_entity(E);
                //buffer_delete_entity(F);

            }

            // deal with creating the fillet arc
            vec2 X; {
                LineLineXResult _X = line_line_intersection(b, b + perpendicularTo(e_ab), d, d + perpendicularTo(e_cd));
                if (_X.lines_are_parallel) {
                    messagef(pallete.orange, "Fillet: ???");
                    return;
                }
                X = _X.point;
            }

            if (!IS_ZERO(radius)) { // arc
                real theta_b_in_degrees;
                real theta_d_in_degrees;
                {
                    theta_b_in_degrees = DEG(angle_from_0_TAU(X, b));
                    theta_d_in_degrees = DEG(angle_from_0_TAU(X, d));
                    if (get_three_point_angle(b, X, d) > PI) {
                        SWAP(&theta_b_in_degrees, &theta_d_in_degrees);
                    }
                }
                buffer_add_arc(X, radius, theta_b_in_degrees, theta_d_in_degrees, false, E->color_code);
            }

        } else if (is_line_arc_or_arc_line) { // this is a very straight forward function
                                              // general idea
                                              // 1. find where relative to line/arc intersection click is
                                              // 2. use that to get the fillet point
                                              // 3. ?????
                                              // 4, perfect fillet

            const Entity *EntL = E->type == EntityType::Line ? E : F;
            LineEntity line = EntL->line;

            const Entity *EntA = E->type == EntityType::Arc  ? E : F;
            ArcEntity arc = EntA->arc;

            LineArcXClosestResult intersection = line_arc_intersection_closest(&line, &arc, reference_point);

            if (intersection.no_possible_intersection) {
                messagef("FILLET: no intersection found");
                return;
            }

            // Determine if fillet should be inside or outside the circle
            real distance_second_click_center = distance(reference_point, arc.center);
            bool fillet_inside_circle = intersection.point_is_on_line_segment && (distance_second_click_center < arc.radius);

            // Get a line parallel to selected to determine where the fillet arc should be
            vec2 line_vector = line.end - line.start;
            bool line_left = cross(line_vector, reference_point - line.start) < 0;
            vec2 line_adjust = radius * normalized(perpendicularTo(line_vector)) * (line_left ? 1.0f : -1.0f);

            LineEntity new_line; // ! color, etc. undefined
            new_line.start = line.start + line_adjust; 
            new_line.end = line.end + line_adjust; 

            // Same thing but for the arc 
            real start_val = dot(normalized(intersection.point - arc.center), normalized(intersection.point - line.start)); 
            real end_val = dot(normalized(intersection.point - arc.center), normalized(intersection.point - line.end));
            bool start_inside_circle = start_val > -TINY_VAL;
            bool end_inside_circle = end_val > -TINY_VAL;
            if (abs(distance(intersection.point, line.start)) < 0.001f) {
                start_inside_circle = end_inside_circle;
            }
            if (abs(distance(intersection.point, line.end)) < 0.001f) {
                end_inside_circle = start_inside_circle;
            }
            if (start_inside_circle == end_inside_circle) { 
                fillet_inside_circle = end_inside_circle;
            }

            ArcEntity new_arc = arc;
            new_arc.radius += radius * (fillet_inside_circle ? -1 : 1);

            // calculate fillet center and intersections
            LineArcXClosestResult fillet_point = line_arc_intersection_closest(&new_line, &new_arc, reference_point);
            vec2 fillet_center = fillet_point.point;
            vec2 line_fillet_intersect = fillet_center - line_adjust;
            vec2 arc_fillet_intersect = fillet_center - radius * (fillet_inside_circle ? -1 : 1) * normalized(fillet_center - arc.center);

            // calculate fillet angles
            real fillet_line_theta = ATAN2(line_fillet_intersect - fillet_center);
            real fillet_arc_theta = ATAN2(arc_fillet_intersect - fillet_center);

            if (fmod(TAU + fillet_line_theta - fillet_arc_theta, TAU) > PI) {
                real temp = fillet_line_theta;
                fillet_line_theta = fillet_arc_theta;
                fillet_arc_theta = temp;
            }

            // make fillet arc
            Entity fillet_arc = _make_arc(fillet_center, radius, DEG(fillet_arc_theta), DEG(fillet_line_theta), false, E->color_code);
            if (radius > TINY_VAL) {
                _buffer_add_entity(fillet_arc);
            }

            // determine which end of the line should be changed
            bool end_in_direction = (dot(normalized(fillet_center - intersection.point), normalized(line.end - intersection.point)) > 0);
            bool start_in_direction = (dot(normalized(fillet_center - intersection.point), normalized(line.start - intersection.point)) > 0);
            bool extend_start;
            if (end_in_direction != start_in_direction) { 
                extend_start = end_in_direction;
            } else {
                extend_start = distance(intersection.point, line.end) > distance(intersection.point, line.start);
            }

            // handle zero radius case
            if (radius == 0 && (end_inside_circle != start_inside_circle)) {
                extend_start = fillet_inside_circle != start_inside_circle;
            }

            // add the new line
            if (extend_start) {
                buffer_add_line(line_fillet_intersect, EntL->line.end, EntL->is_selected, EntL->color_code);
            } else {
                buffer_add_line(EntL->line.start, line_fillet_intersect, EntL->is_selected, EntL->color_code);
            }

            // arc stuff
            real divide_theta = DEG(ATAN2(fillet_center - arc.center));
            real theta_where_line_was_tangent = DEG(ATAN2(line_fillet_intersect - arc.center));

            // kinda weird but checks if divide theta > theta where line was tangent
            vec2 middle_angle_vec = entity_get_middle(&fillet_arc);
            real fillet_middle_arc = DEG(ATAN2(middle_angle_vec - arc.center));

            // this is a slight nudge to ensure that the correct angle is adjusted
            if (ARE_EQUAL(divide_theta, theta_where_line_was_tangent)) {
                real offset = DEG(ATAN2(reference_point - arc.center)); 
                fillet_middle_arc += ANGLE_IS_BETWEEN_CCW_DEGREES(offset, divide_theta, divide_theta + 180.0f) ? -1.0f : 1.0f; 
            }

            // good luck
            if (!(ANGLE_IS_BETWEEN_CCW_DEGREES(divide_theta, arc.end_angle_in_degrees - 0.001f, arc.end_angle_in_degrees + 0.001f) || 
                        ANGLE_IS_BETWEEN_CCW_DEGREES(divide_theta, arc.start_angle_in_degrees - 0.001f, arc.start_angle_in_degrees + 0.001f))) {
                if (ANGLE_IS_BETWEEN_CCW_DEGREES(fillet_middle_arc, arc.start_angle_in_degrees, divide_theta)) {
                    buffer_add_arc(EntA->arc.center, EntA->arc.radius, divide_theta, EntA->arc.end_angle_in_degrees, EntA->is_selected, EntA->color_code);
                } else {
                    buffer_add_arc(EntA->arc.center, EntA->arc.radius, EntA->arc.start_angle_in_degrees, divide_theta, EntA->is_selected, EntA->color_code);
                }
            }
        } else { ASSERT(is_arc_arc);
            ArcEntity arc_a = E->arc;
            ArcEntity arc_b = F->arc;
            real _other_fillet_radius = radius + (radius == 0 ? 1 : 0);

            bool fillet_inside_arc_a = distance(arc_a.center, reference_point) < arc_a.radius;
            bool fillet_inside_arc_b = distance(arc_b.center, reference_point) < arc_b.radius;

            ArcEntity new_arc_a = arc_a;
            new_arc_a.radius = arc_a.radius + (fillet_inside_arc_a ? -1 : 1) * _other_fillet_radius;

            ArcEntity new_arc_b = arc_b;
            new_arc_b.radius = arc_b.radius + (fillet_inside_arc_b ? -1 : 1) * _other_fillet_radius;


            ArcArcXClosestResult fillet_point = arc_arc_intersection_closest(&new_arc_a, &new_arc_b, reference_point);

            if (fillet_point.no_possible_intersection) {
                messagef("FILLET: no intersection found");
                return;
            }

            vec2 fillet_center = fillet_point.point;
            vec2 arc_a_fillet_intersect = fillet_center - _other_fillet_radius * (fillet_inside_arc_a ? -1 : 1) * normalized(fillet_center - arc_a.center);
            vec2 arc_b_fillet_intersect = fillet_center - _other_fillet_radius * (fillet_inside_arc_b ? -1 : 1) * normalized(fillet_center - arc_b.center);
            real fillet_arc_a_theta = ATAN2(arc_a_fillet_intersect - fillet_center);
            real fillet_arc_b_theta = ATAN2(arc_b_fillet_intersect - fillet_center);

            // a swap so the fillet goes the right way
            // (smallest angle
            if (fmod(TAU + fillet_arc_a_theta - fillet_arc_b_theta, TAU) < PI) {
                real temp = fillet_arc_b_theta;
                fillet_arc_b_theta = fillet_arc_a_theta;
                fillet_arc_a_theta = temp;
            }
            Entity fillet_arc = _make_arc(fillet_center, _other_fillet_radius, DEG(fillet_arc_a_theta), DEG(fillet_arc_b_theta), false, E->color_code); // if this is changed to radius it breaks, dont ask me why
            if (radius > TINY_VAL) {
                _buffer_add_entity(fillet_arc);
            }

            real divide_theta_a = DEG(ATAN2(fillet_center - arc_a.center));
            real divide_theta_b = DEG(ATAN2(fillet_center - arc_b.center));
            if (radius == 0) {
                ArcArcXClosestResult zero_intersect = arc_arc_intersection_closest(&arc_a, &arc_b, reference_point);
                divide_theta_a = zero_intersect.theta_a;
                divide_theta_b = zero_intersect.theta_b;
            }

            vec2 middle_angle_vec = entity_get_middle(&fillet_arc);
            real fillet_middle_arc_a = DEG(ATAN2(middle_angle_vec - arc_a.center));
            real fillet_middle_arc_b = DEG(ATAN2(middle_angle_vec - arc_b.center));
            if ((radius == 0) != ANGLE_IS_BETWEEN_CCW_DEGREES(fillet_middle_arc_a, arc_a.start_angle_in_degrees, divide_theta_a)) {
                buffer_add_arc(E->arc.center, E->arc.radius, divide_theta_a, E->arc.end_angle_in_degrees, E->is_selected, E->color_code);
            } else {
                buffer_add_arc(E->arc.center, E->arc.radius, E->arc.start_angle_in_degrees, divide_theta_a, E->is_selected, E->color_code);
            }
            if ((radius == 0) != ANGLE_IS_BETWEEN_CCW_DEGREES(fillet_middle_arc_b, arc_b.start_angle_in_degrees, divide_theta_b)) {
                buffer_add_arc(F->arc.center, F->arc.radius, divide_theta_a, F->arc.end_angle_in_degrees, F->is_selected, F->color_code);
            } else {
                buffer_add_arc(F->arc.center, F->arc.radius, F->arc.start_angle_in_degrees, divide_theta_a, F->is_selected, F->color_code);
            }
        }

        // least sus thing ever
        buffer_delete_entity((Entity *)E);
        buffer_delete_entity((Entity *)F);

    }

    void attempt_dogear(Entity *E, Entity *F, vec2 reference_point, real radius) {
        if (E == F) {
            messagef(pallete.orange, "DogEar: clicked same entity twice");
            return;
        }

        if (IS_ZERO(radius)) {
            messagef(pallete.orange, "DogEar: FORNOW: must have non-zero radius");
            return;
        }

        bool is_line_line = (E->type == EntityType::Line) && (F->type == EntityType::Line);
        if (!is_line_line) {
            messagef(pallete.orange, "DogEar: only line-line is supported");
            return;
        }

        //                                    ,--.
        //  a -- b      x          a -- b    e     x
        //                                  :   y  :
        //                                  ,      f
        //                   =>              +.__.'
        //    p         d            p             d
        //              |                          |
        //              c                          c

        // FORNOW: this block is repeated from fillet
        vec2 p = reference_point;
        vec2 a;
        vec2 b;
        vec2 c;
        vec2 d;
        vec2 x;
        vec2 e_ab;
        vec2 e_cd;
        // vec2 *b_ptr;
        // vec2 *d_ptr;
        {
            a     =  E->line.start;
            b     =  E->line.end;
            // b_ptr = &E->line.end;
            c     =  F->line.start;
            d     =  F->line.end;
            // d_ptr = &F->line.end;

            LineLineXResult _x = line_line_intersection(a, b, c, d);
            if (_x.lines_are_parallel) {
                messagef(pallete.orange, "Fillet: lines are parallel");
                return;
            }
            x = _x.point;

            e_ab = normalized(b - a);
            e_cd = normalized(d - c);

            bool swap_ab, swap_cd; {
                vec2 v_xp_in_edge_basis = inverse(hstack(e_ab, e_cd)) * (p - x);
                swap_ab = (v_xp_in_edge_basis.x > 0.0f);
                swap_cd = (v_xp_in_edge_basis.y > 0.0f);
            }

            if (swap_ab) {
                SWAP(&a, &b);
                e_ab *= -1;
                // b_ptr = &E->line.start;
            }

            if (swap_cd) {
                SWAP(&c, &d);
                e_cd *= -1;
                // d_ptr = &F->line.start;
            }
        }

        vec2 y;
        vec2 e_xy;
        real theta_yx_in_degrees;
        {
            e_xy = -normalized(e_ab + e_cd);
            y = x + radius * e_xy;
            theta_yx_in_degrees = DEG(ATAN2(-e_xy));
        }

        Entity G = _make_arc(y, radius, theta_yx_in_degrees - 180.0f, theta_yx_in_degrees + 180.0f, false, E->color_code);

        vec2 e;
        vec2 f;
        {
            // FORNOW: sloppy (use of a, c is wrong i think)
            LineArcXClosestResult _e = line_arc_intersection_closest(&E->line, &G.arc, a);
            ASSERT(!_e.no_possible_intersection);
            e = _e.point;
            LineArcXClosestResult _f = line_arc_intersection_closest(&F->line, &G.arc, c);
            ASSERT(!_f.no_possible_intersection);
            f = _f.point;
        }

        attempt_fillet_ENTITIES_GET_DELETED_AT_END_OF_FRAME(E, &G, e + (e - y), radius);
        attempt_fillet_ENTITIES_GET_DELETED_AT_END_OF_FRAME(F, &G, f + (f - y), radius);

        #if 0
        // // single arc version
        _buffer_add_entity(G);
        #else
        // // split arc version
        Entity G1;
        Entity G2;
        {
            G1 = G;
            G2 = G;
            real half_theta_in_degrees =  0.5f * _WRAP_TO_0_360_INTERVAL(G.arc.end_angle_in_degrees - G.arc.start_angle_in_degrees);
            G1.arc.end_angle_in_degrees -= half_theta_in_degrees;
            G2.arc.start_angle_in_degrees = G1.arc.end_angle_in_degrees;
        }
        _buffer_add_entity(G1);
        _buffer_add_entity(G2);
        #endif
    }


    ///////////
    // SCARY //
    ///////////

    void manifold_wrapper() {
        bool add = ((state_Mesh_command_is_(ExtrudeAdd)) || (state_Mesh_command_is_(RevolveAdd)));
        bool revolve = ((state_Mesh_command_is_(RevolveAdd)) || (state_Mesh_command_is_(RevolveCut)));
        if (!skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack) {
            result->record_me = true;
            result->snapshot_me = true;
            result->checkpoint_me = true;
            other.time_since_successful_feature = 0.0f;

            real out_quantity;
            real in_quantity;
            {
                if (!revolve) { // extrude
                    out_quantity = (add) ? popup->extrude_add_out_length : popup->extrude_cut_out_length;
                    in_quantity = (add) ? popup->extrude_add_in_length : popup->extrude_cut_in_length;
                } else {
                    out_quantity = (add) ? popup->revolve_add_out_angle : popup->revolve_cut_out_angle;
                    in_quantity = (add) ? popup->revolve_add_in_angle : popup->revolve_cut_in_angle;
                }

            }

            { // result.mesh
                CrossSectionEvenOdd cross_section = cross_section_create_FORNOW_QUADRATIC(&drawing->entities, true);
                Mesh tmp = wrapper_manifold(
                        &state.mesh,
                        cross_section.num_polygonal_loops,
                        cross_section.num_vertices_in_polygonal_loops,
                        cross_section.polygonal_loops,
                        get_M_3D_from_2D(),
                        state.Mesh_command,
                        out_quantity,
                        in_quantity,
                        drawing->origin,
                        drawing->axis_base_point,
                        drawing->axis_angle_from_y);
                cross_section_free(&cross_section);

                mesh_free_AND_zero(&state.mesh); // FORNOW
                state.mesh = tmp; // FORNOW
            }
        }

        // reset some stuff
        set_state_Mesh_command(None);
        set_is_selected_for_all_entities(false);
    };
};

Cookbook make_Cookbook(Event event, StandardEventProcessResult *result, bool skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack) {
    Cookbook cookbook = {};
    cookbook.event = event;
    cookbook.result = result;
    cookbook.skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack = skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack;
    return cookbook;
}

void cookbook_free(Cookbook *cookbook) {
    list_free_AND_zero(&cookbook->_add_buffer);
    list_free_AND_zero(&cookbook->_delete_buffer);
}

// <!> End cookbook.cpp <!>
// <!> Begin button.cpp <!> 
// <!> End button.cpp <!>
// <!> Begin process.cpp <!> 
// TODO: rz needs work

// TODO: beautiful buttons; should indicate what's selected in green (persistent)
// FORNOW: rotate copy's usage of second click is wonky

// XXXX: popups should have titles

// TODO: pressing F when Fillet already active should highlight popup field

// TODO: linear copy like layout
// TODO: ???linear copy shouldn't be able to snap to the thing that 

// TODO: mouse still pops on undo/redo
// ~~~~: snaps flicker when typing in the popup 

// XXXX: click modifier belongs in other

StandardEventProcessResult _standard_event_process_NOTE_RECURSIVE(Event event);
StandardEventProcessResult standard_event_process(Event event) {
    // bool global_world_state_changed;
    popup->manager.begin_process(); // NOTE: this is actually why we *need* this helper
                                    //       (the auto-recording is just a bonus)
    StandardEventProcessResult result = _standard_event_process_NOTE_RECURSIVE(event);
    // if (global_world_state_changed) {
    //     result.record_me = true;
    // }

    // // popup->manager.end_process();
    // FORNOW: horrifying workaround
    if (popup->manager.focus_group != ToolboxGroup::None) {
        Command check; {
            check = {};
            // FORNOW only checking the groups that make popups (Aug 14, 2024)
            if (popup->manager.focus_group == ToolboxGroup::Draw) {
                check = state.Draw_command;
            } else if (popup->manager.focus_group == ToolboxGroup::Mesh) {
                check = state.Mesh_command;
            } else if (popup->manager.focus_group == ToolboxGroup::Snap) {
                check = state.Snap_command;
            }
        }
        if (check.group == ToolboxGroup::None) {
            popup->manager.focus_group = ToolboxGroup::None;
        }
    }
    return result;
}

StandardEventProcessResult _standard_event_process_NOTE_RECURSIVE(Event event) {
    event_passed_to_popups = event;
    already_processed_event_passed_to_popups = false;


    void history_printf_script(); // FORNOW forward declaration

    bool skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack = event.snapshot_me;

    bool dxf_anything_selected; {
        dxf_anything_selected = false;
        _for_each_selected_entity_ {
            dxf_anything_selected = true;
            break;
        }
    }
    bool value_to_write_to_selection_mask = (state_Draw_command_is_(Select));


    StandardEventProcessResult result = {};

    Cookbook cookbook = make_Cookbook(event, &result, skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack);
    defer {
        cookbook.end_frame();
        cookbook_free(&cookbook);
    };


    bool is_toolbox_button_mouse_event =
        (event.type == EventType::Mouse)
        && (event.mouse_event.subtype == MouseEventSubtype::ToolboxButton);

    if ((event.type == EventType::Key) || is_toolbox_button_mouse_event) {
        KeyEvent *key_event = &event.key_event;
        if ((key_event->subtype == KeyEventSubtype::Hotkey) || is_toolbox_button_mouse_event) {
            result.record_me = true;

            *toolbox = {};

            real padding = 8.0f;

            real w = 80.0f;

            EasyTextPen Draw_pen = { V2(padding, padding), 12.0f, pallete.white, true };
            EasyTextPen Draw_pen2 = Draw_pen;
            Draw_pen2.font_height_Pixel = 12.0f;
            Draw_pen2.color = pallete.light_gray;

            real h = Draw_pen.font_height_Pixel + Draw_pen2.font_height_Pixel;

            EasyTextPen Xsel_pen = Draw_pen;
            EasyTextPen Xsel_pen2 = Draw_pen2;
            Xsel_pen.origin.x += (w + padding) - 4.0f;
            Xsel_pen2.origin = Xsel_pen.origin;

            EasyTextPen Snap_pen = Xsel_pen;
            EasyTextPen Snap_pen2 = Xsel_pen2;

            EasyTextPen Colo_pen = Snap_pen;
            EasyTextPen Colo_pen2 = Snap_pen2;
            if (command_equals(state.Xsel_command, commands.ByColor)) {
                Colo_pen.origin.y += 116.0f;
            }
            Colo_pen2.origin = Colo_pen.origin;


            EasyTextPen Both_pen = Draw_pen;
            EasyTextPen Both_pen2 = Draw_pen2;
            Both_pen.origin.x = get_x_divider_drawing_mesh_Pixel() - (w / 2);
            Both_pen2.origin = Both_pen.origin;

            EasyTextPen Mesh_pen = Draw_pen;
            EasyTextPen Mesh_pen2 = Draw_pen2;
            Mesh_pen.origin.x = window_get_width_Pixel() - (w + padding);
            Mesh_pen.origin.y += 64.0f;
            Mesh_pen2.origin = Mesh_pen.origin;


            bool special_case_started_frame_with_snaps_enabled_NOTE_fixes_partial_snap_toolbox_graphical_glitch = (state.Draw_command.flags & SNAPPER);

            // TODO: have the non-toggle-able buttons a different color (like before)
            Map<Shortcut, bool> shortcut_already_checked = {};
            defer { map_free_and_zero(&shortcut_already_checked); };
            bool hotkey_consumed_by_GUIBUTTON = false;
            ToolboxGroup most_recent_group_for_SEPERATOR = ToolboxGroup::None;
            auto GUIBUTTON = [&](Command command, bool hide_button = false, bool deactivate_hotkey = false) -> bool {
                most_recent_group_for_SEPERATOR = command.group;
                bool gray_out_shortcut;
                gray_out_shortcut = false; // SUPPRESS COMPILER WARNING
                if (!other._please_suppress_drawing_popup_popup) {
                    if (!deactivate_hotkey && !map_get(&shortcut_already_checked, command.shortcuts[0], false)) {
                        map_put(&shortcut_already_checked, command.shortcuts[0], true);
                        gray_out_shortcut = false;
                    } else {
                        gray_out_shortcut = true;
                    }
                }


                real w = 80.0f;
                ToolboxGroup group = command.group;
                bool is_mode = command.is_mode;
                u64 flags = command.flags;
                String name = command.name;

                bool control = command.shortcuts[0].mods & GLFW_MOD_CONTROL;
                bool shift = command.shortcuts[0].mods & GLFW_MOD_SHIFT;
                bool alt = command.shortcuts[0].mods & GLFW_MOD_ALT;
                uint key = command.shortcuts[0].key;

                bool special_case_dont_draw_toolbox_NOTE_fixes_undo_graphical_glitch = (other._please_suppress_drawing_popup_popup && (group == ToolboxGroup::Snap));
                bool draw_tool = name.data;

                if (1
                        && (!other.hide_toolbox || command_equals(command, commands.ToggleGUI))
                        && (!other._please_suppress_drawing_popup_popup)
                        && (!hide_button)
                        && (group != ToolboxGroup::None)
                        && draw_tool
                        && !special_case_dont_draw_toolbox_NOTE_fixes_undo_graphical_glitch
                        && ((group != ToolboxGroup::Snap) || special_case_started_frame_with_snaps_enabled_NOTE_fixes_partial_snap_toolbox_graphical_glitch)
                   ) {
                    EasyTextPen *pen;
                    EasyTextPen *pen2;
                    bool horz = false;
                    if (group == ToolboxGroup::Draw) {
                        pen = &Draw_pen;
                        pen2 = &Draw_pen2;
                    } else if (group == ToolboxGroup::Xsel) {
                        pen = &Xsel_pen;
                        pen2 = &Xsel_pen2;
                        w = 64.0f;
                    } else if (group == ToolboxGroup::Snap) {
                        pen = &Snap_pen;
                        pen2 = &Snap_pen2;
                        w = 64.0f;
                    } else if (group == ToolboxGroup::Colo) {
                        pen = &Colo_pen;
                        pen2 = &Colo_pen2;
                        w = 64.0f;
                    } else if (group == ToolboxGroup::Both) {
                        pen = &Both_pen;
                        pen2 = &Both_pen2;
                    } else { ASSERT(group == ToolboxGroup::Mesh);
                        pen = &Mesh_pen;
                        pen2 = &Mesh_pen2;
                    }

                    real y = pen->get_y_Pixel();
                    bbox2 bbox = { pen->origin.x, y - 2, pen->origin.x + w, y + h };

                    bool hovering = bbox_contains(bbox, other.mouse_Pixel);

                    if (hovering) {
                        *toolbox = {};
                        toolbox->hot_name = name.data;
                    }

                    vec3 color;
                    {

                        vec3 accent_color = get_accent_color(group); 
                        if (group == ToolboxGroup::Colo) {
                            for_(i, 10) if (command_equals(command, commands_Color[i])) { accent_color = get_color(ColorCode(i)); break; }
                        }

                        bool can_toggle = is_mode;
                        bool toggled;
                        {
                            toggled = false;
                            if (can_toggle) {
                                {
                                    toggled = false;
                                    if (0) {
                                    } else if (group == ToolboxGroup::Draw) {
                                        toggled = command_equals(state.Draw_command, command);
                                    } else if (group == ToolboxGroup::Snap) {
                                        toggled = command_equals(state.Snap_command, command);
                                    } else if (group == ToolboxGroup::Mesh) {
                                        toggled = command_equals(state.Mesh_command, command);
                                    } else if (group == ToolboxGroup::Colo) {
                                        toggled = command_equals(state.Colo_command, command);
                                    } else if (group == ToolboxGroup::Xsel) {
                                        toggled = command_equals(state.Xsel_command, command);
                                    }
                                }
                            }
                        }

                        // vec3 base_color = (can_toggle) ? AVG(pallete.black, pallete.dark_gray) : pallete.dark_gray;
                        vec3 base_color = pallete.dark_gray;
                        if (group == ToolboxGroup::Colo) {
                            base_color = LERP(0.20f, pallete.darker_gray, accent_color);
                        }

                        color = (hovering)
                            ? ((other.mouse_left_drag_pane == Pane::Toolbox) ? AVG(pallete.white, accent_color) : accent_color)
                            : ((toggled) ? accent_color : base_color);

                        if (can_toggle) {
                            eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
                            eso_overlay(true);
                            eso_color(color);
                            eso_bbox_SOUP_QUADS(bbox);
                            eso_end();
                        } else {
                            eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
                            eso_overlay(true);
                            eso_color(color);
                            real r = h / 2;
                            eso_bbox_SOUP_QUADS(bbox_inflate(bbox, { -r, 0.0f }));
                            eso_bbox_SOUP_QUADS(bbox_inflate(bbox, { 0.0f, -r }));
                            eso_end();
                            eso_begin(other.OpenGL_from_Pixel, SOUP_POINTS);
                            eso_size(2 * r);
                            eso_color(color);
                            eso_vertex(bbox.min + V2(r));
                            eso_vertex(bbox.max - V2(r));
                            eso_vertex(bbox.min.x + r, bbox.max.y - r);
                            eso_vertex(bbox.max.x - r, bbox.min.y + r);
                            eso_end();
                        }
                    }




                    vec3 tmp_pen_color = pen->color;
                    vec3 tmp_pen2_color = pen2->color;
                    {
                        // pen->color = V3(1.0f) - color;
                        if (AVG(color) > 0.5f) {
                            pen->color = V3(0.0f);
                        } else {
                            pen->color = V3(1.0f);
                        }
                    }
                    {
                        if (gray_out_shortcut) {
                            pen2->color = LERP(0.8f, pen->color, color);
                        } else {
                            pen2->color = LERP(0.2f, pen->color, color);
                        }
                    }

                    KeyEvent tmp = { {}, key, control, shift, alt };
                    pen->offset_Pixel.x = 0.5f * (w - _easy_text_dx(pen, name));
                    pen->offset_Pixel.x = ROUND(pen->offset_Pixel.x);
                    easy_text_draw(pen, name);
                    pen2->offset_Pixel.y = pen->offset_Pixel.y;
                    pen2->offset_Pixel.x = 0.5f * (w - _easy_text_dx(pen2, key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(&tmp)));
                    pen2->offset_Pixel.x = ROUND(pen2->offset_Pixel.x);
                    // if (!gray_out_shortcut) {
                    //     easy_text_drawf(pen2, key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(&tmp));
                    // } else {
                    //     easy_text_drawf(pen2, "");
                    // }
                    easy_text_drawf(pen2, key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(&tmp));

                    pen->color = tmp_pen_color;
                    pen2->color = tmp_pen2_color;





                    pen->offset_Pixel.y = pen2->offset_Pixel.y + 4;
                    if (horz) {
                        pen->offset_Pixel = {};
                        pen2->offset_Pixel = {};
                        pen->origin.x += w + 2;
                        pen2->origin.x += w + 2;
                    }
                }

                // TODO: ScriptEvent
                // TODO: this should store whether we're hovering in toolbox as well as the event that will be generated if we click

                if (hotkey_consumed_by_GUIBUTTON) return false;

                bool inner_result; {
                    inner_result = false;
                    if (is_toolbox_button_mouse_event) {
                        inner_result |= (name.data == event.mouse_event.mouse_event_toolbox_button.name);
                    } else if (!deactivate_hotkey) {
                        for (Shortcut *shortcut = command.shortcuts; shortcut < command.shortcuts + COMMAND_MAX_NUM_SHORTCUTS; ++shortcut) {
                            bool tmp = _key_lambda(key_event, shortcut->key, (shortcut->mods & GLFW_MOD_CONTROL), (shortcut->mods & GLFW_MOD_SHIFT));
                            inner_result |= tmp;
                            hotkey_consumed_by_GUIBUTTON |= tmp;
                        }
                    }
                }

                // canned logic
                if (inner_result) {
                    if (flags & TWO_CLICK) {
                        ASSERT(is_mode);
                        two_click_command->awaiting_second_click = false;
                        two_click_command->tangent_first_click = false;

                        mesh_two_click_command->awaiting_second_click = false;
                    }
                    if (flags & FOCUS_THIEF) {
                        popup->manager.manually_set_focus_group(group);
                    }
                    if (flags & NO_RECORD) {
                        result.record_me = false;
                    }
                    if (is_mode) {
                        if (0) {
                        } else if (group == ToolboxGroup::Draw) {
                            state.Draw_command = command;
                            state.Snap_command = commands.None;
                            state.Xsel_command = commands.None;
                            state.Colo_command = commands.None;
                        } else if (group == ToolboxGroup::Snap) {
                            state.Snap_command = command;
                        } else if (group == ToolboxGroup::Mesh) {
                            state.Mesh_command = command;
                        } else if (group == ToolboxGroup::Xsel) {
                            state.Xsel_command = command;
                        } else if (group == ToolboxGroup::Colo) {
                        } else { ASSERT(0);
                        }
                    }
                }

                return inner_result;
            };

            auto SEPERATOR = [&]() {
                if (other.hide_toolbox) return;

                ToolboxGroup group = most_recent_group_for_SEPERATOR;
                ASSERT(group != ToolboxGroup::None);

                real eps = 4;
                if (group == ToolboxGroup::Draw) {
                    Draw_pen.offset_Pixel.y += eps;
                } else if (group == ToolboxGroup::Snap) {
                    messagef(pallete.red, "horrifying stuff with Snap pen origin/offset");
                } else if (group == ToolboxGroup::Mesh) {
                    Mesh_pen.offset_Pixel.y += eps;
                } else { ASSERT(group == ToolboxGroup::Both);
                    Both_pen.offset_Pixel.y += eps;
                }
            };

            Command prev_Draw_command = state.Draw_command;
            Command prev_Mesh_command = state.Mesh_command;
            { // GUIBUTTON

                // NOTE: ordered by priority

                { // Both
                    if (GUIBUTTON(commands.ToggleGUI)) { 
                        other.hide_toolbox = !other.hide_toolbox;
                    }

                    SEPERATOR();

                    { // Escape
                        if (GUIBUTTON(commands.Escape)) {
                            // do_once { messagef(pallete.orange, "ESCAPE maybe sus."); };
                            if (popup->manager.focus_group == ToolboxGroup::Draw) {
                                if (!state_Draw_command_is_(None)) {
                                    set_state_Draw_command(None);
                                    set_state_Snap_command(None);
                                    set_state_Colo_command(None);
                                } else {
                                    // Size, Load, Save...
                                    set_state_Mesh_command(None);
                                }
                            } else if (popup->manager.focus_group == ToolboxGroup::Mesh) {
                                set_state_Mesh_command(None);
                            } else if (popup->manager.focus_group == ToolboxGroup::Snap) {
                                set_state_Snap_command(None);
                            } else {
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);
                                set_state_Colo_command(None);
                                set_state_Mesh_command(None);
                                set_state_Xsel_command(None);
                            }
                        }
                    }

                    SEPERATOR();

                    { // undo
                        if (GUIBUTTON(commands.Undo)) {
                            other._please_suppress_drawing_popup_popup = true;
                            history_undo();
                        }
                    }

                    { // redo
                        bool button_redo = GUIBUTTON(commands.Redo);
                        // bool hotkey_redo_alternate = GUIBUTTON(commands.REDO_ALTERNATE);
                        // bool hotkey_redo_alternate_alternate = GUIBUTTON(commands.REDO_ALTERNATE_ALTERNATE);
                        if (button_redo) {
                            // _standard_event_process_NOTE_RECURSIVE({}); // FORNOW (prevent flicker on redo with nothing left to redo)
                            other._please_suppress_drawing_popup_popup = true;
                            history_redo();

                        }
                    }
                    SEPERATOR();
                    if (GUIBUTTON(commands.ToggleDetails)) { 
                        other.show_details = !other.show_details;
                    }
                }

                { // Colo
                    { // OfSelection
                        if (state_Draw_command_is_(SetColor)) {
                            if (GUIBUTTON(commands.OfSelection)) { 
                                set_state_Colo_command(OfSelection);
                            }
                        }
                    }
                    { // ColorX
                        bool hide_buttons = !(0
                                || state_Draw_command_is_(SetColor)
                                || ((SELECT_OR_DESELECT()) && (state_Xsel_command_is_(ByColor)))
                                );
                        bool spoof_is_mode_false = 0
                            || (state_Draw_command_is_(SetColor) && state_Colo_command_is_(OfSelection))
                            || (SELECT_OR_DESELECT() && state_Xsel_command_is_(ByColor))
                            ;
                        if (true) {
                            bool hotkey_quality;
                            uint digit = 0;
                            {
                                hotkey_quality = false;
                                for_(color, 9) {
                                    Command command = commands_Color[color];
                                    if (spoof_is_mode_false) command.is_mode = false;
                                    if (GUIBUTTON(command, hide_buttons)) {
                                        hotkey_quality = true;
                                        digit = color;
                                        break;
                                    }
                                }
                            }

                            if (hotkey_quality) {
                                if (SELECT_OR_DESELECT() && (state_Xsel_command_is_(ByColor))) { // [sd]q0
                                    _for_each_entity_ {
                                        uint i = uint(entity->color_code);
                                        if (i != digit) continue;
                                        cookbook.entity_set_is_selected(entity, value_to_write_to_selection_mask);
                                    }
                                    set_state_Draw_command(None);
                                    set_state_Xsel_command(None);
                                } else if ((state_Draw_command_is_(SetColor)) && (state_Colo_command_is_(OfSelection))) { // qs0
                                    _for_each_selected_entity_ cookbook.entity_set_color(entity, ColorCode(digit));
                                    set_state_Draw_command(None);
                                    set_state_Colo_command(None);
                                    _for_each_entity_ entity->is_selected = false;
                                } else { // 0
                                    set_state_Draw_command(SetColor);
                                    state.Colo_command = commands_Color[digit];
                                    set_state_Snap_command(None);
                                }
                            }
                        }
                    }
                }

                { // Xsel
                    if (SELECT_OR_DESELECT()) {
                        if (GUIBUTTON(commands.All)) { 
                            result.checkpoint_me = true;
                            cookbook.set_is_selected_for_all_entities(state_Draw_command_is_(Select));
                            set_state_Draw_command(None);
                            set_state_Xsel_command(None);
                        }
                        GUIBUTTON(commands.Connected);
                        GUIBUTTON(commands.Window);
                        bool deactive_sq = !command_equals(state.Xsel_command, commands.None);
                        GUIBUTTON(commands.ByColor, false, deactive_sq);
                    }
                }

                { // Snap 
                    if (state.Draw_command.flags & SNAPPER) {
                        if (GUIBUTTON(commands.Center)) preview->mouse_snap = preview->mouse; // FORNOW
                        if (GUIBUTTON(commands.End)) preview->mouse_snap = preview->mouse; // FORNOW
                        if (GUIBUTTON(commands.Intersect)) preview->mouse_snap = preview->mouse; // FORNOW
                        if (GUIBUTTON(commands.Middle)) preview->mouse_snap = preview->mouse; // FORNOW
                        if (GUIBUTTON(commands.Perp)) preview->mouse_snap = preview->mouse; // FORNOW
                        if (GUIBUTTON(commands.Quad)) preview->mouse_snap = preview->mouse; // FORNOW
                                                                                            //if (GUIBUTTON(commands.Tangent)) preview->mouse_snap = preview->mouse; // FORNOW
                        if (GUIBUTTON(commands.XY)) preview->xy_xy = preview->mouse; // FORNOW
                        if (GUIBUTTON(commands.Zero)) {
                            Event equivalent = {};
                            equivalent.type = EventType::Mouse;
                            equivalent.mouse_event.subtype = MouseEventSubtype::Drawing;
                            return _standard_event_process_NOTE_RECURSIVE(equivalent);
                        }
                        if (!command_equals(state.Snap_command, commands.None)) {
                            if (GUIBUTTON(commands.ClearSnap)) {
                                state.Snap_command = commands.None;
                            }
                        }
                    }
                }

                { // Draw
                    GUIBUTTON(commands.Select);
                    GUIBUTTON(commands.Deselect);
                    SEPERATOR();
                    if (GUIBUTTON(commands.Delete)) {
                        // trust me you want this code (imagine deleting stuff while in the middle of a two click command)
                        set_state_Draw_command(None);
                        set_state_Snap_command(None);
                        set_state_Xsel_command(None);
                        set_state_Colo_command(None);

                        bool checkpoint = false;
                        for (int i = drawing->entities.length - 1; i >= 0; --i) {
                            if (drawing->entities.array[i].is_selected) {
                                cookbook._delete_entity(i);
                                checkpoint = true;
                            }
                        }
                        result.checkpoint_me = checkpoint;
                    }
                    SEPERATOR();
                    if (GUIBUTTON(commands.SetColor)) set_state_Colo_command(Color0);
                    SEPERATOR();
                    GUIBUTTON(commands.Line);
                    GUIBUTTON(commands.Circle);
                    GUIBUTTON(commands.Box);
                    if (GUIBUTTON(commands.Polygon)) preview->polygon_num_sides = popup->polygon_num_sides;
                    // SEPERATOR();
                    // GUIBUTTON(commands.DiamCircle);
                    GUIBUTTON(commands.CenterLine);
                    GUIBUTTON(commands.CenterBox);
                    SEPERATOR();
                    GUIBUTTON(commands.Measure);
                    SEPERATOR();
                    GUIBUTTON(commands.Move);
                    GUIBUTTON(commands.Drag);
                    GUIBUTTON(commands.Rotate);
                    GUIBUTTON(commands.Scale);
                    SEPERATOR();
                    GUIBUTTON(commands.Copy);
                    GUIBUTTON(commands.RCopy);
                    SEPERATOR();
                    GUIBUTTON(commands.MirrorX);
                    GUIBUTTON(commands.MirrorY);
                    // GUIBUTTON(commands.Mirror2);
                    SEPERATOR();
                    GUIBUTTON(commands.Fillet);
                    GUIBUTTON(commands.DogEar);
                    GUIBUTTON(commands.Offset);
                    GUIBUTTON(commands.Divide2);
                    SEPERATOR();
                    GUIBUTTON(commands.SetOrigin);
                    GUIBUTTON(commands.SetAxis);
                    SEPERATOR();
                    if (GUIBUTTON(commands.ClearDrawing)) {
                        result.checkpoint_me = true;
                        result.snapshot_me = true;
                        list_free_AND_zero(&drawing->entities);
                        *drawing = {};
                        messagef(pallete.light_gray, "ClearDrawing");
                    }
                    if (GUIBUTTON(commands.ZoomDrawing)) {
                        init_camera_drawing();
                    }
                    SEPERATOR();
                    GUIBUTTON(commands.OpenDXF);
                    GUIBUTTON(commands.SaveDXF);
                    SEPERATOR();
                    SEPERATOR();
                    SEPERATOR();
                    if (GUIBUTTON(commands.PowerFillet)) {
                        set_state_Draw_command(PowerFillet);
                        set_state_Snap_command(None);
                        set_state_Mesh_command(None);
                    }
                }

                { // Mesh
                    if (GUIBUTTON(commands.ExtrudeAdd)) {
                        preview->extrude_in_length = 0; // FORNOW
                        preview->extrude_out_length = 0; // FORNOW
                    }
                    if (GUIBUTTON(commands.ExtrudeCut)) {
                        preview->extrude_in_length = 0; // FORNOW
                        preview->extrude_out_length = 0; // FORNOW
                    }
                    SEPERATOR();
                    if (GUIBUTTON(commands.RevolveAdd)) {
                        preview->revolve_in_angle = 0; // FORNOW
                        preview->revolve_out_angle = 0; // FORNOW
                    }
                    if (GUIBUTTON(commands.RevolveCut)) {
                        preview->revolve_in_angle = 0; // FORNOW
                        preview->revolve_out_angle = 0; // FORNOW
                    }
                    SEPERATOR();
                    if (GUIBUTTON(commands.CyclePlane)) {
                        // TODO: 'Y' remembers last terminal choice of plane for next time
                        result.checkpoint_me = true;
                        other.time_since_plane_selected = 0.0f;

                        // already one of the three primary planes
                        if ((feature_plane->is_active) && ARE_EQUAL(feature_plane->signed_distance_to_world_origin, 0.0f) && ARE_EQUAL(squaredNorm(feature_plane->normal), 1.0f) && ARE_EQUAL(maxComponent(feature_plane->normal), 1.0f)) {
                            feature_plane->normal = { feature_plane->normal[2], feature_plane->normal[0], feature_plane->normal[1] };
                        } else {
                            feature_plane->is_active = true;
                            feature_plane->signed_distance_to_world_origin = 0.0f;
                            feature_plane->normal = { 0.0f, 1.0f, 0.0f };
                        }
                    }
                    if (GUIBUTTON(commands.NudgePlane)) {
                        if (feature_plane->is_active) {
                            preview->feature_plane_offset = 0.0f; // FORNOW
                        } else {
                            messagef(pallete.orange, "NudgePlane: no feature plane selected");
                            set_state_Mesh_command(None); // FORNOW
                        }
                    }
                    if (GUIBUTTON(commands.HidePlane)) {
                        if (feature_plane->is_active) other.time_since_plane_deselected = 0.0f;
                        feature_plane->is_active = false;
                    }
                    if (GUIBUTTON(commands.MirrorPlaneX)) {
                        other.mirror_3D_plane_X = !other.mirror_3D_plane_X;
                    }
                    if (GUIBUTTON(commands.MirrorPlaneY)) {
                        other.mirror_3D_plane_Y = !other.mirror_3D_plane_Y;
                    }
                    SEPERATOR();
                    if (GUIBUTTON(commands.ClearMesh)) {
                        result.checkpoint_me = true;
                        result.snapshot_me = true;
                        mesh_free_AND_zero(mesh);
                        *feature_plane = {};
                        messagef(pallete.light_gray, "ClearMesh");
                    }
                    if (GUIBUTTON(commands.ZoomMesh)) {
                        init_camera_mesh();
                    }
                    if (GUIBUTTON(commands.ZoomPlane)) {
                        init_camera_mesh();
                        real x = feature_plane->normal.x;
                        real y = feature_plane->normal.y;
                        real z = feature_plane->normal.z;
                        camera_mesh->euler_angles.y = ATAN2({z, x});
                        camera_mesh->euler_angles.x = -ATAN2({norm(V2(z, x)), y});
                    }
                    SEPERATOR();
                    if (GUIBUTTON(commands.Measure3D)) {
                        // Copied from Hide Plane, should maybe be a function
                        if (feature_plane->is_active) other.time_since_plane_deselected = 0.0f;
                        feature_plane->is_active = false;
                    }
                    SEPERATOR();
                    SEPERATOR();
                    GUIBUTTON(commands.OpenSTL);
                    GUIBUTTON(commands.SaveSTL);
                }



                ////////////////////////////////////////////////////////////////////////////////

                #ifndef SHIP
                { // Secret Commands
                    if (GUIBUTTON(commands.PREVIOUS_HOT_KEY_2D)) {
                        set_state_Draw_command(None); // FORNOW: patching space space doing Circle Center
                        return _standard_event_process_NOTE_RECURSIVE(state.space_bar_event);
                    }
                    // if (GUIBUTTON(commands.OrthoCamera)) {
                    //     other.camera_mesh.angle_of_view = CAMERA_3D_PERSPECTIVE_ANGLE_OF_VIEW - other.camera_mesh.angle_of_view;
                    // }

                    if (GUIBUTTON(commands.DivideNearest)) {
                        set_state_Draw_command(DivideNearest);
                        set_state_Snap_command(None);

                    }



                    if (GUIBUTTON(commands.TOGGLE_GRID)) {
                        other.hide_grid = !other.hide_grid;

                    }

                    if (GUIBUTTON(commands.PRINT_HISTORY)) {
                        history_printf_script();

                    }

                    if (GUIBUTTON(commands.TOGGLE_EVENT_STACK)) { 
                        other.show_event_stack = !other.show_event_stack;
                    }




                    if (GUIBUTTON(commands.PREVIOUS_HOT_KEY_3D)) {

                        return _standard_event_process_NOTE_RECURSIVE(state.shift_space_bar_event);
                    }






                    if (GUIBUTTON(commands.HELP_MENU)) {
                        other.show_help = !other.show_help;

                    }


                    if (GUIBUTTON(commands.TOGGLE_LIGHT_MODE)) { // FORNOW
                        {
                            vec3 tmp = pallete.light_gray;
                            pallete.light_gray = pallete.dark_gray;
                            pallete.dark_gray = tmp;
                        }
                        {
                            vec3 tmp = pallete.white;
                            pallete.white = pallete.black;
                            pallete.black = tmp;
                        }
                        {
                            vec3 tmp = pallete.yellow;
                            pallete.yellow = pallete.dark_yellow;
                            pallete.dark_yellow = tmp;
                        }

                    }

                    if (GUIBUTTON(commands.TOGGLE_BUTTONS)) { // FORNOW
                        other.hide_toolbox = !other.hide_toolbox;
                    }
                }
                #endif

                if (key_event->subtype == KeyEventSubtype::Hotkey) { // (not button)
                    if (!hotkey_consumed_by_GUIBUTTON) {
                        result.record_me = false;
                    }
                    if (1
                            && !hotkey_consumed_by_GUIBUTTON
                            && (key_event->key != DUMMY_HOTKEY)
                            && (key_event->key != 0)
                            && (key_event->key != 0)
                       ) {
                        messagef(pallete.orange, "Hotkey: %s not recognized", key_event_get_cstring_for_printf_NOTE_ONLY_USE_INLINE(key_event), key_event->control, key_event->shift, key_event->alt, key_event->key);
                    }
                }


            }
            bool changed_click_mode = (!command_equals(prev_Draw_command, state.Draw_command));
            bool changed_enter_mode = (!command_equals(prev_Mesh_command, state.Mesh_command));
            if (changed_click_mode && click_mode_SPACE_BAR_REPEAT_ELIGIBLE()) state.space_bar_event = event;
            if (changed_enter_mode && enter_mode_SHIFT_SPACE_BAR_REPEAT_ELIGIBLE()) state.shift_space_bar_event = event;
        } else { ASSERT(key_event->subtype == KeyEventSubtype::Popup);
            // NOTE: this case has been moved into POPUP();
            result.record_me = true; // FORNOW
        }
    } else if (event.type == EventType::Mouse) {
        MouseEvent *mouse_event = &event.mouse_event;
        if (mouse_event->subtype == MouseEventSubtype::Drawing) {
            MouseEventDrawing *mouse_event_drawing = &mouse_event->mouse_event_drawing;

            result.record_me = true;
            if (state_Draw_command_is_(Measure)) result.record_me = false;
            if (mouse_event->mouse_held) result.record_me = false;

            MagicSnapResult snap_result = mouse_event_drawing->snap_result;
            vec2 *mouse = &snap_result.mouse_position;

            // TODO: commands.cpp flag
            if (snap_result.snapped && ( 
                        (state_Draw_command_is_(Box))
                        || (state_Draw_command_is_(CenterBox))
                        || (state_Draw_command_is_(CenterLine))
                        || (state_Draw_command_is_(Circle))
                        || (state_Draw_command_is_(Line))
                        || (state_Draw_command_is_(Polygon))
                        || (state_Draw_command_is_(DiamCircle)))) {
                ASSERT(snap_result.entity_index_snapped_to >= 0);
                ASSERT(snap_result.entity_index_snapped_to < drawing->entities.length);
                cookbook.attempt_divide_entity_at_point(snap_result.entity_index_snapped_to, *mouse);
                other.snap_divide_dot = *mouse;
                other.size_snap_divide_dot = 7.0f;
            }


            // fornow window wonky case
            if (_non_WINDOW__SELECT_DESELECT___OR___SET_COLOR()) { // NOTES: includes sc and qc
                result.record_me = false;
                DXFFindClosestEntityResult dxf_find_closest_entity_result = dxf_find_closest_entity(&drawing->entities, mouse_event_drawing->snap_result.mouse_position);
                if (dxf_find_closest_entity_result.success) {
                    Entity *hot_entity = dxf_find_closest_entity_result.closest_entity;
                    if (!state_Xsel_command_is_(Connected) && !state_Colo_command_is_(OfSelection)) {
                        if (SELECT_OR_DESELECT()) {
                            cookbook.entity_set_is_selected(hot_entity, value_to_write_to_selection_mask);
                        } else {
                            bool found = false;
                            for_(digit, 10) {
                                if (command_equals(state.Colo_command, commands_Color[digit])) {
                                    cookbook.entity_set_color(hot_entity, ColorCode(digit));
                                    found = true;
                                    break;
                                }
                            }
                            ASSERT(found);
                        }
                    } else {
                        #define GRID_CELL_WIDTH 0.003f

                        auto scalar_bucket = [&](real a) -> real {
                            real ret = roundf(a / GRID_CELL_WIDTH) * GRID_CELL_WIDTH;
                            return ret == -0 ? 0 : ret; // what a fun bug
                        };

                        auto make_key = [&](vec2 p) -> vec2 {
                            return { scalar_bucket(p.x), scalar_bucket(p.y) };
                        };

                        auto nudge_key = [&](vec2 key, int dx, int dy) -> vec2 {
                            return make_key(V2(key.x + dx * GRID_CELL_WIDTH, key.y + dy * GRID_CELL_WIDTH));
                        };

                        struct GridPointSlot {
                            bool populated;
                            int entity_index;
                            bool end_NOT_start;
                        };

                        struct GridCell {
                            GridPointSlot slots[5];
                        };

                        Map<vec2, GridCell> grid; { // TODO: build grid
                            grid = {};

                            auto push_into_grid_unless_cell_full__make_cell_if_none_exists = [&](vec2 p, uint entity_index, bool end_NOT_start) {
                                vec2 key = make_key(p);
                                GridCell *cell = _map_get_pointer(&grid, key);
                                if (cell == NULL) {
                                    map_put(&grid, key, {});
                                    cell = _map_get_pointer(&grid, key);
                                }
                                for_(i, ARRAY_LENGTH(cell->slots)) {
                                    GridPointSlot *slot = &cell->slots[i];
                                    if (slot->populated) continue;
                                    slot->populated = true;
                                    slot->entity_index = entity_index;
                                    slot->end_NOT_start = end_NOT_start;
                                    // printf("%f %f [%d]\n", key.x, key.y, i);
                                    break;
                                }
                            };

                            for_(entity_index, drawing->entities.length) {
                                Entity *entity = &drawing->entities.array[entity_index];

                                vec2 start;
                                vec2 end;
                                bool poosh = false;
                                if (entity->type == EntityType::Circle) {
                                    CircleEntity *circle = &entity->circle;
                                    if (circle->has_pseudo_point) {
                                        poosh = true;
                                        start = end = circle->get_pseudo_point();
                                    }
                                } else {
                                    poosh = true;
                                    entity_get_start_and_end_points(entity, &start, &end);
                                }
                                if (poosh) {
                                    push_into_grid_unless_cell_full__make_cell_if_none_exists(start, entity_index, false);
                                    push_into_grid_unless_cell_full__make_cell_if_none_exists(end, entity_index, true);
                                }
                            }
                        }

                        bool *edge_marked = (bool *) calloc(drawing->entities.length, sizeof(bool));

                        ////////////////////////////////////////////////////////////////////////////////
                        // NOTE: We are now done adding to the grid, so we can now operate directly on GridCell *'s
                        //       We will use _map_get_pointer(...)
                        ////////////////////////////////////////////////////////////////////////////////


                        auto get_key = [&](GridPointSlot *point, bool other_endpoint) {
                            bool end_NOT_start; {
                                end_NOT_start = point->end_NOT_start;
                                if (other_endpoint) end_NOT_start = !end_NOT_start;
                            }
                            vec2 p; {
                                Entity *entity = &drawing->entities.array[point->entity_index];
                                if (entity->type == EntityType::Circle) {
                                    CircleEntity *circle = &entity->circle;
                                    ASSERT(circle->has_pseudo_point);
                                    p = circle->get_pseudo_point();
                                } else {
                                    if (end_NOT_start) {
                                        p = entity_get_end_point(entity);
                                    } else {
                                        p = entity_get_start_point(entity);
                                    }
                                }
                            }
                            return make_key(p);
                        };

                        auto get_any_point_not_part_of_an_marked_entity = [&](vec2 key) -> GridPointSlot * {
                            GridCell *cell = _map_get_pointer(&grid, key);
                            if (!cell) return NULL;

                            for_(i, ARRAY_LENGTH(cell->slots)) {
                                GridPointSlot *slot = &cell->slots[i];
                                if (!slot->populated) continue;
                                if (edge_marked[slot->entity_index]) continue;
                                return slot;
                            }
                            return NULL;
                        };


                        uint hot_entity_index = uint(hot_entity - drawing->entities.array);

                        // NOTE: we will mark the hot entity, and then shoot off from both its endpoints
                        edge_marked[hot_entity_index] = true;
                        cookbook.entity_set_is_selected(&drawing->entities.array[hot_entity_index], value_to_write_to_selection_mask);

                        Entity *entity = &drawing->entities.array[hot_entity_index]; // FORNOW: this is a scary name to give (just doing it out of laziness atm Jim Sep 27 2024)

                        // this should be moved earlier in the code especially once this crap is turned into a function/its own file
                        bool special_case_circle_no_pseudo_point = ((entity->type == EntityType::Circle) && (!entity->circle.has_pseudo_point));
                        if (special_case_circle_no_pseudo_point) {
                            entity->is_selected = value_to_write_to_selection_mask;
                        } else {
                            for_(pass, 2) {
                                vec2 seed; {
                                    vec2 p;
                                    if (entity->type == EntityType::Circle) {
                                        CircleEntity *circle = &entity->circle;
                                        ASSERT(circle->has_pseudo_point);
                                        p = circle->get_pseudo_point();
                                    } else {
                                        if (pass == 0) {
                                            p = entity_get_start_point(entity);
                                        } else {
                                            p = entity_get_end_point(entity);
                                        }
                                    }
                                    seed = make_key(p);
                                }

                                Queue<vec2> queue = {};
                                queue_enqueue(&queue, seed);

                                while (queue.length) {
                                    seed = queue_dequeue(&queue);
                                    for (int dx = -1; dx <= 1; ++dx) {
                                        for (int dy = -1; dy <= 1; ++dy) {
                                            while (1) {
                                                GridPointSlot *tmp = get_any_point_not_part_of_an_marked_entity(nudge_key(seed, dx, dy));

                                                if (!tmp) break;

                                                cookbook.entity_set_is_selected(&drawing->entities.array[tmp->entity_index], value_to_write_to_selection_mask);
                                                GridPointSlot *nullCheck = get_any_point_not_part_of_an_marked_entity(get_key(tmp, true));
                                                if (nullCheck) queue_enqueue(&queue, get_key(nullCheck, false)); // get other end);
                                                edge_marked[tmp->entity_index] = true;
                                            } 
                                        }
                                    }
                                }

                                queue_free_AND_zero(&queue);
                            }
                        }




                        map_free_and_zero(&grid);
                        free(edge_marked);

                        /*uint loop_index = dxf_pick_loops.loop_index_from_entity_index[hot_entity_index];
                          DXFEntityIndexAndFlipFlag *loop = dxf_pick_loops.loops[loop_index];
                          uint num_entities = dxf_pick_loops.num_entities_in_loops[loop_index];
                          for (DXFEntityIndexAndFlipFlag *entity_index_and_flip_flag = loop; entity_index_and_flip_flag < &loop[num_entities]; ++entity_index_and_flip_flag) {
                          cookbook.entity_set_is_selected(&drawing->entities[entity_index_and_flip_flag->entity_index], value_to_write_to_selection_mask);
                          }*/
                    }
                }
            } else if (!mouse_event->mouse_held) {
                if ((state.Draw_command.flags | state.Xsel_command.flags) & TWO_CLICK) { // FORNOW
                    if (!two_click_command->awaiting_second_click) {
                        DXFFindClosestEntityResult find_nearest_result = dxf_find_closest_entity(&drawing->entities, mouse_event_drawing->snap_result.mouse_position);
                        bool first_click_accepted; {
                            if (!first_click_must_acquire_entity()) {
                                first_click_accepted = true;
                            } else {
                                first_click_accepted = find_nearest_result.success;
                            }
                        }
                        if (first_click_accepted) {
                            two_click_command->awaiting_second_click = true;
                            two_click_command->first_click = mouse_event_drawing->snap_result.mouse_position;
                            if (!two_click_command->tangent_first_click) { // ???
                                two_click_command->entity_closest_to_first_click = find_nearest_result.closest_entity;
                            }
                            set_state_Snap_command(None);
                            if (!other._please_suppress_drawing_popup_popup) { // bump bumps cursor bump cursor bumps
                                if (state_Draw_command_is_(Rotate)) {
                                    double xpos, ypos;
                                    glfwGetCursorPos(glfw_window, &xpos, &ypos);
                                    real x_new = real(xpos) + 64.0f;
                                    real y_new = real(ypos);
                                    glfwSetCursorPos(glfw_window, x_new, y_new);
                                    callback_cursor_position(glfw_window, x_new, y_new);
                                }
                                if (state_Draw_command_is_(SetAxis)) {
                                    double xpos, ypos;
                                    glfwGetCursorPos(glfw_window, &xpos, &ypos);
                                    real theta = (PI / 2) + drawing->axis_angle_from_y;
                                    real r = 64.0f;
                                    real x_new = real(xpos) + r * COS(theta);
                                    real y_new = real(ypos) - r * SIN(theta);
                                    glfwSetCursorPos(glfw_window, x_new, y_new);
                                    callback_cursor_position(glfw_window, x_new, y_new);
                                }
                            }

                            // FORNOW FORNOW
                            preview->popup_second_click = two_click_command->first_click;
                            preview->xy_xy = two_click_command->first_click;
                            preview->color_mouse = pallete.black;
                        }
                    } else { // (two_click_command->awaiting_second_click)
                        vec2 first_click = two_click_command->first_click;
                        vec2 second_click = *mouse;
                        vec2 click_vector = (second_click - first_click);
                        vec2 average_click = AVG(first_click, second_click);
                        real click_theta = angle_from_0_TAU(first_click, second_click);
                        real clicks_are_same = IS_ZERO(click_vector);
                        real length_click_vector = norm(click_vector);

                        if (0) {
                        } else if (state_Draw_command_is_(SetAxis)) {
                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);
                            drawing->axis_base_point = first_click;
                            drawing->axis_angle_from_y = (-PI / 2) + click_theta;
                        } else if (state_Draw_command_is_(Box)) {
                            if (IS_ZERO(ABS(first_click.x - second_click.x))) {
                                messagef(pallete.orange, "Box: must have non-zero width ");
                            } else if (IS_ZERO(ABS(first_click.y - second_click.y))) {
                                messagef(pallete.orange, "Box: must have non-zero height");
                            } else {
                                result.checkpoint_me = true;
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);
                                vec2 other_corner_A = { first_click.x, second_click.y };
                                vec2 other_corner_B = { second_click.x, first_click.y };
                                cookbook.buffer_add_line(first_click,  other_corner_A);
                                cookbook.buffer_add_line(first_click,  other_corner_B);
                                cookbook.buffer_add_line(second_click, other_corner_A);
                                cookbook.buffer_add_line(second_click, other_corner_B);
                            }
                        } else if (state_Draw_command_is_(CenterBox)) {
                            if (IS_ZERO(ABS(first_click.x - second_click.x))) {
                                messagef(pallete.orange, "Box: must have non-zero width ");
                            } else if (IS_ZERO(ABS(first_click.y - second_click.y))) {
                                messagef(pallete.orange, "Box: must have non-zero height");
                            } else {
                                result.checkpoint_me = true;
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);
                                vec2 one_corner = second_click;
                                vec2 center = first_click;
                                real other_y = 2 * center.y - one_corner.y;
                                real other_x = 2 * center.x - one_corner.x;
                                cookbook.buffer_add_line(one_corner, V2(one_corner.x, other_y));
                                cookbook.buffer_add_line(V2(one_corner.x, other_y),  V2(other_x, other_y));
                                cookbook.buffer_add_line(V2(other_x, other_y), V2(other_x, one_corner.y));
                                cookbook.buffer_add_line(V2(other_x, one_corner.y), one_corner);
                            }
                        } else if (state_Draw_command_is_(CenterLine)) {
                            if (clicks_are_same) {
                                messagef(pallete.orange, "Line: must have non-zero length");
                            } else {
                                result.checkpoint_me = true;
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);
                                vec2 mirrored_click = first_click + (first_click - second_click);
                                cookbook.buffer_add_line(mirrored_click, second_click);
                            }
                        } else if (state_Draw_command_is_(Fillet)) {
                            result.checkpoint_me = true;

                            set_state_Snap_command(None);
                            DXFFindClosestEntityResult _F = dxf_find_closest_entity(&drawing->entities, second_click);
                            if (_F.success) {
                                Entity *E = two_click_command->entity_closest_to_first_click;
                                Entity *F = _F.closest_entity;
                                cookbook.attempt_fillet_ENTITIES_GET_DELETED_AT_END_OF_FRAME(E, F, second_click, popup->fillet_radius);
                                two_click_command->awaiting_second_click = false;
                            }
                        } else if (state_Draw_command_is_(DogEar)) {
                            result.checkpoint_me = true;

                            set_state_Snap_command(None);
                            DXFFindClosestEntityResult _F = dxf_find_closest_entity(&drawing->entities, second_click);
                            if (_F.success) {
                                Entity *E = two_click_command->entity_closest_to_first_click;
                                Entity *F = _F.closest_entity;
                                cookbook.attempt_dogear(E, F, average_click, popup->dogear_radius);
                                two_click_command->awaiting_second_click = false;
                            }
                        } else if (state_Draw_command_is_(Circle)) {
                            if (clicks_are_same) {
                                messagef(pallete.orange, "Circle: must have non-zero diameter");
                            } else {
                                // messagef(pallete.light_gray, "Circle");
                                result.checkpoint_me = true;
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);
                                real r = length_click_vector;

                                #if 0
                                real theta_a_in_degrees = DEG(click_theta);
                                real theta_b_in_degrees = theta_a_in_degrees + 180.0f;
                                cookbook.buffer_add_arc(first_click, r, theta_a_in_degrees, theta_b_in_degrees);
                                cookbook.buffer_add_arc(first_click, r, theta_b_in_degrees, theta_a_in_degrees);
                                #else
                                cookbook.buffer_add_circle(first_click, r, false, {});
                                #endif
                            }
                        } else if (state_Draw_command_is_(DiamCircle)) {
                            if (clicks_are_same) {
                                messagef(pallete.orange, "TwoEdgeCircle: must have non-zero diameter");
                            } else {
                                result.checkpoint_me = true;
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);
                                vec2 center = average_click;
                                real theta_a_in_degrees = DEG(ATAN2(second_click - center));
                                real theta_b_in_degrees = theta_a_in_degrees + 180.0f;
                                real radius = length_click_vector / 2;
                                cookbook.buffer_add_arc(center, radius, theta_a_in_degrees, theta_b_in_degrees);
                                cookbook.buffer_add_arc(center, radius, theta_b_in_degrees, theta_a_in_degrees);
                                // messagef(pallete.light_gray, "Circle");
                            }
                        } else if (state_Draw_command_is_(Divide2)) { // TODO: make sure no 0 length shenanigans
                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);

                            Entity *closest_entity_one = two_click_command->entity_closest_to_first_click; 
                            DXFFindClosestEntityResult closest_result_two = dxf_find_closest_entity(&drawing->entities, second_click);
                            if (closest_result_two.success) {
                                Entity *closest_entity_two = closest_result_two.closest_entity;
                                if (closest_entity_one == closest_entity_two) {
                                    messagef(pallete.orange, "TwoClickDivide: clicked same entity twice");
                                } else {
                                    if (closest_entity_one->type == EntityType::Line && closest_entity_two->type == EntityType::Line) {
                                        LineEntity segment_one = closest_entity_one->line;
                                        LineEntity segment_two = closest_entity_two->line;
                                        vec2 a = segment_one.start;
                                        vec2 b = segment_one.end;
                                        vec2 c = segment_two.start;
                                        vec2 d = segment_two.end;
                                        LineLineXResult X_result = line_line_intersection(a, b, c, d);
                                        bool neither_line_extension_hits_the_other_segment = ((!X_result.point_is_on_segment_ab) && (!X_result.point_is_on_segment_cd));
                                        if (neither_line_extension_hits_the_other_segment) {
                                            messagef(pallete.orange, "TwoClickDivide: no intersection found");
                                        } else {
                                            if (X_result.point_is_on_segment_ab) {
                                                cookbook.buffer_add_line(X_result.point, a, false, closest_entity_one->color_code);
                                                cookbook.buffer_add_line(X_result.point, b, false, closest_entity_one->color_code);
                                                cookbook.buffer_delete_entity(closest_entity_one);
                                            }
                                            if (X_result.point_is_on_segment_cd) {
                                                cookbook.buffer_add_line(X_result.point, c, false, closest_entity_two->color_code);
                                                cookbook.buffer_add_line(X_result.point, d, false, closest_entity_two->color_code);
                                                cookbook.buffer_delete_entity(closest_entity_two);
                                            } 
                                        }
                                    } else if (closest_entity_one->type == EntityType::Arc && closest_entity_two->type == EntityType::Arc){
                                        // math for this is here https://paulbourke.net/geometry/circlesphere/

                                        ArcEntity arcA = closest_entity_one->arc;
                                        ArcEntity arcB = closest_entity_two->arc;

                                        ArcArcXResult arc_x_arc_result = arc_arc_intersection(&arcA, &arcB);

                                        bool p1Works = arc_x_arc_result.point1_is_on_arc_a || arc_x_arc_result.point1_is_on_arc_b; 
                                        bool p2Works = arc_x_arc_result.point2_is_on_arc_a || arc_x_arc_result.point2_is_on_arc_b; 
                                        bool cut_arc_a = false;
                                        bool cut_arc_b = false;
                                        real theta_a = 0;
                                        real theta_b = 0;

                                        if (p1Works) {
                                            real click_to_p1 = distance(arc_x_arc_result.point1, second_click);
                                            real click_to_p2 = distance(arc_x_arc_result.point2, second_click);
                                            if (p2Works && click_to_p2 < click_to_p1) { 
                                                theta_a = arc_x_arc_result.theta_2a;
                                                theta_b = arc_x_arc_result.theta_2b;
                                                cut_arc_a = arc_x_arc_result.point2_is_on_arc_a;
                                                cut_arc_b = arc_x_arc_result.point2_is_on_arc_b;
                                            } else {
                                                theta_a = arc_x_arc_result.theta_1a;
                                                theta_b = arc_x_arc_result.theta_1b;
                                                cut_arc_a = arc_x_arc_result.point1_is_on_arc_a;
                                                cut_arc_b = arc_x_arc_result.point1_is_on_arc_b;
                                            }
                                        } else if (p2Works) {
                                            theta_a = arc_x_arc_result.theta_2a;
                                            theta_b = arc_x_arc_result.theta_2b;
                                            cut_arc_a = arc_x_arc_result.point2_is_on_arc_a;
                                            cut_arc_b = arc_x_arc_result.point2_is_on_arc_b;
                                        }
                                        if (cut_arc_a) {
                                            cookbook.buffer_add_arc(arcA.center, arcA.radius, arcA.start_angle_in_degrees, theta_a, false, closest_entity_one->color_code);
                                            cookbook.buffer_add_arc(arcA.center, arcA.radius, theta_a, arcA.end_angle_in_degrees, false, closest_entity_one->color_code);
                                            cookbook.buffer_delete_entity(closest_entity_one);
                                        }
                                        if (cut_arc_b) {
                                            cookbook.buffer_add_arc(arcB.center, arcB.radius, arcB.start_angle_in_degrees, theta_b, false, closest_entity_two->color_code);
                                            cookbook.buffer_add_arc(arcB.center, arcB.radius, theta_b, arcB.end_angle_in_degrees, false, closest_entity_two->color_code);
                                            cookbook.buffer_delete_entity(closest_entity_two);
                                        }
                                        if (!cut_arc_a && !cut_arc_b) {
                                            messagef(pallete.orange, "TwoClickDivide: no intersection found");
                                        }
                                    } else { ASSERT((closest_entity_one->type == EntityType::Line) && (closest_entity_two->type == EntityType::Arc)); // kinda nasty but only way 
                                                                                                                                                      //       || (closest_entity_two->type == EntityType::Arc && closest_entity_two->type == EntityType::Line));
                                        Entity *entity_arc;
                                        Entity *entity_line;
                                        if (closest_entity_one->type == EntityType::Arc) {
                                            entity_arc = closest_entity_one;
                                            entity_line = closest_entity_two;
                                        } else {
                                            entity_arc = closest_entity_two;
                                            entity_line = closest_entity_one;
                                        }
                                        ArcEntity *arc = &entity_arc->arc;
                                        LineEntity *line = &entity_line->line;

                                        LineArcXResult line_x_arc_result = line_arc_intersection(line, arc);
                                        bool p1Works = line_x_arc_result.point1_is_on_arc || line_x_arc_result.point1_is_on_line_segment;
                                        bool p2Works = line_x_arc_result.point2_is_on_arc || line_x_arc_result.point2_is_on_line_segment;

                                        vec2 intersect = {};
                                        real theta = 0;
                                        bool cutLine = false;
                                        bool cutArc = false;

                                        if (p1Works) {
                                            real click_to_p1 = distance(line_x_arc_result.point1, second_click);
                                            real click_to_p2 = distance(line_x_arc_result.point2, second_click);
                                            if (p2Works && click_to_p2 < click_to_p1) { 
                                                intersect = line_x_arc_result.point2;
                                                theta = line_x_arc_result.theta_2;
                                                cutLine = line_x_arc_result.point2_is_on_line_segment;
                                                cutArc = line_x_arc_result.point2_is_on_arc;
                                            } else {
                                                intersect = line_x_arc_result.point1;
                                                theta = line_x_arc_result.theta_1;
                                                cutLine = line_x_arc_result.point1_is_on_line_segment;
                                                cutArc = line_x_arc_result.point1_is_on_arc;
                                            }
                                        } else if (p2Works) {
                                            intersect = line_x_arc_result.point2;
                                            theta = line_x_arc_result.theta_2;
                                            cutLine = line_x_arc_result.point2_is_on_line_segment;
                                            cutArc = line_x_arc_result.point2_is_on_arc;
                                        }

                                        if (p1Works || p2Works) {
                                            if (cutLine) {
                                                cookbook.buffer_add_line(intersect, line->start, false, entity_line->color_code);
                                                cookbook.buffer_add_line(intersect, line->end, false, entity_line->color_code);
                                                cookbook.buffer_delete_entity(entity_line);
                                            }
                                            if (cutArc) {
                                                cookbook.buffer_add_arc(arc->center, arc->radius, arc->start_angle_in_degrees, theta, false, entity_arc->color_code);
                                                cookbook.buffer_add_arc(arc->center, arc->radius, theta, arc->end_angle_in_degrees, false, entity_arc->color_code);
                                                cookbook.buffer_delete_entity(entity_arc);
                                            }
                                        }
                                        if (!cutArc && !cutLine) {
                                            messagef(pallete.orange, "TwoClickDivide: no intersection found");
                                        }
                                    }
                                }
                            }
                        } else if (state_Draw_command_is_(Line)) {
                            if (clicks_are_same) {
                                messagef(pallete.orange, "Line: must have non-zero length");
                            } else {
                                result.checkpoint_me = true;
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);
                                cookbook.buffer_add_line(first_click, second_click);
                            }
                        } else if (state_Draw_command_is_(Measure)) {
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);
                            real angle = DEG(click_theta);
                            real length = length_click_vector;
                            messagef(pallete.cyan, "Angle is %gdeg.", angle);
                            messagef(pallete.cyan, "Length is %gmm.", length);

                            messagef(pallete.yellow, "EXPERIMENTAL: Measure copies into field.");
                            _POPUP_MEASURE_HOOK(length);
                        } else if (state_Draw_command_is_(Mirror2)) {

                            // TODO: entity_mirrored

                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);

                            real theta = ATAN2(click_vector);
                            real theta_in_degrees = DEG(theta);

                            auto Q = [theta, first_click](vec2 p) {
                                p -= first_click;
                                p = rotated(p, -theta);
                                p = cwiseProduct(V2(1, -1), p);
                                p = rotated(p, theta);
                                p += first_click;
                                return p;
                            };

                            auto R = [theta_in_degrees](real angle_in_degrees) {
                                return -(angle_in_degrees - theta_in_degrees) + theta_in_degrees;
                            };

                            _for_each_selected_entity_ {
                                if (entity->type == EntityType::Line) {
                                    LineEntity *line = &entity->line;
                                    cookbook.buffer_add_line(Q(line->start), Q(line->end), true, entity->color_code);
                                } else { ASSERT(entity->type == EntityType::Arc);
                                    ArcEntity *arc = &entity->arc;
                                    cookbook.buffer_add_arc(Q(arc->center), arc->radius, R(arc->end_angle_in_degrees), R(arc->start_angle_in_degrees), true, entity->color_code);
                                }
                                entity->is_selected = false;
                            }
                        } else if (state_Draw_command_is_(Rotate)) {
                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);
                            _for_each_selected_entity_ {
                                *entity = entity_rotated(entity, first_click, click_theta);
                            }
                        } else if (state_Draw_command_is_(RCopy)) {
                            if (popup->rotate_copy_num_total_copies < 2) {
                                messagef(pallete.orange, "RCopy: must have at least 2 total copies");
                            } else {
                                result.checkpoint_me = true;
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);

                                real dtheta_deg = popup->rotate_copy_angle;
                                if (IS_ZERO(dtheta_deg)) dtheta_deg = 180.0f;
                                real dtheta = RAD(dtheta_deg);

                                _for_each_selected_entity_ {
                                    for_(j, popup->rotate_copy_num_total_copies - 1) {
                                        real theta = (j + 1) * dtheta;

                                        cookbook._buffer_add_entity(entity_rotated(entity, first_click, theta));
                                    }
                                }
                            }
                        } else if (state_Draw_command_is_(Drag)) {
                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);


                            #define GRID_CELL_WIDTH 0.003f

                            auto scalar_bucket = [&](real a) -> real {
                                real ret = roundf(a / GRID_CELL_WIDTH) * GRID_CELL_WIDTH;
                                return ret == -0 ? 0 : ret; // what a fun bug
                            };

                            auto make_key = [&](vec2 p) -> vec2 {
                                return { scalar_bucket(p.x), scalar_bucket(p.y) };
                            };

                            auto nudge_key = [&](vec2 key, int dx, int dy) -> vec2 {
                                return make_key(V2(key.x + dx * GRID_CELL_WIDTH, key.y + dy * GRID_CELL_WIDTH));
                            };

                            struct GridPointSlot {
                                bool populated;
                                int entity_index;
                                bool end_NOT_start;
                            };

                            struct GridCell {
                                GridPointSlot slots[5];
                            };

                            Map<vec2, GridCell> grid; { // TODO: build grid
                                grid = {};

                                auto push_into_grid_unless_cell_full__make_cell_if_none_exists = [&](vec2 p, uint entity_index, bool end_NOT_start) {
                                    vec2 key = make_key(p);
                                    GridCell *cell = _map_get_pointer(&grid, key);
                                    if (cell == NULL) {
                                        map_put(&grid, key, {});
                                        cell = _map_get_pointer(&grid, key);
                                    }
                                    for_(i, ARRAY_LENGTH(cell->slots)) {
                                        GridPointSlot *slot = &cell->slots[i];
                                        if (slot->populated) continue;
                                        slot->populated = true;
                                        slot->entity_index = entity_index;
                                        slot->end_NOT_start = end_NOT_start;
                                        // printf("%f %f [%d]\n", key.x, key.y, i);
                                        break;
                                    }
                                };

                                for_(entity_index, drawing->entities.length) {
                                    Entity *entity = &drawing->entities.array[entity_index];

                                    vec2 start;
                                    vec2 end;

                                    if (entity_length(entity) < 0.05f) continue; // TODO: TODO: VERY SCARY 0 LENGTH ENTITIES 
                                                                                 //
                                    entity_get_start_and_end_points(entity, &start, &end);
                                    push_into_grid_unless_cell_full__make_cell_if_none_exists(start, entity_index, false);
                                    push_into_grid_unless_cell_full__make_cell_if_none_exists(end, entity_index, true);
                                }
                            }

                            struct EndpointMark {
                                bool start_marked;
                                bool end_marked;
                            };
                            EndpointMark *endpoint_marks = (EndpointMark *)calloc(drawing->entities.length, sizeof(EndpointMark));

                            char *to_move = (char *) calloc(drawing->entities.length, sizeof(char));

                            ////////////////////////////////////////////////////////////////////////////////
                            // NOTE: We are now done adding to the grid, so we can now operate directly on GridCell *'s
                            //       We will use _map_get_pointer(...)
                            ////////////////////////////////////////////////////////////////////////////////


                            auto get_key = [&](GridPointSlot *point, bool other_endpoint) {
                                bool end_NOT_start; {
                                    end_NOT_start = point->end_NOT_start;
                                    if (other_endpoint) end_NOT_start = !end_NOT_start;
                                }
                                vec2 p; {
                                    Entity *entity = &drawing->entities.array[point->entity_index];
                                    if (end_NOT_start) {
                                        p = entity_get_end_point(entity);
                                    } else {
                                        p = entity_get_start_point(entity);
                                    }
                                }
                                return make_key(p);
                            };

                            auto get_any_point_not_part_of_an_marked_entity = [&](vec2 key) -> GridPointSlot * {
                                GridCell *cell = _map_get_pointer(&grid, key);
                                if (!cell) return NULL;

                                for_(i, ARRAY_LENGTH(cell->slots)) {
                                    GridPointSlot *slot = &cell->slots[i];
                                    if (!slot->populated) continue;
                                    EndpointMark *mark = &endpoint_marks[slot->entity_index];
                                    if (slot->end_NOT_start) {
                                        if (mark->end_marked) continue;
                                    } else {
                                        if (mark->start_marked) continue;
                                    }
                                    return slot;
                                }
                                return NULL;
                            };

                            typedef struct EntVecMapping {
                                vec2 p;
                                int parentIndex;
                                bool start;
                            } EntVecMapping;

                            typedef struct EntEntEndMapping {
                                int entityToConnectToIndex;
                                bool connectToStart;

                                int entityToBeMovedIndex;
                                bool moveStart;
                            } EntEntEndMapping;

                            Queue<EntEntEndMapping> movePairs = {};

                            _for_each_selected_entity_ {

                                int hot_entity_index = entity - drawing->entities.array;

                                endpoint_marks[hot_entity_index].start_marked = true;
                                endpoint_marks[hot_entity_index].end_marked = true;

                                for_(pass, 2) {
                                    vec2 seed; {
                                        vec2 p;
                                        if (pass == 0) {
                                            p = entity_get_start_point(&drawing->entities.array[hot_entity_index]);
                                        } else {
                                            p = entity_get_end_point(&drawing->entities.array[hot_entity_index]);
                                        }
                                        seed = make_key(p);
                                    }


                                    Queue<EntVecMapping> queue = {};
                                    queue_enqueue(&queue, { seed, hot_entity_index, pass == 0 });

                                    while (queue.length) {
                                        EntVecMapping curParent = queue_dequeue(&queue);
                                        seed = curParent.p;

                                        for (int dx = -1; dx <= 1; ++dx) {
                                            for (int dy = -1; dy <= 1; ++dy) {
                                                while (1) {
                                                    vec2 curPos = nudge_key(seed, dx, dy);
                                                    GridPointSlot *tmp = get_any_point_not_part_of_an_marked_entity(curPos);

                                                    if (!tmp) break;

                                                    Entity ent = drawing->entities.array[tmp->entity_index];

                                                    if (ent.type != EntityType::Line) {
                                                        GridPointSlot *nullCheck = get_any_point_not_part_of_an_marked_entity(get_key(tmp, true));
                                                        to_move[tmp->entity_index] = true;
                                                        endpoint_marks[tmp->entity_index].start_marked = true;
                                                        endpoint_marks[tmp->entity_index].end_marked = true;

                                                        if (nullCheck)  {
                                                            ASSERT(ent.type == EntityType::Arc);
                                                            vec2 startPoint = entity_get_start_point(&ent);
                                                            vec2 endPoint = entity_get_end_point(&ent);
                                                            bool start = distance(startPoint, curPos) < distance(endPoint, curPos);


                                                            queue_enqueue(&queue, { get_key(nullCheck, false), tmp->entity_index, !start }); // not start because this is the other end that we are adding
                                                        }
                                                    } else { // what to do if it is a line
                                                        vec2 startPoint = entity_get_start_point(&ent);
                                                        vec2 endPoint = entity_get_end_point(&ent);
                                                        bool start = distance(startPoint, curPos) < distance(endPoint, curPos);

                                                        if (start) {
                                                            endpoint_marks[tmp->entity_index].start_marked = true;
                                                        } else {
                                                            endpoint_marks[tmp->entity_index].end_marked = true;
                                                        }

                                                        queue_enqueue(&movePairs, { curParent.parentIndex, curParent.start, tmp->entity_index, start });
                                                    }


                                                } 
                                            }
                                        }
                                    }

                                    queue_free_AND_zero(&queue);
                                }
                            }

                            _for_each_entity_ { // TODO: dont actually need to go over each but im lazy 
                                if (to_move[entity - drawing->entities.array] || entity->is_selected) {
                                    if (entity->type == EntityType::Line) {
                                        LineEntity *line = &entity->line;
                                        line->start += click_vector;
                                        line->end   += click_vector;
                                    } else { ASSERT(entity->type == EntityType::Arc);
                                        ArcEntity *arc = &entity->arc;
                                        arc->center += click_vector;
                                    }
                                }
                            }


                            while (movePairs.length) {
                                EntEntEndMapping curMapping = queue_dequeue(&movePairs);

                                Entity *entToConnectTo = &drawing->entities.array[curMapping.entityToConnectToIndex];
                                Entity *entToMove = &drawing->entities.array[curMapping.entityToBeMovedIndex];

                                vec2 pointToConnectTo = curMapping.connectToStart ? entity_get_start_point(entToConnectTo) : entity_get_end_point(entToConnectTo);
                                if (entToMove->type == EntityType::Line) {
                                    if (popup->drag_extend_line == 0) {
                                        if (curMapping.moveStart) {
                                            entToMove->line.start = pointToConnectTo;
                                        } else {
                                            entToMove->line.end = pointToConnectTo;
                                        }
                                    } else {
                                        // TODO: dont make duplicate lines 
                                        // TODO: make more similar to Jim's idea
                                        cookbook.buffer_add_line(pointToConnectTo, curMapping.moveStart ? entToMove->line.start : entToMove->line.end);
                                    }
                                }
                            }

                            free(endpoint_marks);
                            queue_free_AND_zero(&movePairs);
                            map_free_and_zero(&grid);
                        } else if (state_Draw_command_is_(Move)) {
                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);
                            _for_each_selected_entity_ {
                                *entity = entity_translated(entity, click_vector);
                            }
                        } else if (state_Draw_command_is_(Copy)) {
                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);
                            uint num_additional_copies = MAX(1U, popup->linear_copy_num_additional_copies);
                            for_(i, num_additional_copies) {
                                vec2 translation_vector = real(i + 1) * click_vector;
                                bool is_selected = (i == num_additional_copies - 1);
                                _for_each_selected_entity_ {
                                    Entity new_entity = entity_translated(entity, translation_vector);
                                    new_entity.is_selected = is_selected;
                                    cookbook._buffer_add_entity(new_entity);
                                }
                            }
                            _for_each_selected_entity_ entity->is_selected = false;
                        } else if (state_Draw_command_is_(Polygon)) {
                            uint polygon_num_sides = popup->polygon_num_sides;
                            if (clicks_are_same) {
                                messagef(pallete.orange, "Polygon: must have non-zero size");
                            } else {
                                result.checkpoint_me = true;
                                set_state_Draw_command(None);
                                set_state_Snap_command(None);
                                real delta_theta = TAU / polygon_num_sides;
                                vec2 center = first_click;
                                vec2 vertex_0 = second_click;
                                real radius = distance(center, vertex_0);
                                real theta_0 = ATAN2(vertex_0 - center);
                                // cookbook.buffer_add_line(center, vertex_0); // center line (so sayeth LAYOUT)
                                for_(i, polygon_num_sides) {
                                    real theta_i = theta_0 + (i * delta_theta);
                                    real theta_ip1 = theta_0 + ((i + 1) * delta_theta);
                                    cookbook.buffer_add_line(
                                            get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta_i),
                                            get_point_on_circle_NOTE_pass_angle_in_radians(center, radius, theta_ip1)
                                            );
                                }
                            }
                        } else if (WINDOW_SELECT_OR_WINDOW_DESELECT()) {
                            set_state_Draw_command(None);
                            set_state_Xsel_command(None);
                            bbox2 window = {
                                MIN(first_click.x, second_click.x),
                                MIN(first_click.y, second_click.y),
                                MAX(first_click.x, second_click.x),
                                MAX(first_click.y, second_click.y)
                            };
                            _for_each_entity_ {
                                if (bbox_contains(window, entity_get_bbox(entity))) {
                                    cookbook.entity_set_is_selected(entity, value_to_write_to_selection_mask);
                                }
                            }
                        }
                    }
                } else {
                    if (state_Draw_command_is_(SetOrigin)) {
                        result.checkpoint_me = true;
                        set_state_Draw_command(None);
                        set_state_Snap_command(None);
                        drawing->origin = *mouse;
                    } else if (state_Draw_command_is_(DivideNearest)) { 
                        DXFFindClosestEntityResult closest_results = dxf_find_closest_entity(&drawing->entities, *mouse); // TODO *closest* -> *nearest*
                        if (closest_results.success) {
                            result.checkpoint_me = true;
                            set_state_Draw_command(None);
                            set_state_Snap_command(None);
                            Entity *entity = closest_results.closest_entity;
                            if (entity->type == EntityType::Line) {
                                LineEntity *line = &entity->line;
                                cookbook.buffer_add_line(line->start, 
                                        closest_results.line_nearest_point, 
                                        entity->is_selected, 
                                        entity->color_code);
                                cookbook.buffer_add_line(closest_results.line_nearest_point, 
                                        line->end, 
                                        entity->is_selected, 
                                        entity->color_code);
                                cookbook.buffer_delete_entity(entity);
                            } else { ASSERT(entity->type == EntityType::Arc);
                                ArcEntity *arc = &entity->arc;
                                if (ANGLE_IS_BETWEEN_CCW(RAD(closest_results.arc_nearest_angle_in_degrees), 
                                            RAD(arc->start_angle_in_degrees), 
                                            RAD(arc->end_angle_in_degrees))) {
                                    cookbook.buffer_add_arc(arc->center, 
                                            arc->radius, 
                                            arc->start_angle_in_degrees, 
                                            closest_results.arc_nearest_angle_in_degrees, 
                                            entity->is_selected, 
                                            entity->color_code);
                                    cookbook.buffer_add_arc(arc->center, 
                                            arc->radius, 
                                            closest_results.arc_nearest_angle_in_degrees, 
                                            arc->end_angle_in_degrees, 
                                            entity->is_selected, 
                                            entity->color_code);
                                    cookbook.buffer_delete_entity(entity);
                                }
                            }          
                        }
                    } else if (state_Draw_command_is_(PowerFillet)) {
                        do_once { messagef(pallete.red, "(Jim) i maybe broke this by messing with fillet\nlet's take a look together :)"); };
                        result.checkpoint_me = true;
                        set_state_Draw_command(None);
                        set_state_Snap_command(None);

                        List<Entity*> selected_entities = {};
                        selected_entities._capacity = 0;

                        _for_each_selected_entity_ {
                            if (entity->type == EntityType::Line) {
                                entity->line.start -= *mouse;
                                entity->line.end -= *mouse;
                            } else {
                                entity->arc.center -= *mouse;
                            }
                            list_push_back(&selected_entities, entity);
                        }

                        qsort(selected_entities.array, selected_entities.length, sizeof(Entity*), [](const void *a, const void *b) -> int {
                                real a_angle = ATAN2(entity_get_middle(*(Entity **)a)); // why not Enity *? idk but it crashes otherwise
                                real b_angle = ATAN2(entity_get_middle(*(Entity **)b)); 
                                if (a_angle < b_angle) {
                                return 1;
                                } else if (b_angle > a_angle) {
                                return -1;
                                }
                                return 0;
                                }); // no this is not evil 
                        for_(i, selected_entities.length) {
                            Entity *entity = selected_entities.array[i];
                            if (entity->type == EntityType::Line) {
                                entity->line.start += *mouse;
                                entity->line.end += *mouse;
                            } else { ASSERT(entity->type == EntityType::Arc);
                                entity->arc.center += *mouse;
                            }
                        }

                        for_(i, selected_entities.length) {
                            cookbook.attempt_fillet_ENTITIES_GET_DELETED_AT_END_OF_FRAME(selected_entities.array[i], selected_entities.array[(i+1) % selected_entities.length], *mouse, popup->fillet_radius);
                        }
                    } else if (state_Draw_command_is_(MirrorX)) {
                        result.checkpoint_me = true;
                        set_state_Draw_command(None);
                        set_state_Snap_command(None);
                        _for_each_selected_entity_ {
                            if (entity->type == EntityType::Line) {
                                LineEntity *line = &entity->line;
                                cookbook.buffer_add_line(
                                        V2(-(line->start.x - mouse->x) + mouse->x, line->start.y),
                                        V2(-(line->end.x - mouse->x) + mouse->x, line->end.y),
                                        true,
                                        entity->color_code
                                        );
                            } else if (entity->type == EntityType::Arc) {
                                ArcEntity *arc = &entity->arc;
                                cookbook.buffer_add_arc(
                                        V2(-(arc->center.x - mouse->x) + mouse->x, arc->center.y),
                                        arc->radius,
                                        180 - arc->end_angle_in_degrees,
                                        180 - arc->start_angle_in_degrees,
                                        true,
                                        entity->color_code);
                            } else { ASSERT(entity->type == EntityType::Circle);
                                // TODO
                                ;
                            }
                            entity->is_selected = false;
                        }
                    } else if (state_Draw_command_is_(MirrorY)) {
                        result.checkpoint_me = true;
                        set_state_Draw_command(None);
                        set_state_Snap_command(None);
                        _for_each_selected_entity_ {
                            if (entity->type == EntityType::Line) {
                                LineEntity *line = &entity->line;
                                cookbook.buffer_add_line(
                                        V2(line->start.x, -(line->start.y - mouse->y) + mouse->y),
                                        V2(line->end.x, -(line->end.y - mouse->y) + mouse->y),
                                        true,
                                        entity->color_code
                                        );
                            } else if (entity->type == EntityType::Arc) {
                                ArcEntity *arc = &entity->arc;
                                cookbook.buffer_add_arc(
                                        V2(arc->center.x, -(arc->center.y - mouse->y) + mouse->y),
                                        arc->radius,
                                        -arc->end_angle_in_degrees,
                                        -arc->start_angle_in_degrees,
                                        true,
                                        entity->color_code);
                            } else { ASSERT(entity->type == EntityType::Circle);
                                // TODO
                                ;
                            }
                            entity->is_selected = false;
                        }
                    } else if (state_Draw_command_is_(Offset)) {
                        // TODO: entity_offseted (and preview drawing)
                        if (IS_ZERO(popup->offset_distance)) {
                            messagef(pallete.orange, "Offset: must have non-zero distance");
                        } else {
                            DXFFindClosestEntityResult closest_results = dxf_find_closest_entity(&drawing->entities, *mouse);
                            if (closest_results.success) {
                                result.checkpoint_me = true;
                                set_state_Snap_command(None);
                                cookbook._buffer_add_entity(entity_offsetted(closest_results.closest_entity, popup->offset_distance, *mouse));
                            }
                        }
                    } else {
                        result.record_me = false;
                    }
                }
            }
        } else if (mouse_event->subtype == MouseEventSubtype::Mesh) {
            MouseEventMesh *mouse_event_mesh = &mouse_event->mouse_event_mesh;
            result.record_me = false;
            if (!mouse_event->mouse_held) {
                MagicSnapResult3D snap_result = magic_snap_3d();
                
                // int index_of_first_triangle_hit_by_ray = -1;
                // vec3 exact_hit_pos;
                // {
                //     real min_distance = HUGE_VAL;
                //     for_(i, mesh->num_triangles) {
                //         vec3 p[3]; {
                //             for_(j, 3) p[j] = mesh->vertex_positions[mesh->triangle_indices[i][j]];
                //         }
                //         RayTriangleIntersectionResult ray_triangle_intersection_result = ray_triangle_intersection(mouse_event_mesh->mouse_ray_origin, mouse_event_mesh->mouse_ray_direction, p[0], p[1], p[2]);
                //         if (ray_triangle_intersection_result.hit) {
                //             if (ray_triangle_intersection_result.distance < min_distance) {
                //                 min_distance = ray_triangle_intersection_result.distance;
                //                 exact_hit_pos = ray_triangle_intersection_result.pos;
                //                 index_of_first_triangle_hit_by_ray = i; // FORNOW
                //             }
                //         }
                //     }
                // }

                if (snap_result.hit_mesh) { // something hit
                    result.checkpoint_me = result.record_me = true;
                    if (state_Mesh_command_is_(Measure3D)) result.checkpoint_me = result.record_me = false;
                    feature_plane->is_active = !(state.Mesh_command.flags & HIDE_FEATURE_PLANE);
                    other.time_since_plane_selected = 0.0f;
                    {
                        feature_plane->normal = mesh->triangle_normals[snap_result.triangle_index];
                        feature_plane->signed_distance_to_world_origin = dot(feature_plane->normal, snap_result.mouse_position);

                        if (state.Mesh_command.flags & TWO_CLICK) {
                            if (!mesh_two_click_command->awaiting_second_click) {
                                mesh_two_click_command->first_click = snap_result.mouse_position;
                                mesh_two_click_command->awaiting_second_click = true;
                            } else {
                                vec3 first_click = mesh_two_click_command->first_click;
                                vec3 second_click = snap_result.mouse_position;

                                messagef(pallete.white, "First: %.3f %.3f %.3f\nSecond: %.3f %.3f %.3f\n", first_click.x, first_click.y, first_click.z, second_click.x, second_click.y, second_click.z);
                                if (0) {
                                } else if (state_Mesh_command_is_(Measure3D)) {
                                    messagef(pallete.cyan, "Length is %gmm.", norm(second_click - first_click));
                                    set_state_Mesh_command(None);
                                }
                            }
                        }
                    }
                }
            }
        } else { ASSERT(mouse_event->subtype == MouseEventSubtype::Popup);
            // NOTE: this case has been moved into POPUP();
            result.record_me = true; // FORNOW
        }
    } else if (event.type == EventType::None) {
        result.record_me = false;
    }

    { // sanity checks
      // ASSERT(popup->active_cell_index <= popup->num_cells);
        ASSERT(popup->cursor <= POPUP_CELL_LENGTH);
        ASSERT(popup->selection_cursor <= POPUP_CELL_LENGTH);
    }

    popup->_FORNOW_info_mouse_is_hovering = false; // FORNOW

    { // POPUP
        bool _gui_key_enter; {
            _gui_key_enter = false;
            if (event.type == EventType::Key) {
                KeyEvent *key_event = &event.key_event;
                if (key_event->subtype == KeyEventSubtype::Popup) {
                    _gui_key_enter = (key_event->key == GLFW_KEY_ENTER);
                }
            }
        }

        auto gui_key_enter = [&](ToolboxGroup group) {
            return (_gui_key_enter && (group == popup->manager.focus_group));
        };

        { // POPUP
            { // Snap
                if (state_Snap_command_is_(XY)) {
                    // sus calling this a modifier but okay; make sure it's first or else bad bad
                    POPUP(state.Snap_command,
                            true,
                            CellType::Real, STRING("x"), &popup->xy_x_coordinate,
                            CellType::Real, STRING("y"), &popup->xy_y_coordinate);
                    if (gui_key_enter(ToolboxGroup::Snap)) {
                        set_state_Snap_command(None);
                        return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(popup->xy_x_coordinate, popup->xy_y_coordinate));
                    }
                }
            }

            { // click_mode
                vec2 *first_click = &two_click_command->first_click;
                if (state_Draw_command_is_(Circle)) {
                    if (two_click_command->awaiting_second_click) {
                        real prev_circle_diameter = popup->circle_diameter;
                        real prev_circle_radius = popup->circle_radius;
                        real prev_circle_circumference = popup->circle_circumference;
                        POPUP(state.Draw_command,
                                false,
                                CellType::Real, STRING("diameter"), &popup->circle_diameter,
                                CellType::Real, STRING("radius"), &popup->circle_radius,
                                CellType::Real, STRING("circumference"), &popup->circle_circumference);
                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->circle_radius, first_click->y));
                        } else {
                            if (prev_circle_diameter != popup->circle_diameter) {
                                popup->circle_radius = popup->circle_diameter / 2;
                                popup->circle_circumference = PI * popup->circle_diameter;
                            } else if (prev_circle_radius != popup->circle_radius) {
                                popup->circle_diameter = 2 * popup->circle_radius;
                                popup->circle_circumference = PI * popup->circle_diameter;
                            } else if (prev_circle_circumference != popup->circle_circumference) {
                                popup->circle_diameter = popup->circle_circumference / PI;
                                popup->circle_radius = popup->circle_diameter / 2;
                            }
                        }
                    }
                } else if (state_Draw_command_is_(DiamCircle)) {
                    ;
                } else if (state_Draw_command_is_(Line)) {
                    if (two_click_command->awaiting_second_click) {
                        real prev_line_length = popup->line_length;
                        real prev_line_angle  = popup->line_angle;
                        real prev_line_run    = popup->line_run;
                        real prev_line_rise   = popup->line_rise;
                        POPUP(state.Draw_command,
                                true,
                                CellType::Real, STRING("run (dx)"),    &popup->line_run,
                                CellType::Real, STRING("rise (dy)"),   &popup->line_rise,
                                CellType::Real, STRING("length"), &popup->line_length,
                                CellType::Real, STRING("angle"),  &popup->line_angle
                             );
                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->line_run, first_click->y + popup->line_rise));
                        } else {
                            if ((prev_line_length != popup->line_length) || (prev_line_angle != popup->line_angle)) {
                                popup->line_run  = popup->line_length * COS(RAD(popup->line_angle));
                                popup->line_rise = popup->line_length * SIN(RAD(popup->line_angle));
                            } else if ((prev_line_run != popup->line_run) || (prev_line_rise != popup->line_rise)) {
                                popup->line_length = SQRT(popup->line_run * popup->line_run + popup->line_rise * popup->line_rise);
                                popup->line_angle = DEG(ATAN2(popup->line_rise, popup->line_run));
                            }
                        }
                    }
                } else if (state_Draw_command_is_(CenterLine)) {
                    // Copied from Line
                    if (two_click_command->awaiting_second_click) {
                        real prev_line_length = popup->line_length;
                        real prev_line_angle  = popup->line_angle;
                        real prev_line_run    = popup->line_run;
                        real prev_line_rise   = popup->line_rise;
                        POPUP(state.Draw_command,
                                true,
                                CellType::Real, STRING("run (dx)"),    &popup->line_run,
                                CellType::Real, STRING("rise (dy)"),   &popup->line_rise,
                                CellType::Real, STRING("length"), &popup->line_length,
                                CellType::Real, STRING("angle"),  &popup->line_angle
                             );
                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->line_run, first_click->y + popup->line_rise));
                        } else {
                            if ((prev_line_length != popup->line_length) || (prev_line_angle != popup->line_angle)) {
                                popup->line_run  = popup->line_length * COS(RAD(popup->line_angle));
                                popup->line_rise = popup->line_length * SIN(RAD(popup->line_angle));
                            } else if ((prev_line_run != popup->line_run) || (prev_line_rise != popup->line_rise)) {
                                popup->line_length = SQRT(popup->line_run * popup->line_run + popup->line_rise * popup->line_rise);
                                popup->line_angle = DEG(ATAN2(popup->line_rise, popup->line_run));
                            }
                        }
                    }
                } else if (state_Draw_command_is_(Box)) {
                    if (two_click_command->awaiting_second_click) {
                        POPUP(state.Draw_command,
                                true,
                                CellType::Real, STRING("width"), &popup->box_width,
                                CellType::Real, STRING("height"), &popup->box_height);
                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->box_width, first_click->y + popup->box_height));
                        }
                    }
                } else if (state_Draw_command_is_(CenterBox)) {
                    if (two_click_command->awaiting_second_click) {
                        POPUP(state.Draw_command,
                                true,
                                CellType::Real, STRING("width"), &popup->box_width,
                                CellType::Real, STRING("height"), &popup->box_height);
                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->box_width / 2.0f, first_click->y + popup->box_height / 2.0f));
                        }
                    }
                } else if (state_Draw_command_is_(Drag)) {
                    // FORNOW: this is repeated from Line
                    if (two_click_command->awaiting_second_click) {
                        real prev_drag_length = popup->drag_length;
                        real prev_drag_angle = popup->drag_angle;
                        real prev_drag_run = popup->drag_run;
                        real prev_drag_rise = popup->drag_rise;
                        POPUP(state.Draw_command,
                                true,
                                CellType::Uint, STRING("1 for extend line"), &popup->drag_extend_line,
                                CellType::Real, STRING("run (dx)"), &popup->drag_run,
                                CellType::Real, STRING("rise (dy)"), &popup->drag_rise,
                                CellType::Real, STRING("length"), &popup->drag_length,
                                CellType::Real, STRING("angle"), &popup->drag_angle
                             );
                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->drag_run, first_click->y + popup->drag_rise));
                        } else {
                            if ((prev_drag_length != popup->drag_length) || (prev_drag_angle != popup->drag_angle)) {
                                popup->drag_run = popup->drag_length * COS(RAD(popup->drag_angle));
                                popup->drag_rise = popup->drag_length * SIN(RAD(popup->drag_angle));
                            } else if ((prev_drag_run != popup->drag_run) || (prev_drag_rise != popup->drag_rise)) {
                                popup->drag_length = SQRT(popup->drag_run * popup->drag_run + popup->drag_rise * popup->drag_rise);
                                popup->drag_angle = DEG(ATAN2(popup->drag_rise, popup->drag_run));
                            }
                        }
                    }
                } else if (state_Draw_command_is_(Move)) {
                    // FORNOW: this is repeated from Line
                    if (two_click_command->awaiting_second_click) {
                        real prev_move_length = popup->move_length;
                        real prev_move_angle = popup->move_angle;
                        real prev_move_run = popup->move_run;
                        real prev_move_rise = popup->move_rise;
                        POPUP(state.Draw_command,
                                true,
                                CellType::Real, STRING("run (dx)"), &popup->move_run,
                                CellType::Real, STRING("rise (dy)"), &popup->move_rise,
                                CellType::Real, STRING("length"), &popup->move_length,
                                CellType::Real, STRING("angle"), &popup->move_angle
                             );
                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->move_run, first_click->y + popup->move_rise));
                        } else {
                            if ((prev_move_length != popup->move_length) || (prev_move_angle != popup->move_angle)) {
                                popup->move_run = popup->move_length * COS(RAD(popup->move_angle));
                                popup->move_rise = popup->move_length * SIN(RAD(popup->move_angle));
                            } else if ((prev_move_run != popup->move_run) || (prev_move_rise != popup->move_rise)) {
                                popup->move_length = SQRT(popup->move_run * popup->move_run + popup->move_rise * popup->move_rise);
                                popup->move_angle = DEG(ATAN2(popup->move_rise, popup->move_run));
                            }
                        }
                    }
                } else if (state_Draw_command_is_(Rotate)) {
                    if (two_click_command->awaiting_second_click) {
                        POPUP(state.Draw_command,
                                true,
                                CellType::Real, STRING("angle"), &popup->rotate_angle
                             );
                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(*first_click + e_theta(RAD(popup->rotate_angle))));
                        }
                    }
                } else if (state_Draw_command_is_(RCopy)) {
                    if (two_click_command->awaiting_second_click) {
                        real prev_rotate_copy_angle_in_degrees = popup->rotate_copy_angle;
                        uint prev_rotate_copy_num_copies = popup->rotate_copy_num_total_copies;
                        POPUP(state.Draw_command,
                                true,
                                CellType::Uint, STRING("num_total_copies"), &popup->rotate_copy_num_total_copies,
                                CellType::Real, STRING("angle"), &popup->rotate_copy_angle
                             );
                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D({})); // FORNOW
                        } else {
                            if (prev_rotate_copy_angle_in_degrees != popup->rotate_copy_angle) {
                                popup->rotate_copy_num_total_copies = MAX(2U, uint(360.0f / popup->rotate_copy_angle));
                            } else if (prev_rotate_copy_num_copies != popup->rotate_copy_num_total_copies) {
                                popup->rotate_copy_angle = 360.0f / popup->rotate_copy_num_total_copies;
                            }
                        }
                    }
                } else if (state_Draw_command_is_(Copy)) {
                    if (two_click_command->awaiting_second_click) {
                        real prev_linear_copy_length = popup->linear_copy_length;
                        real prev_linear_copy_angle = popup->linear_copy_angle;
                        real prev_linear_copy_run = popup->linear_copy_run;
                        real prev_linear_copy_rise = popup->linear_copy_rise;

                        POPUP(state.Draw_command,
                                true,
                                CellType::Uint, STRING("num_additional_copies"), &popup->linear_copy_num_additional_copies,
                                CellType::Real, STRING("run (dx)"), &popup->linear_copy_run,
                                CellType::Real, STRING("rise (dy)"), &popup->linear_copy_rise,
                                CellType::Real, STRING("length"), &popup->linear_copy_length,
                                CellType::Real, STRING("angle"), &popup->linear_copy_angle
                             );
                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->linear_copy_run, first_click->y + popup->linear_copy_rise));
                        } else {
                            if ((prev_linear_copy_length != popup->linear_copy_length) || (prev_linear_copy_angle != popup->linear_copy_angle)) {
                                popup->linear_copy_run = popup->linear_copy_length * COS(RAD(popup->linear_copy_angle));
                                popup->linear_copy_rise = popup->linear_copy_length * SIN(RAD(popup->linear_copy_angle));
                            } else if ((prev_linear_copy_run != popup->linear_copy_run) || (prev_linear_copy_rise != popup->linear_copy_rise)) {
                                popup->linear_copy_length = SQRT(popup->linear_copy_run * popup->linear_copy_run + popup->linear_copy_rise * popup->linear_copy_rise);
                                popup->linear_copy_angle = DEG(ATAN2(popup->linear_copy_rise, popup->linear_copy_run));
                            }
                        }
                    }
                } else if (state_Draw_command_is_(Polygon)) {
                    if (two_click_command->awaiting_second_click) {
                        uint prev_polygon_num_sides = popup->polygon_num_sides;
                        real prev_polygon_distance_to_corner = popup->polygon_distance_to_corner;
                        real prev_polygon_distance_to_side = popup->polygon_distance_to_side;
                        real prev_polygon_side_length = popup->polygon_side_length;
                        POPUP(state.Draw_command,
                                false,
                                CellType::Uint, STRING("num_sides"), &popup->polygon_num_sides, 
                                CellType::Real, STRING("distance_to_corner"), &popup->polygon_distance_to_corner,
                                CellType::Real, STRING("distance_to_side"), &popup->polygon_distance_to_side,
                                CellType::Real, STRING("side_length"), &popup->polygon_side_length);

                        popup->polygon_num_sides = MIN(MAX(3U, popup->polygon_num_sides), 256U);
                        if (gui_key_enter(ToolboxGroup::Draw)) {
                            return _standard_event_process_NOTE_RECURSIVE(make_mouse_event_2D(first_click->x + popup->polygon_distance_to_corner, first_click->y));
                        } else {
                            popup->polygon_num_sides = MAX(3U, popup->polygon_num_sides); // FORNOW
                            real theta = PI / popup->polygon_num_sides;
                            if (prev_polygon_num_sides != popup->polygon_num_sides) {
                                popup->polygon_distance_to_side = popup->polygon_distance_to_corner * COS(theta);
                                popup->polygon_side_length = 2 * popup->polygon_distance_to_corner * SIN(theta);
                            } else if (prev_polygon_distance_to_corner != popup->polygon_distance_to_corner) {
                                popup->polygon_distance_to_side = popup->polygon_distance_to_corner * COS(theta);
                                popup->polygon_side_length = 2 * popup->polygon_distance_to_corner * SIN(theta);
                            } else if (prev_polygon_distance_to_side != popup->polygon_distance_to_side) {
                                popup->polygon_distance_to_corner = popup->polygon_distance_to_side / COS(theta); 
                                popup->polygon_side_length = 2 * popup->polygon_distance_to_side * TAN(theta);
                            } else if (prev_polygon_side_length != popup->polygon_side_length) {
                                popup->polygon_distance_to_corner = popup->polygon_side_length / (2 * SIN(theta));
                                popup->polygon_distance_to_side = popup->polygon_side_length / (2 * TAN(theta));
                            }
                        }
                    }
                } else if (state_Draw_command_is_(Offset)) {
                    POPUP(state.Draw_command,
                            false,
                            CellType::Real, STRING("distance"), &popup->offset_distance);
                } else if (state_Draw_command_is_(Fillet)) {
                    POPUP(state.Draw_command,
                            false,
                            CellType::Real, STRING("radius"), &popup->fillet_radius);
                } else if (state_Draw_command_is_(DogEar)) {
                    POPUP(state.Draw_command,
                            false,
                            CellType::Real, STRING("radius"), &popup->dogear_radius);
                } else if (state_Draw_command_is_(PowerFillet)) {
                    POPUP(state.Draw_command,
                            false,
                            CellType::Real, STRING("radius"), &popup->fillet_radius);
                } else if (state_Draw_command_is_(OpenDXF)) {
                    POPUP(state.Draw_command,
                            false,
                            CellType::String, STRING("filename"), &popup->open_dxf_filename);
                    if (gui_key_enter(ToolboxGroup::Draw)) {
                        if (FILE_EXISTS(popup->open_dxf_filename)) {
                            if (string_matches_suffix(popup->open_dxf_filename, STRING(".dxf"))) {
                                result.record_me = true;
                                result.checkpoint_me = true;
                                result.snapshot_me = true;

                                { // conversation_dxf_load
                                    ASSERT(FILE_EXISTS(popup->open_dxf_filename));

                                    list_free_AND_zero(&drawing->entities);

                                    entities_load(popup->open_dxf_filename, &drawing->entities);

                                    if (!skip_mesh_generation_and_expensive_loads_because_the_caller_is_going_to_load_from_the_redo_stack) {
                                        init_camera_drawing();
                                        drawing->origin = {};
                                    }
                                }
                                set_state_Draw_command(None);
                                messagef(pallete.light_gray, "OpenDXF \"%s\"", popup->open_dxf_filename.data);
                            } else {
                                messagef(pallete.orange, "OpenDXF: \"%s\" must be *.dxf", popup->open_dxf_filename.data);
                            }
                        } else {
                            messagef(pallete.orange, "Load: \"%s\" not found", popup->open_dxf_filename.data);
                        }
                    }
                } else if (state_Draw_command_is_(OverwriteDXF)) {
                    result.record_me = false;
                    POPUP(state.Draw_command,
                            true,
                            CellType::String, STRING("confirm (y/n)"), &popup->overwrite_dxf_yn_buffer);
                    if (gui_key_enter(ToolboxGroup::Draw)) {
                        if (popup->overwrite_dxf_yn_buffer.data[0] == 'y') {
                            if (string_matches_suffix(popup->save_dxf_filename, STRING(".dxf"))) {
                                {
                                    bool success = drawing_save_dxf(drawing, popup->save_dxf_filename);
                                    ASSERT(success);
                                }
                                set_state_Draw_command(None);
                                messagef(pallete.light_gray, "OverwriteDXF \"%s\"", popup->save_dxf_filename.data);
                            } else {
                                messagef(pallete.orange, "OverwriteDXF \"%s\" must be *.dxf", popup->save_dxf_filename.data);
                            }
                        } else if (popup->overwrite_dxf_yn_buffer.data[0] == 'n') {
                            set_state_Draw_command(None);
                            messagef(pallete.orange, "OverwriteDXF declined");
                        } else {
                            messagef(pallete.orange, "OverwriteDXF confirm with y or n");
                        }
                    }
                } else if (state_Draw_command_is_(SaveDXF)) {
                    result.record_me = false;
                    POPUP(state.Draw_command,
                            false,
                            CellType::String, STRING("filename"), &popup->save_dxf_filename);
                    if (gui_key_enter(ToolboxGroup::Draw)) {
                        if (!FILE_EXISTS(popup->save_dxf_filename)) {
                            if (string_matches_suffix(popup->save_dxf_filename, STRING(".dxf"))) {
                                set_state_Draw_command(None);
                                {
                                    bool success = drawing_save_dxf(drawing, popup->save_dxf_filename);
                                    ASSERT(success);
                                }
                                messagef(pallete.light_gray, "SaveDXF \"%s\"", popup->save_dxf_filename.data);
                            } else {
                                messagef(pallete.orange, "SaveDXF \"%s\" must be *.dxf", popup->save_dxf_filename.data);
                            }
                        } else {
                            messagef(pallete.orange, "SaveDXF \"%s\" already exists", popup->save_dxf_filename.data);
                            set_state_Draw_command(OverwriteDXF);
                        }
                    }
                } else if (state_Draw_command_is_(Scale)) {
                    result.record_me = false;
                    POPUP(state.Draw_command,
                            false,
                            CellType::Real, STRING("scale factor"), &popup->scale_factor);
                    if (gui_key_enter(ToolboxGroup::Draw)) {
                        if (!IS_ZERO(popup->scale_factor)) {
                            bbox2 bbox = entities_get_bbox(&drawing->entities, true);
                            vec2 bbox_center = AVG(bbox.min, bbox.max);
                            _for_each_selected_entity_ {
                                if (entity->type == EntityType::Line) {
                                    LineEntity *line = &entity->line;
                                    line->start = scaled_about(line->start, bbox_center, popup->scale_factor);
                                    line->end = scaled_about(line->end, bbox_center, popup->scale_factor);
                                } else if (entity->type == EntityType::Arc) {
                                    ArcEntity *arc = &entity->arc;
                                    arc->center = scaled_about(arc->center, bbox_center, popup->scale_factor);
                                    arc->radius *= popup->scale_factor;
                                } else { ASSERT(entity->type == EntityType::Circle);
                                    CircleEntity *circle = &entity->circle;
                                    circle->center = scaled_about(circle->center, bbox_center, popup->scale_factor);
                                    circle->radius *= popup->scale_factor;
                                }
                            }
                        }
                        set_state_Draw_command(None);
                    }
                }
            }

            { // Mesh
                if (0) {
                } else if (state_Mesh_command_is_(OpenSTL)) {
                    POPUP(state.Mesh_command,
                            false,
                            CellType::String, STRING("filename"), &popup->open_stl_filename);
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (FILE_EXISTS(popup->open_stl_filename)) {
                            if (string_matches_suffix(popup->open_stl_filename, STRING(".stl"))) {
                                result.record_me = true;
                                result.checkpoint_me = true;
                                result.snapshot_me = true;
                                { // conversation_stl_load(...)
                                    ASSERT(FILE_EXISTS(popup->open_stl_filename));
                                    // ?
                                    stl_load(popup->open_stl_filename, mesh);
                                    init_camera_mesh();
                                }
                                set_state_Mesh_command(None);
                                messagef(pallete.light_gray, "OpenSTL \"%s\"", popup->open_stl_filename.data);
                            } else {
                                messagef(pallete.orange, "OpenSTL: \"%s\" must be *.stl", popup->open_stl_filename.data);
                            }
                        } else {
                            messagef(pallete.orange, "Load: \"%s\" not found", popup->open_stl_filename.data);
                        }
                    }
                } else if (state_Mesh_command_is_(OverwriteSTL)) {
                    result.record_me = false;
                    POPUP(state.Mesh_command,
                            true,
                            CellType::String, STRING("confirm (y/n)"), &popup->overwrite_stl_yn_buffer);
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (popup->overwrite_stl_yn_buffer.data[0] == 'y') {
                            if (string_matches_suffix(popup->save_stl_filename, STRING(".stl"))) {
                                {
                                    bool success = mesh_save_stl(mesh, popup->save_stl_filename);
                                    ASSERT(success);
                                }
                                set_state_Mesh_command(None);
                                messagef(pallete.light_gray, "OverwriteSTL \"%s\"", popup->save_stl_filename.data);
                            } else {
                                messagef(pallete.orange, "OverwriteSTL \"%s\" must be *.stl", popup->save_stl_filename.data);
                            }
                        } else if (popup->overwrite_stl_yn_buffer.data[0] == 'n') {
                            set_state_Mesh_command(None);
                            messagef(pallete.orange, "OverwriteSTL declined");
                        } else {
                            messagef(pallete.orange, "OverwriteSTL confirm with y or n");
                        }
                    }
                } else if (state_Mesh_command_is_(SaveSTL)) {
                    result.record_me = false;
                    POPUP(state.Mesh_command,
                            false,
                            CellType::String, STRING("filename"), &popup->save_stl_filename);
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (!FILE_EXISTS(popup->save_stl_filename)) {
                            if (string_matches_suffix(popup->save_stl_filename, STRING(".stl"))) {
                                set_state_Mesh_command(None);
                                {
                                    bool success = mesh_save_stl(mesh, popup->save_stl_filename);
                                    ASSERT(success);
                                }
                                messagef(pallete.light_gray, "SaveSTL \"%s\"", popup->save_stl_filename.data);
                            } else {
                                messagef(pallete.orange, "SaveSTL \"%s\" must be *.stl", popup->save_stl_filename.data);
                            }
                        } else {
                            messagef(pallete.orange, "SaveSTL \"%s\" already exists", popup->save_stl_filename.data);
                            set_state_Mesh_command(OverwriteSTL);
                        }
                    }
                } else if (state_Mesh_command_is_(ExtrudeAdd)) {
                    POPUP(state.Mesh_command,
                            true,
                            CellType::Real, STRING("out_length"), &popup->extrude_add_out_length,
                            CellType::Real, STRING("in_length"),  &popup->extrude_add_in_length);
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (!dxf_anything_selected) {
                            messagef(pallete.orange, "ExtrudeAdd: selection empty");
                        } else if (!feature_plane->is_active) {
                            messagef(pallete.orange, "ExtrudeAdd: no feature plane selected");
                        } else if (IS_ZERO(popup->extrude_add_in_length) && IS_ZERO(popup->extrude_add_out_length)) {
                            messagef(pallete.orange, "ExtrudeAdd: total extrusion length zero");
                        } else {
                            cookbook.manifold_wrapper();
                            if (IS_ZERO(popup->extrude_add_in_length)) {
                                messagef(pallete.light_gray, "ExtrudeAdd %gmm", popup->extrude_add_out_length);
                            } else {
                                messagef(pallete.light_gray, "ExtrudeAdd %gmm %gmm", popup->extrude_add_out_length, popup->extrude_add_in_length);
                            }
                        }
                    }
                } else if (state_Mesh_command_is_(ExtrudeCut)) {
                    POPUP(state.Mesh_command,
                            true,
                            CellType::Real, STRING("in_length"), &popup->extrude_cut_in_length,
                            CellType::Real, STRING("out_length"), &popup->extrude_cut_out_length);
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (!dxf_anything_selected) {
                            messagef(pallete.orange, "ExtrudeCut: selection empty");
                        } else if (!feature_plane->is_active) {
                            messagef(pallete.orange, "ExtrudeCut: no feature plane selected");
                        } else if (IS_ZERO(popup->extrude_cut_in_length) && IS_ZERO(popup->extrude_cut_out_length)) {
                            messagef(pallete.orange, "ExtrudeCut: total extrusion length zero");
                        } else if (mesh->num_triangles == 0) {
                            messagef(pallete.orange, "ExtrudeCut: current mesh empty");
                        } else {
                            cookbook.manifold_wrapper();
                            if (IS_ZERO(popup->extrude_cut_out_length)) {
                                messagef(pallete.light_gray, "ExtrudeCut %gmm", popup->extrude_cut_in_length);
                            } else {
                                messagef(pallete.light_gray, "ExtrudeCut %gmm %gmm", popup->extrude_cut_in_length, popup->extrude_cut_out_length);
                            }
                        }
                    }
                } else if (state_Mesh_command_is_(RevolveAdd)) {
                    POPUP(state.Mesh_command,
                            true,
                            CellType::Real, STRING("out_angle"), &popup->revolve_add_out_angle,
                            CellType::Real, STRING("in_angle"), &popup->revolve_add_in_angle
                         );
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (!dxf_anything_selected) {
                            messagef(pallete.orange, "RevolveAdd: selection empty");
                        } else if (!feature_plane->is_active) {
                            messagef(pallete.orange, "RevolveAdd: no feature plane selected");
                        } else {
                            cookbook.manifold_wrapper();
                            messagef(pallete.light_gray, "RevolveAdd");
                        }
                    }
                } else if (state_Mesh_command_is_(RevolveCut)) {
                    POPUP(state.Mesh_command,
                            true,
                            CellType::Real, STRING("in_angle"), &popup->revolve_cut_in_angle,
                            CellType::Real, STRING("out_angle"), &popup->revolve_cut_out_angle
                         );
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        if (!dxf_anything_selected) {
                            messagef(pallete.orange, "RevolveCut: selection empty");
                        } else if (!feature_plane->is_active) {
                            messagef(pallete.orange, "RevolveCut: no feature plane selected");
                        } else if (mesh->num_triangles == 0) {
                            messagef(pallete.orange, "RevolveCut: current mesh empty");
                        } else {
                            cookbook.manifold_wrapper();
                            messagef(pallete.light_gray, "RevolveCut");
                        }
                    }
                } else if (state_Mesh_command_is_(NudgePlane)) {
                    POPUP(state.Mesh_command,
                            true,
                            CellType::Real, STRING("rise"), &popup->feature_plane_nudge);
                    if (gui_key_enter(ToolboxGroup::Mesh)) {
                        result.record_me = true;
                        result.checkpoint_me = true;
                        feature_plane->signed_distance_to_world_origin += popup->feature_plane_nudge;
                        set_state_Mesh_command(None);
                        messagef(pallete.light_gray, "NudgePlane %gmm", popup->feature_plane_nudge);
                    }
                }
            }
        }
    }


    // FORNOW: remove zero length at end of loop TODO: don't allow their creation
    _for_each_entity_ {
        if (entity_length(entity) < GRID_CELL_WIDTH) {
            messagef("WARNING: zero length entity detected and deleted");
            cookbook.buffer_delete_entity(entity);
        }
        if (entity->type == EntityType::Circle) {
            if (entity->circle.radius < GRID_CELL_WIDTH) {
                messagef("WARNING: zero length entity detected and deleted");
                cookbook.buffer_delete_entity(entity);
            }
        }
        if (entity->type == EntityType::Arc) {
            if (entity->arc.radius < GRID_CELL_WIDTH) {
                messagef("WARNING: zero length entity detected and deleted");
                cookbook.buffer_delete_entity(entity);
            }
        }
    }

    // event_passed_to_popups = {}; // FORNOW: probably unnecessary
    // already_processed_event_passed_to_popups = false; // FORNOW: probably unnecessary
    return result;
}

#if 0
void history_process_event(Event freshly_baked_event) {
    // bool undo;
    // bool redo;
    // {
    //     undo = false;
    //     redo = false;
    //     if (freshly_baked_event.type == EventType::Key) {
    //         KeyEvent *key_event = &freshly_baked_event.key_event;
    //         auto key_lambda = [key_event](uint key, bool control = false, bool shift = false) -> bool {
    //             return _key_lambda(key_event, key, control, shift);
    //         };
    //         if (!((popup->_FORNOW_active_popup_unique_ID__FORNOW_name0) && (popup->cell_type[popup->active_cell_index] == CellType::String))) { // FORNOW
    //             undo = (key_lambda('Z', true) || key_lambda('U'));
    //             redo = (key_lambda('Y', true) || key_lambda('Z', true, true) || key_lambda('U', false, true));
    //         }
    //     }
    // }

    // if (undo) {
    //     other._please_suppress_drawing_popup_popup = true;
    //     history_undo();
    // } else if (redo) {
    //     _standard_event_process_NOTE_RECURSIVE({}); // FORNOW (prevent flicker on redo with nothing left to redo)
    //     other._please_suppress_drawing_popup_popup = true;
    //     history_redo();
    // } else {
    history_process_event(freshly_baked_event);
    // }
}
#endif
// <!> End process.cpp <!>
// <!> Begin script.cpp <!> 
void script_process(String string) {
    // TODO: gui
    // TODOLATER (weird 'X' version): char *string = "^osplash.drawing\nyscx2020\ne\t50";
    #define TAG_LENGTH 3
    bool control = false;
    for_(i, string.length) {
        char c = string.data[i];
        if (c == '^') {
            control = true;
        } else if (c == '<') {
            bool is_instabaked = true;
            Event instabaked_event = {};
            RawEvent _raw_event = {};
            {
                uint next_i; {
                    next_i = i;
                    while (string.data[++next_i] != '>') {} // ++next_i intentional
                }
                {
                    char *tag = &string.data[i + 1];
                    if (strncmp(tag, "m2d", TAG_LENGTH) == 0) {
                        // NOTE: user of this api specified click in pre-snapped world coordinates
                        //       (as opposed to [pre-snapped] pixel coordinates, which is what bake_event takes)
                        // FORNOW
                        char *params = &string.data[i + 1 + TAG_LENGTH];
                        vec2 p; {
                            sscanf(params, "%f %f", &p.x, &p.y);
                        }
                        MagicSnapResult snap_result = magic_snap(p);
                        instabaked_event = make_mouse_event_2D(snap_result.mouse_position);
                        instabaked_event.mouse_event.mouse_event_drawing.snap_result = snap_result;
                    } else if (strncmp(tag, "m3d", TAG_LENGTH) == 0) {
                        char *params = &string.data[i + 1 + TAG_LENGTH];
                        vec3 mouse_ray_origin;
                        vec3 mouse_ray_direction;
                        sscanf(params, "%f %f %f %f %f %f", &mouse_ray_origin.x, &mouse_ray_origin.y, &mouse_ray_origin.z, &mouse_ray_direction.x, &mouse_ray_direction.y, &mouse_ray_direction.z);
                        instabaked_event = make_mouse_event_3D(mouse_ray_origin, mouse_ray_direction);
                    } else if (strncmp(tag, "esc", TAG_LENGTH) == 0) {
                        is_instabaked = false;
                        _raw_event.type = EventType::Key;
                        RawKeyEvent *_raw_key_event = &_raw_event.raw_key_event;
                        _raw_key_event->key = GLFW_KEY_ESCAPE;
                    }
                }
                i = next_i;
            }
            if (is_instabaked) {
                history_process_event(instabaked_event);
            } else {
                Event freshly_baked_event = bake_event(_raw_event);
                history_process_event(freshly_baked_event);
            }
        } else {
            RawEvent raw_event = {};
            raw_event.type = EventType::Key;
            RawKeyEvent *raw_key_event = &raw_event.raw_key_event;
            raw_key_event->control = control;
            {
                if ('a' <= c && c <= 'z') {
                    raw_key_event->key = 'A' + (c - 'a');
                } else if ('A' <= c && c <= 'Z') {
                    raw_key_event->shift = true;
                    raw_key_event->key = c;
                } else if (c == '{') {
                    raw_key_event->shift = true;
                    raw_key_event->key = '[';
                } else if (c == '}') {
                    raw_key_event->shift = true;
                    raw_key_event->key = ']';
                } else if (c == '\n') {
                    raw_key_event->key = GLFW_KEY_ENTER;
                } else if (c == '\t') {
                    raw_key_event->key = GLFW_KEY_TAB;
                } else if (c == '\33') {
                    raw_key_event->key = GLFW_KEY_ESCAPE;
                } else if (c == '\b') {
                    raw_key_event->key = GLFW_KEY_BACKSPACE;
                } else {
                    raw_key_event->key = c;
                }
            }
            control = false;
            Event freshly_baked_event = bake_event(raw_event);
            history_process_event(freshly_baked_event);
        }
    }
}
// <!> End script.cpp <!>

int main() {
    { // init
        init_camera_drawing();
        init_camera_mesh();
        script_process(STRING(startup_script));
        { // callbacks
            glfwSetKeyCallback(glfw_window, callback_key);
            glfwSetCursorPosCallback(glfw_window, callback_cursor_position);
            glfwSetMouseButtonCallback(glfw_window, callback_mouse_button);
            glfwSetScrollCallback(glfw_window, callback_scroll);
            glfwSetFramebufferSizeCallback(glfw_window, callback_framebuffer_size);
            glfwSetDropCallback(glfw_window, callback_drop);
            { // NOTE: patch first frame mouse position issue
                other.OpenGL_from_Pixel = window_get_OpenGL_from_Pixel();

                { // spoof callback_cursor_position
                    double xpos, ypos;
                    glfwGetCursorPos(glfw_window, &xpos, &ypos);
                    callback_cursor_position(NULL, xpos, ypos);
                }
            }
        }
        // glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        { // cursors_init();
            other.cursors.crosshair = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
            other.cursors.ibeam = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
            other.cursors.hresize = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
            other.cursors.hand = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
        }
    }


    messagef(pallete.red, "TODO: Center snap should visualize the entity (same for all of them i think)");
    messagef(pallete.red, "TODO: expand scripting to allow SHIFT+SPACE (just use what vimrc does)");
    messagef(pallete.red, "TODO: measure should populate the active Mesh field; this would be really nice");
    messagef(pallete.red, "TODO: popup->linear_copy_num_additional_copies should default to 1 (ZERO_OUT is problem)");
    messagef(pallete.red, "TODO: make negative extrude and revolves work");
    messagef(pallete.red, "TODO: blue move/rotate/copy (certainly) still needs work");
    // messagef(pallete.red, "TODO: move shouldn't snap to entities being moved");
    // messagef(pallete.blue, "TODO: EXCLUDE_SELECTED_ENTITIES_FROM_SECOND_CLICK_SNAP flag");
    messagef(pallete.red, "TODO: rotate about origin bumps the mouse unnecessarily (or like...wrong?)");
    messagef(pallete.yellow, "TODO: CIRCLE type entity");
    messagef(pallete.yellow, "TODO: - Select Connected");
    messagef(pallete.yellow, "TODO: - TwoClickDivide");
    messagef(pallete.green, "Offset broken clicking here *------* ");
    messagef(pallete.green, "TODO (fun): Offset visualization and tweening");



    // messagef(pallete.red, "?TODO: possible to get into a state where Escape no longer cancels Select, Connected (just popups)");


    // messagef(pallete.red, "TODO: rename pallete -> pallete (allow for pallete swaps later)");
    #ifdef SHIP
    // messagef(pallete.light_gray, "press ? for help");
    #endif
    /*
       messagef(pallete.red, "TODO: Intersection snap");
       messagef(pallete.red, "TODO: SHIP should disable all the commands without without without without without without without without buttons");
       messagef(pallete.red, "TODO: Save/Load need buttons");
       messagef(pallete.red, "TODO: Camera clip planes still jacked (including ortho)");
       messagef(pallete.red, "TODO: Camera hotkeys ;, ' need buttons");
       messagef(pallete.red, "TODO: Rezoom camera needs button");
       messagef(pallete.red, "TODO: Beatiful button presses");
       messagef(pallete.red, "TODO: config needs inches vs. mm");
       messagef(pallete.red, "TODO: config needs bool to hide gui");
       messagef(pallete.red, "TODO: Select/deselect snaps");
       messagef(pallete.red, "TODO: Push power fillet to beta");
       messagef(pallete.red, "TODO: Push power offset (shell) to beta");
       messagef(pallete.red, "TODO: Save/Load DXF broken for some arcs if you load\n      and save the dxf in LAYOUT in the middle.");
       */

    auto SEND_DUMMY = [&]() {
        // "process" dummy event to draw popups and buttons
        // NOTE: it's a Key;Hotkey event in order to enter that section of the code
        // FORNOW: buttons drawn on EVERY event (no good; TODO fix later)
        Event dummy = {};
        dummy.type = EventType::Key;
        dummy.key_event.subtype = KeyEventSubtype::Hotkey;
        dummy.key_event.key = DUMMY_HOTKEY;
        history_process_event(dummy);
    };

    void _messages_draw(); // forward declaration

    glfwHideWindow(glfw_window); // to avoid one frame flicker 
    uint64_t frame = 0;
    while (!glfwWindowShouldClose(glfw_window)) {
        if (other.slowmo) SLEEP(100);
        glfwSwapBuffers(glfw_window);
        glFinish(); // 69363856
                    // SLEEP(1);
        glClearColor(pallete.black.x, pallete.black.y, pallete.black.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        eso_size(1.5f);

        other.OpenGL_from_Pixel = window_get_OpenGL_from_Pixel();

        other._please_suppress_drawing_popup_popup = false;
        other._please_suppress_drawing_toolbox = false;

        memset(popup->a_popup_from_this_group_was_already_called_this_frame, 0, sizeof(popup->a_popup_from_this_group_was_already_called_this_frame));

        if (other.stepping_one_frame_while_paused) other.paused = false;
        if (!other.paused) { // update
            { // time_since
              // FORNOW: HAAAAAACK TODO: time frames and actually increment time properly
                real dt = 0.0167f;
                #ifdef OPERATING_SYSTEM_WINDOWS
                dt *= 2;
                #endif
                _for_each_entity_ entity->time_since_is_selected_changed += dt;
                other.time_since_cursor_start += dt;
                other.time_since_successful_feature += dt;
                other.time_since_plane_selected += dt;
                other.time_since_plane_deselected += dt;
                other.time_since_mouse_moved += dt;
                other.time_since_popup_second_click_not_the_same += dt;
                // time_since_successful_feature = 1.0f;

                bool going_inside = 0
                    || ((state_Mesh_command_is_(ExtrudeAdd)) && (popup->extrude_add_in_length > 0.0f))
                    || (state_Mesh_command_is_(ExtrudeCut));
                if (!going_inside) {
                    other.time_since_going_inside = 0.0f;
                } else {
                    other.time_since_going_inside += dt;
                }

                _for_each_entity_ {
                    vec3 target_color = get_color((entity->is_selected) ? ColorCode::Selection : entity->color_code);
                    if (entity->is_selected) target_color = CLAMPED_LERP(3.0f * entity->time_since_is_selected_changed - 0.1f, AVG(pallete.white, target_color), target_color);
                    JUICEIT_EASYTWEEN(&entity->preview_color, target_color, 3.0f);
                }
            }


            { // events
                SEND_DUMMY();
                glfwPollEvents();
                if (raw_event_queue.length) {
                    while (raw_event_queue.length) {
                        RawEvent raw_event = queue_dequeue(&raw_event_queue);
                        Event freshly_baked_event = bake_event(raw_event);
                        history_process_event(freshly_baked_event);
                    }
                }
            }

            _messages_update();
        } else {
            SEND_DUMMY();
            glfwPollEvents();
            ;
        }

        { // draw
            conversation_draw();
            _messages_draw();
        }

        if (frame++ == 1) glfwShowWindow(glfw_window);
        if (other.stepping_one_frame_while_paused) {
            other.stepping_one_frame_while_paused = false;
            other.paused = true;
        }

    }
}

