#ifndef EMICRO_DB_H
#define EMICRO_DB_H

extern zend_class_entry * emicro_db_ce;

EMICRO_MODULE_D(db);

#define EMICRO_DB_TABLE "emicro_db_table"
#define EMICRO_DB_DSN "emicro_db_dsn"

#endif