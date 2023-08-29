#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <ftw.h>
#include <fts.h>
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "walk_tree_and_switch_module.h"
#include "find_plugins.h"
#include "plugin_api.h"
#define LAB1DEBUG 0

#define log_info(M, ...) fprintf(stderr, "\n\033[31m[FATAL ERROR]\033[0m (%s:%d) " M "\n", __FILE__, __LINE__)

/*
#define loadingBar() \
{\
	int j,i=0;\
	for(;i<=100;i++)\
	{\
		fprintf(stdout,"\r\033[33mLoading \033[32m[");\
		for(j=0;j<i;j+=5)\
		{\
			fprintf(stdout,"#");\
		}\
		for(;j<100;j+=5)\
		{\
			fprintf(stdout," ");\
		}\
		fprintf(stdout,"]\033[33m %3d%% ",i);\
		fflush(stdout);\
		usleep(10000);\
	}\
	fprintf(stdout,"\n\033[0m");\
}\
*/

// LAB1.1
bool flag;
int i;
char *target,*str2,*str3,*str,*str4,*arr,*path,*path_to_lib;
char *check;
int for_switch;

// LAB1.2
int plugin_info_trigger=0;
int plugin_opt_trigger;
int Work_Type;
char *str_p;
const char *path_to_plugin;
int count_of_found_plugins;
int is_N = 0 ,is_P = 0,is_O = 0;
int len = 0;
void** dl = 0;

// Prototypes
int With_Third_Party_Plugins(int argc__, char* argv__[]);
int check_file_access(const char* , int );
typedef int (*ppf_func_t)(const char*, struct option*, size_t); //plugin_process_file
typedef int (*pgi_func_t)(struct plugin_info*); // plugin_get_info;

struct longopt 
{
    struct option *all_opt ; 
    size_t all_opt_len ; 
    struct option *opts_to_pass ; 
    size_t optToPass ; 
    ppf_func_t func; // plugin_process_file
    pgi_func_t info; // plugin_get_info;
};


int CountSO (const char* dirname, int* len) {
    DIR* dir = opendir(dirname);
    if (dir == NULL) {
        #if LAB1DEBUG==3
        fprintf(stdout,"CountSO\n");
        #endif
        fprintf(stderr, " \033[31mERROR:\033[0m No directory %s\n", dirname);
        return -1;
    }
    struct dirent* entity;
    entity = readdir(dir);
    *len = 0;

    while (entity != NULL) {
        int flen = strlen(entity->d_name);
        if ((entity->d_type == DT_REG) && (entity->d_name[flen-1] == 'o') && (entity->d_name[flen-2] == 's') && (entity->d_name[flen-3] == '.')) {
            (*len)++;
        }
        entity = readdir(dir);
    }
    closedir(dir);
    return 0;
}


int TryToOpen (const char* dirname, void* dl[], int len) {
    DIR* dir = opendir(dirname);
    if (dir == NULL) {
        #if LAB1DEBUG==3
        fprintf(stdout,"TryToOpen\n");
        #endif
        fprintf(stderr, "\033[31m ERROR:\033[m No directory %s\n", dirname);
        return -1;
    }
    struct dirent* entity;
    entity = readdir(dir);
    int index = 0;

    while (entity != NULL && index < len) {
        int flen = strlen(entity->d_name);
        if ((entity->d_type == DT_REG) && (flen > 3) && (entity->d_name[flen-1] == 'o') && (entity->d_name[flen-2] == 's') && (entity->d_name[flen-3] == '.')) {
            size_t file_name_len = strlen(dirname) + strlen(entity->d_name) + 2 ;
            char* file_name = malloc(file_name_len);
            sprintf(file_name, "%s/%s", dirname, entity->d_name);
            dl[index] = dlopen(file_name, RTLD_LAZY);
            if (dl[index] == NULL) {
                fprintf(stderr, "\033[31mERROR:\033[0m Failed to dlopen %s\n%s\n", entity->d_name, dlerror());
                continue;
                //return -1;
            }
            else {
                index++;  
            }
            free(file_name);

        }
        entity = readdir(dir);
    }
    closedir(dir);
    return 0;
}
  

int Search(const char* dirname, int tlen, struct longopt sup_all[], int is_or, int is_not) {
     FTS* ftsp;
    FTSENT* p;
    const char* paths[] = { dirname, NULL };

    ftsp = fts_open((char* const*)paths, FTS_PHYSICAL | FTS_NOCHDIR, NULL);

    if (ftsp == NULL) {
        perror("fts_open");
        return -1;
    }

    while ((p = fts_read(ftsp)) != NULL) {
        if (p->fts_info == FTS_D) {
            continue; 
        }

        if (strcmp(p->fts_name, ".") == 0 || strcmp(p->fts_name, "..") == 0 || strcmp(p->fts_name, "~") == 0) {
            continue; 
        }

        char* file_path = p->fts_accpath;

        if (p->fts_info == FTS_F) {
            if (check_file_access(file_path, R_OK) == -1) {
                fprintf(stderr, "ERROR: Failed to access file %s: %s\n", file_path, strerror(errno));
                continue;
            }

            int ret_true = 0; 
            int plugins_call = 0; 

            for (int i = 0; i < tlen; i++) {
                if (sup_all[i].optToPass > 0) {
                    plugins_call++;
                    int ret_fun = sup_all[i].func(file_path, sup_all[i].opts_to_pass, sup_all[i].optToPass);
                    if (ret_fun == 0) {
                        ret_true++;
                    }
                    if (ret_fun < 0) {
                        fts_close(ftsp);
                        errno = 0;
                        fprintf(stdout, "Error information: %s\n", strerror(errno));
                        return -1;
                    }
                }
            }

            if (plugins_call) {
                // short_opt A and no opt;
                if (ret_true == plugins_call && is_or == 0 && is_not == 0) {
                    fprintf(stdout, "files found: %s\n", file_path);
                }
                // short_opt O;
                else if (ret_true > 0 && is_or == 1 && is_not == 0) {
                    fprintf(stdout, "files found: %s\n", file_path);
                }
                // short_opt NA;
                else if (ret_true < plugins_call && is_or == 0 && is_not == 1) {
                    fprintf(stdout, "files found: %s\n", file_path);
                }
                // short_opt NO;
                else if (ret_true == 0 && is_or == 1 && is_not == 1) {
                    fprintf(stdout, "files found: %s\n", file_path);
                }
            }
        }
    }

    fts_close(ftsp);
    return 0;
}

int check_file_access(const char* filename, int mode) {
    if (access(filename, mode) == 0) {
        return 0;
    }
    else {
        perror("\033[34mERROR:\033[0m checking file access");
        return -1;
    }
}

int With_Third_Party_Plugins(int argc__, char* argv__[])
{
  if(argc__==2)
  {
    if((!strcmp(argv__[1],"-h")||!strcmp(argv__[1],"--help")||!strcmp(argv__[1],"-v")||!strcmp(argv__[1],"--version")))
    {
	
    }
    else
    {
      log_info("Incorrect use of the program. Try [-h] or [--help] for help\n");		

      exit(EXIT_FAILURE);
    }
  }

    
    struct longopt *sup_all = 0;
    char *f_name = 0;
    opterr = 0;
    
    int  is_o = 0, is_n = 0  ,is_P = 0 ;

    int len = 0;

    void** dl = 0;

    char **new_argv = (char**) malloc (argc__ * sizeof(char*));
    if(!new_argv){
        fprintf(stdout,"\033[31mERROR:\033[0m could not allocate for argv copy\n");
    }

    memcpy(new_argv, argv__, argc__ * sizeof(char*));

     
    int ret_shrt = 0;
    while((ret_shrt = getopt(argc__,new_argv, "P:AON"))!=-1)
        
    {

        switch(ret_shrt){

            case 'P':
                is_P = 1;
                if (CountSO(optarg, &len)){
                    fprintf(stderr, "\033[31mERROR:\033[0m unable to count file.so\n");
                    goto END;
                }

                dl = calloc (len, sizeof(void*));
                if (TryToOpen(optarg, dl, len)){
                    fprintf(stderr, "\033[31mERROR:\033[0m unable to open file.so\n");
                    goto END;
                }
                if(argv__[optind][1] ==  '-') {
                   
                   goto START;
                }
               
                break;
            case 'A': 
                if(argv__[optind][1] ==  '-') goto START;
                break;
            case 'O':
                is_o = 1;
                if(argv__[optind][1] ==  '-') goto START;
                break;
            case 'N':
                is_n = 1;
                if(argv__[optind][1] ==  '-') goto START;
                break;
        }
    }
    

    START: 
    if(!is_P) {
        if (CountSO(".", &len)) {
            fprintf (stderr, "\033[31mERROR:\033[0m unable to count in curren dir\n");
            goto END;
        }
        dl = calloc (len, sizeof(void*));
        if (TryToOpen(".", dl, len)){
            fprintf (stderr,"\033[31mERROR: \033[0munable to dlopen libs in a current dir\n");
            goto END;
        }
    }
    
    sup_all = calloc (len, sizeof(struct longopt));

    for (int i = 0; i < len; i++){

        sup_all[i].info = dlsym(dl[i], "plugin_get_info");
        if(!sup_all[i].info){
            fprintf(stderr, "\033[31mERROR:\033[0m dlsym() failed: %s\n", dlerror());
            continue;
        }
        struct plugin_info pi = {0};
        if (sup_all[i].info(&pi) < 0){
            fprintf(stderr, "\033[31mERROR:\033[0m plugin_get_info() failed\n");
            continue;
        }
        if (pi.sup_opts_len == 0){
            fprintf(stderr, "\033[31mERROR:\033[0m library supports no options! How so?\n");
            continue;
        }
         

        sup_all[i].func = dlsym(dl[i], "plugin_process_file");
        if(!sup_all[i].func) {
            fprintf(stderr, "\033[31mERROR:\033[0m no plugin_process_file() function found\n");
            goto END;
        }

        sup_all[i].all_opt_len = pi.sup_opts_len;
        sup_all[i].all_opt = calloc(pi.sup_opts_len + 1, sizeof(struct option));

        if (!sup_all[i].all_opt){
            fprintf(stderr, "\033[31mERROR:\033[0m calloc() failed:%s\n", strerror(errno));
            goto END;
        }

        for (size_t j = 0; j < pi.sup_opts_len; j++) {
            memcpy(&sup_all[i].all_opt[j], &pi.sup_opts[j].opt, sizeof(struct option));
        }

        sup_all[i].optToPass = 0;
        sup_all[i].opts_to_pass = calloc(pi.sup_opts_len, sizeof(struct option));
        if(!sup_all[i].opts_to_pass) {
            fprintf(stderr, "\033[31mERROR:\033[0m calloc() failed: %s\n", strerror(errno));
            goto END;
        }
    }
	
    for (int i = 0; i < len; i++) {
        optind = 1;   
        memcpy(new_argv, argv__, argc__ * sizeof(char*));
        while (1){
            int opt_ind = 0;
            int ret = getopt_long_only(argc__, new_argv, "", sup_all[i].all_opt, &opt_ind);
          
            if (ret == -1) break;
            if(ret != 0) continue;
         

            if ((size_t) sup_all[i].optToPass == sup_all[i].all_opt_len){
                fprintf(stderr, "\033[31mERROR:\033[0m too many options!\n");
                goto END;
            }

            memcpy(sup_all[i].opts_to_pass + sup_all[i].optToPass, sup_all[i].all_opt + opt_ind, sizeof(struct option));
         

            if ((sup_all[i].all_opt + opt_ind)->has_arg) {
                (sup_all[i].opts_to_pass + sup_all[i].optToPass)->flag = (int*)strdup(optarg);
            }

            sup_all[i].optToPass++;
        }
    }

    if (getenv("LAB1DEBUG")) {
        for (int i = 0; i < len; i++){
            fprintf(stderr, "\033[33mDEBUG:\033[0m optToPass = %ld\n", sup_all[i].optToPass);
            for (size_t j = 0; j < sup_all[i].optToPass; j++) {
            fprintf(stderr, "\033[33mDEBUG:\033[0m passing option '%s' with arg '%s'\n",
                (sup_all[i].opts_to_pass[j]).name,
                (char*)(sup_all[i].opts_to_pass[j]).flag);
            }
        }
        fprintf(stdout, "\033[33mDEBUG:\033[0mOptions passed! \n");
    }


    errno = 0;
    f_name = strdup(argv__[argc__-1]);
    int ret_main = Search(f_name, len, sup_all, is_o, is_n);  

    if (getenv("LAB1DEBUG")){
        fprintf (stdout, "\033[33mDEBUG:\033[0m Search() returned %d\n",ret_main); 
    }
    
    if(ret_main < 0){
        fprintf(stderr, "\033[33mERROR:\033[0m infomation: %s\n", strerror(errno));
    }

    END:
    if (sup_all){
        for (int i = 0; i < len; i++){
            for(size_t j = 0; j < sup_all[i].optToPass; j++){
                if (sup_all[i].opts_to_pass[j].flag) free(sup_all[i].opts_to_pass[j].flag);
            }
            if (sup_all[i].opts_to_pass) free(sup_all[i].opts_to_pass);
            if (sup_all[i].all_opt) free(sup_all[i].all_opt);
        }
        free(sup_all);
    }

    if(new_argv) free(new_argv);
    if(f_name) free(f_name);
    if (dl){
        for (int i = 0; i < len; i++){
            if(dl[i]) dlclose(dl[i]);
        }
        free (dl);
    }

    
    #if LAB1DEBUG==2
    fprintf(stdout,"EXIT FROM ...int With_Third_Party_Plugins(int argc__, char* argv__[])\n");
    #endif
    return 0;
}



int main(int argc, char* argv[])
{	
        plugin_opt_trigger=lib_find(plugin_info_trigger);
        #if LAB1DEBUG==2
        fprintf(stdout,"plugin_opt_trigger=%d\n",plugin_opt_trigger); 
        #endif
    
        if(plugin_opt_trigger==0)
        {
          if(argv[1])
          {
            if(!strcmp(argv[1],"-P"))
            {
	      With_Third_Party_Plugins( argc,  argv);
	      goto END;
            }
          }
          else
          {
            log_info("Incorrect use of the program. Try [-h] or [--help] for help\n");		
	    flag=false;
	    #if LAB1DEBUG==2
	    fprintf(stdout,"flag=%d\n",flag);
	    #endif
	    exit(EXIT_FAILURE);              
          }
          
        }
        else if(plugin_opt_trigger>0)
        {
          if(argv[1])
          {
            str4=argv[1];
            if((str4[0]=='/'||(str4[0]=='.'&&str4[1]=='/'))||(!strcmp(argv[1],"-h")||!strcmp(argv[1],"--help")||!strcmp(argv[1],"-v")||!strcmp(argv[1],"--version")))
	    {
                
	    }
            else
            {
              With_Third_Party_Plugins( argc,  argv);
              goto END;
            }
          }
        }
	
	#if LAB1DEBUG==1 
	fprintf(stdout,"argv[0]=%s\targc=%d\n",argv[0],argc);
	for(int i=1;i<argc;i++)
	{
		fprintf(stdout,"argv[%d]=%s\n",i,argv[i]);
	}
	fprintf(stdout,"flag at start=%d\n",flag);
	#endif
	if(argc==1)
	{
		log_info("Incorrect use of the program. Try [-h] or [--help] for help\n");
		flag=false;
		
		#if LAB1DEBUG==1 
		fprintf(stdout,"flag=%d\n",flag);
		#endif
		
		exit(EXIT_FAILURE);
	}

	else if(argc==2)
	{
		if((!strcmp(argv[1],"-h")||!strcmp(argv[1],"--help")||!strcmp(argv[1],"-v")||!strcmp(argv[1],"--version")))
		{
			flag=true;
			#if LAB1DEBUG==1 
			fprintf(stdout,"flag=%d\n",flag);
			#endif
		}
		else
		{
			log_info("Incorrect use of the program. Try [-h] or [--help] for help\n");		
			flag=false;
			#if LAB1DEBUG==1 
			fprintf(stdout,"flag=%d\n",flag);
			#endif
			exit(EXIT_FAILURE);
		}
	}
	else if (argc==4)
	{
		str=argv[1];
		str2=argv[2];

		if((!strcmp(argv[1],"-h")||!strcmp(argv[1],"--help")||!strcmp(argv[1],"-v")||!strcmp(argv[1],"--version")))
		{
			log_info("Incorrect use of the program. Try [-h] or [--help] for help\n");
			flag=false;
			#if LAB1DEBUG==1 
			fprintf(stdout,"flag=%d\n",flag);
			#endif
			exit(EXIT_FAILURE);
		}
		else if((!strcmp(argv[2],"-h")||!strcmp(argv[2],"--help")||!strcmp(argv[2],"-v")||!strcmp(argv[2],"--version")))
		{
			log_info("Incorrect use of the program. Try [-h] or [--help] for help\n");
			flag=false;
			#if LAB1DEBUG==1 
			fprintf(stdout,"flag=%d\n",flag);
			#endif
			exit(EXIT_FAILURE);
		}
		else if((str[0]=='.'&&str[1]=='/')||str[0]=='/')
		{
			if((str2[0]=='.'&&str2[1]=='/')||str2[0]=='/') 
			{
				log_info("Incorrect use of the program. Try [-h] or [--help] for help\n");
				flag=false;
				#if LAB1DEBUG==1 
				fprintf(stdout,"flag=%d\n",flag);
				#endif
				exit(EXIT_FAILURE);
			}
			else if(!strcmp(argv[3],"--string-in-hex")||!strcmp(argv[3],"-S")||!strcmp(argv[3],"--string")||!strcmp(argv[3],"-s"))
			{
				#if LAB1DEBUG==1
				fprintf(stdout,"Good job!\nEntry -> ftw()\nStart_Directory=%s\nTarget=%s\n",argv[1],argv[2]);
				#endif
				
				target=argv[2];
				path=argv[1];
				flag=true;
			}
			else
			{
				log_info("\n");
				flag=false;
				#if LAB1DEBUG==1 
				fprintf(stdout,"flag=%d\n",flag);
				#endif
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			log_info("Incorrect use of the program. Try [-h] or [--help] for help\n");
			flag=false;
			#if LAB1DEBUG==1 
			fprintf(stdout,"flag=%d\n",flag);
			#endif
			exit(EXIT_FAILURE);
		}
	}	
	else
	{
		log_info("Incorrect use of the program. Try [-h] or [--help] for help\n");
		flag=false;
		exit(EXIT_FAILURE);
	}
	#if LAB1DEBUG==1 
	fprintf(stdout,"flag before get_opt =%d\n",flag);
	#endif
	
	if(flag==true)
	{
		const struct option long_options[] = {
			{ "help", no_argument, 0, 'h' },
			{ "version", no_argument, 0, 'v' },
			{ "string-in-hex", no_argument, 0, 'S' },
			{ "string", no_argument, 0, 's' },
		};
		int rez;
		int option_index;
		while ((rez=getopt_long(argc,argv,"hvSs",long_options,&option_index))!=-1)
		{
		
			switch(rez)
			{
				case 'v': 
				{
					fprintf(stdout,"\n\tv 1.2\n\n");
					goto END;
				};
				case 'h': 
				{
					fprintf(stdout,"\nlab12lvcN32451: ./lab12lvcN32451 [OPTION]\n\t\t./lab11lvcN32451 directory target [FIND_MOD]\t\t\n\n\tDisplay information about builtin commands.\n\n\tThe order of building commands is strictly linear. Follow the instructions.\n\n\tOptions:\n\t  -h [--help]\tprogram help output\n\t  -v [--version]\tprogram version output\n\t  -S [--string-in-hex]\tfind all files that contain an array of Latinic characters in hexadecimal representation \n\t  -s [--string]\t\tfind all files that contain string \n\t\n\n\tArguments:\n\t  directory\tThe path from which the search begins\n\t  target\tIf it is a word, then it is enclosed in single quotes, otherwise in double quotes.\n\n\tExit Status:\n\t  Returns the full path to the file where the target was found. If files were encountered that cannot be opened (due to lack of privileges), then the full path to the file that could not be opened is given.\n\nThirt-party plugins: ./lab12lvcN32451 [SHORT OPTION] [ARG] [LONG OPTION] [ARG2] [PATH]\n\n\tOptions:\n\t  -P\tPath to dir with plugins (default cwd)\n\t  -A\tCombining plug-in options using the "AND" operation (effective by default).\n\t  -O\tCombining plugin options using OR operation. \n\t  -N\t\tInverting the search condition (after combining the options of theplug-ins with -A or -O).\n\t \n\n\tArguments:\n\t  ARG\tThe path with plugins (if use -P)\n\t  ARG2\tOption argument (if required).\n\t  PATH\tStart dir to search (if required).\n\n\tExit Status:\n\t  Returns the full path to the file where the target was found. If files were encountered that cannot be opened (due to lack of privileges), then the full path to the file that could not be opened is given. Display information about builtin commands when plugin(s) was founded.\n\n\tThe order of building commands is strictly linear. Follow the instructions.\n\n");
					
					plugin_info_trigger=1;
					/*
					loadingBar();
					*/
					if(lib_find(plugin_info_trigger)==0)
                                        {
                                            fprintf(stdout,"Cant find plugins in cwd.\n");
                                        }
					break;
					
				};
				case 'S': 
			
				{
					int len=strlen(target);
					int g,j;
					for(g=0;g<len;g++)
					{
						if(target[g]==' '||target[g]=='\t'||target[g]=='\n')
						{
							for(j=g;j<len;j++)
							{
								target[j]=target[j+1];
							}
							len--;
							g--;
						}
					}
					for_switch=1;
					ftw_find(path,target,for_switch);
					break;
				};
				case 's': 
				{
					int len=strlen(target);
					int g,j;
					for(g=0;g<len;g++)
					{
						if(target[g]==' '||target[g]=='\t'||target[g]=='\n')
						{
							for(j=g;j<len;j++)
							{
								target[j]=target[j+1];
							}
							len--;
							g--;
						}
					}
					for_switch=2;
					ftw_find(path,target,for_switch);
					
					break;
				};
			};
		};
	}
	str=NULL;
	target=NULL;
        return 0;
        END:
	  return 0;
}

