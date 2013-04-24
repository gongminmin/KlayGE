LOCAL_PATH := $(call my-dir)

REFRACT_PATH := $(LOCAL_PATH)
REFRACT_SRC_PATH := $(LOCAL_PATH)/../../../../src/Refract

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(REFRACT_SRC_PATH)/../../../../External/boost \
		$(REFRACT_SRC_PATH)/../../../../KFL/include \
		$(REFRACT_SRC_PATH)/../../../Core/Include \
		
LOCAL_MODULE := Refract
LOCAL_PATH := $(REFRACT_SRC_PATH)
CPP_FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES := $(CPP_FILE_LIST:$(LOCAL_PATH)/%=%)
LOCAL_LDLIBS := -llog -landroid
LOCAL_WHOLE_STATIC_LIBRARIES := KlayGE_Render_OpenGLES KlayGE_Scene_OCTree KlayGE_Input_MsgInput
LOCAL_STATIC_LIBRARIES := KlayGE_Core KFL glloader kfont MeshMLLib \
		boost_chrono boost_filesystem boost_system boost_thread \
		LZMA my_android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module, boost)
$(call import-module, LZMA)
$(call import-module, KFL)
$(call import-module, Core/KlayGE_Core)
$(call import-module, Plugins/Render/OpenGLES/KlayGE_Render_OpenGLES)
$(call import-module, Plugins/Scene/OCTree/KlayGE_Scene_OCTree)
$(call import-module, Plugins/Input/MsgInput/KlayGE_Input_MsgInput)
$(call import-module, glloader)
$(call import-module, kfont)
$(call import-module, MeshMLLib)
$(call import-module, android_native_app_glue)
