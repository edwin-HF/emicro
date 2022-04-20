#ifndef EMICRO_APPLICATION_H
#define EMICRO_APPLICATION_H

#define EMICRO_APPLICATION_APP_PATH "emicro_application_app_path"

extern zend_class_entry * emicro_application_ce;

zval* load(char *path);
void init_annotation();
void scan_cb_config(char* file);

EMICRO_MODULE_D(application);

#endif