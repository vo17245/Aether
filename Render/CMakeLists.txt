if(MSVC)
    include("${CMAKE_SOURCE_DIR}/CMake/VsConfig.cmake")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(MODULE_NAME Render)
file(GLOB_RECURSE FILES
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" 
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_library(${MODULE_NAME} STATIC ${FILES})

target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Vendor")
target_link_libraries(${MODULE_NAME} PUBLIC Vulkan)
target_link_libraries(${MODULE_NAME} PUBLIC Filesystem )
target_link_libraries(${MODULE_NAME} PUBLIC glslang )
target_link_libraries(${MODULE_NAME} PUBLIC Core)
target_link_libraries(${MODULE_NAME} PUBLIC glfw_header)
target_link_libraries(${MODULE_NAME} PUBLIC Window_header)



