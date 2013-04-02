LOCAL_PATH := $(call my-dir)

KLAYGE_PLUGIN_PYTHON_PATH := $(LOCAL_PATH)
KLAYGE_PLUGIN_PYTHON_SRC_PATH := $(LOCAL_PATH)/../../../../../../Plugins/Src/Script/Python

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(KLAYGE_PLUGIN_PYTHON_SRC_PATH)/../../../../../External/boost \
		$(KLAYGE_PLUGIN_PYTHON_SRC_PATH)/../../../../../External/Python/include \
		$(KLAYGE_PLUGIN_PYTHON_SRC_PATH)/../../../../../KFL/include \
		$(KLAYGE_PLUGIN_PYTHON_SRC_PATH)/../../../../Core/Include \
		$(KLAYGE_PLUGIN_PYTHON_SRC_PATH)/../../../Include \
		$(KLAYGE_PLUGIN_PYTHON_SRC_PATH)/../../../../../External/android_native_app_glue
		
LOCAL_MODULE := KlayGE_Script_Python
LOCAL_PATH := $(KLAYGE_PLUGIN_PYTHON_SRC_PATH)
CPP_FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES := $(CPP_FILE_LIST:$(LOCAL_PATH)/%=%)
		
LOCAL_CFLAGS := -DKLAYGE_BUILD_DLL -DKLAYGE_PYTHON_SE_SOURCE

LOCAL_LDLIBS := -llog

include $(BUILD_STATIC_LIBRARY)
