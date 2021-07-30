/* emicro extension for PHP */

#ifndef PHP_EMICRO_H
# define PHP_EMICRO_H

extern zend_module_entry emicro_module_entry;
# define phpext_emicro_ptr &emicro_module_entry

# define PHP_EMICRO_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_EMICRO)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

ZEND_BEGIN_MODULE_GLOBALS (emicro)

ZEND_END_MODULE_GLOBALS (emicro)

#define EMICRO_MODULE_D(module) int emicro_##module()
#define EMICRO_STARTUP(module) emicro_##module()


#endif	/* PHP_EMICRO_H */
