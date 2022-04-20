#ifndef EMICRO_CONFIG_H
#define EMICRO_CONFIG_H

#define EMICRO_CONFIG_PATH "emicro_config_path"

extern zend_class_entry * emicro_config_ce;

void scan_config(char* path);
zval* config_find(char* key);
int8_t validate_config_cache(char *file,int64_t mt);

EMICRO_MODULE_D(config);

#endif