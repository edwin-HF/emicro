#ifndef EMICRO_HELPER_H
#define EMICRO_HELPER_H

#define parse_annotation(document, callback, callback_params) \
         _parse_annotation(document, callback, callback_params, NULL)

#define parse_annotation_filter(document, callback, callback_params, filter) \ 
        _parse_annotation(document, callback, callback_params, filter)

typedef void (*scan_callback)(char *file);
typedef void (*parse_annotation_callback)(char *annotation, char *annotation_param, char *position, void *params);
typedef void* (*explode_callback)(char *a_str,size_t a_str_len);

char* replace(zval* str, char *find, char *replace);
void reg_replace(const char *str, char *pattern, char *replace, char* retval);
void scan_dir(char *path, scan_callback callback);
void _parse_annotation(char* document, parse_annotation_callback callback, void *callback_params, char *filter);
void l_trim(char *src, char *des, size_t len);
void* explode_single(char *str, char *splite, explode_callback callback);


#endif