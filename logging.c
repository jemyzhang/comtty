#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "logging.h"
#include "common.h"

int create_log(char *logpath)
{
     if((fopen(logpath, "wb")) == NULL)
    {
        printf("Resource %s busy\n",logpath);
        return -1;
    }
    return 0;
}

static int put_log(char *logpath, char *log, int size)
{
    FILE *flog;
    if ((flog = fopen(logpath,"a")) == NULL)
    {
        return -1;
    }else
    {
        fwrite(log,size,1,flog);
        fclose(flog);
    }
    return 0;
}

#define LOG_BLOCK_SIZE 64*1024
void log_to_file(CTRL_INFO_t *pctrl, int pipe[2])
{
    char *pbuf = (char *)malloc(LOG_BLOCK_SIZE);

    int flags = fcntl(pipe[0], F_GETFL);
    fcntl(pipe[0], F_SETFL, flags | O_NONBLOCK);
    close(pipe[1]); //close write

    while(1)
    {
        int rdsize;
        while((rdsize = read(pipe[0], pbuf, LOG_BLOCK_SIZE)) > 0)
        {
            if(pctrl->log_switch)
            {
                put_log(pctrl->log_path, pbuf, rdsize);
            }
        }
        if(pctrl->sig_term)
        {
            break;
        }
        sleep(1);
    }

    close(pipe[0]);

    free(pbuf);

}
