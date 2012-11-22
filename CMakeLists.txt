set(CMAKE_BUILD_TYPE "Debug")
add_executable(stack_trace stack_trace.cpp StackTrace.cpp)
target_link_libraries(stack_trace dw unwind)
