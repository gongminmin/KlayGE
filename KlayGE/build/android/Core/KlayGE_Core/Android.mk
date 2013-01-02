LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := KlayGE_Core
LOCAL_SRC_FILES := ../../../../lib/android_$(TARGET_ARCH_ABI)/libKlayGE_Core.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../../Core/Include \
			$(LOCAL_PATH)/../../../../../External/android_native_app_glue

include $(PREBUILT_STATIC_LIBRARY)
