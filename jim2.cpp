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
