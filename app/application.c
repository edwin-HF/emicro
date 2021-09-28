#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "main/SAPI.h"
#include "php_main.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"
#include "../../reflection/php_reflection.h"

#include "../php_emicro.h"
#include "helper.h"
#include "../app/application.h"
#include "request.h"

#include <regex.h>

zend_class_entry * emicro_application_ce;

ZEND_BEGIN_ARG_INFO(arginfo_application_dispatcherNamespace, 0)
    ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_application_annotationNamespace, 0)
    ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_application_setAppPath, 0)
    ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_application_load, 0)
    ZEND_ARG_INFO(0, class)
ZEND_END_ARG_INFO()

PHP_METHOD(emicro_application, dispatcherNamespace){
    
    zval *obj = getThis();
    zend_string *params;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STR(params);
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_str(emicro_application_ce,obj,ZEND_STRL(EMICRO_APPLICATION_DISPATCHER_NAMESPACE),params);

    RETURN_OBJ(obj);

}

PHP_METHOD(emicro_application, setAppPath){
    
    zval *obj = getThis();
    zend_string *params;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STR(params);
    ZEND_PARSE_PARAMETERS_END();

    zend_update_static_property_string(emicro_application_ce,ZEND_STRL(EMICRO_APPLICATION_APP_PATH),params->val);

    RETURN_OBJ(obj);

}

PHP_METHOD(emicro_application, getAppPath){
    
    zval *obj = getThis();
    zval *retval, *rv;

    ZEND_PARSE_PARAMETERS_NONE();

    retval = zend_read_static_property(emicro_application_ce,ZEND_STRL(EMICRO_APPLICATION_APP_PATH),1);


    RETURN_ZVAL(retval,1,1);

}

PHP_METHOD(emicro_application, annotationNamespace){
    
    zval *obj = getThis();
    zend_string *params;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STR(params);
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_str(emicro_application_ce,obj,ZEND_STRL(EMICRO_APPLICATION_ANNOTATION_NAMESPACE),params);

    RETURN_OBJ(obj);

}

PHP_METHOD(emicro_application, __construct){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_application, __clone){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_application, load){

    zval *this = getThis();
    zval *app_path , *rv;
    zval *class;
    size_t class_len;
    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_ZVAL(class);
    ZEND_PARSE_PARAMETERS_END();

    app_path =zend_read_static_property(emicro_application_ce,ZEND_STRL(EMICRO_APPLICATION_APP_PATH),1);

    char *realpath  = Z_STRVAL_P(app_path);
    char *classPath = replace(class,"\\","/");
    char *className = emalloc(MAXPATHLEN);

    php_sprintf(className,"%s/%s.php",realpath,classPath);

    load(className);

    efree(className);

}

PHP_METHOD(emicro_application, getInstance){
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

    init_config();

	zend_update_static_property(emicro_application_ce, ZEND_STRL(EMICRO_APPLICATION_INSTANCE), instance);

    RETURN_ZVAL(instance,1,0);
}

PHP_METHOD(emicro_application, run){

    zval* this = getThis();

    zval func_name;
    zval func_autoload;
    zval params[1],ret;

    char *appPath = app_path();

    zend_update_static_property_string(emicro_application_ce,ZEND_STRL(EMICRO_APPLICATION_APP_PATH),appPath);

    ZVAL_STRING(&func_name, "spl_autoload_register");
    ZVAL_STRING(&params[0], "EMicro\\Application::load");

    call_user_function(NULL,NULL,&func_name,&ret,1,params);
    
    init_router_map();
    dispatcher();
    
}

void print_g(){

    php_printf("global i = %d\n",EMICRO_G(i));
    EMICRO_G(i) = EMICRO_G(i) + 1;

    zend_string *cur_key;
    zval *cur_val;
    ZEND_HASH_FOREACH_STR_KEY_VAL(EMICRO_G(file_router_mt),cur_key,cur_val){

        zend_string *key;
        zval *cur_v;

        php_printf("\n %s --> ",cur_key->val);
        php_printf("type -- %d\n",Z_TYPE_P(cur_val));
        php_printf("count -- %d\n",Z_ARR_P(cur_val)->nNumUsed);


        zval *timestamp = zend_hash_str_find(Z_ARR_P(cur_val),"c_timestamp",strlen("c_timestamp"));

        php_printf("time -- %d\n",Z_LVAL_P(timestamp));

        zval *z_router = zend_hash_str_find(Z_ARR_P(cur_val),"c_router",strlen("c_router"));
// php_printf(" %ld \n",ZSTR_VAL(Z_STR_P(timestamp)));
        ZEND_HASH_FOREACH_VAL(Z_ARR_P(z_router),cur_v){

            php_printf("router -- %s\n",ZSTR_VAL(Z_STR_P(cur_v)));
            // php_printf("ccc -- \n");
            // php_printf(" %ld \n",ZSTR_VAL(Z_STR_P(cur_v)));
            // php_printf(" %s ",ZSTR_VAL(Z_STR_P(cur_v)));

        }ZEND_HASH_FOREACH_END();

    }ZEND_HASH_FOREACH_END();

}

char* app_path(){

    zval str;
    ZVAL_STRING(&str,root_path());

    return replace(&str,"public","application");

}

char* root_path(){
    char* root;
    root = VCWD_GETCWD(root,MAXPATHLEN);
    return root;
}

zval* load(char *path) {

    if (access(path,F_OK) != -1)
    {
        zend_file_handle file_handle;
        zval *retval = (zval*)emalloc(sizeof(zval));

        zend_stream_init_filename(&file_handle, path);
        php_execute_simple_script(&file_handle,retval);
        return retval;
    }

    return NULL;

}


void scan_cb_config(char* file){

    char *filename = basename(file);
    char config[MAXNAMLEN] = {0};
    strncpy(config,filename,strlen(filename) - 4);

    zval *z_config = load(file);

    zend_string *key;
    zval *curr_item;

    HashTable *ht_config = EMICRO_G(config);

    zend_hash_str_update(ht_config,config,strlen(config),z_config);

}

void init_config(){
    char *path =  app_path();
    char config_path[MAXPATHLEN];

    php_sprintf(config_path,"%s/config",path);

    scan_dir(config_path,scan_cb_config);

}

zend_function_entry emicro_application_methods[] = {
    PHP_ME(emicro_application, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_application, __clone, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_application, getInstance, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_application, load, arginfo_application_load, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_application, dispatcherNamespace, arginfo_application_dispatcherNamespace, ZEND_ACC_PUBLIC)
    PHP_ME(emicro_application, annotationNamespace, arginfo_application_annotationNamespace, ZEND_ACC_PUBLIC)
    PHP_ME(emicro_application, setAppPath, arginfo_application_setAppPath, ZEND_ACC_PUBLIC)
    PHP_ME(emicro_application, getAppPath, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(emicro_application, run, NULL, ZEND_ACC_PUBLIC)
    { NULL, NULL, NULL }

};

EMICRO_MODULE_D(application) {
	zend_class_entry emicro_application;
	INIT_NS_CLASS_ENTRY(emicro_application,"EMicro","Application",emicro_application_methods);
	emicro_application_ce = zend_register_internal_class( &emicro_application TSRMLS_CC);
	emicro_application_ce->ce_flags |= ZEND_ACC_FINAL;

	//static
	zend_declare_property_null(emicro_application_ce, ZEND_STRL(EMICRO_APPLICATION_INSTANCE), ZEND_ACC_PRIVATE | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(emicro_application_ce, ZEND_STRL(EMICRO_APPLICATION_DISPATCHER_NAMESPACE),"controller", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(emicro_application_ce, ZEND_STRL(EMICRO_APPLICATION_ANNOTATION_NAMESPACE),"annotation", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(emicro_application_ce, ZEND_STRL(EMICRO_APPLICATION_APP_PATH),"", ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);

	return SUCCESS; // @suppress("Symbol is not resolved")
}