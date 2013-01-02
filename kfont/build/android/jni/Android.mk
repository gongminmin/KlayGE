LOCAL_PATH := $(call my-dir)
KFONT_PATH := $(LOCAL_PATH)
KFONT_SRC_PATH := $(LOCAL_PATH)/../../../src

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(KFONT_SRC_PATH)/../../External/boost \
	$(KFONT_SRC_PATH)/../../External/7z \
	$(KFONT_SRC_PATH)/../../KFL/include \
	$(KFONT_SRC_PATH)/../include \
	
LOCAL_MODULE := kfont
LOCAL_SRC_FILES := kfont.cpp
LOCAL_PATH := $(KFONT_SRC_PATH)
LOCAL_CFLAGS := -DKFONT_SOURCE
include $(BUILD_STATIC_LIBRARY)
