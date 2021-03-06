#include "php.h"
#include "php_ini.h"
#include "standard/php_filestat.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include <regex.h>
#include "helper.h"

char* replace(zval* str, char *find, char *replace){

    zval func_name;
    zval ret_replace;
    zval params[3];

    ZVAL_STRING(&func_name, "str_replace");
    ZVAL_STRING(&params[0], find);
    ZVAL_STRING(&params[1], replace);
    params[2] = *str;

    call_user_function(NULL,NULL,&func_name,&ret_replace,3,params);

    return Z_STRVAL(ret_replace);

}

int reg_match(const char *str, char *pattern){

    char *seek_ptr = str;
    int find = 0;

    regex_t pattern_compiled;
    
    regcomp(&pattern_compiled,pattern,REG_EXTENDED | REG_NEWLINE);

    while (1)
    {

        regmatch_t pmatch[1] = {};
        size_t     nmatch = 1;

        int reg_ret = regexec(&pattern_compiled,seek_ptr,nmatch,pmatch,0);

        if (reg_ret != REG_NOERROR || reg_ret == REG_NOMATCH)
        {
            break;
        }else{
            find = 1;
        }
        
        seek_ptr += pmatch[0].rm_eo;
    }

    regfree(&pattern_compiled);

    return find;

}

int reg_router(const char *path, const char *uri, char router_params[10][MAXPATHLEN]){

    char *seek_ptr = uri;
    int router_param_count = 0;

    char pattern[MAXPATHLEN] = {0};
    reg_replace(path,"\:[^\/]*","([^\/]+)",pattern);

    regex_t pattern_compiled;
    
    regcomp(&pattern_compiled,pattern,REG_EXTENDED | REG_NEWLINE);

    while (1)
    {

        regmatch_t pmatch[10] = {};
        size_t     nmatch = 10-1;

        int reg_ret = regexec(&pattern_compiled,seek_ptr,nmatch,pmatch,0);

        if (reg_ret != REG_NOERROR || reg_ret == REG_NOMATCH)
        {
            break;
        }else{

            for (size_t i = 0; i < nmatch; i++)
            {

                char params[MAXPATHLEN] = {0};
                char params_len = 0;
                for (size_t j = pmatch[i+1].rm_so; j < pmatch[i+1].rm_eo; j++)
                {
                    params[params_len++] = seek_ptr[j];
                }

                if (params_len > 0)
                {
                    strcpy(router_params[router_param_count++],params);
                }

            }
            
        }
        
        seek_ptr += pmatch[0].rm_eo;
    }

    regfree(&pattern_compiled);

    return router_param_count;

}

void reg_replace(const char *str, char *pattern, char *replace, char* str_replace){

    char *seek_ptr = str;

    regex_t pattern_compiled;
    
    regcomp(&pattern_compiled,pattern,REG_EXTENDED);
    size_t size_replace = 0;

    while (1)
    {

        size_t size_replace = strlen(str_replace);
        regmatch_t pmatch[1] = {};
        size_t     nmatch = 1;

        int reg_ret = regexec(&pattern_compiled,seek_ptr,nmatch,pmatch,0);

        if (reg_ret != REG_NOERROR || reg_ret == REG_NOMATCH)
        {
            char tmp[MAXPATHLEN] = {0};
            for (size_t idx = 0; idx < strlen(seek_ptr); idx++)
            {
                tmp[idx] = seek_ptr[idx];
            }
            strcat(str_replace,tmp);
            break;
        }

        for (size_t idx = 0; idx < pmatch[0].rm_so; idx++)
        {
            str_replace[size_replace++] = seek_ptr[idx];
        }

        strcat(str_replace,replace);

        seek_ptr += pmatch[0].rm_eo;
    }

    regfree(&pattern_compiled);

}

void scan_dir(char *path, scan_callback callback){

    DIR *dir;
    struct dirent *ptr;
    char base[1000];
 
    if ((dir=opendir(path)) == NULL)
    {
         php_printf("Open dir error...");
         exit(1);
    }

    while ((ptr=readdir(dir)) != NULL)
    {
         if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0){
            continue;
         }else if(ptr->d_type == 8){ // file
            char file[MAXNAMLEN];
            php_sprintf(file,"%s/%s",path,ptr->d_name);
            callback(file);
         }else if(ptr->d_type == 4){ //dir
             memset(base,'\0',sizeof(base));
             strcpy(base,path);
             strcat(base,"/");
             strcat(base,ptr->d_name);
             scan_dir(base,callback);
         }
     }
     closedir(dir);

}

void _parse_annotation(char* document, parse_annotation_callback callback, void *callback_params, char *filter){

    
    if (strlen(document) > 0)
    {
        regex_t pattern_compiled;
        char *pattern = "@(\\w*?)\\(?([^\\)\\!]*?)?\\)?\\s*!?\\s*(\\w*)?";
        regcomp(&pattern_compiled,pattern,REG_EXTENDED|REG_NEWLINE);

        char *seek_ptr = document;
        char filter_annotation[MAXNAMLEN] = {0};
        char filter_param[MAXNAMLEN]      = {0};
        char filter_position[MAXNAMLEN]   = {0};

        while (1)
        {

            regmatch_t pmatch[4] = {};
            size_t     nmatch = 4;
            int reg_ret = regexec(&pattern_compiled,seek_ptr,nmatch,pmatch,0);

            if (reg_ret != 0)
            {
                break;
            }
            
            char annotation[255] = {};
            char param[255]      = {};
            char position[255]   = {};

            for (size_t i = 0; i < nmatch; i++)
            {
                size_t bi = 0;
                for (size_t j = pmatch[i].rm_so; j < pmatch[i].rm_eo; j++)
                {
                    if (i == 1)
                    {
                        annotation[bi] = seek_ptr[j];
                    }
                    if (i == 2)
                    {
                        param[bi] = seek_ptr[j];
                    }
                    if (i == 3)
                    {
                        position[bi] = seek_ptr[j];
                    }
                    bi++;
                }

            }


            if (filter == NULL)
            {
                callback(annotation,param,position,callback_params);
            }else{
                if (strcmp(annotation,filter) == 0)
                {
                    strcpy(filter_annotation,annotation);
                    strcpy(filter_param,param);
                    strcpy(filter_position,position);
                }
            }
            

            seek_ptr += pmatch[0].rm_eo;
            
        }

        if (filter != NULL)
        {
            callback(filter_annotation,filter_param,filter_position,callback_params);
        }

        regfree (&pattern_compiled);

    }else{
        callback("","","",callback_params);
    }
    

}

void l_trim(char *src, char *des, size_t len){

    if (len < 0 || len == 0)
    {
        strcpy(des,src);
    }
    
    char ret[MAXNAMLEN] = {0};

    for (size_t i = 0; i < strlen(src); i++)
    {
        if (i < len)
        {
            continue;
        }

        ret[i-len] = src[i];
        
    }

    strcpy(des,ret);

}


void* explode_single(char *str, char *splite, explode_callback callback){

    size_t len = 0;
    char *a_str[255];
    char *cur_str = strtok(str,splite);

    while (cur_str)
    {
        a_str[len] = cur_str;
        len++;
        cur_str = strtok(NULL,splite);
    }

    return callback(a_str,len);

}