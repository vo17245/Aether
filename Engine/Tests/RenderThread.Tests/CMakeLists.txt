add_executable(RenderThread.Tests main.cpp)
target_link_libraries(RenderThread.Tests PRIVATE Render)
add_test(NAME RenderThread.Tests COMMAND RenderThread.Tests)
