#ifndef _COMMONFUNC_H
#define _COMMONFUNC_H

#include <stdio.h>

typedef struct {
    char sig_term;
    char sig_blockoutput;
    char log_switch;
    char log_path[1024];
} CTRL_INFO_t;

long get_file_size(FILE *fp);
void _init_array(char *buf,int size);
char *_strim(char *string);

#endif /*_COMMONFUNC_H*/