add_executable(RenderGraphArena.Tests main.cpp ../Render.Tests/Unit/src/ResourceArenaTests.cpp)
target_link_libraries(RenderGraphArena.Tests PRIVATE Render)
