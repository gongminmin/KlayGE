LOCAL_PATH := $(call my-dir)

BOOST_PATH := ../../../../../../../External/boost
$(call import-module, boost)

KLAYGE_CORE_PATH := ../../../../../../
$(call import-module, KlayGE_Core)

KLAYGE_PLUGIN_OCTREE_PATH := $(LOCAL_PATH)
KLAYGE_PLUGIN_OCTREE_SRC_PATH := $(LOCAL_PATH)/../../../../../../Plugins/Src/Scene/OCTree

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(KLAYGE_PLUGIN_OCTREE_SRC_PATH)/../../../../../External/boost \
		$(KLAYGE_PLUGIN_OCTREE_SRC_PATH)/../../../../Core/Include \
		$(KLAYGE_PLUGIN_OCTREE_SRC_PATH)/../../../Include \
		
LOCAL_MODULE := KlayGE_Scene_OCTree_gcc
LOCAL_PATH := $(KLAYGE_PLUGIN_OCTREE_SRC_PATH)
LOCAL_SRC_FILES := \
		OCTree.cpp \
		OCTreeFactory.cpp \

		
LOCAL_CFLAGS := -DKLAYGE_BUILD_DLL -DKLAYGE_OCTREE_SM_SOURCE

LOCAL_STATIC_LIBRARIES := KlayGE_Core boost_date_time boost_filesystem boost_signals boost_system boost_thread

include $(BUILD_SHARED_LIBRARY)
