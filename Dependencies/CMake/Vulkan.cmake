add_library(Vulkan INTERFACE)
include("${CMAKE_SOURCE_DIR}/CMake/Local.cmake")
target_link_libraries(Vulkan INTERFACE "${VULKAN_LIB_DIR}/vulkan-1.lib")
target_include_directories(Vulkan INTERFACE "${VULKAN_INCLUDE_DIR}")
