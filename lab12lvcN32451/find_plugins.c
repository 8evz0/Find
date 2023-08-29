#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <ftw.h>
#include <unistd.h>
#include <limits.h>

#include "plugin_api.h"

#define LAB11DEBUG 0
#define log_info(M, ...) fprintf(stderr, "\n\033[5m\033[31m[INFO]\033[0m (%s:%d) " M "\n", __FILE__, __LINE__)

int Find_Plugins_Value_trigger_0=0;
int Find_Plugins_Value_trigger_1=0;
int plugin_info_trigger_;

int Tree_Walk(const char * fpath, const struct stat * st, int tflag)
{
	(void)st;
	switch(tflag)
	{
		case FTW_F:
		{
		        int scores=0;
			char *lib_name = strdup(fpath);
			void *dl = dlopen(fpath, RTLD_LAZY);
                        if (!dl) 
                        {
                            //fprintf(stderr, "ERROR: dlopen() failed: %s\n", dlerror()); В переменную окружения дебаг!
                            goto END;
                        }

                        // Check for plugin_get_info() func
                        void *func = dlsym(dl, "plugin_get_info");
                        void *func_ = dlsym(dl, "plugin_process_file");
                        
                        if (!func) 
                        {
                            //fprintf(stderr, "ERROR: dlsym() failed: %s\n", dlerror()); В переменную окружения дебаг!
                            goto END;
                           
                        }
                        else
                        {
                           scores++;
                        }
                        
                        if (!func_) 
                        {
                            //fprintf(stderr, "ERROR: dlsym() failed: %s\n", dlerror()); В переменную окружения дебаг!
                            goto END;
                        }
                        else
                        {
                           scores++;
                        }
                        
                        struct plugin_info pi = {0};
                        
                        typedef int (*pgi_func_t)(struct plugin_info*);
                        pgi_func_t pgi_func = (pgi_func_t)func;            

                        int ret = pgi_func(&pi);
                        if (ret < 0) {        
                            fprintf(stderr, "ERROR: plugin_get_info() failed\n");
                            goto END;
                        }
                      
                        if(scores==2 &plugin_info_trigger_==1)
                        {
                             Find_Plugins_Value_trigger_1++;
                            fprintf(stdout, "%d. %s\n", Find_Plugins_Value_trigger_1,lib_name);
                            // Plugin info
                            fprintf(stdout, "\tPlugin purpose:\t\t%s\n", pi.plugin_purpose);
                            fprintf(stdout, "\tPlugin author:\t\t%s\n", pi.plugin_author);
                            fprintf(stdout, "\tSupported options: ");
                            if (pi.sup_opts_len > 0) {
                                fprintf(stdout, "\n");
                                for (size_t i = 0; i < pi.sup_opts_len; i++) {
                                    fprintf(stdout, "\t--%s\t\t%s\n", pi.sup_opts[i].opt.name, pi.sup_opts[i].opt_descr);
                                }
                            }
                            else {
                                fprintf(stdout, "none (!?)\n");
                            }
                            fprintf(stdout, "\n");
                            
                        }
                        else
                        {
                          Find_Plugins_Value_trigger_0++;
                        }
                       
                        END:
                        if (lib_name) free(lib_name);
                        if (dl) dlclose(dl);
                        
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
int lib_find(int plugin_info_trigger_from_main)
{	
        plugin_info_trigger_=plugin_info_trigger_from_main;
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            if(ftw(cwd, Tree_Walk, 50)== -1)
	    {					
         	perror("ftw");
       		exit(EXIT_FAILURE);
       	    }
        }
        else 
        {
            perror("getcwd() error");
            exit(EXIT_FAILURE);
        }
        
        if(plugin_info_trigger_==1)
        {
          return  Find_Plugins_Value_trigger_1;
        }
        else if(plugin_info_trigger_==0)
        {
          return  Find_Plugins_Value_trigger_0;
        }
}
