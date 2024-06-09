struct String {
    char *data;
    uint length;
};

bool string_pointer_is_valid(String string, char *pointer) {
    return (string.data <= pointer) && (pointer < string.data + string.length);
}

#define STRING(cstring_literal) { (char *)(cstring_literal), uint(strlen(cstring_literal)) }

#define STRING_STRUCT_CALLOC(name, length) String name = { (char *) calloc(1, length) }

String _string_from_cstring(char *cstring) {
    return { (char *)(cstring), uint(strlen(cstring)) };
}


bool string_matches_prefix(String string, String prefix) {
    if (string.length < prefix.length) return false;
    return (memcmp(string.data, prefix.data, prefix.length) == 0);
}

bool string_matches_suffix(String string, String suffix) {
    if (string.length < suffix.length) return false;
    return (memcmp(&string.data[string.length - suffix.length], &suffix.data[suffix.length - suffix.length], suffix.length) == 0);
}

real strtof(String string) {
    // FORNOW
    static char cstring[4096];
    memset(cstring, 0, sizeof(cstring));
    memcpy(cstring, string.data, string.length);
    return strtof(cstring, NULL);
}

bool FILE_EXISTS(String filename) {
    // cstring
    static char cstring[4096];
    memset(cstring, 0, sizeof(cstring));
    memcpy(cstring, filename.data, filename.length);
    return _FILE_EXISTS(cstring);
}

FILE *FILE_OPEN(String filename, char *code) {
    ASSERT(FILE_EXISTS(filename));

    // FORNOW
    static char FORNOW[4096];
    memset(FORNOW, 0, sizeof(FORNOW));
    memcpy(FORNOW, filename.data, filename.length);
    return fopen(FORNOW, code);
}

