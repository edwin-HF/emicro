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

ZEND_DECLARE_MODULE_GLOBALS(emicro);

ZEND_BEGIN_ARG_INFO(arginfo_emicro_sayHello, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(emicro_dispatcher, controller){
    zval *rv;
    zval *retval = zend_read_property(emicro_dispatcher_ce,getThis(),ZEND_STRL(EMICRO_DISPATCHER_CONTROLLER),1,rv);

    RETURN_ZVAL(retval,1,1);

}

PHP_METHOD(emicro_dispatcher, method){
    zval *rv;
    zval *retval = zend_read_property(emicro_dispatcher_ce,getThis(),ZEND_STRL(EMICRO_DISPATCHER_METHOD),1,rv);
    RETURN_ZVAL(retval,1,1);
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

    if (strcmp(annotation,"Route") == 0)
    {
        strcpy(c_router,annotation_param);
    }else{
        char *r_class = reg_replace(class,"[\\]{1}","/");
        strcpy(c_router,r_class);
        efree(r_class);
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
    


    zval app_obj,*c_rv;
    emicro_call_static_method(emicro_application_ce,"getInstance",&app_obj);
    zval *z_controller = zend_read_property(emicro_application_ce,&app_obj,ZEND_STRL(EMICRO_APPLICATION_DISPATCHER_NAMESPACE),1,c_rv);

    char *ns = ZSTR_VAL(Z_STR_P(z_controller));

    char pattern[255];
    php_sprintf(pattern,".*%s/",ns);

    char *filename = reg_replace(file,pattern,"");
    char *ns_class = reg_replace(filename,"/","\\");

    char class[MAXNAMLEN] = {0};
    char nsController[MAXNAMLEN] = {0};
    strncpy(class,ns_class,strlen(filename) - 4);

    php_sprintf(nsController,"%s\\%s",ns, class);
    
    char *class_document = ref_class_doc(nsController);

    char* router[2] = {ns,class};

    parse_annotation_filter(class_document,annotation_cb_dispatcher_class,router,"Route");
    
    efree(filename);
    efree(ns_class);
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
    zval *retval = call_dispatcher(s_controller,s_method);
    annotation_run(&called_method_after,retval);
    annotation_run(&called_class_after,retval);

    dispatcher_return(retval);
    efree(retval);
}


zval* call_dispatcher(char *class, char *method){

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
    zval controllerMethod, *retval, params[1];

    retval = (zval*)emalloc(sizeof(zval));
    
    ZVAL_STRING(&controllerMethod, method);
    ZVAL_ZVAL(&params[0],&obj_request,1,1);

    call_user_function(NULL,&obj_controller,&controllerMethod,retval,1,params);

    return retval;

}

void dispatcher_return(zval *retval){

    if (Z_TYPE_P(retval) != IS_NULL)
    {
        zval func_json_encode, json_encode_params[1], z_retval;
        
        ZVAL_STRING(&func_json_encode, "json_encode");
        ZVAL_ZVAL(&json_encode_params[0],retval,1,1);

        call_user_function(NULL,NULL,&func_json_encode,&z_retval,1,json_encode_params);

        php_printf("%s",ZSTR_VAL(Z_STR(z_retval)));
    }

}