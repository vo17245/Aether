if(MSVC)
    include("${CMAKE_SOURCE_DIR}/CMake/VsConfig.cmake")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(MODULE_NAME HelloWorld)
file(GLOB_RECURSE FILES
"src/*.cpp" 
"src/*.h"
)

add_executable(${MODULE_NAME}  ${FILES})

target_include_directories(${MODULE_NAME} PUBLIC "src")
find_package(Entry REQUIRED CONFIG)
target_link_libraries(${MODULE_NAME} PUBLIC Aether::Entry)




