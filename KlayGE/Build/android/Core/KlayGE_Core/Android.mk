LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := KlayGE_Core
LOCAL_SRC_FILES := ../../../../lib/android_$(TARGET_ARCH_ABI)/libKlayGE_Core_gcc.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../../Core/Include

include $(PREBUILT_STATIC_LIBRARY)
