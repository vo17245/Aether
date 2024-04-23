@echo off
echo "=== Copy Assimp ==="
copy "Dependencies\assimp-5.3.0\bin\assimp-vc143-mt.dll"   "build\Sandbox\Debug"
copy "Dependencies\assimp-5.3.0\bin\assimp-vc143-mt.dll"   "build\AetherEditor\Debug"
echo "=== Copy Samplerate"
copy "Dependencies\x64_windows_msvc\samplerate\debug\bin\samplerate.dll" "build\Sandbox\Debug"
copy "Dependencies\x64_windows_msvc\samplerate\debug\bin\samplerate.dll" "build\AetherEditor\Debug"
pause