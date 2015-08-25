#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"

char *_strim(char *string)
{
    int start = 0;
    int end = 0;
    char buf[1024];
    char *p = buf;
    if(string == NULL) return NULL;
    
    strcpy(buf,string);
    end = strlen(buf) - 1;
    while( *p++ == ' ') start++;

    p = buf + end;
    while( *p-- == ' ' && end !=0) end --;
    
     if(start <= end)
     {
        buf[end+1] = '\0';   
        p = (char *)malloc(strlen(buf) -start + 1);
        strcpy(p,&buf[start]);
     }
     else
     {
        p = (char *)malloc(strlen(buf) + 1);
        strcpy(p,buf);
     }  
    return  p;
}

int _read_line(FILE *fp, char *linetxt)
{
    char buf = 0;
    int cnt = 0;
    while(!feof(fp))
    {
        buf = fgetc(fp);
        if(buf != 0xa && buf != 0xd) break;    
    }
    while(!feof(fp) && buf != 0xa && buf != 0xd)
    {
        *(linetxt + cnt) = buf;
        buf = fgetc(fp);
        cnt ++;   
     }
     if(cnt == 0) return -1;
     return 0;
}

int _analy_line(char *linetxt,CONFIG_t *config)
{
    char *p = linetxt;
    char buf[1024], *pbuf;
    int cnt = 0;
        
    if(*linetxt == '#')
    {
        config->key = (char *)malloc(2);
        config->value = (char *)malloc(strlen(p));
        memset(config->key,0,2);
        memset(config->value,0,strlen(p));
        strcpy(config->key, "#");
        strcpy(config->value,(linetxt + 1));
     }else
     {
        memset(buf,0,sizeof(buf));
        pbuf = buf;
        while( *p != '=')
        {
              *pbuf++ = *p++;
              cnt ++;
        }
        if( cnt == 0) buf[0] = ' ';
        config->key = _strim(buf);
        p++; //=
        memset(buf,0,sizeof(buf));
        pbuf = buf;
        cnt = 0;
        while(*p != '\0')
        {
            if(*p != '"')
            {
                *pbuf++ = *p;
                cnt ++;
            }
            p++;
        }
        if( cnt == 0) buf[0] = ' ';
        config->value = _strim(buf);
     }
    return 0;
}

int load_config(char *file,CONFIG_t *configs, int size)
{
    int err = 0;
    int ret = 0;
    FILE *fp;
    char linetxt[1024];
    CONFIG_t configbuf;
    int cnt;
    
    if((fp = fopen(file,"rb")) == NULL)
    {
        return -1;
     }
        
    for(cnt = 0; cnt < size && !feof(fp); cnt ++)
    {
        memset(linetxt,0,sizeof(linetxt));
        err = _read_line(fp,linetxt);
        if(err == 0)
        {
            err = _analy_line(linetxt,&configbuf);
            if(err == 0)
            {
                (configs + cnt)->key = (char *)malloc(strlen(configbuf.key) + 1);
                (configs + cnt)->value = (char *)malloc(strlen(configbuf.value) + 1);
                memset((configs + cnt)->key,0,strlen(configbuf.key) + 1);
                memset((configs + cnt)->value,0,strlen(configbuf.value)+1);
                strcpy( (configs + cnt)->key, configbuf.key);
                strcpy( (configs + cnt)->value,configbuf.value);
                free(configbuf.key);
                free(configbuf.value);
                ret ++;
            }
        }
    }
    fclose(fp);
    if(cnt < size)
    {
        (configs + cnt)->key = NULL;
     }
    return ret; 
    
}

int reload_config(char *file,CONFIG_t *configs, int size)
{
    int cnt = 0,ret;
    for(;cnt < size; cnt++)
    {
        if((configs+cnt)->key != NULL) free((configs+cnt)->key);    
        if((configs+cnt)->value != NULL) free((configs+cnt)->value);    
    }
    ret = load_config(file,configs,size);
    return ret;
}

int save_config(char *file,CONFIG_t *configs, int size)
{
    FILE *fp;
    int cnt;
    
    if((fp = fopen(file,"wb")) == NULL)
        return -1;
    
    for(cnt = 0; cnt < size; cnt ++)
    {
        if((configs + cnt)->key != NULL)
        {
            if(*((configs + cnt)->key) != '#')
            {
                //printf("%s = \"%s\";\n", (configs + cnt)->key, (configs + cnt)->value);
                fprintf(fp,"%s = \"%s\"\n", (configs + cnt)->key, (configs + cnt)->value);
            }
           else
           {
                //printf("%s%s\n", (configs + cnt)->key, (configs + cnt)->value);
                fprintf(fp,"%s%s\n", (configs + cnt)->key, (configs + cnt)->value);
            }
        }
        else
        {
            break;
        }
    }
    fclose(fp);
    return 0;
}

