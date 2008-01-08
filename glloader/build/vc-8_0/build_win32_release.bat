CALL "%VS80COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv glloader.sln /Build "Release|Win32"
pause
