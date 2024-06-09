// // types
// basic types
typedef uint32_t uint;
typedef float real;
typedef uint8_t u8;
// vecD
template <uint D> union Vector;
typedef Vector<2> vec2;
typedef Vector<3> vec3;
typedef Vector<4> vec4;
#define vecD Vector<D>
#define tuD  template <uint D>
#define tuDv tuD vecD
// matD
template <uint D> union Matrix;
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
#define defer auto CONCAT(defer_, __COUNTER) = defer_dummy() + [&]()
// run_before_main
// - code inside run_before_main { ... }; runs before main
struct run_before_main_dummy {};
template <typename F> bool operator + (run_before_main_dummy, F &&f) { f(); return true; }
#define run_before_main static bool CONCAT(run_before_main_, __COUNTER) = run_before_main_dummy() + []()

// // math
// constants
#define TINY_VAL real(1e-6)
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
// SGN
int SGN(  int a) { return (a < 0) ? -1 : 1; }
int SGN(float a) { return (a < 0) ? -1 : 1; }
// ABS
int  ABS( int a) { return (a < 0) ? -a : a; }
real ABS(real a) { return (a < 0) ? -a : a; }
// MIN
int  MIN( int a,  int b) { return (a < b) ? a : b; }
uint MIN(uint a, uint b) { return (a < b) ? a : b; }
real MIN(real a, real b) { return (a < b) ? a : b; }
// MAX
int  MAX( int a,  int b) { return (a > b) ? a : b; }
uint MAX(uint a, uint b) { return (a > b) ? a : b; }
real MAX(real a, real b) { return (a > b) ? a : b; }
// floating-poiut comparisons
bool IS_ZERO(real a) { return (ABS(a) < TINY_VAL); }
tuD bool IS_ZERO(vecD a) { for_(d, D) if (!IS_ZERO(a[d])) return false; return true; }
bool ARE_EQUAL(real a, real b) { return IS_ZERO(ABS(a - b)); }
bool IS_BETWEEN(real p, real a, real b) { return (((a - TINY_VAL) < p) && (p < (b + TINY_VAL))); }
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

// // cstring
bool MATCHES_PREFIX(char *cstring, char *prefix) {
    while (*cstring == ' ') ++cstring; // ?
    while (*prefix == ' ') ++prefix; // ?
    size_t strlen_string = strlen(cstring);
    size_t strlen_prefix = strlen(prefix);
    if (strlen_string < strlen_prefix) return false;
    for (unsigned int i = 0; i < strlen_prefix; ++i) {
        if (cstring[i] != prefix[i]) return false;
    }
    return true;
}
//
bool MATCHES_SUFFIX(char *cstring, char *suffix) {
    size_t strlen_string = strlen(cstring);
    size_t strlen_suffix = strlen(suffix);
    if (strlen_string < strlen_suffix) return false;
    for (unsigned int i = 0; i < strlen_suffix; ++i) {
        if (cstring[strlen_string - 1 - i] != suffix[strlen_suffix - 1 - i]) return false;
    }
    return true;
}

// // file-handling
bool FILE_EXISTS(char *filename) {
    FILE *file = (FILE *) fopen(filename, "r");
    if (!file) {
        return false;
    }
    fclose(file);
    return true;
}
