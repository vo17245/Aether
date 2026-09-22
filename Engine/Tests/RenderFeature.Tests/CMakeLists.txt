add_executable(RenderFeature.Tests main.cpp)
target_link_libraries(RenderFeature.Tests PRIVATE Render)
add_test(NAME RenderFeature.Tests COMMAND RenderFeature.Tests)
