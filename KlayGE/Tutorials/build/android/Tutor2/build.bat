call %ANDROID_NDK%\ndk-build -j 3

if NOT EXIST assets mkdir assets
del /Q /F assets\*.*

copy ..\..\..\..\bin\android_armeabi\KlayGE.cfg assets
copy ..\..\..\..\media\Fonts\gkai00mp.kfont assets
copy ..\..\..\..\media\RenderFX\Font.kfx assets
copy ..\..\..\..\media\RenderFX\RenderableHelper.kfx assets
copy ..\..\..\..\Samples\media\Common\teapot.meshml.model_bin assets

call ant debug
adb install -r bin\KlayGE_Tutor2-debug.apk
