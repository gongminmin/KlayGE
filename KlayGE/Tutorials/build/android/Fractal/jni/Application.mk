NDK_MODULE_PATH := $(call my-dir)/../../../../../../External/boost/android$(HOST_DIRSEP)$(call my-dir)/../../../../../../External$(HOST_DIRSEP)$(call my-dir)/../../../../../../External/7z/build/android/LZMA$(HOST_DIRSEP)$(call my-dir)/../../../../../Build/android$(HOST_DIRSEP)$(call my-dir)/../../../../../../KFL/build/android$(HOST_DIRSEP)$(call my-dir)/../../../../../../glloader/build/android$(HOST_DIRSEP)$(call my-dir)/../../../../../../kfont/build/android$(HOST_DIRSEP)$(call my-dir)/../../../../../../MeshMLLib/build/android

APP_BUILD_SCRIPT := $(call my-dir)/Android.mk

APP_OPTIM := release

APP_PLATFORM 	:= android-9
APP_STL 		:= gnustl_static
APP_CPPFLAGS 	:= -fexceptions -frtti
APP_ABI 		:= armeabi armeabi-v7a x86
APP_MODULES     := Fractal

APP_TOOLCHAIN_VERSION := 4.6.3
