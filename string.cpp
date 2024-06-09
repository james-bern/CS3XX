struct String {
    char *data;
    uint length;
};

bool string_pointer_is_valid(String string, char *pointer) {
    return (string.data <= pointer) && (pointer < string.data + string.length);
}

#define STRING_FROM_CSTRING_LITERAL(cstring_literal) { (char *)(cstring_literal), uint(strlen(cstring_literal)) }
