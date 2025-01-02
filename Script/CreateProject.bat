cd ..
cmake  -B Build -S .   -DCMAKE_EXPORT_COMPILE_COMMANDS=on -DCMAKE_BUILD_TYPE=Debug -G Ninja
cp Build/compile_commands.json .
cd Script
pause