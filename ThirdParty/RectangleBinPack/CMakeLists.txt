file(GLOB_RECURSE FILES
"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp" 
"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
)

set(MODULE_NAME "RectangleBinPack")
add_library(${MODULE_NAME} STATIC ${FILES})
target_include_directories(${MODULE_NAME} PUBLIC
    "Source"
)


