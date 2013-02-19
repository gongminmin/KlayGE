LOCAL_PATH := $(call my-dir)
KFL_PATH := $(LOCAL_PATH)
KFL_SRC_PATH := $(LOCAL_PATH)/../../../src

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(KFL_SRC_PATH)/../../External/boost \
		$(KFL_SRC_PATH)/../../External/rapidxml \
		$(KFL_SRC_PATH)/../../External/android_native_app_glue \
		$(KFL_SRC_PATH)/../include \
		
LOCAL_MODULE := KFL
LOCAL_PATH := $(KFL_SRC_PATH)
CPP_FILE_LIST := $(wildcard $(LOCAL_PATH)/Kernel/*.cpp) \
				$(wildcard $(LOCAL_PATH)/Math/*.cpp)
LOCAL_SRC_FILES := $(CPP_FILE_LIST:$(LOCAL_PATH)/%=%)
		
LOCAL_CFLAGS := -DKFL_SOURCE

LOCAL_LDLIBS := -llog

include $(BUILD_STATIC_LIBRARY)
