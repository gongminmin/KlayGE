LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := KlayGE_RenderEngine_OpenGLES
LOCAL_SRC_FILES := ../../../../../../lib/android_$(TARGET_ARCH_ABI)/libKlayGE_RenderEngine_OpenGLES_gcc.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../../../../Plugins/Include \
		$(LOCAL_PATH)/../../../../../../../glloader/include

include $(PREBUILT_STATIC_LIBRARY)
