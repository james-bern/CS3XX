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
