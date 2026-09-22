add_executable(RenderInFlight.Tests main.cpp)
target_link_libraries(RenderInFlight.Tests PRIVATE Render)
add_test(NAME RenderInFlight.Tests COMMAND RenderInFlight.Tests)
