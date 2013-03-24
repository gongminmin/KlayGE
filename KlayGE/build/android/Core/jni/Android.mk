LOCAL_PATH := $(call my-dir)
KLAYGE_PATH := $(LOCAL_PATH)
KLAYGE_SRC_PATH := $(LOCAL_PATH)/../../../../Core/Src

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(KLAYGE_SRC_PATH)/../../../External/boost \
		$(KLAYGE_SRC_PATH)/../../../External/android_native_app_glue \
		$(KLAYGE_SRC_PATH)/../../../External/7z \
		$(KLAYGE_SRC_PATH)/../../../KFL/include \
		$(KLAYGE_SRC_PATH)/../../../kfont/include \
		$(KLAYGE_SRC_PATH)/../../../MeshMLLib/include \
		$(KLAYGE_SRC_PATH)/../Include \
		$(KLAYGE_SRC_PATH)/../../Plugins/Include \
		
LOCAL_MODULE := KlayGE_Core
LOCAL_PATH := $(KLAYGE_SRC_PATH)
CPP_FILE_LIST := $(wildcard $(LOCAL_PATH)/AppLayer/*.cpp) \
				$(wildcard $(LOCAL_PATH)/Audio/*.cpp) \
				$(wildcard $(LOCAL_PATH)/Input/*.cpp) \
				$(wildcard $(LOCAL_PATH)/Kernel/*.cpp) \
				$(wildcard $(LOCAL_PATH)/Net/*.cpp) \
				$(wildcard $(LOCAL_PATH)/Pack/*.cpp) \
				$(wildcard $(LOCAL_PATH)/Render/*.cpp) \
				$(wildcard $(LOCAL_PATH)/Script/*.cpp) \
				$(wildcard $(LOCAL_PATH)/Show/*.cpp) \
				$(wildcard $(LOCAL_PATH)/Scene/*.cpp) \
				$(wildcard $(LOCAL_PATH)/UI/*.cpp)
LOCAL_SRC_FILES := $(CPP_FILE_LIST:$(LOCAL_PATH)/%=%)
		
LOCAL_CFLAGS := -DKLAYGE_BUILD_DLL -DKLAYGE_CORE_SOURCE

LOCAL_LDLIBS := -llog

include $(BUILD_STATIC_LIBRARY)
