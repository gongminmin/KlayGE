LOCAL_PATH := $(call my-dir)
KFONT_PATH := $(LOCAL_PATH)
KFONT_SRC_PATH := $(LOCAL_PATH)/../../../src

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(KFONT_SRC_PATH)/../../External/boost \
	$(KFONT_SRC_PATH)/../../External/7z \
	$(KFONT_SRC_PATH)/../../KFL/include \
	$(KFONT_SRC_PATH)/../include \
	
LOCAL_MODULE := kfont
LOCAL_PATH := $(KFONT_SRC_PATH)
CPP_FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES := $(CPP_FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_CFLAGS := -DKFONT_SOURCE

LOCAL_LDLIBS := -llog

include $(BUILD_STATIC_LIBRARY)
