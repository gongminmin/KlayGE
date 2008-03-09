CALL "%VS80COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv Tools.sln /Build "Debug|Win32"
pause
