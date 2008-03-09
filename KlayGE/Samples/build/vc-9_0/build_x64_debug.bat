CALL "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" x86_amd64
devenv Samples.sln /Build "Debug|x64"
pause
