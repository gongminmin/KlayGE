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
cp ../../../../media/RenderFX/DepthToSM.kfx assets
cp ../../../../media/RenderFX/RenderableHelper.kfx assets
cp ../../../../media/RenderFX/UI.kfx assets
cp ../../../../media/PostProcessors/ColorGrading.ppml assets
cp ../../../../media/PostProcessors/DepthToSM.ppml assets
cp ../../../../media/Textures/2D/powered_by_klayge.dds assets
cp ../../../../media/Textures/2D/ui.dds assets
cp ../../../../media/Textures/Cube/uffizi_cross_c.dds assets
cp ../../../../media/Textures/Cube/uffizi_cross_y.dds assets
cp ../../../../Samples/media/Common/teapot.meshml.model_bin assets
cp ../../../media/Refract/Refract.kfx assets
cp ../../../media/Refract/Refract.uiml assets

ant debug
adb install -r bin/KlayGE_Refract-debug.apk
