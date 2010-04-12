CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv glloader.sln /Build "Debug|Win32"
pause
