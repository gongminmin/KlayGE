#!/bin/bash

$ANDROID_NDK/ndk-build -j 3

if [ ! -d assets ]; then
	mkdir assets
fi
rm -rf assets\*.*

cp ../../../../bin/KlayGE.cfg assets
cp ../../../../media/Fonts/gkai00mp.kfont assets
cp ../../../../media/RenderFX/Blur.kfx assets
cp ../../../../media/RenderFX/Copy.kfx assets
cp ../../../../media/RenderFX/Font.kfx assets
cp ../../../../media/RenderFX/LensEffects.kfx assets
cp ../../../../media/RenderFX/PostToneMapping.kfx assets
cp ../../../../media/RenderFX/Resizer.kfx assets
cp ../../../../media/RenderFX/ToneMapping.kfx assets
cp ../../../../media/PostProcessors/Copy.ppml assets
cp ../../../../media/PostProcessors/LensEffects.ppml assets
cp ../../../../media/PostProcessors/PostToneMapping.ppml assets
cp ../../../../media/PostProcessors/Resizer.ppml assets
cp ../../../../media/Textures/3D/color_grading.dds assets

ant debug
adb install -r bin/KlayGE_Tutor1-debug.apk
