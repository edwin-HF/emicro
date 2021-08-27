#ifndef EMICRO_DISPATCHER_H
#define EMICRO_DISPATCHER_H

extern zend_class_entry * emicro_dispatcher_ce;

#define EMICRO_DISPATCHER_INSTANCE "emicro_dispatcher_instance"
#define EMICRO_DISPATCHER_MOUDLE "emicro_dispatcher_moudle"
#define EMICRO_DISPATCHER_CONTROLLER "emicro_dispatcher_controller"
#define EMICRO_DISPATCHER_METHOD "emicro_dispatcher_method"
#define EMICRO_DISPATCHER_MULTI_MOUDLE "emicro_dispatcher_multi_moudle"

EMICRO_MODULE_D(dispatcher);

#endif