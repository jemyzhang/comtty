#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "common.h"

long get_file_size(FILE *fp) {
    long size;
    fseek(fp, 0L, 2);
    size = ftell(fp);
    rewind(fp);
    return size;
}

void MSG_INFO(char *fmt,...)
{
    va_list vptr;
    va_start(vptr, fmt);
    printf("\n\x1b[37;42m");
    if(fmt[strlen(fmt)-1] == '\n')
    {
        char *tmp = strdup(fmt);
        tmp[strlen(fmt)-1] = '\0';
        vprintf(tmp, vptr);
        printf("\x1b[0m\n");
        free(tmp);
    }else{
        vprintf(fmt, vptr);
        printf("\x1b[0m");
    }
}

void MSG_ERR(char *fmt,...)
{
    va_list vptr;
    va_start(vptr, fmt);
    printf("\n\x1b[37;41m");
    if(fmt[strlen(fmt)-1] == '\n')
    {
        char *tmp = strdup(fmt);
        tmp[strlen(fmt)-1] = '\0';
        vprintf(tmp, vptr);
        printf("\x1b[0m\n");
        free(tmp);
    }else{
        vprintf(fmt, vptr);
        printf("\x1b[0m");
    }
}
