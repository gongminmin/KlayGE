#!/bin/bash

$ANDROID_NDK/ndk-build -j 3

if [ ! -d assets ]; then
	mkdir assets
fi
rm -rf assets\*.*

cp ../../../../bin/android_armeabi/KlayGE.cfg assets
cp ../../../../media/Fonts/gkai00mp.kfont assets
cp ../../../../media/RenderFX/Font.kfx assets
cp ../../../../media/RenderFX/RenderableHelper.kfx assets
cp ../../../../Samples/media/Common/teapot.meshml.model_bin assets

ant debug
adb install -r bin/KlayGE_Tutor2-debug.apk
