cmake_minimum_required(VERSION 3.10)
project(Aether)
set(CMAKE_CXX_STANDARD 23)
# 关闭严格别名和整数环绕优化
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fno-strict-overflow -fno-strict-aliasing)
endif()


include("CMake/Local.cmake")
add_subdirectory("Core")
add_subdirectory("Core.Tests")
add_subdirectory("Render")
add_subdirectory("Render.Tests")
add_subdirectory("ThirdParty")
add_subdirectory("Filesystem")
add_subdirectory("Filesystem.Tests")
add_subdirectory("Window")
add_subdirectory("Window.Tests")