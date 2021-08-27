#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "../php_emicro.h"
#include "dispatcher.h"

zend_class_entry * emicro_dispatcher_ce;

ZEND_BEGIN_ARG_INFO(arginfo_emicro_sayHello, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(emicro_dispatcher, __construct){

	zval* router = dispatchParams(TRACK_VARS_GET,"s");
    char* str_router = Z_STRVAL_P(router);
    char* delimiter = "/";
    char* split_save_pointer;
    char* split_str;
    char* controller = NULL;
    char* method = NULL;
    while (NULL != (split_str = php_strtok_r(str_router,delimiter,&split_save_pointer)))
    {
        str_router = NULL;

        if (controller == NULL)
        {
            controller = split_str;
            continue;
        }

        if (method == NULL)
        {
            method = (split_str);
            break;
        }
        
    }

}

PHP_METHOD(emicro_dispatcher, moudle){

}

PHP_METHOD(emicro_dispatcher, controller){

}

PHP_METHOD(emicro_dispatcher, method){

}

PHP_METHOD(emicro_dispatcher, defaultNamespace){

}

PHP_METHOD(emicro_dispatcher, annotationNamespace){

}


PHP_METHOD(emicro_dispatcher, getInstance){
    ZEND_PARSE_PARAMETERS_NONE();

    zval *instance = zend_read_static_property(emicro_dispatcher_ce, ZEND_STRL(EMICRO_DISPATCHER_INSTANCE), 1);

    if (Z_TYPE_P(instance) == IS_OBJECT)
    {
        RETURN_ZVAL(instance,1,0);
    }

    if (object_init_ex(instance,emicro_dispatcher_ce) == FAILURE)
    {
	    php_printf("err obtain application instance\n");
        return;
    }

	zval func_construct;
	ZVAL_STRING(&func_construct, "__construct");
    call_user_function(NULL,instance,&func_construct,NULL,0,NULL);

	zend_update_static_property(emicro_dispatcher_ce, ZEND_STRL(EMICRO_DISPATCHER_INSTANCE), instance);

    RETURN_ZVAL(instance,1,0);
}

zend_function_entry emicro_dispatcher_methods[] = {
	PHP_ME(emicro_dispatcher, moudle, NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END

};


EMICRO_MODULE_D(dispatcher) {

	zend_class_entry emicro_dispatcher;
	INIT_NS_CLASS_ENTRY(emicro_dispatcher,"EMicro","Dispatcher",emicro_dispatcher_methods);

	zend_declare_property_null(emicro_dispatcher_ce,ZEND_STRL(EMICRO_DISPATCHER_INSTANCE),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC);
	zend_declare_property_string(emicro_dispatcher_ce,ZEND_STRL(EMICRO_DISPATCHER_MOUDLE),"",ZEND_ACC_PRIVATE);
	zend_declare_property_string(emicro_dispatcher_ce,ZEND_STRL(EMICRO_DISPATCHER_CONTROLLER),"",ZEND_ACC_PRIVATE);
	zend_declare_property_string(emicro_dispatcher_ce,ZEND_STRL(EMICRO_DISPATCHER_METHOD),"",ZEND_ACC_PRIVATE);
	zend_declare_property_bool(emicro_dispatcher_ce,ZEND_STRL(EMICRO_DISPATCHER_MULTI_MOUDLE),0,ZEND_ACC_PRIVATE);

	emicro_dispatcher_ce = zend_register_internal_class( &emicro_dispatcher TSRMLS_CC);

	return SUCCESS; 
}