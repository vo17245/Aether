@echo off
setlocal
pushd "%~dp0depot_tools" || exit /b 1
chcp 65001 >nul
python build.py
set "BUILD_EXIT_CODE=%ERRORLEVEL%"
popd
exit /b %BUILD_EXIT_CODE%
