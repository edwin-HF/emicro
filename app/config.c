#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "main/SAPI.h"
#include "php_main.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "../php_emicro.h"
#include "helper.h"
#include "config.h"
#include "loader.h"

#include <regex.h>

zend_class_entry * emicro_config_ce;

ZEND_BEGIN_ARG_INFO(arginfo_config_scan, 0)
    ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_config_get, 0)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, default)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_config_set, 0)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()


PHP_METHOD(emicro_config, __construct){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_config, __clone){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_config, scan){

    char *path;
    size_t path_len;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STRING(path, path_len);
    ZEND_PARSE_PARAMETERS_END();

    if (!path)
    {
        zend_throw_exception(NULL,"path can not empty!",500);
    }

    scan_config(path);
}

void scan_config_cb(char* file){

    struct stat buf;
    if (stat(file,&buf) != 0)
    {
        zend_throw_exception(NULL,"obtain config file stat err",500);
    }

    char *filename = basename(file);
    char config[MAXNAMLEN] = {0};
    strncpy(config,filename,strlen(filename) - 4);

    int cached = validate_config_cache(config,buf.st_mtim.tv_sec);

    if (!cached)
    {

        zend_try {

            zval *z_config = load_file(file);

            if (Z_TYPE_P(z_config) == IS_ARRAY)
            {

                zend_string *key;
                zval *curr_item;

                HashTable *ht_config = EMICRO_G(config);

                zval *zc_config = emicro_arr_deep_dup(z_config);
                
                zend_hash_str_update(ht_config,config,strlen(config),zc_config);

                HashTable *ht_config_mt = EMICRO_G(file_config_mt);

                zval z_now;
                ZVAL_LONG(&z_now,time(NULL));

                zend_hash_str_update(ht_config_mt,config,strlen(config),&z_now);
                
                pefree(z_config,0);
            }
        } zend_catch {

        } zend_end_try();



    }

}

void scan_config(char* path){
    scan_dir(path,scan_config_cb);
}

PHP_METHOD(emicro_config, set){

    char *key;
    size_t key_len;
    zval *value;

    ZEND_PARSE_PARAMETERS_START(2,2)
        Z_PARAM_STRING(key, key_len);
        Z_PARAM_ZVAL(value);
    ZEND_PARSE_PARAMETERS_END();

    if (key_len == 0)
    {
        zend_throw_exception(NULL,"key can not empty!",500);
    }


    // todo

}


PHP_METHOD(emicro_config, get){

    char *key;
    size_t key_len;
    zval *default_val = NULL;

    ZEND_PARSE_PARAMETERS_START(2,2)
        Z_PARAM_STRING(key, key_len);
        Z_PARAM_ZVAL(default_val);
    ZEND_PARSE_PARAMETERS_END();

    if (key_len == 0)
    {
        zend_throw_exception(NULL,"key can not empty!",500);
    }

    zval* retval = config_find(key);

    if (retval == NULL)
	{
		
		if (default_val == NULL)
		{
			RETURN_NULL();
		}else{
			RETURN_ZVAL(default_val,0,1);
		}
		
	}else{
		RETURN_ZVAL(retval,1,0);
	}


}



void* config_read_cb(char **a_str, size_t len){

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

zval* config_find(char* key){           
    return (zval*)explode_single(key,".",config_read_cb);
}


zend_function_entry emicro_config_methods[] = {
    PHP_ME(emicro_config, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_config, __clone, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_config, scan, arginfo_config_scan, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_config, set, arginfo_config_set, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_config, get, arginfo_config_get, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    { NULL, NULL, NULL }

};

EMICRO_MODULE_D(config) {
	zend_class_entry emicro_config;
	INIT_NS_CLASS_ENTRY(emicro_config,"EMicro","Config",emicro_config_methods);
	emicro_config_ce = zend_register_internal_class( &emicro_config TSRMLS_CC);
	emicro_config_ce->ce_flags |= ZEND_ACC_FINAL;

	//static
	zend_declare_property_string(emicro_config_ce, ZEND_STRL(EMICRO_CONFIG_PATH),"", ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);


	return SUCCESS; // @suppress("Symbol is not resolved")
}

int8_t validate_config_cache(char *file,int64_t mt){

    HashTable *ht = EMICRO_G(file_config_mt);

    zval *c_time = zend_hash_str_find(ht,file,strlen(file));

    if (c_time == NULL)
    {
        return 0;
    }

    if (mt > Z_LVAL_P(c_time))
    {

        HashTable *ht_config = EMICRO_G(config);

        zend_hash_str_del(ht_config,file,strlen(file));

        return 0;
    }
    
    return 1;

}