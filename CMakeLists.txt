add_executable(stack_trace stack_trace.cpp)
target_link_libraries(stack_trace dw unwind)
