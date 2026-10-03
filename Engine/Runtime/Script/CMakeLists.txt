set(MODULE_NAME Script)
if(WIN32)
    set(_dotnet_os win)
    set(_hostfxr_name hostfxr.dll)
elseif(APPLE)
    set(_dotnet_os mac)
    set(_dotnet_rid_os osx)
    set(_hostfxr_name libhostfxr.dylib)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(_dotnet_os linux)
    set(_hostfxr_name libhostfxr.so)
else()
    message(FATAL_ERROR "Script is not supported on ${CMAKE_SYSTEM_NAME}")
endif()
if(NOT _dotnet_rid_os)
    set(_dotnet_rid_os "${_dotnet_os}")
endif()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64|ARM64)$")
    set(_dotnet_arch arm64)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
    set(_dotnet_arch x64)
else()
    message(FATAL_ERROR "Script requires an arm64 or x64 .NET SDK")
endif()

if(NOT AETHER_DEPENDENCIES_DIR)
    message(FATAL_ERROR "Set AETHER_DEPENDENCIES_DIR in CMake/Local.cmake before building Script")
endif()
set(SCRIPT_DOTNET_ROOT "${AETHER_DEPENDENCIES_DIR}/Bin/${_dotnet_os}-${_dotnet_arch}/dotnet")
get_filename_component(SCRIPT_DOTNET_ROOT "${SCRIPT_DOTNET_ROOT}" ABSOLUTE)

# Resolve hosting files only from the configured SDK. Natural sorting handles
# 10.0.12 versus 10.0.9 without hardcoding a runtime patch version.
file(GLOB _hostfxr_paths "${SCRIPT_DOTNET_ROOT}/host/fxr/*/${_hostfxr_name}")
file(GLOB _hosting_headers
    "${SCRIPT_DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${_dotnet_rid_os}-${_dotnet_arch}/*/runtimes/${_dotnet_rid_os}-${_dotnet_arch}/native/hostfxr.h")
if(NOT _hostfxr_paths OR NOT _hosting_headers)
    message(FATAL_ERROR "Script needs a .NET SDK with hosting headers at '${SCRIPT_DOTNET_ROOT}'. Check AETHER_DEPENDENCIES_DIR in CMake/Local.cmake, or disable AETHER_BUILD_SCRIPT.")
endif()
list(SORT _hostfxr_paths COMPARE NATURAL ORDER DESCENDING)
list(SORT _hosting_headers COMPARE NATURAL ORDER DESCENDING)
list(GET _hostfxr_paths 0 SCRIPT_HOSTFXR_PATH)
list(GET _hosting_headers 0 _hosting_header)
get_filename_component(_hosting_include "${_hosting_header}" DIRECTORY)

# Embed configuration for the newest installed stable Microsoft.NETCore.App.
file(GLOB _runtime_directories "${SCRIPT_DOTNET_ROOT}/shared/Microsoft.NETCore.App/*")
set(_runtime_versions)
foreach(_directory IN LISTS _runtime_directories)
    get_filename_component(_version "${_directory}" NAME)
    if(IS_DIRECTORY "${_directory}" AND _version MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
        list(APPEND _runtime_versions "${_version}")
    endif()
endforeach()
if(NOT _runtime_versions)
    message(FATAL_ERROR "Script needs a stable Microsoft.NETCore.App runtime in '${SCRIPT_DOTNET_ROOT}'")
endif()
list(SORT _runtime_versions COMPARE NATURAL ORDER DESCENDING)
list(GET _runtime_versions 0 SCRIPT_DOTNET_RUNTIME_VERSION)
string(REGEX MATCH "^[0-9]+\\.[0-9]+" _runtime_family "${SCRIPT_DOTNET_RUNTIME_VERSION}")
set(SCRIPT_DOTNET_TFM "net${_runtime_family}")
configure_file(CMake/ScriptConfig.h.in generated/ScriptConfig.h @ONLY)

add_library(${MODULE_NAME} STATIC src/Script/Runtime.cpp src/Private/TemporaryRuntimeConfig.cpp)
add_library(Aether::Script ALIAS Script)
set_property(TARGET Script PROPERTY AETHER_SCRIPT_DOTNET_ROOT "${SCRIPT_DOTNET_ROOT}")
target_compile_features(${MODULE_NAME} PUBLIC cxx_std_23)
target_include_directories(${MODULE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:Script/include>
    PRIVATE "${_hosting_include}" "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(${MODULE_NAME} PRIVATE ${CMAKE_DL_LIBS})

install(TARGETS ${MODULE_NAME} EXPORT ScriptTargets
    ARCHIVE DESTINATION Script/lib
    LIBRARY DESTINATION Script/lib
    RUNTIME DESTINATION Script/bin)
install(DIRECTORY src/Script DESTINATION Script/include FILES_MATCHING PATTERN "*.h")
include(CMakePackageConfigHelpers)
configure_package_config_file(CMake/ScriptConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/ScriptConfig.cmake" INSTALL_DESTINATION Script)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ScriptConfig.cmake" DESTINATION Script)
install(EXPORT ScriptTargets FILE ScriptTargets.cmake NAMESPACE Aether:: DESTINATION Script)
