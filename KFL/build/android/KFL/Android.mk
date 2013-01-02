LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := KFL
LOCAL_SRC_FILES := ../../../lib/android_$(TARGET_ARCH_ABI)/libKFL_gcc.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../KFL/include \
			$(LOCAL_PATH)/../../../../External/android_native_app_glue

include $(PREBUILT_STATIC_LIBRARY)
