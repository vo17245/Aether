cd ..
cmake  -B DummyBuild -S .   -DCMAKE_EXPORT_COMPILE_COMMANDS=on -DCMAKE_BUILD_TYPE=Debug -G Ninja -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc 
cp DummyBuild/compile_commands.json .
cd Script
pause