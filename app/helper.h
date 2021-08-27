#ifndef EMICRO_HELPER_H
#define EMICRO_HELPER_H

#define parse_annotation(document, callback, callback_params) \
         _parse_annotation(document, callback, callback_params, NULL)

#define parse_annotation_filter(document, callback, callback_params, filter) \ 
        _parse_annotation(document, callback, callback_params, filter)

typedef void (*scan_callback)(char *file);
typedef void (*parse_annotation_callback)(char *annotation, char *annotation_param, void *params);

char* replace(zval* str, char *find, char *replace);
void scan_dir(char *path, scan_callback callback);
void _parse_annotation(char* document, parse_annotation_callback callback, void *callback_params, char *filter);
void l_trim(char *src, char *des, size_t len);


#endif