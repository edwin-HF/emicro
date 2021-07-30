#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "../php_emicro.h"
#include "../app/application.h"

zend_class_entry * emicro_application_ce;

ZEND_BEGIN_ARG_INFO(arginfo_emicro_sayHello, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(emicro_application,sayHello){

    ZEND_PARSE_PARAMETERS_NONE();

    zval* g;
	zend_array* global = &EG(symbol_table);
    zval* pg = &PG(http_globals)[TRACK_VARS_SERVER];

    zval* zfnt;
    zend_array* fnt  = &EG(class_table);

	php_printf("The hello application %s is loaded and working!\r\n", "emicro");

    zend_string* searchKey = zend_string_init(ZEND_STRL("_POST"),0);

    zval* searchVal = zend_hash_find(global,searchKey);
    
    // zval* pwd = zend_hash_str_find(Z_ARR_P(searchVal),ZEND_STRL("PWD"));

    // RETURN_ZVAL(pwd,1,0);
    // ZVAL_ARR(g,global);


    // RETURN_NULL();
    // ZVAL_ARR(zfnt,fnt);
// RETURN_ARR(global);
    // RETURN_ZVAL(pg,1,0);

    RETURN_BOOL(PG(auto_globals_jit));

}

PHP_METHOD(emicro_application, __construct){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_application, __clone){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_application,getInstance){
    ZEND_PARSE_PARAMETERS_NONE();

    zval *instance = zend_read_static_property(emicro_application_ce, ZEND_STRL(EMICRO_APPLICATION_INSTANCE), 1);

    if (Z_TYPE_P(instance) == IS_OBJECT)
    {
        RETURN_ZVAL(instance,1,0);
    }

    if (object_init_ex(instance,emicro_application_ce) == FAILURE)
    {
	    php_printf("err obtain application instance\n");
        return;
    }

	zend_update_static_property(emicro_application_ce, ZEND_STRL(EMICRO_APPLICATION_INSTANCE), instance);

    RETURN_ZVAL(instance,1,0);
}


zend_function_entry emicro_application_methods[] = {
    PHP_ME(emicro_application, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_application, __clone, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_application, getInstance, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(emicro_application, sayHello, arginfo_emicro_sayHello, ZEND_ACC_PUBLIC)
    { NULL, NULL, NULL }

};


EMICRO_MODULE_D(application) {
	zend_class_entry emicro_application;
	INIT_NS_CLASS_ENTRY(emicro_application,"EMicro","Application",emicro_application_methods);
	emicro_application_ce = zend_register_internal_class( &emicro_application TSRMLS_CC);
	emicro_application_ce->ce_flags |= ZEND_ACC_FINAL;

	//static
	zend_declare_property_null(emicro_application_ce, ZEND_STRL(EMICRO_APPLICATION_INSTANCE), ZEND_ACC_PRIVATE | ZEND_ACC_STATIC TSRMLS_CC);

	return SUCCESS; // @suppress("Symbol is not resolved")
}