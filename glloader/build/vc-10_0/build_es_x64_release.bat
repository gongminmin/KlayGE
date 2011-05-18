CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86_amd64
devenv glloader.sln /Build "Release-GLES|x64"
pause
