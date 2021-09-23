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

zend_class_entry * emicro_dispatcher_ce;

ZEND_DECLARE_MODULE_GLOBALS(emicro);

ZEND_BEGIN_ARG_INFO(arginfo_emicro_sayHello, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(emicro_dispatcher, __construct){

	zval* router = dispatchParams(TRACK_VARS_GET,"s");
    char* str_router = Z_STRVAL_P(router);
    char* delimiter = "/";
    char* split_save_pointer;
    char* split_str;
    char* controller = NULL;
    char* method = NULL;
    while (NULL != (split_str = php_strtok_r(str_router,delimiter,&split_save_pointer)))
    {
        str_router = NULL;

        if (controller == NULL)
        {
            controller = split_str;
            continue;
        }

        if (method == NULL)
        {
            method = (split_str);
            break;
        }
        
    }

}

PHP_METHOD(emicro_dispatcher, moudle){

}

PHP_METHOD(emicro_dispatcher, controller){

}

PHP_METHOD(emicro_dispatcher, method){

}

PHP_METHOD(emicro_dispatcher, defaultNamespace){

}

PHP_METHOD(emicro_dispatcher, annotationNamespace){

}


PHP_METHOD(emicro_dispatcher, getInstance){
    ZEND_PARSE_PARAMETERS_NONE();

    zval *instance = zend_read_static_property(emicro_dispatcher_ce, ZEND_STRL(EMICRO_DISPATCHER_INSTANCE), 1);

    if (Z_TYPE_P(instance) == IS_OBJECT)
    {
        RETURN_ZVAL(instance,1,0);
    }

    if (object_init_ex(instance,emicro_dispatcher_ce) == FAILURE)
    {
	    php_printf("err obtain application instance\n");
        return;
    }

	zval func_construct;
	ZVAL_STRING(&func_construct, "__construct");
    call_user_function(NULL,instance,&func_construct,NULL,0,NULL);

	zend_update_static_property(emicro_dispatcher_ce, ZEND_STRL(EMICRO_DISPATCHER_INSTANCE), instance);

    RETURN_ZVAL(instance,1,0);
}

zend_function_entry emicro_dispatcher_methods[] = {
	PHP_ME(emicro_dispatcher, getInstance, NULL, ZEND_ACC_PUBLIC)
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

	emicro_dispatcher_ce = zend_register_internal_class( &emicro_dispatcher TSRMLS_CC);

	return SUCCESS; 
}

static char* ref_class_doc(char *class){

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

void annotation_cb_dispatcher_method(char *annotation, char *annotation_param , char *position,  void *params){

    char **router = (char**)(params);
    char *r_router = router[0];
    char *ns_class = router[1];
    char *method   = router[2];
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
    zval z_router_map;
    array_init(&z_router_map);

    add_next_index_string(&z_router_map,ns_class);
    add_next_index_string(&z_router_map,method);

    zend_hash_str_update(ht,router_path,strlen(router_path),&z_router_map);

    
}

void annotation_cb_dispatcher_class(char *annotation, char *annotation_param, char *position, void *params){

    char **router = (char**)(params);
    char ns_class[MAXNAMLEN];
    char *ns = router[0];
    char *class = router[1];
    php_sprintf(ns_class,"%s\\%s",ns,class);

    zval reflection_class;
    zval ctor_name, reflection, ctor_params[1];
    ZVAL_STRING(&ctor_name,"__construct");
    ZVAL_STRING(&ctor_params[0],ns_class);

    object_init_ex(&reflection_class,reflection_class_ptr);
    call_user_function(NULL,&reflection_class,&ctor_name,&reflection,1,&ctor_params);

    zval func_methods, doc;
    ZVAL_STRING(&func_methods,"getMethods");
    call_user_function(NULL,&reflection_class,&func_methods,&doc,0,NULL);

    zval *method_obj,*rv;

    char c_router[MAXNAMLEN] = {0};

    if (strcmp(annotation,"Controller") == 0)
    {
        strcpy(c_router,annotation_param);
    }else{
        strcpy(c_router,class);
    }

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL(doc),method_obj){

        zval *method = zend_read_property(reflection_method_ptr,method_obj,ZEND_STRL("name"),1,rv);
        char *str_method = ZSTR_VAL(Z_STR_P(method));
        
        zval func_ref_method,doc_handler;
        zval func_ref_method_params[1] = {method};

        ZVAL_STRING(&func_ref_method,"getMethod");
        ZVAL_STRING(&func_ref_method_params[0],str_method);

        call_user_function(NULL,&reflection_class,&func_ref_method,&doc_handler,1,&func_ref_method_params);
        
        zval func_doc_method, doc_method;
        ZVAL_STRING(&func_doc_method,"getDocComment");
        call_user_function(NULL,&doc_handler,&func_doc_method,&doc_method,0,NULL);

        char *document = ZSTR_VAL(Z_STR(doc_method));
        char *params[3];


        if (Z_TYPE(doc_method) == IS_FALSE)
        {
            document = "";
        }

        params[0] = c_router;
        params[1] = ns_class;
        params[2] = str_method;

        parse_annotation_filter(document,annotation_cb_dispatcher_method,params,"Route");


    }ZEND_HASH_FOREACH_END();

}

void scan_cb_dispatcher(char *file){


    // struct stat buf;
    // if (stat(file,&buf) != 0)
    // {
    //     zend_throw_exception(NULL,"obtain file stat err",500);
    // }
    // php_printf("last modify time %s\n",ctime(&buf.st_mtim));
    

    char* filename = basename(file);
    char class[MAXNAMLEN] = {0};
    char nsController[MAXNAMLEN] = {0};
    strncpy(class,filename,strlen(filename) - 4);
    zval app_obj,*c_rv;
    emicro_call_static_method(emicro_application_ce,"getInstance",&app_obj);

    zval *z_controller = zend_read_property(emicro_application_ce,&app_obj,ZEND_STRL(EMICRO_APPLICATION_DISPATCHER_NAMESPACE),1,c_rv);

    char *ns = ZSTR_VAL(Z_STR_P(z_controller));

    php_sprintf(nsController,"%s\\%s",ns, class);

    char *class_document = ref_class_doc(nsController);

    char* router[2] = {ns,class};

    parse_annotation_filter(class_document,annotation_cb_dispatcher_class,router,"Controller");

}

void init_router_map(){

    zval app_obj,*c_rv;

    zval *z_path = zend_read_static_property(emicro_application_ce,ZEND_STRL(EMICRO_APPLICATION_APP_PATH),1);
    char *path = ZSTR_VAL(Z_STR_P(z_path));

    emicro_call_static_method(emicro_application_ce,"getInstance",&app_obj);
    zval *z_controller = zend_read_property(emicro_application_ce,&app_obj,ZEND_STRL(EMICRO_APPLICATION_DISPATCHER_NAMESPACE),1,c_rv);

    char root[MAXPATHLEN];
    php_sprintf(root,"%s/%s",path,ZSTR_VAL(Z_STR_P(z_controller)));

    scan_dir(root,scan_cb_dispatcher);

}


void dispatcher(){

    zval* router = dispatchParams(TRACK_VARS_GET,"s");
    char router_path[MAXNAMLEN];

    l_trim(ZSTR_VAL(Z_STR_P(router)),router_path,1);

    HashTable *ht = EMICRO_G(router);

    zval* router_map = zend_hash_str_find(ht,router_path,strlen(router_path));

    if (router_map == NULL)
    {
        zend_throw_exception(NULL,"router not exist \n",0);
        return;
    }
    
    zval *item;
    zend_string *key;
    int8_t index = 0;

    zval *z_controller = zend_hash_index_find(Z_ARRVAL_P(router_map),0);
    zval *z_method     = zend_hash_index_find(Z_ARRVAL_P(router_map),1);

    char *s_controller = ZSTR_VAL(Z_STR_P(z_controller));
    char *s_method     = ZSTR_VAL(Z_STR_P(z_method));

    zval ref_class;
    zval ctor_name, reflection, ctor_params[1];
    ZVAL_STRING(&ctor_name,"__construct");
    ZVAL_STRING(&ctor_params[0],s_controller);

    object_init_ex(&ref_class,reflection_class_ptr);
    call_user_function(NULL,&ref_class,&ctor_name,&reflection,1,&ctor_params);


    zval func_reflection_method,doc_handler;
    zval func_reflection_method_params[1];

    ZVAL_STRING(&func_reflection_method,"getMethod");
    ZVAL_STRING(&func_reflection_method_params[0],s_method);

    call_user_function(NULL,&ref_class,&func_reflection_method,&doc_handler,1,&func_reflection_method_params);

    zval func_doc_method, doc_method;
    ZVAL_STRING(&func_doc_method,"getDocComment");
    call_user_function(NULL,&doc_handler,&func_doc_method,&doc_method,0,NULL);

    zval called_before, called_after;

    array_init(&called_before);
    array_init(&called_after);

    void *params[2] = {&called_before,&called_after};
    char *s_doc_method = "";

    if (Z_TYPE(doc_method) != IS_FALSE)
    {
        s_doc_method = ZSTR_VAL(Z_STR(doc_method));
    }

    parse_annotation(s_doc_method,annotation_cb_dispatcher,params);

    zval *z_item;
    ZEND_HASH_FOREACH_VAL(Z_ARR(called_before),z_item){

        zval *z_class  = zend_hash_index_find(Z_ARR_P(z_item),0);
        zval *z_method = zend_hash_index_find(Z_ARR_P(z_item),1);

        call_dispatcher_annotation(ZSTR_VAL(Z_STR_P(z_class)),ZSTR_VAL(Z_STR_P(z_method)),NULL);

    }ZEND_HASH_FOREACH_END();

    zval *retval = call_dispatcher(&ref_class,s_method);

    ZEND_HASH_FOREACH_VAL(Z_ARR(called_after),z_item){

        zval *z_class  = zend_hash_index_find(Z_ARR_P(z_item),0);
        zval *z_method = zend_hash_index_find(Z_ARR_P(z_item),1);

        call_dispatcher_annotation(ZSTR_VAL(Z_STR_P(z_class)),ZSTR_VAL(Z_STR_P(z_method)),retval);

    }ZEND_HASH_FOREACH_END();

    
}

void annotation_cb_dispatcher(char *annotation, char *annotation_param, char *position, void *params){

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

zval* call_dispatcher(zval *ref_class, char *method){

    zval obj_controller, obj_request;

    zval ref_controller_func, controller_retval;
    ZVAL_STRING(&ref_controller_func, "newInstance");
    call_user_function(NULL,ref_class,&ref_controller_func,&obj_controller,0,NULL);

    object_init_ex(&obj_request, emicro_request_ce);

    zval func_construct, construct_retval;
    zval controllerMethod, *retval, params[1];

    retval = (zval*)emalloc(sizeof(zval));
    
    ZVAL_STRING(&controllerMethod, method);
    ZVAL_ZVAL(&params[0],&obj_request,1,1);

    call_user_function(NULL,&obj_controller,&controllerMethod,retval,1,params);

    return retval;

}

void call_dispatcher_annotation(char *annotation, char *annotation_param, zval *retval){
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

        efree(retval);
    }
}