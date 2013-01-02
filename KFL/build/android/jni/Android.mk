LOCAL_PATH := $(call my-dir)
KFL_PATH := $(LOCAL_PATH)
KFL_SRC_PATH := $(LOCAL_PATH)/../../../src

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(KFL_SRC_PATH)/../../External/boost \
		$(KFL_SRC_PATH)/../../External/rapidxml \
		$(KFL_SRC_PATH)/../../External/android_native_app_glue \
		$(KFL_SRC_PATH)/../include \
		
LOCAL_MODULE := KFL_gcc
LOCAL_PATH := $(KFL_SRC_PATH)
LOCAL_SRC_FILES := \
		Kernel/CpuInfo.cpp \
		Kernel/DllLoader.cpp \
		Kernel/KFL.cpp \
		Kernel/Log.cpp \
		Kernel/ThrowErr.cpp \
		Kernel/Timer.cpp \
		Kernel/Util.cpp \
		Kernel/XMLDom.cpp \
		Math/Math.cpp \

		
LOCAL_CFLAGS := -DKFL_SOURCE

LOCAL_LDLIBS := -llog

include $(BUILD_STATIC_LIBRARY)
