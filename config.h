#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdlib.h>
#include <stdio.h>

typedef struct{
    char *key;
    char *value;
}CONFIG_t;

int load_config(char *file,CONFIG_t *configs, int size);
int reload_config(char *file,CONFIG_t *configs, int size);
int save_config(char *file,CONFIG_t *configs, int size);


#endif /*_CONFIG_H*/