#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "config.h"


const CONFIG_t default_config[] = {
        { "#", "# Config file for comtty[Ver 1.1]" },
        { "#", "# Configuration of com-port" },
        { "PortName", "/dev/ttyS0" },
        { "Baudrate", "115200" },
        { "DataBits", "8" },
        { "StopBit", "1" },
        { "Parity", "N" },
        { "#", "# Configuration of key shortcut" },
        { "#", "# \"\\n\" -- 0x0d" },
        { "#", "# \"#!-\" sleep for 10ms" },
        { "#", "# \"#!|\" sleep for 1s" },
        { "#", "# \"#!~\" sleep for 5s" },
        { "#", "# \"#!!\"sleep for 10s" },
        { "F1", "\\n" },
        { "F2", "\\n" },
        { "F3", "\\n" },
        { "F4", "\\n" },
        { "F5", "\\n" },
        { "F6", "\\n" },
        { "F7", "\\n" },
        { "F8", "\\n" },
        { "F9", "\\n" },
        { "F10", "\\n" },
        { "F11", "\\n" },
        { "F12", "\\n" },
#if 0
{ "UP", "2-4-\\n" },
    { "DOWN", "2-5-\\n" },
    { "LEFT", "2-7-\\n" },
    { "RIGHT", "2-6-\\n" },
#endif
        { "#", "# end of configure file" },
        {NULL,NULL},
};


static char* trim(char *s)
{
    int len = strlen(s);
    while(isspace(s[len-1])) --len;
    while(*s && isspace(*s)) ++s, --len;
    return strndup(s,len);
}

static int parseline(char *s, CONFIG_t *config)
{
    if(*s == '#')
    {
        config->key = strdup("#");
        config->value = strdup(s +1);
    }else {
        char* token = strtok(s, "=");
        if(token == NULL) return -1;
        config->key = trim(token);
        char *v = trim(strtok(NULL,"="));
        //retrieve content inside the quotation marks
        token = strtok(v, "\"");
        config->value = trim(token);
        free(v);
    }
    return 0;
}

int load_config(const char *file,CONFIG_t *configs, int size)
{
    FILE *fp;
    char sbuf[1024];
    int cnt = 0;

    if((fp = fopen(file,"rb")) == NULL)
    {
        return -1;
    }

    while(cnt < size && fgets(sbuf, sizeof(sbuf), fp))
    {
        if(parseline(sbuf, configs+cnt) == 0)
        {
            cnt++;
        }
    }
    fclose(fp);

    if(cnt < size)
    {
        configs[cnt].key = NULL;
    }

    return cnt;

}

int reload_config(const char *file,CONFIG_t *configs, int size)
{
    int cnt;
    for(cnt = 0;cnt < size; cnt++)
    {
        if(configs[cnt].key != NULL)
        {
            free(configs[cnt].key);
            configs[cnt].key = NULL;
        }
        if(configs[cnt].value != NULL){
            free(configs[cnt].value);
            configs[cnt].value = NULL;
        }
    }
    return load_config(file,configs,size);
}

int save_config(const char *file,CONFIG_t *configs, int size)
{
    FILE *fp;
    int cnt;

    if((fp = fopen(file,"wb")) == NULL)
        return -1;

    for(cnt = 0; cnt < size && configs[cnt].key != NULL; cnt ++)
    {
        if(*(configs[cnt].key) != '#')
        {
            fprintf(fp,"%s = \"%s\"\n", configs[cnt].key, configs[cnt].value);
        }
        else
        {
            fprintf(fp,"%s%s\n", configs[cnt].key, configs[cnt].value);
        }
    }
    fclose(fp);
    return 0;
}

int config_create_default(const char* file, CONFIG_t *configs, int size)
{
    int ret = 0,cnt = 0;
    while(default_config[cnt].key != NULL && cnt < size)
    {
        configs[cnt].key = strdup(default_config[cnt].key);
        configs[cnt].value = strdup(default_config[cnt].value);
        cnt ++;
    }
    ret = save_config(file,configs,cnt);
    return ret;
}

char *config_getvalue(char *key, CONFIG_t *configs)
{
    int i = 0;
    while(configs[i].key != NULL)
    {
        if(strcmp(configs[i].key,key) == 0)
        {
            return configs[i].value;
        }
        i ++;
    }
    return NULL;
}


