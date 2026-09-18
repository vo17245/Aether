set(MODULE_NAME Physics.Tests)
file(GLOB_RECURSE FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(${MODULE_NAME} ${FILES})
target_compile_features(${MODULE_NAME} PRIVATE cxx_std_23)
target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/Vendor")
target_link_libraries(${MODULE_NAME} PRIVATE Physics)
