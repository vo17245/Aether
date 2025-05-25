if(MSVC)
    include("${CMAKE_SOURCE_DIR}/CMake/VsConfig.cmake")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(MODULE_NAME UI)
file(GLOB_RECURSE FILES
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" 
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_library(${MODULE_NAME} STATIC ${FILES})

target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Vendor")
target_link_libraries(${MODULE_NAME} PUBLIC Render)
target_link_libraries(${MODULE_NAME} PUBLIC Core)
target_link_libraries(${MODULE_NAME} PUBLIC Window)
target_link_libraries(${MODULE_NAME} PUBLIC Resource)
target_link_libraries(${MODULE_NAME} PUBLIC Scene)
target_link_libraries(${MODULE_NAME} PUBLIC tinyxml2)
target_link_libraries(${MODULE_NAME} PUBLIC Text)


if(AETHER_RUNTIME_CHECK)
target_compile_definitions(${MODULE_NAME} PUBLIC AETHER_RUNTIME_CHECK=1)
endif()