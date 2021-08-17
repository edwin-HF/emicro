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

PHP_METHOD(emicro_controller,sayHello){
	ZEND_PARSE_PARAMETERS_NONE();

    zend_string *key;
	zend_array* global = &EG(symbol_table);

    key = zend_string_init("hello global",strlen("hello global"),0);

	php_printf("The hello application %s is loaded and working!\r\n", "emicro");


    zend_string* searchKey = zend_string_init(ZEND_STRL("_SERVER"),0);

    zval* searchVal = zend_hash_find(global,searchKey);
    
    RETURN_ZVAL(searchVal,0,NULL);


}


zend_function_entry emicro_dispatcher_methods[] = {
	PHP_ME(emicro_controller, sayHello, NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END

};


EMICRO_MODULE_D(dispatcher) {

	zend_class_entry emicro_dispatcher;
	INIT_NS_CLASS_ENTRY(emicro_dispatcher,"EMicro","Dispatcher",emicro_dispatcher_methods);
	emicro_dispatcher_ce = zend_register_internal_class( &emicro_dispatcher TSRMLS_CC);

	return SUCCESS; 
}