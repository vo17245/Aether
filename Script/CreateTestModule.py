import os
import sys
import shutil

if len(sys.argv)!=2:
    print("Usage: python CreateTestModule.py <module_name>") 

module_name=sys.argv[1]

cmake_template="""
if(MSVC)
    include("${CMAKE_SOURCE_DIR}/CMake/VsConfig.cmake")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(MODULE_NAME MODULE_NAME_PLACEHOLDER)
file(GLOB_RECURSE FILES
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" 
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(${MODULE_NAME}  ${FILES})

target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Vendor")
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Core/src")
"""
cmake=cmake_template.replace("MODULE_NAME_PLACEHOLDER",module_name)

src_template="""
#include "Core/Base.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
using namespace Aether;

TEST_CASE("")
{

}

"""
os.mkdir("src")
with open("src/main.cpp","w",encoding="utf-8") as f:
    f.write(src_template)

with open("CMakeLists.txt","w",encoding="utf-8") as f:
    f.write(cmake)
