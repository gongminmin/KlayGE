LOCAL_PATH := $(call my-dir)
GLLOADER_PATH := $(LOCAL_PATH)
GLLOADER_SRC_PATH := $(LOCAL_PATH)/../../../src

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(GLLOADER_SRC_PATH) $(GLLOADER_SRC_PATH)/../include
LOCAL_MODULE := glloader
LOCAL_SRC_FILES := glloader_egl.c glloader_gl.c glloader_gles.c glloader_glx.c glloader_wgl.c utils.cpp
LOCAL_PATH := $(GLLOADER_SRC_PATH)
LOCAL_CFLAGS := -DGLLOADER_GLES_SUPPORT -DGLLOADER_SOURCE
include $(BUILD_STATIC_LIBRARY)
