#ifndef __GET_KEY_H
#define __GET_KEY_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>

enum {
    KEYESC = 0x1b,
    BKSPC = 8,
    KYTAB = 9,
    KEYUP = -20,
    KEYDW,
    KEYRT,
    KEYLF,
    KEYF1,
    KEYF2,
    KEYF3,
    KEYF4,
    KEYF5,
    KEYF6,
    KEYF7,
    KEYF8,
    KEYF9,
    KEYF10,
    KEYF11,
    KEYF12,
    KEYINS,
    KEYDEL,
};

int read_input_seq(char echo, char flush, char *cbuf, size_t size);
char gen_fake_key(char *code);

int _get_input(char *pdst, char endflag, char sChar, char eChar);
int _get_input_string(char *pdst);
int _get_input_num(void);
void _init_array(char *array,int size);

#endif /*__GET_KEY_H*/
