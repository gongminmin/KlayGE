call %ANDROID_NDK%\ndk-build -j 3

if NOT EXIST assets mkdir assets
del /Q /F assets\*.*

copy ..\..\..\..\bin\android_armeabi\KlayGE.cfg assets
copy ..\..\..\..\media\Fonts\gkai00mp.kfont assets
copy ..\..\..\..\media\RenderFX\Font.kfx assets

call ant debug
adb install -r bin\KlayGE_InputCaps-debug.apk
