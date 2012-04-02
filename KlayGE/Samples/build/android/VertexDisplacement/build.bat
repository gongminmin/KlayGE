call %ANDROID_NDK%\ndk-build -j 3

if NOT EXIST assets mkdir assets
del /Q /F assets\*.*

copy ..\..\..\..\bin\android_armeabi\KlayGE.cfg assets
copy ..\..\..\..\media\Fonts\gkai00mp.kfont assets
copy ..\..\..\..\media\RenderFX\Font.kfx assets
copy ..\..\..\..\media\RenderFX\ColorGrading.kfx assets
copy ..\..\..\..\media\RenderFX\UI.kfx assets
copy ..\..\..\..\media\PostProcessors\ColorGrading.ppml assets
copy ..\..\..\..\media\Textures\2D\powered_by_klayge.dds assets
copy ..\..\..\..\media\Textures\2D\ui.dds assets
copy ..\..\..\media\VertexDisplacement\VertexDisplacement.kfx assets
copy ..\..\..\media\VertexDisplacement\VertexDisplacement.uiml assets

call ant debug
adb install -r bin\KlayGE_VertexDisplacement-debug.apk
