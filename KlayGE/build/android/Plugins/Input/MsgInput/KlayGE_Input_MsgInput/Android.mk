LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := KlayGE_Input_MsgInput
LOCAL_SRC_FILES := ../../../../../../lib/android_$(TARGET_ARCH_ABI)/libKlayGE_Input_MsgInput.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../../../../Plugins/Include

include $(PREBUILT_STATIC_LIBRARY)
