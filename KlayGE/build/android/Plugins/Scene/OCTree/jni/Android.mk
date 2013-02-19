LOCAL_PATH := $(call my-dir)

KLAYGE_PLUGIN_OCTREE_PATH := $(LOCAL_PATH)
KLAYGE_PLUGIN_OCTREE_SRC_PATH := $(LOCAL_PATH)/../../../../../../Plugins/Src/Scene/OCTree

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(KLAYGE_PLUGIN_OCTREE_SRC_PATH)/../../../../../External/boost \
		$(KLAYGE_PLUGIN_OCTREE_SRC_PATH)/../../../../../KFL/include \
		$(KLAYGE_PLUGIN_OCTREE_SRC_PATH)/../../../../Core/Include \
		$(KLAYGE_PLUGIN_OCTREE_SRC_PATH)/../../../Include \
		$(KLAYGE_PLUGIN_OCTREE_SRC_PATH)/../../../../../External/android_native_app_glue
		
LOCAL_MODULE := KlayGE_Scene_OCTree
LOCAL_PATH := $(KLAYGE_PLUGIN_OCTREE_SRC_PATH)
CPP_FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES := $(CPP_FILE_LIST:$(LOCAL_PATH)/%=%)
		
LOCAL_CFLAGS := -DKLAYGE_BUILD_DLL -DKLAYGE_OCTREE_SM_SOURCE

LOCAL_LDLIBS := -llog

include $(BUILD_STATIC_LIBRARY)
