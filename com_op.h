#ifndef _COM_OP_H
#define _COM_OP_H

#define FALSE  0
#define TRUE  1

typedef struct {
    char portname[256];
    int baudrate;
    int databits;
    int stopbit;
    int parity;
}COM_CONFIG_t;

int setup_serialport(int fd, int speed, int databits,int stopbits,int parity);
int sendchar(int device, char cmd);
int sendcmds(int device, char *cmd);
int sendbytes(int device, char *pbytes, int size);
int readbytes(int device, char *pool, int size);
int sendfile(char *file,int device,int packsize);

#endif /*_COM_OP_H*/
