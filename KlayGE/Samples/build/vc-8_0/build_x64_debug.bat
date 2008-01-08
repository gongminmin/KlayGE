CALL "%ProgramFiles%\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86_amd64
devenv Samples.sln /Build "Debug|x64"
pause
