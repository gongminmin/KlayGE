export PATH=$PATH:$ANDROID_NDK

ndk-build

mkdir ../../lib/android_armeabi
cp obj/local/armeabi/libglloader.a ../../lib/android_armeabi/

mkdir ../../lib/android_armeabi-v7a
cp obj/local/armeabi-v7a/libglloader.a ../../lib/android_armeabi-v7a/

mkdir ../../lib/android_x86
cp obj/local/x86/libglloader.a ../../lib/android_x86/
