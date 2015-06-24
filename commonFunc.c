#include "commonFunc.h"

long get_file_size(FILE *fp) {
    long size;
    fseek(fp, 0L, 2);
    size = ftell(fp);
    rewind(fp);
    return size;
}

void _init_array(char *buf,int size)
{
    int i;
    for( i = 0; i < size; i++) *buf++ = 0;
    return;
}

