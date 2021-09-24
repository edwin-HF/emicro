#ifndef EMICRO_FACTORY_H
#define EMICRO_FACTORY_H

extern zend_class_entry * emicro_factory_ce;

EMICRO_MODULE_D(factory);

zval* call_method(char *class, char *method, zval *params);

#endif