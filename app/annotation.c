#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "standard/php_string.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"
#include "../../reflection/php_reflection.h"

#include "application.h"
#include "annotation.h"

static void call_annotation(char *annotation, char *annotation_param, zval *retval){
    zval app_obj, *c_rv;

    emicro_call_static_method(emicro_application_ce,"getInstance",&app_obj);

    zval *z_annotation = zend_read_property(emicro_application_ce,&app_obj,ZEND_STRL(EMICRO_APPLICATION_ANNOTATION_NAMESPACE),1,c_rv);

    char ns_annotation[MAXNAMLEN];
    php_sprintf(ns_annotation,"%s\\%s",ZSTR_VAL(Z_STR_P(z_annotation)), annotation);

    zval annotation_obj, annotation_func, annotation_retval, annotation_params[2];
    zend_string *a_key = zend_string_init(ns_annotation,strlen(ns_annotation), 0);
    zend_class_entry *obj_ptr = zend_lookup_class(a_key);
    zend_string_free(a_key);

    if (obj_ptr)
    {
        object_init_ex(&annotation_obj, obj_ptr);
    
        ZVAL_STRING(&annotation_func, "run");
        ZVAL_STRING(&annotation_params[0],annotation_param);
        if (retval != NULL)
        {
            ZVAL_ZVAL(&annotation_params[1],retval,1,1);
        }else{
            ZVAL_NULL(&annotation_params[1]);
        }

        call_user_function(NULL,&annotation_obj,&annotation_func,&annotation_retval,2,annotation_params);

    }
}

void annotation_run(zval *list, zval *retval){
    zval *z_item;
    ZEND_HASH_FOREACH_VAL(Z_ARR_P(list),z_item){

        zval *z_class  = zend_hash_index_find(Z_ARR_P(z_item),0);
        zval *z_method = zend_hash_index_find(Z_ARR_P(z_item),1);

        call_annotation(ZSTR_VAL(Z_STR_P(z_class)),ZSTR_VAL(Z_STR_P(z_method)),retval);

    }ZEND_HASH_FOREACH_END();

}


void annotation_cb(char *annotation, char *annotation_param, char *position, void *params){

    void **cb_params = (void**)params;
    zval *called_before = (zval*)cb_params[0];
    zval *called_after  = (zval*)cb_params[1];

    if (strcmp(annotation,"") != 0)
    {
        zval cm;
        array_init(&cm);

        add_next_index_string(&cm,annotation);
        add_next_index_string(&cm,annotation_param);

        if (strcmp(position,"after") == 0)
        {
            add_next_index_zval(called_after,&cm);
        }else{
            add_next_index_zval(called_before,&cm);
        }

    }

}

char* ref_class_doc(char *class){

    zval reflection_class;
    zval ctor_name, reflection, ctor_params[1];
    ZVAL_STRING(&ctor_name,"__construct");
    ZVAL_STRING(&ctor_params[0],class);

    object_init_ex(&reflection_class,reflection_class_ptr);
    call_user_function(NULL,&reflection_class,&ctor_name,&reflection,1,&ctor_params);

    zval func_doc_method, doc;
    ZVAL_STRING(&func_doc_method,"getDocComment");
    call_user_function(NULL,&reflection_class,&func_doc_method,&doc,0,NULL);

    if (Z_TYPE(doc) == IS_FALSE)
    {
        return "";
    }else{
        return ZSTR_VAL(Z_STR(doc));
    }
    
}

char* ref_method_doc(char *class, char* method){

    zval ref_class;
    zval ctor_name, reflection, ctor_params[1];
    ZVAL_STRING(&ctor_name,"__construct");
    ZVAL_STRING(&ctor_params[0],class);

    object_init_ex(&ref_class,reflection_class_ptr);
    call_user_function(NULL,&ref_class,&ctor_name,&reflection,1,&ctor_params);


    zval func_reflection_method,doc_handler;
    zval func_reflection_method_params[1];

    ZVAL_STRING(&func_reflection_method,"getMethod");
    ZVAL_STRING(&func_reflection_method_params[0],method);

    call_user_function(NULL,&ref_class,&func_reflection_method,&doc_handler,1,&func_reflection_method_params);

    zval func_doc_method, doc_method;
    ZVAL_STRING(&func_doc_method,"getDocComment");
    call_user_function(NULL,&doc_handler,&func_doc_method,&doc_method,0,NULL);

    char *s_doc_method = "";

    if (Z_TYPE(doc_method) != IS_FALSE)
    {
        s_doc_method = ZSTR_VAL(Z_STR(doc_method));
    }

    return s_doc_method;

}