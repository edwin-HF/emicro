#ifndef EMICRO_ANNOTATION_H
#define EMICRO_ANNOTATION_H

void annotation_run(zval *list, zval *retval);
char* ref_class_doc(char *class);
char* ref_method_doc(char *class, char* method);
void annotation_cb(char *annotation, char *annotation_param, char *position, void *params);

#endif