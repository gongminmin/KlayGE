#!/bin/bash

$ANDROID_NDK/ndk-build -j 3

if [ ! -d assets ]; then
	mkdir assets
fi
rm -rf assets\*.*

cp ../../../../bin/android_armeabi/KlayGE.cfg assets
cp ../../../../media/Fonts/gkai00mp.kfont assets
cp ../../../../media/RenderFX/Font.kfx assets
cp ../../../../media/RenderFX/ColorGrading.kfx assets
cp ../../../../media/RenderFX/PostProcess.kfx assets
cp ../../../../media/RenderFX/UI.kfx assets
cp ../../../../media/PostProcessors/ColorGrading.ppml assets
cp ../../../../media/Textures/2D/powered_by_klayge.dds assets
cp ../../../../media/Textures/2D/ui.dds assets
cp ../../../media/Fractal/Fractal.kfx assets
cp ../../../media/Fractal/Fractal.uiml assets

ant debug
adb install -r bin/KlayGE_Fractal-debug.apk
