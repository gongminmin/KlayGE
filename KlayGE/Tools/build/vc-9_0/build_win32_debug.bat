CALL "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv Tools.sln /Build "Debug|Win32"
pause
