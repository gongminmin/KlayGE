#!/bin/bash

echo "Building KlayGE Core..."

cd Core

$ANDROID_NDK/ndk-build

if [ ! -d ../../../lib/android_armeabi ]; then
	mkdir ../../../lib/android_armeabi
fi
cp obj/local/armeabi/libKlayGE_Core_gcc.a ../../../lib/android_armeabi/

if [ ! -d ../../../lib/android_armeabi-v7a ]; then
	mkdir ../../../lib/android_armeabi-v7a
fi
cp obj/local/armeabi-v7a/libKlayGE_Core_gcc.a ../../../lib/android_armeabi-v7a/

if [ ! -d ../../../lib/android_x86 ]; then
	mkdir ../../../lib/android_x86
fi
cp obj/local/x86/libKlayGE_Core_gcc.a ../../../lib/android_x86/

cd ..


echo "Building KlayGE OpenGLES2 plugin..."

cd Plugins/Render/OpenGLES2

$ANDROID_NDK/ndk-build

if [ ! -d ../../../../../lib/android_armeabi ]; then
	mkdir ../../../../../lib/android_armeabi
fi
cp obj/local/armeabi/libKlayGE_RenderEngine_OpenGLES2_gcc.a ../../../../../lib/android_armeabi/

if [ ! -d ../../../../../lib/android_armeabi-v7a ]; then
	mkdir ../../../../../lib/android_armeabi-v7a
fi
cp obj/local/armeabi-v7a/libKlayGE_RenderEngine_OpenGLES2_gcc.a ../../../../../lib/android_armeabi-v7a/

if [ ! -d ../../../../../lib/android_x86 ]; then
	mkdir ../../../../../lib/android_x86
fi
cp obj/local/x86/libKlayGE_RenderEngine_OpenGLES2_gcc.a ../../../../../lib/android_x86/

cd ../../..


echo "Building KlayGE OCTree plugin..."

cd Plugins/Scene/OCTree

$ANDROID_NDK/ndk-build

if [ ! -d ../../../../../lib/android_armeabi ]; then
	mkdir ../../../../../lib/android_armeabi
fi
cp obj/local/armeabi/libKlayGE_Scene_OCTree_gcc.a ../../../../../lib/android_armeabi/

if [ ! -d ../../../../../lib/android_armeabi-v7a ]; then
	mkdir ../../../../../lib/android_armeabi-v7a
fi
cp obj/local/armeabi-v7a/libKlayGE_Scene_OCTree_gcc.a ../../../../../lib/android_armeabi-v7a/

if [ ! -d ../../../../../lib/android_x86 ]; then
	mkdir ../../../../../lib/android_x86
fi
cp obj/local/x86/libKlayGE_Scene_OCTree_gcc.a ../../../../../lib/android_x86/

cd ../../..
