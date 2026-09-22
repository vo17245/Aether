add_executable(Lifecycle.Tests main.cpp)
target_link_libraries(Lifecycle.Tests PRIVATE Async MainLoop)
add_test(NAME Lifecycle.Tests COMMAND Lifecycle.Tests)
