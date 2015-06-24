#ifndef _PIPE_OP_H
#define _PIPE_OP_H
#include <stdio.h>
#include "commonFunc.h"

typedef struct {
    char sig_term;
    char sig_cmdmode;
    char log_switch;
    char *log_path;
}PIPE_INFO_t;

int pipe_read(char *pipe,PIPE_INFO_t *pipe_info);
int pipe_write(char *pipe,PIPE_INFO_t *pipe_info);
int pipe_size(char *pipe);

#endif /*_PIPE_OP_H*/