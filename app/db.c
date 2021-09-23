#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "standard/php_string.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "../app/db.h"

zend_class_entry * emicro_db_ce;

ZEND_BEGIN_ARG_INFO(arginfo_db_table,0)
	ZEND_ARG_INFO(0,table)
ZEND_END_ARG_INFO()


PHP_METHOD(emicro_db,table){

	char *table_name;
	size_t table_len;

	ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STRING(table_name,table_len);
    ZEND_PARSE_PARAMETERS_END();



}

PHP_METHOD(emicro_db,connect){

}

PHP_METHOD(emicro_db,where){

}

PHP_METHOD(emicro_db,field){

}

PHP_METHOD(emicro_db,join){

}

PHP_METHOD(emicro_db,orderBy){

}

PHP_METHOD(emicro_db,limit){

}

PHP_METHOD(emicro_db,groupBy){

}

PHP_METHOD(emicro_db,find){

}

PHP_METHOD(emicro_db,value){

}

PHP_METHOD(emicro_db,select){

}

PHP_METHOD(emicro_db,insert){

}

PHP_METHOD(emicro_db,update){

}

PHP_METHOD(emicro_db,delete){

}

PHP_METHOD(emicro_db,beginTransaction){

}

PHP_METHOD(emicro_db,commit){

}

PHP_METHOD(emicro_db,rollback){

}

PHP_METHOD(emicro_db,getLastInsertId){

}


zend_function_entry emicro_db_methods[] = {
	PHP_ME(emicro_db, table, arginfo_db_table, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_FE_END

};


EMICRO_MODULE_D(db) {

	zend_class_entry emicro_db;
	INIT_NS_CLASS_ENTRY(emicro_db,"EMicro","DB",emicro_db_methods);
	emicro_db_ce = zend_register_internal_class( &emicro_db TSRMLS_CC);

	// zend_declare_property_null(emicro_db_ce,ZEND_STRL(),ZEND_ACC_PUBLIC);

	return SUCCESS; 
}