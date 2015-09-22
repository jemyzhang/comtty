#ifndef _COMMONFUNC_H
#define _COMMONFUNC_H

#include <stdio.h>

void MSG_INFO(char *fmt,...);
void MSG_ERR(char *fmt,...);

typedef struct {
    int  device;
    char sig_term;
    char sig_blockoutput;
    char sig_timestamp;
    char log_switch;
    char log_path[1024];
} CTRL_INFO_t;

long get_file_size(FILE *fp);

#endif /*_COMMONFUNC_H*/