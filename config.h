#ifndef _CONFIG_H
#define _CONFIG_H

typedef struct{
    char *key;
    char *value;
}CONFIG_t;

int load_config(const char *file,CONFIG_t *configs, int size);
int reload_config(const char *file,CONFIG_t *configs, int size);
int save_config(const char *file,CONFIG_t *configs, int size);
int config_create_default(const char* file, CONFIG_t *configs, int size);
char *config_getvalue(char *key, CONFIG_t *configs);


#endif /*_CONFIG_H*/