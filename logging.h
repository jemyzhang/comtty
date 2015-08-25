#ifndef _LOGGING_H
#define _LOGGING_H

#include "common.h"

int create_log(char *logpath);
//int put_log(char *logpath, char *log, int size);
void log_to_file(CTRL_INFO_t *pctrl, int pipe[2]);
#endif /*_LOGGING_H*/