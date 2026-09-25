add_library(ProjectAsset STATIC
    src/Private/Types.cpp
    src/Private/ProjectManifest.cpp
    src/Private/AssetResolver.cpp
    src/Private/ProjectLock.cpp
)
add_library(Aether::ProjectAsset ALIAS ProjectAsset)
target_compile_features(ProjectAsset PUBLIC cxx_std_23)
target_link_libraries(ProjectAsset PUBLIC Core Filesystem)
target_include_directories(ProjectAsset PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Public>
    $<INSTALL_INTERFACE:ProjectAsset/include>
)
install(TARGETS ProjectAsset EXPORT ProjectAssetTargets
    ARCHIVE DESTINATION ProjectAsset/lib
    LIBRARY DESTINATION ProjectAsset/lib
    RUNTIME DESTINATION ProjectAsset/bin)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/" DESTINATION ProjectAsset/include FILES_MATCHING PATTERN "*.h")
include(CMakePackageConfigHelpers)
configure_package_config_file(CMake/ProjectAssetConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/ProjectAssetConfig.cmake" INSTALL_DESTINATION ProjectAsset)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ProjectAssetConfig.cmake" DESTINATION ProjectAsset)
install(EXPORT ProjectAssetTargets FILE ProjectAssetTargets.cmake NAMESPACE Aether:: DESTINATION ProjectAsset)
