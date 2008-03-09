CALL "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv Samples.sln /Build "Debug|Win32"
pause
