#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ftw.h>
#include <unistd.h>

#include "find_plugins.h"
#define LAB11DEBUG 0

#define log_info(M, ...) fprintf(stderr, "\n\033[5m\033[31m[INFO]\033[0m (%s:%d) " M "\n", __FILE__, __LINE__)
char *target_;
int switch_lib;
const char *path_;
//Функция чтения и удаления из файла пробелов, табуляции и символа переноса строки. А также при нахождении совокупностей символов вида 0x12 0xf2 и приведения их к виду 12f2

int Module_Switch(int,const char* )
{
	// запуск модуля в зависимости от выбранной опции
	switch(switch_lib)
	{
		case 1: // 1 - find file with hex values by cyrillic string (0xab 0x2d ....) 
		{
			void *string_in_hex_lib= dlopen("./lib_find_string_target_in_hex.so",RTLD_LAZY | RTLD_LOCAL);
			if (!string_in_hex_lib)  
			{
				fprintf(stdout,"\033[5m\033[31mERROR\033[0m dlopen: %s\n\t\n", dlerror());
       				exit(EXIT_FAILURE);
			}
			int (*module_string_in_hex)(char*,const char*) = dlsym(string_in_hex_lib, "string_in_hex");			
			if (module_string_in_hex == NULL)
			{
				fprintf(stdout,"\033[5m\033[31mERROR\033[0m dlsym: %s\n", dlerror());
       				exit(EXIT_FAILURE);
			}
			
			#if LAB11DEBUG==1
			fprintf(stdout,"...module_string_in_hex(target_)\n");
			#endif
			
			if(module_string_in_hex(target_,path_) ==0)
			{ 
				fprintf(stdout,"\n\033[32mPATH TO FILE\033[0m\t%s\n",path_);
				dlclose(string_in_hex_lib);
			}
			else
			{
				dlclose(string_in_hex_lib);
			}
			break;
			
		}
		case 2: // 2 - find file by string (fsdfa/das31/3412/ыва/аыва23) 
		{
			void *string_lib= dlopen("./lib_find_string_target.so",RTLD_LAZY | RTLD_LOCAL);
			if (!string_lib)  
			{
				fprintf(stdout,"\033[5m\033[31mERROR\033[0m dlopen: %s\n\t\n", dlerror());
       				exit(EXIT_FAILURE);
			}
			int (*module_string)(char*,const char*) = dlsym(string_lib, "For_String");			
			if (module_string == NULL)
			{
				fprintf(stdout,"\033[5m\033[31mERROR\033[0m dlsym: %s\n", dlerror());
       				exit(EXIT_FAILURE);
			}
			
			#if LAB11DEBUG==1
			fprintf(stdout,"...module_string)\n");
			#endif
			
			if(module_string(target_,path_) ==0)
			{ 
				fprintf(stdout,"\n\033[32mPATH TO FILE\033[0m\t%s\n",path_);
				dlclose(string_lib);
			}
			else
			{
				dlclose(string_lib);
			}
			break;
		}
	}
}

int Walk_Tree(const char * fpath, const struct stat * st, int tflag)
{
	(void)st;
	switch(tflag)
	{
		case FTW_F:
		{
			path_=fpath;
			#if LAB11DEBUG==1
			fprintf(stdout,"path_to_file\t%s\n",fpath);
			#endif
			Module_Switch(switch_lib,fpath);
		};
	        case FTW_D:
	        {
	        	break;
	        };
	        case FTW_SL:
	        {
	        	return -1;
	        };
	        case FTW_NS:
	        {
	        	return -1;
	        };
	        case FTW_DNR:
	        {
	        	return -1;
		};
	}
	return 0;
}
int ftw_find(char* fpath,char*target_from_lab11lvcN32451,int int_for_switch_from_lab11llvcN32451)
{		
	#if LAB11DEBUG==1
	fprintf(stdout,"...ftw_find(char* fpath,char*target_from_lab11lvcN32451,int int_for_switch_from_lab11llvcN32451,char *DPath)\n");
	#endif
	
	switch_lib=int_for_switch_from_lab11llvcN32451;
	target_=target_from_lab11lvcN32451;


	if(ftw(fpath, Walk_Tree, 50)== -1)
	{					
         	perror("ftw");
       		exit(EXIT_FAILURE);
       	}
}
