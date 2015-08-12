#include <string.h>
#include "com_op.h"

int speed_arr[] = { B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300,};
int name_arr[] = { 115200, 57600, 38400,  19200,  9600,  4800,  2400,  1200,  300};

int sendchar(int device, char cmd)
{
     int ret = 0;
     ret = write(device,&cmd,sizeof(cmd));
     return ret;
}

int sendbytes(int device, char *pbytes, int size)
{
    int ret = 0;
    ret = write(device,pbytes,size);
    return ret;
}

int readbytes(int device, char *pool, int size)
{
    int ret = 0;
    ret = read(device,pool,size);
    return ret;
}


int sendcmds(int device, char *cmds)
{
    int ret = 0;
    int i;
    if ( cmds == NULL) return -1;
    for(i = 0; i < strlen(cmds); i ++)
    {
        char c = cmds[i];
        switch(c)
        {
            case '#':
                if(cmds[i+1] != '\0')
                {
                    i++;
                    char c1 = cmds[i];
                    if(c1 == '!')
                    {
                        if(cmds[i+1] != '\0')
                        {
                            i++;
                            char c2 = cmds[i];
                            switch(c2)
                            {
                                 case '-':
                                     usleep(10000);
                                     break;
                                 case '|':
                                     sleep(1);
                                     break;
                                 case '~':
                                     sleep(5);
                                     break;
                                 case '!':
                                     sleep(10);
                                     break;
                                default:
                                    sendchar(device, c);
                                    sendchar(device, c1);
                                    sendchar(device, c2);
                                    break;
                            }
                        }else{
                            sendchar(device, c);
                            sendchar(device, c1);
                        }
                    }else{
                        sendchar(device, c);
                        sendchar(device, c1);
                    }
                }else{
                    sendchar(device, c);
                }
                break;
            case '\n':
                ret = sendchar(device, 0x0d);
                break;
            case '\\':
                if( i <  strlen(cmds) -1)
                {
                    i ++;
                    switch(*(cmds + i))
                    {
                        case 'n':
                            ret = sendchar(device, 0x0d);
                            break;
                        default:
                            ret = sendchar(device, *(cmds + i));
                            break;
                     }
                 }
                break;
            default:
                ret = sendchar(device, c);
                usleep(1000);
                if(ret != 1) {
                    //send again
                    ret = sendchar(device, c);
                }
                break;
        }
    }
    return ret;  
}

int sendfile(char *file,int device,int packsize)
{
    FILE *fp;
    unsigned int filesize,sentcnt = 0;
    int sendpersent = 0;
    
    char sendbuf[packsize];
    int getsize = packsize;
    int sentsize = packsize;
    time_t start_tm = 0, end_tm = 0;
    
    _init_array(sendbuf,sizeof(sendbuf));
    
    if((fp = fopen(file,"rb")) == NULL)
    {
        return -1;
    }
    filesize = get_file_size(fp);
    if(filesize == 0) return -1;
    if(filesize < packsize) {
        sentsize = filesize;
        getsize = sentsize + 1;
        }
    
    printf("\nSending...\33[s");
    time(&start_tm);
    while(!feof(fp))
    {
        fread(&sendbuf,getsize,1,fp);
        sendbytes(device,sendbuf,sentsize);
        sentcnt += sentsize;
        sendpersent = sentcnt*100/filesize;
        if( (filesize - sentcnt) < packsize )
        {
            sentsize = filesize - sentcnt;
            getsize = sentsize + 1;
        }else
        {
            sentsize = packsize;
            getsize = sentsize;
        }
        _init_array(sendbuf,sizeof(sendbuf));
        printf("\33[u\33[k%d bytes [%d%%]",sentcnt,sendpersent);
        fflush(stdout);
     }
     time(&end_tm);
     printf("[SPD:%.0fs]\n",
     difftime(end_tm,start_tm));
     return 0;
}

/**
*@brief   设置串口数据位，停止位和效验位
*@param  fd     类型  int  打开的串口文件句柄
*@param  databits 类型  int 数据位   取值 为 7 或者8
*@param  stopbits 类型  int 停止位   取值为 1 或者2
*@param  parity  类型  int  效验类型 取值为N,E,O,,S
*/
int setup_serialport(int fd, int speed, int databits,int stopbits,int parity)
{ 
    int i;
    struct termios options;
    if(tcgetattr(fd, &options) != 0)
    {
        printf("failed to get tc attr\n");
        return -1;
    }

    //set to raw mode
    cfmakeraw(&options);

    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    { 
        if  (speed == name_arr[i])
        { 
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
            break;
        }
    }
    if(i >= sizeof(speed_arr)/sizeof(speed_arr[0]))
    {
        printf("Do not support Baudrate %d\n",speed);
        return -1;
    }

    options.c_cflag &= ~CSIZE; 
    switch (databits) /*设置数据位数*/
    {   
        case 7:     
            options.c_cflag |= CS7; 
            break;
        case 8:     
            options.c_cflag |= CS8;
            break;   
        default:    
            printf("Unsupported data bits\n");
            return -1;
    }
    switch (parity) 
    {   
        case 'n':
        case 'N':    
            options.c_cflag &= ~PARENB;   /* Clear parity enable */
            options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
            break;  
        case 'o':   
        case 'O':     
            options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/  
            options.c_iflag |= INPCK;             /* Disnable parity checking */ 
            break;  
        case 'e':  
        case 'E':   
            options.c_cflag |= PARENB;     /* Enable parity */    
            options.c_cflag &= ~PARODD;   /* 转换为偶效验*/     
            options.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
        case 'S': 
        case 's':  /*as no parity*/   
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;  
        default:   
            printf("Unsupported parity\n");    
            return -1;
    }  
    /* Set input parity option */ 
    if (parity != 'n')   
    {
        options.c_iflag |= INPCK; 
    }
    /* 设置停止位*/  
    switch (stopbits)
    {   
        case 1:    
            options.c_cflag &= ~CSTOPB;  
            break;  
        case 2:    
            options.c_cflag |= CSTOPB;  
            break;
        default:    
            printf("Unsupported stop bits\n");  
            return -1;
    } 

    tcflush(fd,TCIOFLUSH);
    tcflush(fd,TCIFLUSH);

    options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/   
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */

    if (tcsetattr(fd,TCSANOW,&options) != 0)
    { 
        printf("failed setup serial port\n");   
        return -1;
    } 

    return 0;
}

