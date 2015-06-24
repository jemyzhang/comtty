#ifndef _LOGGING_H
#define _LOGGING_H
#include <stdlib.h>
#include <stdio.h>

int create_log(char *logpath);
int put_log(char *logpath, char *log, int size);

#endif /*_LOGGING_H*/