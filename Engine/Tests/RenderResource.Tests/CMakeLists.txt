add_executable(RenderResource.Tests main.cpp)
target_link_libraries(RenderResource.Tests PRIVATE Render)
add_test(NAME RenderResource.Tests COMMAND RenderResource.Tests)
