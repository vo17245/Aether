cd /D Aether
call CreateInclude.bat
cd ..
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
