#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "main/SAPI.h"
#include "php_main.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "../php_emicro.h"
#include "helper.h"
#include "command.h"
#include "application.h"
#include "annotation.h"
#include "request.h"

#include <regex.h>

zend_class_entry * emicro_command_ce;

ZEND_BEGIN_ARG_INFO(arginfo_command_run, 0)
    ZEND_ARG_INFO(0, path)
    ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_command_scan, 0)
    ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()


void annotation_cb_command_method(char *annotation, char *annotation_param, char *position, void *params){




    void** cm_params = (void**)params;

    char* class  = (char*)cm_params[0];
    char* method = (char*)cm_params[1];

    zval *z_cm;
    zval *z_command_map = zend_read_static_property(emicro_command_ce, ZEND_STRL(EMICRO_COMMAND_COLLECTION_INSTANCE), 1);

    if (Z_TYPE_P(z_command_map) == IS_NULL)
    {
        z_command_map = (zval*)pemalloc(sizeof(zval),1);
        array_init(z_command_map);
        Z_ARR_P(z_command_map) = (HashTable*)pemalloc(sizeof(HashTable),1);
        zend_hash_init(Z_ARR_P(z_command_map),0,NULL,NULL,1);
    }

    z_cm = (zval*)pemalloc(sizeof(zval),1);
    array_init(z_cm);
    Z_ARR_P(z_cm) = (HashTable*)pemalloc(sizeof(HashTable),1);
    zend_hash_init(Z_ARR_P(z_cm),0,NULL,NULL,1);

    add_index_string(z_cm, 0, class);
    add_index_string(z_cm, 1, method);

    add_assoc_zval(z_command_map,annotation_param,z_cm);

    zend_update_static_property(emicro_command_ce, ZEND_STRL(EMICRO_COMMAND_COLLECTION_INSTANCE), z_command_map);


}

void ref_command_class_method_doc_cb(char *class, char *method, char* doc_comment, void *params){



    char *m_params[2];

    m_params[0] = class;
    m_params[1] = method;

    parse_annotation_filter(doc_comment,annotation_cb_command_method,m_params,"exec");

}

void command_annotation_scan_cb(char *file){
        
    struct stat buf;
    if (stat(file,&buf) != 0)
    {
        zend_throw_exception(NULL,"obtain file stat err",500);
    }

    char pattern[255];
    php_sprintf(pattern,".*application/");


    char filename[MAXPATHLEN] = {0};
    char ns_class[MAXPATHLEN] = {0};

    reg_replace(file,pattern,"",filename);
    reg_replace(filename,"/","\\",ns_class);

    char class[MAXNAMLEN] = {0};
    strncpy(class,ns_class,strlen(filename) - 4);

    char *class_document = ref_class_doc(class);

    if (strlen(class_document) > 0)
    {
        if (reg_match(class_document,"Command"))
        {
            ref_class_method_doc(class,ref_command_class_method_doc_cb, NULL);
        }
    }

}


void init_command_annotation(){

    zval* app_path =zend_read_static_property(emicro_command_ce,ZEND_STRL(EMICRO_COMMAND_APP_PATH),1);

    scan_dir(ZSTR_VAL(Z_STR_P(app_path)), command_annotation_scan_cb);

}

void run_command(char *command, zval* params){

    zval *z_command_map = zend_read_static_property(emicro_command_ce, ZEND_STRL(EMICRO_COMMAND_COLLECTION_INSTANCE), 1);

    if (Z_TYPE_P(z_command_map) == IS_NULL)
    {
        zend_throw_exception(NULL,"command not found!",500);
        return;
    }

    zval *z_cm = zend_hash_str_find(Z_ARR_P(z_command_map), command, strlen(command));

    if (z_cm == NULL)
    {
        zend_throw_exception(NULL,"command not found!",500);
        return;
    }

    if (Z_TYPE_P(z_cm) != NULL)
    {
        zval *z_class  = zend_hash_index_find(Z_ARR_P(z_cm), 0);
        zval *z_method = zend_hash_index_find(Z_ARR_P(z_cm), 1);

        zend_class_entry *obj_ptr = zend_lookup_class(Z_STR_P(z_class));

        if (obj_ptr)
        {

            zval command_obj, command_func, command_params[1], retval;

            object_init_ex(&command_obj, obj_ptr);
        
            ZVAL_ZVAL(&command_func,z_method,1,1);
            ZVAL_ZVAL(&command_params[0],params,1,1);

            call_user_function(NULL,&command_obj,&command_func,&retval,1,command_params);

        }

    }
    
}

PHP_METHOD(emicro_command, scan){
    
    zend_string *params;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STR(params);
    ZEND_PARSE_PARAMETERS_END();

    zend_update_static_property_string(emicro_command_ce,ZEND_STRL(EMICRO_COMMAND_APP_PATH),params->val);

}


PHP_METHOD(emicro_command, __construct){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_command, __clone){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_command, run){

    zval *path;
    zval *params;

    ZEND_PARSE_PARAMETERS_START(2,2)
        Z_PARAM_ZVAL(path);
        Z_PARAM_ZVAL(params);
    ZEND_PARSE_PARAMETERS_END();

    if (!path)
    {
        zend_throw_exception(NULL,"handler can not empty!",500);
    }

    init_command_annotation();
    run_command(ZSTR_VAL(Z_STR_P(path)), params);

}


zend_function_entry emicro_command_methods[] = {
    PHP_ME(emicro_command, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_command, __clone, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_command, run, arginfo_command_run, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_command, scan, arginfo_command_scan, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    { NULL, NULL, NULL }

};

EMICRO_MODULE_D(command) {
	zend_class_entry emicro_command;
	INIT_NS_CLASS_ENTRY(emicro_command,"EMicro","Command",emicro_command_methods);
	emicro_command_ce = zend_register_internal_class( &emicro_command TSRMLS_CC);
	emicro_command_ce->ce_flags |= ZEND_ACC_FINAL;

	//static
	zend_declare_property_null(emicro_command_ce, ZEND_STRL(EMICRO_COMMAND_COLLECTION_INSTANCE), ZEND_ACC_PRIVATE | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(emicro_command_ce, ZEND_STRL(EMICRO_COMMAND_APP_PATH),"", ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);


	return SUCCESS; // @suppress("Symbol is not resolved")
}