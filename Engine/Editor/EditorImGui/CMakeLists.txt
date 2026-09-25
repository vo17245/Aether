add_library(EditorImGui STATIC
    src/Private/CorePanels.cpp
    src/Private/EditorLayer.cpp)
add_library(Aether::EditorImGui ALIAS EditorImGui)
target_compile_features(EditorImGui PUBLIC cxx_std_23)
target_link_libraries(EditorImGui PUBLIC EditorFramework Window ImGui)
target_include_directories(EditorImGui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Public>
    $<INSTALL_INTERFACE:EditorImGui/include>
)
install(TARGETS EditorImGui EXPORT EditorImGuiTargets
    ARCHIVE DESTINATION EditorImGui/lib
    LIBRARY DESTINATION EditorImGui/lib
    RUNTIME DESTINATION EditorImGui/bin)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/" DESTINATION EditorImGui/include FILES_MATCHING PATTERN "*.h")
include(CMakePackageConfigHelpers)
configure_package_config_file(CMake/EditorImGuiConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/EditorImGuiConfig.cmake" INSTALL_DESTINATION EditorImGui)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/EditorImGuiConfig.cmake" DESTINATION EditorImGui)
install(EXPORT EditorImGuiTargets FILE EditorImGuiTargets.cmake NAMESPACE Aether:: DESTINATION EditorImGui)
