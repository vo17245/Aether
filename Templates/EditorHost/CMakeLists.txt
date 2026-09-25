cmake_minimum_required(VERSION 3.20)
project(EditorHostSample LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(GAME_BUILD_EDITOR "Build this game's editor host" ON)
find_package(Entry CONFIG REQUIRED)
find_package(GameFeature CONFIG REQUIRED)
if(GAME_BUILD_EDITOR)
    find_package(EditorFramework CONFIG REQUIRED)
    find_package(EditorImGui CONFIG REQUIRED)
endif()

add_subdirectory(Features/PaletteFeature)
add_subdirectory(Features/SampleSceneFeature)

add_executable(SampleGame Game/RuntimeApplication.cpp Game/RuntimeRegistration.cpp)
target_compile_features(SampleGame PRIVATE cxx_std_23)
target_link_libraries(SampleGame PRIVATE Aether::Entry SampleSceneFeatureRuntime)

if(GAME_BUILD_EDITOR)
    add_executable(SampleGameEditor Game/EditorApplication.cpp Game/RuntimeRegistration.cpp)
    target_compile_features(SampleGameEditor PRIVATE cxx_std_23)
    target_link_libraries(SampleGameEditor PRIVATE Aether::Entry Aether::EditorFramework
        Aether::EditorImGui SampleSceneFeatureEditor PaletteFeatureEditor)
    file(COPY TestProject DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endif()
