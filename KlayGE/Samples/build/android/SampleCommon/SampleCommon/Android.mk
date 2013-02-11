LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := SampleCommon
LOCAL_SRC_FILES := ../../../../lib/android_$(TARGET_ARCH_ABI)/libSampleCommon.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../../src/Common

include $(PREBUILT_STATIC_LIBRARY)
