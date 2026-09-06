



add_executable(Sandbox Source/Sandbox.cpp)
target_link_libraries(Sandbox PRIVATE Entry)

if(MSVC)
    target_compile_options(Sandbox PRIVATE /utf-8)
endif()

if(WIN32 AND TARGET SDL3::SDL3-shared)
    add_custom_command(TARGET Sandbox POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:SDL3::SDL3-shared>"
            "$<TARGET_FILE_DIR:Sandbox>"
        VERBATIM
    )
endif()

