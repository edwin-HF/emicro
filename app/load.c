#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "../php_emicro.h"
#include "../app/load.h"

int execute(char *path) {
	zend_file_handle file_handle;
	zend_op_array *op_array;
	char *realpath;

	realpath = (char *) ecalloc(MAXPATHLEN, sizeof(char));

	if (!VCWD_REALPATH(path, realpath)) {
		return 0;
	}
	efree(realpath);

	file_handle.filename = path;
	file_handle.free_filename = 0;
	file_handle.type = ZEND_HANDLE_FILENAME;
	file_handle.opened_path = NULL;
	file_handle.handle.fp = NULL;

	op_array = zend_compile_file(&file_handle, ZEND_INCLUDE);

	if (op_array && file_handle.handle.stream.handle) {
		if (!file_handle.opened_path) {
			file_handle.opened_path = zend_string_init(path, strlen(path), 0);
		}

		zend_hash_add_empty_element(&EG(included_files),
				file_handle.opened_path);
	}
	zend_destroy_file_handle(&file_handle);

	if (op_array) {
		zval result;

		ZVAL_UNDEF(&result);
		zend_execute(op_array, &result);

		destroy_op_array(op_array);
		efree_size(op_array, sizeof(op_array));
		if (!EG(exception)) {
			zval_ptr_dtor(&result);
		}
		return 1;
	}
	return 0;
}