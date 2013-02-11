LOCAL_PATH := $(call my-dir)

SAMPLE_COMMON_PATH := $(LOCAL_PATH)
SAMPLE_COMMON_SRC_PATH := $(LOCAL_PATH)/../../../../src/Common

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(SAMPLE_COMMON_SRC_PATH)/../../../../External/boost \
		$(SAMPLE_COMMON_SRC_PATH)/../../../../External/android_native_app_glue \
		$(SAMPLE_COMMON_SRC_PATH)/../../../../KFL/include \
		$(SAMPLE_COMMON_SRC_PATH)/../../../Core/Include \
		
LOCAL_MODULE := SampleCommon
LOCAL_PATH := $(SAMPLE_COMMON_SRC_PATH)
LOCAL_SRC_FILES := SampleCommon.cpp

include $(BUILD_STATIC_LIBRARY)

$(call import-module, android_native_app_glue)
