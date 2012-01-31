LOCAL_PATH := $(call my-dir)

KLAYGE_PLUGIN_OPENGLES_PATH := $(LOCAL_PATH)
KLAYGE_PLUGIN_OPENGLES_SRC_PATH := $(LOCAL_PATH)/../../../../../../Plugins/Src/Render/OpenGLES

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(KLAYGE_PLUGIN_OPENGLES_SRC_PATH)/../../../../../External/boost \
		$(KLAYGE_PLUGIN_OPENGLES_SRC_PATH)/../../../../Core/Include \
		$(KLAYGE_PLUGIN_OPENGLES_SRC_PATH)/../../../Include \
		$(KLAYGE_PLUGIN_OPENGLES_SRC_PATH)/../../../../../glloader/include \
		
LOCAL_MODULE := KlayGE_RenderEngine_OpenGLES_gcc
LOCAL_PATH := $(KLAYGE_PLUGIN_OPENGLES_SRC_PATH)
LOCAL_SRC_FILES := \
		OGLESFrameBuffer.cpp \
		OGLESGraphicsBuffer.cpp \
		OGLESMapping.cpp \
		OGLESQuery.cpp \
		OGLESRenderEngine.cpp \
		OGLESRenderFactory.cpp \
		OGLESRenderLayout.cpp \
		OGLESRenderStateObject.cpp \
		OGLESRenderView.cpp \
		OGLESRenderWindow.cpp \
		OGLESShaderObject.cpp \
		OGLESTexture.cpp \
		OGLESTexture1D.cpp \
		OGLESTexture2D.cpp \
		OGLESTexture3D.cpp \
		OGLESTextureCube.cpp \

		
LOCAL_CFLAGS := -DKLAYGE_BUILD_DLL -DKLAYGE_OGLES_RE_SOURCE -DGLLOADER_GLES_SUPPORT

LOCAL_LDLIBS := -llog -landroid
LOCAL_STATIC_LIBRARIES := KlayGE_Core glloader boost_date_time boost_filesystem boost_signals boost_system boost_thread

include $(BUILD_STATIC_LIBRARY)

$(call import-module, boost)
$(call import-module, KlayGE_Core)
$(call import-module, glloader)
