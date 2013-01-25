call %ANDROID_NDK%\ndk-build -j 3

if NOT EXIST assets mkdir assets
del /Q /F assets\*.*

copy ..\..\..\..\bin\android_armeabi\KlayGE.cfg assets
copy ..\..\..\..\media\Fonts\gkai00mp.kfont assets
copy ..\..\..\..\media\RenderFX\Font.kfx assets
copy ..\..\..\..\media\RenderFX\ColorGrading.kfx assets
copy ..\..\..\..\media\RenderFX\DepthToSM.kfx assets
copy ..\..\..\..\media\RenderFX\RenderableHelper.kfx assets
copy ..\..\..\..\media\RenderFX\UI.kfx assets
copy ..\..\..\..\media\PostProcessors\ColorGrading.ppml assets
copy ..\..\..\..\media\PostProcessors\DepthToSM.ppml assets
copy ..\..\..\..\media\Textures\2D\powered_by_klayge.dds assets
copy ..\..\..\..\media\Textures\2D\ui.dds assets
copy ..\..\..\..\media\Textures\Cube\uffizi_cross_c.dds assets
copy ..\..\..\..\media\Textures\Cube\uffizi_cross_y.dds assets
copy ..\..\..\..\Samples\media\Common\teapot.meshml.model_bin assets
copy ..\..\..\media\Refract\Refract.kfx assets
copy ..\..\..\media\Refract\Refract.uiml assets

call ant debug
adb install -r bin\KlayGE_Refract-debug.apk
