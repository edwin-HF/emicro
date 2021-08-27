#ifndef EMICRO_APPLICATION_H
#define EMICRO_APPLICATION_H

#define EMICRO_APPLICATION_INSTANCE "emicro_instance_application"
#define EMICRO_APPLICATION_DISPATCHER_NAMESPACE "emicro_application_dispatcher_namespace"
#define EMICRO_APPLICATION_ANNOTATION_NAMESPACE "emicro_application_annotation_namespace"
#define EMICRO_APPLICATION_APP_PATH "emicro_application_app_path"

extern zend_class_entry * emicro_application_ce;

char* root_path();
char* app_path();
zval* load(char *path);
static void dispatcher();
void annotation_dispatcher_callback(char *file);
void parse_annotation_dispatcher_callback(char *annotation, char *annotation_param, void *params);
void parse_annotation_method_callback(char *annotation, char *annotation_param, void *params);

EMICRO_MODULE_D(application);

#endif