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

ZEND_DECLARE_MODULE_GLOBALS(emicro);

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

    parse_annotation_dispatcher();

    dispatcher();

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

static void dispatcher(){

    zval* router = dispatchParams(TRACK_VARS_GET,"s");
    char router_path[MAXNAMLEN];

    l_trim(ZSTR_VAL(Z_STR_P(router)),router_path,1);

    HashTable *ht = EMICRO_G(router);

    zval* router_map = zend_hash_str_find(ht,router_path,strlen(router_path));

    zval *item;
    zend_string *key;
    int8_t index = 0;

    zval *z_controller = zend_hash_index_find(Z_ARRVAL_P(router_map),0);
    zval *z_method     = zend_hash_index_find(Z_ARRVAL_P(router_map),1);

    char *s_controller = ZSTR_VAL(Z_STR_P(z_controller));
    char *s_method     = ZSTR_VAL(Z_STR_P(z_method));

    zval obj_controller, obj_request;

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

    zval func_doc_method, doc_method[MAXPATHLEN];
    ZVAL_STRING(&func_doc_method,"getDocComment");
    call_user_function(NULL,&doc_handler,&func_doc_method,&doc_method,0,NULL);

    if (Z_STR_P(doc_method)->len > 0){
        parse_annotation(ZSTR_VAL(Z_STR_P(doc_method)),parse_annotation_method_callback,NULL);
    }

    zval ref_controller_func, controller_retval;
    ZVAL_STRING(&ref_controller_func, "newInstance");
    call_user_function(NULL,&ref_class,&ref_controller_func,&obj_controller,0,NULL);

	object_init_ex(&obj_request, emicro_request_ce);

    zval func_construct, construct_retval;
    zval controllerMethod, retval, params[1];
    
    ZVAL_STRING(&controllerMethod, s_method);
    ZVAL_ZVAL(&params[0],&obj_request,1,1);

    call_user_function(NULL,&obj_controller,&controllerMethod,&retval,1,params);

    
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

void parse_annotation_method_callback(char *annotation, char *annotation_param, void *params){

    zval app_obj, *c_rv;

    emicro_call_static_method(emicro_application_ce,"getInstance",&app_obj);

    zval *z_annotation = zend_read_property(emicro_application_ce,&app_obj,ZEND_STRL(EMICRO_APPLICATION_ANNOTATION_NAMESPACE),1,c_rv);

    char ns_annotation[MAXNAMLEN];
    php_sprintf(ns_annotation,"%s\\%s",ZSTR_VAL(Z_STR_P(z_annotation)), annotation);

    zval annotation_obj, annotation_func, annotation_retval, annotation_params[1];
    zend_string *a_key = zend_string_init(ns_annotation,strlen(ns_annotation), 0);
    zend_class_entry *obj_ptr = zend_lookup_class(a_key);
    zend_string_free(a_key);

    if (obj_ptr)
    {
        object_init_ex(&annotation_obj, obj_ptr);
    
        ZVAL_STRING(&annotation_func, "run");
        ZVAL_STRING(&annotation_params[0],annotation_param);

        call_user_function(NULL,&annotation_obj,&annotation_func,&annotation_retval,1,annotation_params);

    }

}

void parse_annotation_dispatcher_method_callback(char *annotation, char *annotation_param, void *params){

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
        php_sprintf(router_path,"%s/%s",r_router,annotation_param);
    }else{
        php_sprintf(router_path,"%s/%s",r_router,method);
    }


    HashTable *ht = EMICRO_G(router);
    zval z_router_map;
    array_init(&z_router_map);

    add_next_index_string(&z_router_map,ns_class);
    add_next_index_string(&z_router_map,method);

    zend_hash_str_update(ht,router_path,strlen(router_path),&z_router_map);

    
}

void parse_annotation_dispatcher_callback(char *annotation, char *annotation_param, void *params){

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

    if ( strcmp(annotation,"Controller") == 0)
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

        parse_annotation_filter(document,parse_annotation_dispatcher_method_callback,params,"Route");


    }ZEND_HASH_FOREACH_END();

}

void annotation_dispatcher_callback(char *file){

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

    parse_annotation_filter(class_document,parse_annotation_dispatcher_callback,router,"Controller");

}

void parse_annotation_dispatcher(){

    char *path =  app_path();
    zval app_obj,*c_rv;

    emicro_call_static_method(emicro_application_ce,"getInstance",&app_obj);
    zval *z_controller = zend_read_property(emicro_application_ce,&app_obj,ZEND_STRL(EMICRO_APPLICATION_DISPATCHER_NAMESPACE),1,c_rv);

    char root[MAXPATHLEN];
    php_sprintf(root,"%s/%s",path,ZSTR_VAL(Z_STR_P(z_controller)));

    scan_dir(root,annotation_dispatcher_callback);

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