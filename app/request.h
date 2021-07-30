#ifndef EMICRO_REQUEST_H
#define EMICRO_REQUEST_H

extern zend_class_entry * emicro_request_ce;

EMICRO_MODULE_D(request);

zval* parseParams(size_t type, char* key, bool_int disableDispatch);

#define queryParams(type,key) parseParams(type,key,1)
#define dispatchParams(type,key) parseParams(type,key,0)

#endif