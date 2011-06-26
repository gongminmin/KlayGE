CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv glloader.sln /Build "Debug|Win32"
devenv glloader.sln /Build "Release|Win32"
devenv glloader.sln /Build "Debug-GLES|Win32"
devenv glloader.sln /Build "Release-GLES|Win32"

CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86_amd64
devenv glloader.sln /Build "Debug|x64"
devenv glloader.sln /Build "Release|x64"
devenv glloader.sln /Build "Debug-GLES|x64"
devenv glloader.sln /Build "Release-GLES|x64"

if not "%1" == "-q" pause
