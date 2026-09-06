add_executable(ImGui.RenderGraph.Tests RenderGraphBackendTests.cpp)
target_link_libraries(ImGui.RenderGraph.Tests PRIVATE Entry)
if(MSVC)
    target_compile_options(ImGui.RenderGraph.Tests PRIVATE /utf-8)
endif()
if(WIN32 AND TARGET SDL3::SDL3-shared)
    add_custom_command(TARGET ImGui.RenderGraph.Tests POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:SDL3::SDL3-shared>" "$<TARGET_FILE_DIR:ImGui.RenderGraph.Tests>"
        VERBATIM)
endif()
