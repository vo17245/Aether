if(MSVC)
    include("${CMAKE_SOURCE_DIR}/CMake/VsConfig.cmake")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(MODULE_NAME Filesystem)
file(GLOB_RECURSE FILES
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" 
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

file(GLOB_RECURSE PLATFORM_FILES
"${CMAKE_CURRENT_SOURCE_DIR}/platform/*.cpp" 
"${CMAKE_CURRENT_SOURCE_DIR}/platform/*.h" 
)

add_library(${MODULE_NAME} STATIC ${FILES} ${PLATFORM_FILES})

target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Vendor")
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Core/src")
target_link_libraries(${MODULE_NAME} Core)








