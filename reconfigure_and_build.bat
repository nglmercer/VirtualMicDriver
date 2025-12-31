@echo off
echo Reconfiguring for Visual Studio 2019...
cd /d "c:\Users\mm\Documents\audio\vaudio_windows\VirtualMicDriver"
rmdir /s /q build
"C:\Program Files\CMake\bin\cmake.exe" -S . -B build -G "Visual Studio 16 2019" -A x64
echo.
echo Building...
"C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe" "build\VirtualMicDriver.sln" /p:Configuration=Debug /p:Platform=x64 /v:normal
echo.
echo Complete. Exit code: %ERRORLEVEL%
pause
