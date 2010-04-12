CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv glloader.sln /Build "Release-GLES|Win32"
pause
