#ifndef EMICRO_COMMAND_H
#define EMICRO_COMMAND_H

#define EMICRO_COMMAND_INSTANCE "emicro_instance_command"
#define EMICRO_COMMAND_COLLECTION_INSTANCE "emicro_command_collection"
#define EMICRO_COMMAND_BASE_PATH "emicro_command_base_path"
#define EMICRO_COMMAND_APP_PATH "emicro_command_app_path"

extern zend_class_entry * emicro_command_ce;

void init_command_autoload();


EMICRO_MODULE_D(command);

#endif