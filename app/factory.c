#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "standard/php_string.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "../php_emicro.h"
#include "factory.h"
#include "annotation.h"
#include "helper.h"

zend_class_entry * emicro_factory_ce;

ZEND_BEGIN_ARG_INFO(arginfo_factory_call,0)
	ZEND_ARG_INFO(0,class)
	ZEND_ARG_INFO(0,method)
	ZEND_ARG_INFO(0,params)
ZEND_END_ARG_INFO()


PHP_METHOD(emicro_factory,call){

	char *class, *method;
	size_t class_len, method_len;
	zval *params;
	ZEND_PARSE_PARAMETERS_START(2, 3)
		Z_PARAM_STRING(class,class_len)
		Z_PARAM_STRING(method,method_len)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(params)
	ZEND_PARSE_PARAMETERS_END();


	zval called_class_before, called_class_after, called_method_before, called_method_after;

    array_init(&called_class_before);
    array_init(&called_class_after);
    array_init(&called_method_before);
    array_init(&called_method_after);

    void *class_params[2] = {&called_class_before,&called_class_after};

	char *s_doc_class = ref_class_doc(class);
    parse_annotation(s_doc_class,annotation_cb,class_params);

    void *method_params[2] = {&called_method_before,&called_method_after};

	char *s_doc_method = ref_method_doc(class,method);
    parse_annotation(s_doc_method,annotation_cb,method_params);

    annotation_run(&called_class_before, NULL);
    annotation_run(&called_method_before, NULL);

    zval *retval = call_method(class,method,params);
    annotation_run(&called_method_after,retval);
    annotation_run(&called_class_after,retval);

    RETURN_ZVAL(retval,0,1);

    pefree(retval,0);

}


zend_function_entry emicro_factory_methods[] = {
	PHP_ME(emicro_factory, call, arginfo_factory_call, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_FE_END

};


EMICRO_MODULE_D(factory) {

	zend_class_entry emicro_factory;
	INIT_NS_CLASS_ENTRY(emicro_factory,"EMicro","Factory",emicro_factory_methods);
	emicro_factory_ce = zend_register_internal_class( &emicro_factory TSRMLS_CC);

	return SUCCESS; 
}

zval* call_method(char *class, char *method, zval *m_params){

    zval obj_controller, ctor_name;
    zend_string *s_class = zend_string_init(class, strlen(class), 0);
    zend_class_entry *ce = zend_lookup_class(s_class);

    object_init_ex(&obj_controller, ce);

    if (ce->constructor) {
        zend_function *func = ce->constructor;
        zend_call_method(&obj_controller, ce, &func, ZSTR_VAL(func->common.function_name), ZSTR_LEN(func->common.function_name), NULL, 0, NULL, NULL);
    }

    zval controllerMethod, *retval, params[50];

    retval = (zval*)pemalloc(sizeof(zval),0);
    
    ZVAL_STRING(&controllerMethod, method);

	zval *cur_item;
	int8_t params_idx = 0;
	ZEND_HASH_FOREACH_VAL(Z_ARR_P(m_params),cur_item){
		ZVAL_ZVAL(&params[params_idx++],cur_item,1,1);
	}ZEND_HASH_FOREACH_END();

    call_user_function(NULL,&obj_controller,&controllerMethod,retval,params_idx,params);

    return retval;

}