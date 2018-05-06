#include "util.h"

int get_file_size(FILE *file)
{
    fseek(file, 0L, SEEK_END);
    int size = ftell(file);

    rewind(file);

    return size;
}
