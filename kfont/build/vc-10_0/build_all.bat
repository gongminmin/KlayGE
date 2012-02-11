CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv kfont.sln /Build "Debug|Win32"
devenv kfont.sln /Build "Release|Win32"

CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86_amd64
devenv kfont.sln /Build "Debug|x64"
devenv kfont.sln /Build "Release|x64"

if not "%1" == "-q" pause
