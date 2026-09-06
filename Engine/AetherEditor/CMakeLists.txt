if(MSVC)
    include("${CMAKE_SOURCE_DIR}/CMake/VsConfig.cmake")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(MODULE_NAME AetherEditor)
file(GLOB_RECURSE FILES
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" 
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(${MODULE_NAME}  ${FILES})

target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(${MODULE_NAME} PUBLIC Entry)
target_link_libraries(${MODULE_NAME} PUBLIC ImGui)
target_link_libraries(${MODULE_NAME} PUBLIC Debug)

target_link_libraries(${MODULE_NAME} PUBLIC UI)
find_package(efsw REQUIRED CONFIG)
target_link_libraries(${MODULE_NAME} PUBLIC efsw::efsw-static)
target_link_libraries(${MODULE_NAME} PUBLIC Project)
target_link_libraries(${MODULE_NAME} PUBLIC Async)


if(WIN32 AND TARGET SDL3::SDL3-shared)
    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:SDL3::SDL3-shared>"
            "$<TARGET_FILE_DIR:${MODULE_NAME}>"
        VERBATIM
    )
endif()



