CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv glloader.sln /Build "Debug-GLES|Win32"
pause
