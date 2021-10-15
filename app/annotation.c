#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "standard/php_string.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "../php_emicro.h"
#include "application.h"
#include "annotation.h"
#include "dispatcher.h"
#include "helper.h"

void annotation_scan_cb(char *file){

    struct stat buf;
    if (stat(file,&buf) != 0)
    {
        zend_throw_exception(NULL,"obtain file stat err",500);
    }

    int cached = validate_annotation_cache(file,buf.st_mtim.tv_sec);

    if (!cached)
    {
        char pattern[255];
        php_sprintf(pattern,".*application/");

        char filename[MAXPATHLEN] = {0};
        char ns_class[MAXPATHLEN] = {0};

        reg_replace(file,pattern,"",filename);
        reg_replace(filename,"/","\\",ns_class);

        char class[MAXNAMLEN] = {0};
        strncpy(class,ns_class,strlen(filename) - 4);

        char *class_document = ref_class_doc(class);

        if (reg_match(class_document,"Annotation"))
        {
            init_annotation_map(file, class, class_document);
        }

        if (reg_match(class_document,"Controller"))
        {
            init_router_map(file, class, class_document);
        }

        zval z_now;
        ZVAL_LONG(&z_now,time(NULL));

        zend_hash_str_update(EMICRO_G(file_annotation_mt),file,strlen(file),&z_now);

    }

}

void init_annotation_map(char *file, char *class, char *class_document){

    char s_annotation[MAXNAMLEN] = {0};

    reg_replace(class,".*[\\]{1}","",s_annotation);

    zend_string *s_class = zend_string_init(class,strlen(class),1);
    zval *z_class = (zval*)pemalloc(sizeof(zval),1);
    ZVAL_STR(z_class,s_class);

    zend_hash_str_update(EMICRO_G(annotation),s_annotation,strlen(s_annotation),z_class);

}

static void call_annotation(char *annotation, char *annotation_param, zval *retval){

    zval *z_class = zend_hash_str_find(EMICRO_G(annotation),annotation,strlen(annotation));

    if (z_class != NULL)
    {

        zend_class_entry *obj_ptr = zend_lookup_class(Z_STR_P(z_class));

        if (obj_ptr)
        {

            zval annotation_obj, annotation_func, annotation_retval, annotation_params[2];

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

    zend_string *s_class = zend_string_init(class,strlen(class),0);
    zend_class_entry *ce = zend_lookup_class(s_class);
    zend_string_release(s_class);

    if (ce != NULL && ce->type == ZEND_USER_CLASS && ce->info.user.doc_comment)
    {
        return ZSTR_VAL(ce->info.user.doc_comment);
    }

    return "";
    
}

char* ref_method_doc(char *class, char* method){

    zend_string *s_class = zend_string_init(class,strlen(class),0);
    zend_class_entry *ce = zend_lookup_class(s_class);
    zend_string_release(s_class);

    char *l_method = zend_str_tolower_dup(method,strlen(method));
    
    zend_op_array *op_array = zend_hash_str_find_ptr(&ce->function_table,l_method,strlen(l_method));

    if (op_array != NULL && op_array->type == ZEND_USER_FUNCTION && op_array->doc_comment)
    {
        return ZSTR_VAL(op_array->doc_comment);
    }

    return "";
    
}

void ref_class_method_doc(char *class,ref_method_doc_cb ref_cb,void *params){

    zend_string *s_class = zend_string_init(class,strlen(class),0);
    zend_class_entry *ce = zend_lookup_class(s_class);
    zend_string_release(s_class);

    zend_op_array *op_array;

    ZEND_HASH_FOREACH_PTR(&ce->function_table, op_array) {
        if (op_array->type == ZEND_USER_FUNCTION) {
            if (op_array->doc_comment)
            {
                ref_cb(class,op_array->function_name->val,op_array->doc_comment->val,params);
            }else{
                ref_cb(class,op_array->function_name->val,"",params);
            }
            
        }
    } ZEND_HASH_FOREACH_END();

}

int8_t validate_annotation_cache(char *file, int64_t mt){

    HashTable *ht = EMICRO_G(file_annotation_mt);

    zval *c_time = zend_hash_str_find(ht,file,strlen(file));

    if (c_time == NULL)
    {
        return 0;
    }

    if (mt > Z_LVAL_P(c_time))
    {
        return 0;
    }
    
    return 1;

}