#include "logging.h"

int create_log(char *logpath)
{
     if((fopen(logpath, "wb")) == NULL)
    {
        printf("Resource %s busy\n",logpath);
        return -1;
    }
    return 0;
}

int put_log(char *logpath, char *log, int size)
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
