cmake_minimum_required(VERSION 3.18)

# Also supports testing Script alone without building renderer dependencies.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(AetherScriptTests LANGUAGES CXX)
    include("${CMAKE_CURRENT_SOURCE_DIR}/../../CMake/Local.cmake")
    add_subdirectory("../../Runtime/Script" Script)
    enable_testing()
endif()

get_target_property(_dotnet_root Script AETHER_SCRIPT_DOTNET_ROOT)
if(WIN32)
    set(_dotnet "${_dotnet_root}/dotnet.exe")
else()
    set(_dotnet "${_dotnet_root}/dotnet")
endif()
set(_managed_output "${CMAKE_CURRENT_BINARY_DIR}/ManagedIL/$<CONFIG>")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/empty-nuget")
add_custom_command(
    OUTPUT "${_managed_output}/ScriptFixture.dll"
    COMMAND "${CMAKE_COMMAND}" -E env
        "DOTNET_ROOT=${_dotnet_root}"
        "DOTNET_CLI_HOME=${CMAKE_CURRENT_BINARY_DIR}/.dotnet"
        "DOTNET_CLI_TELEMETRY_OPTOUT=1"
        "DOTNET_NOLOGO=1"
        "NUGET_PACKAGES=${CMAKE_CURRENT_BINARY_DIR}/.nuget/packages"
        "${_dotnet}" build "${CMAKE_CURRENT_SOURCE_DIR}/Managed/ScriptFixture.csproj"
        --configuration "$<IF:$<CONFIG:Debug>,Debug,Release>"
        --output "${_managed_output}"
        --source "${CMAKE_CURRENT_BINARY_DIR}/empty-nuget"
        "-p:BaseIntermediateOutputPath=${CMAKE_CURRENT_BINARY_DIR}/obj/"
    DEPENDS Managed/ScriptFixture.csproj Managed/EntryPoints.cs
    COMMENT "Building managed Script integration fixture"
    VERBATIM)
add_custom_target(ScriptFixture DEPENDS "${_managed_output}/ScriptFixture.dll")

add_executable(ScriptTests main.cpp)
target_link_libraries(ScriptTests PRIVATE Aether::Script)
add_dependencies(ScriptTests ScriptFixture)
set(_runtime_temp "${CMAKE_CURRENT_BINARY_DIR}/runtime-temp")
file(MAKE_DIRECTORY "${_runtime_temp}")
add_test(NAME ScriptTests COMMAND "${CMAKE_COMMAND}" -E env
    "TMPDIR=${_runtime_temp}" "TMP=${_runtime_temp}" "TEMP=${_runtime_temp}" "TEMPDIR=${_runtime_temp}"
    "$<TARGET_FILE:ScriptTests>" "${_managed_output}/ScriptFixture.dll" "${_dotnet_root}" "${_runtime_temp}")
set_tests_properties(ScriptTests PROPERTIES TIMEOUT 60)

add_executable(ScriptTemporaryConfigTests TemporaryRuntimeConfigTests.cpp)
target_link_libraries(ScriptTemporaryConfigTests PRIVATE Aether::Script)
target_include_directories(ScriptTemporaryConfigTests PRIVATE "../../Runtime/Script/src/Private")
add_test(NAME ScriptTemporaryConfigTests COMMAND ScriptTemporaryConfigTests normal)

set(_abort_modes create-failure file-cleanup-failure directory-cleanup-failure)
if(UNIX)
    list(APPEND _abort_modes write-failure)
endif()
foreach(_mode IN LISTS _abort_modes)
    add_test(NAME "ScriptTemporaryConfig.${_mode}" COMMAND "${CMAKE_COMMAND}"
        "-DTEST_EXECUTABLE=$<TARGET_FILE:ScriptTemporaryConfigTests>"
        "-DTEST_MODE=${_mode}" -P "${CMAKE_CURRENT_SOURCE_DIR}/CheckAbort.cmake")
    set_tests_properties("ScriptTemporaryConfig.${_mode}" PROPERTIES TIMEOUT 15)
endforeach()
