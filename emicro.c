/* emicro extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "php_emicro.h"
#include "app/helper.h"
#include "app/annotation.h"
#include "app/application.h"
#include "app/dispatcher.h"
#include "app/request.h"
#include "app/factory.h"
#include "app/command.h"
#include "app/loader.h"
#include "app/config.h"

ZEND_DECLARE_MODULE_GLOBALS(emicro);

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

void emicro_call_static_method(zend_class_entry *ce, char* method, zval *retval){

    zval z_func;
    char *str_func = pemalloc(MAXPATHLEN,0);
    php_sprintf(str_func,"%s::%s",ZSTR_VAL(ce->name),method);
    ZVAL_STRING(&z_func,str_func);
    call_user_function(NULL,NULL,&z_func,retval,0,NULL);
    pefree(str_func,0);

}

void z_global_dtor(zval *zv)
{

	switch(Z_TYPE_P(zv)){
		case IS_STRING:
			zend_string_release(Z_STR_P(zv));
			break;
			break;
		default:
		break;
	}

	ZVAL_UNDEF(zv);

}

void z_dispatcher_dtor(zval *zv)
{

	switch(Z_TYPE_P(zv)){
		case IS_STRING:
			zend_string_release(Z_STR_P(zv));
			break;
		case IS_ARRAY:
			zend_array_destroy(Z_ARR_P(zv));
			break;
		default:
			
		break;
	}

	ZVAL_UNDEF(zv);
}

static void init_global(){

	EMICRO_G(router) = (HashTable*)pemalloc(sizeof(HashTable),1);
	zend_hash_init(EMICRO_G(router),0,NULL,z_global_dtor,1);

	EMICRO_G(config) = (HashTable*)pemalloc(sizeof(HashTable),1);
	zend_hash_init(EMICRO_G(config),0,NULL,NULL,1);

	EMICRO_G(annotation) = (HashTable*)pemalloc(sizeof(HashTable),1);
	zend_hash_init(EMICRO_G(annotation),0,NULL,NULL,1);

	EMICRO_G(file_router_mt) = (HashTable*)pemalloc(sizeof(HashTable),1);
	zend_hash_init(EMICRO_G(file_router_mt),0,NULL,z_global_dtor,1);

	EMICRO_G(file_config_mt) = (HashTable*)pemalloc(sizeof(HashTable),1);
	zend_hash_init(EMICRO_G(file_config_mt),0,NULL,NULL,1);

	EMICRO_G(file_annotation_mt) = (HashTable*)pemalloc(sizeof(HashTable),1);
	zend_hash_init(EMICRO_G(file_annotation_mt),0,NULL,NULL,1);

	EMICRO_G(root_path) = (char*)pemalloc(sizeof(char)*MAXPATHLEN,1);
	EMICRO_G(app_path)  = (char*)pemalloc(sizeof(char)*MAXPATHLEN,1);
	memset(EMICRO_G(root_path),0,sizeof(EMICRO_G(root_path)));
	memset(EMICRO_G(app_path),0,sizeof(EMICRO_G(app_path)));

	EMICRO_G(i) = 0;
}

void* config_callback(char **a_str, size_t len){

	HashTable *ht_config = EMICRO_G(config);
	char *ht_config_key = a_str[0];

	zval *ret = zend_hash_str_find(ht_config,ht_config_key,strlen(ht_config_key));

	if (ret != NULL)
	{
		for (size_t i = 1; i < len; i++)
		{
			if (Z_TYPE_P(ret) == IS_ARRAY)
			{
				
				ret = zend_hash_str_find(Z_ARRVAL_P(ret),a_str[i],strlen(a_str[i]));
			}
		}
	}

	return ret;
	
}

void release_global(){

	if (EMICRO_G(router))
	{
		zend_hash_clean(EMICRO_G(router));
	}

	if (EMICRO_G(config))
	{
		zend_hash_clean(EMICRO_G(config));
	}

	if (EMICRO_G(annotation))
	{
		zend_hash_clean(EMICRO_G(annotation));
	}
	
	if (EMICRO_G(file_router_mt))
	{
		zend_hash_clean(EMICRO_G(file_router_mt));
	}

	if (EMICRO_G(file_config_mt))
	{
		zend_hash_clean(EMICRO_G(file_config_mt));
	}

	if (EMICRO_G(file_annotation_mt))
	{
		zend_hash_clean(EMICRO_G(file_annotation_mt));
	}

	if (EMICRO_G(root_path))
	{
		pefree(EMICRO_G(root_path),1);
	}

	if (EMICRO_G(app_path))
	{
		pefree(EMICRO_G(app_path),1);
	}
	
}

zval* emicro_arr_deep_dup(zval *source){

	zval *target = (zval*)pemalloc(sizeof(zval),1);
	array_init(target); 
	Z_ARR_P(target) = (HashTable*)pemalloc(sizeof(HashTable),1);
	zend_hash_init(Z_ARR_P(target),0,NULL,z_global_dtor,1);

	zval *cur_item;
	zend_string *cur_key;

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARR_P(source),cur_key,cur_item){

		zval *z_val, *z_arr, *z_key;

		if (cur_key)
		{
			z_key = (zval*)pemalloc(sizeof(zval),1);
			ZVAL_PSTRING(z_key,ZSTR_VAL(cur_key));
		}

		switch (Z_TYPE_P(cur_item))
		{
			case IS_ARRAY:
					z_arr = emicro_arr_deep_dup(cur_item);
					zend_hash_update(Z_ARR_P(target),Z_STR_P(z_key),z_arr);
					goto end;
				break;

			case IS_STRING:
					z_val = (zval*)pemalloc(sizeof(zval),1);
					ZVAL_PSTRING(z_val,ZSTR_VAL(Z_STR_P(cur_item)));
				break;

			default:
					z_val = (zval*)pemalloc(sizeof(zval),1);
					ZVAL_ZVAL(z_val,cur_item,1,1);
				break;
		}

		if (cur_key)
		{
			zend_hash_update(Z_ARR_P(target),Z_STR_P(z_key),z_val);
		}else{
			add_next_index_zval(target,z_val);
		}

		
	}ZEND_HASH_FOREACH_END();

end:
	return target;

}

/* }}} */

/* {{{ string config( [ string $var ] )
 */
PHP_FUNCTION(config)
{
	char* key;
	size_t key_len;
	zval *retval;
	zval *z_default = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_STRING(key, key_len)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(z_default)
	ZEND_PARSE_PARAMETERS_END();

	char s_key[key_len];
	strcpy(s_key,key);
	
	retval = (zval*)explode_single(s_key,".",config_callback);

	if (retval == NULL)
	{
		
		if (z_default == NULL)
		{
			RETURN_NULL();
		}else{
			RETURN_ZVAL(z_default,0,1);
		}
		
	}else{
		RETURN_ZVAL(retval,1,0);
	}

}

PHP_GINIT_FUNCTION(emicro) {

}

PHP_MINIT_FUNCTION(emicro){

	init_global();

	EMICRO_STARTUP(application);
	EMICRO_STARTUP(dispatcher);
	EMICRO_STARTUP(request);
	EMICRO_STARTUP(factory);
	EMICRO_STARTUP(command);
	EMICRO_STARTUP(loader);
	EMICRO_STARTUP(config);

	return SUCCESS;

}

/* }}}*/

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(emicro)
{
#if defined(ZTS) && defined(COMPILE_DL_EMICRO)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	EMICRO_G(i) = EMICRO_G(i) + 1;
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(emicro){
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(emicro){
	release_global();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(emicro)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "emicro support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ arginfo
 */

ZEND_BEGIN_ARG_INFO(arginfo_emicro_config, 0)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0,default)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ emicro_functions[]
 */
static const zend_function_entry emicro_functions[] = {
	PHP_FE(config,		arginfo_emicro_config)
	PHP_FE_END
};
/* }}} */

/* {{{ emicro_module_entry
 */
zend_module_entry emicro_module_entry = {
	STANDARD_MODULE_HEADER,
	"emicro",					/* Extension name */
	emicro_functions,			/* zend_function_entry */
	PHP_MINIT(emicro),							/* PHP_MINIT - Module initialization */
	PHP_MSHUTDOWN(emicro),							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(emicro),			/* PHP_RINIT - Request initialization */
	PHP_RSHUTDOWN(emicro),							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(emicro),			/* PHP_MINFO - Module info */
	PHP_EMICRO_VERSION,		/* Version */
	PHP_MODULE_GLOBALS(emicro),
	PHP_GINIT(emicro),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_EMICRO
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(emicro)
#endif
