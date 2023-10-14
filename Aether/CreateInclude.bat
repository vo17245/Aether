@echo off

mkdir include
mkdir include\Aether
xcopy /E /I /Y src\* include\Aether
del /S /Q include\Aether\*.cpp
pause