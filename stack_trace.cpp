#include "StackTrace.h"

void c(void)
{
    StackTrace::printStack(StackTrace::getStack());
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
        StackTrace::printStack(StackTrace::getStack());
    }
}