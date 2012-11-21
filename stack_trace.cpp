#define UNW_LOCAL_ONLY

#include <elfutils/libdwfl.h>
#include <libunwind.h>
#include <cxxabi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

static void debugInfo(FILE* out,const void* ip)
{
	char *debuginfo_path=NULL;

	Dwfl_Callbacks callbacks;
	
	callbacks.find_elf = dwfl_linux_proc_find_elf;
	callbacks.find_debuginfo = dwfl_standard_find_debuginfo;
	callbacks.debuginfo_path = &debuginfo_path;

	Dwfl* dwfl=dwfl_begin(&callbacks);
	assert(dwfl!=NULL);

	assert(dwfl_linux_proc_report (dwfl, getpid())==0);
	assert(dwfl_report_end (dwfl, NULL, NULL)==0);

	Dwarf_Addr addr = (uintptr_t)ip;

	Dwfl_Module* module=dwfl_addrmodule (dwfl, addr);

	const char* function_name = dwfl_module_addrname(module, addr);

	char* function_real_name;
    int     status;
  
    function_real_name = abi::__cxa_demangle(function_name, 0, 0, &status);

	if (status == 0) {
		fprintf(out,"%s (", function_real_name);
	}
	else {
		fprintf(out,"%s (", function_name);
	}
	
	free(function_real_name);

	Dwfl_Line *line=dwfl_getsrc (dwfl, addr);
	
	if(line!=NULL)
	{
		int nline;
		Dwarf_Addr addr;
		const char* filename=dwfl_lineinfo (line, &addr,&nline,NULL,NULL,NULL);
		fprintf(out,"%s:%d",strrchr(filename,'/')+1,nline);
	}
	else
	{
		fprintf(out,"%p",ip);
	}
}

static void __attribute__((noinline)) printStackTrace(FILE* out, int skip)
{
	unw_context_t uc;
	unw_getcontext(&uc);

	unw_cursor_t cursor;
	unw_init_local(&cursor, &uc);

	while(unw_step(&cursor)>0)
	{

		unw_word_t ip;
		unw_get_reg(&cursor, UNW_REG_IP, &ip);

		unw_word_t offset;
		char name[32];
		assert(unw_get_proc_name(&cursor, name,sizeof(name), &offset)==0);

		if(skip<=0)
		{
			debugInfo(out,(void*)(ip-4));
			fprintf(out,")\n");
		}

		if(strcmp(name,"main")==0)
			break;
		
		skip--;
	}

}

void c(void)
{
	printStackTrace(stdout, 0);
	throw 123;
}

void b(void)
{
	c();
}

void a(void)
{
	b();
}

int main(int argc, char*argv[])
{
	try {
		a();
	}
	catch (int i) {
		printStackTrace(stdout, 0);
	}
}
