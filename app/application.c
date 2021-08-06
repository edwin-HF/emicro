#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "main/SAPI.h"
#include "php_main.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "../php_emicro.h"
#include "../app/application.h"
#include "request.h"

zend_class_entry * emicro_application_ce;

ZEND_BEGIN_ARG_INFO(arginfo_application_dispatcherNamespace, 0)
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

PHP_METHOD(emicro_application, __construct){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_application, __clone){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_application, load){

    zval *class;
    size_t class_len;
    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_ZVAL(class);
    ZEND_PARSE_PARAMETERS_END();

    char *realpath = app_path();
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

    ZVAL_STRING(&func_name, "spl_autoload_register");
    ZVAL_STRING(&params[0], "EMicro\\Application::load");

    call_user_function(NULL,NULL,&func_name,&ret,1,params);

    dispatcher(this);

}


char* app_path(){

    zval func_name;
    zval ret_replace;
    zval params[3];
    char* realpath;
    char* root;
    root = VCWD_GETCWD(root,MAXPATHLEN);

    ZVAL_STRING(&func_name, "str_replace");
    ZVAL_STRING(&params[0], "public");
    ZVAL_STRING(&params[1], "application");
    ZVAL_STRING(&params[2], root);

    call_user_function(NULL,NULL,&func_name,&ret_replace,3,params);
    return Z_STRVAL(ret_replace);

}

char* replace(zval* str, char *find, char *replace){

    zval func_name;
    zval ret_replace;
    zval params[3];

    ZVAL_STRING(&func_name, "str_replace");
    ZVAL_STRING(&params[0], find);
    ZVAL_STRING(&params[1], replace);
    // ZVAL_ZVAL(&params[2],str,1,0);
    params[2] = *str;

    call_user_function(NULL,NULL,&func_name,&ret_replace,3,params);

    return Z_STRVAL(ret_replace);

}

char* public_path(){
    char* root;
    root = VCWD_GETCWD(root,MAXPATHLEN);
    return root;
}

int load(char *path) {

	zend_file_handle file_handle;

    zend_stream_init_filename(&file_handle, path);
    php_execute_script(&file_handle);

    return 1;

}

static void dispatcher(zval* this){

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
            // controller[0] = toupper(split_str[0]);
            continue;
        }

        if (method == NULL)
        {
            method = (split_str);
            break;
        }
        
    }

    char *nsController = emalloc(PATH_MAX);
    zval *d_controller, *rv;

    d_controller = zend_read_property(emicro_application_ce,this,ZEND_STRL(EMICRO_APPLICATION_DISPATCHER_NAMESPACE),1,rv);

    php_sprintf(nsController,"%s\\%s",Z_STR_P(d_controller)->val, controller);

    zval controllerObj,requestObj;
	zend_string *c_key = zend_string_init(nsController,strlen(nsController), 0);
	zend_class_entry *obj_ptr = zend_lookup_class(c_key);
	zend_string_free(c_key);

	object_init_ex(&controllerObj, obj_ptr);
	object_init_ex(&requestObj, emicro_request_ce);

    zval controllerMethod, retval, params[1];
    
    ZVAL_STRING(&controllerMethod, method);
    ZVAL_ZVAL(&params[0],&requestObj,1,1);

    call_user_function(NULL,&controllerObj,&controllerMethod,&retval,1,params);
    
}

zend_function_entry emicro_application_methods[] = {
    PHP_ME(emicro_application, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_application, __clone, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_application, getInstance, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_application, load, arginfo_application_load, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_application, dispatcherNamespace, arginfo_application_dispatcherNamespace, ZEND_ACC_PUBLIC)
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

	return SUCCESS; // @suppress("Symbol is not resolved")
}