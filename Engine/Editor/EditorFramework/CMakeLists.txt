add_library(EditorFramework STATIC
    src/Private/Storage/AtomicReplaceFile.cpp
    src/Private/EditorHost.cpp
    src/Private/EditorFeatureRegistrar.cpp
    src/Private/Systems/PlayLifecycle.cpp
    src/Private/Systems/Commands.cpp
    src/Private/Systems/InspectorModel.cpp
    src/Private/Systems/WorldDocument.cpp
    src/Private/Systems/AssetImport.cpp
    src/Private/Systems/ViewportRouter.cpp
    src/Private/Systems/AssetEditor.cpp
    src/Private/Systems/ProjectLifecycle.cpp
)
add_library(Aether::EditorFramework ALIAS EditorFramework)
target_compile_features(EditorFramework PUBLIC cxx_std_23)
target_link_libraries(EditorFramework PUBLIC GameFeature ProjectAsset World Filesystem)
target_include_directories(EditorFramework PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Public>
    $<INSTALL_INTERFACE:EditorFramework/include>
)
install(TARGETS EditorFramework EXPORT EditorFrameworkTargets
    ARCHIVE DESTINATION EditorFramework/lib
    LIBRARY DESTINATION EditorFramework/lib
    RUNTIME DESTINATION EditorFramework/bin)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/" DESTINATION EditorFramework/include FILES_MATCHING PATTERN "*.h")
include(CMakePackageConfigHelpers)
configure_package_config_file(CMake/EditorFrameworkConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/EditorFrameworkConfig.cmake" INSTALL_DESTINATION EditorFramework)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/EditorFrameworkConfig.cmake" DESTINATION EditorFramework)
install(EXPORT EditorFrameworkTargets FILE EditorFrameworkTargets.cmake NAMESPACE Aether:: DESTINATION EditorFramework)
