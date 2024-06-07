#ifndef JIM_CPP
#define JIM_CPP
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

// FORNOW: cstring stuff
bool poe_prefix_match(char *string, char *prefix) {
    while (*string == ' ') ++string; // ?
    while (*prefix == ' ') ++prefix; // ?
    size_t strlen_string = strlen(string);
    size_t strlen_prefix = strlen(prefix);
    if (strlen_string < strlen_prefix) return false;
    for (unsigned int i = 0; i < strlen_prefix; ++i) {
        if (string[i] != prefix[i]) return false;
    }
    return true;
}

bool poe_suffix_match(char *string, char *suffix) {
    size_t strlen_string = strlen(string);
    size_t strlen_suffix = strlen(suffix);
    if (strlen_string < strlen_suffix) return false;
    for (unsigned int i = 0; i < strlen_suffix; ++i) {
        if (string[strlen_string - 1 - i] != suffix[strlen_suffix - 1 - i]) return false;
    }
    return true;
}

bool poe_file_exists(char *filename) {
    FILE *file = (FILE *) fopen(filename, "r");
    if (!file) {
        return false;
    }
    fclose(file);
    return true;
}



// https://en.cppreference.com/w/c/algorithm/qsort
void jim_sort_against(void *base, uint nitems, int size, real *corresp_values_to_sort_against, bool sort_both_arrays = false) {
    struct qsortHelperStruct {
        uint index;
        real value;
    };
    qsortHelperStruct *helperArray = (qsortHelperStruct *) calloc(sizeof(qsortHelperStruct), nitems); {
        for_(i, nitems) helperArray[i] = { i, corresp_values_to_sort_against[i] };
    }

    int(* comp)(qsortHelperStruct *, qsortHelperStruct *) \
        = [](qsortHelperStruct *a, qsortHelperStruct *b) -> int { return (a->value < b->value) ? -1 : 1; }; 

    qsort(helperArray, nitems, sizeof(qsortHelperStruct), (int (*)(const void *, const void *))comp);

    {
        void *tmp_buffer = malloc(nitems * size); { // fornow
            for_(i, nitems) memcpy(\
                    ((char *) tmp_buffer) + (i * size), \
                    ((char *) base) + (helperArray[i].index * size), \
                    size);
            memcpy(base, tmp_buffer, nitems * size);
        } free(tmp_buffer);
    }

    if (sort_both_arrays) {
        real *tmp_buffer = (real *) malloc(nitems * sizeof(real)); {
            for_(i, nitems) tmp_buffer[i] = corresp_values_to_sort_against[helperArray[i].index];
            memcpy(corresp_values_to_sort_against, tmp_buffer, nitems * sizeof(real));
        } free(tmp_buffer);
    }
}


// if file has been modified (more than a second since the last modification)
// returns the result of fopen(filename, mode)
// otherwise returns NULL
#include <sys/types.h>
#include <sys/stat.h>
#ifdef OPERATING_SYSTEM_WINDOWS
#define stat _stat
#else
#include <unistd.h>
#endif
FILE *jim_hot_fopen(char *filename, char *mode = "r") {
    struct Stamp {
        char filename[128];
        time_t mod_time;
    };

    static unsigned int num_stamps;
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
        ASSERT(num_stamps < ARRAY_LENGTH(stamps));
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




// ohno
// #define _UNIQUE_ISH_VARIABLE_NAME CONCAT(_VAR_, __COUNTER__)
// #define BEGIN_PRE_MAIN static int _UNIQUE_ISH_VARIABLE_NAME = []() {
// #define END_PRE_MAIN return 0; }();

#endif
