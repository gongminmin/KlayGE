CALL "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv glloader.sln /Build "Debug|Win32"
devenv glloader.sln /Build "Release|Win32"

CALL "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" x86_amd64
devenv glloader.sln /Build "Debug|x64"
devenv glloader.sln /Build "Release|x64"

if not "%1" == "-q" pause
