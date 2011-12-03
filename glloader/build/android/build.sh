export PATH=$PATH:$ANDROID_NDK

# Workaround for NDK r7
mkdir obj
mkdir obj/local
mkdir obj/local/armeabi
mkdir obj/local/armeabi-v7a
cp $ANDROID_NDK/sources/cxx-stl/gnu-libstdc++/libs/armeabi/libgnustl_static.a obj/local/armeabi/
cp $ANDROID_NDK/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a/libgnustl_static.a obj/local/armeabi-v7a/

ndk-build
