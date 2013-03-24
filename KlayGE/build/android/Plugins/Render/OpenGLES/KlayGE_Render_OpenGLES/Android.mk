LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := KlayGE_Render_OpenGLES
LOCAL_SRC_FILES := ../../../../../../lib/android_$(TARGET_ARCH_ABI)/libKlayGE_Render_OpenGLES.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../../../../Plugins/Include \
		$(LOCAL_PATH)/../../../../../../../glloader/include

include $(PREBUILT_STATIC_LIBRARY)
