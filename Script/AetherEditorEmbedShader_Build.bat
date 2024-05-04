@echo off
cd ..
if %ERRORLEVEL% neq 0 (
    echo "cd command failed."
    pause
    exit /b %ERRORLEVEL%
)


DepotTools\bin\x64_windows\python-3.12.3-embed-amd64\python.exe DepotTools\src\Entry.py AetherEditorEmbedShader CreateNinja
if %ERRORLEVEL% neq 0 (
    echo "Python command failed."
    pause
    exit /b %ERRORLEVEL%
)
DepotTools\bin\x64_windows\python-3.12.3-embed-amd64\python.exe DepotTools\src\Entry.py AetherEditorEmbedShader Build
if %ERRORLEVEL% neq 0 (
    echo "Python command failed."
    pause
    exit /b %ERRORLEVEL%
)


cd Script
if %ERRORLEVEL% neq 0 (
    echo "cd command failed."
    pause
    exit /b %ERRORLEVEL%
)

pause