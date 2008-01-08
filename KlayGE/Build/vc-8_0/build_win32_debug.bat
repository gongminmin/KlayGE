CALL "%VS80COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv KlayGE.sln /Build "Debug|Win32"
pause
