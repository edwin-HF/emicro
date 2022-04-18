#ifndef EMICRO_APPLICATION_H
#define EMICRO_APPLICATION_H

#define EMICRO_APPLICATION_INSTANCE "emicro_instance_application"
#define EMICRO_APPLICATION_DISPATCHER_NAMESPACE "emicro_application_dispatcher_namespace"
#define EMICRO_APPLICATION_ANNOTATION_NAMESPACE "emicro_application_annotation_namespace"
#define EMICRO_APPLICATION_APP_PATH "emicro_application_app_path"

extern zend_class_entry * emicro_application_ce;

zval* load(char *path);
void init_config();
void init_annotation();
void init_autoload();
int8_t validate_config_cache(char *file,int64_t mt);
void scan_cb_config(char* file);

EMICRO_MODULE_D(application);

#endif