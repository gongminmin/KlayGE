CALL "%ProgramFiles%\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86_amd64
devenv glloader.sln /Build "Release|x64"
pause
