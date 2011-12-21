echo "Building KlayGE Core..."

cd Core

call %ANDROID_NDK%\ndk-build

if NOT EXIST ..\..\..\lib\android_armeabi mkdir ..\..\..\lib\android_armeabi
copy /Y obj\local\armeabi\libKlayGE_Core_gcc.a ..\..\..\lib\android_armeabi\

if NOT EXIST ..\..\..\lib\android_armeabi-v7a mkdir ..\..\..\lib\android_armeabi-v7a
copy /Y obj\local\armeabi-v7a\libKlayGE_Core_gcc.a ..\..\..\lib\android_armeabi-v7a\

if NOT EXIST ..\..\..\lib\android_x86 mkdir ..\..\..\lib\android_x86
copy /Y obj\local\x86\libKlayGE_Core_gcc.a ..\..\..\lib\android_x86\

cd ..


echo "Building KlayGE OpenGLES2 plugin..."

cd Plugins/Render/OpenGLES2

call %ANDROID_NDK%\ndk-build

if NOT EXIST ..\..\..\..\..\lib\android_armeabi mkdir ..\..\..\..\..\lib\android_armeabi
copy /Y obj\local\armeabi\libKlayGE_RenderEngine_OpenGLES2_gcc.a ..\..\..\..\..\lib\android_armeabi\

if NOT EXIST ..\..\..\..\..\lib\android_armeabi-v7a mkdir ..\..\..\..\..\lib\android_armeabi-v7a
copy /Y obj\local\armeabi-v7a\libKlayGE_RenderEngine_OpenGLES2_gcc.a ..\..\..\..\..\lib\android_armeabi-v7a\

if NOT EXIST ..\..\..\..\..\lib\android_x86 mkdir ..\..\..\..\..\lib\android_x86
copy /Y obj\local\x86\libKlayGE_RenderEngine_OpenGLES2_gcc.a ..\..\..\..\..\lib\android_x86\

cd ../../..


echo "Building KlayGE OCTree plugin..."

cd Plugins/Scene/OCTree

call %ANDROID_NDK%\ndk-build

if NOT EXIST ..\..\..\..\..\lib\android_armeabi mkdir ..\..\..\..\..\lib\android_armeabi
copy /Y obj\local\armeabi\libKlayGE_Scene_OCTree_gcc.a ..\..\..\..\..\lib\android_armeabi\

if NOT EXIST ..\..\..\..\..\lib\android_armeabi-v7a mkdir ..\..\..\..\..\lib\android_armeabi-v7a
copy /Y obj\local\armeabi-v7a\libKlayGE_Scene_OCTree_gcc.a ..\..\..\..\..\lib\android_armeabi-v7a\

if NOT EXIST ..\..\..\..\..\lib\android_x86 mkdir ..\..\..\..\..\lib\android_x86
copy /Y obj\local\x86\libKlayGE_Scene_OCTree_gcc.a ..\..\..\..\..\lib\android_x86\

cd ../../..
