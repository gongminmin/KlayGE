#!/bin/bash

$ANDROID_NDK/ndk-build

ant debug
adb install -r bin/KlayGE_EmptyApp-debug.apk
