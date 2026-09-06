



add_executable(Sandbox Source/Sandbox.cpp)
target_link_libraries(Sandbox PRIVATE Entry)

if(MSVC)
    target_compile_options(Sandbox PRIVATE /utf-8)
endif()

