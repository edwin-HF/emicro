#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "main/SAPI.h"
#include "php_main.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "../php_emicro.h"
#include "helper.h"
#include "application.h"
#include "annotation.h"
#include "request.h"
#include "loader.h"

#include <regex.h>

zend_class_entry * emicro_application_ce;

ZEND_BEGIN_ARG_INFO(arginfo_application_run, 0)
    ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_application_scan, 0)
    ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_application_load, 0)
    ZEND_ARG_INFO(0, class)
ZEND_END_ARG_INFO()

PHP_METHOD(emicro_application, scan){
    
    zend_string *path;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STR(path);
    ZEND_PARSE_PARAMETERS_END();

    zend_update_static_property_string(emicro_application_ce,ZEND_STRL(EMICRO_APPLICATION_APP_PATH), ZSTR_VAL(path));

    // 注册自动加载
    zend_update_static_property_string(emicro_loader_ce, ZEND_STRL(EMICRO_LOADER_PATH), ZSTR_VAL(path));
    register_autoload();

}

PHP_METHOD(emicro_application, __construct){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_application, __clone){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_application, run){

    zval *path;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_ZVAL(path);
    ZEND_PARSE_PARAMETERS_END();

    if (!path)
    {
        zend_throw_exception(NULL,"handler can not empty!",500);
    }

    init_annotation();
    init_dispatcher(path);
}


void init_annotation(){

    zval *app_path = zend_read_static_property(emicro_application_ce, ZEND_STRL(EMICRO_APPLICATION_APP_PATH), 1);

    scan_dir(ZSTR_VAL(Z_STR_P(app_path)),annotation_scan_cb);
}

zend_function_entry emicro_application_methods[] = {
    PHP_ME(emicro_application, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_application, __clone, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_application, scan, arginfo_application_scan, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_application, run, arginfo_application_run, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    { NULL, NULL, NULL }

};

EMICRO_MODULE_D(application) {
	zend_class_entry emicro_application;
	INIT_NS_CLASS_ENTRY(emicro_application,"EMicro","Application",emicro_application_methods);
	emicro_application_ce = zend_register_internal_class( &emicro_application TSRMLS_CC);
	emicro_application_ce->ce_flags |= ZEND_ACC_FINAL;

	//static
	zend_declare_property_string(emicro_application_ce, ZEND_STRL(EMICRO_APPLICATION_APP_PATH),"", ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);

	return SUCCESS; // @suppress("Symbol is not resolved")
}