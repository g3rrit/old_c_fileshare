#include "util.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

int get_file_size(FILE *file)
{
    int size = 0;
#ifdef _WIN32
    if(!GetFileSizeEx(file, &size))
        goto error;
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
