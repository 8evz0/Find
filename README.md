# FSP1
A program for recursive file search on a Linux machine using dynamic libraries (plugins)

The program searches for files starting from the specified directory. It is possible to connect third-party plugins, if the plugin meets the specified API.

If you have any questions, comments or suggestions - write, it's important!

lab12lvcN32451: ./lab12lvcN32451 [OPTION]
		./lab11lvcN32451 directory target [FIND_MOD]		

	Display information about builtin commands.

	The order of building commands is strictly linear. Follow the instructions.

	Options:
	  -h [--help]	program help output
	  -v [--version]	program version output
	  -S [--string-in-hex]	find all files that contain an array of Latinic characters in hexadecimal representation 
	  -s [--string]		find all files that contain string 
	

	Arguments:
	  directory	The path from which the search begins
	  target	If it is a word, then it is enclosed in single quotes, otherwise in double quotes.

	Exit Status:
	  Returns the full path to the file where the target was found. If files were encountered that cannot be opened (due to lack of privileges), then the full path to the file that could not be opened is given.

Thirt-party plugins: ./lab12lvcN32451 [SHORT OPTION] [ARG] [LONG OPTION] [ARG2] [PATH]

	Options:
	  -P	Path to dir with plugins (default cwd)
	  -A	Combining plug-in options using the "AND" operation (effective by default).
	  -O	Inverting the search condition (after combining the options of the plug-ins with -A or -O).
	  -N		find all files that contain string
	 

	Arguments:
	  ARG	The path with plugins (if use -P)
	  ARG2	Option argument (if required).
	  PATH	Start dir to search (if required).

	Exit Status:
	  Returns the full path to the file where the target was found. If files were encountered that cannot be opened (due to lack of privileges), then the full path to the file that could not be opened is given.Display information about builtin commands when plugin(s) was founded.

	The order of building commands is strictly linear. Follow the instructions.



Examples of using with screenshots:
1. Build programm
   ![изображение](https://github.com/8evz0/FSP1/assets/65715287/1d0eae84-2086-42d9-a31d-ff7128482417)
3. Find all files that contain string (-s)
   Command: sudo ./lab12lvcN32451 /home "hi" -s
   ![изображение](https://github.com/8evz0/FSP1/assets/65715287/53605463-ce30-44ad-96dc-525077840ed6)

5. Find all files that contain an array of Latinic characters in hexadecimal representation
   Command: sudo ./lab12lvcN32451 /home "hi" -S
6. Using plugins
   Command: sudo ./lab12lvcN32451 --dl-syms "a" / (to find dl by symbols)
   Command: <pre>sudo ./lab12lvcN32451 --double-bin 3.14 / (to find double)
   ![изображение](https://github.com/8evz0/FSP1/assets/65715287/c3021fd5-c3f5-4d9a-9c6d-67833010c3f9)

   
P.S. To create plug-ins for the program you need to use the following API:

#ifndef _PLUGIN_API_H
#define _PLUGIN_API_H

#include <getopt.h>

#define PLUGIN_API_VERSION 1

/*
    Structure describing the option supported by the plugin.
*/
struct plugin_option {
    /* Option in the format supported by getopt_long (man 3 getopt_long). */
    struct option opt;
    /* Description of the option that the plugin provides. */
    const char *opt_descr;
};

/*
    A structure containing information about the plugin.
*/
struct plugin_info {
    /* Plugin purpose */
    const char *plugin_purpose;
    /* Author of the plugin, for example "Ivanov Ivan Ivanovich, N32xx" */
    const char *plugin_author;
    /* Length of the list of options */
    size_t sup_opts_len;
    /* List of options supported by the plugin */
    struct plugin_option *sup_opts;
};


int plugin_get_info(struct plugin_info* ppi);
/*
    plugin_get_info()
    
    A function to get information about a plugin.
    
    Arguments:
        ppi is the address of the structure that the plugin fills with information.
        
    Return value:
          0 - in case of success,
        < 0 - in case of failure (in this case you cannot continue working with this plugin).
*/



int plugin_process_file(const char *fname,
        struct option in_opts[],
        size_t in_opts_len);
/*
    plugin_process_file()
    
    A function to find out if a file meets the given criteria.
    
    Arguments:
        fname - path to the file (full or relative), which is checked for
            correspondence to the criteria specified with the help of in_opts array.

        in_opts - list of options (search criteria) that are passed to the plugin.
            struct option {
               const char *name;
               int has_arg;
               int *flag;
               int val;
            };
            The name field is used to pass the name of the option, the flag field is used to pass the value of the option (as a string).
            option value (as a string). If the option has an argument, the field has_arg
            field is set to a non-zero value. The val field is not used.
           
        in_opts_len - the length of the options list.
                    
    Return value:
          0 - the file meets the given criteria,
        > 0 - the file does NOT meet the specified criteria,
        < 0 - an error occurred in the process
        
    If an error occurred, the errno variable should be set to the appropriate value. 
    to the appropriate value.
*/
        
#endif
