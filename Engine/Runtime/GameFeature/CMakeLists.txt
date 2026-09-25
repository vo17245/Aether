add_library(GameFeature STATIC
    src/Private/EntityMetadata.cpp
    src/Private/ProjectWorld.cpp
    src/Private/RuntimeFeatureRegistrar.cpp
    src/Private/WorldMount.cpp
)
add_library(Aether::GameFeature ALIAS GameFeature)
target_compile_features(GameFeature PUBLIC cxx_std_23)
target_link_libraries(GameFeature PUBLIC World ProjectAsset)
target_include_directories(GameFeature PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Public>
    $<INSTALL_INTERFACE:GameFeature/include>
)
install(TARGETS GameFeature EXPORT GameFeatureTargets
    ARCHIVE DESTINATION GameFeature/lib
    LIBRARY DESTINATION GameFeature/lib
    RUNTIME DESTINATION GameFeature/bin)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/" DESTINATION GameFeature/include FILES_MATCHING PATTERN "*.h")
include(CMakePackageConfigHelpers)
configure_package_config_file(CMake/GameFeatureConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/GameFeatureConfig.cmake" INSTALL_DESTINATION GameFeature)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GameFeatureConfig.cmake" DESTINATION GameFeature)
install(EXPORT GameFeatureTargets FILE GameFeatureTargets.cmake NAMESPACE Aether:: DESTINATION GameFeature)
