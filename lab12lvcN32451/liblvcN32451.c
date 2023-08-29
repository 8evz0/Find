#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include "plugin_api.h"

static char *g_lib_name = "liblvcN32451.so";

static char *g_plugin_purpose = "\n\t\tОпция: --double-bin\n\t\tНазначение: поиск файлов, содержащих заданное вещественное число двойной точности в бинарной (little-endian или big-endian) форме, т. е. в виде восьми последовательных октетов с соответствующими значениями. Вещественное число может быть записано в обычном или научном форматах.\n\t\tПример: --double-bin 2.71\n";

static char *g_plugin_author = "Leo Cherednikov";


#define OPT_DOUBLE_BIN "double-bin"
static struct plugin_option g_po_arr[] = {
/*
    struct plugin_option {
        struct option {
           const char *name;
           int         has_arg;
           int        *flag;
           int         val;
        } opt,
        char *opt_descr
    }
*/
    {
        {
            OPT_DOUBLE_BIN,
            required_argument,
            0, 0,
        },
        "Double bin value is target"
    }

};

static int g_po_arr_len = sizeof(g_po_arr)/sizeof(g_po_arr[0]);

//
//  Private functions
//
static double Find_Files_With_Target(unsigned char*, size_t, size_t);

//
//  API functions
//
int plugin_get_info(struct plugin_info* ppi) 
{
    if (!ppi) 
    {

        fprintf(stderr, "\033[31mERROR:\033[0m invalid argument\n");
        return -1;
    }
    
    ppi->plugin_purpose = g_plugin_purpose;
    ppi->plugin_author = g_plugin_author;
    ppi->sup_opts_len = g_po_arr_len;
    ppi->sup_opts = g_po_arr;
    
    return 0;
}

int plugin_process_file(const char *fname,
        struct option in_opts[],
        size_t in_opts_len) {

    // Return error by default
    int ret_lib1 = 1;
    

      // Pointer to file mapping
    unsigned char *ptr = NULL;
    char *f = NULL;
    
    char *DEBUG = getenv("LAB1DEBUG");
    
    if (!fname || !in_opts || !in_opts_len) {
        errno = EINVAL;
        return -1;
    }
    
    if (DEBUG) {
        for (size_t i = 0; i < in_opts_len; i++) {
            fprintf(stderr, "\033[33mDEBUG:\033[0m %s: OPT '%s' ARG '%s'\n",
                g_lib_name, in_opts[i].name, (char*)in_opts[i].flag);
        }
    }
    
    const double* target_double_bin;
    int size_double_bin = 0;
    int saved_errno = 0;
    
    // Чтение переданного аргумента опции в переменную 
    for (size_t i = 0; i < in_opts_len; i++) {
        if (!strcmp(in_opts[i].name, OPT_DOUBLE_BIN)) {

            target_double_bin = in_opts[i].flag;
            if (DEBUG) {
            fprintf(stdout, "\033[33mDEBUG:\033[0m ARG %s   \n",(char*)target_double_bin);
            }
 
        }
        else {
            errno = EINVAL;
            return -1;
        }
    }
    
    // определяем формат входных данных
    if (strstr(target_double_bin, "e") != NULL || strstr(target_double_bin, "E") != NULL) {
        // научный формат
        if (DEBUG) {
            fprintf(stdout, "\033[33mDEBUG:\033[0m scientific format \n");
        }
        if (sscanf(target_double_bin, "%le", &target_double_bin) != 1) {
            printf("Invalid input!\n");
            return 1;
        }
    } else {
        // обычный формат
        if (DEBUG) {
            fprintf(stderr, "\033[33mDEBUG:\033[0m usually format \n");
        }
        if (sscanf(target_double_bin, "%lf", &target_double_bin) != 1) {
            printf("Invalid input!\n");
            return 1;
        }
    }
   
   // определяем порядок байт
   int is_big_endian = htonl(1) == 1 ? 0 : 1; // 1 в big-endian - 0x00000001, в little-endian - 0x01000000
   
   unsigned char *p = (unsigned char *)&target_double_bin;
    char bin_str[65]; // создаем массив для записи символов

    int index = 0;
    for (int i = is_big_endian ? 7 : 0; is_big_endian ? (i >= 0) : (i < 8); is_big_endian ? (i--) : (i++)) {
       for (int j = 7; j >= 0; j--) {
           // добавляем в массив символ "0" или "1" соответственно текущему биту
           bin_str[index] = ((p[i] >> j) & 1) ? '1' : '0';
           index++;
       }
    }

    // добавляем завершающий нулевой символ в конец массива
    bin_str[64] = '\0';

    //size_p  = strlen((char*)p);
    int size_bin_str= strlen(bin_str);

    //int saved_errno = 0;
    //int size_f ;
    
    
  int size_f ;
  
   
   int fd = open(fname, O_RDONLY);
    if (fd < 0) {
        // errno is set by open()
        return -1;
    }
    
    struct stat st = {0};
    int res = fstat(fd, &st);
    if (res < 0) {
        saved_errno = errno;
        goto END;
    }
    
    // Check that size of file is > 0
    if (st.st_size == 0) {
        if (DEBUG) {
            fprintf(stderr, "\033[33mDEBUG:\033[0m %s: File size should be > 0\n",
                g_lib_name);
        }
        saved_errno = ERANGE;
        goto END;
    }
    
    ptr = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED) {
        saved_errno = errno;
        goto END;
    }

  size_f = st.st_size;
    
    
    f = (char *) mmap(0, st.st_size, PROT_READ, MAP_PRIVATE,fd, 0);

    
        // search for binary string in file f
    int found = 0;
    for (int i = 0; i < size_f - 63; i++) {
        if (memcmp(f + i, bin_str, 64) == 0) {
            found = 1;
            break; // binary string found
        }
    }
    if (found) {
       ret_lib1 = 0;
            
                if (DEBUG) {
                    fprintf(stderr, "\033[33mDEBUG:\033[0m %s: \033[32mfTarget found\033[0m '%s'\n", 
                        g_lib_name, bin_str);
                }
                
    } else {
        if(ret_lib1) ret_lib1 = 1;
                if (DEBUG) {
                    fprintf(stderr, "\033[33mDEBUG:\033[0m %s: Target didnt find '%s'\n", 
                        g_lib_name,  bin_str);
                }
    }


    END:

    if (ptr != MAP_FAILED && ptr != NULL) munmap(ptr, st.st_size);
    if (f != MAP_FAILED && f != NULL) munmap(f,st.st_size);
    
    // Restore errno value
    errno = saved_errno;
    return ret_lib1;
}        


