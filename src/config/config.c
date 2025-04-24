#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

char* load_conninfo_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;
    static char buffer[512];
    if (fgets(buffer, sizeof(buffer), file) == NULL) {
        fclose(file);
        return NULL;
    }
    fclose(file);
    buffer[strcspn(buffer, "\n")] = 0;
    return buffer;
}