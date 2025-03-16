if(MSVC)
    include("${CMAKE_SOURCE_DIR}/CMake/VsConfig.cmake")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(MODULE_NAME Core.Tests)
file(GLOB_RECURSE FILES
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" 
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(${MODULE_NAME} ${FILES})

target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/Vendor")
target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/Core/src")
target_link_libraries(${MODULE_NAME} Core)









