#include "util.h"

#include <stdint.h>
#include <stdio.h>

#ifndef _WIN32
#include <sys/stat.h>
#endif

uint64_t get_file_size(FILE *file)
{
    uint64_t size = 0;
#ifdef _WIN32
    for(char buf; !feof(file); size += fread(&buf, 1, 1, file));
    rewind(file);
#else
    int fd = fileno(file);
    
    struct stat st;
    if(fstat(fd, &st) == -1)
        goto error;

    size = st.st_size;
#endif

    return size;

error:
    printf("error getting file size\n");
    return -1;
}
