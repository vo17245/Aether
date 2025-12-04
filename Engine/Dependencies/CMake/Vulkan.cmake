set(MODULE_NAME Vulkan)
add_library(${MODULE_NAME} INTERFACE)
include("${CMAKE_SOURCE_DIR}/CMake/Local.cmake")
target_link_libraries(${MODULE_NAME} INTERFACE "${VULKAN_LIB_DIR}/vulkan-1.lib")
target_include_directories(${MODULE_NAME} INTERFACE "${VULKAN_INCLUDE_DIR}")





#==================install config=========================
install(TARGETS ${MODULE_NAME}
    EXPORT ${MODULE_NAME}Targets
)

include(CMakePackageConfigHelpers)

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
    DESTINATION ${MODULE_NAME}
)

