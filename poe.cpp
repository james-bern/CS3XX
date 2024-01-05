bool poe_matches_prefix(char *string, char *prefix) {
    while (*string == ' ') ++string;
    while (*prefix == ' ') ++prefix;
    if (strlen(string) < strlen(prefix)) return false;
    for_(i, (int) strlen(prefix)) {
        if (string[i] != prefix[i]) return false;
    }
    return true;
}
