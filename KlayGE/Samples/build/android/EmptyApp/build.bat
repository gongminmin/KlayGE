call %ANDROID_NDK%\ndk-build

call ant debug
adb install -r bin\KlayGE_EmptyApp-debug.apk
