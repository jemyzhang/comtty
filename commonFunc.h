#ifndef _COMMONFUNC_H
#define _COMMONFUNC_H
#include <stdlib.h>
#include <stdio.h>

long get_file_size(FILE *fp);
void _init_array(char *buf,int size);
char *_strim(char *string);

#endif /*_COMMONFUNC_H*/