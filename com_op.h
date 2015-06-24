#ifndef _COM_OP_H
#define _COM_OP_H

#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include "commonFunc.h"

#define FALSE  0
#define TRUE  1

typedef struct {
    int baudrate;
    int databits;
    int stopbit;
    int parity;
}COM_CONFIG_t;

int set_speed(int fd, int speed);
int set_Parity(int fd,int databits,int stopbits,int parity);
int OpenDev(char *Dev);
int sendcmd(int device, char cmd);
int sendcmds(int device, char *cmd);
int sendbytes(int device, char *pbytes, int size);
int recvmsg(int device, char *pool,int size);
int sendfile(char *file,int device,int packsize);

#endif /*_COM_OP_H*/
