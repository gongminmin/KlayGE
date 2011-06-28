CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv Samples.sln /Build "Debug|Win32"
devenv Samples.sln /Build "Release|Win32"

CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86_amd64
devenv Samples.sln /Build "Debug|x64"
devenv Samples.sln /Build "Release|x64"

if not "%1" == "-q" pause
