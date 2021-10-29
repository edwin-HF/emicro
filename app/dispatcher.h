#ifndef EMICRO_DISPATCHER_H
#define EMICRO_DISPATCHER_H

extern zend_class_entry * emicro_dispatcher_ce;

#define EMICRO_DISPATCHER_INSTANCE "emicro_dispatcher_instance"
#define EMICRO_DISPATCHER_MOUDLE "emicro_dispatcher_moudle"
#define EMICRO_DISPATCHER_CONTROLLER "emicro_dispatcher_controller"
#define EMICRO_DISPATCHER_METHOD "emicro_dispatcher_method"
#define EMICRO_DISPATCHER_MULTI_MOUDLE "emicro_dispatcher_multi_moudle"

EMICRO_MODULE_D(dispatcher);

void init_dispatcher(zval *router);
void parse_dispatcher(char *path);
void dispatcher(char *controller, char *method, char router_params[10][MAXPATHLEN], int router_params_len);
void init_router_map(char *file, char *class, char *doc_comment);
void annotation_cb_dispatcher_class(char *annotation, char *annotation_param, char *position, void *params);
void annotation_cb_dispatcher_method(char *annotation, char *annotation_param, char *position, void *params);
void ref_class_method_doc_cb(char *class, char *method, char* doc_comment, void *params);


zval* call_dispatcher(char *class, char *method, char router_params[10][MAXPATHLEN], int router_params_len);
void dispatcher_return(zval *retval);
int8_t validate_dispatcher_cache(char *file, int64_t mt);

#endif