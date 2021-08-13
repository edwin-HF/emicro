#ifndef EMICRO_APPLICATION_H
#define EMICRO_APPLICATION_H

#define EMICRO_APPLICATION_INSTANCE "emicro_instance_application"
#define EMICRO_APPLICATION_DISPATCHER_NAMESPACE "emicro_application_dispatcher_namespace"
#define EMICRO_APPLICATION_ANNOTATION_NAMESPACE "emicro_application_annotation_namespace"

extern zend_class_entry * emicro_application_ce;

int load(char *path);
char* app_path();
char* replace(zval* str, char *find, char *replace);
static void dispatcher();

EMICRO_MODULE_D(application);

#endif