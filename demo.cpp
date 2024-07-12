#include "playground.cpp"
#include "easy_mode.cpp"
int RANDOM_ENTRY() {
    return int(-100 + 200 * real(rand()) / RAND_MAX);
}
int main() {
    FILE *file = fopen("demo.txt", "w");
    #define NUMBER_OF_2X2_SYSTEMS_TO_GENERATE_AND_SOLVE 1000000
    mat2 A;
    vec2 b;
    vec2 x;
    for_(_, NUMBER_OF_2X2_SYSTEMS_TO_GENERATE_AND_SOLVE) {
        A.data[0] = RANDOM_ENTRY();
        A.data[1] = RANDOM_ENTRY();
        A.data[2] = RANDOM_ENTRY();
        A.data[3] = RANDOM_ENTRY();
        b.x = RANDOM_ENTRY();
        b.y = RANDOM_ENTRY();
        x = inverse(A) * b;
        fprintf(file, "the solution to [%gx + %gy = %g\n", A.data[0], A.data[1], b.x);
        fprintf(file, "                [%gx + %gy = %g\n", A.data[2], A.data[3], b.y);
        fprintf(file, "is (x, y) = (%f %f)\n\n", x.x, x.y);

    }
    fclose(file);
}
