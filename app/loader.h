#ifndef EMICRO_LOADER_H
#define EMICRO_LOADER_H

#define EMICRO_LOADER_INSTANCE "emicro_instance_loader"
#define EMICRO_LOADER_PATH "emicro_loader_path"


extern zend_class_entry * emicro_loader_ce;

zval* load_file(char *path);
void register_autoload();


EMICRO_MODULE_D(loader);

#endif