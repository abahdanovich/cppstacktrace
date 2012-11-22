#include "StackTrace.h"

#include <iostream>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <elfutils/libdwfl.h>
#include <libunwind.h>
#include <cxxabi.h>

stacktrace::exec_point_t StackTrace::getLocationInfo(const void* ip)
{
    char *debuginfo_path = NULL;

    Dwfl_Callbacks callbacks;

    callbacks.find_elf = dwfl_linux_proc_find_elf;
    callbacks.find_debuginfo = dwfl_standard_find_debuginfo;
    callbacks.debuginfo_path = &debuginfo_path;

    Dwfl* dwfl = dwfl_begin(&callbacks);
    assert(dwfl != NULL);

    assert(dwfl_linux_proc_report(dwfl, getpid()) == 0);
    assert(dwfl_report_end(dwfl, NULL, NULL) == 0);

    Dwarf_Addr addr = (uintptr_t) ip;

    Dwfl_Module* module = dwfl_addrmodule(dwfl, addr);

    const char* function_name = dwfl_module_addrname(module, addr);

    char* function_real_name;
    int status;

    function_real_name = abi::__cxa_demangle(function_name, 0, 0, &status);

    stacktrace::exec_point_t ep;

    if (status == 0) {
        ep.function_name.append(function_real_name);
    }
    else {
        ep.function_name.append(function_name);
    }

    free(function_real_name);

    Dwfl_Line *line = dwfl_getsrc(dwfl, addr);

    ep.ptr = ip;

    if (line != NULL) {
        int nline;
        Dwarf_Addr addr;
        const char* filename = dwfl_lineinfo(line, &addr, &nline, NULL, NULL, NULL);

        ep.file_name.append(strrchr(filename, '/') + 1);
        ep.line = nline;
    }

    return ep;
}

stacktrace::stack_trace_t __attribute__((noinline)) StackTrace::getStack(int skip)
{
    stacktrace::stack_trace_t stack_trace;
    
#ifdef NDEBUG
    return stack_trace;
#endif    
    
    unw_context_t uc;
    unw_getcontext(&uc);

    unw_cursor_t cursor;
    unw_init_local(&cursor, &uc);

    while (unw_step(&cursor) > 0) {

        unw_word_t ip;
        unw_get_reg(&cursor, UNW_REG_IP, &ip);

        unw_word_t offset;
        char name[64];

        int res = unw_get_proc_name(&cursor, name, sizeof (name), &offset);
        //assert(res == 0);

        if (skip <= 0) {
            stacktrace::exec_point_t ep = getLocationInfo((void*) (ip - 4));
            stack_trace.push_back(ep);
        }

        if (strcmp(name, "main") == 0)
            break;

        skip--;
    }

    return stack_trace;
}

void StackTrace::printStack(stacktrace::stack_trace_t s)
{
    for (size_t i = 0; i < s.size(); i++) {
        stacktrace::exec_point_t ep = s.at(i);
        std::cout << ep.function_name << ", " << ep.ptr << ", " << ep.file_name << ":" << ep.line << std::endl;
    }
}
