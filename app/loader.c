#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "main/SAPI.h"
#include "php_main.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "../php_emicro.h"
#include "helper.h"
#include "loader.h"

#include <regex.h>

zend_class_entry * emicro_loader_ce;

ZEND_BEGIN_ARG_INFO(arginfo_loader_scan, 0)
    ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_loader_load, 0)
    ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_loader_autoload, 0)
    ZEND_ARG_INFO(0, class)
ZEND_END_ARG_INFO()

PHP_METHOD(emicro_loader, __construct){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_loader, __clone){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_loader, scan){

    char *path;
    size_t path_len;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STRING(path, path_len);
    ZEND_PARSE_PARAMETERS_END();

    if (!path)
    {
        zend_throw_exception(NULL,"path can not empty!",500);
    }

    zend_update_static_property_string(emicro_loader_ce, ZEND_STRL(EMICRO_LOADER_PATH), path);

    register_autoload();

}

PHP_METHOD(emicro_loader, autoload){

    zval *this = getThis();
    zval *app_path , *rv;
    char *class;
    size_t class_len;
    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STRING(class,class_len);
    ZEND_PARSE_PARAMETERS_END();

    app_path =zend_read_static_property(emicro_loader_ce,ZEND_STRL(EMICRO_LOADER_PATH),1);


    if (strlen(ZSTR_VAL(Z_STR_P(app_path))) <1)
    {
        zend_throw_exception(NULL,"scan path can not empty!",500);
    }
    

    char *realpath  = Z_STRVAL_P(app_path);
    char className[MAXPATHLEN] = {0};
    char classPath[MAXPATHLEN] = {0};
    reg_replace(class,"[\\]{1}","/",classPath);

    php_sprintf(className,"%s/%s.php",realpath,classPath);

    zval *retval = load_file(className);

    if (retval != NULL)
    {
        pefree(retval,0);
    }
    

}

PHP_METHOD(emicro_loader, load){

    zval *this = getThis();
    zval *app_path , *rv;
    char *path;
    size_t path_len;
    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STRING(path,path_len);
    ZEND_PARSE_PARAMETERS_END();

    load(path);

}

void register_autoload(){

    zval func_name;
    zval func_autoload;
    zval params[1],ret;

    ZVAL_STRING(&func_name, "spl_autoload_register");
    ZVAL_STRING(&params[0], "EMicro\\Loader::autoload");

    call_user_function(NULL,NULL,&func_name,&ret,1,params);

}

zval* load_file(char *path) {

    if (access(path,F_OK) != -1)
    {
        zend_file_handle file_handle;
        zval *retval = (zval*)pemalloc(sizeof(zval),0);

        zend_stream_init_filename(&file_handle, path);
        php_execute_simple_script(&file_handle,retval);
        return retval;
    }

    return NULL;

}


zend_function_entry emicro_loader_methods[] = {
    PHP_ME(emicro_loader, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_loader, __clone, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_loader, autoload, arginfo_loader_autoload, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    // PHP_ME(emicro_loader, scan, arginfo_loader_scan, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_loader, load, arginfo_loader_load, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    { NULL, NULL, NULL }

};

EMICRO_MODULE_D(loader) {
	zend_class_entry emicro_loader;
	INIT_NS_CLASS_ENTRY(emicro_loader,"EMicro","Loader",emicro_loader_methods);
	emicro_loader_ce = zend_register_internal_class( &emicro_loader TSRMLS_CC);
	emicro_loader_ce->ce_flags |= ZEND_ACC_FINAL;

	//static
	zend_declare_property_string(emicro_loader_ce, ZEND_STRL(EMICRO_LOADER_PATH),"", ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);


	return SUCCESS; // @suppress("Symbol is not resolved")
}