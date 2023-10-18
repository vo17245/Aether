cmake_minimum_required(VERSION 3.9)
project(Aether)

set(CMAKE_CXX_STANDARD 20)
set(AETHER_ROOT "..")
set(VENDOR_DIR "${AETHER_ROOT}/Vendor")
set(DEPENDENCIES_DIR "${AETHER_ROOT}/Dependencies")

# Global Configure
set(GLOBAL_INCLUDE_DIR 
${VENDOR_DIR}
"${DEPENDENCIES_DIR}/glew-2.1.0/include"
"${DEPENDENCIES_DIR}/glfw-3.3.8.bin.WIN64/include"
"${DEPENDENCIES_DIR}/assimp-5.3.0/include"
)
set(GLOBAL_LINK_DIR 
"${DEPENDENCIES_DIR}/glew-2.1.0/lib/x64"
"${DEPENDENCIES_DIR}/glfw-3.3.8.bin.WIN64/lib-vc2022"
"${DEPENDENCIES_DIR}/assimp-5.3.0/lib"
)
set(GLOBAL_LIB "glfw3_mt.lib" "glew32s" "assimp-vc143-mt" "OpenGL32")
file(GLOB_RECURSE GLOBAL_FILE
"src/*.cpp" 
"src/*.h"
)
add_definitions(-DGLEW_STATIC)

# compile mode
#set(CMAKE_BUILD_TYPE "Debug")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    # Debug Configure
    add_definitions(-DDEBUG)
    set(DEBUG_INCLUDE_DIR)
    set(DEBUG_LINK_DIR )
    set(DEBUG_LIB)
    set(DEBUG_FILE)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    # Release Configure
    add_definitions(-DNDEBUG)
    set(RELEASE_INCLUDE_DIR)
    set(RELEASE_LINK_DIR )
    set(RELEASE_LIB)
    set(RELEASE_FILE)
else ()
    # Other Configure
endif ()
include("../CMake/vs_config.cmake")
set(PROJECT_FILE ${GLOBAL_FILE} ${DEBUG_FILE} ${RELEASE_FILE})
set(PROJECT_INCLUDE_DIR ${GLOBAL_INCLUDE_DIR} ${DEBUG_INCLUDE_DIR} ${RELEASE_INCLUDE_DIR})
set(PROJECT_LINK_DIR ${GLOBAL_LINK_DIR} ${DEBUG_LINK_DIR} ${RELEASE_LINK_DIR})
set(PROJECT_LIB ${GLOBAL_LIB} ${DEBUG_LIB} ${RELEASE_LIB})
link_directories(${PROJECT_LINK_DIR})
include_directories(${PROJECT_INCLUDE_DIR})
add_library(${PROJECT_NAME} STATIC ${PROJECT_FILE})
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIB})
