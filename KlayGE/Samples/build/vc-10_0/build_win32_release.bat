CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv Samples.sln /Build "Release|Win32"
pause
