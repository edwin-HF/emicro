/* emicro extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "php_emicro.h"
#include "app/application.h"
#include "app/dispatcher.h"
#include "app/controller.h"
#include "app/request.h"

ZEND_DECLARE_MODULE_GLOBALS(emicro);

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

/* {{{ void emicro_test1()
 */
PHP_FUNCTION(emicro_test1)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_printf("The extension %s is loaded and working!\r\n", "emicro");
}
/* }}} */

/* {{{ string emicro_test2( [ string $var ] )
 */
PHP_FUNCTION(emicro_test2)
{
	char *var = "World";
	size_t var_len = sizeof("World") - 1;
	zend_string *retval;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(var, var_len)
	ZEND_PARSE_PARAMETERS_END();

	retval = strpprintf(0, "Hello %s", var);

	RETURN_STR(retval);
}

PHP_GINIT_FUNCTION(emicro) {

}

PHP_MINIT_FUNCTION(emicro){

	EMICRO_STARTUP(application);
	EMICRO_STARTUP(request);

	return SUCCESS;

}

/* }}}*/

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(emicro)
{
#if defined(ZTS) && defined(COMPILE_DL_EMICRO)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(emicro){
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(emicro){
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(emicro)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "emicro support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO(arginfo_emicro_test1, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_emicro_test2, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ emicro_functions[]
 */
static const zend_function_entry emicro_functions[] = {
	PHP_FE(emicro_test1,		arginfo_emicro_test1)
	PHP_FE(emicro_test2,		arginfo_emicro_test2)
	PHP_FE_END
};
/* }}} */

/* {{{ emicro_module_entry
 */
zend_module_entry emicro_module_entry = {
	STANDARD_MODULE_HEADER,
	"emicro",					/* Extension name */
	emicro_functions,			/* zend_function_entry */
	PHP_MINIT(emicro),							/* PHP_MINIT - Module initialization */
	PHP_MSHUTDOWN(emicro),							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(emicro),			/* PHP_RINIT - Request initialization */
	PHP_RSHUTDOWN(emicro),							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(emicro),			/* PHP_MINFO - Module info */
	PHP_EMICRO_VERSION,		/* Version */
	PHP_MODULE_GLOBALS(emicro),
	PHP_GINIT(emicro),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_EMICRO
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(emicro)
#endif
