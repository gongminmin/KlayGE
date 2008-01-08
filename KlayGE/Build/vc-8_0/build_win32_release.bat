CALL "%VS80COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv KlayGE.sln /Build "Release|Win32"
pause
