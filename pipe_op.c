#include <string.h>
#include "pipe_op.h"

int pipe_read(char *pipe,PIPE_INFO_t *pipe_info)
{
    FILE *fpipe;
    if ((fpipe = fopen(pipe,"rb")) == NULL)
    {
        return -1;
    }else
    {
        fread(&(pipe_info->sig_term),sizeof(pipe_info->sig_term),1,fpipe);
        fread(&(pipe_info->sig_cmdmode),sizeof(pipe_info->sig_cmdmode),1,fpipe);
        fread(&(pipe_info->log_switch),sizeof(pipe_info->log_switch),1,fpipe);
        fread(pipe_info->log_path,pipe_size(pipe) - 2,1,fpipe);
        fclose(fpipe);
    } 
    return 0; 

}

int pipe_write(char *pipe,PIPE_INFO_t *pipe_info)
{
    FILE *fpipe;
    if ((fpipe = fopen(pipe,"wb")) == NULL)
    {
        return -1;
    }else
    {
        fwrite(&(pipe_info->sig_term),sizeof(pipe_info->sig_term),1,fpipe);
        fwrite(&(pipe_info->sig_cmdmode),sizeof(pipe_info->sig_cmdmode),1,fpipe);
        fwrite(&(pipe_info->log_switch),sizeof(pipe_info->log_switch),1,fpipe);
        if(pipe_info->log_path != NULL)
            fwrite(pipe_info->log_path,strlen(pipe_info->log_path),1,fpipe);
        fclose(fpipe);
    } 
    return 0; 
}

int pipe_size(char *pipe)
{
    FILE *fpipe;
    int size;
    
    if ((fpipe = fopen(pipe,"rb")) == NULL)
    {
        return -1;
    }else
    {
        size = get_file_size(fpipe);
        fclose(fpipe);
    }
        return size;     
}
