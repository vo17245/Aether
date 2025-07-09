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

target_link_libraries(${MODULE_NAME} PUBLIC Render)
target_link_libraries(${MODULE_NAME} PUBLIC Core)
target_link_libraries(${MODULE_NAME} PUBLIC Window)
target_link_libraries(${MODULE_NAME} PUBLIC Resource)
target_link_libraries(${MODULE_NAME} PUBLIC Scene)
find_package(tinyxml2 REQUIRED CONFIG)
target_link_libraries(${MODULE_NAME} PUBLIC tinyxml2)
target_link_libraries(${MODULE_NAME} PUBLIC Text)
find_package(nativefiledialog REQUIRED CONFIG)
#get_target_property(NFD_INCLUDE_DIR nativefiledialog INTERFACE_INCLUDE_DIRECTORIES)
#message(STATUS "nativefiledialog include dirs: ${NFD_INCLUDE_DIR}")
target_link_libraries(${MODULE_NAME} PUBLIC nativefiledialog)
find_package(Lua REQUIRED CONFIG)
target_link_libraries(${MODULE_NAME} PUBLIC Lua)

if(AETHER_RUNTIME_CHECK)
target_compile_definitions(${MODULE_NAME} PUBLIC AETHER_RUNTIME_CHECK=1)
endif()












target_include_directories(${MODULE_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${MODULE_NAME}/include>
)




#==================install config=========================
install(TARGETS ${MODULE_NAME}
    EXPORT ${MODULE_NAME}Targets
    ARCHIVE DESTINATION ${MODULE_NAME}/lib
    LIBRARY DESTINATION ${MODULE_NAME}/lib
    RUNTIME DESTINATION ${MODULE_NAME}/bin
)

include(CMakePackageConfigHelpers)
# install时拷贝头文件
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/"
    DESTINATION ${MODULE_NAME}/include
    FILES_MATCHING PATTERN "*.h"
)
# 2. 配置并生成 Config 文件
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake/${MODULE_NAME}Config.cmake.in"  # 需要你自己写
    "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}Config.cmake"
    INSTALL_DESTINATION ${MODULE_NAME}/
)

# 3. 安装 Config  文件
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}Config.cmake"
    DESTINATION ${MODULE_NAME}/
)

# 4. 安装导出 Targets
install(EXPORT ${MODULE_NAME}Targets
    FILE ${MODULE_NAME}Targets.cmake
    NAMESPACE Aether::
    DESTINATION ${MODULE_NAME}
)

