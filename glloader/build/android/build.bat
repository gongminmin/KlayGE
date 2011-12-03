set PATH=%PATH%;%ANDROID_NDK%

REM Workaround for NDK r7
mkdir obj
mkdir obj\local
mkdir obj\local\armeabi
mkdir obj\local\armeabi-v7a
copy "%ANDROID_NDK%\sources\cxx-stl\gnu-libstdc++\libs\armeabi\libgnustl_static.a" obj\local\armeabi\
copy "%ANDROID_NDK%\sources\cxx-stl\gnu-libstdc++\libs\armeabi-v7a\libgnustl_static.a" obj\local\armeabi-v7a\

ndk-build
