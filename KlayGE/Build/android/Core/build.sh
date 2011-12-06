export PATH=$PATH:$ANDROID_NDK

ndk-build

mkdir ../../../lib/android_armeabi
cp obj/local/armeabi/libKlayGE_Core_gcc.a ../../../lib/android_armeabi/

mkdir ../../../lib/android_armeabi-v7a
cp obj/local/armeabi-v7a/libKlayGE_Core_gcc.a ../../../lib/android_armeabi-v7a/
