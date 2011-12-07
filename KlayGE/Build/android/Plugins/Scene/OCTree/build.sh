export PATH=$PATH:$ANDROID_NDK

ndk-build

cp libs/armeabi/libKlayGE_Scene_OCTree_gcc.so ../../../../../bin/android_armeabi/Scene/
cp libs/armeabi-v7a/libKlayGE_Scene_OCTree_gcc.so ../../../../../bin/android_armeabi-v7a/Scene/
