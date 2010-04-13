CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86_amd64
devenv KlayGE.sln /Build "Release|x64"
pause
