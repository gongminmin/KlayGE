#!/bin/bash

$ANDROID_NDK/ndk-build -j 3

if [ ! -d assets ]; then
	mkdir assets
fi
rm -rf assets\*.*

cp ../../../../bin/android_armeabi/KlayGE.cfg assets
cp ../../../../media/Fonts/gkai00mp.kfont assets
cp ../../../../media/RenderFX/Font.kfx assets

ant debug
adb install -r bin/KlayGE_Tutor1-debug.apk
