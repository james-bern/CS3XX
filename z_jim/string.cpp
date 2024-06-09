struct String {
    uint32 length;
    char *array;
};

String string_from_cstring(char *cstring) {
    String result = {};
    result.length = strlen(cstring);
    result.array = cstring;
    return result;
}
