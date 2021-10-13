#ifndef EMICRO_ANNOTATION_H
#define EMICRO_ANNOTATION_H

typedef void (*ref_method_doc_cb)(char *class, char* method, char* doc_comment, void *c_params);

void annotation_run(zval *list, zval *retval);
void annotation_scan_cb(char* file);
void annotation_cb(char *annotation, char *annotation_param, char *position, void *params);
char* ref_class_doc(char *class);
char* ref_method_doc(char *class, char* method);
void ref_class_method_doc(char *class,ref_method_doc_cb ref_cb,void *params);
void init_annotation_map(file, class, class_document);
int8_t validate_annotation_cache(char *file, int64_t mt);

#endif