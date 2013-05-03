call %ANDROID_NDK%\ndk-build -j 3

if NOT EXIST assets mkdir assets
del /Q /F assets\*.*

copy ..\..\..\..\bin\KlayGE.cfg assets
copy ..\..\..\..\media\Fonts\gkai00mp.kfont assets
copy ..\..\..\..\media\RenderFX\Blur.kfx assets
copy ..\..\..\..\media\RenderFX\Copy.kfx assets
copy ..\..\..\..\media\RenderFX\Font.kfx assets
copy ..\..\..\..\media\RenderFX\LensEffects.kfx assets
copy ..\..\..\..\media\RenderFX\PostToneMapping.kfx assets
copy ..\..\..\..\media\RenderFX\SumLum.kfx assets
copy ..\..\..\..\media\RenderFX\ToneMapping.kfx assets
copy ..\..\..\..\media\RenderFX\UI.kfx assets
copy ..\..\..\..\media\PostProcessors\Copy.ppml assets
copy ..\..\..\..\media\PostProcessors\LensEffects.ppml assets
copy ..\..\..\..\media\PostProcessors\PostToneMapping.ppml assets
copy ..\..\..\..\media\Textures\2D\powered_by_klayge.dds assets
copy ..\..\..\..\media\Textures\2D\ui.dds assets
copy ..\..\..\..\media\Textures\3D\color_grading.dds assets
copy ..\..\..\media\Text\text.txt assets
copy ..\..\..\media\Text\Text.uiml assets

call ant debug
adb install -r bin\KlayGE_Text-debug.apk
