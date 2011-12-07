export PATH=$PATH:$ANDROID_NDK

ndk-build

cp libs/armeabi/libKlayGE_RenderEngine_OpenGLES2_gcc.so ../../../../../bin/android_armeabi/Render/
cp libs/armeabi-v7a/libKlayGE_RenderEngine_OpenGLES2_gcc.so ../../../../../bin/android_armeabi-v7a/Render/
