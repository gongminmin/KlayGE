LOCAL_PATH := $(call my-dir)

TUTOR2_PATH := $(LOCAL_PATH)
TUTOR2_SRC_PATH := $(LOCAL_PATH)/../../../../src/Tutor2

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(TUTOR2_SRC_PATH)/../../../External/boost \
		$(TUTOR2_SRC_PATH)/../../../KFL/include \
		$(TUTOR2_SRC_PATH)/../../Core/Include \
		
LOCAL_MODULE := Tutor2
LOCAL_PATH := $(TUTOR2_SRC_PATH)
LOCAL_SRC_FILES := Tutor2.cpp
LOCAL_LDLIBS := -llog -landroid
LOCAL_STATIC_LIBRARIES := KlayGE_Core KlayGE_RenderEngine_OpenGLES KlayGE_Scene_OCTree KFL glloader kfont MeshMLLib \
		boost_date_time boost_filesystem boost_signals boost_system boost_thread \
		LZMA my_android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module, boost)
$(call import-module, LZMA)
$(call import-module, KFL)
$(call import-module, Core/KlayGE_Core)
$(call import-module, Plugins/Render/OpenGLES/KlayGE_RenderEngine_OpenGLES)
$(call import-module, Plugins/Scene/OCTree/KlayGE_Scene_OCTree)
$(call import-module, glloader)
$(call import-module, kfont)
$(call import-module, MeshMLLib)
$(call import-module, android_native_app_glue)
