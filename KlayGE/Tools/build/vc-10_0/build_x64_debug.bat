CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86_amd64
devenv Tools.sln /Build "Debug|x64"
pause
