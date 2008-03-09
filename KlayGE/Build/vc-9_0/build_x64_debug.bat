CALL "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" x86_amd64
devenv KlayGE.sln /Build "Debug|x64"
pause
