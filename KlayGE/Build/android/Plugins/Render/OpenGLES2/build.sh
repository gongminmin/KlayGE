#!/bin/bash

$ANDROID_NDK/ndk-build

cp libs/armeabi/libKlayGE_RenderEngine_OpenGLES2_gcc.so ../../../../../bin/android_armeabi/Render/
cp libs/armeabi-v7a/libKlayGE_RenderEngine_OpenGLES2_gcc.so ../../../../../bin/android_armeabi-v7a/Render/
cp libs/x86/libKlayGE_RenderEngine_OpenGLES2_gcc.so ../../../../../bin/android_x86/Render/
