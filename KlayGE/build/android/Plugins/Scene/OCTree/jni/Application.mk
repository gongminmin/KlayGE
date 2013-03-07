APP_BUILD_SCRIPT := $(call my-dir)/Android.mk

APP_OPTIM := release

APP_PLATFORM 	:= android-9
APP_STL 		:= gnustl_static
APP_CPPFLAGS 	:= -std=c++11 -fexceptions -frtti
APP_ABI 		:= armeabi armeabi-v7a x86
APP_MODULES     := KlayGE_Scene_OCTree
