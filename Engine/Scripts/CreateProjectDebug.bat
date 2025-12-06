@echo off
cd ..
cmake  -B Build -S .    -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 17 2022" -A x64 -DAETHER_RUNTIME_CHECK=1 -DAETHER_ENABLE_DEBUG_LOG=1  -DCMAKE_INSTALL_PREFIX=Packages
cmake  -B DummyBuild -S .   -DCMAKE_EXPORT_COMPILE_COMMANDS=on -DCMAKE_BUILD_TYPE=Debug -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DAETHER_RUNTIME_CHECK=1 -DAETHER_ENABLE_DEBUG_LOG=1 
if exist DummyBuild\compile_commands.json (
    copy /Y DummyBuild\compile_commands.json . > nul
) else (
    echo Warning: DummyBuild\compile_commands.json not found.
)
cd Scripts
pause