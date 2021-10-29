#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"
#include "../../reflection/php_reflection.h"

#include "../php_emicro.h"
#include "dispatcher.h"
#include "application.h"
#include "helper.h"
#include "request.h"
#include "annotation.h"

zend_class_entry * emicro_dispatcher_ce;

ZEND_BEGIN_ARG_INFO(arginfo_emicro_sayHello, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(emicro_dispatcher, controller){
    zval *rv;
    zval *retval = zend_read_property(emicro_dispatcher_ce,getThis(),ZEND_STRL(EMICRO_DISPATCHER_CONTROLLER),1,rv);

    RETURN_ZVAL(retval,1,0);

}

PHP_METHOD(emicro_dispatcher, method){
    zval *rv;
    zval *retval = zend_read_property(emicro_dispatcher_ce,getThis(),ZEND_STRL(EMICRO_DISPATCHER_METHOD),1,rv);
    RETURN_ZVAL(retval,1,0);
}


PHP_METHOD(emicro_dispatcher, getInstance){
    
    ZEND_PARSE_PARAMETERS_NONE();

    zval *instance = zend_read_static_property(emicro_dispatcher_ce, ZEND_STRL(EMICRO_DISPATCHER_INSTANCE), 1);

    if (Z_TYPE_P(instance) == IS_OBJECT)
    {
        RETURN_ZVAL(instance,0,1);
    }

    if (object_init_ex(instance,emicro_dispatcher_ce) == FAILURE)
    {
	    php_printf("err obtain application instance\n");
        return;
    }

	zend_update_static_property(emicro_dispatcher_ce, ZEND_STRL(EMICRO_DISPATCHER_INSTANCE), instance);

    RETURN_ZVAL(instance,1,0);
}

zend_function_entry emicro_dispatcher_methods[] = {
	PHP_ME(emicro_dispatcher, getInstance, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(emicro_dispatcher, controller, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(emicro_dispatcher, method, NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END

};


EMICRO_MODULE_D(dispatcher) {

	zend_class_entry emicro_dispatcher;
	INIT_NS_CLASS_ENTRY(emicro_dispatcher,"EMicro","Dispatcher",emicro_dispatcher_methods);

    emicro_dispatcher_ce = zend_register_internal_class( &emicro_dispatcher TSRMLS_CC);

	zend_declare_property_null(emicro_dispatcher_ce,ZEND_STRL(EMICRO_DISPATCHER_INSTANCE),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC);
	zend_declare_property_string(emicro_dispatcher_ce,ZEND_STRL(EMICRO_DISPATCHER_MOUDLE),"",ZEND_ACC_PRIVATE);
	zend_declare_property_string(emicro_dispatcher_ce,ZEND_STRL(EMICRO_DISPATCHER_CONTROLLER),"",ZEND_ACC_PRIVATE);
	zend_declare_property_string(emicro_dispatcher_ce,ZEND_STRL(EMICRO_DISPATCHER_METHOD),"",ZEND_ACC_PRIVATE);
	zend_declare_property_bool(emicro_dispatcher_ce,ZEND_STRL(EMICRO_DISPATCHER_MULTI_MOUDLE),0,ZEND_ACC_PRIVATE);

	return SUCCESS; 
}


void annotation_cb_dispatcher_method(char *annotation, char *annotation_param , char *position,  void *params){
    
    char **router = (char**)(params);
    char *r_router = router[0];
    char *ns_class = router[1];
    char *method   = router[2];
    char *file     = router[3];
    char router_path[MAXPATHLEN] = {0};

    if (strcmp(method,"__construct") == 0)
    {
        return;
    }

    if (strcmp(annotation,"Route") == 0)
    {
        strlen(r_router) == 0 ? strcpy(router_path,annotation_param) : php_sprintf(router_path,"%s/%s",r_router,annotation_param);
    }else{
        strlen(r_router) == 0 ? strcpy(router_path,method) : php_sprintf(router_path,"%s/%s",r_router,method);
    }

    HashTable *ht = EMICRO_G(router);
    zval *z_router_map = (zval*)pemalloc(sizeof(zval),1);
    array_init(z_router_map);
    Z_ARR_P(z_router_map) = (HashTable*)pemalloc(sizeof(HashTable),1);
    zend_hash_init(Z_ARR_P(z_router_map),0,NULL,z_dispatcher_dtor,1);

    zval *z_ns_class = (zval*)pemalloc(sizeof(zval),1);
    zend_string *zs_ns_class = zend_string_init(ns_class,strlen(ns_class),1);
    ZVAL_STR(z_ns_class,zs_ns_class);

    zval *z_method = (zval*)pemalloc(sizeof(zval),1);
    zend_string *zs_method = zend_string_init(method,strlen(method),1);
    ZVAL_STR(z_method,zs_method);

    add_next_index_zval(z_router_map,z_ns_class);
    add_next_index_zval(z_router_map,z_method);

    zend_hash_str_update(ht,router_path,strlen(router_path),z_router_map);

    HashTable *ht_file_router_mt = EMICRO_G(file_router_mt);

    zval *z_file_map = zend_hash_str_find(ht_file_router_mt,file,strlen(file));

    if (z_file_map == NULL)
    {
        z_file_map = (zval*)pemalloc(sizeof(zval),1);
        array_init(z_file_map);
        Z_ARR_P(z_file_map) = (HashTable*)pemalloc(sizeof(HashTable),1);
        zend_hash_init(Z_ARR_P(z_file_map),0,NULL,NULL,1);
    }

    add_assoc_long(z_file_map,"c_timestamp",time(NULL));

    zval *z_c_router = zend_hash_str_find(Z_ARR_P(z_file_map),"c_router",strlen("c_router"));

    if (z_c_router == NULL)
    {
        z_c_router = (zval*)pemalloc(sizeof(zval),1);
        array_init(z_c_router);
        Z_ARR_P(z_c_router) = (HashTable*)pemalloc(sizeof(HashTable),1);
        zend_hash_init(Z_ARR_P(z_c_router),8,NULL,z_global_dtor,1);
    }

    zval *z_router_path = (zval*)pemalloc(sizeof(zval),1);
    zend_string *zs_router_path = zend_string_init(router_path,strlen(router_path),1);
    ZVAL_STR(z_router_path,zs_router_path);

    add_next_index_zval(z_c_router,z_router_path);

    add_assoc_zval(z_file_map,"c_router",z_c_router);

    zend_hash_str_update(ht_file_router_mt,file,strlen(file),z_file_map);

}


void ref_class_method_doc_cb(char *class, char *method, char* doc_comment, void *params){

    char **router = (char**)params;

    char *c_router = router[0];
    char *file     = router[1];

    char *m_params[4];

    m_params[0] = c_router;
    m_params[1] = class;
    m_params[2] = method;
    m_params[3] = file;

    parse_annotation_filter(doc_comment,annotation_cb_dispatcher_method,m_params,"Route");

}

void annotation_cb_dispatcher_class(char *annotation, char *annotation_param, char *position, void *params){

    char **router = (char**)(params);
    char *class = router[0];
    char *file  = router[1];

    char c_router[MAXNAMLEN] = {0};
    if (strcmp(annotation,"Route") == 0)
    {
        strcpy(c_router,annotation_param);
    }else{
        char r_class[MAXPATHLEN] = {0};
        reg_replace(class,".*[\\]{1}","",r_class);
        strcpy(c_router,r_class);
    }

    char *c_params[2] = {c_router,file};

    ref_class_method_doc(class,ref_class_method_doc_cb,c_params);

}

void init_router_map(char *file, char *class, char *doc_comment){

    struct stat buf;
    if (stat(file,&buf) != 0)
    {
        zend_throw_exception(NULL,"obtain file stat err",500);
    }


    int cached = validate_dispatcher_cache(file,buf.st_mtim.tv_sec);

    // php_printf("%s -- validate res -- %d\n",file,cached);

    if (!cached)
    {

        char* router[2] = {class,file};
        parse_annotation_filter(doc_comment,annotation_cb_dispatcher_class,router,"Route");
        
    }

}

void init_dispatcher(zval *router){

    char path[MAXNAMLEN] = {0};
    reg_replace(ZSTR_VAL(Z_STR_P(router)),"\\?.*","",path);

    char router_path[MAXNAMLEN];

    l_trim(path,router_path,1);

    parse_dispatcher(router_path);

}


void dispatcher(char *s_controller, char *s_method, char router_params[10][MAXPATHLEN], int router_params_len){

    zval obj_dispatcher;
    emicro_call_static_method(emicro_dispatcher_ce,"getInstance",&obj_dispatcher);

    zend_update_property_string(emicro_dispatcher_ce,&obj_dispatcher,ZEND_STRL(EMICRO_DISPATCHER_CONTROLLER),s_controller);
    zend_update_property_string(emicro_dispatcher_ce,&obj_dispatcher,ZEND_STRL(EMICRO_DISPATCHER_METHOD),s_method);

    zval called_class_before, called_class_after, called_method_before, called_method_after;

    array_init(&called_class_before);
    array_init(&called_class_after);
    array_init(&called_method_before);
    array_init(&called_method_after);

    void *class_params[2] = {&called_class_before,&called_class_after};
    char *s_doc_class = ref_class_doc(s_controller);

    parse_annotation(s_doc_class,annotation_cb,class_params);

    void *method_params[2] = {&called_method_before,&called_method_after};
    char *s_doc_method = ref_method_doc(s_controller,s_method);

    parse_annotation(s_doc_method,annotation_cb,method_params);

    annotation_run(&called_class_before, NULL);
    annotation_run(&called_method_before, NULL);
    zval *retval = call_dispatcher(s_controller, s_method, router_params, router_params_len);
    annotation_run(&called_method_after,retval);
    annotation_run(&called_class_after,retval);

    dispatcher_return(retval);
    pefree(retval,0);
}


zval* call_dispatcher(char *class, char *method, char router_params[10][MAXPATHLEN], int router_params_len){

    zval ref_class;
    zval ctor_name, reflection, ctor_params[1];
    ZVAL_STRING(&ctor_name,"__construct");
    ZVAL_STRING(&ctor_params[0],class);
    object_init_ex(&ref_class,reflection_class_ptr);
    call_user_function(NULL,&ref_class,&ctor_name,&reflection,1,&ctor_params);

    zval obj_controller, obj_request;

    zval ref_controller_func, controller_retval;
    ZVAL_STRING(&ref_controller_func, "newInstance");
    call_user_function(NULL,&ref_class,&ref_controller_func,&obj_controller,0,NULL);

    object_init_ex(&obj_request, emicro_request_ce);

    zval func_construct, construct_retval;
    zval controllerMethod, *retval, params[router_params_len];

    retval = (zval*)pemalloc(sizeof(zval),0);

    ZVAL_STRING(&controllerMethod, method);

    for (size_t i = 0; i < router_params_len; i++)
    {
        ZVAL_STRING(&params[i], router_params[i]);
    }

    call_user_function(NULL,&obj_controller,&controllerMethod,retval,router_params_len,params);

    return retval;

}

void dispatcher_return(zval *retval){

    if (Z_TYPE_P(retval) != IS_NULL)
    {
        zval func_json_encode, json_encode_params[1], z_retval;
        
        ZVAL_STRING(&func_json_encode, "json_encode");
        ZVAL_ZVAL(&json_encode_params[0],retval,1,1);

        call_user_function(NULL,NULL,&func_json_encode,&z_retval,1,json_encode_params);

        if (Z_TYPE(z_retval) == IS_STRING)
        {
            php_printf("%s",ZSTR_VAL(Z_STR(z_retval)));
        }

    }

}

int8_t validate_dispatcher_cache(char *file,int64_t mt){

    HashTable *ht = EMICRO_G(file_router_mt);

    zval *file_map = zend_hash_str_find(ht,file,strlen(file));

    if (file_map == NULL)
    {

        return 0;
    }

    zval *c_time = zend_hash_str_find(Z_ARR_P(file_map),"c_timestamp",strlen("c_timestamp"));

    if (mt > Z_LVAL_P(c_time))
    {

        HashTable *ht_router = EMICRO_G(router);

        zval *z_c_router = zend_hash_str_find(Z_ARR_P(file_map),"c_router",strlen("c_router"));

        if (z_c_router != NULL)
        {

            zval *cur_item;
            ZEND_HASH_FOREACH_VAL(Z_ARR_P(z_c_router),cur_item){

                char *key = ZSTR_VAL(Z_STR_P(cur_item));
                zend_hash_str_del(ht_router,key,strlen(key));

            }ZEND_HASH_FOREACH_END();

            zend_hash_clean(Z_ARR_P(z_c_router));

        }
        
        return 0;
    }
    
    return 1;

}

void parse_dispatcher(char *path){

    HashTable *ht = EMICRO_G(router);

    zval *router_map, *z_controller, *z_method;

    char router_params[10][MAXPATHLEN] = {0};
    int router_params_len = 0;
    
    router_map = zend_hash_str_find(ht,path,strlen(path));

    if (router_map != NULL)
    {
        goto exec_dispatcher;
    }

    zend_string *key;
    ZEND_HASH_FOREACH_STR_KEY_VAL(ht, key, router_map){

        if (reg_match(ZSTR_VAL(key),"\:"))
        {

            router_params_len = reg_router(key->val, path, router_params);

            if (router_params_len > 0)
            {
                goto exec_dispatcher;
            }
        }

    }ZEND_HASH_FOREACH_END();

    zend_throw_exception(NULL,"router not exist \n",0);
    return;


exec_dispatcher:

    z_controller = zend_hash_index_find(Z_ARRVAL_P(router_map),0);
    z_method     = zend_hash_index_find(Z_ARRVAL_P(router_map),1);

    char *s_controller = ZSTR_VAL(Z_STR_P(z_controller));
    char *s_method     = ZSTR_VAL(Z_STR_P(z_method));

    dispatcher(s_controller, s_method, router_params, router_params_len);

}