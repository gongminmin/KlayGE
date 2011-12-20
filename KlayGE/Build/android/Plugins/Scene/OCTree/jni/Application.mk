NDK_MODULE_PATH := $(call my-dir)/../../../../../../../External/boost/android$(HOST_DIRSEP)$(call my-dir)/../../../../Core

APP_BUILD_SCRIPT := $(call my-dir)/Android.mk

APP_OPTIM := release

APP_PLATFORM 	:= android-9
APP_STL 		:= gnustl_static
APP_CPPFLAGS 	:= -fexceptions -frtti
APP_ABI 		:= armeabi armeabi-v7a x86
APP_MODULES     := KlayGE_Scene_OCTree_gcc
