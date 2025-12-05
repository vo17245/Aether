cmake_minimum_required(VERSION 3.14)
project(HelloWorld)
if(MSVC)
    include("${CMAKE_SOURCE_DIR}/CMake/VsConfig.cmake")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(CMAKE_CXX_STANDARD 23)
include("${CMAKE_SOURCE_DIR}/CMake/Local.cmake")
set(CMAKE_PREFIX_PATH
    "${AETHER_DEPENDENCIES_PACKAGES_DIR}"
    "${AETHER_PACKAGES_DIR}"
)
add_subdirectory("HelloWorld")
