#ifndef EMICRO_APPLICATION_H
#define EMICRO_APPLICATION_H

#define EMICRO_APPLICATION_INSTANCE "emicro_instance_application"

extern zend_class_entry * emicro_application_ce;

int load(char *path);

EMICRO_MODULE_D(application);

#endif