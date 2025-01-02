if(MSVC)
    include("${CMAKE_SOURCE_DIR}/CMake/VsConfig.cmake")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(MODULE_NAME Window)
file(GLOB_RECURSE FILES
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" 
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)



add_library(${MODULE_NAME} STATIC ${FILES} )

target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Vendor")
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Core/src")
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Math/src")
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Filesystem/src")
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Render/src")
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/ThirdParty/glfw-3.4/include")
target_include_directories(${MODULE_NAME} PUBLIC "${VULKAN_INCLUDE_DIR}")
target_link_libraries(${MODULE_NAME} Core)
target_link_libraries(${MODULE_NAME} glfw)








