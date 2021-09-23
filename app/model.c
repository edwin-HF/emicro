#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "standard/php_string.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "../app/model.h"

zend_class_entry * emicro_model_ce;

PHP_METHOD(emicro_model,getPaginationList){

}

PHP_METHOD(emicro_model,getList){

}

PHP_METHOD(emicro_model,getListCount){

}

PHP_METHOD(emicro_model,getInfo){

}

zend_function_entry emicro_model_methods[] = {
	PHP_ME(emicro_model, getList, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_FE_END

};


EMICRO_MODULE_D(model) {

	zend_class_entry emicro_model;
	INIT_NS_CLASS_ENTRY(emicro_model,"EMicro","Model",emicro_model_methods);
	emicro_model_ce = zend_register_internal_class( &emicro_model TSRMLS_CC);

	return SUCCESS; 
}