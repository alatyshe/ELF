#include "debug.h"



#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <iostream>
#include <cstring>

static inline int depth_stacktrace(FILE *out = stderr, unsigned int max_frames = 63)
{
	int 	depth;

	depth = 0;
    // storage array for stack trace address data
    void* addrlist[max_frames+1];

    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));
    if (addrlen == 0)
		return 0;

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char** symbollist = backtrace_symbols(addrlist, addrlen);
    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 1024;
    char* funcname = (char*)malloc(funcnamesize);


    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (int i = 1; i < addrlen; i++)
    {
		char *begin_name = 0, *begin_offset = 0, *end_offset = 0;
		// find parentheses and +address offset surrounding the mangled name:
		// ./module(function+0x15c) [0x8048a6d]
		for (char *p = symbollist[i]; *p; ++p)
		{
		    if (*p == '(')
				begin_name = p;
		    else if (*p == '+')
				begin_offset = p;
		    else if (*p == ')' && begin_offset) {
				end_offset = p;
				break;
		    }
		}

		if (begin_name && begin_offset && end_offset
		    && begin_name < begin_offset)
		{
		    *begin_name++ = '\0';
		    *begin_offset++ = '\0';
		    *end_offset = '\0';

		    // mangled name is now in [begin_name, begin_offset) and caller
		    // offset in [begin_offset, end_offset). now apply
		    // __cxa_demangle():

		    int status;
		    char* ret = abi::__cxa_demangle(begin_name,
						    funcname, &funcnamesize, &status);

		   	if (strcmp(symbollist[i], "python3") == 0)
	    		continue ;
		    if (status == 0) {
				funcname = ret; // use possibly realloc()-ed string
				depth++;
				// fprintf(out, "===  %s : %s+%s\n",
				// symbollist[i], funcname, begin_offset);
		    }
		    else {
				// demangling failed. Output function name as a C function with
				// no arguments.
				// fprintf(out, "!!!  %s : %s()+%s\n",
				// symbollist[i], begin_name, begin_offset);
		    }
		}
		else
		{
		    // couldn't parse the line? print the whole line.
		    // fprintf(out, "---  %s\n", symbollist[i]);
		}
	}
	if (funcname != NULL)
		free(funcname);
	if (symbollist != NULL)
		free(symbollist);
    return (depth);
}





namespace debug {
		t_options	options_main;
}

void		display_debug_info(
		std::string clss_name,
		std::string mthd_name,
		std::string color,
		bool display) 
{
	int 		depth;

	if ((debug::options_main.verbose || display))
	 // && color != GREEN_B)
 	{
		depth = depth_stacktrace();

		for (int i = 0; i < depth; i++)
			std::cout << "  ";
		std::cout << "\x1b[1;33;40m|++|\x1b[0m";
		std::cout << color << clss_name << COLOR_END;
		std::cout << "::" << mthd_name << std::endl;		
	}	
}
