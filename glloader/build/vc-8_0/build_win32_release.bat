CALL "%ProgramFiles%\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86
devenv glloader.sln /Build "Release|Win32"
pause
