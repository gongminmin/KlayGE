CALL "%VS80COMNTOOLS%..\..\VC\vcvarsall.bat" x86_amd64
devenv Samples.sln /Build "Release|x64"
pause
