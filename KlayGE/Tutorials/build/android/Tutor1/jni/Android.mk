LOCAL_PATH := $(call my-dir)

EMPTYAPP_PATH := $(LOCAL_PATH)
EMPTYAPP_SRC_PATH := $(LOCAL_PATH)/../../../../src/EmptyApp

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(EMPTYAPP_SRC_PATH)/../../../External/boost \
		$(EMPTYAPP_SRC_PATH)/../../Core/Include \
		
LOCAL_MODULE := EmptyApp
LOCAL_PATH := $(EMPTYAPP_SRC_PATH)
LOCAL_SRC_FILES := EmptyApp.cpp
LOCAL_LDLIBS := -llog -landroid
LOCAL_STATIC_LIBRARIES := KlayGE_Core KlayGE_RenderEngine_OpenGLES KlayGE_Scene_OCTree glloader kfont \
		boost_date_time boost_filesystem boost_signals boost_system boost_thread \
		LZMA my_android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module, boost)
$(call import-module, LZMA)
$(call import-module, Core/KlayGE_Core)
$(call import-module, Plugins/Render/OpenGLES/KlayGE_RenderEngine_OpenGLES)
$(call import-module, Plugins/Scene/OCTree/KlayGE_Scene_OCTree)
$(call import-module, glloader)
$(call import-module, kfont)
$(call import-module, android_native_app_glue)
