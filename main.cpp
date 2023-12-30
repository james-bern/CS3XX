#include "cs345.cpp"

void app_A() {
    while (cow_begin_frame()) {
        printf("A\n");
    }
}

int main() {
    APPS {
        APP(app_A);
        APP(app_B);
    }

    return 0;
}
