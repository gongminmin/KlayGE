CALL "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv glloader.sln /Build "Debug|Win32"
pause
