#ifndef EMICRO_DISPATCHER_H
#define EMICRO_DISPATCHER_H

extern zend_class_entry * emicro_dispatcher_ce;

#define EMICRO_DISPATCHER_INSTANCE "emicro_dispatcher_instance"
#define EMICRO_DISPATCHER_MOUDLE "emicro_dispatcher_moudle"
#define EMICRO_DISPATCHER_CONTROLLER "emicro_dispatcher_controller"
#define EMICRO_DISPATCHER_METHOD "emicro_dispatcher_method"
#define EMICRO_DISPATCHER_MULTI_MOUDLE "emicro_dispatcher_multi_moudle"

EMICRO_MODULE_D(dispatcher);

void dispatcher();
void init_router_map();
void scan_cb_dispatcher(char *file);
void annotation_cb_dispatcher_class(char *annotation, char *annotation_param, char *position, void *params);
void annotation_cb_dispatcher_method(char *annotation, char *annotation_param, char *position, void *params);
void annotation_cb_dispatcher(char *annotation, char *annotation_param, char *position, void *params);

zval* call_dispatcher(char *class, char *method);
void dispatcher_return(zval *retval);
int8_t validate_dispatcher_cache(char *file, int64_t mt);

#endif