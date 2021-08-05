#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "../php_emicro.h"
#include "../app/application.h"
#include "request.h"

zend_class_entry * emicro_application_ce;

ZEND_BEGIN_ARG_INFO(arginfo_application_registerNamespace, 0)
    ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

PHP_METHOD(emicro_application, registerNamespace){
    
    zval* obj = getThis();
    zval* params;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_ARRAY(params);
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property(emicro_application_ce,obj,ZEND_STRL(EMICRO_APPLICATION_NAMESPACE),params);

    RETURN_OBJ(obj);

}

PHP_METHOD(emicro_application, __construct){
    ZEND_PARSE_PARAMETERS_NONE();
}

PHP_METHOD(emicro_application, __clone){
    ZEND_PARSE_PARAMETERS_NONE();
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

    zval* namespace;

    namespace = zend_read_property(emicro_application_ce,this,ZEND_STRL(EMICRO_APPLICATION_NAMESPACE),1,namespace);

    autoload(controller, namespace);

    // autoload
    // php_printf("controller：%s\n",controller);
    // php_printf("method：%s\n",method);

    char *nsController = emalloc(sizeof(controller) + 12);
    php_sprintf(nsController,"controller\\%s",controller);

    zval controllerObj;
	zend_string *c_key = zend_string_init(nsController,strlen(nsController), 0);
	zend_class_entry *obj_ptr = zend_lookup_class(c_key);
	zend_string_free(c_key);

	object_init_ex(&controllerObj, obj_ptr);

    zval controllerMethod;
    zval retval;
    ZVAL_STRING(&controllerMethod, method);

    call_user_function(NULL,&controllerObj,&controllerMethod,&retval,0,NULL);

}


zend_function_entry emicro_application_methods[] = {
    PHP_ME(emicro_application, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_application, __clone, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(emicro_application, getInstance, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(emicro_application, run, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(emicro_application, registerNamespace, arginfo_application_registerNamespace, ZEND_ACC_PUBLIC)
    { NULL, NULL, NULL }

};

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

char* public_path(){
    char* root;
    root = VCWD_GETCWD(root,MAXPATHLEN);
    return root;
}

void autoload(char* class, zval *namespace){

    char* loadClass = (char*)emalloc(sizeof(class));
    strcpy(loadClass,class);

    char* realpath = app_path();

    zval *item;
    ZEND_HASH_FOREACH_VAL(Z_ARR_P(namespace), item){

        char *tmp = ZSTR_VAL(Z_STR_P(item));
        char *className = emalloc(sizeof(realpath) + sizeof(tmp) + 6);
        php_sprintf(className,"%s/%s/%s.php",realpath,tmp,loadClass);
        load(className);
        efree(className);

    }ZEND_HASH_FOREACH_END();

    efree(loadClass);

}

int load(char *path) {

	zend_file_handle file_handle;
	char *realpath;

	realpath = (char *) ecalloc(MAXPATHLEN, sizeof(char));

	if (!VCWD_REALPATH(path, realpath)) {
		return 0;
	}

	efree(realpath);

    zend_stream_init_filename(&file_handle, (char *) path);

	zend_compile_file(&file_handle, ZEND_REQUIRE_ONCE);

	if (file_handle.handle.stream.handle) {
		if (!file_handle.opened_path) {
			file_handle.opened_path = zend_string_init(path, strlen(path), 0);
		}

		zend_hash_add_empty_element(&EG(included_files),
				file_handle.opened_path);
	}

	zend_destroy_file_handle(&file_handle);

    return 1;

}


EMICRO_MODULE_D(application) {
	zend_class_entry emicro_application;
	INIT_NS_CLASS_ENTRY(emicro_application,"EMicro","Application",emicro_application_methods);
	emicro_application_ce = zend_register_internal_class( &emicro_application TSRMLS_CC);
	emicro_application_ce->ce_flags |= ZEND_ACC_FINAL;

	//static
	zend_declare_property_null(emicro_application_ce, ZEND_STRL(EMICRO_APPLICATION_INSTANCE), ZEND_ACC_PRIVATE | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(emicro_application_ce, ZEND_STRL(EMICRO_APPLICATION_NAMESPACE), ZEND_ACC_PUBLIC TSRMLS_CC);

	return SUCCESS; // @suppress("Symbol is not resolved")
}