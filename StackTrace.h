#ifndef STACKTRACE_H
#define	STACKTRACE_H

#define UNW_LOCAL_ONLY

#include <string>
#include <vector>

namespace stacktrace
{

    typedef struct
    {
        std::string function_name;
        std::string file_name;
        size_t line;
        const void* ptr;

    } exec_point_t;

    typedef std::vector<exec_point_t> stack_trace_t;
}

class StackTrace
{
public:

    static stacktrace::exec_point_t getLocationInfo(const void*);
    static stacktrace::stack_trace_t __attribute__((noinline)) getStack(int = 0);
    static void printStack(stacktrace::stack_trace_t);
};

#endif	/* STACKTRACE_H */

