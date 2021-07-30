#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "standard/php_string.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "../php_emicro.h"
#include "../app/request.h"

zend_class_entry * emicro_request_ce;

ZEND_BEGIN_ARG_INFO(arginfo_request_get, 0)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, default)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_request_post, 0)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, default)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_request_input, 0)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, default)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_request_server, 0)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, default)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_request_cookie, 0)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, default)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_request_files, 0)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, default)
ZEND_END_ARG_INFO()

zval* parseParams(size_t type, char* key, bool_int disableDispatch){

    char* trackMap[] = {
        "_POST",
        "_GET",
        "_COOKIE",
        "_SERVER",
        "_ENV",
        "_FILES",
        "_REQUEST"
    };

    zval* global = NULL;
    zval* retval;

	if (PG(auto_globals_jit)) {
		zend_string *server_str = zend_string_init(trackMap[type], strlen(trackMap[type]), 0);
		zend_is_auto_global(server_str);
		zend_string_release(server_str);
	}

    if (type == TRACK_VARS_REQUEST)
    {
        global = zend_hash_str_find(&EG(symbol_table),trackMap[type],strlen(trackMap[type]));
    }else
    {
        global = &PG(http_globals)[type];
    }

    if (disableDispatch)
    {
        zend_hash_str_del(Z_ARR_P(global),ZEND_STRL("s"));
    }
    

    if (key != NULL)
    {
        return zend_hash_str_find(Z_ARR_P(global),key,strlen(key));
    }
    
    return global;
}

PHP_METHOD(emicro_request, __construct){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_request, __clone){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_request, get){

	char* key = NULL;
	size_t key_len;
    zval* _default = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(key, key_len)
        Z_PARAM_ZVAL(_default)
	ZEND_PARSE_PARAMETERS_END();

    zval* retval = queryParams(TRACK_VARS_GET,key);

    if (retval)
    {
        RETURN_ZVAL(retval,1,0);
    }else if(_default){
        RETURN_ZVAL(_default,1,0);
    }else{
        RETURN_FALSE;
    }

}

PHP_METHOD(emicro_request, post){

	char* key = NULL;
	size_t key_len;
    zval* _default = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(key, key_len)
        Z_PARAM_ZVAL(_default)
	ZEND_PARSE_PARAMETERS_END();

    zval* retval = queryParams(TRACK_VARS_POST,key);
    
    if (retval)
    {
        RETURN_ZVAL(retval,1,0);
    }else if(_default){
        RETURN_ZVAL(_default,1,0);
    }else{
        RETURN_FALSE;
    }

}


PHP_METHOD(emicro_request, input){

	char* key = NULL;
	size_t key_len;
    zval* _default = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(key, key_len)
        Z_PARAM_ZVAL(_default)
	ZEND_PARSE_PARAMETERS_END();

    zval* retval = queryParams(TRACK_VARS_REQUEST,key);
    
    if (retval)
    {
        RETURN_ZVAL(retval,1,0);
    }else if(_default){
        RETURN_ZVAL(_default,1,0);
    }else{
        RETURN_FALSE;
    }

}

PHP_METHOD(emicro_request, server){

	char* key = NULL;
	size_t key_len;
    zval* _default = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(key, key_len)
        Z_PARAM_ZVAL(_default)
	ZEND_PARSE_PARAMETERS_END();

    zval* retval = queryParams(TRACK_VARS_SERVER,key);
    
    if (retval)
    {
        RETURN_ZVAL(retval,1,0);
    }else if(_default){
        RETURN_ZVAL(_default,1,0);
    }else{
        RETURN_FALSE;
    }

}

PHP_METHOD(emicro_request, cookie){

	char* key = NULL;
	size_t key_len;
    zval* _default = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(key, key_len)
        Z_PARAM_ZVAL(_default)
	ZEND_PARSE_PARAMETERS_END();

    zval* retval = queryParams(TRACK_VARS_COOKIE,key);
    
    if (retval)
    {
        RETURN_ZVAL(retval,1,0);
    }else if(_default){
        RETURN_ZVAL(_default,1,0);
    }else{
        RETURN_FALSE;
    }

}

PHP_METHOD(emicro_request, files){

	char* key = NULL;
	size_t key_len;
    zval* _default = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(key, key_len)
        Z_PARAM_ZVAL(_default)
	ZEND_PARSE_PARAMETERS_END();

    zval* retval = queryParams(TRACK_VARS_FILES,key);
    
    if (retval)
    {
        RETURN_ZVAL(retval,1,0);
    }else if(_default){
        RETURN_ZVAL(_default,1,0);
    }else{
        RETURN_FALSE;
    }

}

PHP_METHOD(emicro_request, header){

	char* key = NULL;
	size_t key_len;
    zval* _default = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(key, key_len)
        Z_PARAM_ZVAL(_default)
	ZEND_PARSE_PARAMETERS_END();


    zval func_name;
    zval headers;
    zval* retval;

    ZVAL_STRING(&func_name, "getallheaders");

    call_user_function(NULL,NULL,&func_name,&headers,0,NULL);

    zval *entry;
    zend_array *array;
	zend_string *string_key;
	zend_string *new_key;

    array_init(&array);

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(&headers), string_key, entry) {
        new_key = php_string_tolower(string_key);
        zend_hash_add(array, new_key, entry);
        zend_string_release_ex(new_key, 0);
	} ZEND_HASH_FOREACH_END();


    if (key != NULL)
    {

        zend_string *key2lower = php_string_tolower(zend_string_init(key,key_len,0));

        retval = zend_hash_str_find(array,key2lower->val,key_len);
        zend_string_release(key2lower);

        if (retval)
        {
            RETURN_ZVAL(retval,1,0);
        }else if(_default){
            RETURN_ZVAL(_default,1,0);
        }else{
            RETURN_FALSE;
        }

    }else{
        RETURN_ZVAL(&headers,1,0);
    }

}



zend_function_entry emicro_request_methods[] = {
	PHP_ME(emicro_request, __construct, NULL, ZEND_ACC_PRIVATE)
	PHP_ME(emicro_request, __clone, NULL, ZEND_ACC_PRIVATE)
	PHP_ME(emicro_request, get, arginfo_request_get, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(emicro_request, post, arginfo_request_post, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_request, input, arginfo_request_input, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_request, server, arginfo_request_server, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_request, cookie, arginfo_request_cookie, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_request, files, arginfo_request_files, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_request, header, arginfo_request_files, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_FE_END

};


EMICRO_MODULE_D(request) {

	zend_class_entry emicro_request;
	INIT_NS_CLASS_ENTRY(emicro_request,"EMicro","Request",emicro_request_methods);
	emicro_request_ce = zend_register_internal_class( &emicro_request TSRMLS_CC);

	return SUCCESS; 
}