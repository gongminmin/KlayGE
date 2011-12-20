call %ANDROID_NDK%\ndk-build

if NOT EXIST ..\..\..\..\..\lib\android_armeabi mkdir ..\..\..\..\..\lib\android_armeabi
copy /Y obj\local\armeabi\libKlayGE_Scene_OCTree_gcc.a ..\..\..\..\..\lib\android_armeabi\

if NOT EXIST ..\..\..\..\..\lib\android_armeabi-v7a mkdir ..\..\..\..\..\lib\android_armeabi-v7a
copy /Y obj\local\armeabi-v7a\libKlayGE_Scene_OCTree_gcc.a ..\..\..\..\..\lib\android_armeabi-v7a\

if NOT EXIST ..\..\..\..\..\lib\android_x86 mkdir ..\..\..\..\..\lib\android_x86
copy /Y obj\local\x86\libKlayGE_Scene_OCTree_gcc.a ..\..\..\..\..\lib\android_x86\
